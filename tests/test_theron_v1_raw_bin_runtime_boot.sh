#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQUS02.bin"
expected_md5=f23601102138f87c33025877767ebf76

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron USA raw Track 02 BIN is not staged\n'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/build/test-scratch"}
mkdir -p "$scratch_root"
output=$(mktemp "$scratch_root/firestaff-theron-raw-bin.XXXXXX")
us_only_root=$(mktemp -d "$scratch_root/firestaff-theron-us-only.XXXXXX")
trap 'rm -f "$output"; rm -rf "$us_only_root"' EXIT
ln -s "$track02" "$us_only_root/TQUS02.bin"

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --theron-native us \
    --data-dir "$data_root" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,down,down,down,down,down,down,enter,down,enter,tab' \
    --debug \
    --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 1,0,0 \
    --boot-probe-expect-champions 2 \
    --boot-probe-expect-asset-md5 "$expected_md5" \
    --boot-probe-expect-startup-active 0 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-runtime' "$output" ||
   ! grep -Fq 'levelLoaded=1' "$output" ||
   ! grep -Fq 'party=1,0,0 champions=2' "$output" ||
   grep -Fq 'theronSourceObjects=0' "$output" ||
   ! grep -Eq 'theronSourceObjects=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronDungeonLevelsLoaded=4' "$output" ||
   ! grep -Fq 'theronDungeonSourceHeaders=4' "$output" ||
   ! grep -Eq 'theronDungeonSourceNonzeroTiles=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronActiveChampion=1' "$output" ||
   ! grep -Fq 'theronTrack01CddaReady=0' "$output" ||
   ! grep -Fq 'theronSpawnSourceAuthenticated=1' "$output" ||
   ! grep -Fq 'theronSpawnSourceVariant=2' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameBanks=7' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameVariant=2' "$output" ||
   ! grep -Fq 'theronTrack19NameBankReady=1' "$output" ||
   ! grep -Fq 'theronTrack19NameVariant=2' "$output" ||
   ! grep -Fq 'theronTrack19ItemMappingProven=1' "$output" ||
   grep -Eiq 'System Card|PCE Fast|Mednafen' "$output" ||
   grep -Fq 'deterministic fallback assets' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron USA raw BIN did not reach the native source-backed runtime route' >&2
    exit 1
fi

startup_script='enter,enter,down,down,down,down,down,down,enter,down,enter,tab'
run_pose_probe() {
    local destination=$1
    local script=$2
    local expected_party=$3
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --width 320 --height 224 \
        --game theron \
        --theron-native us \
        --data-dir "$data_root" \
        --boot-probe \
        --boot-probe-frames 0 \
        --script "$script" \
        --debug \
        --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-party "$expected_party" \
        --boot-probe-expect-champions 2 \
        --boot-probe-expect-asset-md5 "$expected_md5" \
        --boot-probe-expect-startup-active 0 \
        --duration 0 >"$destination" 2>&1
}

