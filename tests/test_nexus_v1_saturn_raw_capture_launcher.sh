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
grep -Fq 'press_button_mask=0x10' "$tmp_dir/manifest.txt"
mkdir -p "$tmp_dir/mednafen-home"
"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-home.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-home.txt" \
  --mednafen-home "$tmp_dir/mednafen-home" --no-waiting >/dev/null
grep -Fq "mednafen_home=$tmp_dir/mednafen-home" "$tmp_dir/manifest-home.txt"
grep -Fq 'no_waiting=1' "$tmp_dir/manifest-home.txt"
"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-custom.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-custom.txt" --skip-frames 12 --frame-limit 2 \
  --press-start-frame 1000 --press-start-length 60 --press-button-mask 0x30 >/dev/null
grep -Fq 'press_start_frame=1000' "$tmp_dir/manifest-custom.txt"
grep -Fq 'press_start_length=60' "$tmp_dir/manifest-custom.txt"
grep -Fq 'press_button_mask=0x30' "$tmp_dir/manifest-custom.txt"

python3 - "$tmp_dir/fake-mednafen" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf 'authenticated-test-trace' > \"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
"$launcher" --operator-only --launch --mednafen "$tmp_dir/fake-mednafen" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-real.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-real.txt" >/dev/null
grep -Fq 'raw_sha256=' "$tmp_dir/manifest-real.txt"
grep -Fq 'raw_bytes=24' "$tmp_dir/manifest-real.txt"
if "$launcher" --operator-only --launch --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-launch.txt" >/dev/null 2>&1; then
  echo "expected stock-hook rejection" >&2
  exit 1
fi

python3 - "$launcher" "$tmp_dir" "$bios_sha" "$disc_sha" <<'PY'
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

launcher, tmp_dir, bios_sha, disc_sha = sys.argv[1:]
tmp = Path(tmp_dir)
fake = tmp / "hanging-mednafen"
pid_file = tmp / "hanging-mednafen.pid"
fake.write_text(
    "#!/bin/sh\n"
    "echo $$ > \"$FAKE_PID_FILE\"\n"
    "# FIRESTAFF_NEXUS_TRACE_OUTPUT is intentionally present for hook admission.\n"
    "trap 'exit 130' INT TERM\n"
    "sleep 30\n",
    encoding="utf-8",
)
fake.chmod(0o755)
env = os.environ.copy()
env["FAKE_PID_FILE"] = str(pid_file)
cmd = [
    launcher, "--operator-only", "--launch", "--mednafen", str(fake),
    "--bios", str(tmp / "bios.bin"), "--bios-sha256", bios_sha,
    "--disc", str(tmp / "disc.cue"), "--disc-sha256", disc_sha,
    "--trace", str(tmp / "trace-interrupted.raw"),
    "--validator", "/usr/bin/true",
    "--manifest", str(tmp / "manifest-interrupted.txt"),
]
proc = subprocess.Popen(cmd, env=env, stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL)
for _ in range(50):
    if pid_file.exists():
        break
    time.sleep(0.02)
if not pid_file.exists():
    proc.kill()
    proc.wait()
    raise SystemExit("hanging fake Mednafen did not start")
child_pid = int(pid_file.read_text().strip())
proc.send_signal(signal.SIGTERM)
proc.wait(timeout=5)
for _ in range(50):
    try:
        os.kill(child_pid, 0)
    except ProcessLookupError:
        break
    stat = subprocess.run(["ps", "-p", str(child_pid), "-o", "stat="],
                          check=False, capture_output=True, text=True).stdout.strip()
    if not stat or stat.startswith("Z"):
        break
    time.sleep(0.02)
else:
    raise SystemExit("capture launcher left Mednafen child alive after TERM")
PY

echo "nexus Saturn raw capture launcher: PASS"
