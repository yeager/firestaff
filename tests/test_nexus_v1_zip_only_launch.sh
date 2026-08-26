#!/usr/bin/env bash
# Verify native Nexus launch from an original ZIP container without extracting
# any game member to disk.  The staging root contains only a symlink to the
# operator-owned archive.
set -euo pipefail

probe="${1:?launch-smoke probe path is required}"
work_root="${2:?CTest build directory is required}"
data_root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"

archive=""
for candidate in "$data_root"/*.zip "$data_root"/*.ZIP; do
    [[ -f "$candidate" ]] || continue
    if unzip -Z1 "$candidate" 2>/dev/null | grep -qi \
            '^.*Dungeon Master Nexus.*(Track 1).*\.bin$'; then
        archive="$candidate"
        break
    fi
done
[[ -n "$archive" ]] || exit 77

stage="$(mktemp -d "$work_root/firestaff-nexus-zip-only.XXXXXX")"
cleanup() { rm -rf "$stage"; }
trap cleanup EXIT
ln -s "$archive" "$stage/$(basename "$archive")"

echo "Nexus ZIP-only root: $stage"
"$probe" "$stage"
