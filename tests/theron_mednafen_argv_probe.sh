#!/bin/sh
set -eu
: "${FIRESTAFF_THERON_FAKE_ARGS:?}"
for argument in "$@"; do
    printf '%s\n' "$argument"
done > "$FIRESTAFF_THERON_FAKE_ARGS"
