# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass378-source-portrait-sixshot-retry`
- capture count: 6
- pass: `False`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Problems

- image0001-raw.png: classified title_or_menu, expected dungeon_gameplay
- image0002-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0003-raw.png: classified entrance_menu, expected dungeon_gameplay
- image0004-raw.png: classified dungeon_gameplay, expected spell_panel
- image0006-raw.png: classified dungeon_gameplay, expected inventory
- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass378-source-portrait-sixshot-retry/image0001-raw.png` | `title_or_menu` | `dungeon_gameplay` | NO | sparse viewport plus colorful/right-column title-menu art | `7addedda4327` |
| 2 | `verification-screens/pass378-source-portrait-sixshot-retry/image0002-raw.png` | `entrance_menu` | `dungeon_gameplay` | NO | dungeon entrance/menu controls still occupy the right column | `880d853d1bbe` |
| 3 | `verification-screens/pass378-source-portrait-sixshot-retry/image0003-raw.png` | `entrance_menu` | `dungeon_gameplay` | NO | dungeon entrance/menu controls still occupy the right column | `17bd7e878157` |
| 4 | `verification-screens/pass378-source-portrait-sixshot-retry/image0004-raw.png` | `dungeon_gameplay` | `spell_panel` | NO | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 5 | `verification-screens/pass378-source-portrait-sixshot-retry/image0005-raw.png` | `dungeon_gameplay` | `dungeon_gameplay` | yes | viewport content with mostly dark in-game right column | `48ed3743ab6a` |
| 6 | `verification-screens/pass378-source-portrait-sixshot-retry/image0006-raw.png` | `dungeon_gameplay` | `inventory` | NO | viewport content with mostly dark in-game right column | `48ed3743ab6a` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass378-source-portrait-sixshot-retry/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.3511 | 0.0628 | 9 | 58.50 |
| right_action | 0.9167 | 0.8884 | 12 | 51.58 |
| spell_area | 0.9237 | 0.8920 | 11 | 48.57 |
| right_column | 0.9283 | 0.8628 | 16 | 48.40 |
| inventory_extent | 0.3457 | 0.0523 | 8 | 46.68 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass378-source-portrait-sixshot-retry/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9116 | 0.0000 | 6 | 59.70 |
| right_action | 0.9231 | 0.8971 | 13 | 51.28 |
| spell_area | 0.9398 | 0.9140 | 11 | 46.84 |
| right_column | 0.9420 | 0.8691 | 16 | 46.79 |
| inventory_extent | 0.7627 | 0.0000 | 6 | 69.81 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass378-source-portrait-sixshot-retry/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.9502 | 0.9349 | 13 | 45.24 |
| spell_area | 0.9609 | 0.9462 | 12 | 41.09 |
| right_column | 0.9649 | 0.8985 | 16 | 42.36 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass378-source-portrait-sixshot-retry/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass378-source-portrait-sixshot-retry/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |

### `verification-screens/pass378-source-portrait-sixshot-retry/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9250 | 0.0000 | 6 | 57.48 |
| right_action | 0.0138 | 0.0000 | 2 | 29.74 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.0626 | 0.0597 | 3 | 38.64 |
| inventory_extent | 0.7836 | 0.0000 | 6 | 68.96 |
| title_top | 0.0000 | 0.0000 | 1 | 0.00 |
