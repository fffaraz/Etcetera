#!/bin/bash
## crazy-mac2.sh
## Usage : crazy-mac2.sh <N> </path/to/mac-list.txt>
MACLIST=($(cat ${2}))

# This is for testing, comment this and uncomment out the other for loop
# if this one works the other should also.
for i in $(seq ${1}); do
    echo "mac${i} :  ${MACLIST[${i}-1]}"; done

#for i in $(seq ${1}); do
#    ip link add link wlan0 mac${i} address ${MACLIST[${i}-1]} type macvlan && \
#    ifconfig mac${i} up && \
#    dhclient mac${i};
#done

unset MACLIST
