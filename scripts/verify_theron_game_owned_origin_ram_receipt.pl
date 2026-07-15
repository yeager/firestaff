#!/usr/bin/env perl
use strict;
use warnings;

my $path = shift @ARGV or die "usage: $0 TRACE\n";
open my $trace, '<', $path or die "open $path: $!\n";
my $found = 0;
while (<$trace>) {
    next unless /^pce_cd_origin_main_ram_receipt sequence=(\d+) generation=(\d+) source_lba=(\d+) source_offset=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=(1f[0-7][0-9a-f]{3}) writer_pc=([0-9a-f]{4}) writer_physical_pc=(1f[0-7][0-9a-f]{3}) value=([0-9a-f]{2})$/;
    die "FAIL: impossible raw-sector offset $4\n" if $4 >= 2048;
    $found++;
}
die "FAIL: no game-owned Track02 FIFO-to-RAM receipt\n" unless $found;
print "PASS: $found game-owned source-backed Track02 FIFO-to-RAM receipt(s)\n";
