# Pass 166 — pass 2 reincarnate-only rerun

- Host: N2 (`Firestaff-Worker-VM`)
- Run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-073006-pass166-pass2-reincarnate-only`
- Evidence root: `parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_reincarnate`
- Scenario: `enter_portrait11182_then_reincarnate` / `DM -vv -sn`
- Runner note: run under `xvfb-run -a` after the first attempt failed with `DISPLAY=(null)`.

## Result

**Blocked: `static-no-party`** — known static no-party hash `48ed3743ab6a` appears after the route reaches dungeon gameplay.

Unique hashes observed:

- `014ed52c71a0`
- `125549ae51c3`
- `17bd7e878157`
- `183f110f5d3b`
- `307323fbc1f7`
- `342d4e7bdac2`
- `48ed3743ab6a`
- `ab7acc8ca298`

## Route rows

| # | label | class | sha12 |
|---|-------|-------|-------|
| 1 | `initial` | `entrance_menu` | `ab7acc8ca298` |
| 2 | `click_270_52_2` | `entrance_menu` | `014ed52c71a0` |
| 3 | `after_c407_enter_click` | `entrance_menu` | `014ed52c71a0` |
| 4 | `click_111_82_4` | `graphics_320x200_unclassified` | `183f110f5d3b` |
| 5 | `after_source_portrait_111_82` | `title_or_menu` | `307323fbc1f7` |
| 6 | `click_186_115_6` | `title_or_menu` | `342d4e7bdac2` |
| 7 | `after_source_c161_reincarnate` | `entrance_menu` | `125549ae51c3` |
| 8 | `key_Return_8` | `entrance_menu` | `17bd7e878157` |
| 9 | `after_confirm` | `entrance_menu` | `17bd7e878157` |
| 10 | `key_Up_10` | `dungeon_gameplay` | `48ed3743ab6a` |
| 11 | `after_move_up` | `dungeon_gameplay` | `48ed3743ab6a` |
| 12 | `key_Right_12` | `dungeon_gameplay` | `48ed3743ab6a` |
| 13 | `after_turn_right` | `dungeon_gameplay` | `48ed3743ab6a` |
| 14 | `key_F1_14` | `dungeon_gameplay` | `48ed3743ab6a` |
| 15 | `after_f1_readiness` | `dungeon_gameplay` | `48ed3743ab6a` |
| 16 | `key_F4_16` | `dungeon_gameplay` | `48ed3743ab6a` |
| 17 | `after_f4_readiness` | `dungeon_gameplay` | `48ed3743ab6a` |

## Interpretation

Pass 2 gives useful negative evidence: the portrait/reincarnate route causes visible intermediate changes, but still collapses into the known static no-party dungeon frame and never produces a spell/inventory/control marker. Do not promote this as overlay-ready original evidence.
