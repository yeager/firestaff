#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 2 or die "usage: $0 TRACE_CD TRACK02_BIN\n";
open my $trace, '<', $ARGV[0] or die "open trace: $!\n";
open my $track, '<:raw', $ARGV[1] or die "open Track 02: $!\n";
my %bytes;
while (<$trace>) {
    next unless /^pce_cd_data_origin .*cpu_pc=ea9c port=1808 source_generation=2 source_lba=4165 source_offset=(\d+) data=([0-9a-f]{2})$/;
    $bytes{$1} = hex $2 if $1 < 32;
}
die "FAIL: no complete LBA 4165 FIFO-to-System-Card receipt\n" unless keys %bytes == 32;
seek $track, 1156 * 2352 + 16, 0 or die "seek: $!\n";
my $count = read $track, my $raw, 32;
die "FAIL: truncated Track 02 record 0x484\n" unless defined $count && $count == 32;
for my $offset (0 .. 31) {
    die "FAIL: FIFO value mismatch at LBA 4165 offset $offset\n"
        unless $bytes{$offset} == ord substr($raw, $offset, 1);
}
print "PASS: LBA 4165 record 0x484 bytes 0..31 reach System Card ea9c via port 1808; level/object semantics remain unassigned\n";
