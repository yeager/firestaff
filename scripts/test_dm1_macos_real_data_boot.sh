#!/usr/bin/env bash
set -euo pipefail

# This is a host-macOS smoke gate.  It deliberately consumes the user's
# authenticated DM1 data and never creates, extracts, or rewrites game files.
# CI has no licensed corpus, so an absent corpus is a skip, not a synthetic
# pass.

binary="${1:?usage: $0 /path/to/firestaff}"
if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "SKIP: DM1 macOS smoke gate requires Darwin"
  exit 0
fi

data_root="${FIRESTAFF_DM1_MAC_DATA:-${FIRESTAFF_DATA:-$HOME/.firestaff/data}}"
if [[ ! -e "$data_root" ]]; then
  echo "SKIP: no DM1 data root at $data_root"
  exit 0
fi

log="$(mktemp -t firestaff-dm1-macos-boot.XXXXXX)"
small_log="$(mktemp -t firestaff-dm1-macos-small-window.XXXXXX)"
fmtowns_log="$(mktemp -t firestaff-dm1-macos-fmtowns.XXXXXX)"
trap 'rm -f "$log" "$small_log" "$fmtowns_log"' EXIT

set +e
SDL_VIDEODRIVER=dummy FIRESTAFF_DATA="$data_root" "$binary" \
  --game dm1 --platform pc --duration 0 --boot-probe >"$log" 2>&1
status=$?
set -e

if [[ "$status" != 0 ]]; then
  if grep -Eqi "(original data required|no game data|missing.*data|could not find)" "$log"; then
    echo "SKIP: authenticated DM1 data is not available under $data_root"
    exit 0
  fi
  cat "$log"
  echo "FAIL: DM1 macOS boot probe exited with status $status" >&2
  exit "$status"
fi

receipt="$(grep 'FIRESTAFF BOOT PROBE READY:' "$log" | tail -n 1 || true)"
if [[ -z "$receipt" ]]; then
  cat "$log"
  echo "FAIL: DM1 macOS boot probe emitted no receipt" >&2
  exit 1
fi

for field in \
  'gameId=dm1' \
  'phase=dm1-runtime' \
  'startupActive=0' \
  'levelLoaded=1' \
  'dm1HoCRealAssetCapture=1'; do
  if [[ "$receipt" != *"$field"* ]]; then
    echo "$receipt"
    echo "FAIL: DM1 macOS receipt is missing $field" >&2
    exit 1
  fi
done

set +e
SDL_VIDEODRIVER=dummy FIRESTAFF_DATA="$data_root" "$binary" \
  --game dm1 --platform pc --duration 0 --width 640 --height 400 \
  --boot-probe >"$small_log" 2>&1
small_status=$?
set -e
if [[ "$small_status" != 0 ]]; then
  cat "$small_log"
  echo "FAIL: DM1 macOS small-window probe exited with status $small_status" >&2
  exit "$small_status"
fi
small_receipt="$(grep 'FIRESTAFF BOOT PROBE READY:' "$small_log" | tail -n 1 || true)"
for field in 'window=640x400' 'windowMode=0'; do
  if [[ "$small_receipt" != *"$field"* ]]; then
    echo "$small_receipt"
    echo "FAIL: DM1 macOS small-window receipt is missing $field" >&2
    exit 1
  fi
done

# Exercise the native FM Towns path only when the user's corpus visibly
# contains that edition.  This keeps CI media-free while making the packed
# ZIP/BIN handoff a real-data gate on development machines.
if find "$data_root" -type f \( \
  -iname '*fm*towns*.zip' -o -iname '*fmtowns*.zip' -o \
  -iname '*fm*towns*.cue' -o -iname '*fmtowns*.cue' \) -print -quit | grep -q . || \
   find "$data_root" -type f -path '*/fmtowns_iso/*' -print -quit | grep -q .; then
  set +e
  SDL_VIDEODRIVER=dummy FIRESTAFF_DATA="$data_root" "$binary" \
    --game dm1 --platform fm-towns --duration 0 --boot-probe >"$fmtowns_log" 2>&1
  fmtowns_status=$?
  set -e
  if [[ "$fmtowns_status" != 0 ]]; then
    cat "$fmtowns_log"
    echo "FAIL: DM1 FM Towns macOS boot probe exited with status $fmtowns_status" >&2
    exit "$fmtowns_status"
  fi
  fmtowns_receipt="$(grep 'FIRESTAFF BOOT PROBE READY:' "$fmtowns_log" | tail -n 1 || true)"
  for field in 'gameId=dm1' 'levelLoaded=1' 'phase=dm1-runtime'; do
    if [[ "$fmtowns_receipt" != *"$field"* ]]; then
      echo "$fmtowns_receipt"
      echo "FAIL: DM1 FM Towns receipt is missing $field" >&2
      exit 1
    fi
  done
fi

echo "PASS: DM1 macOS real-data boot (${data_root})"
