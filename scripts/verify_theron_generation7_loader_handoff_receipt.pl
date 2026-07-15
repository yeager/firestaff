#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;

sub usage {
    die "usage: $0 CUE TRACK02_RAW_BIN CD_TRACE\n";
}

@ARGV == 3 or usage();
my ($cue_path, $raw_path, $trace_path) = @ARGV;

# Keep the media-to-FIFO proof separate from this ordered CPU-state receipt.
# A RAM write alone must never be mistaken for a Track 02 payload transfer.
my $fifo_verifier = "$FindBin::Bin/verify_theron_generation7_fifo_destination_receipt.pl";
system($^X, $fifo_verifier, @ARGV) == 0
    or die "FAIL: the generation-7 raw-media/FIFO receipt is not admissible\n";

open my $trace, '<', $trace_path or die "FAIL: cannot read CD trace: $trace_path\n";
my ($complete, $generation8) = (0, 0);
my ($system_card_writes, $game_ram_writes) = (0, 0);
my ($first_system_card, $first_game_ram);
while (my $line = <$trace>) {
    chomp $line;
    if ($line eq 'generation7_fifo_window_complete fifo_sequence=16383') {
        $complete = 1;
        next;
    }
    next unless $complete;
    if ($line eq 'scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1') {
        $generation8 = 1;
        last;
    }
    next unless $line =~ /^post_generation7_main_ram_write writer_pc=([0-9a-f]{4}) writer_physical_pc=([0-9a-f]{6}) logical_destination=([0-9a-f]{4}) physical_destination=(1f[0-7][0-9a-f]{3}) value=([0-9a-f]{2})$/;
    my ($writer_pc, $writer_physical_pc, $logical_destination, $physical_destination, $value) = ($1, $2, $3, $4, $5);
    if ($writer_physical_pc =~ /^00/) {
        ++$system_card_writes;
        $first_system_card //= "$writer_pc/$writer_physical_pc->$logical_destination/$physical_destination:$value";
    }
    if ($writer_physical_pc =~ /^1f/) {
        ++$game_ram_writes;
        $first_game_ram //= "$writer_pc/$writer_physical_pc->$logical_destination/$physical_destination:$value";
    }
}
close $trace;

$complete or die "FAIL: generation-7 FIFO completion marker is missing\n";
$generation8 or die "FAIL: generation-8 boundary is missing after generation 7\n";
$system_card_writes or die "FAIL: no post-generation-7 System Card-to-main-RAM state write was observed\n";
$game_ram_writes or die "FAIL: no post-generation-7 main-RAM code writer was observed\n";

print "PASS: after the byte-exact generation-7 FIFO/CD-register receipt, System Card state writes ($system_card_writes; first $first_system_card) precede main-RAM code writes ($game_ram_writes; first $first_game_ram) before generation 8; this is an ordered loader-state handoff only, not a Track 02 level/object payload claim\n";
