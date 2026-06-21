# Pass 80 — original raw-frame classifier audit

This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.
It is measurement-only and records the layout features behind each label.

- attempt dir: `/tmp/dm1_v3_capture/01_viewport`
- capture count: 3
- pass: `True`
- honesty: Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.

| # | file | classification | expected | ok | reason | sha256 |
|---|------|----------------|----------|----|--------|--------|
| 1 | `/tmp/dm1_v3_capture/01_viewport/image0001-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `8f62254638b4` |
| 2 | `/tmp/dm1_v3_capture/01_viewport/image0002-raw.png` | `dungeon_gameplay` | `` |  | viewport content with mostly dark in-game right column | `0e6eeb39bbbb` |
| 3 | `/tmp/dm1_v3_capture/01_viewport/image0003-raw.png` | `inventory` | `` |  | dense low-color inventory extent over viewport | `8c43b5795765` |

## Region metrics

Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.

### `/tmp/dm1_v3_capture/01_viewport/image0001-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.8933 | 0.0000 | 180 | 60.30 |
| right_action | 0.0156 | 0.0000 | 16 | 27.59 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.1055 | 0.0612 | 51 | 37.82 |
| inventory_extent | 0.8185 | 0.0000 | 158 | 62.65 |
| title_top | 0.2420 | 0.0058 | 110 | 20.19 |

### `/tmp/dm1_v3_capture/01_viewport/image0002-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.8843 | 0.0000 | 178 | 60.39 |
| right_action | 0.0156 | 0.0000 | 16 | 27.59 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.1055 | 0.0612 | 51 | 37.82 |
| inventory_extent | 0.7989 | 0.0000 | 149 | 60.93 |
| title_top | 0.2420 | 0.0058 | 110 | 20.19 |

### `/tmp/dm1_v3_capture/01_viewport/image0003-raw.png`

| region | nonblack | color | unique | luma stddev |
|--------|----------|-------|--------|-------------|
| viewport | 0.9557 | 0.0000 | 128 | 45.83 |
| right_action | 0.0156 | 0.0000 | 16 | 27.59 |
| spell_area | 0.0000 | 0.0000 | 1 | 0.00 |
| right_column | 0.1055 | 0.0612 | 51 | 37.82 |
| inventory_extent | 1.0000 | 0.0000 | 66 | 27.18 |
| title_top | 0.2420 | 0.0058 | 110 | 20.19 |
