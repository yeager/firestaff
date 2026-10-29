#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/capture_theron_mednafen_live_trace.sh
quartz_helper=$repo/scripts/send_theron_macos_quartz_keypair.swift
quartz_grab_helper=$repo/scripts/send_theron_macos_quartz_chord.swift
runtime_verifier=$repo/scripts/verify_theron_mednafen_sdl2_runtime.sh
build_script=$repo/scripts/build_mednafen_theron_irq2_trace.sh
huc6280_23ad_probe=$repo/scripts/apply_mednafen_theron_huc6280_23ad_trace.py
pointer_instrumenter=$repo/scripts/instrument_mednafen_theron_mouse_trace.py
bram_helper=$repo/scripts/prepare_theron_authentic_bram.sh
adpcm_context_patch=$repo/scripts/mednafen_1.32.1_theron_adpcm_fifo_ram_trace_context.patch
adpcm_playback_patch=$repo/scripts/mednafen_1.32.1_theron_adpcm_playback_trace.patch
cdda_command_patch=$repo/scripts/mednafen_1.32.1_theron_cdda_command_trace.patch
state_autoload_patch=$repo/scripts/mednafen_1.32.1_theron_state_autoload.patch
post_dungeon_patch_file=$repo/scripts/mednafen_1.32.1_theron_post_dungeon_ordinal_research.patch
save_manager_dump_patch=$repo/scripts/mednafen_1.32.1_theron_save_manager_code_dump.patch
title_wait_patch=$repo/scripts/mednafen_1.32.1_theron_title_wait_input_research.patch
drator_menu_patch=$repo/scripts/mednafen_1.32.1_theron_drator_menu_route_research.patch
irq2_patch=$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch
consumer_read_patch=$repo/scripts/mednafen_1.32.1_theron_main_ram_consumer_read_trace.patch
ram_provenance_patch=$repo/scripts/mednafen_1.32.1_theron_ram_provenance_trace.patch
loader_write_v3_patch=$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_write_trace_v3.patch
input_grab_patch=$repo/scripts/mednafen_1.32.1_theron_input_grab_trace.patch
later_raw_receipt=$repo/scripts/verify_theron_later_raw_sector_media_receipt.pl
x11_keymap=$repo/scripts/theron_x11_keymap.sh
scripted_input_consumption_verifier=$repo/scripts/verify_theron_scripted_input_consumption.sh
movement_data_instrumenter=$repo/scripts/instrument_mednafen_theron_movement_data_read_trace.py
pce_fast_movement_context_instrumenter=$repo/scripts/instrument_mednafen_theron_pce_fast_movement_context_trace.py

if [[ ! -x "$script" ]]; then
    printf 'FAIL: live Mednafen capture script is not executable\n' >&2
    exit 1
fi
if [[ ! -x "$scripted_input_consumption_verifier" ]]; then
    printf '%s\n' 'FAIL: scripted-input consumption verifier is not executable' >&2
    exit 1
fi
if ! grep -Fq 'wait_for_host_key_scancode_presses' "$script" ||
   ! grep -Fq 'host_key_sequence_scancodes' "$script" ||
   ! grep -Fq 'did not observe host key down' "$script"; then
    printf '%s\n' 'FAIL: live capture must require an observed SDL key-down for every requested host key' >&2
    exit 1
fi
host_key_gate_dir=$(mktemp -d "$repo/build/theron-host-key-gate.XXXXXX")
trap 'rm -rf -- "$host_key_gate_dir"' EXIT
cat >"$host_key_gate_dir/input.trace" <<'THERON_HOST_KEY_TRACE'
host_key_event type=up scancode=91 repeat=0
host_key_event type=down scancode=91 repeat=0
host_key_event type=up scancode=91 repeat=0
host_key_event type=down scancode=91 repeat=0
THERON_HOST_KEY_TRACE
sed -n '/^wait_for_host_key_scancode_presses() {/,/^}/p' "$script" \
    >"$host_key_gate_dir/press-gate.sh"
if ! bash -c 'source "$1"; wait_for_host_key_scancode_presses "$2" 91 1 1' _ \
    "$host_key_gate_dir/press-gate.sh" "$host_key_gate_dir/input.trace" 2>/dev/null; then
    printf '%s\n' 'FAIL: one complete host key press was not recognized' >&2
    exit 1
fi
if bash -c 'source "$1"; wait_for_host_key_scancode_presses "$2" 91 2 1' _ \
    "$host_key_gate_dir/press-gate.sh" "$host_key_gate_dir/input.trace" 2>/dev/null; then
    printf '%s\n' 'FAIL: an unmatched key-down was counted as a complete host key press' >&2
    exit 1
fi
mkdir -p "$repo/build"
python3 "$repo/tests/test_theron_pce_fast_consumer_trace_patch.py"
cd_limit_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_CD_DATA_READ_TRACE_LIMIT=65535 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: CD data-read trace accepted a limit below its bounded minimum' >&2
    exit 1
}
if [[ "$cd_limit_output" != *'THERON_CAPTURE_PCE_FAST_CD_DATA_READ_TRACE_LIMIT must be an integer from 65536 through 1048576'* ]]; then
    printf '%s\n' 'FAIL: invalid CD data-read limit lacked a precise diagnostic' >&2
    exit 1
fi
main_ram_consumer_octal_limit_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT=04000000 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: decimal main-RAM consumer limit above its maximum was accepted through octal parsing' >&2
    exit 1
}
if [[ "$main_ram_consumer_octal_limit_output" != *'THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT must be an integer from 4096 through 1048576'* ]]; then
    printf '%s\n' 'FAIL: out-of-range leading-zero main-RAM consumer limit lacked a precise diagnostic' >&2
    exit 1
fi
main_ram_consumer_zero_padded_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT=0004096 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: valid zero-padded main-RAM consumer limit was rejected' >&2
    exit 1
}
if [[ "$main_ram_consumer_zero_padded_output" != *'SKIP:'* ]]; then
    printf '%s\n' 'FAIL: valid zero-padded main-RAM consumer limit did not reach the missing-media gate' >&2
    exit 1
fi
pce_fast_target_write_octal_limit_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_SAMPLE_LIMIT=09000000 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: decimal PCE Fast target-write limit above its maximum was accepted through octal parsing' >&2
    exit 1
}
if [[ "$pce_fast_target_write_octal_limit_output" != *'THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_SAMPLE_LIMIT must be an integer from 4096 through 1048576'* ]]; then
    printf '%s\n' 'FAIL: out-of-range leading-zero PCE Fast target-write limit lacked a precise diagnostic' >&2
    exit 1
fi
pce_fast_target_write_zero_padded_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_SAMPLE_LIMIT=00008192 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: valid zero-padded PCE Fast target-write limit was rejected' >&2
    exit 1
}
if [[ "$pce_fast_target_write_zero_padded_output" != *'SKIP:'* ]]; then
    printf '%s\n' 'FAIL: valid zero-padded PCE Fast target-write limit did not reach the missing-media gate' >&2
    exit 1
fi
cd_filter_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce \
    THERON_CAPTURE_PCE_FAST_CD_DATA_READ_START_AT_TRACK02=1 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: Track 02 CD data-read filtering accepted the standard PCE module' >&2
    exit 1
}
if [[ "$cd_filter_output" != *'Track 02 CD data-read filtering requires the PCE Fast module'* ]]; then
    printf '%s\n' 'FAIL: invalid CD data-read module lacked a precise diagnostic' >&2
    exit 1
fi
cd_filter_valid_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast \
    THERON_CAPTURE_PCE_FAST_CD_DATA_READ_TRACE_LIMIT=262144 \
    THERON_CAPTURE_PCE_FAST_CD_DATA_READ_START_AT_TRACK02=1 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: valid bounded Track 02 CD data-read settings were rejected' >&2
    exit 1
}
if [[ "$cd_filter_valid_output" != *'SKIP:'* ]]; then
    printf '%s\n' 'FAIL: valid CD data-read settings did not reach the missing-media gate' >&2
    exit 1
fi
cd_ram_write_limit_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_CD_RAM_TARGET_WRITE_TRACE_LIMIT=0 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: CD RAM target-write trace accepted a zero limit' >&2
    exit 1
}
if [[ "$cd_ram_write_limit_output" != *'THERON_CAPTURE_PCE_FAST_CD_RAM_TARGET_WRITE_TRACE_LIMIT must be an integer from 1 through 1048576'* ]]; then
    printf '%s\n' 'FAIL: invalid CD RAM target-write limit lacked a precise diagnostic' >&2
    exit 1
fi
cd_ram_write_valid_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast \
    THERON_CAPTURE_PCE_FAST_CD_RAM_TARGET_WRITE_TRACE_LIMIT=262144 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: valid CD RAM target-write trace limit was rejected' >&2
    exit 1
}
if [[ "$cd_ram_write_valid_output" != *'SKIP:'* ]]; then
    printf '%s\n' 'FAIL: valid CD RAM target-write limit did not reach the missing-media gate' >&2
    exit 1
fi
cd_ram_write_patch=$repo/scripts/mednafen_1.32.1_theron_pce_fast_cd_ram_target_write_trace.patch
if ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CD_RAM_TARGET_WRITE_TRACE_LIMIT' "$cd_ram_write_patch" ||
   ! grep -Fq 'count >= trace_limit' "$cd_ram_write_patch" ||
   grep -Fq 'count >= 8192' "$cd_ram_write_patch" ||
   ! grep -Fq 'poll_first = 0x68 * 8192 + 0x04d2' "$cd_ram_write_patch" ||
   ! grep -Fq 'poll_last = 0x68 * 8192 + 0x04e6' "$cd_ram_write_patch" ||
   ! grep -Fq '(!stage2_target && !poll_target && !existing_target)' "$cd_ram_write_patch" ||
   ! grep -Fq 'cd_ram_target_write_trace_limit=%s' "$script"; then
    printf '%s\n' 'FAIL: bounded CD RAM target-write trace must include the source-locked poll-code window' >&2
    exit 1
fi
vram_commit_limit_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE=1 \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_START_AT_TRACK02=1 \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE_LIMIT=4194305 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: PCE Fast VRAM commit trace accepted a limit above its maximum' >&2
    exit 1
}
if [[ "$vram_commit_limit_output" != *'THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE_LIMIT must be an integer from 1 through 4194304'* ]]; then
    printf '%s\n' 'FAIL: invalid PCE Fast VRAM commit limit lacked a precise diagnostic' >&2
    exit 1
fi
vram_commit_max_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE=1 \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_START_AT_TRACK02=1 \
    THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE_LIMIT=4194304 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: maximum PCE Fast VRAM commit trace limit was rejected' >&2
    exit 1
}
if [[ "$vram_commit_max_output" != *'SKIP: MEDNAFEN_BIN'* ]]; then
    printf '%s\n' 'FAIL: maximum PCE Fast VRAM commit limit did not reach the missing-media gate' >&2
    exit 1
fi
code_ram_write_invalid_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=2 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: PCE Fast code-bank RAM-write trace accepted a value other than 0 or 1' >&2
    exit 1
}
if [[ "$code_ram_write_invalid_output" != *'THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE must be 0 or 1'* ]]; then
    printf '%s\n' 'FAIL: invalid code-bank RAM-write option lacked a precise diagnostic' >&2
    exit 1
fi
code_ram_write_wrong_core_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=1 \
    "$script" 2>&1) && {
    printf '%s\n' 'FAIL: PCE Fast code-bank RAM-write trace accepted the standard PCE module' >&2
    exit 1
}
if [[ "$code_ram_write_wrong_core_output" != *'code-bank RAM-write tracing requires the pce_fast module'* ]]; then
    printf '%s\n' 'FAIL: wrong core for code-bank RAM-write trace lacked a precise diagnostic' >&2
    exit 1
fi
code_ram_write_enabled_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast \
    THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=1 \
    "$script" 2>&1) || {
    printf '%s\n' 'FAIL: enabled code-bank RAM-write option was rejected' >&2
    exit 1
}
if [[ "$code_ram_write_enabled_output" != *'SKIP: MEDNAFEN_BIN'* ]]; then
    printf '%s\n' 'FAIL: enabled code-bank RAM-write option did not reach the missing-media gate' >&2
    exit 1
fi
python3 "$repo/tests/test_theron_scripted_input_patch_parser.py"
python3 "$repo/tests/test_theron_mouse_input_parser.py"
bash "$repo/tests/test_theron_authentic_bram.sh"
input_test_dir=$(mktemp -d "$repo/build/theron-input-consumption.XXXXXX")
swift_module_cache=
window_retry_harness=
cleanup_capture_test_temporaries() {
    local temporary_directory
    for temporary_directory in \
        "$input_test_dir" "$swift_module_cache" "$window_retry_harness"; do
        if [[ -n "$temporary_directory" ]]; then
            rm -rf -- "$temporary_directory"
        fi
    done
}
trap cleanup_capture_test_temporaries EXIT

# The reference-only capture wrapper accepts one bounded delta/button sample
# per PCE mouse-device update. Validate this path before a real capture is
# attempted; valid input reaches the missing-media SKIP below, while malformed
# samples must fail before Mednafen can launch.
mouse_capture_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    THERON_CAPTURE_INPUT_DEVICE=mouse \
    THERON_CAPTURE_MOUSE_INPUT_SCRIPT='127,-127,0;0,0,1' \
    "$script" 2>&1) || {
    printf 'FAIL: bounded mouse input script was rejected:\n%s\n' \
        "$mouse_capture_output" >&2
    exit 1
}
if [[ "$mouse_capture_output" != *'SKIP: MEDNAFEN_BIN'* ]]; then
    printf 'FAIL: valid mouse input did not pass validation to the no-media boundary:\n%s\n' \
        "$mouse_capture_output" >&2
    exit 1
fi

while IFS='|' read -r device plan expected_error; do
    [[ -n "$device" ]] || continue
    if env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
        "THERON_CAPTURE_INPUT_DEVICE=$device" \
        "THERON_CAPTURE_MOUSE_INPUT_SCRIPT=$plan" \
        "$script" >"$input_test_dir/mouse-invalid.stdout" \
        2>"$input_test_dir/mouse-invalid.stderr"; then
        printf 'FAIL: mouse input validator accepted device=%s plan=%s\n' \
            "$device" "$plan" >&2
        exit 1
    fi
    if ! grep -Fq "$expected_error" "$input_test_dir/mouse-invalid.stderr"; then
        printf 'FAIL: mouse input rejection lacked the expected diagnostic for %s|%s\n' \
            "$device" "$plan" >&2
        cat "$input_test_dir/mouse-invalid.stderr" >&2
        exit 1
    fi
done <<'THERON_MOUSE_INPUT_INVALID_CASES'
gamepad|1,0,0|requires THERON_CAPTURE_INPUT_DEVICE=mouse
mouse|128,0,0|must be under 8192 characters
mouse|0,0,16|must be under 8192 characters
mouse|0,0|must be under 8192 characters
THERON_MOUSE_INPUT_INVALID_CASES
printf '%s\n' 'PASS: reference mouse input validator accepts bounded samples and rejects malformed input'

cat >"$input_test_dir/consumed.trace" <<'THERON_CONSUMED_INPUT'
source=mednafen-pce-scripted-input
scripted_pce_input_event frame=9600 key=run mask=0008 hold=90
scripted_pce_input_apply frame=9600 physical=0000 scripted=0008 combined=0008
pce_input_read frame=9600 cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0 value=07
THERON_CONSUMED_INPUT
consumption_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/consumed.trace" run@9600:90 131072)
if [[ "$consumption_receipt" != *'event_frames_with_apply=1'* ||
      "$consumption_receipt" != *'event_frames_followed_by_controller_read=1'* ||
      "$consumption_receipt" != *'event_frames_with_scripted_mask_read=1'* ||
      "$consumption_receipt" != *'controller_poll_boundary=verified'* ||
      "$consumption_receipt" != *'game_or_non_system_card_poll_boundary=observed'* ]]; then
    printf 'FAIL: post-event controller read was not verified:\n%s\n' \
        "$consumption_receipt" >&2
    exit 1
