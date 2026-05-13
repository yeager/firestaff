# Pass 112 — original semantic-route audit

This joins route-shot labels with pass80 raw-frame classifications before original frames are allowed into overlay comparison.

- attempt dir: `verification-screens/pass505-original-overlay-mouse-route-recapture`
- semantic route ready: `false`
- honesty: Semantic route audit only. Passing this gate makes original-vs-Firestaff overlay inputs eligible for pixel comparison; it does not claim pixel parity.

## Problems

- frame classes do not match expected semantic route: ['graphics_320x200_unclassified', 'graphics_320x200_unclassified', 'graphics_320x200_unclassified', 'graphics_320x200_unclassified', 'graphics_320x200_unclassified', 'graphics_320x200_unclassified']
- pass80 classifier reported problems: image0001-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay; image0002-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay; image0003-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay; image0004-raw.png: classified graphics_320x200_unclassified, expected spell_panel; image0005-raw.png: classified graphics_320x200_unclassified, expected dungeon_gameplay; image0006-raw.png: classified graphics_320x200_unclassified, expected inventory; duplicate raw frames detected: 2 unique sha256 value(s) repeat
- duplicate raw frame hashes remain; route did not advance through unique visual states

## Six-shot semantic checkpoints

| # | route label | expected label | classification | expected class | sha256 |
|---|-------------|----------------|----------------|----------------|--------|
| 1 | `party_hud` | `party_hud` | `graphics_320x200_unclassified` | `dungeon_gameplay` | `a17790109e74` |
| 2 | `` | `` | `graphics_320x200_unclassified` | `dungeon_gameplay` | `b1cefb2478a8` |
| 3 | `` | `` | `graphics_320x200_unclassified` | `dungeon_gameplay` | `a17790109e74` |
| 4 | `spell_panel` | `spell_panel` | `graphics_320x200_unclassified` | `spell_panel` | `b1cefb2478a8` |
| 5 | `` | `` | `graphics_320x200_unclassified` | `dungeon_gameplay` | `a17790109e74` |
| 6 | `inventory_panel` | `inventory_panel` | `graphics_320x200_unclassified` | `inventory` | `b1cefb2478a8` |
