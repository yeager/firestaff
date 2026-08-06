#!/usr/bin/env bash
set -euo pipefail

archive=${1:-}
if [[ -z "$archive" || ! -f "$archive" ]]; then
    printf 'FAIL: firestaff_theron archive path is unavailable\n' >&2
    exit 1
fi

if ar t "$archive" | grep -Eq '^theron_v1_track02_creature\\.c\\.o$'; then
    printf 'FAIL: legacy DMWeb creature table is linked into production\n' >&2
    exit 1
fi

printf 'PASS: legacy DMWeb creature table is absent from firestaff_theron\n'
