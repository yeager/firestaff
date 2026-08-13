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

if FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE='840:60:60:0x10' \
  "$launcher" --operator-only --mednafen /usr/bin/true \
    --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
    --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
    --trace "$tmp_dir/trace-sequence-reject.raw" --validator "$validator" \
    --manifest "$tmp_dir/manifest-sequence-reject.txt" >/dev/null 2>&1; then
  echo "expected malformed press-sequence rejection" >&2
  exit 1
fi

FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE='840:60:0x10,960:60:0x20' \
"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-sequence.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-sequence.txt" --skip-frames 0 --frame-limit 1200 \
  --require-input-window >/dev/null
grep -Fq 'press_sequence=840:60:0x10,960:60:0x20' "$tmp_dir/manifest-sequence.txt"

"$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-window.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-window.txt" --skip-frames 10000 \
  --frame-limit 560 --press-start-frame 10500 --press-start-length 60 \
  --require-input-window >/dev/null
grep -Fq 'require_input_window=1' "$tmp_dir/manifest-window.txt"
options_fake="$tmp_dir/fake-mednafen-options"
python3 - "$options_fake" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf '%s\\n' \"$@\" > \"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
FIRESTAFF_NEXUS_MEDNAFEN_OPTIONS='-sound 0 -videoip 0' \
"$launcher" --operator-only --launch --mednafen "$options_fake" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-options.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-options.txt" >/dev/null
grep -Fxq -- '-sound' "$tmp_dir/trace-options.raw"
grep -Fxq -- '0' "$tmp_dir/trace-options.raw"
grep -Fxq -- '-videoip' "$tmp_dir/trace-options.raw"
grep -Fq 'mednafen_options=-sound\ 0\ -videoip\ 0' "$tmp_dir/manifest-options.txt"
if "$launcher" --operator-only --mednafen /usr/bin/true \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-window-reject.raw" --validator "$validator" \
  --manifest "$tmp_dir/manifest-window-reject.txt" --skip-frames 10000 \
  --frame-limit 128 --press-start-frame 10500 --press-start-length 60 \
  --require-input-window >/dev/null 2>&1; then
  echo "expected input-window rejection" >&2
  exit 1
fi

vdp2_fake="$tmp_dir/fake-mednafen-vdp2"
python3 - "$vdp2_fake" <<'PY'
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
write_trace="$tmp_dir/vdp1-writes.trace"
writer_code_trace="$tmp_dir/writer-code.trace"
snapshot="$tmp_dir/vdp1-snapshot.raw"
scsp_read_trace="$tmp_dir/scsp-reads.trace"
printf 'write-receipt' > "$write_trace"
printf 'writer-code-receipt' > "$writer_code_trace"
printf 'snapshot-receipt' > "$snapshot"
printf 'scsp-read-receipt' > "$scsp_read_trace"
python3 - "$tmp_dir/fake-mednafen" "$scsp_read_trace" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf 'authenticated-test-trace' > \"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n"
    "printf 'scsp-read-receipt' > \"$FIRESTAFF_NEXUS_TRACE_SCSP_READS\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
python3 - "$tmp_dir/fake-mednafen" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf '%s,%s,%s,%s' \"$FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT\" > "
    "\"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
FIRESTAFF_NEXUS_TRACE_VDP2_REGS="$tmp_dir/vdp2-registers.trace" \
FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC=0x06011860 \
FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN=0x0 \
FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX=0x40000 \
FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT=20000 \
"$launcher" --operator-only --launch --mednafen "$tmp_dir/fake-mednafen" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-vdp2-env.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-vdp2-env.txt" >/dev/null
grep -Fq '0x06011860,0x0,0x40000,20000' "$tmp_dir/trace-vdp2-env.raw"
source_read_fake="$tmp_dir/fake-mednafen-source-read"
python3 - "$source_read_fake" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf '%s,%s,%s,%s,%s,%s' "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX\" "
    "\"$FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT\" > "
    "\"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS="$tmp_dir/source-reads.trace" \
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN=0x0 \
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX=0x80000 \
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN=0x06002fc4 \
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX=0x06002fc6 \
FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT=200000 \
"$launcher" --operator-only --launch --mednafen "$source_read_fake" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-vdp2-source-read-env.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-vdp2-source-read-env.txt" >/dev/null
grep -Fq "$tmp_dir/source-reads.trace,0x0,0x80000,0x06002fc4,0x06002fc6,200000" \
  "$tmp_dir/trace-vdp2-source-read-env.raw"
sh2_memory_fake="$tmp_dir/fake-mednafen-sh2-memory"
python3 - "$sh2_memory_fake" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf '%s,%s,%s,%s,%s' "
    "\"$FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX\" > "
    "\"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n"
    "printf 'memory-snapshot' > \"$FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT\"\n"
    "printf 'ram-read' > \"$FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS\"\n"
    "printf 'ram-write' > \"$FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
sh2_memory_snapshot="$tmp_dir/sh2-memory.snapshot"
sh2_ram_reads="$tmp_dir/sh2-ram-reads.trace"
sh2_ram_writes="$tmp_dir/sh2-ram-writes.trace"
FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT="$sh2_memory_snapshot" \
FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES=',12,13,' \
FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS="$sh2_ram_reads" \
FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES="$sh2_ram_writes" \
FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN=0x06064500 \
FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX=0x060646ff \
FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_LIMIT=4000 \
"$launcher" --operator-only --launch --mednafen "$sh2_memory_fake" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-sh2-memory.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-sh2-memory.txt" >/dev/null
grep -Fq "$sh2_memory_snapshot,,12,13,,$sh2_ram_reads,0x06064500,0x060646ff" \
  "$tmp_dir/trace-sh2-memory.raw"
