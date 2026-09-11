#!/usr/bin/env bash
# Capture original DM1/CSB FM Towns frames using Tsugaru's own framebuffer
# command, never a desktop/compositor screenshot.
#
# Development evidence only.  Firestaff does not invoke Tsugaru, require a
# BIOS, or extract any game archive at runtime.  The caller supplies original
# media; staged payloads and resulting copyrighted captures remain untracked
# beneath .codex-scratch.

set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:---prepare}"
game="${FMTOWNS_GAME:-dm1}"
archive="${FMTOWNS_ARCHIVE:-}"
rom_dir="${FMTOWNS_ROM_DIR:-}"
tsugaru="${FMTOWNS_TSUGARU:-Tsugaru_CUI}"
out="${FMTOWNS_CAPTURE_OUT:-$repo/.codex-scratch/fmtowns-original-startup-capture}"
stage="${FMTOWNS_STAGE_DIR:-$repo/.codex-scratch/fmtowns-original-media-stage}"
timeline="${FMTOWNS_CAPTURE_TIMELINE:-}"
towns_type="${FMTOWNS_TYPE:-MX}"

usage() {
    cat <<'EOF'
Usage: capture_fmtowns_original_startup.sh [--prepare|--run]

Required for --run:
  FMTOWNS_GAME=dm1|csb
  FMTOWNS_ARCHIVE=/path/to/original-fm-towns.zip
  FMTOWNS_ROM_DIR=/path/to/extracted-fm-towns-rom-directory
  FMTOWNS_CAPTURE_TIMELINE='seconds:label [seconds:label ...]'

Optional:
  FMTOWNS_TSUGARU=/path/to/Tsugaru_CUI    (default: Tsugaru_CUI on PATH)
  FMTOWNS_CAPTURE_OUT=/safe/output/path   (default: repository .codex-scratch)
  FMTOWNS_STAGE_DIR=/safe/staging/path    (default: repository .codex-scratch)
  FMTOWNS_TYPE=MX                          (FM Towns machine type)

The ZIP is staged only for this development-time emulator session because
Tsugaru requires a seekable CUE plus BIN or IMG track image.  The archive is never modified, the
stage must live beneath .codex-scratch, and every result receives hashes for
the archive, selected CUE/track image and ROM files.  Images are produced by Tsugaru's
`SS` command from its emulated framebuffer.  Host desktop captures are not
accepted.  A successful capture is original-emulator evidence only; it is not
a Firestaff pixel-parity claim.
EOF
}

case "$mode" in
    --prepare) usage; exit 0 ;;
    --run) ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac

if [[ "$game" != "dm1" && "$game" != "csb" ]]; then
    echo "ERROR: FMTOWNS_GAME must be dm1 or csb" >&2
    exit 2
fi
if [[ -z "$archive" || ! -f "$archive" || "${archive,,}" != *.zip ]]; then
    echo "ERROR: FMTOWNS_ARCHIVE must name the original FM Towns ZIP" >&2
    exit 3
fi
if [[ -z "$rom_dir" || ! -d "$rom_dir" ]]; then
    echo "ERROR: FMTOWNS_ROM_DIR must name an extracted, caller-supplied ROM directory" >&2
    exit 3
fi
if [[ -z "$timeline" || ! "$timeline" =~ ^[0-9]+:[A-Za-z0-9_-]+(\ [0-9]+:[A-Za-z0-9_-]+)*$ ]]; then
    echo "ERROR: FMTOWNS_CAPTURE_TIMELINE must use seconds:label entries separated by spaces" >&2
    exit 3
fi
for required in "$tsugaru" 7zz sha256sum python3; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "ERROR: required capture tool is unavailable: $required" >&2
        exit 4
    }
done
python3 -c 'from PIL import Image' >/dev/null 2>&1 || {
    echo "ERROR: Pillow is required to validate Tsugaru framebuffer PNGs" >&2
    exit 4
}

