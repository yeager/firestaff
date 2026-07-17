#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${FIRESTAFF_NEXUS_DATA_DIR:-}" ]]; then
  exit 77
fi

for level in $(seq -w 0 15); do
  [[ -f "${FIRESTAFF_NEXUS_DATA_DIR}/LEV${level}.DGN" ]] || exit 77
done

exec "$1"
