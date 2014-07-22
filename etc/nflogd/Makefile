
nflogd: nflogd.cpp NF.hpp PacketHandler.hpp
	g++ -std=c++0x $(CXXFLAGS) -I. -o nflogd nflogd.cpp -I/usr/include/libnetfilter_log/ -lnetfilter_log -lpcap

all: nflogd

clean:
	rm nflogd

