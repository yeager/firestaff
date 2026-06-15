# Pass653 - DM1 V1 mirror candidate key-rotation inventory-click regression

Status: PASS653_DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REGRESSION_LOCKED

DM1 V1 mirror candidate pending, F0361 TURN_* key queue write, F0380 in-flight pending inventory click race, and byte-identical F0293 redraw against no-click rotation.

## Source Checks
- PASS command_f0359_m568_panel_dispatch (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/COMMAND.C)
- PASS command_f0361_keyboard_queue_write (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/COMMAND.C)
- PASS command_f0380_queue_dispatch (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/COMMAND.C)
- PASS revive_candidate_pending_and_clear (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/REVIVE.C)
- PASS champion_slot_and_leader_hand (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/CHAMPION.C)
- PASS chamdraw_redraw_tuple (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/CHAMDRAW.C)
- PASS defs_required_chain (/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DEFS.H)
- PASS module_runtime_helper (src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.c)
- PASS header_public_contract (src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.h)
- PASS c_test_runtime_assertions (tests/test_dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.c)
- PASS cmake_registration (CMakeLists.txt)

## Verification
- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat: rc=0
~~~
PASS dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat 32/32 assertions
~~~
- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass653_dm1_v1_mirror_candidate_keyrot_combo_invclick_regression.py --check-only: rc=0
~~~
PASS pass653 check-only
~~~

## Non-Claims
- No new original DOSBox capture.
- No SDL input capture or framebuffer parity claim.
- No behavior beyond this contract-only key-dispatch inventory-click race.
- No changes to existing mirror-candidate modules.
