#!/usr/bin/env perl
use strict;
use warnings;

my $path = shift @ARGV or die "usage: $0 TRACE\n";
open my $trace, '<', $path or die "open $path: $!\n";
my $found = 0;
while (<$trace>) {
    chomp;
    if (/^pce_cd_origin_ram_receipt / && / fifo_sequence=/ && / physical=/) {
        my %field = map { split /=/, $_, 2 } grep { /=/ } split / /;
        die "FAIL: impossible raw-sector offset $field{source_offset}\n"
            if !defined $field{source_offset} || $field{source_offset} >= 2048;
        die "FAIL: receipt must store into main RAM\n"
            if !defined $field{physical} || $field{physical} !~ /^001f[0-7][0-9a-f]{3}$/;
        die "FAIL: CD read value differs from stored RAM value\n"
            if !defined $field{read_value} || !defined $field{stored_value} ||
               $field{read_value} ne $field{stored_value};
        $found++;
        next;
    }
    next unless /^pce_cd_origin_ram_receipt generation=(\d+) source_lba=(\d+) source_offset=(\d+) reader_pc=([0-9a-f]{4}) writer_pc=([0-9a-f]{4}) logical_destination=([0-9a-f]{4}) physical_destination=([0-9a-f]{8}) value=([0-9a-f]{2})$/;
    die "FAIL: impossible raw-sector offset $3\n" if $3 >= 2048;
    die "FAIL: receipt must store into main RAM\n" if $7 !~ /^001f[0-7][0-9a-f]{3}$/;
    $found++;
}
die "FAIL: no source-backed Track02 RAM receipt\n" unless $found;
print "PASS: $found source-backed Track02 FIFO-to-RAM receipt(s)\n";
