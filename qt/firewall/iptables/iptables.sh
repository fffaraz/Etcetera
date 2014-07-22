#! /bin/sh
### BEGIN INIT INFO
# Provides:          wicd
# Required-Start:    dbus
# Required-Stop:     
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Starts and stops iptables
# Description:       Starts and stops iptables
### END INIT INFO

# Author: CDN 10/12/09
#

IPTABLES="/sbin/iptables"
IPTABLES_RESTORE="/sbin/iptables-restore"

IPTABLES_LIST="$IPTABLES -v -L"

HOME_FIREWALL="/etc/iptables/firewall-home.txt"
PUBLIC_FIREWALL="/etc/iptables/firewall-public.txt"

# default firewall  - symbolic link to one of above files
DEFAULT_FIREWALL="/etc/iptables/firewall-default.txt"


# Flush any existing rules
CLEAR_FIREWALL="$IPTABLES -F"

case "$1" in
  start)
  	echo "Start default firewall"
  	$CLEAR_FIREWALL
  	$IPTABLES_RESTORE < $DEFAULT_FIREWALL
  	$IPTABLES_LIST
		;;

  restart)  
  	echo "Restart default firewall"
  	$CLEAR_FIREWALL
  	$IPTABLES_RESTORE < $DEFAULT_FIREWALL
  	$IPTABLES_LIST
		;;

  home)
  	echo "Start home network firewall"
  	$CLEAR_FIREWALL
  	$IPTABLES_RESTORE < $HOME_FIREWALL
  	$IPTABLES_LIST  	
		;;
		
  public)
  	echo "Start public network firewall"
  	$CLEAR_FIREWALL
  	$IPTABLES_RESTORE < $PUBLIC_FIREWALL
  	$IPTABLES_LIST  	
		;;		
		
  stop)
    $CLEAR_FIREWALL
  	$IPTABLES_LIST
  	echo "All firewall rules removed"
		;;

	*)
		echo "Usage: $SCRIPTNAME {start|home|public|stop|restart}" >&2
		echo "       Defaults to home firewall" >&2		
		exit 1
		;;
esac


exit 0

