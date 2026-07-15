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
    # The bounded post-generation-7 trace is emitted by the same run loop
    # while System Card code prepares the CDB. It is provenance, not a route
    # edge, so tolerate it without admitting arbitrary intervening events.
    next if $line =~ /^post_generation7_main_ram_write writer_pc=[0-9a-f]{4} writer_physical_pc=[0-9a-f]{6} logical_destination=[0-9a-f]{4} physical_destination=1f[0-7][0-9a-f]{3} value=[0-9a-f]{2}$/;
    if ($line =~ /^main_ram_loader_write sequence=\d+ dispatch_sequence=(\d+) logical_destination=[0-9a-f]{4} physical_destination=1f[0-7][0-9a-f]{3} value=[0-9a-f]{2} writer_pc=[0-9a-f]{4} writer_physical_pc=1f[0-7][0-9a-f]{3}$/) {
        die "FAIL: main-RAM write belongs to a different dispatch\n" if $1 != $pending;
        next;
    }
    if ($line =~ /^pce_cd_register_write cpu_pc=[0-9a-f]{4} physical=00001801 data=[0-9a-f]{2}$/) {
        die "FAIL: too many CDB writes after main-RAM e009 dispatch\n" if ++$register_writes > 16;
        next;
    }
    if ($line =~ /^scsi_read_command generation=(\d+) opcode=08 cdb=([0-9a-f]{12}) start_lba=(\d+) sector_count=(\d+)$/) {
        my ($cdb, $lba, $sectors) = ($2, $3, $4);
        my @byte = ($cdb =~ /(..)/g);
        my $cdb_lba = ((hex($byte[1]) & 0x1f) << 16) | (hex($byte[2]) << 8) | hex($byte[3]);
        my $cdb_sectors = hex($byte[4]) || 256;
        die "FAIL: invalid SCSI read after main-RAM e009 dispatch\n"
            if $lba < 3009 || $sectors == 0 || $register_writes != 7 ||
               $byte[0] ne '08' || $byte[5] ne '00' ||
               $cdb_lba != $lba || $cdb_sectors != $sectors;
        $count++;
        undef $pending;
        next;
    }
    die "FAIL: main-RAM e009 dispatch was not followed directly by a SCSI read\n";
}
die "FAIL: incomplete main-RAM e009 dispatch\n" if defined $pending;
die "FAIL: no main-RAM e009/SCSI receipts\n" unless $count;
print "PASS: $count main-RAM e009 dispatches bind through byte-consistent CDB writes to authentic SCSI read records; record semantics remain unassigned\n";
