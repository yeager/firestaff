#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 1 or die "usage: $0 TRACE_CD\n";
open my $fh, '<', $ARGV[0] or die "open: $!\n";

my ($reads, $system_card, $game_owned, $unknown) = (0, 0, 0, 0);
while (<$fh>) {
    next unless /main_ram_control_read/;
    next unless /physical_address=1f01f[7-9a-b]/;
    /reader_physical_pc=([0-9a-f]+)/
        or die "FAIL: control-window read lacks reader provenance\n";
    my $reader = hex $1;
    ++$reads;

    if ($reader >= 0x1f0000 && $reader <= 0x1f7fff) {
        ++$game_owned;
    } elsif ($reader < 0x1f0000 || ($reader >= 0x1fe000 && $reader <= 0x1fffff)) {
        ++$system_card;
    } else {
        ++$unknown;
    }
}

die "FAIL: no authenticated control-window reads\n" unless $reads;
die "FAIL: control window has game-owned reader(s); do not classify this capture as System Card-only\n"
    if $game_owned;
die "FAIL: control window has unclassified reader(s); do not assign CDB or sector semantics\n"
    if $unknown;

print "PASS: $reads control-window reads are System Card-owned ($system_card); no game CDB/sector consumer is proven\n";
