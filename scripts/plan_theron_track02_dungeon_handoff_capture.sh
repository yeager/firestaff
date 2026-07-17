#!/usr/bin/env bash
set -euo pipefail

# Operator-local plan for one original Track 02 dungeon-handoff capture. This
# script never copies media, emits trace rows, or creates bitmap/palette data.
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
live_capture="$script_dir/capture_theron_mednafen_live_trace.sh"
mednafen_bin=${MEDNAFEN_BIN:-}
cue=${THERON_US_CUE:-}
system_card=${THERON_SYSTEM_CARD:-}
capture_root=${THERON_DUNGEON_CAPTURE_ROOT:-}
layout_epoch=${THERON_DUNGEON_CAPTURE_LAYOUT_EPOCH:-}
replay_record=${THERON_DUNGEON_CAPTURE_REPLAY_RECORD:-}
replay_sector=${THERON_DUNGEON_CAPTURE_REPLAY_SECTOR:-}
plan_identity=${THERON_DUNGEON_CAPTURE_PLAN_FNV1A:-}
execute=${THERON_DUNGEON_CAPTURE_EXECUTE:-0}

if [[ -z "$mednafen_bin" || -z "$cue" || -z "$system_card" ||
      -z "$capture_root" || -z "$layout_epoch" || -z "$replay_record" ||
      -z "$replay_sector" || -z "$plan_identity" ]]; then
    printf '%s\n' 'SKIP: MEDNAFEN_BIN, THERON_US_CUE, THERON_SYSTEM_CARD, THERON_DUNGEON_CAPTURE_ROOT, layout epoch, replay record/sector, and plan FNV1A are required'
    exit 0
fi
if [[ ! -x "$mednafen_bin" || ! -f "$cue" || ! -f "$system_card" ||
      ! -d "$capture_root" || -L "$capture_root" || ! -w "$capture_root" ]]; then
    printf '%s\n' 'FAIL: operator Mednafen/CUE/System Card/capture root is unavailable or unsafe' >&2
    exit 1
fi
if [[ ! "$layout_epoch" =~ ^[1-9][0-9]*$ ||
      ! "$replay_record" =~ ^[0-9a-fA-F]+$ ||
      ! "$replay_sector" =~ ^[0-9a-fA-F]+$ ||
      ! "$plan_identity" =~ ^[0-9a-fA-F]+$ ]]; then
    printf '%s\n' 'FAIL: layout epoch, replay record/sector, and plan FNV1A must be nonzero hexadecimal/integer identities' >&2
    exit 1