fi
"$scripted_input_consumption_verifier" \
    "$input_test_dir/consumed.trace" run@9600:90 8388608 >/dev/null
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/consumed.trace" run@9600:90 8388609 \
    >"$input_test_dir/oversized-read-limit.stdout" \
    2>"$input_test_dir/oversized-read-limit.stderr"; then
    printf '%s\n' 'FAIL: scripted-input verifier accepted a read limit above its bound' >&2
    exit 1
fi
cat >"$input_test_dir/mirrored-port.trace" <<'THERON_MIRRORED_INPUT_PORT'
scripted_pce_input_event frame=72 key=run mask=0008 hold=60
scripted_pce_input_apply frame=72 physical=0000 scripted=0008 combined=0008
pce_input_read frame=72 cpu_pc=42ab register=138d raw=0008 sel=0 clr=0 index=0 value=07
THERON_MIRRORED_INPUT_PORT
mirrored_port_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/mirrored-port.trace" run@72:60 8)
if [[ "$mirrored_port_receipt" != *'event_frames_with_scripted_mask_read=1'* ||
      "$mirrored_port_receipt" != *'controller_read_witness_pc=42ab'* ||
      "$mirrored_port_receipt" != *'game_or_non_system_card_poll_boundary=observed'* ]]; then
    printf 'FAIL: a mirrored PCE port-0 read was not verified:\n%s\n' \
        "$mirrored_port_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/mirrored-slot-four.trace" <<'THERON_MIRRORED_SLOT_FOUR_INPUT'
scripted_pce_input_event frame=74 key=run mask=0008 hold=60 port=4
scripted_pce_input_apply frame=74 physical=0000 scripted=0008 port=4
pce_input_read frame=74 cpu_pc=42ab register=1394 raw=0008 sel=0 clr=0 index=4 value=37
THERON_MIRRORED_SLOT_FOUR_INPUT
slot_four_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/mirrored-slot-four.trace" run@74:60 8 4)
if [[ "$slot_four_receipt" != *'scripted_input_port_index=4'* ||
      "$slot_four_receipt" != *'event_frames_with_scripted_mask_read=1'* ||
      "$slot_four_receipt" != *'controller_read_witness_pc=42ab'* ]]; then
    printf 'FAIL: selected PCE input slot 4 did not produce an attributed read:\n%s\n' \
        "$slot_four_receipt" >&2
    exit 1
fi
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/mirrored-slot-four.trace" run@74:60 8 \
    >"$input_test_dir/mirrored-slot-four-default.stdout" \
    2>"$input_test_dir/mirrored-slot-four-default.stderr"; then
    printf '%s\n' 'FAIL: selected PCE input slot 4 was accepted under default slot 0' >&2
    exit 1
fi
cat >"$input_test_dir/mirrored-wrong-index.trace" <<'THERON_MIRRORED_WRONG_INPUT_INDEX'
scripted_pce_input_event frame=73 key=run mask=0008 hold=60
scripted_pce_input_apply frame=73 physical=0000 scripted=0008 combined=0008
pce_input_read frame=73 cpu_pc=42ab register=1394 raw=0008 sel=0 clr=0 index=4 value=37
THERON_MIRRORED_WRONG_INPUT_INDEX
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/mirrored-wrong-index.trace" run@73:60 8 \
    >"$input_test_dir/mirrored-wrong-index.stdout" \
    2>"$input_test_dir/mirrored-wrong-index.stderr"; then
    printf '%s\n' 'FAIL: a mirrored read from controller index 4 was accepted as port 0' >&2
    exit 1
fi
cat >"$input_test_dir/system-card-only.trace" <<'THERON_SYSTEM_CARD_INPUT'
scripted_pce_input_event frame=9600 key=run mask=0008 hold=90
scripted_pce_input_apply frame=9600 physical=0000 scripted=0008 combined=0008
pce_input_read frame=9600 cpu_pc=e4c8 register=1000 raw=0008 sel=0 clr=0 index=0 value=07
THERON_SYSTEM_CARD_INPUT
system_card_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/system-card-only.trace" run@9600:90 131072)
if [[ "$system_card_receipt" != *'controller_poll_boundary=verified'* ||
      "$system_card_receipt" != *'system_card_poll_reads=1'* ||
      "$system_card_receipt" != *'game_or_non_system_card_poll_boundary=not_observed'* ]]; then
    printf 'FAIL: System Card-only input poll was not distinguished from game input:\n%s\n' \
        "$system_card_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/multiple-events.trace" <<'THERON_MULTI_EVENT_INPUT'
scripted_pce_input_event frame=10 key=run mask=0008 hold=1
scripted_pce_input_apply frame=10 physical=0000 scripted=0008 combined=0008
pce_input_read frame=10 cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0 value=07
scripted_pce_input_event frame=12 key=ii mask=0002 hold=1
scripted_pce_input_apply frame=12 physical=0008 scripted=0002 combined=000a
pce_input_read frame=12 cpu_pc=8123 register=1000 raw=000a sel=0 clr=0 index=0 value=05
THERON_MULTI_EVENT_INPUT
"$scripted_input_consumption_verifier" \
    "$input_test_dir/multiple-events.trace" run@10,ii@12 4 >/dev/null
cat >"$input_test_dir/same-frame-events.trace" <<'THERON_SAME_FRAME_INPUT'
scripted_pce_input_event frame=20 key=run mask=0008 hold=1
scripted_pce_input_event frame=20 key=ii mask=0002 hold=1
scripted_pce_input_apply frame=20 physical=0000 scripted=000a combined=000a
pce_input_read frame=20 cpu_pc=8123 register=1000 raw=000a sel=0 clr=0 index=0 value=05
THERON_SAME_FRAME_INPUT
"$scripted_input_consumption_verifier" \
    "$input_test_dir/same-frame-events.trace" run@20,ii@20 2 >/dev/null
cat >"$input_test_dir/legacy-pce-result.trace" <<'THERON_LEGACY_PCE_INPUT'
scripted_pce_input_event frame=60 key=up mask=0010 hold=60
scripted_pce_input_apply frame=60 physical=0000 scripted=0010 combined=0010
pce_input_read frame=60 cpu_pc=44ca register=1000 raw=0010 sel=1 clr=0 index=0
pce_input_port0 raw=0010
pce_input_result frame=60 cpu_pc=44ca register=1000 raw=0010 value=3e sel=1 clr=0 index=0 sp=f2 stack=4b436d74
THERON_LEGACY_PCE_INPUT
legacy_pce_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/legacy-pce-result.trace" up@60:60 1)
if [[ "$legacy_pce_receipt" != *'event_frames_with_scripted_mask_read=1'* ||
      "$legacy_pce_receipt" != *'observed_input_reads=1'* ||
      "$legacy_pce_receipt" != *'controller_read_witness_pc=44ca'* ]]; then
    printf 'FAIL: the PCE raw-read/result pair was not verified as one consumed poll:\n%s\n' \
        "$legacy_pce_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/legacy-mirrored-port.trace" <<'THERON_LEGACY_MIRRORED_PORT'
scripted_pce_input_event frame=61 key=run mask=0008 hold=2
scripted_pce_input_apply frame=61 physical=0000 scripted=0008 combined=0008
pce_input_read frame=61 cpu_pc=42ab register=13ff raw=0008 sel=0 clr=0 index=0
pce_input_port0 raw=0008
pce_input_result frame=61 cpu_pc=42ab register=13ff raw=0008 value=07 sel=0 clr=0 index=0 sp=f2 stack=4b436d74
THERON_LEGACY_MIRRORED_PORT
legacy_mirrored_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/legacy-mirrored-port.trace" run@61:2 4)
if [[ "$legacy_mirrored_receipt" != *'event_frames_with_scripted_mask_read=1'* ||
      "$legacy_mirrored_receipt" != *'controller_read_witness_pc=42ab'* ]]; then
    printf 'FAIL: legacy PCE raw/result pairing rejected a mirrored port read:\n%s\n' \
        "$legacy_mirrored_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/out-of-window-read.trace" <<'THERON_OUT_OF_WINDOW_INPUT'
scripted_pce_input_event frame=70 key=run mask=0008 hold=1
scripted_pce_input_apply frame=70 physical=0000 scripted=0008 combined=0008
pce_input_read frame=71 cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0 value=07
THERON_OUT_OF_WINDOW_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/out-of-window-read.trace" run@70 1 \
    >"$input_test_dir/out-of-window-read.stdout" \
    2>"$input_test_dir/out-of-window-read.stderr"; then
    printf '%s\n' 'FAIL: a controller poll after a one-frame scripted hold was accepted' >&2
    exit 1
fi
if ! grep -Fq 'only read outside its active hold interval' \
    "$input_test_dir/out-of-window-read.stderr"; then
    printf '%s\n' 'FAIL: out-of-window controller poll rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/legacy-pce-result-mismatch.trace" <<'THERON_LEGACY_PCE_MISMATCH'
scripted_pce_input_event frame=61 key=up mask=0010 hold=1
scripted_pce_input_apply frame=61 physical=0000 scripted=0010 combined=0010
pce_input_read frame=61 cpu_pc=44ca register=1000 raw=0010 sel=1 clr=0 index=0
pce_input_result frame=61 cpu_pc=44cb register=1000 raw=0010 value=3e sel=1 clr=0 index=0 sp=f2 stack=4b436d74
THERON_LEGACY_PCE_MISMATCH
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/legacy-pce-result-mismatch.trace" up@61 1 \
    >"$input_test_dir/legacy-pce-result-mismatch.stdout" \
    2>"$input_test_dir/legacy-pce-result-mismatch.stderr"; then
    printf '%s\n' 'FAIL: a mismatched PCE read/result pair was accepted as input consumption' >&2
    exit 1
fi
cat >"$input_test_dir/legacy-pce-result-interleaved.trace" <<'THERON_LEGACY_PCE_INTERLEAVED'
scripted_pce_input_event frame=62 key=up mask=0010 hold=1
scripted_pce_input_apply frame=62 physical=0000 scripted=0010 combined=0010
pce_input_read frame=62 cpu_pc=44ca register=1000 raw=0010 sel=1 clr=0 index=0
trace_marker unrelated=1
pce_input_result frame=62 cpu_pc=44ca register=1000 raw=0010 value=3e sel=1 clr=0 index=0 sp=f2 stack=4b436d74
THERON_LEGACY_PCE_INTERLEAVED
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/legacy-pce-result-interleaved.trace" up@62 1 \
    >"$input_test_dir/legacy-pce-result-interleaved.stdout" \
    2>"$input_test_dir/legacy-pce-result-interleaved.stderr"; then
    printf '%s\n' 'FAIL: a non-adjacent PCE result was accepted as input consumption' >&2
    exit 1
fi
if ! grep -Fq 'legacy PCE input result did not immediately follow its raw read' \
    "$input_test_dir/legacy-pce-result-interleaved.stderr"; then
    printf '%s\n' 'FAIL: non-adjacent PCE read/result rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/legacy-pce-result-frame-mismatch.trace" <<'THERON_LEGACY_PCE_FRAME_MISMATCH'
scripted_pce_input_event frame=63 key=up mask=0010 hold=1
scripted_pce_input_apply frame=63 physical=0000 scripted=0010 combined=0010
pce_input_read frame=63 cpu_pc=44ca register=1000 raw=0010 sel=1 clr=0 index=0
pce_input_result frame=64 cpu_pc=44ca register=1000 raw=0010 value=0e sel=1 clr=0 index=0
THERON_LEGACY_PCE_FRAME_MISMATCH
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/legacy-pce-result-frame-mismatch.trace" up@63 1 \
    >"$input_test_dir/legacy-pce-result-frame-mismatch.stdout" \
    2>"$input_test_dir/legacy-pce-result-frame-mismatch.stderr"; then
    printf '%s\n' 'FAIL: mismatched legacy PCE input frames were accepted' >&2
    exit 1
fi
if ! grep -Fq 'legacy PCE input result frame did not match its raw read' \
    "$input_test_dir/legacy-pce-result-frame-mismatch.stderr"; then
    printf '%s\n' 'FAIL: legacy PCE frame mismatch lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/event-at-read-cap.trace" <<'THERON_CAPPED_INPUT'
pce_input_read frame=1 cpu_pc=8123 register=1000 raw=0000 sel=0 clr=0 index=0
pce_input_read frame=2 cpu_pc=8123 register=1000 raw=0000 sel=0 clr=0 index=0
scripted_pce_input_event frame=12 key=run mask=0008 hold=1
scripted_pce_input_apply frame=12 physical=0000 scripted=0008 combined=0008
THERON_CAPPED_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/event-at-read-cap.trace" run@12 2 \
    >"$input_test_dir/capped.stdout" 2>"$input_test_dir/capped.stderr"; then
    printf '%s\n' 'FAIL: scripted input at the read-trace cap was accepted without a later CPU read' >&2
    exit 1
fi
if ! grep -Fq 'final scripted event frame had no controller-port read exposing its scripted mask' \
    "$input_test_dir/capped.stderr"; then
    printf '%s\n' 'FAIL: capped scripted input rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/unconsumed-event.trace" <<'THERON_UNCONSUMED_INPUT'
scripted_pce_input_event frame=20 key=run mask=0008 hold=1
scripted_pce_input_apply frame=20 physical=0000 scripted=0008 combined=0008
pce_input_write cpu_pc=8123 register=1000 data=0008 sel_before=0 clr_before=0 index=0
THERON_UNCONSUMED_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/unconsumed-event.trace" run@20 2 \
    >"$input_test_dir/unconsumed.stdout" 2>"$input_test_dir/unconsumed.stderr"; then
    printf '%s\n' 'FAIL: scripted input without a controller-port read was accepted' >&2
    exit 1
fi
if ! grep -Fq 'final scripted event frame had no controller-port read exposing its scripted mask' \
    "$input_test_dir/unconsumed.stderr"; then
    printf '%s\n' 'FAIL: unconsumed scripted input rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/wrong-mask.trace" <<'THERON_WRONG_MASK_INPUT'
scripted_pce_input_event frame=40 key=up mask=0010 hold=1
scripted_pce_input_apply frame=40 physical=0000 scripted=0010 combined=0010
pce_input_read frame=40 cpu_pc=8123 register=1000 raw=0000 sel=1 clr=0 index=0 value=0f
THERON_WRONG_MASK_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/wrong-mask.trace" up@40 2 \
    >"$input_test_dir/wrong-mask.stdout" 2>"$input_test_dir/wrong-mask.stderr"; then
    printf '%s\n' 'FAIL: controller polling without the requested direction bit was accepted' >&2
    exit 1
fi
cat >"$input_test_dir/direction-readback.trace" <<'THERON_DIRECTION_READBACK'
scripted_pce_input_event frame=45 key=up mask=0010 hold=1
scripted_pce_input_apply frame=45 physical=0000 scripted=0010 combined=0010
pce_input_read frame=45 cpu_pc=8123 register=1000 raw=0010 sel=1 clr=0 index=0 value=0e
THERON_DIRECTION_READBACK
direction_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/direction-readback.trace" up@45 1)
if [[ "$direction_receipt" != *'event_frames_with_scripted_mask_read=1'* ]]; then
    printf 'FAIL: direction input was not verified in the selected direction bank:\n%s\n' \
        "$direction_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/wrong-direction-bank.trace" <<'THERON_WRONG_DIRECTION_BANK'
scripted_pce_input_event frame=46 key=up mask=0010 hold=1
scripted_pce_input_apply frame=46 physical=0000 scripted=0010 combined=0010
pce_input_read frame=46 cpu_pc=8123 register=1000 raw=0010 sel=0 clr=0 index=0 value=0f
THERON_WRONG_DIRECTION_BANK
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/wrong-direction-bank.trace" up@46 1 \
    >"$input_test_dir/wrong-direction-bank.stdout" 2>"$input_test_dir/wrong-direction-bank.stderr"; then
    printf '%s\n' 'FAIL: raw direction mask without a selected direction-bank value was accepted' >&2
    exit 1
