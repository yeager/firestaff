#!/usr/bin/env sh
set -eu
app=${1:?usage: test_dm2_unsupported_platform_cli.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-unused.zip}
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/build"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/removed-platform.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM

python3 - "$repo_root/data/asset_validator_checksums_m12.json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    records = json.load(stream)["entries"]
if any(record.get("gameId") == "dm2" and
       "PC-9821" in record.get("description", "")
       for record in records):
    raise SystemExit("FAIL: removed DM2 PC-9821 editions remain in the validator catalog")
print("PASS: removed DM2 PC-9821 editions are absent from the validator catalog")
PY

export FIRESTAFF_CONFIG_PATH="$scratch/menu.toml"
for platform in pc98 pc-98 pc9821 pc-9821; do
    for route in direct menu; do
        menu_arg=
        if [ "$route" = menu ]; then menu_arg=--menu; fi
        if output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm2 --platform "$platform" --data-dir "$archive" \
            $menu_arg --boot-probe --duration 0 2>&1); then
            echo "FAIL: unsupported platform $platform was accepted ($route)" >&2
            exit 1
        fi
        case "$output" in
            *'firestaff: --platform must be '*) ;;
            *) printf '%s\n' "$output" >&2; exit 1 ;;
        esac
    done
done
removed_archive=${FIRESTAFF_DM2_REMOVED_ARCHIVE:-}
if [ -n "$removed_archive" ] && [ -f "$removed_archive" ]; then
    check_removed_archive() {
        archive_path=$1
        if output=$(FIRESTAFF_CONFIG_PATH="$scratch/menu.toml" \
            FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            "$app" --game dm2 --data-dir "$archive_path" \
            --boot-probe --duration 1 2>&1); then
            echo "FAIL: removed PC-9821 archive launched without a platform flag: $archive_path" >&2
            exit 1
        fi
        case "$output" in
            *'firestaff: game unavailable for --game: dm2'*) ;;
            *) printf '%s\n' "$output" >&2; exit 1 ;;
        esac
        if output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            "$app" --menu --game dm2 --data-dir "$archive_path" \
            --script 'key:enter,key:enter,key:enter' --duration 1000 2>&1); then
            echo "FAIL: removed PC-9821 archive launched through the menu: $archive_path" >&2
            exit 1
        fi
        case "$output" in
            *'firestaff: launch smoke failed: no launch reached before exit'*) ;;
            *) printf '%s\n' "$output" >&2; exit 1 ;;
        esac
    }

    check_removed_archive "$removed_archive"
    renamed_archive="$scratch/renamed-original.zip"
    ln -s "$removed_archive" "$renamed_archive"
    check_removed_archive "$renamed_archive"
    echo 'PASS: original and renamed PC-9821 archives cannot launch through automatic discovery or the menu'
fi
echo 'PASS: removed PC-9821 platform aliases are rejected before launch'
