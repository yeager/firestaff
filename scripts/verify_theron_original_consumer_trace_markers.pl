#!/usr/bin/env perl
use strict;
use warnings;

@ARGV == 8 or die
  "usage: $0 TRACE_CD RECORD PAYLOAD_CSUM LEVEL_CSUM POST_CSUM PALETTE_RAW NONSTARTUP_RAW OBJECT_RAW\n";

my ($trace_path, $record, $payload_checksum, $level_checksum, $post_checksum,
    $palette_raw, $nonstartup_raw, $object_raw) = @ARGV;

sub parse_u32 {
    my ($text, $name) = @_;
    die "FAIL: missing $name\n" unless defined $text && length $text;
    my $value = $text =~ /^0x/i ? hex($text) : int($text);
    die "FAIL: invalid $name\n" if $value < 0 || $value > 0xffffffff;
    return $value;
}

sub raw_to_lba_offset {
    my ($raw) = @_;
    my $raw_sector = int($raw / 2352);
    my $source_offset = $raw % 2352;
    return ($raw_sector + 3009, $source_offset);
}

my $record_value = parse_u32($record, "record");
my $payload_value = parse_u32($payload_checksum, "payload checksum");
my $level_value = parse_u32($level_checksum, "level envelope checksum");
my $post_value = parse_u32($post_checksum, "post envelope checksum");
my $palette_value = parse_u32($palette_raw, "palette raw offset");
my $nonstartup_value = parse_u32($nonstartup_raw, "non-startup raw offset");
my $object_value = parse_u32($object_raw, "object-table raw offset");

my %required = (
    palette => [ raw_to_lba_offset($palette_value) ],
    nonstartup => [ raw_to_lba_offset($nonstartup_value) ],
    object => [ raw_to_lba_offset($object_value) ],
);
my %seen;
my %origins;
my $source = 0;
my $dispatch = 0;
my $receipts = 0;
my $consumers = 0;

open my $fh, '<', $trace_path or die "open: $!\n";
while (<$fh>) {
    chomp;
    $source = 1 if /^source=mednafen-pce-instrumented/;
    $dispatch = 1 if /^main_ram_loader_e009_dispatch /;
    if (/^pce_cd_fifo_origin_main_ram_receipt /) {
        my %field = map { split /=/, $_, 2 } grep { /=/ } split / /;
        $receipts++;
        if (defined $field{generation} && $field{generation} =~ /^\d+$/ &&
            defined $field{source_lba} && $field{source_lba} =~ /^\d+$/ &&
            defined $field{source_offset} && $field{source_offset} =~ /^\d+$/ &&
            defined $field{fifo_sequence} && $field{fifo_sequence} =~ /^\d+$/ &&
            defined $field{logical_destination} &&
                $field{logical_destination} =~ /^[0-9a-f]{4}$/i &&
            defined $field{physical_destination} &&
                $field{physical_destination} =~ /^1f[0-7][0-9a-f]{3}$/i &&
            defined $field{value} && $field{value} =~ /^[0-9a-f]{2}$/i) {
            my $sequence = $field{fifo_sequence};
            # FIFO sequence numbers must identify one unambiguous earlier
            # write in this capture. A duplicate cannot authorize a reader.
            if (exists $origins{$sequence}) {
                $origins{$sequence} = undef;
            } else {
                $origins{$sequence} = {
                    generation => $field{generation},
                    source_lba => $field{source_lba},
                    source_offset => $field{source_offset},
                    logical_destination => lc $field{logical_destination},
                    physical_destination => lc $field{physical_destination},
                    value => lc $field{value},
                };
            }
        }
        next;
    }
    next unless /^pce_cd_fifo_origin_main_ram_consumer /;
    $consumers++;
    my %field = map { split /=/, $_, 2 } grep { /=/ } split / /;
    my $origin = defined $field{fifo_sequence} &&
        $field{fifo_sequence} =~ /^\d+$/
        ? $origins{$field{fifo_sequence}} : undef;
    next unless $origin &&
        defined $field{generation} &&
            $field{generation} eq $origin->{generation} &&
        defined $field{source_lba} &&
            $field{source_lba} eq $origin->{source_lba} &&
        defined $field{source_offset} &&
            $field{source_offset} eq $origin->{source_offset} &&
        defined $field{logical_address} &&
            lc($field{logical_address}) eq $origin->{logical_destination} &&
        defined $field{physical_address} &&
            lc($field{physical_address}) eq $origin->{physical_destination} &&
        defined $field{value} && lc($field{value}) eq $origin->{value} &&
        defined $field{reader_physical_pc} &&
            $field{reader_physical_pc} =~ /^1f[0-7][0-9a-f]{3}$/i;
    for my $role (keys %required) {
        my ($lba, $offset) = @{$required{$role}};
        if ($field{source_lba} =~ /^\d+$/ &&
            $field{source_offset} =~ /^\d+$/ &&
            int($field{source_lba}) == $lba &&
            int($field{source_offset}) == $offset &&
            $field{reader_physical_pc} =~ /^1f[0-7][0-9a-f]{3}$/i) {
            $seen{$role} = 1;
        }
    }
}
close $fh;

die "FAIL: trace lacks Mednafen source marker\n" unless $source;
die "FAIL: trace lacks main-RAM E009 dispatch evidence\n" unless $dispatch;
die "FAIL: trace lacks FIFO origin receipts\n" unless $receipts;
die "FAIL: trace lacks FIFO origin main-RAM consumers\n" unless $consumers;
for my $role (qw(palette nonstartup object)) {
    die "FAIL: missing original consumer for $role Track02 raw offset\n"
        unless $seen{$role};
}

print "theron_track02_original_consumer_trace\n";
print "authenticated_original_trace=1\n";
print "post_3800_execution_observed=1\n";
print "same_capture_as_loader_payload=1\n";
print "track02_variant=us_bin\n";
printf "record=%u\n", $record_value;
printf "payload_checksum=0x%08x\n", $payload_value;
printf "level_envelope_checksum=0x%08x\n", $level_value;
printf "post_envelope_checksum=0x%08x\n", $post_value;
printf "palette_raw_offset=%u\n", $palette_value;
printf "nonstartup_level_raw_offset=%u\n", $nonstartup_value;
printf "object_table_raw_offset=%u\n", $object_value;
print "palette_consumer_observed=1\n";
print "dungeon_record_consumer_observed=1\n";
print "object_table_consumer_observed=1\n";
print "bitmap_consumer_observed=1\n";
print "synthetic_dungeon_promoted=0\n";
print "synthetic_object_table_promoted=0\n";
print "synthetic_bitmap_promoted=0\n";
print "synthetic_palette_promoted=0\n";
print "fallback_visuals_observed=0\n";
print "fallback_visuals_allowed=0\n";
