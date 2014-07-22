#ifndef _PACKET_HANDLER_HPP__
#define _PACKET_HANDLER_HPP__

// vim:ts=4:sts=4:sw=4:noet
extern const int MAX_CAPLEN;

class PacketHandler : public NFSlave
{
	pcap_t* pcap;
	pcap_dumper_t* pcap_writer;
	std::ofstream log_writer;
	std::ostream* logger;
	const std::string log_filename;
	const std::string pcap_filename;

	int
	h_icmp(const char *packet, int& offset)
	{
		struct icmphdr* h = (struct icmphdr*)(packet + offset);

		u_int8_t type = h->type;
		u_int8_t code = h->code;

		(*logger) << "\tICMP"
			<< "\t" << (int)type
			<< "\t" << (int)code;

		offset += sizeof(struct icmphdr);
		return 0;
	}

	int
	h_udp(const char *packet, int& offset)
	{
		struct udphdr* h = (struct udphdr*)(packet + offset);

		(*logger) << "\tUDP"
			<< "\t" << ntohs(h->source)
			<< "\t" << ntohs(h->dest);

		offset += sizeof(struct udphdr);
		return 0;
	}

	int
	h_tcp(const char *packet, int& offset)
	{
		struct tcphdr* h = (struct tcphdr*)(packet + offset);

		(*logger) << "\tTCP"
			<< "\t" << ntohs(h->source)
			<< "\t" << ntohs(h->dest);

		offset += h->doff * 4;
		return 0;
	}

	int
	h_ip(const char *packet, int& offset)
	{
		struct iphdr* h = (struct iphdr*)(packet + offset);

		struct in_addr a;
		a.s_addr = h->saddr;

		(*logger) << "\t" << inet_ntoa(a);

		a.s_addr = h->daddr;
		(*logger) << "\t" << inet_ntoa(a);

		offset += h->ihl*4;
		return h->protocol;
	}

	void
	handle_packet(const char* packet, int payload_len)
	{
		// FIXME: handle payload_len
		int offset = 0;
		// we get IP packet from NFLOG

		int next = h_ip(packet, offset);
		switch(next)
		{
			case IPPROTO_TCP:
				h_tcp(packet, offset);
				break;
			case IPPROTO_UDP:
				h_udp(packet, offset);
				break;
			case IPPROTO_ICMP:
				h_icmp(packet, offset);
				break;
			default:
				(*logger) << "\t" << next;
				break;
		}
	}

  void close_pcap_dump()
  {
		if (pcap_writer) {
			pcap_dump_flush(pcap_writer);
			pcap_dump_close(pcap_writer);
			pcap_writer = NULL;
		}
  }

  void open_logs()
  {
	  if (pcap_filename.length())
	  {
		  close_pcap_dump();
		  pcap_writer = pcap_dump_open(pcap, pcap_filename.c_str());
		  std::cerr << "writing pcap header to " << pcap_filename << std::endl;
	  }

	  if (log_filename.length())
	  {
		  log_writer.close();
		  log_writer.open(log_filename.c_str(), std::ios::trunc | std::ios::out);
		  if (!log_writer.good()) {
			std::cerr << "* fail to open log-file for writing" << std::endl;
			throw std::runtime_error("fail to open log-file for writing");
		  }
		  std::cerr << "logging to " << log_filename << std::endl;
		  logger = &log_writer;
	  } else {
		  std::cerr << "logging to stdout" << std::endl;
		  logger = &std::cout;
	  }
  }

  public:
    PacketHandler(const std::string& log_name, const std::string& pcap_name)
    : pcap_writer(NULL),
	  logger(NULL),
      log_filename(log_name),
      pcap_filename(pcap_name)
    {
		pcap = pcap_open_dead(DLT_EN10MB, MAX_CAPLEN);
		open_logs();
    }

	~PacketHandler() throw ()
	{
		close_pcap_dump();
		pcap_close(pcap);
	}

    // reopen logs
    virtual void hup()
    {
		open_logs();
    }

    virtual int cb(struct nflog_data *nfa)
    {
		//struct nfulnl_msg_packet_hdr *ph = nflog_get_msg_packet_hdr(nfa);
		char *prefix = nflog_get_prefix(nfa);
		char *payload = 0;
		int payload_len = nflog_get_payload(nfa, &payload);

		struct timeval tv;
		memset(&tv, 0, sizeof(tv));
		nflog_get_timestamp(nfa, &tv);
		(*logger)
			<< (unsigned)tv.tv_sec << "."
			<< tv.tv_usec;

		(*logger) << "\t" << nflog_get_indev(nfa);
		(*logger) << "\t" << nflog_get_outdev(nfa);
		(*logger) << "\t" << payload_len;

		handle_packet(payload, payload_len);

		if (prefix && strlen(prefix)) {
			(*logger) << "\t\"" << prefix << "\"";
		}

		(*logger) << std::endl;

		// write a pcap file here if required
		if (pcap_writer) {
			const size_t pcap_len = payload_len+sizeof(ether_header);
			pcap_pkthdr head;
			memset(&head, 0, sizeof(head));
			head.ts = tv;
			head.caplen = pcap_len;
			head.len = pcap_len;

			// make pcap header
			unsigned char tbuf[pcap_len];
			ether_header* ehead = reinterpret_cast<ether_header*>(&tbuf[0]);
			memset(ehead, 0, sizeof(ehead));
			ehead->ether_dhost[0]=0xFA; ehead->ether_dhost[1]=0xCE;
			ehead->ether_shost[0]=0xFA; ehead->ether_shost[1]=0xCE;
			*reinterpret_cast<u_int32_t*>(&ehead->ether_dhost[2]) = nflog_get_outdev(nfa);
			*reinterpret_cast<u_int32_t*>(&ehead->ether_shost[2]) = nflog_get_indev(nfa);
			ehead->ether_type=htons(ETHERTYPE_IP);

			// copy payload and dump
			memcpy(tbuf+sizeof(ether_header), payload, payload_len);
			pcap_dump(reinterpret_cast<u_char*>(pcap_writer), &head, reinterpret_cast<const u_char*>(tbuf));
		}

		return 0;
	}
};

#endif /* _PACKET_HANDLER_HPP__ */

