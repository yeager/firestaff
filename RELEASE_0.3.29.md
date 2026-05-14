## Firestaff v0.3.29

DM1 V1 source-lock preview release focused on blocked collision queue lifecycle evidence.

### Added
- Added a DM1 V1 blocked collision queue lifecycle verifier and evidence manifest.
- Extended the movement command core compatibility test for blocked-wall queue flushing, reserved command preservation, pending stop replay, and no successful-step cooldown.

### Verified
- `git diff --check` clean.
- `python3 -m py_compile tools/verify_pass544_dm1_v1_blocked_collision_queue_lifecycle.py`.
- CMake Release configure/build on N2.
- Focused runtime gate `dm1_v1_movement_command_core_pc34_compat`.
- Direct verifier `pass544_dm1_v1_blocked_collision_queue_lifecycle`.
- Focused CTest regex `pass544_dm1_v1_blocked_collision_queue_lifecycle|dm1_v1_movement_command_core_pc34_compat`.

Supported release artifacts are built by GitHub Actions for macOS, Windows, and Linux.
