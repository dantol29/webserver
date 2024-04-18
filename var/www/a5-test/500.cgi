#!/usr/bin/perl

#print "Content-type: text/plain\n\n";

$ls = `ls -alR`;

print "$ls\n";
