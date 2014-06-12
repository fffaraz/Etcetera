#!/usr/bin/perl
# Apache Dos Defender
# AAD detects and Drops Dos attacks

use warnings;
use strict;

die "wrong syntax!\nadd <server-status page> <maximum number of connections> <-f>\nadd http://127.0.0.1/server-status/ 40\n" unless $ARGV[1];

my $fg=0;
my $new_chain=0;
my $forward_to_add=0;
my $count;

$SIG{HUP} = \&exitlog;
$SIG{INT} = \&exitlog;
$SIG{QUIT} = \&exitlog;
$SIG{KILL} = \&exitlog;
$SIG{TERM} = \&exitlog;

foreach my $temp (@ARGV) {
    if ($temp =~ '^-f$') {
        $fg=1;
    }
}

if ($fg==0) {
    exit if fork();
}


foreach my $temp (`iptables -L -n -v -x`) {

    if ($temp =~ 'ADD        tcp') {
        $forward_to_add=1;
    }

    if ($temp =~ 'Chain ADD') {
        $new_chain=1;
    }
}

if ($new_chain==0) {
    &verbose ("iptables ADD chain does not exist\n");
    system ("iptables -N ADD");
}

if ($forward_to_add==0) {
    &verbose ("iptables forward does not exist\n");
    system ("iptables -I INPUT -p tcp --dport 80 -j ADD");
}

my $date=scalar(localtime);
open WLOG,">>/var/log/apache_dos_defender.log" or die "unable to open log file : $! \n";
print WLOG "\nApache Dos Defender started :: $date\n";
&verbose ("Apache Dos Defender Started\n");
while () {

    my @links=`links -dump $ARGV[0]`;
    my $start=0;
    my @temp;
    my @ip;
    my @keys;
    my $line;
    my %hash;
    my $iptables;
    foreach $line (@links) {
        if ($start==1) {
            if ($line =~ /----------------------------------------------------------------------/) {
                $start=0;
                next;
            }
            next if ($line =~ /^\s/);
            @temp=split (/\s+/,$line);
            push @ip , $temp[10];
        }
        if ($line =~ /VHost/) {
            $start=1;
        }
    }
    foreach $line (@ip) {
        $hash{$line}++;
    }
    @keys=keys %hash;
    foreach $line (@keys) {
        &verbose ("INFO ::: $line has $hash{$line} connections.\n");
        if ($hash{$line} > $ARGV[1]) {
            $iptables=`iptables -L -n`;
            if ($line =~ /\?/) {
            $line = s/\?//;
        }
            next if $line == '';
            if ($iptables =~ /$line/g) { 
                next ;
            } else {
                next if $line =~ /127.0.0.1/;
                &verbose ("WARNING *** $line has $hash{$line} connections.\n");
                my $date=scalar(localtime);
                print WLOG "$date :: $line has $hash{$line} connections.\n";
                system "iptables -A ADD -s $line -j DROP";
            }}}
    sleep 1;
    $count++;

    if ($count > 3600) {
        &verbose ("Firewall Reinitialized\n");
        system "iptables -F ADD";
        $count=0;
    }}


sub verbose {
    print "@_" if $fg==1;
}

sub exitlog {
    my $date=scalar(localtime);
    print WLOG ("!!! : $date : ADD Terminated\n");
        &verbose ("ADD Terminated\n");
    exit;
}
