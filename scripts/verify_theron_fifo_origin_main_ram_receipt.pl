#!/usr/bin/env perl
use strict; use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %generation_range;
my $receipts = 0;
while (<$fh>) {
    if (/^scsi_read_command generation=(\d+) opcode=08 cdb=[0-9a-f]{12} start_lba=(\d+) sector_count=(\d+)$/) {
        $generation_range{$1} = [$2, $3];
        next;
    }
    next unless /^pce_cd_fifo_origin_main_ram_receipt generation=(\d+) source_lba=(\d+) source_offset=(\d+) fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=(1f[0-7][0-9a-f]{3}) writer_pc=([0-9a-f]{4}) writer_physical_pc=([0-9a-f]{6}) value=([0-9a-f]{2})$/;
    my ($generation, $lba, $offset, $fifo_sequence, $reader_pc,
        $logical_destination, $physical_destination, $writer_pc,
        $writer_physical, $value) = ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10);
    my $range = $generation_range{$generation}
        or die "FAIL: FIFO origin receipt has no preceding READ(6) CDB for generation $generation\n";
    die "FAIL: FIFO origin source offset is outside a raw sector\n" if $offset >= 2048;
    die "FAIL: FIFO origin LBA is outside its observed READ(6) range\n"
        if $lba < $range->[0] || $lba >= $range->[0] + $range->[1];
    die "FAIL: FIFO origin receipt has an invalid main-RAM destination\n"
        unless $physical_destination ge '1f0000' && $physical_destination lt '1f8000';
    die "FAIL: FIFO origin receipt lacks CPU writer provenance\n"
        unless $writer_physical =~ /^(?:00[0-9a-f]{4}|1f[0-7][0-9a-f]{3})$/;
    $receipts++;
}

die "FAIL: no byte-exact FIFO-to-main-RAM receipt was observed\n" unless $receipts;
print "PASS: $receipts byte-exact FIFO-to-main-RAM receipt(s) have CDB and CPU provenance\n";
