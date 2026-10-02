#!/bin/sh
set -eu

test_bin=${1:?test executable is required}
archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
loose=${FIRESTAFF_CSB_FMTOWNS_LOOSE_ROOT:-"$HOME/.firestaff/data/csb/fmtowns_iso"}
if [ ! -f "$archive" ] || [ ! -d "$loose/CDATA" ] ||
   [ ! -d "$loose/CJDATA" ]; then
    echo "SKIP: original CSB FM Towns CD and loose data tree are required"
    exit 77
fi

mixed=$(mktemp -d "${PWD}/.firestaff-csb-menu-media.XXXXXX")
trap 'rm -rf "$mixed"' EXIT
mkdir "$mixed/csb"
mkdir "$mixed/home"
ln -s "$archive" "$mixed/csb/$(basename "$archive")"
ln -s "$loose" "$mixed/csb/fmtowns_iso"
HOME="$mixed/home" XDG_CONFIG_HOME="$mixed/home" \
    "$test_bin" "$mixed" "$(basename "$archive")"
