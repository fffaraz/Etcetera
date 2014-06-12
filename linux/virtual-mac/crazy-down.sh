#!/bin/sh
## crazy-down.sh
for i in $(seq ${1}); do
    ifconfig mac${i} down && \
    ip link delete mac${i};
done
