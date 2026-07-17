#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
verify="$root/probes/nexus/firestaff_nexus_v1_verify_vdp1_capture_artifact.sh"
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
printf bios > "$tmp/bios.bin"; printf disc > "$tmp/game.cue"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}'); disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
{
  printf '%s\n' 'FIRESTAFF_NEXUS_MEDNAFEN_VDP1_CAPTURE_PLAN_V1' 'capture_magic=NXSVDP1C' 'capture_version=1' 'capture_header_bytes=136'
  printf 'bios_sha256=%s\ndisc_sha256=%s\n' "$bios_hash" "$disc_hash"
  printf '%s\n' 'route_epoch=7' 'package_fnv1a64=1020304050607080' 'card_fnv1a64=8877665544332211' 'dgn_fnv1a64=0123456789abcdef' 'dgn_size=4096' 'face_fnv1a64=1111111111111111' 'descriptor_fnv1a64=2222222222222222' 'image_candidate_fnv1a64=3333333333333333' 'palette_candidate_fnv1a64=4444444444444444'
} > "$tmp/plan.txt"
perl -e 'my $p=shift; open my $f, ">", $p or die; binmode $f; print $f "NXSVDP1C",pack("NN",1,136),pack("Q>*",7,0x1020304050607080,0x8877665544332211,0x0123456789abcdef,4096,0x1111111111111111,0x2222222222222222,0x3333333333333333,0x4444444444444444),pack("NN",136,16),pack("Q>*",0x5555555555555555,0x6666666666666666,96,0x7777777777777777,32),"opaque-vdp1-data"; close $f;' "$tmp/capture.vdp1"
bash "$verify" --plan "$tmp/plan.txt" --bios "$tmp/bios.bin" --disc "$tmp/game.cue" --capture "$tmp/capture.vdp1"
printf X | dd of="$tmp/capture.vdp1" bs=1 seek=72 conv=notrunc status=none
if bash "$verify" --plan "$tmp/plan.txt" --bios "$tmp/bios.bin" --disc "$tmp/game.cue" --capture "$tmp/capture.vdp1"; then exit 1; fi
echo "nexus VDP1 local artifact verifier: PASS"
