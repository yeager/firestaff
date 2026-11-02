#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_startup_scan_intro_capture_dm1_real_media.sh <firestaff>}
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo "SKIP: authentic DM1 PC 3.4 archive is required: $archive"
    exit 77
fi

python3 - "$app" "$archive" "${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" <<'PY'
import hashlib
import os
import pathlib
import shutil
import struct
import subprocess
import sys
import tempfile

app, archive_name, locale_dir = sys.argv[1:]
archive = pathlib.Path(archive_name).resolve()
root = pathlib.Path(tempfile.mkdtemp(prefix="startup-scan-intro-dm1-"))
try:
    data_root = root / "data"
    dm1_root = data_root / "dm1"
    dm1_root.mkdir(parents=True)
    (dm1_root / archive.name).symlink_to(archive)
    capture_dir = root / "capture"
    before = hashlib.sha256(archive.read_bytes()).digest()
    env = os.environ.copy()
    env.update({
        "FIRESTAFF_AUTOTEST": "1",
        "FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN": "1",
        "FIRESTAFF_AUTOTEST_STARTUP_SCAN_CAPTURE_DIR": str(capture_dir),
        "FIRESTAFF_LOCALE_DIR": locale_dir,
        "SDL_VIDEODRIVER": "dummy",
        "SDL_AUDIODRIVER": "dummy",
    })
    try:
        result = subprocess.run(
            [app, "--lang", "sv", "--data-dir", str(data_root),
             "--debug", "--verbose"],
            env=env, capture_output=True, text=True, timeout=25)
    except subprocess.TimeoutExpired as error:
        for output in (error.stdout, error.stderr):
            if output:
                print(output.decode(errors="replace") if isinstance(output, bytes)
                      else output, file=sys.stderr)
        raise SystemExit("FAIL: startup scan screenshot did not terminate")

    trace = result.stdout + result.stderr
    captures = list(capture_dir.glob("*.bmp"))
    if result.returncode != 0:
        raise SystemExit(f"FAIL: app exited {result.returncode}\n{trace}")
    if (len(captures) != 1 or
            "autotest startup scan capture progress=" not in trace or
            "autotest injected window-close during startup intro asset scan" not in trace):
        raise SystemExit(f"FAIL: no real startup scan frame was captured\n{trace}")

    with captures[0].open("rb") as image:
        header = image.read(54)
    if len(header) != 54 or header[:2] != b"BM":
        raise SystemExit("FAIL: startup scan capture is not a BMP")
    width, height = struct.unpack_from("<ii", header, 18)
    bits_per_pixel = struct.unpack_from("<H", header, 28)[0]
    if (width, height, bits_per_pixel) != (480, -270, 24):
        raise SystemExit(
            f"FAIL: unexpected startup frame dimensions/format: "
            f"{width}x{height} {bits_per_pixel}bpp")
    # The 480x270 startup intro centers a 376x4 track at x=52, y=235.
    # The rightmost track pixel remains unfilled for every captured scan below
    # 100%, so this checks the visible track rather than a synthetic frame.
    row_stride = (width * 3 + 3) & ~3
    pixel_offset = 54 + 235 * row_stride + 427 * 3
    with captures[0].open("rb") as image:
        image.seek(pixel_offset)
        track_pixel_bgr = image.read(3)
    if track_pixel_bgr != bytes((66, 63, 62)):
        raise SystemExit(
            f"FAIL: centered progress track pixel changed: {track_pixel_bgr!r}")
    if hashlib.sha256(archive.read_bytes()).digest() != before:
        raise SystemExit("FAIL: original DM1 archive changed during scan")
    print("PASS: authentic DM1 scan produced a 480x270 intro frame and closed cleanly")
finally:
    shutil.rmtree(root, ignore_errors=True)
PY
