# Pass 112 — original semantic-route audit

This joins route-shot labels with pass80 raw-frame classifications before original frames are allowed into overlay comparison.

- attempt dir: `verification-screens/pass112-n2-stable-hud-route`
- semantic route ready: `false`
- honesty: Semantic route audit only. Passing this gate makes original-vs-Firestaff overlay inputs eligible for pixel comparison; it does not claim pixel parity.

## Problems

- frame classes do not match expected semantic route: ['dungeon_gameplay', 'dungeon_gameplay', 'wall_closeup', 'wall_closeup', 'wall_closeup', 'wall_closeup']
- pass80 classifier reported problems: image0003-raw.png: classified wall_closeup, expected dungeon_gameplay; image0004-raw.png: classified wall_closeup, expected spell_panel; image0005-raw.png: classified wall_closeup, expected dungeon_gameplay; image0006-raw.png: classified wall_closeup, expected inventory; duplicate raw frames detected: 1 unique sha256 value(s) repeat
- duplicate raw frame hashes remain; route did not advance through unique visual states

## Six-shot semantic checkpoints

| # | route label | expected label | classification | expected class | sha256 |
|---|-------------|----------------|----------------|----------------|--------|
| 1 | `party_hud` | `party_hud` | `dungeon_gameplay` | `dungeon_gameplay` | `48ed3743ab6a` |
| 2 | `` | `` | `dungeon_gameplay` | `dungeon_gameplay` | `47d61e2ae941` |
| 3 | `` | `` | `wall_closeup` | `dungeon_gameplay` | `ee7741746ea9` |
| 4 | `spell_panel` | `spell_panel` | `wall_closeup` | `spell_panel` | `ee7741746ea9` |
| 5 | `` | `` | `wall_closeup` | `dungeon_gameplay` | `ee7741746ea9` |
| 6 | `inventory_panel` | `inventory_panel` | `wall_closeup` | `inventory` | `ee7741746ea9` |
