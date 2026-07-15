#!/usr/bin/env perl
use strict;
use warnings;
@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";
my ($zero, $ff, $other) = (0, 0, 0);
while (<$fh>) {
 next unless /main_ram_loader_write .*physical_destination=1f10[0-9a-f]{2} value=([0-9a-f]{2}) .*writer_physical_pc=1f11(?:73|7a|7d|80|84|8b|90|97)/;
 $1 eq '00' ? $zero++ : $1 eq 'ff' ? $ff++ : $other++;
}
die "FAIL: no authenticated main-RAM initialization writes\n" unless $zero + $ff;
die "FAIL: loader initialization window contains non-sentinel data\n" if $other;
print "PASS: main-RAM loader window is bounded initialization data ($zero zero, $ff ff), not level/object semantics\n";
