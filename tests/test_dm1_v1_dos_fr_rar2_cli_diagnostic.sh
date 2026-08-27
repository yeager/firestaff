#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm1_v1_dos_fr_rar2_cli_diagnostic.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_DOS_FR_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM1 French DOS RAR2 package is not staged'
    exit 77
fi

if output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --data-dir "$archive" --duration 0 2>&1); then
    printf '%s\n' "$output" >&2
    echo 'FAIL: RAR2 package unexpectedly launched' >&2
    exit 1
fi

case "$output" in
    *'RAR 2.0 NOT SUPPORTED: FRENCH DOS PACKAGE'*) ;;
    *)
        printf '%s\n' "$output" >&2
        echo 'FAIL: French DOS package was not diagnosed as unsupported RAR2' >&2
        exit 1
        ;;
esac

if printf '%s\n' "$output" | grep -Fq 'MISSING: GAME DATA'; then
    printf '%s\n' "$output" >&2
    echo 'FAIL: French DOS RAR2 package was misdiagnosed as missing data' >&2
    exit 1
fi
echo 'PASS: authentic DM1 French DOS RAR2 package reports its native limitation'
