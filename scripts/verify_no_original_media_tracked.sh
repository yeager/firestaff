#!/usr/bin/env bash
set -euo pipefail

# Original BIOS/firmware and user-supplied game media are local inputs only.
# Keep this check independent of the working tree so a forced add or a future
# .gitignore change cannot publish proprietary payloads to GitHub.
offenders=$(git ls-files | LC_ALL=C grep -Ei '\.(pce|rom|bin|cue|iso|mdf|mds|chd|bios|firmware)$' || true)

if [[ -n "$offenders" ]]; then
    printf '%s\n' 'FAIL: original BIOS/firmware or game-media payloads are tracked by git:' >&2
    printf '  %s\n' "$offenders" >&2
    exit 1
fi

printf '%s\n' 'PASS: no original BIOS/firmware or game-media payloads are tracked by git.'
