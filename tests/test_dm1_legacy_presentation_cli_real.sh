#!/usr/bin/env bash
set -euo pipefail
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
app=${1:?firestaff binary required}
platform=${2:?platform required}
archive=${3:?original archive required}
[[ -x "$app" && -f "$archive" ]] || exit 77
is_fmtowns_japanese=0
for edition_arg in "${@:4}"; do
    [[ "$edition_arg" != --dm1-fmtowns-ja ]] || is_fmtowns_japanese=1
done
shift 3
runtime_probe="${app}.dm1-presentation-${platform}-$$.json"
trap 'rm -f "$runtime_probe"' EXIT

run_probe() {
    local mode=$1
    local route=$2
    shift 2
    if [[ "$route" == menu ]]; then
        local script='enter,enter,enter'
        if [[ "$platform" == fm-towns ]]; then
            # FM Towns is selected through the pointer-driven 1920x1080 card
            # flow; keyboard Enter does not activate its platform cards.
            script='wait20,click:700:262,wait20,click:410:405,wait20,click:450:405,wait20'
            FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
            FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
                --width 1920 --height 1080 --game dm1 --platform "$platform" \
                "$@" --data-dir "$archive" --menu --script "$script" \
                --presentation-mode v1 --duration 3000 2>&1
        else
            FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
            FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
                --game dm1 --platform "$platform" --data-dir "$archive" \
                "$@" --menu --script "$script" --presentation-mode v1 \
                --duration 3000 2>&1
        fi
    else
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm1 --platform "$platform" --data-dir "$archive" \
            "$@" --presentation-mode "$mode" --boot-probe \
            --boot-probe-frames 100 --duration 0 2>&1
    fi
}

for mode in v1 v20 v21; do
    case "$mode" in v1) expected=0;; v20) expected=1;; v21) expected=2;; esac
    output=$(run_probe "$mode" cli "$@") || {
        printf '%s\n' "$output" >&2; exit 1;
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq "presentationMode=$expected " <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output" ||
       ! grep -Fq "dataDir=$archive" <<<"$output"; then
        printf '%s\n' "$output" >&2
        printf 'FAIL: %s CLI %s original-media launch\n' "$platform" "$mode" >&2
        exit 1
    fi
    if (( is_fmtowns_japanese )); then
        if ! grep -Fq 'assetMd5=edf47d7da5de8184604d6d80477ef01f ' <<<"$output" ||
           ! grep -Fq 'fmtownsProgram=JDM.EXP ' <<<"$output" ||
           ! grep -Fq 'fmtownsProgramMd5=acfbcfa5d65032a4bcabc8d5ea062dcc ' <<<"$output"; then
            printf '%s\n' "$output" >&2
            printf '%s\n' 'FAIL: requested Japanese edition did not bind original JDATA/JDM' >&2
            exit 1
        fi
    fi
done

# The M12 card flow deliberately offers Original and Upscaled quick choices;
# its explicit settings flow covers the remaining presentation mode. Do not
# claim that the direct-launch --presentation-mode override survives a menu
# selection, since making that CLI flag sticky would hide the visible card
# choice. Exercise one complete card-menu route with its real input method.
output=$(run_probe v1 menu "$@") || {
    printf '%s\n' "$output" >&2; exit 1;
}
python3 - "$runtime_probe" "$platform" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

startup = probe.get("startup", {})
presentation = probe.get("presentation", {})
output = probe.get("outputSize", {})
if (probe.get("sourceId") != "dm1" or
        startup.get("phase") != "dm1-runtime" or
        startup.get("active") != 1 or startup.get("levelLoaded") != 1 or
        presentation.get("mode") != 0 or
        output.get("valid") != 1 or
        min(output.get("windowWidth", 0), output.get("windowHeight", 0),
            output.get("drawableWidth", 0), output.get("drawableHeight", 0)) <= 0):
    raise SystemExit(
        f"FAIL: {sys.argv[2]} M12 menu Original did not reach the "
        f"requested presentation mode and active runtime: {probe}")
PY
printf 'PASS: %s M12 menu Original card launches authenticated original media\n' "$platform"
