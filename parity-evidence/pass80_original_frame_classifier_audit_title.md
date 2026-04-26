# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `verification-screens/pass79-original-direct-flags-title`
- capture count: 6
- pass: `True`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

## Warnings

- duplicate raw frames detected: 1 unique sha256 value(s) repeat

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `verification-screens/pass79-original-direct-flags-title/image0001-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 2 | `verification-screens/pass79-original-direct-flags-title/image0002-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 3 | `verification-screens/pass79-original-direct-flags-title/image0003-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 4 | `verification-screens/pass79-original-direct-flags-title/image0004-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 5 | `verification-screens/pass79-original-direct-flags-title/image0005-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |
| 6 | `verification-screens/pass79-original-direct-flags-title/image0006-raw.png` | `title_or_menu` | `` |  | sparse viewport plus colorful/right-column title-menu art | `0494bcb374b1` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `verification-screens/pass79-original-direct-flags-title/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass79-original-direct-flags-title/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass79-original-direct-flags-title/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass79-original-direct-flags-title/image0004-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass79-original-direct-flags-title/image0005-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |

### `verification-screens/pass79-original-direct-flags-title/image0006-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.2263 | 0.0711 | 5 | 30.83 |
| right_action | 0.8966 | 0.8766 | 12 | 57.21 |
| spell_area | 0.9168 | 0.9149 | 11 | 50.13 |
| right_column | 0.9312 | 0.8713 | 16 | 49.02 |
| inventory_extent | 0.2545 | 0.0742 | 5 | 31.86 |
| title_top | 0.5526 | 0.4793 | 8 | 60.32 |
