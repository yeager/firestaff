#!/usr/bin/env perl
use strict; use warnings;
@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";
my $cdb = 0;
my $origin_count = 0;
my $origin_error = '';
my %fifo;
my %stores;
my %reads;
while (<$fh>) {
 $cdb++ if /^scsi_read_command generation=4 opcode=08 cdb=080010891100 start_lba=4233 sector_count=17$/;
 if (/^pce_cd_data_origin .*cpu_pc=ea9c port=1808 source_generation=4 source_lba=(\d+) source_offset=(\d+) data=[0-9a-f]{2}$/) {
  my ($lba, $offset) = ($1, $2);
  my $expected_lba = 4233 + int($origin_count / 2048);
  my $expected_offset = $origin_count % 2048;
  $origin_error ||= "expected LBA $expected_lba offset $expected_offset, got LBA $lba offset $offset"
   if $lba != $expected_lba || $offset != $expected_offset;
  $origin_count++;
  next;
 }
 if (/^main_ram_e009_fifo_read dispatch_sequence=0 generation=4 fifo_sequence=(\d+) reader_pc=ea50 value=([0-9a-f]{2})$/) {
  $fifo{$1} = $2;
  next;
 }
 if (/^main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=(\d+) logical_destination=(225[6-9]) physical_destination=(1f025[6-9]) value=([0-9a-f]{2}) writer_pc=ea52 writer_physical_pc=000a52$/) {
  $stores{$1} = [$2, $3, $4];
  next;
 }
 if (/^g4_main_ram_read sequence=\d+ logical_address=(225[6-8]) physical_address=(1f025[6-8]) value=([0-9a-f]{2}) reader_pc=[0-9a-f]{4} reader_physical_pc=(00[0-9a-f]{4})$/) {
  $reads{$1} = 1;
 }
}
die "FAIL: expected exactly one generation-4 CDB/LBA receipt\n" unless $cdb == 1;
die "FAIL: generation-4 raw CD span is incomplete or reordered ($origin_count bytes; $origin_error)\n"
 unless $origin_count == 17 * 2048 && !$origin_error;
my @expected = (
 [0, '38', '2256', '1f0256'],
 [1, '50', '2257', '1f0257'],
 [2, '37', '2258', '1f0258'],
 [3, '04', '2259', '1f0259'],
);
for my $row (@expected) {
 my ($sequence, $value, $logical, $physical) = @$row;
 die "FAIL: missing generation-4 FIFO reader receipt $sequence\n"
  unless ($fifo{$sequence} // '') eq $value;
 my $store = $stores{$sequence};
 die "FAIL: generation-4 FIFO store $sequence lacks System Card CPU provenance\n"
  unless $store && $store->[0] eq $logical && $store->[1] eq $physical && $store->[2] eq $value;
}
for my $logical (qw(2256 2257 2258)) {
 die "FAIL: generation-4 main-RAM value $logical lacks a System Card CPU reader\n"
  unless $reads{$logical};
}
print "PASS: complete generation-4 LBA 4233..4249 CD span reaches main RAM through System Card CPU only; no game-owned consumer is proven\n";