fi
cat >"$input_test_dir/wrong-button-bank.trace" <<'THERON_WRONG_BUTTON_BANK'
scripted_pce_input_event frame=47 key=run mask=0008 hold=1
scripted_pce_input_apply frame=47 physical=0000 scripted=0008 combined=0008
pce_input_read frame=47 cpu_pc=8123 register=1000 raw=0008 sel=1 clr=0 index=0 value=0f
THERON_WRONG_BUTTON_BANK
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/wrong-button-bank.trace" run@47 1 \
    >"$input_test_dir/wrong-button-bank.stdout" 2>"$input_test_dir/wrong-button-bank.stderr"; then
    printf '%s\n' 'FAIL: raw button mask without a selected button-bank value was accepted' >&2
    exit 1
fi
cat >"$input_test_dir/wrong-input-index.trace" <<'THERON_WRONG_INPUT_INDEX'
scripted_pce_input_event frame=47 key=up mask=0010 hold=1
scripted_pce_input_apply frame=47 physical=0000 scripted=0010 combined=0010
pce_input_read frame=47 cpu_pc=8123 register=1000 raw=0010 sel=1 clr=0 index=1 value=0e
THERON_WRONG_INPUT_INDEX
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/wrong-input-index.trace" up@47 1 \
    >"$input_test_dir/wrong-input-index.stdout" 2>"$input_test_dir/wrong-input-index.stderr"; then
    printf '%s\n' 'FAIL: a matching input read from the wrong controller index was accepted' >&2
    exit 1
fi
cat >"$input_test_dir/combined-bank-readback.trace" <<'THERON_COMBINED_BANK_READBACK'
scripted_pce_input_event frame=48 key=up mask=0010 hold=1
scripted_pce_input_event frame=48 key=run mask=0008 hold=1
scripted_pce_input_apply frame=48 physical=0000 scripted=0018 combined=0018
pce_input_read frame=48 cpu_pc=8123 register=1000 raw=0018 sel=1 clr=0 index=0 value=0e
pce_input_read frame=48 cpu_pc=8123 register=1000 raw=0018 sel=0 clr=0 index=0 value=07
THERON_COMBINED_BANK_READBACK
combined_bank_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/combined-bank-readback.trace" up@48,run@48 2)
if [[ "$combined_bank_receipt" != *'event_frames_with_scripted_mask_read=1'* ]]; then
    printf 'FAIL: simultaneous direction/button input was not verified in both banks:\n%s\n' \
        "$combined_bank_receipt" >&2
    exit 1
fi
if ! grep -Fq 'controller-port read exposing its scripted mask' \
    "$input_test_dir/wrong-mask.stderr"; then
    printf '%s\n' 'FAIL: wrong-mask input rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/partial-combined-mask.trace" <<'THERON_PARTIAL_COMBINED_MASK_INPUT'
scripted_pce_input_event frame=50 key=run mask=0008 hold=1
scripted_pce_input_event frame=50 key=ii mask=0002 hold=1
scripted_pce_input_apply frame=50 physical=0000 scripted=000a combined=000a
pce_input_read frame=50 cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0
THERON_PARTIAL_COMBINED_MASK_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/partial-combined-mask.trace" run@50,ii@50 2 \
    >"$input_test_dir/partial-combined-mask.stdout" 2>"$input_test_dir/partial-combined-mask.stderr"; then
    printf '%s\n' 'FAIL: a partial simultaneous controller mask was accepted' >&2
    exit 1
fi
cat >"$input_test_dir/no-apply.trace" <<'THERON_NO_APPLY_INPUT'
scripted_pce_input_event frame=30 key=run mask=0008 hold=1
pce_input_read frame=30 cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0
THERON_NO_APPLY_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/no-apply.trace" run@30 2 \
    >"$input_test_dir/no-apply.stdout" 2>"$input_test_dir/no-apply.stderr"; then
    printf '%s\n' 'FAIL: controller polling without a scripted apply receipt was accepted' >&2
    exit 1
fi
if ! grep -Fq 'final scripted event frame has no nonzero apply receipt' \
    "$input_test_dir/no-apply.stderr"; then
    printf '%s\n' 'FAIL: scripted apply rejection lacked a precise diagnostic' >&2
    exit 1
fi
if route_output=$(THERON_CAPTURE_MENU_ROUTE=drator-generator \
    bash "$script" 2>&1); then
    printf 'FAIL: drator-generator capture accepted a missing replay script\n' >&2
    exit 1
fi
if [[ "$route_output" != *'requires THERON_CAPTURE_REPLAY_INPUT_SCRIPT'* ]]; then
    printf 'FAIL: drator-generator capture did not reject a stalled input-frame clock\n' >&2
    exit 1
fi
if route_output=$(THERON_CAPTURE_MENU_ROUTE=drator-generator \
    THERON_CAPTURE_REPLAY_INPUT_SCRIPT=run@1:5 bash "$script" 2>&1); then
    printf 'FAIL: cold-start drator-generator accepted RUN at the wrong frame\n' >&2
    exit 1
fi
if [[ "$route_output" != *'RUN replay event at frame 9600'* ]]; then
    printf 'FAIL: cold-start drator-generator did not enforce the authentic RUN handoff frame\n' >&2
    exit 1
fi
if route_output=$(THERON_CAPTURE_MENU_ROUTE=drator-generator \
    THERON_CAPTURE_REPLAY_INPUT_SCRIPT=run@9600:90 \
    THERON_CAPTURE_REPLAY_INPUT_PORT=1 bash "$script" 2>&1); then
    printf '%s\n' 'FAIL: drator-generator accepted research input on a non-source-bound port' >&2
    exit 1
fi
if [[ "$route_output" != *'source-bound to PCE controller port 0'* ]]; then
    printf '%s\n' 'FAIL: invalid Drator research port lacked a precise diagnostic' >&2
    exit 1
fi
if route_output=$(THERON_CAPTURE_REPLAY_INPUT_PORT=5 bash "$script" 2>&1); then
    printf '%s\n' 'FAIL: capture accepted PCE input slot 5 outside the supported range' >&2
    exit 1
fi
if [[ "$route_output" != *'THERON_CAPTURE_REPLAY_INPUT_PORT must be an integer from 0 through 4'* ]]; then
    printf 'FAIL: invalid scripted PCE input slot lacked a precise diagnostic:\n%s\n' \
        "$route_output" >&2
    exit 1
fi
route_output=$(THERON_CAPTURE_MENU_ROUTE=drator-generator \
    THERON_CAPTURE_REPLAY_INPUT_SCRIPT=run@9600:90 bash "$script" 2>&1)
if [[ "$route_output" != *'SKIP: MEDNAFEN_BIN, THERON_US_CUE/THERON_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required'* ]]; then
    printf 'FAIL: valid cold-start route preflight did not reach normal capture argument validation\n' >&2
    exit 1
fi
if ! grep -Fq 'mkdir -p "$home_dir/sav"' "$script" ||
   ! grep -Fq -- '-filesys.path_sav "$home_dir/sav"' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_AUTHENTIC_BRAM must name a 2048-byte original Backup RAM image' "$bram_helper" ||
   ! grep -Fq 'THERON_CAPTURE_AUTHENTIC_BRAM is not the authenticated USA Backup RAM image' "$bram_helper" ||
   ! grep -Fq 'cp "$source_path" "$destination_path"' "$bram_helper" ||
   ! grep -Fq 'md5_file "$destination_path"' "$bram_helper" ||
   ! grep -Fq 'theron_prepare_authentic_bram' "$script" ||
   ! grep -Fq 'firestaff-theron-authentic.sav' "$bram_helper" ||
   ! grep -Fq '"${capture_sav_args[@]}"' "$script" ||
   ! grep -Fq 'mkdir -p "$home_dir/mcs"' "$script" ||
   ! grep -Fq -- '-filesys.path_state "$home_dir/mcs"' "$script"; then
    printf '%s\n' 'FAIL: isolated captures must bind backup RAM and state files to the private capture home' >&2
    exit 1
fi
authentic_bram_copy_line=$(grep -nF 'cp "$source_path" "$destination_path"' "$bram_helper" | head -n1 | cut -d: -f1)
authentic_bram_hash_line=$(grep -nF 'md5_file "$destination_path"' "$bram_helper" | head -n1 | cut -d: -f1)
if grep -Fq 'md5_file "$capture_authentic_bram"' "$script" ||
   [[ -z $authentic_bram_copy_line || -z $authentic_bram_hash_line ||
      $authentic_bram_copy_line -ge $authentic_bram_hash_line ]]; then
    printf '%s\n' 'FAIL: authenticate the private BRAM copy after copying, not the mutable source path' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_TITLE_WAIT_INPUT="$title_wait_input"' "$script" ||
   ! grep -Fq 'RdMem((PC - 3) & 0xffff) == 0x20' "$title_wait_patch" ||
   ! grep -Fq '(PC & 0x1fff) == 0x0865' "$title_wait_patch" ||
   ! grep -Fq 'WrMem(0x2228, 0x08)' "$title_wait_patch"; then
    printf '%s\n' 'FAIL: title-wait research input is not signature-bound and wired into capture' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_MENU_ROUTE="$menu_route"' "$script" ||
   ! grep -Fq 'PC == 0x6e44' "$drator_menu_patch" ||
   ! grep -Fq 'PC == 0x6dbd' "$drator_menu_patch" ||
   ! grep -Fq 'WrMem(0x2228, 0x01)' "$drator_menu_patch"; then
    printf '%s\n' 'FAIL: Drator research route is not signature-bound and wired into capture' >&2
    exit 1
fi
if ! grep -Fq 'instrument_mednafen_theron_pce_fast_drator_route.py' "$build_script" ||
   ! grep -Fq 'TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC)' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_drator_route.py" ||
   ! grep -Fq 'RdMem(0x6e44) == 0xad' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_drator_route.py" ||
   ! grep -Fq 'if(x == 0) new_data |= TheronPCEFastDratorGeneratorInputMask()' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_drator_route.py"; then
    printf '%s\n' 'FAIL: PCE Fast Drator route must be signature-gated and wired to controller port 0' >&2
    exit 1
fi
drator_route_instrumenter="$repo/scripts/instrument_mednafen_theron_pce_fast_drator_route.py"
if ! grep -Fq 'extern unsigned TheronPCEFastScriptFrame;' "$drator_route_instrumenter" ||
   ! grep -Fq 'static int TheronPCEFastDratorMenuRouteMode(void)' "$drator_route_instrumenter" ||
   [[ $(grep -c 'getenv("FIRESTAFF_THERON_MENU_ROUTE")' "$drator_route_instrumenter") != 1 ]]; then
    printf '%s\n' 'FAIL: injected route helper must declare the frame counter and cache the environment mode per emulator process' >&2
    exit 1
fi
if ! grep -Fq "grep -Fqx 'truncated=event_limit' \"\$huc6280_23ad_trace\"" "$script"; then
    printf '%s\n' 'FAIL: bounded post-return capture must not be rejected as a truncated decoder trace' >&2
    exit 1
fi
cat >"$host_key_gate_dir/huc6280.trace" <<'HUC6280_TRUNCATION_MARKERS'
post_return_truncated=event_limit
HUC6280_TRUNCATION_MARKERS
if grep -Fqx 'truncated=event_limit' "$host_key_gate_dir/huc6280.trace"; then
    printf '%s\n' 'FAIL: post-return window truncation was mistaken for an incomplete decoder trace' >&2
    exit 1
fi
printf '%s\n' 'truncated=event_limit' >>"$host_key_gate_dir/huc6280.trace"
if ! grep -Fqx 'truncated=event_limit' "$host_key_gate_dir/huc6280.trace"; then
    printf '%s\n' 'FAIL: incomplete decoder trace marker was not recognized' >&2
    exit 1
fi
if ! grep -Fq 'caller_hash == 0x699e8da1' "$huc6280_23ad_probe" ||
   ! grep -Fq 'routine_hash == 0x3056f96c' "$huc6280_23ad_probe" ||
   ! grep -Fq 'expected_opcodes[4] = { 0x8d, 0xa5, 0x8d, 0x44 }' "$huc6280_23ad_probe" ||
   ! grep -Fq 'expected_offsets[4] = { 2, 5, 7, 10 }' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADCallerCandidateStage < 4' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADEventLimit = 2000000' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADPostReturnEventLimit = 200000' "$huc6280_23ad_probe" ||
   ! grep -Fq 'TheronTraceCDSharedEventSequence++' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADPostReturn ? "post_return" : "decoder"' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADPostReturn = true' "$huc6280_23ad_probe" ||
   ! grep -Fq 'value == 0xa5 && pc <= 0xfe81' "$huc6280_23ad_probe" ||
   ! grep -Fq 'FirestaffTheron23ADStackPC = call_pc + 1' "$huc6280_23ad_probe" ||
   ! grep -Fq 'pc == (uint32)FirestaffTheron23ADRoutinePC + 0x7cu' "$huc6280_23ad_probe" ||
   ! grep -Fq 'caller_signature fnv1a=699e8da1' "$huc6280_23ad_probe" ||
   grep -Fq 'physical_pc == 0x1f2429' "$huc6280_23ad_probe" ||
   grep -Fq 'physical_pc >= 0x1f2386' "$huc6280_23ad_probe" ||
   ! grep -Fq 'return=%04x stack_return=%04x mpr=' "$huc6280_23ad_probe" ||
   ! grep -Fq 'call=BSR caller=' "$script" ||
   ! grep -Fq -- '--huc6280-23ad "$huc6280_23ad_trace"' "$script" ||
   ! grep -Fq -- '--huc6280-23ad-limit 2000000' "$script" ||
   ! grep -Fq 'routine_fnv1a=3056f96c' "$script" ||
   ! grep -Fq 'L2429:  rts' \
       "$repo/docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm"; then
    printf '%s\n' 'FAIL: $23AD probe must identify authentic caller and routine bytes without assuming a live MPR mapping' >&2
    exit 1
fi
if ! grep -Fq '!strcmp(route, "drator-generator")' "$drator_menu_patch" ||
   ! grep -Fq 'if(!route || strcmp(route, "drator-generator")) return 0;' "$drator_menu_patch" ||
   ! grep -Fq 'The authentic cold-start RUN is supplied by the replay script' "$drator_menu_patch" ||
   grep -Fq 'TheronScriptInputFrame() % 300u' "$drator_menu_patch" ||
   grep -Fq 'return 0x0008' "$drator_menu_patch" ||
   ! grep -Fq 'const unsigned row = scan_frame / 270' "$drator_menu_patch" ||
   ! grep -Fq 'if(row < 10)' "$drator_menu_patch" ||
   ! grep -Fq 'if(TheronDratorGeneratorRouteStage == 3 && frame <= 240)' "$drator_menu_patch" ||
   ! grep -Fq '++TheronDratorGeneratorRouteDelay >= 5000' "$drator_menu_patch" ||
   ! grep -Fq 'buttons |= TheronDratorGeneratorInputMask()' "$drator_menu_patch"; then
    printf '%s\n' 'FAIL: Drator generator route lost its source-derived panel targeting or gamepad edge' >&2
    exit 1
fi
if ! grep -Fq 'THERON_US_CUE:-${THERON_CUE:-}' "$script" ||
   ! grep -Fq 'TQJP02End.iso' "$script" ||
   ! grep -Fq '397039af02d50d15c70b74088eb8a1cb' "$script" ||
   ! grep -Fq 'THERON_US_CUE/THERON_CUE' "$script"; then
    printf 'FAIL: live capture must support the authenticated Japanese CUE/ISO route\n' >&2
    exit 1
