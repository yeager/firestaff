#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/capture_theron_mednafen_live_trace.sh
quartz_helper=$repo/scripts/send_theron_macos_quartz_keypair.swift
quartz_grab_helper=$repo/scripts/send_theron_macos_quartz_chord.swift
runtime_verifier=$repo/scripts/verify_theron_mednafen_sdl2_runtime.sh
build_script=$repo/scripts/build_mednafen_theron_irq2_trace.sh
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

if [[ ! -x "$script" ]]; then
    printf 'FAIL: live Mednafen capture script is not executable\n' >&2
    exit 1
fi
if [[ ! -x "$scripted_input_consumption_verifier" ]]; then
    printf '%s\n' 'FAIL: scripted-input consumption verifier is not executable' >&2
    exit 1
fi
mkdir -p "$repo/build"
input_test_dir=$(mktemp -d "$repo/build/theron-input-consumption.XXXXXX")
trap 'rm -rf -- "$input_test_dir"' EXIT
cat >"$input_test_dir/consumed.trace" <<'THERON_CONSUMED_INPUT'
source=mednafen-pce-scripted-input
scripted_pce_input_event frame=9600 key=run mask=0008 hold=90
scripted_pce_input_apply frame=9600 physical=0000 scripted=0008 combined=0008
pce_input_read cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0
THERON_CONSUMED_INPUT
consumption_receipt=$("$scripted_input_consumption_verifier" \
    "$input_test_dir/consumed.trace" run@9600:90 131072)
if [[ "$consumption_receipt" != *'event_frames_with_apply=1'* ||
      "$consumption_receipt" != *'event_frames_followed_by_controller_read=1'* ||
      "$consumption_receipt" != *'controller_poll_boundary=verified'* ||
      "$consumption_receipt" != *'game_or_non_system_card_poll_boundary=observed'* ]]; then
    printf 'FAIL: post-event controller read was not verified:\n%s\n' \
        "$consumption_receipt" >&2
    exit 1
fi
cat >"$input_test_dir/system-card-only.trace" <<'THERON_SYSTEM_CARD_INPUT'
scripted_pce_input_event frame=9600 key=run mask=0008 hold=90
scripted_pce_input_apply frame=9600 physical=0000 scripted=0008 combined=0008
pce_input_read cpu_pc=e4c8 register=1000 raw=0008 sel=0 clr=0 index=0
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
pce_input_read cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0
scripted_pce_input_event frame=12 key=ii mask=0002 hold=1
scripted_pce_input_apply frame=12 physical=0008 scripted=0002 combined=000a
pce_input_read cpu_pc=8123 register=1000 raw=000a sel=0 clr=0 index=0
THERON_MULTI_EVENT_INPUT
"$scripted_input_consumption_verifier" \
    "$input_test_dir/multiple-events.trace" run@10,ii@12 4 >/dev/null
cat >"$input_test_dir/same-frame-events.trace" <<'THERON_SAME_FRAME_INPUT'
scripted_pce_input_event frame=20 key=run mask=0008 hold=1
scripted_pce_input_event frame=20 key=ii mask=0002 hold=1
scripted_pce_input_apply frame=20 physical=0000 scripted=000a combined=000a
pce_input_read cpu_pc=8123 register=1000 raw=000a sel=0 clr=0 index=0
THERON_SAME_FRAME_INPUT
"$scripted_input_consumption_verifier" \
    "$input_test_dir/same-frame-events.trace" run@20,ii@20 2 >/dev/null
cat >"$input_test_dir/event-at-read-cap.trace" <<'THERON_CAPPED_INPUT'
pce_input_read cpu_pc=8123 register=1000 raw=0000 sel=0 clr=0 index=0
pce_input_read cpu_pc=8123 register=1000 raw=0000 sel=0 clr=0 index=0
scripted_pce_input_event frame=12 key=run mask=0008 hold=1
scripted_pce_input_apply frame=12 physical=0000 scripted=0008 combined=0008
THERON_CAPPED_INPUT
if "$scripted_input_consumption_verifier" \
    "$input_test_dir/event-at-read-cap.trace" run@12 2 \
    >"$input_test_dir/capped.stdout" 2>"$input_test_dir/capped.stderr"; then
    printf '%s\n' 'FAIL: scripted input at the read-trace cap was accepted without a later CPU read' >&2
    exit 1
