#!/usr/bin/env perl
use strict;
use warnings;

sub usage {
    die "usage: $0 CUE TRACK02_RAW_BIN CD_TRACE\n";
}

sub fnv1a32 {
    my ($bytes) = @_;
    my $hash = 0x811c9dc5;
    foreach my $byte (unpack('C*', $bytes)) {
        $hash ^= $byte;
        $hash = ($hash * 0x01000193) & 0xffffffff;
    }
    return sprintf('%08x', $hash);
}

@ARGV == 3 or usage();
my ($cue_path, $raw_path, $trace_path) = @ARGV;
open my $cue, '<', $cue_path or die "FAIL: cannot read CUE: $cue_path\n";
my $cue_text = do { local $/; <$cue> };
close $cue;
$cue_text =~ /TRACK\s+02\s+MODE1\/2352[\s\S]*?INDEX\s+01\s+00:03:00/i
    or die "FAIL: CUE does not retain the authenticated US Track 02 INDEX 01 pregap\n";

open my $raw_fh, '<:raw', $raw_path or die "FAIL: cannot read raw Track 02: $raw_path\n";
my $raw = do { local $/; <$raw_fh> };
close $raw_fh;
length($raw) % 2352 == 0 or die "FAIL: raw Track 02 is not MODE1/2352\n";
my $raw_sectors = length($raw) / 2352;
$raw_sectors > 0x739 or die "FAIL: raw Track 02 cannot contain the observed later range\n";

open my $trace, '<', $trace_path or die "FAIL: cannot read CD trace: $trace_path\n";
my @rows;
my $source_count = 0;
while (my $line = <$trace>) {
    chomp $line;
    ++$source_count if $line eq 'source=mednafen-pce-instrumented-cd';
    if ($line =~ /^cd_interface_raw_sector_read lba=(\d+) bytes=2352 sector_fnv1a=([0-9a-f]{8}) span_offset=0 span_bytes=32 span_fnv1a=([0-9a-f]{8})$/) {
        push @rows, [$1, $2, $3];
    }
}
close $trace;
$source_count == 1 or die "FAIL: expected one Mednafen CD provenance marker\n";
@rows >= 39 or die "FAIL: extended later raw-sector witness window is incomplete\n";

my %hash_to_records;
for my $record (0 .. $raw_sectors - 1) {
    my $sector = substr($raw, $record * 2352, 2352);
    push @{ $hash_to_records{fnv1a32($sector)} }, $record;
}

my ($delta, $stage3_seen, $later_count, $later_first, $later_last) = (undef, 0, 0, undef, undef);
for my $row (@rows) {
    my ($lba, $sector_hash, $span_hash) = @$row;
    my $candidates = $hash_to_records{$sector_hash};
    $candidates && @$candidates == 1
        or die "FAIL: captured raw sector does not map uniquely into authenticated Track 02\n";
    my $record = $candidates->[0];
    fnv1a32(substr($raw, $record * 2352, 32)) eq $span_hash
        or die "FAIL: captured raw-sector leading span differs from authenticated Track 02\n";
    my $candidate_delta = $lba - $record;
    if (defined $delta && $candidate_delta != $delta) {
        die "FAIL: raw Track 02 LBA mapping is not stable across captured sectors\n";
    }
    $delta = $candidate_delta;
    $stage3_seen = 1 if $record == 0x4e0;
    if ($record >= 0x72e && $record <= 0x739) {
        ++$later_count;
        $later_first = $record if !defined($later_first) || $record < $later_first;
        $later_last = $record if !defined($later_last) || $record > $later_last;
    }
}
$stage3_seen or die "FAIL: captured trace lacks the authentic US Stage-3 record 0x4e0\n";
$delta == 3009 or die "FAIL: captured physical-to-raw Track 02 delta is not the observed US value\n";
$later_count == 12 && $later_first == 0x72e && $later_last == 0x739
    or die "FAIL: later raw Track 02 range is incomplete\n";

my $manifest = substr($raw, 0x4e0 * 2352 + 16, 0x520);
length($manifest) == 0x520 or die "FAIL: Stage-3 manifest is unavailable\n";
my $first_selector = unpack('n', substr($manifest, 8, 2));
my $base = 0x4e0 - $first_selector;
my %resolved;
for my $ordinal (0 .. 217) {
    my $selector = unpack('n', substr($manifest, 4 + $ordinal * 6 + 4, 2));
    $resolved{$base + $selector} = 1 if $selector;
}
for my $record ($later_first .. $later_last) {
    !$resolved{$record}
        or die "FAIL: later raw range unexpectedly resolves through a Stage-3 descriptor\n";
}

printf "PASS: authentic later raw sectors map LBA-record delta=%d, stage3=0x4e0, later=0x%03x..0x%03x; no Stage-3 descriptor binds the range, so payload semantics remain blocked\n",
    $delta, $later_first, $later_last;
