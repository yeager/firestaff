#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "$0")/.." && pwd)
launcher="$repo_root/probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh"
validator="$repo_root/scripts/validate_nexus_saturn_runtime_capture.py"
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT

dd if=/dev/zero of="$tmp_dir/bios.bin" bs=1 count=32 2>/dev/null
dd if=/dev/zero of="$tmp_dir/disc.cue" bs=1 count=32 2>/dev/null
bios_sha=$(shasum -a 256 "$tmp_dir/bios.bin" | awk '{print $1}')
disc_sha=$(shasum -a 256 "$tmp_dir/disc.cue" | awk '{print $1}')

"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest.txt" --skip-frames 300 --frame-limit 8 >/dev/null

grep -Fq 'skip_frames=300' "$tmp_dir/manifest.txt"
grep -Fq 'frame_limit=8' "$tmp_dir/manifest.txt"
grep -Fq 'press_start_frame=0' "$tmp_dir/manifest.txt"
grep -Fq 'press_start_length=1' "$tmp_dir/manifest.txt"
"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-custom.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-custom.txt" --skip-frames 12 --frame-limit 2 \
  --press-start-frame 1000 --press-start-length 60 >/dev/null
grep -Fq 'press_start_frame=1000' "$tmp_dir/manifest-custom.txt"
grep -Fq 'press_start_length=60' "$tmp_dir/manifest-custom.txt"
if "$launcher" --operator-only --launch --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-launch.txt" >/dev/null 2>&1; then
  echo "expected stock-hook rejection" >&2
  exit 1
fi

echo "nexus Saturn raw capture launcher: PASS"
