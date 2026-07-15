#!/usr/bin/env perl
use strict; use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %pending;
my $returns = 0;
while (<$fh>) {
    if (/^main_ram_loader_e009_dispatch sequence=(\d+) logical_pc=([0-9a-f]{4}) physical_pc=(1f[0-7][0-9a-f]{3}) a=[0-9a-f]{2} x=[0-9a-f]{2} y=[0-9a-f]{2}$/) {
        die "FAIL: duplicate pending main-RAM e009 sequence\n" if exists $pending{$1};
        $pending{$1} = [hex($2) + 3, hex($3) + 3];
        next;
    }
    next unless /^main_ram_loader_e009_return sequence=(\d+) logical_pc=([0-9a-f]{4}) physical_pc=(1f[0-7][0-9a-f]{3})$/;
    my $expected = delete $pending{$1}
        or die "FAIL: e009 return has no matching dispatch\n";
    die "FAIL: e009 return target differs from the observed JSR continuation\n"
        unless hex($2) == $expected->[0] && hex($3) == $expected->[1];
    $returns++;
}

die "FAIL: no complete main-RAM e009 dispatch/return sequence was observed\n" unless $returns;
die "FAIL: main-RAM e009 dispatch did not return before trace end\n" if keys %pending;
print "PASS: $returns main-RAM e009 dispatch/return sequence(s) have exact CPU continuity\n";
