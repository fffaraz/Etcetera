#!/usr/bin/perl

$prefix="192.168.26.";
$start="16";
$stop="254";

while(1)
{

        for($i=$start;$i<=$stop;$i++)
        {
                $ip=$prefix.$i;
                system("ping -s 2 -c 1 -w 1 $ip > /dev/null 2>&1 & ");
        }
        sleep(7);
}
