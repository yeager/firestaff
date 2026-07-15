#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %expected = (
  3 => { generation => 7, lba => 4847, count => 8, x => '00', records => '72e..735' },
  4 => { generation => 8, lba => 4859, count => 1, x => 'ff', records => '73a' },
  5 => { generation => 9, lba => 4855, count => 3, x => 'ff', records => '736..738' },
  6 => { generation => 10, lba => 4858, count => 1, x => 'ff', records => '739' },
);
my (%dispatch, %route, $pending);
my $line = 0;
while (<$fh>) {
  ++$line;
  if (/^main_ram_loader_e009_dispatch /) {
    my ($sequence) = /sequence=(\d+)/;
    next unless defined $sequence && exists $expected{$sequence};
    my ($logical, $physical, $a, $x, $y) =
      /logical_pc=([0-9a-f]+) physical_pc=([0-9a-f]+) a=([0-9a-f]{2}) x=([0-9a-f]{2}) y=([0-9a-f]{2})/;
    die "FAIL: malformed post-G7 dispatch at line $line\n"
      unless defined $logical && defined $physical && defined $a && defined $x && defined $y;
    die "FAIL: post-G7 loader caller changed at line $line\n"
      unless $logical eq '3840' && $physical eq '1f1840' && $a eq '20' && $x eq $expected{$sequence}{x} && $y eq '04';
    die "FAIL: overlapping post-G7 loader dispatch at line $line\n" if defined $pending;
    $dispatch{$sequence} = $line;
    $pending = $sequence;
    next;
  }
  next unless /^scsi_read_command /;
  next unless defined $pending;
  my ($generation, $lba, $count) = /generation=(\d+).*start_lba=(\d+) sector_count=(\d+)/;
  die "FAIL: malformed SCSI command after dispatch $pending\n"
    unless defined $generation && defined $lba && defined $count;
  my $want = $expected{$pending};
  die "FAIL: dispatch $pending has wrong SCSI route at line $line\n"
    unless $generation == $want->{generation} && $lba == $want->{lba} && $count == $want->{count};
  $route{$pending} = $line;
  undef $pending;
}

die "FAIL: unfinished post-G7 loader dispatch $pending\n" if defined $pending;
for my $sequence (sort { $a <=> $b } keys %expected) {
  die "FAIL: missing loader dispatch $sequence\n" unless $dispatch{$sequence};
  die "FAIL: missing SCSI route for loader dispatch $sequence\n" unless $route{$sequence};
}

print "PASS: game loader 1f1840 dispatches post-G7 records 73a, 736..738, 739 via authenticated G8..G10 CDB routes; destination remains unobserved\n";
