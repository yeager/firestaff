#!/usr/bin/env bash
set -euo pipefail
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
app=${1:?firestaff binary required}
platform=${2:?platform required}
archive=${3:?original archive required}
[[ -x "$app" && -f "$archive" ]] || exit 77
for mode in v1 v21; do
    case "$mode" in v1) expected=0;; v21) expected=2;; esac
    for route in cli menu; do
        args=()
        [[ "$route" != menu ]] || args=(--menu --script enter,enter,enter)
        output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm1 --platform "$platform" --data-dir "$archive" \
            "${args[@]}" --presentation-mode "$mode" --boot-probe \
            --boot-probe-frames 100 --duration 0 2>&1) || {
            printf '%s\n' "$output" >&2; exit 1;
        }
        if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
           ! grep -Fq "presentationMode=$expected " <<<"$output" ||
           ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
           ! grep -Fq 'levelLoaded=1' <<<"$output" ||
           ! grep -Fq "dataDir=$archive" <<<"$output"; then
            printf '%s\n' "$output" >&2
            printf 'FAIL: %s %s %s original-media launch\n' "$platform" "$route" "$mode" >&2
            exit 1
        fi
        printf 'PASS: %s %s %s retains presentation and reaches original dungeon\n' "$platform" "$route" "$mode"
    done
done