fi
if ! grep -Fq 'bs=512 skip=1' "$script" ||
   ! grep -Fq '38179df8f4ac870017db21ebcbf53114' "$script" ||
   ! grep -Fq 'system_card_runtime_md5=%s' "$script" ||
   ! grep -Fq '"$capture_cdbios_setting" "$capture_system_card"' "$script" ||
   ! grep -Fq '"$capture_cue"' "$script"; then
    printf '%s\n' 'FAIL: capture must strip and hash-check the 512-byte System Card header before launching Mednafen' >&2
    exit 1
fi
if ! grep -Fq 'capture_clonecd_track02=0' "$script" ||
   ! grep -Fq 'clonecd_start" == 3234' "$script" ||
   ! grep -Fq 'clonecd_count" == 3371' "$script" ||
   ! grep -Fq '168bd6a63784e91885df8c47be62ab5a' "$script"; then
    printf '%s\n' 'FAIL: live capture must authenticate the bounded US CloneCD Track 02 range' >&2
    exit 1
fi
if ! grep -Fq 'capture_force_kill_seconds=5' "$script" ||
   ! grep -Fq 'if [[ "$capture_mednafen_module" == pce_fast ]]; then' "$script" ||
   ! grep -Fq 'capture_shutdown_signal=TERM' "$script" ||
   ! grep -Fq 'capture_shutdown_signal=INT' "$script" ||
   ! grep -Fq 'gtimeout -k "$capture_force_kill_seconds" -s "$capture_shutdown_signal"' "$script" ||
   ! grep -Fq 'timeout -k "$capture_force_kill_seconds" -s "$capture_shutdown_signal"' "$script"; then
    printf '%s\n' 'FAIL: live capture timeout must force-terminate a Mednafen process that ignores its soft shutdown signal' >&2
    exit 1
fi
if [[ ! -x "$later_raw_receipt" ]] ||
   ! grep -Fq 'captured physical-to-raw Track 02 delta is not the observed US value' "$later_raw_receipt" ||
   ! grep -Fq 'no Stage-3 descriptor binds the range, so payload semantics remain blocked' "$later_raw_receipt"; then
    printf 'FAIL: later raw-sector receipt must retain its authenticated fail-closed boundary\n' >&2
    exit 1
fi
bash -n "$script"
if [[ ! -f "$x11_keymap" ]]; then
    printf '%s\n' 'FAIL: X11 keyboard mapping helper is missing' >&2
    exit 1
fi
source "$x11_keymap"
while IFS=' ' read -r key scancode expected; do
    [[ -n "$key" ]] || continue
    actual=$(theron_x11_key_for_sdl_mapping "$key" "$scancode") || {
        printf 'FAIL: no X11 key for SDL mapping %s:%s\n' "$key" "$scancode" >&2
        exit 1
    }
    if [[ "$actual" != "$expected" ]]; then
        printf 'FAIL: X11 mapping %s:%s returned %s, expected %s\n' \
            "$key" "$scancode" "$actual" "$expected" >&2
        exit 1
    fi
done <<'THERON_X11_KEYMAP'
i 91 KP_3
ii 90 KP_2
up 26 w
down 22 s
left 4 a
right 7 d
i 32 3
ii 31 2
up 82 Up
down 81 Down
left 80 Left
right 79 Right
i 12 i
i 29 z
ii 27 x
i 54 comma
i 55 period
ii 54 comma
ii 55 period
THERON_X11_KEYMAP
while IFS=' ' read -r binding expected; do
    [[ -n "$binding" ]] || continue
    actual=$(theron_x11_chord_for_sdl_binding "$binding") || {
        printf 'FAIL: no X11 chord for SDL binding %s\n' "$binding" >&2
        exit 1
    }
    if [[ "$actual" != "$expected" ]]; then
        printf 'FAIL: SDL binding %s returned X11 chord %s, expected %s\n' \
            "$binding" "$actual" "$expected" >&2
        exit 1
    fi
done <<'THERON_X11_CHORDS'
101+ctrl+shift ctrl+shift+Menu
10+ctrl+shift ctrl+shift+g
8+ctrl+shift ctrl+shift+e
82+alt+meta alt+super+Up
THERON_X11_CHORDS
for binding in '' 101 999+ctrl+shift 101+unknown; do
    if theron_x11_chord_for_sdl_binding "$binding" >/dev/null 2>&1; then
        printf 'FAIL: X11 grab chord parser accepted unsupported binding %s\n' \
            "${binding:-<empty>}" >&2
        exit 1
    fi
done
while IFS=' ' read -r scancode expected; do
    [[ -n "$scancode" ]] || continue
    actual=$(theron_x11_key_for_sdl_scancode "$scancode") || {
        printf 'FAIL: no X11 keysym for supported SDL scancode %s\n' "$scancode" >&2
        exit 1
    }
    if [[ "$actual" != "$expected" ]]; then
        printf 'FAIL: SDL scancode %s returned X11 keysym %s, expected %s\n' \
            "$scancode" "$actual" "$expected" >&2
        exit 1
    fi
done <<'THERON_X11_SCANCODES'
4 a
5 b
6 c
7 d
8 e
9 f
10 g
11 h
12 i
13 j
14 k
15 l
16 m
17 n
18 o
19 p
20 q
21 r
22 s
23 t
24 u
25 v
26 w
27 x
28 y
29 z
30 1
31 2
32 3
33 4
34 5
35 6
36 7
37 8
38 9
39 0
40 Return
41 Escape
42 BackSpace
43 Tab
44 space
45 minus
46 equal
47 bracketleft
48 bracketright
49 backslash
50 numbersign
51 semicolon
52 apostrophe
53 grave
54 comma
55 period
56 slash
57 Caps_Lock
58 F1
59 F2
60 F3
61 F4
62 F5
63 F6
64 F7
65 F8
66 F9
67 F10
68 F11
69 F12
70 Print
71 Scroll_Lock
72 Pause
73 Insert
74 Home
75 Page_Up
76 Delete
77 End
78 Page_Down
79 Right
80 Left
81 Down
82 Up
89 KP_1
90 KP_2
91 KP_3
92 KP_4
93 KP_5
94 KP_6
95 KP_7
96 KP_8
97 KP_9
98 KP_0
101 Menu
224 Control_L
225 Shift_L
226 Alt_L
227 Super_L
228 Control_R
229 Shift_R
230 Alt_R
231 Super_R
THERON_X11_SCANCODES
for scancode in 0 3 83 88 99 100 102 223 232; do
    if theron_x11_key_for_sdl_scancode "$scancode" >/dev/null 2>&1; then
        printf 'FAIL: X11 scancode mapper accepted unsupported SDL scancode %s\n' \
            "$scancode" >&2
        exit 1
    fi
done
if theron_x11_key_for_sdl_mapping run 0 >/dev/null 2>&1; then
    printf '%s\n' 'FAIL: X11 keymap accepted an unlisted PCE mapping' >&2
    exit 1
fi
if grep -Eq '/tmp|TMPDIR' "$script" ||
   ! grep -Fq 'capture_scratch_root=${THERON_CAPTURE_SCRATCH_ROOT:-"$script_dir/../.codex-scratch"}' "$script" ||
   ! grep -Fq 'mktemp -d "$capture_scratch_root/firestaff-theron-mednafen.XXXXXX"' "$script"; then
    printf 'FAIL: live capture scratch must stay under the repository unless explicitly overridden\n' >&2
    exit 1
fi
if [[ ! -x "$runtime_verifier" ]] ||
   ! grep -Fq 'sdl2-compat' "$runtime_verifier" ||
   ! grep -Fq 'use a real SDL2 runtime for authentic Quartz/SDL capture' "$runtime_verifier" ||
   ! grep -Fq 'Compiled against SDL' "$runtime_verifier" ||
   ! grep -Fq 'running with SDL' "$runtime_verifier" ||
   ! grep -Fq 'version_is_at_least' "$runtime_verifier" ||
   ! grep -Fq 'isolated SDL startup probe' "$runtime_verifier"; then
    printf 'FAIL: live capture build must reject SDL2-compat and verify an isolated runtime/version match\n' >&2
    exit 1
fi
if [[ ! -x "$build_script" ]] || ! grep -Fq -- '--without-libflac' "$build_script" ||
   ! grep -Fq 'CXXFLAGS="${CXXFLAGS:--O2}" ./configure' "$build_script" ||
   ! grep -Fq 'git -C "$build_root/source" init --quiet' "$build_script" ||
   ! grep -Fq 'git -C "$build_root/source" add --all --force' "$build_script" ||
   ! grep -Fq 'apply --recount --ignore-space-change' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_input_result_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_scripted_pce_input.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_input_grab_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_cd_transfer_owner_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_main_ram_loader_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_main_ram_loader_write_trace_v3.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_main_ram_e009_critical_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_main_ram_consumer_read_trace.patch' "$build_script"; then
    printf 'FAIL: raw Track 02 trace build must not depend on an unrelated FLAC header path\n' >&2
    exit 1
fi
if [[ ! -f "$consumer_read_patch" ]] ||
   ! grep -Fq '@@ -407,11 +409,30 @@' "$consumer_read_patch" ||
   ! grep -Fq 'const uint16 reader_pc = GetRegister(GSREG_PC);' "$consumer_read_patch"; then
    printf 'FAIL: main-RAM consumer patch must retain its applicable hunk span and reader context\n' >&2
    exit 1
fi
if [[ ! -f "$movement_data_instrumenter" ]] ||
   ! grep -Fq 'TheronPCECDInstructionFetch' "$movement_data_instrumenter" ||
   ! grep -Fq 'physical_address >= 0x0e8a00' "$movement_data_instrumenter" ||
   ! grep -Fq 'physical_address <= 0x0e8aff' "$movement_data_instrumenter" ||
   ! grep -Fq 'logical_address == 0x2905' "$movement_data_instrumenter" ||
   ! grep -Fq 'reader_pc == 0xd34d' "$movement_data_instrumenter" ||
   ! grep -Fq 'FIRESTAFF_THERON_MOVEMENT_DATA_READ_TRACE' "$movement_data_instrumenter" ||
   ! grep -Fq 'instrument_mednafen_theron_movement_data_read_trace.py' "$build_script"; then
    printf 'FAIL: movement-data capture must exclude instruction fetches and retain bounded command provenance\n' >&2
    exit 1
fi
if [[ ! -f "$pce_fast_movement_context_instrumenter" ]] ||
   ! grep -Fq 'RdOp' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'movement_command_write' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'input_latch_write' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'runtime_state_write' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'main_ram_write' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'A == 0x2911 || A == 0x2912 || A == 0x2920' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq '0x0e8a00' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'TheronTraceInstructionPC' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'TheronPCEFastScriptFrame' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_START_FRAME' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'frame=%u' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'mpr0=%02x' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'mpr7=%02x' "$pce_fast_movement_context_instrumenter" ||
   grep -Fq 'GetRealPC_EXTERNAL' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'TheronPCEFastMovementRuntimeStateWrite' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq '0x2000u + zero_page_address' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'ST_ZP(r)' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'ST_ZPX(r)' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'ST_ZPY(r)' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'RMW_ZP(op)' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'RMW_ZPX(op)' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE' "$pce_fast_movement_context_instrumenter" ||
   ! grep -Fq 'instrument_mednafen_theron_pce_fast_movement_context_trace.py' "$build_script" ||
   ! grep -Fq 'build_jobs=1' "$build_script"; then
    printf 'FAIL: PCE Fast movement context must record data-only reads without borrowing PCE command provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'pce_fast_movement_context_trace="${trace}.pce-fast-movement-context"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE="$pce_fast_movement_context_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_SAMPLE_LIMIT="${THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_SAMPLE_LIMIT:-128}"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_START_FRAME="${THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_START_FRAME:-$input_trace_start_frame}"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE="${THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE:-}"' "$script"; then
    printf '%s\n' 'FAIL: capture must own and bound the PCE Fast movement-context trace' >&2
    exit 1
fi
if [[ ! -f "$ram_provenance_patch" ]] ||
   ! grep -Fq '@@ -420,8 +428,12 @@' "$ram_provenance_patch" ||
   ! grep -Fq 'const uint8 reader_mpr = MPR[reader_pc >> 13];' "$ram_provenance_patch"; then
    printf 'FAIL: RAM-provenance patch must follow the consumer-reader patch context\n' >&2
    exit 1
fi
if [[ ! -f "$adpcm_context_patch" ]] ||
   ! grep -Fq 'static void TheronTracePCECDAdpcm(const char *format, ...)' "$adpcm_context_patch" ||
   ! grep -Fq 'ADPCM.ReadBuffer = ADPCM.RAM[adpcm_address];' "$adpcm_context_patch" ||
   ! grep -Fq 'FIRESTAFF_PATCH_BLANK_CONTEXT' "$adpcm_context_patch" ||
   ! grep -Fq 'adpcm_context_rendered=' "$build_script" ||
   ! grep -Fq 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_adpcm_fifo_ram_trace_context.patch' "$build_script" ||
   grep -Fq '< "$repo/scripts/mednafen_1.32.1_theron_adpcm_fifo_ram_trace.patch"' "$build_script"; then
    printf 'FAIL: the capture build must use the context-bound ADPCM patch, not stale line-only hunks\n' >&2
    exit 1
