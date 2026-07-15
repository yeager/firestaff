#!/usr/bin/env perl
use strict;
use warnings;

sub usage {
    die "usage: $0 CUE TRACK02_RAW_BIN CD_TRACE\n";
}

sub fnv1a32 {
    my ($bytes) = @_;
    my $hash = 0x811c9dc5;
    for my $byte (unpack('C*', $bytes)) {
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
    or die "FAIL: CUE does not retain the authenticated Track 02 MODE1/2352 entry\n";

open my $raw_fh, '<:raw', $raw_path or die "FAIL: cannot read raw Track 02: $raw_path\n";
my $raw = do { local $/; <$raw_fh> };
close $raw_fh;
length($raw) % 2352 == 0 or die "FAIL: raw Track 02 is not MODE1/2352\n";

my %expected = (
    8 => [4859, [0x73a]],
    9 => [4855, [0x736 .. 0x738]],
    10 => [4858, [0x739]],
);
my (%commands, %bindings, %raw_hashes, %fifo, %destinations);
open my $trace, '<', $trace_path or die "FAIL: cannot read CD trace: $trace_path\n";
while (my $line = <$trace>) {
    chomp $line;
    if ($line =~ /^scsi_read_command generation=(8|9|10) opcode=08 cdb=[0-9a-f]{12} start_lba=(\d+) sector_count=(\d+)$/) {
        $commands{$1} = [$2, $3];
    }
    if ($line =~ /^scsi_read_sector_binding generation=(8|9|10) start_lba=(\d+) sector_count=(\d+) lba=(\d+) sector_index=(\d+)$/) {
        push @{ $bindings{$1} }, [$2, $3, $4, $5];
    }
    if ($line =~ /^cd_interface_raw_sector_read lba=(\d+) bytes=2352 sector_fnv1a=([0-9a-f]{8}) /) {
        $raw_hashes{$1} = $2;
    }
    ++$fifo{$1} if $line =~ /^pce_cd_fifo_read generation=(8|9|10) /;
    ++$destinations{$1} if $line =~ /^pce_cd_fifo_destination_receipt generation=(8|9|10) /;
}
close $trace;

for my $generation (8 .. 10) {
    my ($lba, $records) = @{ $expected{$generation} };
    my $command = $commands{$generation}
        or die "FAIL: expected later SCSI generation $generation is missing\n";
    $command->[0] == $lba && $command->[1] == @$records
        or die "FAIL: later SCSI generation $generation has an unexpected raw-media address\n";
    my $rows = $bindings{$generation} || [];
    @$rows == @$records or die "FAIL: later SCSI generation $generation lacks sector bindings\n";
    for my $index (0 .. $#$records) {
        my $record = $records->[$index];
        my $expected_lba = $record + 3009;
        my ($row_lba, $row_count, $bound_lba, $bound_index) = @{ $rows->[$index] };
        $row_lba == $lba && $row_count == @$records && $bound_lba == $expected_lba && $bound_index == $index
            or die "FAIL: later SCSI generation $generation loses the raw-record coordinate\n";
        my $observed_hash = $raw_hashes{$bound_lba}
            or die "FAIL: later SCSI generation $generation lacks a full raw-sector hash\n";
        fnv1a32(substr($raw, $record * 2352, 2352)) eq $observed_hash
            or die "FAIL: later SCSI generation $generation raw sector differs from authenticated Track 02\n";
    }
    !($fifo{$generation} || 0) && !($destinations{$generation} || 0)
        or die "FAIL: this negative receipt is invalid once generation $generation reaches a FIFO consumer\n";
}

print "PASS: generations 8..10 authenticate raw Track 02 records 0x73a, 0x736..0x738, and 0x739 through SCSI/sector bindings, but have no FIFO or destination consumer in the captured boot path; loader/RAM/object semantics remain unbound\n";
