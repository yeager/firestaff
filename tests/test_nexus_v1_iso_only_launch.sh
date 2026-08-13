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

iso_name="$(sed -nE 's/^[[:space:]]*FILE[[:space:]]+"([^"]+\.iso)".*/\1/ip' "$cue" | head -n 1)"
[[ -n "$iso_name" ]] || exit 77
iso="$(dirname "$cue")/$iso_name"
[[ -f "$iso" ]] || exit 77

tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-nexus-iso-only.XXXXXX")"
trap 'rm -rf "$tmpdir"' EXIT
ln -s "$cue" "$tmpdir/$(basename "$cue")"
ln -s "$iso" "$tmpdir/$(basename "$iso")"

echo "Nexus ISO-only root: $tmpdir"
exec "$probe" "$tmpdir"
