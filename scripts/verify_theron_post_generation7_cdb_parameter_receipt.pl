#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %expected_cdb = (
  4 => '080012fb0100',
  5 => '080012f70300',
  6 => '080012fa0100',
);
my %shadow;
my %seen;
my $pending;
while (<$fh>) {
  if (/^post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=[0-9a-f]{4} physical_destination=(1f01e[567]) value=([0-9a-f]{2})$/) {
    $shadow{$1} = $2;
    next;
  }
  if (/^main_ram_loader_e009_dispatch sequence=(\d+) logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04$/) {
    next unless exists $expected_cdb{$1};
    die "FAIL: nested post-G7 parameter dispatch\n" if defined $pending;
    if ($1 == 4) {
      die "FAIL: missing game-RAM register shadow before dispatch 4\n"
        unless ($shadow{'1f01e5'} // '') eq 'ff' &&
               ($shadow{'1f01e6'} // '') eq '20' &&
               ($shadow{'1f01e7'} // '') eq '04';
    }
    $pending = $1;
    next;
  }
  next unless defined $pending;
  next if /^post_generation7_main_ram_write /;
  next if /^pce_cd_register_write /;
  if (/^scsi_read_command generation=\d+ opcode=08 cdb=([0-9a-f]{12}) start_lba=\d+ sector_count=\d+$/) {
    die "FAIL: dispatch $pending CDB changed\n" unless $1 eq $expected_cdb{$pending};
    $seen{$pending} = $1;
    undef $pending;
    next;
  }
  die "FAIL: unexpected row between post-G7 dispatch and CDB\n";
}

die "FAIL: incomplete post-G7 CDB dispatch\n" if defined $pending;
for my $sequence (sort { $a <=> $b } keys %expected_cdb) {
  die "FAIL: missing CDB for dispatch $sequence\n" unless $seen{$sequence};
}
die "FAIL: fixed ABI tuple unexpectedly identifies one CDB\n"
  if $seen{4} eq $seen{5} || $seen{4} eq $seen{6} || $seen{5} eq $seen{6};

print "PASS: game-RAM shadows ff/20/04 before post-G7 JSR e009, while fixed A/X/Y dispatches map to three distinct CDBs; ABI is indirect, not direct LBA/count encoding\n";
