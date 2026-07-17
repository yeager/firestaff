#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd); verify="$root/probes/nexus/firestaff_nexus_v1_verify_prs3_material_capture_artifact.sh"; tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
printf '%s\n' 'route_epoch=7' 'package_fnv1a64=1020304050607080' 'card_fnv1a64=8877665544332211' 'entry_index=4' 'compressed_offset=288' 'compressed_length=52' 'declared_output_bytes=128' 'compressed_fnv1a64=1122334455667788' > "$tmp/plan.txt"
perl -e 'my $p=shift; open my $f, ">", $p or die; binmode $f; print $f "NXSPRS3M",pack("NN",1,96),pack("Q>*",7,0x1020304050607080,0x8877665544332211),pack("N*",4,288,52,128),pack("Q>",0x1122334455667788),pack("NN",96,8),pack("Q>*",0x9988776655443322,0x55,12),"opaque!!"; close $f;' "$tmp/capture.prs3"
bash "$verify" --plan "$tmp/plan.txt" --capture "$tmp/capture.prs3"
printf X | dd of="$tmp/capture.prs3" bs=1 seek=56 conv=notrunc status=none
if bash "$verify" --plan "$tmp/plan.txt" --capture "$tmp/capture.prs3"; then exit 1; fi
echo "nexus PRS3 material local artifact verifier: PASS"
