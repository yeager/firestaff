# Pass 112 — original semantic-route audit

This joins route-shot labels with pass80 raw-frame classifications before original frames are allowed into overlay comparison.

- attempt dir: `verification-screens/pass112-n2-route-probe`
- semantic route ready: `false`
- honesty: Semantic route audit only. Passing this gate makes original-vs-Firestaff overlay inputs eligible for pixel comparison; it does not claim pixel parity.

## Problems

- frame classes do not match expected semantic route: ['graphics_320x200_unclassified', 'title_or_menu', 'title_or_menu', 'title_or_menu', 'entrance_menu', 'entrance_menu']
- pass80 classifier reported problems: image0001-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay; image0002-raw.png: classified title_or_menu, expected dungeon_gameplay; image0003-raw.png: classified title_or_menu, expected dungeon_gameplay; image0004-raw.png: classified title_or_menu, expected spell_panel; image0005-raw.png: classified entrance_menu, expected dungeon_gameplay; image0006-raw.png: classified entrance_menu, expected inventory; duplicate raw frames detected: 1 unique sha256 value(s) repeat
- duplicate raw frame hashes remain; route did not advance through unique visual states

## Six-shot semantic checkpoints

| # | route label | expected label | classification | expected class | sha256 |
|---|-------------|----------------|----------------|----------------|--------|
| 1 | `party_hud` | `party_hud` | `graphics_320x200_unclassified` | `dungeon_gameplay` | `f7f58cbefcf2` |
| 2 | `` | `` | `title_or_menu` | `dungeon_gameplay` | `307323fbc1f7` |
| 3 | `` | `` | `title_or_menu` | `dungeon_gameplay` | `307323fbc1f7` |
| 4 | `spell_panel` | `spell_panel` | `title_or_menu` | `spell_panel` | `307323fbc1f7` |
| 5 | `` | `` | `entrance_menu` | `dungeon_gameplay` | `074422f14319` |
| 6 | `inventory_panel` | `inventory_panel` | `entrance_menu` | `inventory` | `880d853d1bbe` |
