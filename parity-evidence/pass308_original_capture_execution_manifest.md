# pass308_original_capture_execution_manifest

- status: `PASS_CAPTURE_EXECUTED_STATE_ORACLE_PENDING`
- screenshots/pixels: not tracked

| batch | labels covered | classes |
|---|---|---|
| `A` | `['move_forward_west', 'start_south', 'turn_right_west']` | `{'dungeon_gameplay': 1, 'wall_closeup': 5}` |
| `B` | `['start_south', 'turn_left_east']` | `{'dungeon_gameplay': 1, 'wall_closeup': 5}` |
| `C` | `['blocked_forward_south_wall', 'start_south']` | `{'dungeon_gameplay': 6}` |

Remaining blockers:
- party tuple/F0128 state is not source-bound for the original runtime yet
- duplicate viewport hashes are preserved as evidence and must not be interpreted as pixel parity
- no screenshots/PPM/PNG are promoted to tracked release artifacts by this pass

Not claimed:
- pixel parity
- complete state-oracle proof
- bitmap-byte publication
