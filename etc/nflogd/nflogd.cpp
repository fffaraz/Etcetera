
#if 0
NetFilter Logger Daemon
Copyright (C) 2009,2012 urykhy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

--

very simple daemon to log NFLOG''ed packets from iptables

build with:
 g++ nflogd.c -I/usr/include/libnetfilter_log/ -lnetfilter_log -lpcap

run with:
  sudo ./nflogd --channel 1

FORMAT:
  timestamp indev outdev ip-size src dest proto proto-1 proto-2 ["prefix"]

TODO:
  threads ?
    listener - producer
    worker - file writer

#endif
// vim:ts=4:sts=4:sw=4:noet

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <getopt.h>
#include <pcap.h>
#include <signal.h>
#include <pwd.h>
#include <sys/prctl.h>
#include <grp.h>
#include <limits.h>

extern "C" {
#include <libnetfilter_log.h>
}

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#include <NF.hpp>
#include <PacketHandler.hpp>

// few globals ;)

const int MAX_CAPLEN = 4096;
std::string log_filename;
std::string pcap_filename;
std::string pid_filename;
std::string username;
NFmain::ChannelT channel_list;

#define SETSIG(sig, fun, flags) \
	do { \
		struct sigaction sa;\
		memset(&sa,0,sizeof(struct sigaction)); \
		sa.sa_handler = fun; \
		sa.sa_flags = flags; \
		sigemptyset(&sa.sa_mask); \
		sigaction(sig, &sa, NULL); \
	} while(0)

volatile sig_atomic_t term_flag = 0;
static void term_handler(int signo) {
	term_flag=1;
}
volatile sig_atomic_t hup_flag = 0;
static void hup_handler(int signo) {
	hup_flag=1;
}

enum {
	O_CHANNEL,
	O_PCAP,
	O_DAEMON,
	O_PID,
	O_USER
};

void usage() {
	std::cerr << std::endl <<
		"use arguments:\n"
		"\t--channel <number>   - channel to listen on, ie 1,5-8\n"
		"\t--pcap <filename>    - filename to dump packets in pcap format\n"
		"\t--daemon <filename>  - daemonize and log to filename\n"
		"\t--pid <filename>     - write pid to\n"
		"\t--user <username>    - user to switch after init\n"
		"\n";
}

