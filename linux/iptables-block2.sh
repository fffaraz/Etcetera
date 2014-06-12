
## eth1 is wan port on server ##
/sbin/iptables -A INPUT -i eth1 -s 10.0.0.0/8 -j LOG --log-prefix "IP DROP SPOOF A: "
/sbin/iptables -A INPUT -i eth1 -s 172.16.0.0/12 -j LOG --log-prefix "IP DROP SPOOF B: "
/sbin/iptables -A INPUT -i eth1 -s 192.168.0.0/16 -j LOG --log-prefix "IP DROP SPOOF C: "
/sbin/iptables -A INPUT -i eth1 -s 224.0.0.0/4 -j LOG --log-prefix "IP DROP MULTICAST D: "
/sbin/iptables -A INPUT -i eth1 -s 240.0.0.0/5 -j LOG --log-prefix "IP DROP SPOOF E: "
/sbin/iptables -A INPUT -i eth1 -d 127.0.0.0/8 -j LOG --log-prefix "IP DROP LOOPBACK: "
 
/sbin/iptables -A INPUT -i eth1 -s 10.0.0.0/8 -j DROP
/sbin/iptables -A INPUT -i eth1 -s 172.16.0.0/12 -j DROP
/sbin/iptables -A INPUT -i eth1 -s 192.168.0.0/16 -j DROP
/sbin/iptables -A INPUT -i eth1 -s 224.0.0.0/4 -j DROP
/sbin/iptables -A INPUT -i eth1 -s 240.0.0.0/5 -j DROP
/sbin/iptables -A INPUT -i eth1 -d 127.0.0.0/8 -j DROP
 
/sbin/iptables-save > /root/my-iptables.rules
