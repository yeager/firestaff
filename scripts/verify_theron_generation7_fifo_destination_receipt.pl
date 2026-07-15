#!/usr/bin/env perl
use strict;
use warnings;

sub usage {
    die "usage: $0 CUE TRACK02_RAW_BIN CD_TRACE\n";
}

@ARGV == 3 or usage();
my ($cue_path, $raw_path, $trace_path) = @ARGV;
open my $cue, '<', $cue_path or die "FAIL: cannot read CUE: $cue_path\n";
my $cue_text = do { local $/; <$cue> };
close $cue;
$cue_text =~ /TRACK\s+02\s+MODE1\/2352[\s\S]*?INDEX\s+01\s+00:03:00/i
    or die "FAIL: CUE does not retain the authenticated Track 02 MODE1/2352 entry\n";

open my $raw_fh, '<:raw', $raw_path or die "FAIL: cannot read raw Track 02: $raw_path\n";
my $raw = do { local $/; <$raw_fh> };
close $raw_fh;
length($raw) % 2352 == 0 or die "FAIL: raw Track 02 is not MODE1/2352\n";

open my $trace, '<', $trace_path or die "FAIL: cannot read CD trace: $trace_path\n";
my ($command_seen, $marker_seen) = (0, 0);
my (@fifo, @destinations, @mismatches);
while (my $line = <$trace>) {
    chomp $line;
    $marker_seen = 1 if $line eq 'source=mednafen-pce-generation7-fifo-destination-receipt';
    $command_seen = 1
        if $line eq 'scsi_read_command generation=7 opcode=08 cdb=080012ef0800 start_lba=4847 sector_count=8';
    if ($line =~ /^pce_cd_fifo_read generation=7 fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) value=([0-9a-f]{2})$/) {
        push @fifo, [$1, $2, hex($3)];
    }
    if ($line =~ /^pce_cd_fifo_destination_receipt generation=7 fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=([0-9a-f]{6}) value=([0-9a-f]{2})$/) {
        push @destinations, [$1, $2, $3, $4, hex($5)];
    }
    if ($line =~ /^pce_cd_fifo_store_mismatch generation=7 fifo_sequence=(\d+) reader_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=([0-9a-f]{6}) read_value=([0-9a-f]{2}) stored_value=([0-9a-f]{2})$/) {
        push @mismatches, [$1, $2, $3, $4, hex($5), hex($6)];
    }
}
close $trace;

$marker_seen or die "FAIL: generation-7 FIFO receipt provenance marker is missing\n";
$command_seen or die "FAIL: generation 7 is not the authenticated LBA 4847 eight-sector READ(6)\n";
@fifo == 8 * 2048 or die "FAIL: generation-7 FIFO receipt is truncated\n";
@destinations + @mismatches == @fifo
    or die "FAIL: every generation-7 FIFO byte must have one immediate store classification\n";

my $expected = '';
for my $record (0x72e .. 0x735) {
    $expected .= substr($raw, $record * 2352 + 16, 2048);
}
length($expected) == @fifo or die "FAIL: authenticated user-data span has the wrong length\n";

my (%destinations_by_sequence, %mismatches_by_sequence);
for my $destination (@destinations) {
    $destinations_by_sequence{$destination->[0]} = $destination;
}
for my $mismatch (@mismatches) {
    $mismatches_by_sequence{$mismatch->[0]} = $mismatch;
}

for my $index (0 .. $#fifo) {
    my ($sequence, $reader_pc, $value) = @{ $fifo[$index] };
    $sequence == $index or die "FAIL: generation-7 FIFO sequence is not contiguous\n";
    $reader_pc eq 'eb33' or die "FAIL: generation-7 FIFO reader is not the observed System Card routine\n";
    $value == ord(substr($expected, $index, 1))
        or die "FAIL: generation-7 FIFO bytes differ from authenticated Track 02 records 0x72e..0x735\n";

    if (my $destination = $destinations_by_sequence{$sequence}) {
        my ($destination_sequence, $destination_pc, $logical, $physical, $stored) = @$destination;
        $destination_sequence == $sequence && $destination_pc eq $reader_pc && $stored == $value
            or die "FAIL: FIFO and destination receipts lose byte identity\n";
        my $expected_logical = $index & 1 ? '0003' : '0002';
        my $expected_physical = $index & 1 ? '1fe003' : '1fe002';
        $logical eq $expected_logical && $physical eq $expected_physical
            or die "FAIL: generation-7 destination is not the observed System Card CD-register pair\n";
    } elsif (my $mismatch = $mismatches_by_sequence{$sequence}) {
        my ($mismatch_sequence, $mismatch_pc, undef, $physical, $read_value, $stored_value) = @$mismatch;
        $mismatch_sequence == $sequence && $mismatch_pc eq $reader_pc && $read_value == $value
            or die "FAIL: FIFO mismatch receipt loses raw-byte identity\n";
        $physical =~ /^1f[0-7][0-9a-f]{3}$/ && $stored_value != $read_value
            or die "FAIL: FIFO mismatch is not an explicit rejected main-RAM write\n";
    } else {
        die "FAIL: generation-7 FIFO byte has no immediate store classification\n";
    }
}

print "PASS: generation 7 reads 16384 byte-exact MODE1 user-data bytes from Track 02 records 0x72e..0x735 through System Card PC eb33; " . scalar(@destinations) . " immediate bytes reach alternating PCE CD-register destinations 0x1802/0x1803 and " . scalar(@mismatches) . " mismatched main-RAM writes are explicitly rejected, so no raw payload-copy or dungeon/object semantics are admitted\n";
