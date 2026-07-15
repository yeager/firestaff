#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $trace, '<', $ARGV[0] or die "open $ARGV[0]: $!\n";

my $pending;
my $register_writes = 0;
my $count = 0;
while (my $line = <$trace>) {
    chomp $line;
    if ($line =~ /^main_ram_loader_e009_dispatch sequence=(\d+) logical_pc=([0-9a-f]{4}) physical_pc=(1f[0-7][0-9a-f]{3}) a=([0-9a-f]{2}) x=([0-9a-f]{2}) y=([0-9a-f]{2})$/) {
        die "FAIL: nested main-RAM e009 dispatch\n" if defined $pending;
        $pending = $1;
        $register_writes = 0;
        next;
    }
    next unless defined $pending;
    if ($line =~ /^pce_cd_register_write cpu_pc=[0-9a-f]{4} physical=00001801 data=[0-9a-f]{2}$/) {
        die "FAIL: too many CDB writes after main-RAM e009 dispatch\n" if ++$register_writes > 16;
        next;
    }
    if ($line =~ /^scsi_read_command generation=(\d+) opcode=08 cdb=[0-9a-f]{12} start_lba=(\d+) sector_count=(\d+)$/) {
        die "FAIL: invalid SCSI read after main-RAM e009 dispatch\n" if $2 < 3009 || $3 == 0 || $register_writes != 7;
        $count++;
        undef $pending;
        next;
    }
    die "FAIL: main-RAM e009 dispatch was not followed directly by a SCSI read\n";
}
die "FAIL: incomplete main-RAM e009 dispatch\n" if defined $pending;
die "FAIL: no main-RAM e009/SCSI receipts\n" unless $count;
print "PASS: $count main-RAM e009 dispatches bind through complete CDB writes to authentic SCSI read records; record semantics remain unassigned\n";