baseline_output=$(mktemp "$scratch_root/firestaff-theron-us-baseline.XXXXXX")
turn_output=$(mktemp "$scratch_root/firestaff-theron-us-turn.XXXXXX")
move_output=$(mktemp "$scratch_root/firestaff-theron-us-move.XXXXXX")
forward_blocked_output=$(mktemp "$scratch_root/firestaff-theron-us-forward-blocked.XXXXXX")
turn_left_output=$(mktemp "$scratch_root/firestaff-theron-us-turn-left.XXXXXX")
keyboard_right_output=$(mktemp "$scratch_root/firestaff-theron-us-keyboard-right.XXXXXX")
keyboard_left_output=$(mktemp "$scratch_root/firestaff-theron-us-keyboard-left.XXXXXX")
turn_alias_left_output=$(mktemp "$scratch_root/firestaff-theron-us-turn-alias-left.XXXXXX")
turn_alias_right_output=$(mktemp "$scratch_root/firestaff-theron-us-turn-alias-right.XXXXXX")
forward_alias_keypad_output=$(mktemp "$scratch_root/firestaff-theron-us-forward-alias-keypad.XXXXXX")
mouse_turn_left_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-turn-left.XXXXXX")
mouse_turn_right_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-turn-right.XXXXXX")
mouse_forward_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-forward.XXXXXX")
mouse_back_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-back.XXXXXX")
mouse_step_pair_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-step-pair.XXXXXX")
mouse_next_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-next.XXXXXX")
mouse_wait_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-wait.XXXXXX")
mouse_unmapped_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-unmapped.XXXXXX")
mouse_item_drop_output=$(mktemp "$scratch_root/firestaff-theron-us-mouse-item-drop.XXXXXX")
trap 'rm -f "$output" "$baseline_output" "$turn_output" "$move_output" "$forward_blocked_output" "$turn_left_output" "$keyboard_right_output" "$keyboard_left_output" "$turn_alias_left_output" "$turn_alias_right_output" "$forward_alias_keypad_output" "$mouse_turn_left_output" "$mouse_turn_right_output" "$mouse_forward_output" "$mouse_back_output" "$mouse_step_pair_output" "$mouse_next_output" "$mouse_wait_output" "$mouse_unmapped_output" "$mouse_item_drop_output"; rm -rf "$us_only_root"' EXIT
run_pose_probe "$baseline_output" "$startup_script" '1,0,0'
run_pose_probe "$turn_output" "$startup_script,key:e" '1,0,1'
run_pose_probe "$move_output" "$startup_script,down" '1,1,0'
run_pose_probe "$forward_blocked_output" "$startup_script,up" '1,0,0'
run_pose_probe "$turn_left_output" "$startup_script,key:q" '1,0,3'
run_pose_probe "$keyboard_right_output" "$startup_script,down,key:right" '2,1,0'
run_pose_probe "$keyboard_left_output" \
    "$startup_script,down,key:right,key:left" '1,1,0'
run_pose_probe "$turn_alias_left_output" "$startup_script,key:home" '1,0,3'
run_pose_probe "$turn_alias_right_output" "$startup_script,key:kp6" '1,0,1'
# KP8 is a native forward alias.  The script injects a real SDL keydown into
# the regular event loop after moving onto a source-backed open tile.
# Move one tile backward, then verify KP8 moves forward along the same
# authentic open route and returns to the source-backed starting square.
run_pose_probe "$forward_alias_keypad_output" "$startup_script,down,key:kp8" '1,0,0'
run_pose_probe "$mouse_turn_left_output" "$startup_script,click:204:144" '1,0,3'
run_pose_probe "$mouse_turn_right_output" "$startup_script,click:292:144" '1,0,1'
run_pose_probe "$mouse_forward_output" "$startup_script,down,click:248:144" '1,0,0'
run_pose_probe "$mouse_back_output" "$startup_script,click:248:163" '1,1,0'
run_pose_probe "$mouse_step_pair_output" \
    "$startup_script,down,click:292:163,click:204:163" '1,1,0'
run_pose_probe "$mouse_next_output" "$startup_script,click:204:182" '1,0,0'
run_pose_probe "$mouse_wait_output" "$startup_script,click:292:201" '1,0,0'
run_pose_probe "$mouse_unmapped_output" "$startup_script,click:100:80" '1,0,0'
run_pose_probe "$mouse_item_drop_output" \
    "$startup_script,down,down,key:q,click:248:182,click:292:182,click:204:201" \
    '1,2,3'
if ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=0' "$baseline_output" ||
   ! grep -Fq 'party=1,0,1 champions=2 runtimeTick=0' "$turn_output" ||
   ! grep -Fq 'party=1,1,0 champions=2 runtimeTick=1' "$move_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=0' "$forward_blocked_output" ||
   ! grep -Fq 'party=1,0,3 champions=2 runtimeTick=0' "$turn_left_output" ||
   ! grep -Fq 'party=2,1,0 champions=2 runtimeTick=2' "$keyboard_right_output" ||
   ! grep -Fq 'party=1,1,0 champions=2 runtimeTick=3' "$keyboard_left_output" ||
   ! grep -Fq 'party=1,0,3 champions=2 runtimeTick=0' "$turn_alias_left_output" ||
   ! grep -Fq 'party=1,0,1 champions=2 runtimeTick=0' "$turn_alias_right_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=2' "$forward_alias_keypad_output" ||
   ! grep -Fq 'input=backward result=1 action="MOVE" outcome="THERON STEPPED BACK"' \
       "$forward_alias_keypad_output" ||
   ! grep -Fq 'input=forward result=1 action="MOVE" outcome="THERON ADVANCED"' \
       "$forward_alias_keypad_output" ||
   ! grep -Fq 'party=1,0,3 champions=2 runtimeTick=0' "$mouse_turn_left_output" ||
   ! grep -Fq 'party=1,0,1 champions=2 runtimeTick=0' "$mouse_turn_right_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=2' "$mouse_forward_output" ||
   ! grep -Fq 'party=1,1,0 champions=2 runtimeTick=1' "$mouse_back_output" ||
   ! grep -Fq 'party=1,1,0 champions=2 runtimeTick=3' "$mouse_step_pair_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=0' "$mouse_next_output" ||
   ! grep -Fq 'theronActiveChampion=0' "$mouse_next_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=1' "$mouse_wait_output" ||
   ! grep -Fq 'party=1,0,0 champions=2 runtimeTick=0' "$mouse_unmapped_output" ||
   ! grep -Fq 'input=pointer-click point=100,68 result=0' "$mouse_unmapped_output" ||
   ! grep -Fq 'party=1,2,3 champions=2 runtimeTick=2' "$mouse_item_drop_output" ||
   ! grep -Eq 'pointer-click point=248,170 result=1 action="[^"]+" outcome="ITEM PICKED UP"' "$mouse_item_drop_output" ||
   ! grep -Eq 'pointer-click point=292,170 result=1 action="[^"]+" outcome="SANDALS"' "$mouse_item_drop_output" ||
   ! grep -Eq 'pointer-click point=204,189 result=1 action="[^"]+" outcome="ITEM DROPPED"' "$mouse_item_drop_output" ||
   ! grep -Fq 'pointer-click point=204,132' "$mouse_turn_left_output" ||
   ! grep -Fq 'pointer-click point=292,132' "$mouse_turn_right_output" ||
   ! grep -Fq 'pointer-click point=248,132' "$mouse_forward_output" ||
   ! grep -Eq 'pointer-click point=292,151 result=1 action="[^"]+" outcome="THERON STEPPED RIGHT"' \
       "$mouse_step_pair_output" ||
   ! grep -Eq 'pointer-click point=204,151 result=1 action="[^"]+" outcome="THERON STEPPED LEFT"' \
       "$mouse_step_pair_output" ||
   ! grep -Fq 'pointer-click point=248,151' "$mouse_back_output"; then
    cat "$baseline_output" "$turn_output" "$move_output" \
        "$forward_blocked_output" "$turn_left_output" \
        "$keyboard_right_output" "$keyboard_left_output" \
        "$turn_alias_left_output" "$turn_alias_right_output" \
        "$forward_alias_keypad_output" \
        "$mouse_turn_left_output" "$mouse_back_output" \
        "$mouse_turn_right_output" "$mouse_forward_output" \
        "$mouse_step_pair_output" \
        "$mouse_next_output" "$mouse_wait_output" "$mouse_unmapped_output" \
        "$mouse_item_drop_output" >&2
    printf '%s\n' 'FAIL: authentic Theron USA keyboard and mouse input did not match source-backed pose/tick behavior' >&2
    exit 1
fi

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game theron --platform pce \
    --data-dir "$track02" \
    --script 'wait2,click:1173:728,wait2,click:410:405,wait2,click:450:405,wait2' \
    --duration 5000 2>&1) || {
        printf '%s\n' "$menu_output" >&2
        printf '%s\n' 'FAIL: authentic raw Track 02 did not launch through mouse-selected M12 cards' >&2
        exit 1
    }
if ! grep -Fq '[TQR] Verified Track 02 accepted:' <<<"$menu_output" ||
   grep -Fq 'deterministic fallback assets' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: mouse-selected M12 cards lost the authentic Track 02 route' >&2
    exit 1
fi

set +e
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron --theron-native jp --data-dir "$us_only_root" \
    --boot-probe --duration 0 >"$output" 2>&1
wrong_region_rc=$?
set -e
if [[ $wrong_region_rc -ne 2 ]] ||
   ! grep -Fq 'that region' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: JP native selection borrowed the available USA Track 02' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron USA raw BIN reaches runtime; keyboard movement/turning and mouse movement/champion/wait/pickup/item-select/drop match source-backed state'
