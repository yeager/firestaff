#!/usr/bin/env sh
set -eu

app=${1:?usage: test_retroachievements_log_secret_redaction.sh <firestaff-binary>}
sentinel='firestaff-test-secret-never-log-7e02'

if [ ! -x "$app" ]; then
    printf '%s\n' 'SKIP: firestaff binary is not available'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --retroachievements --ra-user release-test-user \
    --ra-token "$sentinel" --duration 0 2>&1)

case "$output" in
    *"$sentinel"*|*'log-7e02'*|*'****7e02'*)
        printf '%s\n' "$output" >&2
        printf '%s\n' 'FAIL: RetroAchievements diagnostic exposed API-token material' >&2
        exit 1
        ;;
esac

case "$output" in
    *'RetroAchievements: ready user=release-test-user credentials=configured hardcore=1'*) ;;
    *)
        printf '%s\n' "$output" >&2
        printf '%s\n' 'FAIL: RetroAchievements readiness diagnostic was not emitted' >&2
        exit 1
        ;;
esac

printf '%s\n' 'PASS: RetroAchievements diagnostics never expose API-token material'
