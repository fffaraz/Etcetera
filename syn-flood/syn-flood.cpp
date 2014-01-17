/*
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

 */

#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<iomanip>
#include<string.h>
#include<getopt.h>

//#define __USE_BSD
#include<netinet/ip.h>
#define __FAVOR_BSD
#include<netinet/tcp.h>

#include<arpa/inet.h>

using namespace std;

unsigned short checksum(unsigned char* packet, int length);

void print_usage (FILE* stream, int exit_code, char* program_name);

int main(int argc, char **argv)
{
	unsigned short d_port = 0;
	unsigned long d_addr = 0;
	int num_packets = 10;
	static int verbose = 0;

	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"verbose",	no_argument,     	&verbose, 	1},
		{"brief",	no_argument,     	&verbose, 	0},
		{"help",	no_argument,     	0, 			'h'},
		{"num",		required_argument,	0, 			'n'},
		{"ip",		required_argument,	0, 			'i'},
		{"port",	required_argument,	0, 			'p'},
		{0, 		0, 					0, 			0}
	};

    int opt;
    int option_index = 0;
    while ((opt = getopt_long (argc, argv, "hi:p:n:", long_options, &option_index)) != -1)
	{
		switch (opt)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
				break;

			case 'h':
				print_usage(stdout, EXIT_SUCCESS, argv[0]);
				break;

			case 'i':
				d_addr=inet_addr(optarg);
				break;

			case 'p':
				d_port = atoi(optarg);
				break;

			case 'n':
				num_packets = atoi(optarg);
				break;

			case '?':
				/* The user specified an invalid option.  */
				print_usage(stderr, EXIT_FAILURE, argv[0]);
				break;

			default:
				cout<<"default"<<endl;
				abort ();
		}
	}

    if(!d_port||!d_addr)
    	print_usage(stderr, EXIT_FAILURE, argv[0]);

	int payload_size = 0;

	unsigned char *packet = (unsigned char*)
			calloc(1,sizeof(struct ip)+sizeof(struct tcphdr)+payload_size);

	//char* payload = (char*)(packet+sizeof(struct ip)+sizeof(struct tcphdr));
	//char data[]={0x20, 0xa8};
	//memcpy(payload, data, 2);

	srand(time(NULL));

	for(int i=0; i!=num_packets; i++)
	{
		unsigned long s_addr = rand();
		unsigned short s_port = rand();

		struct tcphdr *tcph = (struct tcphdr *)
			(packet + sizeof (struct ip));
		tcph->th_sport = s_port;
		tcph->th_dport = htons (d_port);
		tcph->th_seq = rand();
		tcph->th_ack = 0;
		tcph->th_x2 = 0;
		tcph->th_off = 5;
		tcph->th_flags = TH_SYN;
		tcph->th_win = rand();
		tcph->th_sum = 0;
		tcph->th_urp = 0;

		struct pseudo_header
		{
			unsigned long s_addr;
			unsigned long d_addr;
			char zero;
			unsigned char protocol;
			unsigned short length;
		}*pseudo;
		pseudo = (struct pseudo_header *)
				(packet + sizeof(struct ip)-sizeof(struct pseudo_header));

		pseudo->s_addr = s_addr;
		pseudo->d_addr = d_addr;
		pseudo->protocol = IPPROTO_TCP;
		pseudo->length = htons(sizeof(struct tcphdr)+payload_size);
		pseudo->zero = 0;

		tcph->th_sum = checksum((unsigned char *)
			pseudo, sizeof(struct pseudo_header)+sizeof(struct tcphdr)+payload_size);

		struct ip *iph = (struct ip *) (packet);
		iph->ip_hl = 5;
		iph->ip_v = 4;
		iph->ip_tos = 0;
		iph->ip_len = htons(sizeof (struct ip) + sizeof (struct tcphdr)+payload_size);
		iph->ip_id = rand();
		iph->ip_off = 0;
		iph->ip_ttl = rand();
		iph->ip_p = 6;
		iph->ip_sum = 0;
		iph->ip_src.s_addr = s_addr;
		iph->ip_dst.s_addr = d_addr;

		iph->ip_sum = checksum ((unsigned char *) iph, sizeof(struct ip));

        int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
        struct sockaddr_in sin;

        sin.sin_family = AF_INET;
        sin.sin_port = htons (d_port);
        sin.sin_addr.s_addr = d_addr;

		int one = 1;
		const int *val = &one;
		if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
			perror("Error setting socket options");

		if (sendto (s, packet, sizeof(struct ip)+sizeof(struct tcphdr)+payload_size,
			0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
			perror("Error sending packet");
		else
		{
			if(verbose)
			{
				string source(inet_ntoa(iph->ip_src));
				string dest(inet_ntoa(iph->ip_dst));
				cout<<source<<":"<<dec<<ntohs(tcph->th_sport)
					<<" ------> "<<dest<<":"<<dec<<ntohs(tcph->th_dport)<<endl;
			}
		}
		close(s);
	}
	cout<<"Sent "<<num_packets<<" packets."<<endl;
	free(packet);

	return 0;
}

void print_usage (FILE* stream, int exit_code, char* program_name)
{
  fprintf (stream, "Usage:  %s --ip IP --port PORT [verbose]\n", program_name);
  fprintf (stream,
           "  -h  --help             Display this usage information.\n"
           "  -i  --ip               Destination IP address.\n"
		   "  -p  --port             Destination port.\n"
		   "  -n  --num              Number of packets to send.\n"
           "  -v  --verbose          Print verbose messages.\n");
  exit (exit_code);
}

unsigned short checksum(unsigned char* packet, int length)
{
	unsigned int sum = 0;
	int i=0;
	while(i<length)	//16-bit sum
	{
		//is there only 1 byte left (odd length)
		if((i+1)==length)
		{
			sum+=packet[i]; //add it to the sum
			i+=1; //or break
		}
		//2 bytes left (even length)
		else
		{
			sum += (packet[i] | (packet[i+1] << 8));	//add it to the sum
			i+=2;
		}
	}
	unsigned int carry = (sum & 0xFF0000)>>16;	//get carry
	sum &= 0xFFFF;	//remove carry from sum's MSB
	sum += carry;	//add carry to sum for 1's compliment
	return ~sum;
}
