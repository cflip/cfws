#!/usr/bin/perl
print "Content-Type: text/plain\n\n";
foreach (sort keys %ENV) {
	print "$_: $ENV{$_}\n";
}
1;
