#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my %allowed_writer = map { $_ => 1 } qw(
  1f0cc9 1f1173 1f1175 1f117a 1f117d 1f1180 1f1184 1f1185
  1f118b 1f1190 1f1191 1f1197
);
my ($writes, $control, $initialization, $last_write_line, $g7_line) = (0, 0, 0, 0, 0);
my $line = 0;
while (<$fh>) {
  ++$line;
  if (/^main_ram_loader_write /) {
    my ($dispatch) = /dispatch_sequence=(\d+)/;
    my ($destination) = /physical_destination=([0-9a-f]+)/;
    my ($value) = /value=([0-9a-f]{2})/;
    my ($writer) = /writer_physical_pc=([0-9a-f]+)/;
    die "FAIL: malformed loader-writer receipt at line $line\n"
      unless defined $dispatch && defined $destination && defined $value && defined $writer;
    die "FAIL: loader writer has CDB dispatch provenance at line $line\n" if $dispatch != 0;
    die "FAIL: unexpected game writer $writer at line $line\n" unless $allowed_writer{$writer};
    ++$writes;
    $last_write_line = $line;
    if ($destination =~ /^1f01f[6-9ab]$/) {
      ++$control;
      next;
    }
    if ($destination =~ /^1f10[0-9a-f]{2}$/) {
      die "FAIL: non-sentinel loader initialization byte $value at line $line\n"
        unless $value eq '00' || $value eq 'ff';
      ++$initialization;
      next;
    }
    die "FAIL: unclassified game-owned loader destination $destination at line $line\n";
  }
  $g7_line = $line
    if /^scsi_read_command generation=7 opcode=08 cdb=080012ef0800 start_lba=4847 sector_count=8$/;
}

die "FAIL: expected 128 observed game-owned loader writes, got $writes\n" unless $writes == 128;
die "FAIL: expected 12 control-window writes, got $control\n" unless $control == 12;
die "FAIL: expected 116 sentinel initialization writes, got $initialization\n" unless $initialization == 116;
die "FAIL: missing authentic generation-7 LBA 4847 read\n" unless $g7_line;
die "FAIL: generation-7 read did not follow the observed writer corpus\n"
  unless $last_write_line < $g7_line;

print "PASS: 128 game-owned writes are dispatch-0 control/init state before G7 LBA4847; no G7 loader or record consumer is evidenced\n";
