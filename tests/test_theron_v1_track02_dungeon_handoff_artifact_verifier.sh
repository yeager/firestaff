#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script="$repo/scripts/verify_theron_track02_dungeon_handoff_artifact.sh"
[[ -x "$script" ]] || { echo 'FAIL: artifact verifier is not executable' >&2; exit 1; }
bash -n "$script"
for fact in 'THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1' 'capture_target_plan_fnv1a=' 'palette_identity=' 'bitmap_identity=' 'destination_identity=' 'never reads a payload window'; do
    grep -Fq "$fact" "$script" || { echo "FAIL: verifier lacks $fact" >&2; exit 1; }
done
[[ $("$script") == SKIP:* ]] || { echo 'FAIL: missing artifact inputs must skip safely' >&2; exit 1; }
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
printf 'bad\n' > "$tmp/plan"; : > "$tmp/artifact"; : > "$tmp/trace"
if "$script" --verify "$tmp/plan" "$tmp/artifact" "$tmp/trace" >/dev/null 2>&1; then
    echo 'FAIL: malformed local artifact was accepted' >&2; exit 1
fi
echo 'theron Track 02 dungeon handoff artifact verifier: PASS'
