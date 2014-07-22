#!/bin/bash
#######################
# 
# CDN 10/12/2009
#
# firewall-laptop.sh
#
# Firewall rules for laptop when working
# away from home
#
# Note: Must be run as root
#
#######################

IPTABLES="sudo /sbin/iptables"
IPTABLES_SAVE="sudo /sbin/iptables-save"
TARGET="/etc/iptables/firewall-home.txt"

# Flush any existing rules
$IPTABLES -F

# used to identify the firewall type
$IPTABLES -X PUBLIC-NETWORK
$IPTABLES -N HOME-NETWORK


# Default Policies
$IPTABLES -P INPUT   DROP
$IPTABLES -P FORWARD DROP
$IPTABLES -P OUTPUT  ACCEPT

# Accept local host
$IPTABLES -A INPUT -i lo --source 127.0.0.1 --destination 127.0.0.1 -j ACCEPT


# Accept established connection packets
$IPTABLES -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT

# Accept anything on local network
$IPTABLES -A INPUT -s 192.168.1.0/24 -j ACCEPT


# Accept HTTP and HTTPS
$IPTABLES -A INPUT -p tcp --dport http -j ACCEPT
$IPTABLES -A INPUT -p tcp --dport https -j ACCEPT

# Accept ssh 
$IPTABLES -A INPUT -p tcp --dport ssh -j ACCEPT

# Allow anything from bigbox based on mac address or ip
# bigbox has two LAN Cards - this only works for hard wired ethernet connections
# $IPTABLES -A INPUT mac --mac-source 00:10:dc:fe:1a:70 -j ACCEPT
# $IPTABLES -A INPUT mac --mac-source 00:00:21:20:4a:31 -j ACCEPT



# Allow limited number of pings
$IPTABLES -A INPUT -p icmp -m limit --limit 1/second --limit-burst 5 -j ACCEPT
$IPTABLES -A INPUT -p icmp -j DROP

$IPTABLES_SAVE > $TARGET
 
# List rules
$IPTABLES -v -L







