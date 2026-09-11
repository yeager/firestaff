#!/usr/bin/env bash
# Pass 462: Linux/N2 original DM1 PC34 TITLE raw screenshot capture.
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
STAGE="${DM1_ORIGINAL_STAGE_DIR:-}"
# Capture artifacts are local, disposable verification material.  Keep the
# default in the repository scratch area rather than embedding a machine- or
# user-specific location in a reusable script.
ROOT="${OUT_ROOT:-$REPO/.codex-scratch/pass462-title-end-capture-parity}"
STAMP="${PASS462_STAMP:-$(date -u +%Y%m%dT%H%M%SZ)}"
OUT_DIR="${OUT_DIR:-$ROOT/$STAMP-title}"
DOSBOX="${DOSBOX:-/usr/bin/dosbox}"
SHOT_COUNT="${SHOT_COUNT:-24}"
WAIT_BEFORE_SHOTS_MS="${WAIT_BEFORE_SHOTS_MS:-2500}"
WAIT_BETWEEN_SHOTS_MS="${WAIT_BETWEEN_SHOTS_MS:-180}"
WAIT_AFTER_SELECTOR_MS="${WAIT_AFTER_SELECTOR_MS:-700}"
mkdir -p "$OUT_DIR"
if [ -z "$STAGE" ] || [ ! -f "$STAGE/DM.EXE" ]; then
    echo "missing DM.EXE stage; set DM1_ORIGINAL_STAGE_DIR to a local original PC 3.4 staging directory" >&2
    exit 3
fi
cat > "$OUT_DIR/dosbox-title-pass462.conf" <<CONF
[sdl]
fullscreen=false
output=surface
[dosbox]
# Current DOSBox-X uses this [dosbox] setting for its exit confirmation.
# The matching command-line override below protects against a conflicting
# user configuration when DOSBOX points at dosbox-x.
quit warning=false
machine=vgaonly
memsize=4
[cpu]
core=normal
cycles=3000
[mixer]
nosound=true
[autoexec]
mount c "$STAGE"
c:
DM VGA
CONF
cat > "$OUT_DIR/run-title-xvfb.sh" <<RUN
#!/usr/bin/env bash
set -euo pipefail
cd "$OUT_DIR"
if [ "\$(basename "$DOSBOX")" = "dosbox-x" ]; then
  "$DOSBOX" -exit -set "dosbox quit warning=false" -conf "$OUT_DIR/dosbox-title-pass462.conf" >"$OUT_DIR/dosbox-title-pass462.log" 2>&1 &
else
  "$DOSBOX" -conf "$OUT_DIR/dosbox-title-pass462.conf" >"$OUT_DIR/dosbox-title-pass462.log" 2>&1 &
fi
pid=\$!
cleanup(){ kill \$pid >/dev/null 2>&1 || true; wait \$pid >/dev/null 2>&1 || true; }
trap cleanup EXIT
python3 - <<PY
import time; time.sleep($WAIT_BEFORE_SHOTS_MS/1000)
PY
win=\$(xdotool search --sync --onlyvisible --name DOSBox | head -1)
# Original PC34 text selectors: graphics, sound, input.  Keep this explicit so
# the captured frames are post-selector TITLE/runtime frames, not prompt text.
for i in 1 2 3; do
  xdotool key --window "\$win" 1 Return || true
  python3 - <<PY
import time; time.sleep(0.25)
PY
done
python3 - <<PY
import time; time.sleep($WAIT_AFTER_SELECTOR_MS/1000)
PY
for i in \$(seq 1 "$SHOT_COUNT"); do
  xdotool key --window "\$win" ctrl+F5 || true
  python3 - <<PY
import time; time.sleep($WAIT_BETWEEN_SHOTS_MS/1000)
PY
done
RUN
chmod +x "$OUT_DIR/run-title-xvfb.sh"
xvfb-run -a "$OUT_DIR/run-title-xvfb.sh" >"$OUT_DIR/xvfb-title-pass462.log" 2>&1 || true
python3 - "$OUT_DIR" <<'PY'
from pathlib import Path
from PIL import Image
import csv, hashlib, sys, datetime
out = Path(sys.argv[1])
rows=[]
for i,p in enumerate(sorted((out/'capture').glob('*.png')) + sorted(out.glob('*.png')), 1):
    digest=hashlib.sha256(p.read_bytes()).hexdigest()
    try:
        im=Image.open(p); w,h=im.size
    except Exception:
        w=h=0
    st=p.stat()
    rows.append({'index':i,'path':str(p),'sha256':digest,'size_bytes':st.st_size,'mtime_utc':datetime.datetime.fromtimestamp(st.st_mtime, datetime.timezone.utc).isoformat(),'width':w,'height':h})
with (out/'title_capture_manifest.tsv').open('w', newline='') as f:
    wr=csv.DictWriter(f, fieldnames=['index','path','sha256','size_bytes','mtime_utc','width','height'], delimiter='\t')
    wr.writeheader(); wr.writerows(rows)
print(out)
print('captures', len(rows))
PY
