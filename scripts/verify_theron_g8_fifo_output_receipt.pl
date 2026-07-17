#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my ($dispatch, $cdb, $marker, $output, $consumer) = (0, 0, 0, 0, 0);
my $phase = 0;
while (<$fh>) {
  chomp;
  if (/^main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04$/) {
    die "FAIL: duplicate or reordered G8 dispatch\n" unless $phase == 0;
    $dispatch = 1; $phase = 1; next;
  }
  if (/^scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1$/) {
    die "FAIL: G8 CDB lacks its callsite\n" unless $phase == 1;
    $cdb = 1; $phase = 2; next;
  }
  if ($_ eq 'g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4') {
    die "FAIL: G8 marker lacks its CDB\n" unless $phase == 2 && !$marker;
    $marker = 1; next;
  }
  if (/^pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 source_offset=(\d+) fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=(1f[0-7][0-9a-f]{3}) writer_pc=([0-9a-f]{4}) writer_physical_pc=(1f[0-7][0-9a-f]{3}) value=([0-9a-f]{2})$/) {
    die "FAIL: G8 output lacks its opt-in marker\n" unless $phase == 2 && $marker;
    die "FAIL: malformed G8 FIFO output\n" if $1 >= 2352 || !$2;
    $output++; $phase = 3; next;
  }
  if (/^pce_cd_fifo_origin_main_ram_consumer /) {
    ++$consumer; next;
  }
  die "FAIL: unsupported trace row\n" if length;
}
die "FAIL: missing G8 dispatch\n" unless $dispatch;
die "FAIL: missing G8 CDB\n" unless $cdb;
die "FAIL: missing opt-in G8 output marker\n" unless $marker;
die "FAIL: expected exactly one G8 FIFO output\n" unless $output == 1;
die "FAIL: consumer claims belong to a later receipt\n" if $consumer;
print "PASS: G8 0x73a FIFO-to-game-RAM output is capture-attested; consumer remains unclaimed\n";
