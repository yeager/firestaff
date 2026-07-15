#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 2 or die "usage: $0 TRACE_CD TRACK02_BIN\n";
my ($trace_path, $track_path) = @ARGV;
open my $trace, '<', $trace_path or die "open trace: $!\n";
open my $track, '<:raw', $track_path or die "open Track 02: $!\n";

my @sectors;
my $collect = 0;
while (<$trace>) {
    if (/^scsi_read_command generation=2 .*start_lba=4165 sector_count=4$/) {
        $collect = 1;
        next;
    }
    next unless $collect;
    last if /^scsi_read_command /;
    if (/^cd_interface_raw_sector_read lba=(\d+) bytes=2352 sector_fnv1a=([0-9a-f]{8}) span_offset=0 span_bytes=32 span_fnv1a=([0-9a-f]{8})$/) {
        push @sectors, [$1, $2, $3];
        last if @sectors == 4;
    }
}

die "FAIL: no authenticated generation-2 LBA 4165/4-sector receipt\n" unless @sectors == 4;

sub fnv1a32 {
    my ($bytes) = @_;
    my $hash = 2166136261;
    $hash = (($hash ^ ord($_)) * 16777619) & 0xffffffff for split //, $bytes;
    return sprintf '%08x', $hash;
}

for my $index (0 .. 3) {
    my ($lba, $sector_hash, $span_hash) = @{$sectors[$index]};
    my $record = $lba - 3009;
    die "FAIL: unexpected LBA ordering\n" unless $lba == 4165 + $index;
    die "FAIL: LBA maps before Track 02\n" if $record < 0;
    seek $track, $record * 2352, 0 or die "seek Track 02: $!\n";
    my $bytes_read = read $track, my $raw, 2352;
    die "read Track 02 record $record: $!\n" unless defined $bytes_read;
    die "FAIL: truncated Track 02 record $record\n" unless $bytes_read == 2352;
    die "FAIL: raw sector hash mismatch at record $record\n" unless fnv1a32($raw) eq $sector_hash;
    die "FAIL: raw sector prefix hash mismatch at record $record\n"
        unless fnv1a32(substr($raw, 0, 32)) eq $span_hash;
}

print "PASS: LBA 4165..4168 binds byte-exactly to Track 02 records 0x484..0x487; level/object semantics remain unassigned\n";
