#!/usr/bin/env sh
set -eu
app=${1:?usage: test_dm2_unsupported_platform_cli.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-unused.zip}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/build"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/removed-platform.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM
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
    if output=$(FIRESTAFF_CONFIG_PATH="$scratch/menu.toml" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --game dm2 --data-dir "$removed_archive" \
        --boot-probe --duration 1 2>&1); then
        echo 'FAIL: removed PC-9821 archive launched without a platform flag' >&2
        exit 1
    fi
    case "$output" in
        *'firestaff: game unavailable for --game: dm2'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
    if output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --menu --game dm2 --data-dir "$removed_archive" \
        --script 'key:enter,key:enter,key:enter' --duration 1000 2>&1); then
        echo 'FAIL: removed PC-9821 archive launched through the menu' >&2
        exit 1
    fi
    case "$output" in
        *'firestaff: launch smoke failed: no launch reached before exit'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
    echo 'PASS: original PC-9821 archive cannot launch through automatic discovery or the menu'
fi
echo 'PASS: removed PC-9821 platform aliases are rejected before launch'
