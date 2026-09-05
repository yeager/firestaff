#!/usr/bin/env bash
set -euo pipefail

probe="${1:?launch-smoke probe path is required}"
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"

cue=""
for candidate in "$root"/*.cue "$root"/*.CUE; do
    [[ -f "$candidate" ]] || continue
    if grep -qi 'Dungeon Master Nexus' "$candidate"; then
        cue="$candidate"
        break
    fi
done
[[ -n "$cue" ]] || exit 77

# The retail disc is a MODE1/2352 data BIN followed by CDDA BIN tracks.
# Identify the data track from CUE semantics, never from a misleading .iso
# filename convention.
grep -Eq '^[[:space:]]*TRACK[[:space:]]+01[[:space:]]+MODE1/(2048|2352)' "$cue" || exit 77
mapfile -t media_names < <(sed -nE 's/^[[:space:]]*FILE[[:space:]]+"([^"]+)".*/\1/p' "$cue")
[[ ${#media_names[@]} -gt 0 ]] || exit 77
for media_name in "${media_names[@]}"; do
    [[ "$media_name" != */* && "$media_name" != *\\* ]] || exit 77
    [[ -f "$(dirname "$cue")/$media_name" ]] || exit 77
done

runtime_root=${FIRESTAFF_TEST_RUNTIME_DIR:-"$(pwd)/test-runtime"}
mkdir -p "$runtime_root"
tmpdir="$(mktemp -d "$runtime_root/firestaff-nexus-iso-only.XXXXXX")"
trap 'rm -rf "$tmpdir"' EXIT
ln -s "$cue" "$tmpdir/$(basename "$cue")"
for media_name in "${media_names[@]}"; do
    ln -s "$(dirname "$cue")/$media_name" "$tmpdir/$media_name"
done

echo "Nexus CUE/BIN-only root: $tmpdir"
exec "$probe" "$tmpdir"
