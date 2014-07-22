#ifndef _NF_HPP__
#define _NF_HPP__

// vim:ts=4:sts=4:sw=4:noet
extern const int MAX_CAPLEN;

static inline
int nonblock(int fd)
{
	int flags = 0;
	if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
		flags = 0;
	}

	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

struct NFSlave
{
	virtual int cb(struct nflog_data *nfa) =0;
    virtual void hup() = 0;
	virtual ~NFSlave() throw () {};
};

class NFmain{
	NFSlave* handler;
	int fd;
	struct nflog_handle* h;

	typedef std::list<struct nflog_g_handle *> BindT;
	BindT ch_bind;

	static int cb(struct nflog_g_handle *gh,
				  struct nfgenmsg *nfmsg,
				  struct nflog_data *nfa,
				  void *data)
	{
		((NFSlave*)data)->cb(nfa);
		return 0;
	}

  public:
	typedef std::list<int> ChannelT;

    NFmain(const ChannelT& ch, NFSlave* handlerIn)
    : handler(handlerIn), fd(-1), h(0)
    {
		h = nflog_open();
		if (!h) {
			std::cerr << "* nflog_open failed:" << nflog_errno << std::endl;
			throw std::runtime_error("nflog_open");
		}

		std::cerr << "binding nfnetlink_log to AF_INET" << std::endl;
		if (nflog_bind_pf(h, AF_INET) < 0) {
			std::cerr << "* nflog_bind_pf failed:" << nflog_errno << std::endl;
			throw std::runtime_error("nflog_bind_pf");
		}

		for(ChannelT::const_iterator i = ch.begin();
			i != ch.end();
			++i)
		{
			std::cerr << "binding this socket to group " << *i << std::endl;;
			struct nflog_g_handle * qh = nflog_bind_group(h, *i);
			if (!qh) {
				std::cerr << "* nflog_bind_group failed:" << nflog_errno << std::endl;
				throw std::runtime_error("nflog_bind_group");
			}
			nflog_callback_register(qh, &cb, handler);

			std::cerr << "setting copy_packet mode" << std::endl;
			if (nflog_set_mode(qh, NFULNL_COPY_PACKET, MAX_CAPLEN) < 0) {
				std::cerr << "* nflog_set_mode failed:" << nflog_errno << std::endl;
				throw std::runtime_error("nflog_set_mode");
			}

			ch_bind.push_back(qh);
		}

		fd = nflog_fd(h);

		if (nonblock(fd)){
			std::cerr << "* nonblock() failed:" << errno << std::endl;
			throw std::runtime_error("nonblock");
		}
    }

    void loop(volatile sig_atomic_t& term_flag, volatile sig_atomic_t& hup_flag)
    {
		int rv = -1;
		char buf[MAX_CAPLEN];

		struct pollfd pfd;
		pfd.fd = fd;
		while(!term_flag)
		{

			pfd.events = POLLIN;
			pfd.revents = 0;
			int poll_rc = poll(&pfd, 1, 1000);

			if (poll_rc == 1)
			{
				if (pfd.revents == POLLIN)
				{
					memset(&buf, 0, sizeof(buf));
					rv = recv(fd, buf, sizeof(buf), 0);
					if (rv > 0) {
						/* handle messages in just-received packet */
						nflog_handle_packet(h, buf, rv);
					} else {
						std::cerr << "* recv() failed:" << errno << std::endl;
						term_flag = 1;
					}
				} else {
					std::cerr << "* pfd.revents is " << pfd.revents << " (POLLIN expected)" << std::endl;
					term_flag = 1;
				}
			} else if (poll_rc < 0) {
				std::cerr << "* poll() failed:" << errno << std::endl;
			}
			if (hup_flag) {
				handler->hup();
				hup_flag = 0;
			}
		}
	}

	~NFmain() throw()
	{
		for(BindT::iterator i = ch_bind.begin();
			i != ch_bind.end();
			++i)
		{
			nflog_unbind_group(*i);
		}
		nflog_close(h);
	}
};

#endif

