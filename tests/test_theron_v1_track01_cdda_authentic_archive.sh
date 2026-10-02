#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 || ! -x "$1" ]]; then
    printf 'usage: %s <handoff-test-binary>\n' "$0" >&2
    exit 2
fi

case "${OSTYPE:-}" in
    msys*|cygwin*)
        printf 'SKIP: authentic archive CDDA decoding requires POSIX SDL audio\n'
        exit 77
        ;;
esac

us_archive=${FIRESTAFF_THERON_US_7Z:-"$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (USA).7z"}
jp_archive=${FIRESTAFF_THERON_JP_7Z:-"$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan).7z"}
if [[ ! -f "$us_archive" || ! -f "$jp_archive" ]]; then
    printf 'SKIP: authentic US and Japanese Theron 7z archives are required\n'
    exit 77
fi

extractor=$(command -v 7zz || command -v 7z || true)
if [[ -z "$extractor" ]]; then
    printf 'SKIP: 7zz or 7z is required to read authentic disc archives\n'
    exit 77
fi

hash_file() {
    local algorithm=$1
    local path=$2
    if [[ "$algorithm" == sha256 ]]; then
        if command -v sha256sum >/dev/null 2>&1; then
            sha256sum "$path" | cut -d ' ' -f 1
        else
            shasum -a 256 "$path" | cut -d ' ' -f 1
        fi
    elif command -v md5sum >/dev/null 2>&1; then
        md5sum "$path" | cut -d ' ' -f 1
    else
        md5 -q "$path"
    fi
}

temporary_root=$(mktemp -d "$(dirname "$1")/firestaff-theron-cdda.XXXXXX")
trap 'rm -rf "$temporary_root"' EXIT

extract_disc() {
    local archive=$1
    local prefix=$2
    local cue_member=$3
    local track01_member=$4
    local track02_member=$5
    if ! "$extractor" x -y "-o$temporary_root" "$archive" \
        "$cue_member" "$track01_member" "$track02_member" >/dev/null; then
        printf 'FAIL: authentic %s disc members could not be read\n' "$prefix" >&2
        exit 1
    fi
    for member in "$cue_member" "$track01_member" "$track02_member"; do
        if [[ ! -s "$temporary_root/$member" ]]; then
            printf 'FAIL: authentic %s archive is missing %s\n' "$prefix" "$member" >&2
            exit 1
        fi
    done
}

us_cue="Dungeon Master - Theron's Quest (USA).cue"
us_track01="Dungeon Master - Theron's Quest (USA) (Track 01).bin"
us_track02="Dungeon Master - Theron's Quest (USA) (Track 02).bin"
jp_cue="Dungeon Master - Theron's Quest (Japan).cue"
jp_track01="Dungeon Master - Theron's Quest (Japan) (Track 01).bin"
jp_track02="Dungeon Master - Theron's Quest (Japan) (Track 02).bin"
extract_disc "$us_archive" US "$us_cue" "$us_track01" "$us_track02"
extract_disc "$jp_archive" JP "$jp_cue" "$jp_track01" "$jp_track02"

us_md5=$(hash_file md5 "$temporary_root/$us_track02")
jp_md5=$(hash_file md5 "$temporary_root/$jp_track02")
if [[ "$us_md5" != f23601102138f87c33025877767ebf76 ||
      "$jp_md5" != b7afb338ad31be1025b53f9aff12d73a ]]; then
    printf 'FAIL: authentic US/JP Track 02 pair failed its known MD5\n' >&2
    exit 1
fi

us_track01_sha256=$(hash_file sha256 "$temporary_root/$us_track01")
jp_track01_sha256=$(hash_file sha256 "$temporary_root/$jp_track01")
if [[ "$us_track01_sha256" != 8c5603906ff0428f62046e47add6b6f9f9fd0c4bd787ede1fd67cff42bd99e48 ||
      "$jp_track01_sha256" != b30dc3c2355a4213424a2d06e224ef8c0a421ff2c070f5db71c49dd3fd7fed59 ]]; then
    printf 'FAIL: authentic US/JP Track 01 raw audio failed its known SHA-256\n' >&2
    exit 1
fi

SDL_AUDIODRIVER=dummy "$1" --authentic-only \
    "$temporary_root/$us_cue" "$temporary_root/$jp_cue"
