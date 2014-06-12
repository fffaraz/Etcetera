sudo ip link add link <your-nicN> mac0 type macvlan && sudo ifconfig mac0 up

ifconfig mac0 inet 192.168.1.107 netmask 255.255.255.0 #static/manual config

dhclient mac0 # For a dhcp-client, to get ip from router.

sudo ip route add default via 192.168.1.1 

