#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my ($patch_opcode, $patch_operand, $dispatch4, $g8) = (0, 0, 0, 0);
my $window_index = 0;
my @window = (
  ['1f01e7', '04'],
  ['1f01e6', '20'],
  ['1f01e5', 'ff'],
);
while (<$fh>) {
  if (/^post_generation7_main_ram_write writer_pc=384d writer_physical_pc=1f184d logical_destination=3837 physical_destination=1f1837 value=1e$/) {
    $patch_opcode = 1;
    next;
  }
  if (/^post_generation7_main_ram_write writer_pc=3852 writer_physical_pc=1f1852 logical_destination=3838 physical_destination=1f1838 value=20$/) {
    die "FAIL: parameter thunk operand precedes opcode patch\n" unless $patch_opcode;
    $patch_operand = 1;
    next;
  }
  if (/^post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=[0-9a-f]{4} physical_destination=(1f01e[567]) value=([0-9a-f]{2})$/) {
    die "FAIL: parameter thunk executed before its code patch\n" unless $patch_opcode && $patch_operand;
    die "FAIL: unexpected parameter thunk store order or value\n"
      unless $window_index < @window && $1 eq $window[$window_index][0] && $2 eq $window[$window_index][1];
    ++$window_index;
    next;
  }
  if (/^main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04$/) {
    die "FAIL: dispatch 4 precedes completed parameter thunk\n" unless $window_index == @window;
    $dispatch4 = 1;
    next;
  }
  if (/^scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1$/) {
    die "FAIL: G8 CDB precedes dispatch 4\n" unless $dispatch4;
    $g8 = 1;
    next;
  }
  die "FAIL: uninspected CD-to-thunk source appeared in corpus\n"
    if /^pce_cd_origin_(?:main_)?ram_receipt .*physical_destination=1f183[78]/;
}

die "FAIL: missing game-owned parameter thunk opcode patch\n" unless $patch_opcode;
die "FAIL: missing game-owned parameter thunk operand patch\n" unless $patch_operand;
die "FAIL: incomplete game-owned parameter thunk stores\n" unless $window_index == @window;
die "FAIL: missing post-thunk dispatch 4\n" unless $dispatch4;
die "FAIL: missing G8 CDB route\n" unless $g8;
print "PASS: game-owned 1f184d/1f1852 patch 1f1837/38, then 1f1837 stages ff/20/04 before JSR e009 and G8; CD source remains unobserved\n";
