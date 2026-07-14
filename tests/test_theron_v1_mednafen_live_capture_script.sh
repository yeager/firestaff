#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/capture_theron_mednafen_live_trace.sh
quartz_helper=$repo/scripts/send_theron_macos_quartz_keypair.swift
runtime_verifier=$repo/scripts/verify_theron_mednafen_sdl2_runtime.sh

if [[ ! -x "$script" ]]; then
    printf 'FAIL: live Mednafen capture script is not executable\n' >&2
    exit 1
fi
bash -n "$script"
if [[ ! -x "$runtime_verifier" ]] ||
   ! grep -Fq 'sdl2-compat' "$runtime_verifier" ||
   ! grep -Fq 'use a real SDL2 runtime for authentic Quartz/SDL capture' "$runtime_verifier"; then
    printf 'FAIL: live capture build must reject the SDL2-compat event bridge\n' >&2
    exit 1
fi
if [[ ! -f "$quartz_helper" ]] ||
   ! grep -Fq 'CGEvent(keyboardEventSource: source' "$quartz_helper" ||
   ! grep -Fq 'CGPreflightPostEventAccess()' "$quartz_helper" ||
   ! grep -Fq 'down.postToPid(targetPid)' "$quartz_helper" ||
   ! grep -Fq 'up.postToPid(targetPid)' "$quartz_helper" ||
   ! grep -Fq 'quartz_keypair=posted_to_pid' "$quartz_helper"; then
    printf 'FAIL: capture script must retain the checked-in Quartz keypair helper\n' >&2
    exit 1
fi
if swift "$quartz_helper" 36 1 0 >/dev/null 2>&1; then
    printf 'FAIL: Quartz helper accepted a non-positive target PID\n' >&2
    exit 1
fi
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
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY requires THERON_MEDNAFEN_HOME with an explicit PCE input mapping' "$script" ||
   ! grep -Fq 'set targetProcess to first application process whose unix id is $mednafen_ui_pid' "$script" ||
   ! grep -Fq 'cliclick "c:${host_focus_x},${host_focus_y}"' "$script" ||
   ! grep -Fq 'mednafen_ui_pid=$(pgrep -f "$mednafen_bin" | tail -n 1 || true)' "$script" ||
   ! grep -Fq 'return) host_key_code=36' "$script" ||
   ! grep -Fq 'select) host_key_code=48' "$script" ||
   ! grep -Fq 'i) host_key_code=34' "$script" ||
   ! grep -Fq 'swift "$quartz_keypair_script" "$host_key_code" "$host_key_hold" "$mednafen_ui_pid"' "$script" ||
   ! grep -Fq 'Quartz helper did not attest PID-targeted key delivery' "$script" ||
   ! grep -Fq 'host input requires Swift and the checked-in Quartz keypair helper' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_REPEATS must be a positive integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_DELAY must be a non-negative integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLD must be a positive integer' "$script" ||
   ! grep -Fq 'requested host key was not observed by Mednafen SDL dispatch' "$script"; then
    printf 'FAIL: capture script must keep the opt-in macOS Return focus/input gate\n' >&2
    exit 1
fi
if ! grep -Fq 'dynamic CPU receipts lack a complete authentic raw-sector receipt' "$script" ||
   ! grep -Fq 'sector_fnv1a=' "$script" ||
   ! grep -Fq 'span_offset=0 span_bytes=32 span_fnv1a=' "$script"; then
    printf 'FAIL: capture script must gate dynamic reads on a complete authentic raw-sector receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'dynamic_cd_read_destination_span pc=4093 destination=3800 bytes=32 fnv1a=' "$script"; then
    printf 'FAIL: capture script must require the dynamic CD_READ destination-RAM receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'raw sector span lacks prior input, CDIRQ, and non-System-Card PCECD caller receipts' "$script" ||
   ! grep -Fq 'pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3}' "$script"; then
    printf 'FAIL: capture script must gate raw sectors on observed non-System-Card caller evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'host_key_events=%s' "$script" ||
   ! grep -Fq 'host_sdl_events=%s' "$script" ||
   ! grep -Fq 'host_sdl_event_types=%s' "$script" ||
   ! grep -Fq 'host_window_events=%s' "$script" ||
   ! grep -Fq 'host_focus_state_events=%s' "$script" ||
   ! grep -Fq 'host_input_target_pid=%s' "$script" ||
   ! grep -Fq 'host_input_focus=screen_click:%s,%s' "$script" ||
   ! grep -Fq 'host_input_delivery=quartz_pid_key_down_up' "$script" ||
   ! grep -Fq 'host_input_delivery_attempts=%s' "$script" ||
   ! grep -Fq 'trace_input_order_receipt()' "$script" ||
   ! grep -Fq 'pce_input_transactions_after_first_host' "$script" ||
   ! grep -Fq 'host_input_order=after_last_observed_pce_input_poll' "$script" ||
   ! grep -Fq 'System Card wait; host_keys=%s input=%s input_after_first_host=%s irq=%s non_system_card_pcecd=%s' "$script" ||
   ! grep -Fq 'loader reached authentic raw sectors but dynamic CPU receipts are absent' "$script" ||
   ! grep -Fq 'dynamic receipts absent; host_keys=%s input=%s irq=%s non_system_card_pcecd=%s' "$script"; then
    printf 'FAIL: capture script must report missing transition evidence counts\n' >&2
    exit 1
fi
if ! grep -Fq 'trace_count()' "$script" ||
   ! grep -Fq 'trace_event_types()' "$script" ||
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
if ! grep -Fq 'stage2_system_card_receipt="${trace}.stage2-system-card"' "$script" ||
   ! grep -Fq 'verify_theron_stage2_system_card_call_trace.sh' "$script" ||
   ! grep -Fq 'Absence is expected for captures that do not reach this exact stage.' "$script"; then
    printf 'FAIL: capture script must preserve a separate fail-closed stage-two loader receipt\n' >&2
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
