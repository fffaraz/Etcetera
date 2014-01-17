#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
extern int errno;

#ifndef NOFILE
#define NOFILE 1024
#endif

int go_background(void);
char *hostlookup(unsigned long int);

struct ippkt
{
   struct iphdr ip;
   struct icmphdr icmp;
   char buffer[10000];
}pkt;

int go_background(void)
{
   int fd;
   int fs;
   
   if(getppid() != 1)
   {
      signal(SIGTTOU, SIG_IGN);
      signal(SIGTTIN, SIG_IGN);
      signal(SIGTSTP, SIG_IGN);
      fs=fork();
      if(fs < 0)
      {
         perror("fork");
         exit(1);
      }
      if(fs > 0) exit(0);
      setpgrp();
      fd=open("/dev/tty", O_RDWR);
      if(fd >= 0)
      {
         ioctl(fd, TIOCNOTTY, (char *)NULL);
         close(fd);
      }
   }
   for(fd=0;fd < NOFILE;fd++) close(fd);
   errno=0;
   chdir("/");
   umask(0);
}
               
int main(void)
{
   int s;
   int i;

   setuid(0);   
   if(geteuid() != 0)
   {
      printf("This program requires root privledges\n");
      exit(0);
   }
   go_background();
   s=socket(AF_INET, SOCK_RAW, 1);
   openlog("icmplog", 0, LOG_DAEMON);
   
   while(1)
   {
      read(s, (struct ippkt *)&pkt, 9999);
      if(pkt.ip.ihl != 5)
      {
         syslog(LOG_NOTICE, "suspicious ip options from %s", hostlookup(pkt.ip.daddr));
         continue;
      }         
      if(pkt.icmp.type == ICMP_DEST_UNREACH) syslog(LOG_NOTICE,"destination unreachable from %s", hostlookup(pkt.ip.saddr));
      if(pkt.icmp.type == ICMP_SOURCE_QUENCH) syslog(LOG_NOTICE,"source quench from %s", hostlookup(pkt.ip.saddr));
      if(pkt.icmp.type == ICMP_REDIRECT) syslog(LOG_NOTICE,"source route from %s", hostlookup(pkt.ip.saddr));
      if(pkt.icmp.type == ICMP_ECHO) syslog(LOG_NOTICE,"ping from %s", hostlookup(pkt.ip.saddr));
      if(pkt.icmp.type == ICMP_INFO_REQUEST) syslog(LOG_NOTICE,"info request from %s", hostlookup(pkt.ip.saddr));
   }
}
   

char *hostlookup(unsigned long int in)
{
   static char blah[1024];
   struct in_addr i;
   struct hostent *he;
         
   i.s_addr=in;
   he=gethostbyaddr((char *)&i, sizeof(struct in_addr),AF_INET);
   if(he == NULL) strcpy(blah, inet_ntoa(i));
   else strcpy(blah, he->h_name);
   return blah;
}
                        
                           