#!/usr/bin/env bash
set -euo pipefail

# Local pre-resume verifier. It validates only the opaque capture envelope and
# identities; it never reads a payload window, decodes pixels, or writes data.
if [[ $# -eq 0 ]]; then
    printf '%s\n' 'SKIP: --verify <capture-plan> <capture-artifact> <mednafen-trace> is required'
    exit 0
fi
if [[ $# -ne 4 || $1 != --verify ]]; then
    printf '%s\n' "usage: $0 --verify <capture-plan> <capture-artifact> <mednafen-trace>" >&2
    exit 2
fi
plan=$2 artifact=$3 trace=$4
for path in "$plan" "$artifact" "$trace"; do
    [[ -f "$path" && ! -L "$path" ]] || { printf '%s\n' 'REJECTED: plan, artifact, and trace must be direct regular files' >&2; exit 1; }
done
md5_file() {
    if command -v md5 >/dev/null 2>&1; then md5 -q "$1"
    elif command -v md5sum >/dev/null 2>&1; then md5sum "$1" | awk '{print $1}'
    else return 1
    fi
}
mapfile -t plan_rows < "$plan"
[[ ${#plan_rows[@]} -eq 18 && ${plan_rows[0]} == THERON_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_V1 &&
   ${plan_rows[1]} == route=dungeon_handoff && ${plan_rows[15]} == payload_policy=opaque_only &&
   ${plan_rows[16]} == decoder_policy=forbidden && ${plan_rows[17]} == render_policy=no_draw ]] || {
    printf '%s\n' 'REJECTED: capture-plan envelope is malformed' >&2; exit 1;
}
track02_md5=$(printf '%s\n' "${plan_rows[@]}" | sed -n 's/^track02_md5=//p')
plan_trace_md5=$(printf '%s\n' "${plan_rows[@]}" | sed -n 's/^source_trace_md5=//p')
plan_identity=$(printf '%s\n' "${plan_rows[@]}" | sed -n 's/^capture_target_plan_fnv1a=//p')
artifact_path=$(printf '%s\n' "${plan_rows[@]}" | sed -n 's/^capture_artifact_path=//p')
[[ $track02_md5 =~ ^[0-9a-f]{32}$ && $plan_trace_md5 =~ ^[0-9a-f]{32}$ && $plan_identity =~ ^[1-9a-f][0-9a-f]*$ && $artifact_path == "$artifact" ]] || {
    printf '%s\n' 'REJECTED: artifact path, Track 02 identity, or source trace identity does not match the local plan' >&2; exit 1;
}
mapfile -t rows < "$artifact"
[[ ${#rows[@]} -eq 8 && ${rows[0]} == THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1 ]] || {
    printf '%s\n' 'REJECTED: capture artifact envelope is malformed' >&2; exit 1;
}
[[ ${rows[1]} == "track02_md5=$track02_md5" ]] || {
    printf '%s\n' 'REJECTED: artifact Track 02 MD5 drifts from plan' >&2; exit 1;
}
trace_md5=$(md5_file "$trace") || { printf '%s\n' 'REJECTED: md5 utility is required' >&2; exit 1; }
[[ $trace_md5 == "$plan_trace_md5" && ${rows[2]} == "mednafen_trace_md5=$trace_md5" && ${rows[3]} == "capture_target_plan_fnv1a=$plan_identity" && ${rows[4]} == campaign_route=2 ]] || {
    printf '%s\n' 'REJECTED: artifact trace hash or dungeon route drifts from plan' >&2; exit 1;
}
for route in 0 1 2; do
    row=${rows[$((route + 5))]}
    [[ $row == route=$route\ * && $row == *'loader_checksum='* &&
       $row == *'palette_identity='* && $row == *'bitmap_identity='* &&
       $row == *'destination_record='* && $row == *'destination_identity='* ]] || {
        printf '%s\n' 'REJECTED: artifact descriptor/palette/bitmap envelope is incomplete' >&2; exit 1;
    }
    [[ $row =~ palette_identity=[1-9a-f][0-9a-f]* && $row =~ bitmap_identity=[1-9a-f][0-9a-f]* &&
       $row =~ destination_identity=[1-9a-f][0-9a-f]* ]] || {
        printf '%s\n' 'REJECTED: artifact identity is zero or malformed' >&2; exit 1;
    }
done
printf '%s\n' 'VERIFIED: opaque Track 02 dungeon-handoff artifact envelope is ready for existing M12/M11 resume admission'