fi
if [[ ! -f "$adpcm_playback_patch" ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_ADPCM_PLAYBACK_TRACE' "$script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_adpcm_playback_trace.patch' "$build_script" ||
   ! grep -Fq 'adpcm_control_result cpu_pc=%04x physical_pc=%06x' "$adpcm_playback_patch" ||
   ! grep -Fq 'playback_start=%u' "$adpcm_playback_patch"; then
    printf 'FAIL: Theron capture no longer retains CPU/MPR-bound ADPCM playback starts\n' >&2
    exit 1
fi
if [[ ! -f "$cdda_command_patch" ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_CDDA_COMMAND_TRACE' "$script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_cdda_command_trace.patch' "$build_script" ||
   ! grep -Fq 'source=mednafen-pce-instrumented-cdda-command-v1' "$cdda_command_patch" ||
   ! grep -Fq 'opcode != 0xD8 && opcode != 0xD9 && opcode != 0xDA' "$cdda_command_patch" ||
   grep -Fq 'cd.command_buffer_pos != 10' "$cdda_command_patch" ||
   ! grep -Fq 'status=good' "$cdda_command_patch"; then
    printf 'FAIL: authentic accepted PCE CDDA commands must be captured with command and status provenance\n' >&2
    exit 1
fi
if [[ ! -f "$input_grab_patch" ]] ||
   ! grep -Fq 'input_grab_state enabled=%u' "$input_grab_patch" ||
   ! grep -Fq 'TheronTraceHostInput' "$input_grab_patch" ||
   ! grep -Fq 'input_grab_state enabled=1' "$script" ||
   ! grep -Fq 'did not attest InputGrab=1 after host chord retries' "$script"; then
   printf 'FAIL: capture must require Mednafen-owned InputGrab=1 provenance\n' >&2
   exit 1
fi
if [[ ! -f "$loader_write_v3_patch" ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE' "$loader_write_v3_patch" ||
   ! grep -Fq 'dispatch_sequence=unbound' "$loader_write_v3_patch" ||
   ! grep -Fq 'writer_physical_pc' "$loader_write_v3_patch"; then
    printf 'FAIL: loader-write capture must retain explicit unbound MPR/source provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_1.32.1_theron_vram_vce_snapshot.patch' "$build_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VRAM_SNAPSHOT="$vram_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VCE_SNAPSHOT="$vce_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_STATE_SNAPSHOT="$vdc_state_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_SAT_SNAPSHOT="$vdc_sat_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_SNAPSHOT="$main_ram_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_BRAM_SNAPSHOT="$bram_snapshot"' "$script" ||
   ! grep -Fq "require_snapshot_size \"\$vdc_sat_snapshot\" 512" "$script" ||
   ! grep -Fq "require_snapshot_size \"\$main_ram_snapshot\" 8192" "$script" ||
   ! grep -Fq "require_snapshot_size \"\$bram_snapshot\" 2048" "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_STATE_V1' "$script" ||
   ! grep -Fq 'same-instant HuC6270 register snapshot' "$script" ||
   ! grep -Fq 'VDC::GSREG_MWR' "$repo/scripts/mednafen_1.32.1_theron_vram_vce_snapshot.patch"; then
    printf 'FAIL: live capture must retain same-instant VDC VRAM, VCE palette and HuC6270 register snapshots\n' >&2
    exit 1
fi
if ! grep -Fq 'old_color % 256 + (input_byte % 2) * 256' "$script" ||
   grep -Fq 'int(input_byte / 2) % 2' "$script"; then
    printf 'FAIL: PCE Fast VCE register 5 validation must use input bit 0 for the color high bit\n' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_1.32.1_theron_vdc_io_trace.patch' "$build_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_IO_TRACE="$vdc_io_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_IO_TRACE_V1' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_VDC_IO_TRACE_LIMIT' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_VDC_IO_TRACE_LIMIT="$vdc_io_trace_limit"' "$script" ||
   ! grep -Fq 'sequence=${vdc_io_trace_limit}' "$script" ||
   ! grep -Fq 'TheronTraceVDCIOPostWrite' "$repo/scripts/mednafen_1.32.1_theron_vdc_io_trace.patch" ||
   ! grep -Fq 'TheronGraphicsSnapshotDumped' "$repo/scripts/mednafen_1.32.1_theron_vram_vce_snapshot.patch" ||
   ! grep -Fq 'writer_physical_pc=%06x' "$repo/scripts/mednafen_1.32.1_theron_vdc_io_trace.patch"; then
    printf 'FAIL: live capture must retain the side-effect-free VDC I/O writer trace\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_CAPTURE_INPUT_TRACE_LIMIT' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_INPUT_TRACE_LIMIT="$input_trace_limit"' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_INPUT_TRACE_START_FRAME' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_INPUT_TRACE_START_FRAME="$input_trace_start_frame"' "$script" ||
   ! grep -Fq 'input_trace_start_frame=$((earliest_replay_frame - 120))' "$script" ||
   ! grep -Fq 'input_trace_limit_default=65536' "$script" ||
   ! grep -Fq 'input_trace_limit_default=1048576' "$script" ||
   ! grep -Fq '|| -n "$host_key" || -n "$host_key_sequence"' "$script" ||
   ! grep -Fq 'input_trace_limit < 65536 || input_trace_limit > 8388608' "$script" ||
   ! grep -Fq 'read_limit > 8388608' "$scripted_input_consumption_verifier" ||
   ! grep -Fq 'verify_theron_scripted_input_consumption.sh' "$script" ||
   ! grep -Fq 'event_frames_followed_by_controller_read' "$scripted_input_consumption_verifier" ||
   ! grep -Fq 'event_frames_with_scripted_mask_read' "$scripted_input_consumption_verifier" ||
   ! grep -Fq 'game_or_non_system_card_poll_boundary=not_observed' "$scripted_input_consumption_verifier" ||
   ! grep -Fq 'input_trace_limit=%s' "$script"; then
    printf 'FAIL: live capture must bound the controller trace and require post-event CPU polling\n' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_1.32.1_theron_main_ram_e009_register_trace.patch' "$build_script" ||
   ! grep -Fq 'main_ram_e009_register_writes=%s' "$script"; then
    printf 'FAIL: capture must retain bounded main-RAM e009 register-write provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_1.32.1_theron_rng_consumer_trace.patch' "$build_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_CONSUMER_TRACE="$rng_consumer_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_CONSUMER_SAMPLE_LIMIT="$rng_consumer_sample_limit"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_CODE_TRACE="$rng_code_trace"' "$script" ||
   ! grep -Fq 'rng_consumer_samples=%s' "$script" ||
   ! grep -Fq 'rng_code_windows=%s' "$script" ||
   ! grep -Fq 'rng_consumer_window sequence=%u step=%u pc=%04x physical_pc=%08x' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_CODE_TRACE' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_STATE_TRACE' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'rng_state_boundary sequence=%u kind=%s pc=%04x physical_pc=%08x' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'caller_physical_pc=%08x' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'source=mednafen-pce-instrumented-rng-state-v2' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'rng_state_physical_pc == 0x000d0667' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq '0x2100u + ((reg_sp + 1u)' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'rng_code_window entry=%s logical_pc=%04x physical_pc=%08x' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'logical_pc == 0x5d64' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'logical_pc == 0x5d6a' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch"; then
    printf 'FAIL: capture build must retain a bounded raw RNG-consumer execution window\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_CAPTURE_RNG_CONSUMER_SAMPLE_LIMIT' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_RNG_CONSUMER_WINDOW_LIMIT' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_CONSUMER_WINDOW_LIMIT="$rng_consumer_window_limit"' "$script" ||
   ! grep -Fq 'rng_consumer_sample_limit=%u' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'rng_consumer_window_limit=%u' "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch" ||
   ! grep -Fq 'rng_consumer_sample_limit=' "$repo/src/theron/theron_v1_mednafen_spawn_consumer_trace.c"; then
    printf '%s\n' 'FAIL: RNG capture must declare and validate its bounded sample limit' >&2
    exit 1
fi
if [[ ! -f "$irq2_patch" ]] ||
   ! grep -Fq 'fputs("source=mednafen-pce-instrumented-cd\n", trace);' "$irq2_patch" ||
   grep -Fq 'fputs("source=mednafen-pce-instrumented-cd\\n", trace);' "$irq2_patch" ||
   ! grep -Fq 'if(ok && trace_count < 256)' "$irq2_patch"; then
    printf 'FAIL: raw-sector provenance trace must be parseable and retain its full witness window\n' >&2
    exit 1
fi
if ! grep -Fq 'trace_files_are_line_delimited()' "$script" ||
   ! grep -Fq 'index($_, chr(92) . "n")' "$script" ||
   ! grep -Fq 'existing_trace_files' "$script" ||
   ! grep -Fq '"$rng_code_trace" "$rng_state_trace" "$rng_generator_context_trace" "$vdc_io_trace" "$command_ram_trace"' "$script" ||
   ! grep -Fq 'Mednafen emitted a literal backslash-n in a trace record' "$script"; then
    printf 'FAIL: capture script must reject merged literal-backslash-n trace rows\n' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_COMMAND_RAM_TRACE="$command_ram_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_COMMAND_CONSUMER_TRACE="$command_consumer_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_COMMAND_CODE_SNAPSHOT="$command_code_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_COMMAND_RAM_BEFORE_SNAPSHOT="$command_ram_before_snapshot"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_COMMAND_RAM_AFTER_SNAPSHOT="$command_ram_after_snapshot"' "$script" ||
   ! grep -Fq 'command_ram_boundary sequence=65536' "$script" ||
   ! grep -Fq 'input-only control emitted command snapshot sidecars' "$script" ||
   ! grep -Fq 'command_before_ram_snapshot_bytes=8192' "$script" ||
   ! grep -Fq 'command_after_ram_snapshot_bytes=8192' "$script" ||
   ! grep -Fq 'command_consumer_boundary sequence=' "$script" ||
   ! grep -Fq 'command_consumer_reads=' "$script" ||
   ! grep -Fq 'command_consumer_source_reads=' "$script" ||
   ! grep -Fq 'RNG state trace is not a contiguous entry/return sequence' "$script" ||
   ! grep -Fq 'command_consumer_post_dispatch_source_reads=' "$script" ||
   ! grep -Fq 'address == "2905" && pc == "d34d"' "$script" ||
   ! grep -Fq 'logical_address=[23][0-9a-f][0-9a-f][0-9a-f]' "$script" ||
   ! grep -Fq 'count >= 65536' "$script"; then
    printf '%s\n' 'FAIL: capture must retain an atomic authentic command/RAM/code window' >&2
    exit 1
fi
if [[ ! -f "$quartz_helper" ]] ||
   ! grep -Fq 'CGEvent(keyboardEventSource: source' "$quartz_helper" ||
   ! grep -Fq 'CGPreflightPostEventAccess()' "$quartz_helper" ||
   ! grep -Fq 'if globalHid {' "$quartz_helper" ||
   ! grep -Fq 'targetApplication.activate()' "$quartz_helper" ||
   ! grep -Fq 'not_required_pid_delivery' "$quartz_helper" ||
   ! grep -Fq 'quartz_target_not_frontmost activation=' "$quartz_helper" ||
   ! grep -Fq 'down.postToPid(targetPid)' "$quartz_helper" ||
   ! grep -Fq 'up.postToPid(targetPid)' "$quartz_helper" ||
   ! grep -Fq 'down.post(tap: .cghidEventTap)' "$quartz_helper" ||
   ! grep -Fq 'quartz_keypair=posted_to_global_hid' "$quartz_helper"; then
   printf 'FAIL: capture script must retain the checked-in Quartz keypair helper\n' >&2
   exit 1
fi
if [[ ! -f "$quartz_grab_helper" ]] ||
   ! grep -Fq 'Ctrl+Shift+G' "$quartz_grab_helper" ||
   ! grep -Fq 'quartz_chord_keys=ctrl+shift+g' "$quartz_grab_helper" ||
   ! grep -Fq 'postToPid(targetPid)' "$quartz_grab_helper" ||
   ! grep -Fq 'quartz_chord=posted_to_global_hid' "$quartz_grab_helper"; then
   printf 'FAIL: capture must activate Mednafen input grabbing through the checked-in Quartz chord helper\n' >&2
   exit 1
fi
if ! grep -Fq 'quartz_grab_script=' "$script" ||
   ! grep -Fq 'host chord retries' "$script" ||
   ! grep -Fq 'input_grab_state enabled=1' "$script" ||
   ! grep -Fq 'input_grab_chord_events=%s' "$script"; then
   printf 'FAIL: capture must attest input-grab activation before host input\n' >&2
   exit 1
fi
if ! grep -Fq 'expected_quartz_activation=quartz_activation=not_required' "$script" ||
   ! grep -Fq 'expected_quartz_activation=quartz_activation=accepted' "$script" ||
   ! grep -Fq 'Quartz global HID delivery requires Mednafen to own the foreground' "$script" ||
   ! grep -Fq 'quartz_target_not_frontmost activation=' "$quartz_helper" ||
   ! grep -Fq 'quartz_activation=\(activation)' "$quartz_helper" ||
   ! grep -Fq 'quartz_target_focus=\(focus)' "$quartz_helper"; then
    printf 'FAIL: capture must attest that the target owns the foreground\n' >&2
    exit 1
fi
if grep -Fq 'activationAccepted' "$quartz_helper"; then
    printf 'FAIL: Quartz helper must not reference an undefined activation result\n' >&2
    exit 1
fi
if command -v swiftc >/dev/null 2>&1; then
    swift_module_cache=$(mktemp -d "$repo/build/firestaff-theron-swift-module-cache.XXXXXX")
    # AppKit module caches are large. Keep the focused check repeatable on
    # constrained developer disks instead of leaving generated artifacts.
    trap 'rm -rf -- "$swift_module_cache"' EXIT
    if ! swiftc -module-cache-path "$swift_module_cache" -typecheck "$quartz_helper" >/dev/null 2>&1; then
        printf 'FAIL: Quartz helper does not type-check\n' >&2
        exit 1
    fi
    if ! swiftc -module-cache-path "$swift_module_cache" -typecheck "$quartz_grab_helper" >/dev/null 2>&1; then
        printf 'FAIL: Quartz input-grab helper does not type-check\n' >&2
        exit 1
    fi
fi
if swift "$quartz_helper" 36 1 0 >/dev/null 2>&1; then
    printf 'FAIL: Quartz helper accepted a non-positive target PID\n' >&2
    exit 1
fi
if ! grep -Fq -- '-force_module "$capture_mednafen_module"' "$script" ||
   ! grep -Fq -- 'THERON_CAPTURE_MEDNAFEN_MODULE' "$script" ||
   ! grep -Fq -- 'THERON_CAPTURE_POINTER_C1DF_TRACE' "$script" ||
   ! grep -Fq -- "pointer_c1df_begin' \"\$binary\"" "$script" ||
   ! grep -Fq -- 'pointer_c1df_trace_rows=%s' "$script" ||
   ! grep -Fq -- 'mednafen_module=%s' "$script" ||
   ! grep -Fq -- 'capture_arcadecard_setting=pce.arcadecard' "$script" ||
   ! grep -Fq -- 'capture_arcadecard_setting=pce_fast.arcadecard' "$script" ||
   ! grep -Fq -- 'capture_cdbios_setting=pce_fast.cdbios' "$script" ||
   ! grep -Fq -- 'capture_input_port_setting=pce.input.port1' "$script" ||
   ! grep -Fq -- 'capture_input_port_setting=pce_fast.input.port1' "$script" ||
   ! grep -Fq -- 'capture_input_args=("-$capture_input_port_setting" "$capture_input_device")' "$script" ||
   ! grep -Fq -- 'capture_input_args+=(' "$script" ||
   ! grep -Fq -- '"-pce_fast.input.port1.gamepad.$key"' "$script" ||
   ! grep -Fq -- '"keyboard 0x0 $pce_scancode"' "$script" ||
   ! grep -Fq -- '"${capture_input_args[@]}"' "$script"; then
    printf 'FAIL: capture script must force the PCE module and disable unrelated Arcade Card emulation\n' >&2
    exit 1
fi
if ! grep -Fq -- 'FIRESTAFF_THERON_PCE_FAST_POINTER_TRACE_SUPPORT' "$build_script" ||
   ! grep -Fq -- '"$capture_pointer_c1df_trace" == 1' "$script" ||
   ! grep -Fq -- 'capture_mednafen_module=pce_fast' "$script"; then
    printf '%s\n' 'FAIL: C1DF pointer tracing must build and capture with the PCE Fast core' >&2
    exit 1
fi
if ! grep -Fq -- 'A == 0x21D6 || (A >= 0x28FC && A <= 0x2905)' "$pointer_instrumenter" ||
   ! grep -Fq -- 'TheronTracePointerCodeWindow' "$pointer_instrumenter" ||
   ! grep -Fq -- 'HuCPU.PCERead[bank](physical)' "$pointer_instrumenter" ||
   ! grep -Fq -- 'code_window=%s' "$pointer_instrumenter" ||
   ! grep -Fq -- 'A == 0x28B8' "$pointer_instrumenter" ||
   ! grep -Fq -- 'A == 0x2912' "$pointer_instrumenter" ||
   ! grep -Fq -- 'TheronPointerAddress(A) || TheronPointerCandidateAddress(A) ||' "$pointer_instrumenter" ||
   ! grep -Fq -- 'FIRESTAFF_THERON_POINTER_RAM_WRITE_TRACE' "$pointer_instrumenter" ||
   ! grep -Fq -- 'FIRESTAFF_THERON_POINTER_RAM_WRITE_TRACE="$pce_fast_pointer_ram_write_trace"' "$script" ||
   ! grep -Fq -- 'pointer_ram_write_trace_rows=%s' "$script"; then
    printf '%s\n' 'FAIL: mouse pointer traces must watch and report the candidate RAM writes' >&2
    exit 1
fi
if ! python3 - "$pointer_instrumenter" <<'PY'
import ast
import re
import sys
from pathlib import Path

instrumenter = Path(sys.argv[1])
tree = ast.parse(instrumenter.read_text(encoding="utf-8"))
replacement = None
for node in ast.walk(tree):
    if not isinstance(node, ast.Call) or getattr(node.func, "id", None) != "replace_once":
        continue
    if len(node.args) < 4:
        continue
    try:
        path = ast.literal_eval(node.args[1])
        code = ast.literal_eval(node.args[3])
    except (TypeError, ValueError):
        continue
    if path == "src/pce_fast/huc6280.cpp" and "TheronPointerC1DFReturnPC =" in code:
        replacement = code
        break

if replacement is None:
    raise SystemExit("FAIL: generated PCE Fast C1DF hook was not found")

condition_match = re.search(
    r"\bif\s*\((?P<condition>(?:[^()]|\([^()]*\))*)\)\s*"
    r"\{\s*TheronPointerC1DFReturnPC\s*=",
    replacement,
    re.DOTALL,
)
if condition_match is None:
    raise SystemExit("FAIL: C1DF caller condition was not found")

condition = condition_match.group("condition")
address = r"RdOp\s*\(\s*TheronTraceInstructionPC\s*(?:\+\s*\d+\s*)?\)"
number = r"(?:0[xX][0-9a-fA-F]+|\d+)"
comparison = re.compile(
    rf"(?P<left_call>{address})\s*(?P<left_op>==|!=|<=|>=|<|>)\s*"
    rf"(?P<left_value>{number})"
    rf"|(?P<right_value>{number})\s*"
    rf"(?P<right_op>==|!=|<=|>=|<|>)\s*(?P<right_call>{address})"
)

byte_comparisons = set()
for match in comparison.finditer(condition):
    if match.group("left_call"):
        call = match.group("left_call")
        operator = match.group("left_op")
        value = match.group("left_value")
    else:
        call = match.group("right_call")
        operator = match.group("right_op")
        value = match.group("right_value")
    offset_match = re.search(r"\+\s*(\d+)", call)
    offset = int(offset_match.group(1)) if offset_match else 0
    if operator != "==":
        raise SystemExit("FAIL: C1DF instruction bytes must be matched exactly")
    byte_comparisons.add((offset, int(value, 0)))

if byte_comparisons != {(0, 0x20), (1, 0xDF), (2, 0xC1)}:
    raise SystemExit("FAIL: C1DF condition must match opcode 0x20 and operands 0xDF, 0xC1")

unmatched = comparison.sub("", condition)
unmatched = re.sub(r"\s+|&&|[()]", "", unmatched)
if unmatched:
    raise SystemExit("FAIL: C1DF condition contains an additional caller-PC restriction")
PY
then
    printf '%s\n' 'FAIL: C1DF tracing must recognize the complete JSR $C1DF instruction at any caller address' >&2
    exit 1
fi
if ! grep -Fq 'PC Engine (CD)/TurboGrafx 16 (CD)/SuperGrafx' "$script" ||
   ! grep -Fq 'Some instrumented 1.32.1 macOS builds omit the module-list block' "$script"; then
    printf 'FAIL: capture module gate must retain the authenticated help-less PCE fallback\n' >&2
    exit 1
fi
if ! grep -Fq 'No help-less fallback is safe for pce_fast' "$script" ||
   ! grep -Fq 'never infer `pce_fast` from a' "$script"; then
    printf 'FAIL: capture module gate must reject an unadvertised pce_fast string match\n' >&2
    exit 1
fi
if ! grep -Fq 'capture_split_iso_cache=' "$script" ||
   ! grep -Fq 'ceb02343868f80cec899e9b239aff2da' "$script" ||
   ! grep -Fq 'theron-capture.cue' "$script" ||
   ! grep -Fq 'cp "$cue" "$capture_cue"' "$script" ||
   ! grep -Fq 'ln -s "$capture_split_iso_cache" "$track02_capture_member"' "$script" ||
   ! grep -Fq 'filesys.untrusted_fip_check rejects absolute paths' "$script" ||
   ! grep -Fq 'production intake assembles and hashes this exact ISO' "$script" ||
   ! grep -Fq '"$capture_cue"' "$script"; then
    printf 'FAIL: live capture must reuse the authenticated split-ISO materialization path\n' >&2
    exit 1
fi
if ! grep -Fq 'MODE1/2048' "$script" ||
   ! grep -Fq '397039af02d50d15c70b74088eb8a1cb|ceb02343868f80cec899e9b239aff2da' "$script" ||
   ! grep -Fq 'MODE1\/(2352|2048)' "$script" ||
   ! grep -Fq 'track02_mode=%s' "$script"; then
    printf 'FAIL: live capture must admit the authenticated MODE1/2048 ISO route separately from raw BIN\n' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_IRQ2_INPUT_TRACE="$input_trace"' "$script"; then
    printf 'FAIL: capture script must retain a raw controller input receipt\n' >&2
    exit 1
fi
python3 - "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch" \
    "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_input.patch" <<'PY'
import re
import sys
from pathlib import Path

hunk = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")
for name in sys.argv[1:]:
    lines = Path(name).read_text(encoding="utf-8").splitlines()
    for index, header in enumerate(lines):
        match = hunk.match(header)
        if not match:
            continue
        old_count = new_count = 0
        cursor = index + 1
        while cursor < len(lines) and not lines[cursor].startswith(("@@ ", "--- a/")):
            line = lines[cursor]
            if line == "FIRESTAFF_PATCH_BLANK_CONTEXT" or line.startswith("FIRESTAFF_PATCH_TAB_CONTEXT"):
                old_count += 1
                new_count += 1
            elif line.startswith(" "):
                old_count += 1
                new_count += 1
            elif line.startswith("+"):
                new_count += 1
            elif line.startswith("-"):
                old_count += 1
            elif line.startswith("\\ No newline"):
                pass
            else:
                raise SystemExit(f"{name}:{cursor + 1}: invalid unified-diff context")
            cursor += 1
        expected_old = int(match.group(2) or 1)
        expected_new = int(match.group(4) or 1)
        if (old_count, new_count) != (expected_old, expected_new):
            raise SystemExit(
                f"{name}:{index + 1}: hunk declares {expected_old}/{expected_new}, "
                f"contains {old_count}/{new_count}"
            )
PY

if ! grep -Fq 'source=mednafen-pce-fast-instrumented-input' "$script" ||
   ! grep -Fq 'scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch' "$repo/scripts/build_mednafen_theron_irq2_trace.sh" ||
   ! grep -Fq 'static void TheronPCEFastInputTraceRead' "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_INPUT_TRACE_START_FRAME' "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch" ||
   ! grep -Fq 'TheronPCEFastScriptFrame >= TheronPCEFastInputTraceStartFrame' "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch"; then
    printf '%s\n' 'FAIL: pce_fast capture must retain source- and bus-bound input traces' >&2
    exit 1
fi
if ! awk '
    function finish_hunk() {
        if (in_hunk && seen != expected)
            exit 1
    }
    /^@@ -[0-9]+(,[0-9]+)? \+[0-9]+(,[0-9]+)? @@/ {
        finish_hunk()
        range_count = split($3, range, ",")
        expected = (range_count == 2) ? range[2] : 1
        seen = 0
        in_hunk = 1
        next
    }
    /^(---|\+\+\+) / {
        finish_hunk()
        in_hunk = 0
        next
    }
    in_hunk && (/^[ +]/ || /^FIRESTAFF_PATCH_(BLANK|TAB)_CONTEXT/) { seen++ }
    END { finish_hunk() }
' "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch"; then
    printf '%s\n' 'FAIL: PCE Fast input-trace patch has an invalid unified-diff hunk count' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_1.32.1_theron_scripted_pce_fast_input.patch' "$repo/scripts/build_mednafen_theron_irq2_trace.sh" ||
   ! grep -Fq 'FIRESTAFF_THERON_REPLAY_INPUT_SCRIPT' "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_fast_input.patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_REPLAY_INPUT_PORT' "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_fast_input.patch" ||
   ! grep -Fq 'scripted_pce_input_event frame=%u key=%s mask=%04x hold=%u port=%u' "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_fast_input.patch" ||
   ! grep -Fq 'if(x == (int)scripted_port) new_data |= scripted_mask' "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_fast_input.patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_REPLAY_INPUT_PORT="$replay_input_port"' "$script" ||
   ! grep -Fq 'scripted_pce_input_port=%s' "$script"; then
    printf '%s\n' 'FAIL: pce_fast replay input must expose and report the configured controller slot' >&2
    exit 1
fi
if ! grep -Fq 'if [[ "$capture_mednafen_module" == pce ]]; then' "$script" ||
   ! grep -Fq 'require_snapshot_size "$pce_fast_main_ram_snapshot" 8192' "$script" ||
   ! grep -Fq 'pce_fast_main_ram_snapshot_bytes=8192' "$script" ||
   ! grep -Fq 'PCE Fast capture must not emit PCE-only VDC snapshots or traces' "$script" ||
   ! grep -Fq 'vdc_io_writes=unavailable' "$script" ||
   ! grep -Fq 'vdc_io_trace_limit=unavailable' "$script" ||
   ! grep -Fq 'transition_ram_snapshot=$pce_fast_main_ram_snapshot' "$script" ||
   ! grep -Fq 'transition_ram_2041=$(od -An -tx1 -j 65 -N 1 "$transition_ram_snapshot"' "$script"; then
    printf '%s\n' 'FAIL: capture snapshots must match the selected Mednafen core' >&2
    exit 1
fi
if ! grep -Fq 'theron_input_read_result_pending' "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch" ||
   ! grep -Fq 'theron_script_trace_frame >= theron_input_read_trace_start_frame' "$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch" ||
   ! grep -Fq 'theron_script_trace_frame >= theron_input_write_trace_start_frame' "$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch" ||
   grep -Fq 'theron_input_read_count <= theron_input_read_trace_limit' "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch" ||
   ! grep -Fq 'HuCPU.PeekLogical(0x2100u | ((sp + 1u) & 0xffu))' "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch" ||
   ! grep -Fq 'stack=%02x%02x%02x%02x' "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch"; then
    printf 'FAIL: input-result evidence must be bounded and retain authentic caller-stack bytes\n' >&2
    exit 1
fi
if ! grep -Fq 'TheronIrq2TraceCriticalSamples[critical_slot] < 4096' "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch"; then
    printf 'FAIL: IRQ evidence must remain bounded during System Card polling loops\n' >&2
    exit 1
fi
if ! grep -Fq 'require_instrumented_mednafen_binary()' "$script" ||
   ! grep -Fq 'for marker in FIRESTAFF_THERON_IRQ2_TRACE FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE' "$script" ||
   ! grep -Fq 'grep -aFq "$marker" "$binary"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE' "$script" ||
   ! grep -Fq 'MEDNAFEN_BIN lacks the required Firestaff Theron instrumentation' "$script" ||
   ! grep -Fq 'build_mednafen_theron_irq2_trace.sh' "$script"; then
    printf 'FAIL: capture script must reject an uninstrumented Mednafen binary before launch\n' >&2
    exit 1
fi
if ! grep -Fq 'capture_pce_fast_vram_commit_trace" == 1' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_START_AT_TRACK02' "$script" ||
   ! grep -Fq 'prefix=omitted-until-first-source-valid-track02-read' "$script" ||
   ! grep -Fq "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE' \"\$mednafen_bin\"" "$script" ||
   ! grep -Fq 'MEDNAFEN_BIN lacks the required PCE Fast VRAM commit instrumentation' "$script"; then
    printf '%s\n' 'FAIL: PCE Fast VRAM capture must reject a binary missing its commit-trace producer before launch' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE="$main_ram_loader_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE="$main_ram_consumer_trace"' "$script" ||
   ! grep -Fq 'pce_fast_main_ram_consumer_range=${THERON_CAPTURE_PCE_FAST_MAIN_RAM_CONSUMER_RANGE:-}' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE="$pce_fast_main_ram_consumer_range"' "$script" ||
   ! grep -Fq 'range_last > 0x1FFFFF' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_CONSUMER_SAMPLE_LIMIT="$main_ram_consumer_sample_limit"' "$script" ||
   ! grep -Fq 'main_ram_loader_tii_transfers=%s' "$script" ||
   ! grep -Fq 'continuation_tii_source_3c80=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_rts=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_post_rts=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_call_entries=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_entry_next=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_entry_successor_next=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_bra=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_bra_targets=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_bra_target_jsrs=%s' "$script" ||
   ! grep -Fq 'main_ram_loader_e009_dispatches=%s' "$script" ||
   ! grep -Fq 'main_ram_consumer_reads=%s' "$script" ||
   ! grep -Fq 'pce_fast_code_ram_writes=%s' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE="$pce_fast_code_ram_write_trace"' "$script" ||
   ! grep -Fq 'pce_fast_code_ram_write_trace_enabled=${THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE:-0}' "$script" ||
   ! grep -Fq 'main_ram_target_reads=%s' "$script" ||
   ! grep -Fq 'main_ram_target_writes=%s' "$script"; then
    printf 'FAIL: capture script must retain the post-$3800 TII producer receipt\n' >&2
    exit 1
fi
code_ram_write_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_code_ram_write_trace.patch"
if [[ ! -f "$code_ram_write_patch" ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE' "$code_ram_write_patch" ||
   ! grep -Fq 'TheronTraceInstructionPhysicalPC < 0x1F1C00' "$code_ram_write_patch" ||
   ! grep -Fq 'TheronTraceInstructionPhysicalPC >= 0x1F1E00' "$code_ram_write_patch" ||
   ! grep -Fq 'physical_address < 0x0D0000 || physical_address >= 0x0D2000' "$code_ram_write_patch" ||
   ! grep -Fq 'static void TheronTraceA1B7BMTWrite(uint32 physical_destination,' "$code_ram_write_patch" ||
   ! grep -Fq 'mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x' "$code_ram_write_patch" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_code_ram_write_trace.patch' "$build_script"; then
    printf '%s\n' 'FAIL: PCE Fast code-window RAM-write trace must retain source PC, mapping and bounded destination writes' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_SUPPORT' "$build_script" ||
   ! grep -Fq 'if [[ "$pce_fast_code_ram_write_trace" == 1 ]]' "$build_script" ||
   ! grep -Fq 'configure_pce_fast=(--enable-pce-fast)' "$build_script"; then
    printf '%s\n' 'FAIL: the code-bank RAM-write build option must enable PCE Fast' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_ERROR' "$code_ram_write_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_ERROR' "$script" ||
   ! grep -Fq '[[ ! -s "$pce_fast_code_ram_write_trace" ]]' "$script" ||
   ! grep -Fq 'source=mednafen-pce-fast-code-ram-write event_sequence_source=TheronTraceCDSharedEventSequence' "$script"; then
    printf '%s\n' 'FAIL: PCE Fast code-bank RAM-write trace failures and missing receipts must be rejected' >&2
    exit 1
fi
main_ram_target_write_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_main_ram_target_write_trace.patch"
if [[ ! -f "$main_ram_target_write_patch" ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE' "$main_ram_target_write_patch" ||
   ! grep -Fq 'TheronTracePCEFastMainRAMTargetWrite(A, physical, V);' "$main_ram_target_write_patch" ||
   ! grep -Fq 'writer_code_bytes=%s' "$main_ram_target_write_patch" ||
   ! grep -Fq 'event_sequence=%u' "$main_ram_target_write_patch" ||
   grep -Fq 'HuCPU.A' "$main_ram_target_write_patch" ||
   grep -Fq 'HuCPU.X' "$main_ram_target_write_patch" ||
   grep -Fq 'HuCPU.Y' "$main_ram_target_write_patch" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_main_ram_target_write_trace.patch' "$build_script" ||
   ! grep -Fq 'THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE="$pce_fast_main_ram_target_write_range"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_TARGET_WRITE_TRACE_ERROR' "$script" ||
   ! grep -Fq 'pce_fast_main_ram_target_writes=%s' "$script"; then
    printf '%s\n' 'FAIL: bounded PCE Fast target-write tracing must retain MPRs, code bytes and the shared event sequence' >&2
    exit 1
fi
target_write_trace_assignment_line=$(grep -n '^pce_fast_main_ram_target_write_trace=' "$script" | head -n 1 | cut -d: -f1)
target_write_trace_wait_line=$(grep -n '^wait "\$mednafen_pid"$' "$script" | cut -d: -f1)
target_write_trace_validation_line=$(grep -nF 'FAIL: Mednafen did not produce a valid PCE Fast main-RAM target-write trace' "$script" | cut -d: -f1)
if [[ ! "$target_write_trace_assignment_line" =~ ^[0-9]+$ ||
      ! "$target_write_trace_wait_line" =~ ^[0-9]+$ ||
      ! "$target_write_trace_validation_line" =~ ^[0-9]+$ ]] ||
   (( target_write_trace_assignment_line >= target_write_trace_wait_line ||
      target_write_trace_validation_line <= target_write_trace_wait_line )); then
    printf '%s\n' 'FAIL: target-write trace path must be assigned before launch and validated after Mednafen exits' >&2
    exit 1
fi
code_ram_write_build_invalid_output=$(env -i "PATH=${PATH:-/usr/bin:/bin}" "HOME=${HOME:-.}" \
    FIRESTAFF_MEDNAFEN_SOURCE_ROOT="$repo" \
    FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_SUPPORT=2 \
    "$build_script" 2>&1) && {
    printf '%s\n' 'FAIL: PCE Fast code-bank RAM-write build support accepted a value other than 0 or 1' >&2
    exit 1
}
if [[ "$code_ram_write_build_invalid_output" != *'FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_SUPPORT must be 0 or 1'* ]]; then
    printf '%s\n' 'FAIL: invalid code-bank RAM-write build support lacked a precise diagnostic' >&2
    exit 1
fi
if [[ ! -f "$consumer_read_patch" ]] ||
   ! grep -Fq 'TheronPCECDTraceMainRAMConsumerRead' "$consumer_read_patch" ||
   ! grep -Fq 'physical_address >= 0x1f0000 && physical_address < 0x1f8000' "$consumer_read_patch" ||
   ! grep -Fq 'TheronPCECDMainRAMConsumerTraceLimit' "$consumer_read_patch" ||
   ! grep -Fq 'main_ram_consumer_read sequence=%u logical_address=%04x physical_address=%06x value=%02x reader_pc=%04x reader_physical_pc=%06x a=%02x x=%02x y=%02x sp=%02x p=%02x' "$consumer_read_patch" ||
   ! grep -Fq 'TheronPCECDTraceMainRAMConsumerWrite' "$consumer_read_patch" ||
   ! grep -Fq 'main_ram_target_write sequence=%u logical_address=%04x physical_address=%06x' "$consumer_read_patch" ||
   grep -Fq 'reader_physical_pc >= 0x1f0000 && reader_physical_pc < 0x1f8000' "$consumer_read_patch" ||
   grep -Fq 'writer_physical_pc >= 0x1f0000 && writer_physical_pc < 0x1f8000' "$consumer_read_patch" ||
   grep -Fq '\\\\n' "$consumer_read_patch"; then
    printf 'FAIL: consumer-read patch must retain bounded game-owned RAM provenance with real line-delimited output\n' >&2
    exit 1
fi
if ! grep -Fq 'spawn_consumer_read sequence=%u logical_address=%04x physical_address=%06x' "$consumer_read_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_SPAWN_CONSUMER_TRACE="$spawn_consumer_trace"' "$script" ||
   ! grep -Fq 'spawn_consumer_reads=%s' "$script"; then
    printf 'FAIL: capture must retain the disassembly-bound spawn consumer receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'spawn_consumer_registers sequence=%u pc=%04x physical_pc=%08x' "$irq2_patch" ||
   ! grep -Fq 'TheronIrq2TraceSpawnEntryB0E5SampleLimit = 256' "$irq2_patch" ||
   ! grep -Fq 'logical_pc == 0xb0e5 &&' "$irq2_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_SPAWN_REGISTER_TRACE="$spawn_register_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_SPAWN_REGISTER_SAMPLE_LIMIT=' "$script" ||
   ! grep -Fq 'spawn_register_samples=%s' "$script"; then
    printf 'FAIL: capture must retain disassembly-bound spawn register samples\n' >&2
    exit 1
fi
if ! grep -Fq 'mednafen_binary_md5=$(md5_file "$mednafen_bin")' "$script" ||
   ! grep -Fq 'could not hash the instrumented Mednafen binary' "$script" ||
   ! grep -Fq 'mednafen_binary_md5=%s' "$script"; then
    printf 'FAIL: capture receipt must bind the exact instrumented Mednafen binary\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_MEDNAFEN_HOME must name an existing Mednafen configuration directory' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_SDL_VIDEODRIVER' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_STARTUP_GRACE must be a positive integer' "$script" ||
   ! grep -Fq 'wait_for_trace_producer()' "$script" ||
   ! grep -Fq 'Mednafen did not produce an instrumented trace before host-input scheduling' "$script" ||
   ! grep -Fq 'host_key_schedule_seconds=$SECONDS' "$script" ||
   ! grep -Fq 'host_input_schedule_origin=trace_ready' "$script" ||
   ! grep -Fq 'cleanup_capture()' "$script" ||
   ! grep -Fq 'kill -TERM -- "-$mednafen_pid"' "$script" ||
   ! grep -Fq 'interrupted capture cannot leave Mednafen holding its home lock' "$script" ||
   ! grep -Fq 'configured home is an input-map template' "$script" ||
   ! grep -Fq 'could not prepare an isolated Mednafen capture home' "$script"; then
    printf 'FAIL: capture script must gate an explicit GUI input configuration\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_CAPTURE_HOST_KEY must name a supported PCE key' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY requires a non-dummy SDL video driver' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY requires THERON_MEDNAFEN_HOME with an explicit PCE input mapping' "$script" ||
   ! grep -Fq 'require_capture_profile_mappings()' "$script" ||
   ! grep -Fq 'does not retain a supported %s mapping' "$script" ||
   ! grep -Fq 'capture_host_code_for_mapping()' "$script" ||
   ! grep -Fq 'i:91) printf' "$script" ||
   ! grep -Fq 'i:32) printf' "$script" ||
   ! grep -Fq 'i:29) printf' "$script" ||
   ! grep -Fq 'ii:90) printf' "$script" ||
   ! grep -Fq 'ii:31) printf' "$script" ||
   ! grep -Fq 'ii:27) printf' "$script" ||
   ! grep -Fq 'i:54) printf' "$script" ||
   ! grep -Fq 'i:55) printf' "$script" ||
   ! grep -Fq 'ii:54) printf' "$script" ||
   ! grep -Fq 'ii:55) printf' "$script" ||
   ! grep -Fq 'capture_i_host_code' "$script" ||
   ! grep -Fq 'capture_ii_host_code' "$script" ||
   ! grep -Fq 'must retain RUN=40 and SELECT=43' "$script" ||
   ! grep -Fq 'set targetProcess to first application process whose unix id is $target_pid' "$script" ||
   ! grep -Fq 'cliclick "c:${host_focus_x},${host_focus_y}"' "$script" ||
   ! grep -Fq 'resolve_mednafen_ui_pid()' "$script" ||
   ! grep -Fq 'resolve_mednafen_ui_pid_with_retry()' "$script" ||
   ! grep -Fq 'activate_mednafen_ui_pid_with_retry()' "$script" ||
   ! grep -Fq 'mednafen_ui_pid=$(resolve_mednafen_ui_pid_with_retry "$mednafen_pid" || true)' "$script" ||
   ! grep -Fq 'activate_mednafen_ui_pid_with_retry "$mednafen_ui_pid"' "$script" ||
   grep -Fq 'pgrep -f "$mednafen_bin"' "$script" ||
   ! grep -Fq 'run|return) printf '\''%s'\'' 36 ;;' "$script" ||
   ! grep -Fq 'select) printf '\''%s'\'' 48 ;;' "$script" ||
   ! grep -Fq 'i) printf '\''%s'\'' "$capture_i_host_code" ;;' "$script" ||
   ! grep -Fq 'ii) printf '\''%s'\'' "$capture_ii_host_code" ;;' "$script" ||
   ! grep -Fq 'up) printf '\''%s'\'' "$capture_up_host_code" ;;' "$script" ||
   ! grep -Fq 'down) printf '\''%s'\'' "$capture_down_host_code" ;;' "$script" ||
   ! grep -Fq 'left) printf '\''%s'\'' "$capture_left_host_code" ;;' "$script" ||
   ! grep -Fq 'right) printf '\''%s'\'' "$capture_right_host_code" ;;' "$script" ||
   ! grep -Fq 'i:91) printf '\''%s'\'' KP_3 ;;' "$x11_keymap" ||
   ! grep -Fq 'ii:90) printf '\''%s'\'' KP_2 ;;' "$x11_keymap" ||
   ! grep -Fq 'up:82) printf '\''%s'\'' Up ;;' "$x11_keymap" ||
   ! grep -Fq 'i:32) printf '\''%s'\'' 3 ;;' "$x11_keymap" ||
   ! grep -Fq 'right:7) printf '\''%s'\'' d ;;' "$x11_keymap" ||
   ! grep -Fq 'Linux host input requires SDL_VIDEODRIVER=x11 and xdotool' "$script" ||
   ! grep -Fq 'Linux X11 host input requires THERON_CAPTURE_INPUT_ROUTE=pid' "$script" ||
   ! grep -Fq 'resolve_mednafen_window_id_with_retry "$mednafen_ui_pid"' "$script" ||
   ! grep -Fq 'xdotool search --onlyvisible --pid "$target_pid"' "$script" ||
   ! grep -Fq 'xdotool getwindowpid "$candidate_window"' "$script" ||
   ! grep -Fq 'xdotool windowfocus --sync "$mednafen_window_id"' "$script" ||
   ! grep -Fq 'xdotool getwindowfocus 2>/dev/null' "$script" ||
   ! grep -Fq 'grab_binding=$(capture_profile_binding command.toggle_grab)' "$script" ||
   ! grep -Fq 'theron_x11_chord_for_sdl_binding "$grab_binding"' "$script" ||
   ! grep -Fq 'xdotool key "$input_grab_x11_chord"' "$script" ||
   ! grep -Fq 'xdotool keydown "$host_key_current_code"' "$script" ||
   ! grep -Fq 'xdotool keyup "$host_key_current_code"' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_INPUT_ROUTE must be pid or global_hid' "$script" ||
   ! grep -Fq 'quartz_arguments+=(--global-hid)' "$script" ||
   ! grep -Fq 'if [[ "$input_route" == global_hid ]]; then' "$script" ||
   ! grep -Fq 'Quartz helper did not attest requested key delivery' "$script" ||
   ! grep -Fq 'host input requires Swift and the checked-in Quartz keypair helper' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_REPEATS must be a positive integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_DELAY must be a non-negative integer' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_SEQUENCE must be comma-separated PCE key@seconds entries' "$script" ||
   ! grep -Fq 'if [[ -n "$host_key_holds" && -z "$host_key_sequence" ]]; then' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLDS requires THERON_CAPTURE_HOST_KEY_SEQUENCE' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLDS must be comma-separated positive seconds' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLDS must match the host-key sequence length' "$script" ||
   ! grep -Fq 'host_key_current_hold=${host_key_sequence_holds[$((host_key_attempt - 1))]}' "$script" ||
   ! grep -Fq 'sleep "$host_key_current_hold"' "$script" ||
   ! grep -Fq 'quartz_arguments=("$host_key_current_code" "$host_key_current_hold" "$mednafen_ui_pid")' "$script" ||
   ! grep -Fq 'requested_host_key_holds_seconds=%s' "$script" ||
   ! grep -Eq 'THERON_CAPTURE_HOST_KEY_HOLDS=4([[:space:]]|$)' "$repo/docs/theron/THERON_MAC_SDL_MEDNAFEN_LOCAL.md" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_SEQUENCE times must be ordered' "$script" ||
   ! grep -Fq 'host_key_sequence_codes+=("$(capture_host_key_for_label "$host_key_sequence_label")")' "$script" ||
   ! grep -Fq 'requested_host_key_sequence=%s' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLD must be a positive integer' "$script" ||
   ! grep -Fq 'requested host key was not observed by Mednafen SDL dispatch' "$script"; then
    printf 'FAIL: capture script must keep the opt-in macOS Return focus/input gate\n' >&2
    exit 1
fi
if ! grep -Fq 'THERON_CAPTURE_REPLAY_INPUT_SCRIPT cannot be combined with host-key input' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_REPLAY_INPUT_SCRIPT must be comma-separated PCE key@frame or key@frame:hold entries' "$script" ||
   ! grep -Fq 'MEDNAFEN_BIN lacks the required Firestaff Theron scripted-PCE-input producer' "$script" ||
   ! grep -Fq "grep -aFq 'FIRESTAFF_THERON_REPLAY_INPUT_SCRIPT' \"\$mednafen_bin\"" "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_REPLAY_INPUT_SCRIPT="$replay_input_script"' "$script" ||
   ! grep -Fq 'scripted_pce_input_events=%s' "$script" ||
   ! grep -Fq 'input_delivery=scripted_pce_replay' "$script" ||
   ! grep -Fq 'scripted_pce_input_plan=%s' "$script"; then
    printf 'FAIL: capture script must retain explicit scripted-PCE-input provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'autoload_movie=${THERON_CAPTURE_AUTOLOAD_MOVIE:-}' "$script" ||
   ! grep -Fq 'autoload_state_magic=$(od -An -tx1 -N4 "$autoload_state"' "$script" ||
   ! grep -Fq 'autoload_state_md5=$(md5_file "$autoload_state")' "$script" ||
   ! grep -Fq 'autoload_state_md5=%s' "$script" ||
   ! grep -Fq 'points to HUBM SRAM, not a Mednafen savestate' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_AUTOLOAD_MOVIE must name an existing Mednafen movie file' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_AUTOLOAD_STATE and THERON_CAPTURE_AUTOLOAD_MOVIE cannot be combined' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_AUTOLOAD_MOVIE="$autoload_movie"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_AUTOLOAD_MOVIE' "$state_autoload_patch"; then
    printf 'FAIL: capture script and Mednafen patch must retain authentic movie-replay provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'profile_sram_manifest="${trace}.profile-sram"' "$script" ||
   ! grep -Fq "find \"\$home_dir/sav\" -type f -name '*.sav' -size 2048c -print0" "$script" ||
   ! grep -Fq 'configured_profile_sram_files=%s' "$script" ||
   ! grep -Fq 'sed '\''s/^/configured_profile_sram=/'\'' "$profile_sram_manifest"' "$script"; then
    printf '%s\n' 'FAIL: transition receipt must hash valid SRAM files copied into the isolated profile without claiming they were loaded' >&2
    exit 1
fi
if ! grep -Fq 'post_dungeon_ordinal=${THERON_CAPTURE_POST_DUNGEON_ORDINAL:-}' "$script" ||
   ! grep -Fq 'THERON_CAPTURE_POST_DUNGEON_ORDINAL must be 0..6' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_POST_DUNGEON_ORDINAL="$post_dungeon_ordinal"' "$script"; then
    printf 'FAIL: capture script must retain bounded post-dungeon research provenance\n' >&2
    exit 1
fi
if ! grep -Fq 'save_manager_code_dump="${trace}.save-manager-code"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_SAVE_MANAGER_CODE_DUMP="$save_manager_code_dump"' "$script" ||
   ! grep -Fq 'SetMPR(6, 0x6d)' "$save_manager_dump_patch" ||
   ! grep -Fq 'for(unsigned i = 0; i < 8192; i++)' "$save_manager_dump_patch" ||
   ! grep -Fq 'SetMPR(6, original_mpr6)' "$save_manager_dump_patch" ||
   ! grep -Fq 'mednafen_1.32.1_theron_save_manager_code_dump.patch' "$build_script"; then
    printf 'FAIL: authentic read-only save-manager code capture was not retained\n' >&2
    exit 1
fi
if ! grep -Fq 'replay_post_dungeon_overlay=${THERON_CAPTURE_REPLAY_POST_DUNGEON_OVERLAY:-0}' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_POST_DUNGEON_OVERLAY_ISO="$post_dungeon_overlay_iso"' "$script" ||
   ! grep -Fq 'post_dungeon_overlay_replay=%s' "$script" ||
   ! grep -Fq 'hash == 0x337de858U' "$post_dungeon_patch_file" ||
   ! grep -Fq 'PC = 0xdf29' "$post_dungeon_patch_file"; then
    printf 'FAIL: bounded real-US post-dungeon overlay replay was not retained\n' >&2
    exit 1
fi
if ! grep -Fq 'dungeon_bank_20da=%s' "$script" ||
   ! grep -Fq 'dungeon_bank_20db=%s' "$script" ||
   ! grep -Fq 'od -An -tx1 -j 218 -N 1 "$transition_ram_snapshot"' "$script" ||
   ! grep -Fq 'od -An -tx1 -j 219 -N 1 "$transition_ram_snapshot"' "$script"; then
    printf 'FAIL: capture receipt must retain original dungeon-bank selector bytes\n' >&2
    exit 1
fi
if ! grep -Fq 'ram_203f=%s' "$script" ||
   ! grep -Fq 'ram_2031=%s' "$script" ||
   ! grep -Fq 'ram_2040=%s' "$script" ||
   ! grep -Fq 'ram_2041=%s' "$script" ||
   grep -Fq 'party_direction_203f=%s' "$script" ||
   grep -Fq 'current_level_2031=%s' "$script" ||
   grep -Fq 'party_x_2040=%s' "$script" ||
   grep -Fq 'party_y_2041=%s' "$script" ||
   ! grep -Fq 'runtime_byte_2038=%s' "$script"; then
    printf '%s\n' 'FAIL: live transition receipt must keep unjoined RAM bytes semantically opaque' >&2
    exit 1
fi
party_ram_trace_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_main_ram_snapshot.patch"
if ! grep -Fq 'TheronTraceInstructionPhysicalPC' "$party_ram_trace_patch" ||
   ! grep -Fq 'const unsigned offset = address & 0x1FFF' "$party_ram_trace_patch" ||
   ! grep -Fq 'static DECLFW(BaseRAMWrite_Mirrored)' "$party_ram_trace_patch" ||
   [[ $(grep -Fc '+ TheronTracePartyRAMWrite(A, V);' "$party_ram_trace_patch") -ne 2 ]] ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_PARTY_RAM_TRACE="$pce_fast_party_ram_trace"' "$script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_instruction_pc_trace.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_party_trace_budget.patch' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_main_ram_consumer_read.patch' "$build_script"; then
    printf '%s\n' 'FAIL: Theron party-RAM writer tracing must cover mirrored BaseRAM writes with current HuC6280 PC' >&2
    exit 1
fi
instruction_pc_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_instruction_pc_trace.patch"
party_ram_budget_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_party_trace_budget.patch"
party_ram_consumer_read_patch="$repo/scripts/mednafen_1.32.1_theron_pce_fast_main_ram_consumer_read.patch"
if [[ ! -f "$party_ram_consumer_read_patch" ]] ||
   ! grep -Fq 'TheronTraceMainRAMConsumerRead(A, physical_address, value)' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'static unsigned per_offset[8192] = {};' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_CONSUMER_SAMPLE_LIMIT' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'target_range_sample_count >= sample_limit' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'main_ram_sample_count >= sample_limit' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'sample_class=%s' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'main_ram_consumer_truncated=target_range_sample_limit limit=%u' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'main_ram_consumer_truncated=main_ram_sample_limit limit=%u' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'reason=per_offset_sample_limit offset=%04x limit=16' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_TRACE_ERROR error=write' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_TRACE_ERROR error=truncation' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'main_ram_consumer_target_range_reads=%s' "$script" ||
   ! grep -Fq 'main_ram_consumer_target_range_truncated=%s' "$script" ||
   ! grep -Fq 'main_ram_consumer_truncated=%s' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_TRACE_ERROR' "$script" ||
   ! grep -Fq 'source=mednafen-pce-fast-main-ram-consumer-read event_sequence_source=TheronTraceCDSharedEventSequence' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'target_range_enabled = true' "$party_ram_consumer_read_patch" ||
   ! grep -Fq '!target_range_enabled || physical_address < target_range_first' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'const bool target_range_read = target_range_enabled' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'main_ram_consumer_read sequence=%u event_sequence=%u' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'const bool target_writer_read = TheronTraceInstructionPC >= 0xEA80' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'physical_address < 0x1F0000 || physical_address >= 0x1F8000' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'offset = physical_address & 0x1FFF' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'reader_physical_pc=%06x reader_code_bytes=%s' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'HuCPU.FastPageR[' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'char reader_code_bytes[17] = "unavailable"' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'for(unsigned i = 0; i < 8; i++)' "$party_ram_consumer_read_patch"; then
    printf '%s\n' 'FAIL: pce_fast main-RAM consumer trace must sample bounded reads with source-positioned instruction bytes' >&2
    exit 1
fi
if ! awk '
    /if\(target_range_read && target_range_sample_count >= sample_limit\)/ { target_limit=NR }
    /main_ram_consumer_truncated=target_range_sample_limit/ { target_marker=NR }
    /target_range_sample_count\+\+/ { target_increment=NR }
    /if\(!target_range_read && main_ram_sample_count >= sample_limit\)/ { main_limit=NR }
    /main_ram_consumer_truncated=main_ram_sample_limit limit=/ { main_marker=NR }
    /main_ram_sample_count\+\+/ { main_increment=NR }
    /per_offset\[offset\] >= 16/ { offset_limit=NR }
    /main_ram_consumer_truncated=main_ram_sample_limit reason=per_offset_sample_limit/ { offset_marker=NR }
    /per_offset\[offset\]\+\+/ { offset_increment=NR }
    END {
        if (!(target_limit < target_marker && target_marker < target_increment &&
              main_limit < main_marker && main_marker < main_increment &&
              offset_limit < offset_marker && offset_marker < offset_increment)) exit 1
    }
' "$party_ram_consumer_read_patch"; then
    printf '%s\n' 'FAIL: consumer truncation markers must report a dropped read after a cap, not merely reaching it' >&2
    exit 1
fi
if ! grep -Fq 'if(HuCPU.MPR[1] == 0xF8 && offset < 0x100)' "$party_ram_trace_patch" ||
   ! grep -Fq 'IsSGX || BaseRAM[offset] == value' "$party_ram_trace_patch" ||
   ! grep -Fq 'per_offset[offset] >= 4096' "$party_ram_budget_patch" ||
   ! grep -Fq 'TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r)' "$instruction_pc_patch" ||
   ! grep -Fq 'TheronTraceDirectMainRAMWrite(uint32 offset, uint8 old_value, uint8 value)' "$party_ram_trace_patch"; then
    printf '%s\n' 'FAIL: direct zero-page BaseRAM stores must retain old/new values and instruction-PC provenance' >&2
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
if ! grep -Fq 'raw sector span lacks input, CDIRQ, and authenticated CD->RAM origin receipts' "$script" ||
   ! grep -Fq 'authenticated_cd_ram_receipts=%s' "$script" ||
   ! grep -Fq 'pce_cd_(origin_ram_receipt|fifo_origin_ram_receipt|origin_main_ram_receipt|fifo_origin_main_ram_receipt)' "$script"; then
    printf 'FAIL: capture script must gate raw sectors on authenticated CD->RAM origin evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'host_key_events=%s' "$script" ||
   ! grep -Fq 'host_sdl_events=%s' "$script" ||
   ! grep -Fq 'host_sdl_event_types=%s' "$script" ||
   ! grep -Fq 'host_window_events=%s' "$script" ||
   ! grep -Fq 'host_focus_state_events=%s' "$script" ||
   ! grep -Fq 'host_input_target_pid=%s' "$script" ||
   ! grep -Fq 'host_input_focus=screen_click:%s,%s' "$script" ||
   ! grep -Fq 'host_input_delivery=quartz_%s_key_down_up' "$script" ||
   ! grep -Fq 'host_input_delivery_attempts=%s' "$script" ||
   ! grep -Fq 'wait_for_host_key_events()' "$script" ||
   ! grep -Fq 'Mednafen did not observe preflight key-down attempt %s after host delivery' "$script" ||
   ! grep -Fq 'host_input_backend=%s' "$script" ||
   ! grep -Fq 'host_input_focus=x11_window:%s' "$script" ||
   ! grep -Fq 'host_input_delivery=xdotool_x11_key_down_up' "$script" ||
   ! grep -Fq 'if (( host_key_attempt <= 2 ))' "$script" ||
   ! grep -Fq 'wait_for_host_key_events "$input_trace" "$((host_key_attempt * 2 - 1))" 40' "$script" ||
   ! grep -Fq 'trace_input_order_receipt()' "$script" ||
   ! grep -Fq 'pce_input_transactions_after_first_host' "$script" ||
   ! grep -Fq 'host_input_order=after_last_observed_pce_input_poll' "$script" ||
   ! grep -Fq 'scsi_read_commands=%s' "$script" ||
   ! grep -Fq 'scsi_read_sector_bindings=%s' "$script" ||
   ! grep -Fq 'byte_exact_fifo_ram_destinations=%s' "$script" ||
   ! grep -Fq 'adpcm_fifo_reads=%s' "$script" ||
   ! grep -Fq 'adpcm_ram_writes=%s' "$script" ||
   ! grep -Fq 'adpcm_ram_read_prepares=%s' "$script" ||
   ! grep -Fq 'adpcm_cpu_reads=%s' "$script" ||
   ! grep -Fq 'byte_exact_origin_ram_receipts=%s' "$script" ||
   ! grep -Fq 'game_main_ram_e009_dispatches=%s' "$script" ||
   ! grep -Fq 'System Card wait; host_keys=%s input=%s input_after_first_host=%s irq=%s authenticated_cd_ram=%s' "$script" ||
   ! grep -Fq 'loader reached authentic raw sectors but no authenticated CD->RAM origin receipt was observed' "$script" ||
   ! grep -Fq 'main_ram_e009_dispatches=%s' "$script" ||
   ! grep -Fq 'main_ram_e009_register_writes=%s' "$script" ||
   ! grep -Fq 'dynamic receipts absent; host_keys=%s input=%s irq=%s authenticated_cd_ram=%s' "$script"; then
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
   ! grep -Fq 'vdc_io_writes=%s' "$script" ||
   ! grep -Fq 'transition=missing' "$script" ||
   ! grep -Fq 'transition=observed' "$script"; then
    printf 'FAIL: capture script must publish an observed-or-missing transition receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'record_c3a0_window=%u' "$irq2_patch" ||
   ! grep -Fq 'logical_pc >= 0xc3a0 && logical_pc <= 0xc429' "$irq2_patch" ||
   ! grep -Fq 'c3a0_a9=%02x c3a0_ab=%02x c3a0_ac=%02x c3a0_2998=%02x c3a0_299c=%02x' "$irq2_patch" ||
   ! grep -Fq 'record_c3a0_window_seen' "$repo/src/theron/theron_v1_mednafen_spawn_consumer_trace.c"; then
    printf 'FAIL: live capture must preserve the source-locked C3A0 caller window and table inputs\n' >&2
    exit 1
fi
if ! grep -Fq 'rng_generator_context_trace="${trace}.rng-generator-context"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_RNG_GENERATOR_CONTEXT_TRACE="$rng_generator_context_trace"' "$script" ||
   ! grep -Fq 'rng_generator_contexts=%s' "$script"; then
    printf 'FAIL: live capture must retain the per-RNG authentic 8 KiB generator context\n' >&2
    exit 1
fi
if ! grep -Fq 'stage2_system_card_receipt="${trace}.stage2-system-card"' "$script" ||
   ! grep -Fq 'verify_theron_stage2_system_card_call_trace.sh' "$script" ||
   ! grep -Fq 'Absence is expected for captures that do not reach this exact stage.' "$script"; then
    printf 'FAIL: capture script must preserve a separate fail-closed stage-two loader receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'instrument_mednafen_theron_pce_fast_a1a9_init_trace.py' "$build_script" ||
   ! grep -Fq 'b1 = RdAtPC();' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_a1a9_init_trace.py" ||
   ! grep -Fq 'opcode=%02x bytes=%02x' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_a1a9_init_trace.py" ||
   ! grep -Fq 'HuCPU.A, X_local, Y_local, HuCPU.S, P_local, b1' \
       "$repo/scripts/instrument_mednafen_theron_pce_fast_a1a9_init_trace.py" ||
   ! grep -Fq 'pce_fast_a1a9_init_trace="${trace}.a1a9-init"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_A1A9_INIT_TRACE="$pce_fast_a1a9_init_trace"' "$script" ||
   ! grep -Fq '"$pce_fast_a1a9_init_trace"' "$script"; then
    printf '%s\n' 'FAIL: A1A9 trace must use live HuC6280 register locals and preserve its capture sidecar' >&2
    exit 1
fi
if ! grep -Fq 'resolve_mednafen_window_id_with_retry "$mednafen_ui_pid"' "$script" ||
   ! grep -Fq 'for ((attempt = 0; attempt < attempts; ++attempt)); do' "$script" ||
   ! grep -Fq 'xdotool search --onlyvisible --pid "$target_pid"' "$script" ||
   ! grep -Fq 'owner_pid=$(xdotool getwindowpid "$candidate_window"' "$script"; then
    printf '%s\n' 'FAIL: X11 capture must retry SDL window discovery after the Mednafen process starts' >&2
    exit 1
fi
window_retry_harness=$(mktemp -d "$repo/build/theron-x11-window-retry.XXXXXX")
sed -n '/^resolve_mednafen_window_id_with_retry()/,/^}/p' "$script" >"$window_retry_harness/resolver.sh"
cat >"$window_retry_harness/xdotool" <<'MOCK_XDOTOOL'
#!/usr/bin/env bash
set -euo pipefail
state_file=${THERON_X11_MOCK_STATE:?}
case "$1" in
    search)
        count=0
        [[ ! -f "$state_file" ]] || count=$(<"$state_file")
        count=$((count + 1))
        printf '%s\n' "$count" >"$state_file"
        if (( count >= 3 )); then printf '%s\n' 4194305; fi
        ;;
    getwindowpid)
        if [[ "$2" == 4194305 ]]; then printf '%s\n' "${THERON_X11_MOCK_WINDOW_PID:?}"; fi
        ;;
    *) exit 2 ;;
esac
MOCK_XDOTOOL
chmod +x "$window_retry_harness/xdotool"
if ! PATH="$window_retry_harness:$PATH" THERON_X11_MOCK_STATE="$window_retry_harness/searches" \
    THERON_X11_MOCK_WINDOW_PID=7319 bash -c 'source "$1"; resolve_mednafen_window_id_with_retry 7319 5' \
    _ "$window_retry_harness/resolver.sh" | grep -Fxq 4194305; then
    printf '%s\n' 'FAIL: X11 window discovery did not retry until SDL created the owned window' >&2
    exit 1
fi
if PATH="$window_retry_harness:$PATH" THERON_X11_MOCK_STATE="$window_retry_harness/foreign-searches" \
    THERON_X11_MOCK_WINDOW_PID=7320 bash -c 'source "$1"; resolve_mednafen_window_id_with_retry 7319 3' \
    _ "$window_retry_harness/resolver.sh" >/dev/null 2>&1; then
    printf '%s\n' 'FAIL: X11 window discovery accepted a window owned by another process' >&2
    exit 1
fi

output=$(env -u MEDNAFEN_BIN -u THERON_US_CUE -u THERON_CUE -u THERON_SYSTEM_CARD \
    -u THERON_LIVE_TRACE_OUTPUT "$script")
if [[ "$output" != 'SKIP: MEDNAFEN_BIN, THERON_US_CUE/THERON_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required' ]]; then
    printf 'FAIL: capture script did not reject unstaged live inputs\n' >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

printf 'PASS: live capture script requires explicit authentic inputs\n'
