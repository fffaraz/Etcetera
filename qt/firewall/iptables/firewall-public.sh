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
TARGET="/etc/iptables/firewall-public.txt"

# Flush any existing rules
$IPTABLES -F

# used to identify the firewall type
$IPTABLES -X HOME-NETWORK
$IPTABLES -N PUBLIC-NETWORK

# Default Policies
$IPTABLES -P INPUT   DROP
$IPTABLES -P FORWARD DROP
$IPTABLES -P OUTPUT  ACCEPT

# Accept local host
$IPTABLES -A INPUT -i lo --source 127.0.0.1 --destination 127.0.0.1 -j ACCEPT


# Accept established connection packets
$IPTABLES -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT

# Accept HTTP and HTTPS
$IPTABLES -A INPUT -p tcp --dport http -j ACCEPT
$IPTABLES -A INPUT -p tcp --dport https -j ACCEPT

# Accept ssh 
$IPTABLES -A INPUT -p tcp --dport ssh -j ACCEPT

# Allow limited number of pings
$IPTABLES -A INPUT -p icmp -m limit --limit 1/second --limit-burst 5 -j ACCEPT
$IPTABLES -A INPUT -p icmp -j DROP

$IPTABLES_SAVE > $TARGET

# List rules
$IPTABLES -v -L







