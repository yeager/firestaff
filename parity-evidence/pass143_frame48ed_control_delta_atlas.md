# Pass 143 — frame 48ed control delta atlas

- source run: `<N2_RUNS>/20260428-192537-pass141-pm-f1-dungeon-control-readiness`
- purpose: test whether the repeated `48ed3743ab6a` dungeon frame hides real control changes or is a static no-party/start frame.
- total rows: 54
- unique hashes: 16

## Hash counts

- `17bd7e878157`: 14 rows; classes=entrance_menu
- `014ed52c71a0`: 10 rows; classes=entrance_menu
- `48ed3743ab6a`: 9 rows; classes=dungeon_gameplay
- `ceb0c2eec633`: 5 rows; classes=entrance_menu
- `7addedda4327`: 3 rows; classes=title_or_menu
- `1eb3c7fe6494`: 2 rows; classes=entrance_menu
- `f2b6ba10f514`: 2 rows; classes=inventory
- `183f110f5d3b`: 1 rows; classes=graphics_320x200_unclassified
- `1ee706538fb3`: 1 rows; classes=dungeon_gameplay
- `38f411f2ea7e`: 1 rows; classes=inventory
- `5257b4c8ee2d`: 1 rows; classes=entrance_menu
- `672193a5deba`: 1 rows; classes=entrance_menu
- `67893775f5c1`: 1 rows; classes=title_or_menu
- `8bc78f18f48a`: 1 rows; classes=entrance_menu
- `d15db869b9f6`: 1 rows; classes=graphics_320x200_unclassified
- `f8d841d4357c`: 1 rows; classes=title_or_menu

## Focus rows

| scenario | input | sha12 | class | changed_ratio | bbox | file |
|---|---|---|---|---:|---|---|
| `f1_key4_movement` | `key Up` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_key4_movement/image0008-key_Up_8.png` |
| `f1_key4_movement` | `key Left` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_key4_movement/image0009-key_Left_9.png` |
| `f1_key4_movement` | `key Right` | `48ed3743ab6a` | `dungeon_gameplay` | 0.461969 | `(0, 0, 320, 200)` | `f1_key4_movement/image0010-key_Right_10.png` |
| `f1_key4_movement` | `key Down` | `48ed3743ab6a` | `dungeon_gameplay` | 0.0 | `None` | `f1_key4_movement/image0011-key_Down_11.png` |
| `f1_panel_clicks_then_move` | `key Up` | `1ee706538fb3` | `dungeon_gameplay` | 0.461813 | `(0, 0, 320, 200)` | `f1_panel_clicks_then_move/image0009-key_Up_9.png` |
| `f1_panel_clicks_then_move` | `key Right` | `48ed3743ab6a` | `dungeon_gameplay` | 0.018766 | `(233, 53, 320, 169)` | `f1_panel_clicks_then_move/image0010-key_Right_10.png` |
| `f1_panel_clicks_then_move` | `key Left` | `48ed3743ab6a` | `dungeon_gameplay` | 0.0 | `None` | `f1_panel_clicks_then_move/image0011-key_Left_11.png` |
| `f1_return_movement` | `key Up` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_return_movement/image0008-key_Up_8.png` |
| `f1_return_movement` | `key Up` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_return_movement/image0009-key_Up_9.png` |
| `f1_return_movement` | `key Right` | `48ed3743ab6a` | `dungeon_gameplay` | 0.461969 | `(0, 0, 320, 200)` | `f1_return_movement/image0010-key_Right_10.png` |
| `f1_return_movement` | `key Left` | `48ed3743ab6a` | `dungeon_gameplay` | 0.0 | `None` | `f1_return_movement/image0011-key_Left_11.png` |
| `f1_space_movement` | `key Up` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_space_movement/image0008-key_Up_8.png` |
| `f1_space_movement` | `key Right` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_space_movement/image0009-key_Right_9.png` |
| `f1_space_movement` | `key Down` | `48ed3743ab6a` | `dungeon_gameplay` | 0.461969 | `(0, 0, 320, 200)` | `f1_space_movement/image0010-key_Down_10.png` |
| `f1_top_clicks_then_move` | `key Up` | `17bd7e878157` | `entrance_menu` | 0.0 | `None` | `f1_top_clicks_then_move/image0009-key_Up_9.png` |
| `f1_top_clicks_then_move` | `key Right` | `48ed3743ab6a` | `dungeon_gameplay` | 0.461969 | `(0, 0, 320, 200)` | `f1_top_clicks_then_move/image0010-key_Right_10.png` |
| `f1_top_clicks_then_move` | `key Left` | `48ed3743ab6a` | `dungeon_gameplay` | 0.0 | `None` | `f1_top_clicks_then_move/image0011-key_Left_11.png` |

## Interpretation

A route is only a party-control candidate if movement inputs produce distinct dungeon frames or a non-trivial visual delta. Repeated `48ed3743ab6a` with zero/near-zero deltas means this is likely a static no-party/dungeon placeholder rather than real champion control; non-zero deltas or new dungeon hashes should become the next route target.
