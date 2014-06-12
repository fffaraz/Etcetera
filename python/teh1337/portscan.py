#!/usr/bin/env python
import sys, socket, threading

if len(sys.argv) != 2:
	print "Usage: %s <Target IP>" % sys.argv[0]
	sys.exit(1)

target = sys.argv[1]
open = []
port = 1
thread_limit = 1500

class Scan(threading.Thread):
	global target, open
	def __init__(self,port):
		threading.Thread.__init__(self)
		self.port = port
	def run(self):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try: 
			s.connect((target,self.port))
			open.append("%i" % self.port)
			s.close()
		except: pass
		
scanning = "Scanning host: %s" % target
print scanning
print "-" * len(scanning)

while port <= 65535:
	if threading.activeCount() < thread_limit:
		Scan(port).start()
		port += 1
		sys.stdout.write("\r"+"Port:"+(" " * (len(scanning) - len("Port:%i" % port)))+"%i" % port)

sys.stdout.write('\r'+(" "*(len(scanning))))
print "\rOpen TCP ports:"
for i in range(len(open)): print open[i]
