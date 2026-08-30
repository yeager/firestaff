#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

app=${1:?usage: test_dm1_v1_fmtowns_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
expected_md5=c10c512f63461ebe79b5ac365115b61b
expected_edm_md5=c27e7b984df9753912c3375dc121919f

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 FM Towns archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq 'platformHandoff=fmtowns-tmenu-edm' <<<"$output" &&
    grep -Fq 'fmtownsProgram=EDM.EXP' <<<"$output" &&
    grep -Fq "fmtownsProgramMd5=$expected_edm_md5" <<<"$output" &&
    grep -Fq 'fmtownsMenuSelectsProgram=1' <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform fm-towns --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform fm-towns --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

# The public launcher is also fully pointer-navigable.  Use the native
# 1920x1080 card canvas explicitly so scripted physical coordinates do not
# depend on a CI host's default window size.  The waits model separate input
# frames; no keyboard event selects the game, platform, or presentation.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform fm-towns \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform fm-towns --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'platformHandoff=fmtowns-tmenu-edm' <<<"$gameplay_output" ||
   ! grep -Fq 'fmtownsProgram=EDM.EXP' <<<"$gameplay_output" ||
   ! grep -Fq "fmtownsProgramMd5=$expected_edm_md5" <<<"$gameplay_output" ||
   ! grep -Fq 'fmtownsMenuSelectsProgram=1' <<<"$gameplay_output" ||
   ! grep -Fq 'dm1FmtownsCddaPlaying=1' <<<"$gameplay_output" ||
   ! grep -Fq 'dm1FmtownsCddaTrack=2' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 FM Towns up input did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic DM1 FM Towns ZIP reaches CLI, menu, TMENU/EDM handoff, and native movement runtime in memory'