grep -Fq "FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_sha256=$(shasum -a 256 "$sh2_memory_snapshot" | awk '{print $1}')" \
  "$tmp_dir/manifest-sh2-memory.txt"
grep -Fq "FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS_sha256=$(shasum -a 256 "$sh2_ram_reads" | awk '{print $1}')" \
  "$tmp_dir/manifest-sh2-memory.txt"
grep -Fq "FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES_sha256=$(shasum -a 256 "$sh2_ram_writes" | awk '{print $1}')" \
  "$tmp_dir/manifest-sh2-memory.txt"
dma_fake="$tmp_dir/fake-mednafen-dma"
python3 - "$dma_fake" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text(
    "#!/bin/sh\n"
    "printf '%s,%s,%s,%s,%s,%s' "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX\" "
    "\"$FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT\" > "
    "\"$FIRESTAFF_NEXUS_TRACE_OUTPUT\"\n",
    encoding="utf-8",
)
os.chmod(sys.argv[1], 0o755)
PY
FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES="$tmp_dir/scu-dma.trace" \
FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN=0x06000000 \
FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX=0x08000000 \
FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN=0x05e00000 \
FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX=0x05f00000 \
FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT=200000 \
"$launcher" --operator-only --launch --mednafen "$dma_fake" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-scu-dma-env.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-scu-dma-env.txt" >/dev/null
grep -Fq "$tmp_dir/scu-dma.trace,0x06000000,0x08000000,0x05e00000,0x05f00000,200000" \
  "$tmp_dir/trace-scu-dma-env.raw"
FIRESTAFF_NEXUS_TRACE_VDP1_WRITES="$write_trace" \
FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE="$writer_code_trace" \
FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT="$snapshot" \
FIRESTAFF_NEXUS_TRACE_SCSP_READS="$scsp_read_trace" \
"$launcher" --operator-only --launch --mednafen "$vdp2_fake" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-real.raw" --validator /usr/bin/true \
  --manifest "$tmp_dir/manifest-real.txt" >/dev/null
grep -Fq 'raw_sha256=' "$tmp_dir/manifest-real.txt"
grep -Fq 'raw_bytes=24' "$tmp_dir/manifest-real.txt"
grep -Fq "vdp1_write_trace_sha256=$(shasum -a 256 "$write_trace" | awk '{print $1}')" "$tmp_dir/manifest-real.txt"
grep -Fq "vdp1_writer_code_trace_sha256=$(shasum -a 256 "$writer_code_trace" | awk '{print $1}')" "$tmp_dir/manifest-real.txt"
grep -Fq "vdp1_snapshot_sha256=$(shasum -a 256 "$snapshot" | awk '{print $1}')" "$tmp_dir/manifest-real.txt"
grep -Fq "FIRESTAFF_NEXUS_TRACE_SCSP_READS_sha256=$(shasum -a 256 "$scsp_read_trace" | awk '{print $1}')" "$tmp_dir/manifest-real.txt"
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

set +e
timeout_cmd=(
  "$launcher" --operator-only --launch --mednafen "$tmp_dir/hanging-mednafen"
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha"
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha"
  --trace "$tmp_dir/trace-timeout.raw" --validator /usr/bin/true
  --manifest "$tmp_dir/manifest-timeout.txt" --timeout-seconds 1
)
"${timeout_cmd[@]}" >/dev/null 2>&1
timeout_status=$?
set -e
if [[ "$timeout_status" -eq 0 ]]; then
  echo "expected capture timeout" >&2
  exit 1
fi
if [[ -e "$tmp_dir/trace-timeout.raw" ]]; then
  echo "timed-out capture must not leave a raw witness" >&2
  exit 1
fi
grep -Fq 'capture_exit_status=1' "$tmp_dir/manifest-timeout.txt"

reject_validator="$tmp_dir/reject-validator"
python3 - "$reject_validator" <<'PY'
import os
import sys
from pathlib import Path

Path(sys.argv[1]).write_text("#!/bin/sh\nexit 1\n", encoding="utf-8")
os.chmod(sys.argv[1], 0o755)
PY
set +e
"$launcher" --operator-only --launch --mednafen "$tmp_dir/fake-mednafen" \
  --bios "$tmp_dir/bios.bin" --bios-sha256 "$bios_sha" \
  --disc "$tmp_dir/disc.cue" --disc-sha256 "$disc_sha" \
  --trace "$tmp_dir/trace-rejected.raw" --validator "$reject_validator" \
  --manifest "$tmp_dir/manifest-rejected.txt" >/dev/null 2>&1
rejected_status=$?
set -e
if [[ "$rejected_status" -eq 0 ]]; then
  echo "invalid raw capture must fail even when Mednafen exits zero" >&2
  exit 1
fi
grep -Fq 'capture_exit_status=1' "$tmp_dir/manifest-rejected.txt"

echo "nexus Saturn raw capture launcher: PASS"
