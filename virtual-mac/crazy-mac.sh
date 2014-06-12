#!/bin/sh
## crazy-mac.sh
for i in $(seq ${1}); do
    ip link add link wlan0 mac${i} type macvlan && \
    ifconfig mac${i} up && \
    dhclient mac${i};
done
