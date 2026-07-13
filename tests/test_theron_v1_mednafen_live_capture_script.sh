#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/capture_theron_mednafen_live_trace.sh

if [[ ! -x "$script" ]]; then
    printf 'FAIL: live Mednafen capture script is not executable\n' >&2
    exit 1
fi
bash -n "$script"
if ! grep -Fq -- '-pce.arcadecard 0' "$script"; then
    printf 'FAIL: capture script must disable unrelated Arcade Card emulation\n' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_IRQ2_INPUT_TRACE="$input_trace"' "$script"; then
    printf 'FAIL: capture script must retain a raw controller input receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_MEDNAFEN_HOME must name an existing Mednafen configuration directory' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_SDL_VIDEODRIVER' "$script"; then
    printf 'FAIL: capture script must gate an explicit GUI input configuration\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_CAPTURE_HOST_KEY currently supports only return, i, or select' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY requires a non-dummy SDL video driver' "$script" ||
   ! grep -Fq 'set targetProcess to first application process whose name is "mednafen"' "$script" ||
   ! grep -Fq 'key down {return}' "$script" ||
   ! grep -Fq 'key up {return}' "$script" ||
   ! grep -Fq 'key down {tab}' "$script" ||
   ! grep -Fq 'key code 85' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_REPEATS must be a positive integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_DELAY must be a non-negative integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLD must be a positive integer' "$script" ||
   ! grep -Fq 'requested host key was not observed by Mednafen SDL dispatch' "$script"; then
    printf 'FAIL: capture script must keep the opt-in macOS Return focus/input gate\n' >&2
    exit 1
fi
if ! grep -Fq 'dynamic CPU receipts lack a bounded authentic raw-sector span' "$script" ||
   ! grep -Fq 'span_offset=0 span_bytes=32 span_fnv1a=' "$script"; then
    printf 'FAIL: capture script must gate dynamic reads on an authentic raw-sector span\n' >&2
    exit 1
fi
if ! grep -Fq 'raw sector span lacks prior input, CDIRQ, and non-System-Card PCECD caller receipts' "$script" ||
   ! grep -Fq 'pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3}' "$script"; then
    printf 'FAIL: capture script must gate raw sectors on observed non-System-Card caller evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'host_key_events=%s' "$script" ||
   ! grep -Fq 'System Card wait; host_keys=%s input=%s irq=%s non_system_card_pcecd=%s' "$script" ||
   ! grep -Fq 'dynamic receipts absent; host_keys=%s input=%s irq=%s non_system_card_pcecd=%s' "$script"; then
    printf 'FAIL: capture script must report missing transition evidence counts\n' >&2
    exit 1
fi
if ! grep -Fq 'trace_count()' "$script" ||
   ! grep -Fq 'local count' "$script" ||
   ! grep -Fq '"${count:-0}"' "$script"; then
    printf 'FAIL: capture script must emit numeric zero counts when a trace file is absent\n' >&2
    exit 1
fi
if ! grep -Fq 'source=authentic-mednafen-transition-receipt' "$script" ||
   ! grep -Fq 'transition=missing' "$script" ||
   ! grep -Fq 'transition=observed' "$script"; then
    printf 'FAIL: capture script must publish an observed-or-missing transition receipt\n' >&2
    exit 1
fi

output=$(env -u MEDNAFEN_BIN -u THERON_US_CUE -u THERON_SYSTEM_CARD \
    -u THERON_LIVE_TRACE_OUTPUT "$script")
if [[ "$output" != 'SKIP: MEDNAFEN_BIN, THERON_US_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required' ]]; then
    printf 'FAIL: capture script did not reject unstaged live inputs\n' >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

printf 'PASS: live capture script requires explicit authentic inputs\n'
