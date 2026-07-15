#!/usr/bin/env perl
use strict; use warnings;
@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";
my ($cdb, $origin, $stores) = (0, 0, 0);
while (<$fh>) {
 $cdb = 1 if /^scsi_read_command generation=4 opcode=08 cdb=080010891100 start_lba=4233 sector_count=17$/;
 $origin = 1 if /^pce_cd_data_origin .*cpu_pc=ea9c port=1808 source_generation=4 source_lba=4233 source_offset=0 /;
 $stores++ if /^main_ram_e009_fifo_destination .*generation=4 .*physical_destination=1f025[6-9] .*writer_physical_pc=000a52$/;
}
die "FAIL: missing generation-4 CDB/LBA receipt\n" unless $cdb;
die "FAIL: missing generation-4 System Card FIFO origin\n" unless $origin;
die "FAIL: generation-4 RAM stores are not all System Card-owned\n" unless $stores == 4;
print "PASS: generation 4 LBA 4233..4249 reaches System Card only; no game-RAM consumer is proven\n";
