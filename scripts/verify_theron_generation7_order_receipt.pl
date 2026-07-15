#!/usr/bin/env perl
use strict; use warnings;
@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";
my $phase = 0;
my $expected_byte = 0;
my $waiting_destination = 0;
while (<$fh>) {
 if (/^scsi_read_command generation=7 .*start_lba=4847 sector_count=8$/) { $phase = 1; next; }
 if ($phase == 1 && /^pce_cd_data_origin .*source_generation=7 source_lba=(\d+) source_offset=(\d+) /) {
  my ($lba, $offset) = ($1, $2);
  my $expected_lba = 4847 + int($expected_byte / 2048);
  my $expected_offset = $expected_byte % 2048;
  die "FAIL: generation-7 FIFO origin is not byte-exact at byte $expected_byte\n"
    if $lba != $expected_lba || $offset != $expected_offset;
  $phase = 2;
  next;
 }
 if ($phase == 2 && /^pce_cd_fifo_read generation=7 /) { $waiting_destination = 1; $phase = 3; next; }
 if ($phase == 3 && /^pce_cd_fifo_destination_receipt generation=7 .*logical_destination=000[23] physical_destination=1fe00[23] /) {
  die "FAIL: FIFO destination appeared before FIFO read\n" unless $waiting_destination;
  $waiting_destination = 0;
  ++$expected_byte;
  $phase = $expected_byte == 10240 ? 4 : 1;
  next;
 }
 if ($phase == 4 && /^generation7_fifo_window_complete /) { $phase = 5; next; }
 if ($phase == 5 && /^post_generation7_main_ram_write .*writer_physical_pc=1f1[1-8][0-9a-f]{2}/) {
  print "PASS: generation 7 LBA 4847..4851 / records 0x72e..0x732 are byte-exact FIFO origins; game-RAM relation is ordering only\n";
  exit 0;
 }
}
die "FAIL: incomplete generation-7 FIFO/game-RAM receipt (verified $expected_byte/10240 bytes)\n";