template<class T>
void tokenize(const std::string& str, T& tokens, const std::string& delimiters)
{
    size_t lastPos = str.find_first_not_of(delimiters, 0);
    size_t pos = str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void parse_range(const std::string& r)
{
	size_t pos = r.find('-');
	if (pos != std::string::npos) {
		int cl = stoi(r.substr(0, pos));
		int cr = stoi(r.substr(pos+1));
		for (; cl <= cr; cl++)
		{
			channel_list.push_back(cl);
		}
		return;
	}
	channel_list.push_back(stoi(r));
}

void parse_channels(const std::string& opt)
{
	std::vector<std::string> l;
	tokenize(opt, l, ",");
	for (size_t i = 0; i < l.size(); i++)
	{
		parse_range(l[i]);
	}
}

int setup_option(int c)
{
	switch(c)
	{
		case O_CHANNEL:parse_channels(optarg); break;
		case O_PCAP:   pcap_filename = optarg; break;
		case O_DAEMON: log_filename = optarg; break;
		case O_PID:    pid_filename = optarg; break;
		case O_USER:   username = optarg; break;
		default: return 0;
	}

	return 1;
}

int setup(int argc, char** argv)
{
	if (argc == 1) {
		return 0;
	}

	static struct option long_options[] = {
		{"channel",   required_argument, 0, O_CHANNEL},
		{"pcap",      required_argument, 0, O_PCAP},
		{"daemon",    required_argument, 0, O_DAEMON},
		{"pid",       required_argument, 0, O_PID},
		{"user",      required_argument, 0, O_USER},
		{0,0,0,0} };

	int c = -1;
	while ( -1 != (c = getopt_long(argc, argv, "", long_options, NULL)) )
	{
		if(!setup_option(c))
		{
			return 0;
		}
	}

	return 1;
}

void
daemonize()
{
	pid_t pid=fork();
	if (-1 == pid) {
		std::cerr << "* fail to fork: " << errno << std::endl;
		exit(-1);
	}

	if (pid) {
		// parent
		_exit(0);
	}

	if (-1 == setsid()) {
		std::cerr << "* setsid(2) failed: " << errno << std::endl;
		exit(-1);
	}

	chdir("/");

	int fd = open("/dev/null",O_RDWR);
	dup2(fd, 0);
	dup2(fd, 1);

	// daemon must close (2) if this is tty
	// it's ok if redirected to file
	if ( isatty(2) ) {
		dup2(fd, 2);
	}

	close(fd);
}

void switch_user()
{
	if (username.empty()) {
		std::cerr << "* still running as root" << std::endl;
		return;
	}
	struct passwd* i = getpwnam(username.c_str());
	if (!i) {
		std::cerr << "* fail to getpwnam for user " << username << std::endl;
		return;
	}

	std::vector<gid_t> gids;
	gids.resize(NGROUPS_MAX);
	int ngroups = NGROUPS_MAX;
	if (getgrouplist(username.c_str(), i->pw_gid, gids.data(), &ngroups) == -1)
	{
		std::cerr << "* fail to get supplementary groups, errno " << errno << std::endl;
		return;
	}
	gids.resize(ngroups);

	if (setgroups(gids.size(), gids.data())) {
		std::cerr << "* fail to set supplementary groups, errno " << errno << std::endl;
        return;
	}
	if (setgid(i->pw_gid)) {
		std::cerr << "* fail to setgid to " << i->pw_gid << std::endl;
		return;
	}
	if (setuid(i->pw_uid)) {
		std::cerr << "* fail to setuid to " << i->pw_uid << std::endl;
		return;
	}
	std::cerr << "user switched to " << username << std::endl;
}

void set_dumpable()
{
	prctl(PR_SET_DUMPABLE, 1);
}

void write_pid()
{
	if(pid_filename.length())
	{
		std::ofstream wr(pid_filename.c_str(), std::ios::trunc | std::ios::out);
		wr << getpid();
	}
}

void clean_pid()
{
	if(pid_filename.length())
	{
		unlink(pid_filename.c_str());
	}
}

void check_pid()
{
	if (pid_filename.length())
	{
		struct stat st;
		int rc = stat(pid_filename.c_str(), &st);

		if (rc==-1 && errno==ENOENT) {
			return;
		}
		std::cerr << "pidfile [" << pid_filename << "] already exists" << std::endl;

		std::ifstream rd(pid_filename.c_str(), std::ios::in);
		pid_t pid=0;
		rd >> pid;
		if (pid && kill(pid, 0))
		{
			std::cerr << "pidfile looks to be stale, continuie" << std::endl;
			return;
		}
		std::cerr << "* other instance seems to run, exiting" << std::endl;
		exit(-1);
	}
}

int main(int argc, char** argv)
{
	if(!setup(argc, argv))
	{
		usage();
		exit(-2);
	}

	check_pid();

	std::cerr << "parsed channels: ";
	std::copy(channel_list.begin(), channel_list.end(),
      std::ostream_iterator<int> (std::cerr, " "));
	std::cerr << std::endl;
	std::cerr << "nflogd starting..." << std::endl;

	PacketHandler ph(log_filename, pcap_filename);
	NFmain nf(channel_list, &ph);

	// daemonize here
	if (log_filename.length()){
		daemonize();
	}

	// setup signal
	SETSIG(SIGTERM, term_handler, SA_RESTART);
	SETSIG(SIGINT, term_handler, SA_RESTART);
	SETSIG(SIGHUP, hup_handler, SA_RESTART);

	write_pid();
	std::cerr << "going into main loop" << std::endl;

	switch_user();
	set_dumpable();
	nf.loop(term_flag, hup_flag);

	std::cerr << "terminating..." << std::endl;
	clean_pid();

	return 0;
}

