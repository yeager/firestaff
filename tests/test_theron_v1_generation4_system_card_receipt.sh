#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd); trace=$(mktemp); trap 'rm -f "$trace"' EXIT
perl - <<'PERL' >"$trace"
print "scsi_read_command generation=4 opcode=08 cdb=080010891100 start_lba=4233 sector_count=17\n";
for my $index (0 .. (17 * 2048) - 1) {
    my $lba = 4233 + int($index / 2048);
    my $offset = $index % 2048;
    printf "pce_cd_data_origin sequence=%u cpu_pc=ea9c port=1808 source_generation=4 source_lba=%u source_offset=%u data=00\n", $index, $lba, $offset;
}
my @rows = (
    [0, '38', '2256', '1f0256'],
    [1, '50', '2257', '1f0257'],
    [2, '37', '2258', '1f0258'],
    [3, '04', '2259', '1f0259'],
);
for my $row (@rows) {
    my ($sequence, $value, $logical, $physical) = @$row;
    print "main_ram_e009_fifo_read dispatch_sequence=0 generation=4 fifo_sequence=$sequence reader_pc=ea50 value=$value\n";
    print "main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=$sequence logical_destination=$logical physical_destination=$physical value=$value writer_pc=ea52 writer_physical_pc=000a52\n";
}
for my $row (@rows[0 .. 2]) {
    my (undef, $value, $logical, $physical) = @$row;
    print "g4_main_ram_read sequence=1 logical_address=$logical physical_address=$physical value=$value reader_pc=cc1a reader_physical_pc=002c1a\n";
}
PERL
perl "$repo/scripts/verify_theron_generation4_system_card_receipt.pl" "$trace"

sed 's/writer_physical_pc=000a52/writer_physical_pc=1f1852/' "$trace" >"$trace.bad"
if perl "$repo/scripts/verify_theron_generation4_system_card_receipt.pl" "$trace.bad"; then
    printf 'FAIL: verifier accepted a game-owned generation-4 writer\n' >&2
    exit 1
fi
