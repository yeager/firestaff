# Pass211 DM1 V1 original movement fresh blocker manifest

Status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`

This is a repo-friendly preservation manifest for `verification-screens/pass210-n2-original-movement-route-fresh/`. PNG/PPM payloads are intentionally not committed.

## Shot binding

| shot | raw frame | route label | classifier | raw sha256 prefix | duplicate? |
|---:|---|---|---|---|---|
| 1 | `image0001-raw.png` | `start` | `entrance_menu` | `17bd7e878157` | no |
| 2 | `image0002-raw.png` | `turn_left` | `wall_closeup` | `fbeb1b82cd09` | yes |
| 3 | `image0003-raw.png` | `turn_right` | `dungeon_gameplay` | `48ed3743ab6a` | yes |
| 4 | `image0004-raw.png` | `forward` | `dungeon_gameplay` | `48ed3743ab6a` | yes |
| 5 | `image0005-raw.png` | `turn_left_2` | `wall_closeup` | `fbeb1b82cd09` | yes |
| 6 | `image0006-raw.png` | `post_redraw` | `dungeon_gameplay` | `48ed3743ab6a` | yes |

## Duplicate groups

- raw `fbeb1b82cd09`: shots [2, 5] (image0002-raw.png, image0005-raw.png)
- raw `48ed3743ab6a`: shots [3, 4, 6] (image0003-raw.png, image0004-raw.png, image0006-raw.png)
- viewport `701689e73fc0`: shots [1, 3, 4, 6] (01_ingame_start_original_viewport_224x136.ppm, 03_ingame_move_forward_original_viewport_224x136.ppm, 04_ingame_spell_panel_original_viewport_224x136.ppm, 06_ingame_inventory_panel_original_viewport_224x136.ppm)
- viewport `1e71ed879980`: shots [2, 5] (02_ingame_turn_right_original_viewport_224x136.ppm, 05_ingame_after_cast_original_viewport_224x136.ppm)

## Blocker

The route still cannot produce a non-duplicate, semantically aligned movement-only sequence. It includes an entrance-menu frame and repeated post-command viewport hashes, so it does not promote original-vs-Firestaff parity evidence.