fi
if ! grep -Fq 'final scripted event frame has no subsequent controller-port read' \
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
if ! grep -Fq 'final scripted event frame has no subsequent controller-port read' \
    "$input_test_dir/unconsumed.stderr"; then
    printf '%s\n' 'FAIL: unconsumed scripted input rejection lacked a precise diagnostic' >&2
    exit 1
fi
cat >"$input_test_dir/no-apply.trace" <<'THERON_NO_APPLY_INPUT'
scripted_pce_input_event frame=30 key=run mask=0008 hold=1
pce_input_read cpu_pc=8123 register=1000 raw=0008 sel=0 clr=0 index=0
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
route_output=$(THERON_CAPTURE_MENU_ROUTE=drator-generator \
    THERON_CAPTURE_REPLAY_INPUT_SCRIPT=run@9600:90 bash "$script" 2>&1)
if [[ "$route_output" != *'SKIP: MEDNAFEN_BIN, THERON_US_CUE/THERON_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required'* ]]; then
    printf 'FAIL: valid cold-start route preflight did not reach normal capture argument validation\n' >&2
    exit 1
fi
if ! grep -Fq 'mkdir -p "$home_dir/sav"' "$script" ||
   ! grep -Fq -- '-filesys.path_sav "$home_dir/sav"' "$script" ||
   ! grep -Fq 'mkdir -p "$home_dir/mcs"' "$script" ||
   ! grep -Fq -- '-filesys.path_state "$home_dir/mcs"' "$script"; then
    printf '%s\n' 'FAIL: isolated captures must bind backup RAM and state files to the private capture home' >&2
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
   ! grep -Fq 'input_trace_limit_default=65536' "$script" ||
   ! grep -Fq 'input_trace_limit_default=1048576' "$script" ||
   ! grep -Fq '|| -n "$host_key" || -n "$host_key_sequence"' "$script" ||
   ! grep -Fq 'input_trace_limit < 65536 || input_trace_limit > 1048576' "$script" ||
   ! grep -Fq 'verify_theron_scripted_input_consumption.sh' "$script" ||
   ! grep -Fq 'event_frames_followed_by_controller_read' "$scripted_input_consumption_verifier" ||
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
   ! grep -Fq 'index($_, chr(92) . chr(92) . "n")' "$script" ||
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
    swift_tmp_root=${TMPDIR:-${RUNNER_TEMP:-/private/tmp}}
    swift_module_cache=$(mktemp -d "$swift_tmp_root/firestaff-theron-swift-module-cache.XXXXXX")
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
   ! grep -Fq -- 'mednafen_module=%s' "$script" ||
   ! grep -Fq -- 'capture_arcadecard_setting=pce.arcadecard' "$script" ||
   ! grep -Fq -- 'capture_arcadecard_setting=pce_fast.arcadecard' "$script" ||
   ! grep -Fq -- 'capture_cdbios_setting=pce_fast.cdbios' "$script" ||
   ! grep -Fq -- 'capture_input_port_setting=pce.input.port1' "$script" ||
   ! grep -Fq -- 'capture_input_port_setting=pce_fast.input.port1' "$script" ||
   ! grep -Fq -- 'capture_input_args=("-$capture_input_port_setting" gamepad)' "$script" ||
   ! grep -Fq -- 'capture_input_args+=(' "$script" ||
   ! grep -Fq -- '"-pce_fast.input.port1.gamepad.$key"' "$script" ||
   ! grep -Fq -- '"keyboard 0x0 $pce_scancode"' "$script" ||
   ! grep -Fq -- '"${capture_input_args[@]}"' "$script"; then
    printf 'FAIL: capture script must force the PCE module and disable unrelated Arcade Card emulation\n' >&2
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
if ! grep -Fq 'source=mednafen-pce-fast-instrumented-input' "$script" ||
   ! grep -Fq 'scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch' "$repo/scripts/build_mednafen_theron_irq2_trace.sh" ||
   ! grep -Fq 'static void TheronPCEFastInputTraceRead' "$repo/scripts/mednafen_1.32.1_theron_pce_fast_input_trace.patch"; then
    printf '%s\n' 'FAIL: pce_fast capture must retain source- and bus-bound input traces' >&2
    exit 1
