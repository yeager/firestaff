#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
cue=${FIRESTAFF_THERON_JP_CUE:-}
test_temp_dir=${FIRESTAFF_TEST_TEMP_DIR:-"$(dirname "$app")"}
extracted_media_root=
if [[ -z "$cue" ]]; then
    for candidate in \
        "$HOME/.firestaff/data/theron/TQJP.cue" \
        "$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1).cue"; do
        if [[ -f "$candidate" ]]; then
            cue=$candidate
            break
        fi
    done
fi
expected_md5_raw=b7afb338ad31be1025b53f9aff12d73a
expected_md5_iso=397039af02d50d15c70b74088eb8a1cb

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$cue" ]]; then
    archive_root="$HOME/.firestaff/data/theron"
    extractor=$(command -v 7zz || command -v 7z || true)
    archive=
    archive_layout=
    for candidate in \
        "$archive_root/Dungeon Master - Theron's Quest (Japan).7z" \
        "$archive_root/Dungeon Master - Theron's Quest (Japan) (1).7z"; do
        if [[ -f "$candidate" ]]; then
            archive=$candidate
            break
        fi
    done
    if [[ -n "$archive" && -n "$extractor" ]]; then
        if [[ "$archive" == *"(1).7z" ]]; then
            archive_layout="Dungeon Master - Theron's Quest (Japan)"
        fi
        if [[ ! -d "$test_temp_dir" ]]; then
            printf 'FAIL: test temporary directory is unavailable: %s\n' "$test_temp_dir" >&2
            exit 1
        fi
        extracted_media_root=$(mktemp -d "$test_temp_dir/firestaff-theron-jp-cue-media.XXXXXX")
        if [[ -n "$archive_layout" ]]; then
            if ! "$extractor" x "$archive" "-o$extracted_media_root" \
                "$archive_layout/Dungeon Master - Theron's Quest (Japan).cue" \
                "$archive_layout/Dungeon Master - Theron's Quest (Japan) (Track 02).bin" >/dev/null; then
                rm -rf "$extracted_media_root"
                printf 'FAIL: authentic Theron JP archive could not provide its CUE and Track 02\n' >&2
                exit 1
            fi
            media_root="$extracted_media_root/$archive_layout"
        else
            if ! "$extractor" e "$archive" "-o$extracted_media_root" \
                "Dungeon Master - Theron's Quest (Japan).cue" \
                "Dungeon Master - Theron's Quest (Japan) (Track 02).bin" >/dev/null; then
                rm -rf "$extracted_media_root"
                printf 'FAIL: authentic Theron JP archive could not provide its CUE and Track 02\n' >&2
                exit 1
            fi
            media_root=$extracted_media_root
        fi
        cue="$media_root/Dungeon Master - Theron's Quest (Japan).cue"
    fi
fi
if [[ ! -f "$cue" ]]; then
    printf 'SKIP: authentic Theron JP CUE is not staged\n'
    exit 77
fi

if [[ -n "$extracted_media_root" ]]; then
    trap 'rm -rf "$extracted_media_root"' EXIT
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$cue" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,action,up,right,down,left,up,up' \
    --boot-probe-expect-phase theron-runtime \
    --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 2,3,0 \
    --boot-probe-expect-champions 1 \
    --boot-probe-expect-startup-active 0 \
    --duration 0 2>&1) || {
    printf '%s\n' "$output" >&2
    exit 1
}

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output" ||
   ! grep -Fq 'sourceKind=4 sourceId=theron ' <<<"$output" ||
   ! grep -Eq "assetMd5=($expected_md5_raw|$expected_md5_iso)" <<<"$output" ||
   ! grep -Fq 'phase=theron-runtime' <<<"$output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$output" ||
   ! grep -Fq 'party=2,3,0 champions=1' <<<"$output" ||
   ! grep -Fq 'startupActive=0' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron JP CUE did not reach the native Track 02 Akutuba runtime route' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron JP CUE reaches Akutuba runtime and accepts six native movement inputs'
