#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_startup_scan_intro_capture_dm1_real_media.sh <firestaff>}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

if [[ ! -x "$app" ]]; then
    echo "SKIP: Firestaff executable is unavailable: $app"
    exit 77
fi

python3 - "$app" "${FIRESTAFF_LOCALE_DIR:-$script_dir/../po}" <<'PY'
import hashlib
import os
import pathlib
import re
import shutil
import struct
import subprocess
import sys
import tempfile

app, locale_dir = sys.argv[1:]
cases = [
    ("DM1 PC 3.4", "FIRESTAFF_DM1_PC34_ARCHIVE",
     pathlib.Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip", "dm1"),
    ("CSB FM Towns", "FIRESTAFF_CSB_FMTOWNS_ARCHIVE",
     pathlib.Path.home() / ".firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip", "csb"),
    ("DM2 FM Towns", "FIRESTAFF_DM2_FMTOWNS_ARCHIVE",
     pathlib.Path.home() / ".firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip", "dm2"),
]

def sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.digest()

archives = []
for label, variable, fallback, game in cases:
    archive_name = os.environ.get(variable, str(fallback))
    archive = pathlib.Path(archive_name).expanduser()
    if not archive.is_file():
        print(f"SKIP: authentic {label} archive is required: {archive}")
        raise SystemExit(77)
    archives.append((label, archive.resolve(), game))

root = pathlib.Path(tempfile.mkdtemp(prefix="startup-scan-intro-real-media-"))
try:
    for label, archive, game in archives:
        case_root = root / game
        data_root = case_root / "data"
        game_root = data_root / game
        game_root.mkdir(parents=True)
        (game_root / archive.name).symlink_to(archive)
        capture_dir = case_root / "capture"
        before = sha256_file(archive)
        env = os.environ.copy()
        env.update({
            "FIRESTAFF_AUTOTEST": "1",
            "FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN": "1",
            "FIRESTAFF_AUTOTEST_STARTUP_SCAN_CAPTURE_DIR": str(capture_dir),
            "FIRESTAFF_AUTOTEST_STARTUP_SCAN_CAPTURE_AFTER_PROGRESS": "1",
            "FIRESTAFF_LOCALE_DIR": locale_dir,
            "SDL_VIDEODRIVER": "dummy",
            "SDL_AUDIODRIVER": "dummy",
        })
        try:
            result = subprocess.run(
                [app, "--lang", "sv", "--data-dir", str(data_root),
                 "--debug", "--verbose"],
                env=env, capture_output=True, text=True, timeout=60)
        except subprocess.TimeoutExpired as error:
            for output in (error.stdout, error.stderr):
                if output:
                    print(output.decode(errors="replace") if isinstance(output, bytes)
                          else output, file=sys.stderr)
            raise SystemExit(f"FAIL: {label} startup scan capture did not terminate")

        trace = result.stdout + result.stderr
        captures = list(capture_dir.glob("*.bmp"))
        if result.returncode != 0:
            raise SystemExit(f"FAIL: {label} app exited {result.returncode}\n{trace}")
        if ('startup game=none mode=menu' not in trace or
                'startup-scan-intro language-index=1 '
                'label="SÖKER EFTER SPELDATA' not in trace):
            raise SystemExit(
                f"FAIL: {label} default no-game intro did not show its Swedish scan label\n{trace}")
        if (len(captures) != 1 or
                "autotest startup scan capture progress=" not in trace or
                "autotest injected window-close during startup intro asset scan" not in trace):
            raise SystemExit(f"FAIL: no real {label} startup scan frame was captured\n{trace}")
        captured_progress = re.search(
            r"autotest startup scan capture progress=(\d+)%", trace)
        if not captured_progress or not 0 < int(captured_progress.group(1)) < 100:
            raise SystemExit(
                f"FAIL: {label} scan capture did not show in-progress work\n{trace}")

        with captures[0].open("rb") as image:
            header = image.read(54)
        if len(header) != 54 or header[:2] != b"BM":
            raise SystemExit(f"FAIL: {label} scan capture is not a BMP")
        width, height = struct.unpack_from("<ii", header, 18)
        bits_per_pixel = struct.unpack_from("<H", header, 28)[0]
        if (width, height, bits_per_pixel) != (480, -270, 24):
            raise SystemExit(
                f"FAIL: unexpected {label} startup frame dimensions/format: "
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
                f"FAIL: {label} centered progress track pixel changed: {track_pixel_bgr!r}")
        fill_pixel_offset = 54 + 235 * row_stride + 52 * 3
        with captures[0].open("rb") as image:
            image.seek(fill_pixel_offset)
            fill_pixel_bgr = image.read(3)
        if fill_pixel_bgr != bytes((207, 202, 200)):
            raise SystemExit(
                f"FAIL: {label} nonzero scan progress was not drawn: {fill_pixel_bgr!r}")
        if sha256_file(archive) != before:
            raise SystemExit(f"FAIL: original {label} archive changed during scan")
        print(
            f"PASS: authentic {label} scan drew nonzero progress in its "
            "480x270 intro frame and closed cleanly")
finally:
    shutil.rmtree(root, ignore_errors=True)
PY
