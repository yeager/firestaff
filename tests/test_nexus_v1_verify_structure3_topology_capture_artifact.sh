#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
verify="$root/probes/nexus/firestaff_nexus_v1_verify_structure3_topology_capture_artifact.sh"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
printf bios > "$tmp/bios.bin"
printf disc > "$tmp/game.cue"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}')
disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
{
  printf 'FIRESTAFF_NEXUS_MEDNAFEN_STRUCTURE3_TOPOLOGY_CAPTURE_PLAN_V1\n'
  printf 'capture_magic=NXS3TOP1\ncapture_version=1\ncapture_header_bytes=124\n'
  printf 'bios_sha256=%s\ndisc_sha256=%s\n' "$bios_hash" "$disc_hash"
  printf '%s\n' 'route_epoch=7' 'package_fnv1a64=1020304050607080' 'card_fnv1a64=8877665544332211' 'dgn_fnv1a64=0123456789abcdef' 'dgn_size=4096' 'structure1f_entry_index=2' 'structure3_entry_index=3' 'face_ordinal=4' 'vertex_table_offset=128' 'vertex_table_length=48' 'vertex_table_fnv1a64=1111111111111111' 'referenced_vertex_rows_fnv1a64=2222222222222222' 'normal_offset=256' 'normal_length=12' 'normal_fnv1a64=3333333333333333'
} > "$tmp/plan.txt"
perl -e 'my $p=shift; open my $f, ">", $p or die; binmode $f; print $f "NXS3TOP1", pack("NN",1,124), pack("Q>*",7,0x1020304050607080,0x8877665544332211,0x0123456789abcdef,4096), pack("N*",2,3,4,128,48), pack("Q>*",0x1111111111111111,0x2222222222222222), pack("NN",256,12), pack("Q>",0x3333333333333333), pack("NN",124,16), pack("Q>",0x4444444444444444), "opaque-topology!!"; close $f;' "$tmp/capture.top"
bash "$verify" --plan "$tmp/plan.txt" --bios "$tmp/bios.bin" --disc "$tmp/game.cue" --capture "$tmp/capture.top"
printf X | dd of="$tmp/capture.top" bs=1 seek=76 conv=notrunc status=none
if bash "$verify" --plan "$tmp/plan.txt" --bios "$tmp/bios.bin" --disc "$tmp/game.cue" --capture "$tmp/capture.top"; then exit 1; fi
echo "nexus Structure3 topology local artifact verifier: PASS"
