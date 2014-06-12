
BLOCKDB=”/path/to/ip.blocked.file”
# omit comments lines
IPS=$(grep -Ev "^#" $BLOCKDB)
for i in $IPS do
    iptables -A INPUT -s $i -j DROP
    iptables -A OUTPUT -d $i -j DROP
done