fi
if ! grep -Fq 'if [[ "$capture_mednafen_module" == pce ]]; then' "$script" ||
   ! grep -Fq 'require_snapshot_size "$pce_fast_main_ram_snapshot" 8192' "$script" ||
   ! grep -Fq 'pce_fast_main_ram_snapshot_bytes=8192' "$script" ||
   ! grep -Fq 'PCE Fast capture must not emit PCE-only VDC snapshots or traces' "$script" ||
   ! grep -Fq 'vdc_io_writes=unavailable' "$script" ||
   ! grep -Fq 'vdc_io_trace_limit=unavailable' "$script" ||
   ! grep -Fq 'transition_ram_snapshot=$pce_fast_main_ram_snapshot' "$script" ||
   ! grep -Fq 'party_y_2041=$(od -An -tx1 -j 65 -N 1 "$transition_ram_snapshot"' "$script"; then
    printf '%s\n' 'FAIL: capture snapshots must match the selected Mednafen core' >&2
    exit 1
fi
if ! grep -Fq 'theron_input_read_count < theron_input_read_trace_limit' "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch" ||
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
if ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE="$main_ram_loader_trace"' "$script" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE="$main_ram_consumer_trace"' "$script" ||
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
   ! grep -Fq 'main_ram_target_reads=%s' "$script" ||
   ! grep -Fq 'main_ram_target_writes=%s' "$script"; then
    printf 'FAIL: capture script must retain the post-$3800 TII producer receipt\n' >&2
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
   ! grep -Fq 'THERON_CAPTURE_HOST_KEY_HOLDS=4,1,1' "$repo/docs/THERON_MAC_SDL_MEDNAFEN_LOCAL.md" ||
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
   ! grep -Fq 'autoload_state_magic=$(dd if="$autoload_state" bs=1 count=4' "$script" ||
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
if ! grep -Fq 'party_direction_203f=%s' "$script" ||
   ! grep -Fq 'current_level_2031=%s' "$script" ||
   ! grep -Fq 'party_x_2040=%s' "$script" ||
   ! grep -Fq 'party_y_2041=%s' "$script" ||
   ! grep -Fq 'runtime_byte_2038=%s' "$script"; then
    printf '%s\n' 'FAIL: live transition receipt must preserve raw party-position provenance' >&2
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
   ! grep -Fq 'count >= 65536 || per_offset[offset] >= 16' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'physical_address < 0x1F0000 || physical_address >= 0x1F8000' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'offset = physical_address & 0x1FFF' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'reader_physical_pc=%06x reader_code_bytes=%s' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'HuCPU.FastPageR[' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'char reader_code_bytes[17] = "unavailable"' "$party_ram_consumer_read_patch" ||
   ! grep -Fq 'for(unsigned i = 0; i < 8; i++)' "$party_ram_consumer_read_patch"; then
    printf '%s\n' 'FAIL: pce_fast main-RAM consumer trace must sample bounded reads with source-positioned instruction bytes' >&2
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
if ! grep -Fq 'resolve_mednafen_window_id_with_retry "$mednafen_ui_pid"' "$script" ||
   ! grep -Fq 'for ((attempt = 0; attempt < attempts; ++attempt)); do' "$script" ||
   ! grep -Fq 'xdotool search --onlyvisible --pid "$target_pid"' "$script" ||
   ! grep -Fq 'owner_pid=$(xdotool getwindowpid "$candidate_window"' "$script"; then
    printf '%s\n' 'FAIL: X11 capture must retry SDL window discovery after the Mednafen process starts' >&2
    exit 1
fi
window_retry_harness=$(mktemp -d "${TMPDIR:-/dev/shm}/theron-x11-window-retry.XXXXXX")
trap 'rm -rf "$window_retry_harness"' EXIT
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