fi
if (( 16#$replay_record == 0 || 16#$replay_sector == 0 ||
       16#$replay_record != 16#$replay_sector )); then
    printf '%s\n' 'FAIL: replay final record and raw sector must be the same nonzero observed Track 02 coordinate' >&2
    exit 1
fi

md5_file() {
    if command -v md5 >/dev/null 2>&1; then md5 -q "$1"
    elif command -v md5sum >/dev/null 2>&1; then md5sum "$1" | awk '{print $1}'
    else return 1
    fi
}

track02_member=$(awk '
    /^FILE "/ { line=$0; sub(/^FILE "/, "", line); sub(/" BINARY[[:space:]]*$/, "", line); file=line; next }
    /^[[:space:]]*TRACK[[:space:]]+02[[:space:]]+MODE1\/2352[[:space:]]*$/ { print file; exit }
' "$cue")
if [[ -z "$track02_member" || "$track02_member" == */* || "$track02_member" == *\\* ]]; then
    printf '%s\n' 'FAIL: CUE has no safe Track 02 MODE1/2352 payload' >&2
    exit 1
fi
track02_path="$(dirname -- "$cue")/$track02_member"
if [[ ! -f "$track02_path" ]]; then
    printf '%s\n' 'FAIL: CUE Track 02 payload is unavailable' >&2
    exit 1
fi
track02_md5=$(md5_file "$track02_path") || {
    printf '%s\n' 'FAIL: md5 or md5sum is required for media attestation' >&2
    exit 1
}
system_card_md5=$(md5_file "$system_card") || {
    printf '%s\n' 'FAIL: could not hash System Card for media attestation' >&2
    exit 1
}
if [[ "$system_card_md5" != ff1a674273fe3540ccef576376407d1d ]]; then
    printf '%s\n' 'FAIL: System Card 3.0 MD5 mismatch' >&2
    exit 1
fi
case "$track02_md5" in
    b7afb338ad31be1025b53f9aff12d73a|f23601102138f87c33025877767ebf76) ;;
    *) printf '%s\n' 'FAIL: CUE Track 02 is not an authenticated Theron raw BIN' >&2; exit 1 ;;
esac

cue_abs=$(cd -- "$(dirname -- "$cue")" && pwd -P)/$(basename -- "$cue")
track02_abs=$(cd -- "$(dirname -- "$track02_path")" && pwd -P)/$(basename -- "$track02_path")
system_card_abs=$(cd -- "$(dirname -- "$system_card")" && pwd -P)/$(basename -- "$system_card")
root_abs=$(cd -- "$capture_root" && pwd -P)
trace_path="$root_abs/theron-track02-dungeon-handoff.mednafen.trace"
descriptor_path="$root_abs/theron-track02-dungeon-handoff.descriptor.manifest"
artifact_path="$root_abs/theron-track02-dungeon-handoff.capture-artifact"
plan_path="$root_abs/theron-track02-dungeon-handoff.capture-plan"

if [[ -e "$plan_path" || -e "$trace_path" || -e "$descriptor_path" || -e "$artifact_path" ]]; then
    printf '%s\n' 'FAIL: operator capture output path already exists; refusing to overwrite evidence' >&2
    exit 1
fi

umask 077
{
    printf '%s\n' 'THERON_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_V1'
    printf 'route=dungeon_handoff\n'
    printf 'cue_path=%s\ntrack02_payload_path=%s\ntrack02_md5=%s\n' "$cue_abs" "$track02_abs" "$track02_md5"
    printf 'system_card_path=%s\nsystem_card_md5=%s\n' "$system_card_abs" "$system_card_md5"
    printf 'layout_epoch=%s\nreplay_final_record=%s\nreplay_final_raw_sector=%s\n' "$layout_epoch" "$replay_record" "$replay_sector"
    printf 'capture_target_plan_fnv1a=%s\n' "$plan_identity"
    printf 'mednafen_trace_path=%s\ndescriptor_manifest_path=%s\ncapture_artifact_path=%s\n' "$trace_path" "$descriptor_path" "$artifact_path"
    printf '%s\n' 'payload_policy=opaque_only'
    printf '%s\n' 'decoder_policy=forbidden'
    printf '%s\n' 'render_policy=no_draw'
} > "$plan_path"

if [[ "$execute" != 0 && "$execute" != 1 ]]; then
    rm -f "$plan_path"
    printf '%s\n' 'FAIL: THERON_DUNGEON_CAPTURE_EXECUTE must be 0 or 1' >&2
    exit 1
fi
if [[ "$execute" == 1 ]]; then
    if [[ ! -x "$live_capture" ]]; then
        rm -f "$plan_path"
        printf '%s\n' 'FAIL: existing Mednafen live capture route is unavailable' >&2
        exit 1
    fi
    MEDNAFEN_BIN="$mednafen_bin" THERON_US_CUE="$cue" THERON_SYSTEM_CARD="$system_card" \
        THERON_LIVE_TRACE_OUTPUT="$trace_path" "$live_capture"
    if [[ ! -s "$trace_path" ]]; then
        rm -f "$plan_path"
        printf '%s\n' 'FAIL: Mednafen did not produce an observed trace' >&2
        exit 1
    fi
    printf '%s\n' 'CAPTURED: original Mednafen trace is local; descriptor and bitmap/palette artifacts require existing strict conversion/import validation'
else
    printf '%s\n' 'READY: local original-capture plan written; Mednafen was not launched and no trace or payload artifact was created'
fi