scratch_root="$repo/.codex-scratch"
mkdir -p "$scratch_root"
out="$(mkdir -p "$out" && cd "$out" && pwd)"
stage="$(mkdir -p "$stage" && cd "$stage" && pwd)"
case "$out" in "$scratch_root"/*) ;; *) echo "ERROR: capture output must remain beneath .codex-scratch" >&2; exit 3;; esac
case "$stage" in "$scratch_root"/*) ;; *) echo "ERROR: media stage must remain beneath .codex-scratch" >&2; exit 3;; esac
if find "$out" -mindepth 1 -maxdepth 1 -print -quit | grep -q .; then
    echo "ERROR: capture output must be empty; use a fresh directory" >&2
    exit 3
fi

# The exact CUE/track-image pair is selected from real media, not guessed from a
# filename.  Staging outside the source tree prevents game payloads entering
# a commit by accident.
if find "$stage" -mindepth 1 -maxdepth 1 -print -quit | grep -q .; then
    echo "ERROR: media stage must be empty; use a fresh .codex-scratch directory" >&2
    exit 3
fi
7zz x -y "-o$stage" "$archive" >/dev/null
mapfile -d '' cue_files < <(find "$stage" -type f -iname '*.cue' -print0)
mapfile -d '' track_files < <(find "$stage" -type f \( -iname '*.bin' -o -iname '*.img' \) -print0)
if [[ "${#cue_files[@]}" -ne 1 || "${#track_files[@]}" -ne 1 ]]; then
    echo "ERROR: original archive must contain exactly one CUE and one BIN or IMG track image" >&2
    exit 5
fi
cue="${cue_files[0]}"
track_image="${track_files[0]}"

# Tsugaru's Linux CUI resolves ROM names with exact upper-case filenames,
# whereas verified user ROM sets commonly use lower-case names.  Build a
# private, symlink-only case-normalized view; never rename or copy firmware.
rom_stage="$out/rom-case-normalized"
mkdir -p "$rom_stage"
for expected_rom in FMT_SYS.ROM FMT_DOS.ROM FMT_FNT.ROM FMT_F20.ROM FMT_DIC.ROM; do
    mapfile -d '' matches < <(find "$rom_dir" -maxdepth 1 -type f -iname "$expected_rom" -print0)
    if [[ "${#matches[@]}" -ne 1 ]]; then
        echo "ERROR: ROM directory must contain exactly one case-insensitive $expected_rom" >&2
        exit 5
    fi
    ln -s "${matches[0]}" "$rom_stage/$expected_rom"
done

# Record each command before execution.  Tsugaru's CUI command interpreter
# runs concurrently with the VM, so timed `SS` requests sample the emulated
# framebuffer directly and do not depend on SDL/X11 focus or cursor state.
previous=0
index=0
command_file="$out/tsugaru-capture-commands.txt"
{
    for entry in $timeline; do
        second="${entry%%:*}"
        label="${entry#*:}"
        if (( second < previous )); then
            echo "ERROR: capture timestamps must be nondecreasing" >&2
            exit 5
        fi
        index=$((index + 1))
        printf 'sleep %s\n' "$((second - previous))"
        printf 'SS "%s/startup-%02d-%ss-%s.png"\n' "$out" "$index" "$second" "$label"
        previous="$second"
    done
    printf 'QUIT\n'
} >"$command_file"

run_commands() {
    while IFS= read -r command; do
        case "$command" in
            sleep\ *) sleep "${command#sleep }" ;;
            *) printf '%s\n' "$command" ;;
        esac
    done <"$command_file"
}

set +e
# Tsugaru's first positional argument is its ROM directory.  It is not an
# option: passing a made-up -ROMDIR flag would silently turn that directory
# into an invalid option and leave a false failed-capture trail.
run_commands | "$tsugaru" "$rom_stage" -CD "$cue" -BOOTKEY CD \
    -TOWNSTYPE "$towns_type" -FORCEQUITONPOFF >"$out/tsugaru.log" 2>&1
tsugaru_status=${PIPESTATUS[1]}
set -e
if [[ "$tsugaru_status" -ne 0 ]]; then
    echo "ERROR: Tsugaru exited with status $tsugaru_status; see tsugaru.log" >&2
    exit 6
fi

expected="$(wc -w <<<"$timeline" | tr -d ' ')"
actual="$(find "$out" -maxdepth 1 -type f -name 'startup-*.png' | wc -l | tr -d ' ')"
if [[ "$actual" -ne "$expected" ]]; then
    echo "ERROR: Tsugaru produced $actual/$expected framebuffer capture(s)" >&2
    exit 7
fi

python3 - "$out" "$expected" <<'PY'
from pathlib import Path
from PIL import Image
import sys

out = Path(sys.argv[1])
expected = int(sys.argv[2])
frames = sorted(out.glob("startup-*.png"))
if len(frames) != expected:
    raise SystemExit("ERROR: frame list changed during validation")
for frame in frames:
    with Image.open(frame) as image:
        if image.width < 1 or image.height < 1:
            raise SystemExit(f"ERROR: empty framebuffer image: {frame.name}")
PY

{
    printf 'schema=firestaff.fmtowns.original.capture.v1\n'
    printf 'scope=original Tsugaru framebuffer capture; no Firestaff parity claim\n'
    printf 'game=%s\n' "$game"
    printf 'capture_backend=tsugaru-cui-SS\n'
    printf 'cursor_policy=host_cursor_excluded_by_emulated_framebuffer_capture\n'
    printf 'towns_type=%s\n' "$towns_type"
    printf 'archive_sha256=%s\n' "$(sha256sum "$archive" | awk '{print $1}')"
    printf 'cue_sha256=%s\n' "$(sha256sum "$cue" | awk '{print $1}')"
    printf 'track_image_sha256=%s\n' "$(sha256sum "$track_image" | awk '{print $1}')"
    printf 'tsugaru_sha256=%s\n' "$(sha256sum "$(command -v "$tsugaru")" | awk '{print $1}')"
    while IFS= read -r -d '' rom; do
        printf 'rom_sha256=%s\n' "$(sha256sum "$rom" | awk '{print $1}')"
    done < <(find "$rom_dir" -maxdepth 1 -type f -name '*.rom' -print0 | sort -z)
    for frame in "$out"/startup-*.png; do
        printf 'frame_sha256=%s\n' "$(sha256sum "$frame" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

echo "PASS: wrote $actual original $game FM Towns framebuffer capture(s)"
