#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my (%seen, $phase);
while (<$fh>) {
    if (/^main_ram_game_window_read .*physical_address=(1f100[0-7]) .*reader_physical_pc=1f0c88$/) {
        die "FAIL: game-window read arrives after its dispatch\n" if defined $phase;
        $seen{$1} = 1;
        next;
    }
    next unless keys %seen == 8;

    if (!defined $phase && /^main_ram_loader_e009_dispatch .*physical_pc=1f0cc7 /) {
        $phase = 'dispatch';
        next;
    }
    if (defined $phase && $phase eq 'dispatch' &&
        /^scsi_read_command generation=2 .*start_lba=4165 sector_count=4$/) {
        $phase = 'scsi';
        last;
    }
}

die "FAIL: no complete game-owned 1f0c88 read of 1f1000..1f1007\n" unless keys %seen == 8;
die "FAIL: no following game-owned 1f0cc7 e009 dispatch\n" unless defined $phase;
die "FAIL: no following authenticated generation-2 SCSI read\n" unless $phase eq 'scsi';

print "PASS: game-owned 1f1000..1f1007 read leads to 1f0cc7 e009 and SCSI generation 2 (LBA 4165, 4 sectors); record semantics remain unassigned\n";
