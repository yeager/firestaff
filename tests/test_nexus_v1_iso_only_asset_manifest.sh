#!/usr/bin/env bash
set -euo pipefail

probe="${1:?asset-manifest probe path is required}"
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

grep -Eq '^[[:space:]]*TRACK[[:space:]]+01[[:space:]]+MODE1/(2048|2352)' "$cue" || exit 77
mapfile -t media_names < <(sed -nE 's/^[[:space:]]*FILE[[:space:]]+"([^"]+)".*/\1/p' "$cue")
[[ ${#media_names[@]} -gt 0 ]] || exit 77
for media_name in "${media_names[@]}"; do
    [[ "$media_name" != */* && "$media_name" != *\\* ]] || exit 77
    [[ -f "$(dirname "$cue")/$media_name" ]] || exit 77
done

runtime_root=${FIRESTAFF_TEST_RUNTIME_DIR:-"$(pwd)/test-runtime"}
mkdir -p "$runtime_root"
tmpdir="$(mktemp -d "$runtime_root/firestaff-nexus-manifest-cue-bin-only.XXXXXX")"
trap 'find "$tmpdir" -mindepth 1 -maxdepth 1 -type l -delete; rmdir "$tmpdir"' EXIT
ln -s "$cue" "$tmpdir/$(basename "$cue")"
for media_name in "${media_names[@]}"; do
    ln -s "$(dirname "$cue")/$media_name" "$tmpdir/$media_name"
done

echo "Nexus CUE/BIN-only manifest root: $tmpdir"
exec "$probe" "$tmpdir"
