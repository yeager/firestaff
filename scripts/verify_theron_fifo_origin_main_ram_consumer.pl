#!/usr/bin/env perl
use strict; use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %origins;
my $consumers = 0;
while (<$fh>) {
    if (/^pce_cd_fifo_origin_main_ram_receipt generation=(\d+) source_lba=(\d+) source_offset=(\d+) fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=(1f[0-7][0-9a-f]{3}) writer_pc=([0-9a-f]{4}) writer_physical_pc=([0-9a-f]{6}) value=([0-9a-f]{2})$/) {
        $origins{$4} = [$1, $2, $3, $7, $10];
        next;
    }
    next unless /^pce_cd_fifo_origin_main_ram_consumer sequence=(\d+) generation=(\d+) source_lba=(\d+) source_offset=(\d+) fifo_sequence=(\d+) logical_address=([0-9a-f]{4}) physical_address=(1f[0-7][0-9a-f]{3}) value=([0-9a-f]{2}) reader_pc=([0-9a-f]{4}) reader_physical_pc=(1f[0-7][0-9a-f]{3})$/;
    my ($generation, $lba, $offset, $fifo_sequence, $physical, $value) = ($2, $3, $4, $5, $7, $8);
    my $origin = $origins{$fifo_sequence}
        or die "FAIL: game-owned reader has no preceding FIFO-origin receipt\n";
    die "FAIL: game-owned reader changed FIFO source provenance\n"
        unless $origin->[0] == $generation && $origin->[1] == $lba && $origin->[2] == $offset;
    die "FAIL: game-owned reader changed the tracked RAM cell\n"
        unless $origin->[3] eq $physical && $origin->[4] eq $value;
    $consumers++;
}

die "FAIL: no game-owned CPU consumer of a FIFO-origin RAM cell was observed\n" unless $consumers;
print "PASS: $consumers game-owned FIFO-origin main-RAM consumer(s) have exact raw-CD provenance\n";
