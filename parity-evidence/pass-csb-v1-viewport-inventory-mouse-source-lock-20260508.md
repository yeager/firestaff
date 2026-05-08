# CSB V1 viewport/inventory mouse source-lock pass — 2026-05-08

Result: PASS — source evidence and verifier for a narrow CSB V1 viewport/inventory mouse seam. This pass does **not** claim rendered/pixel parity; it locks the source routing order that future CSB viewport and inventory parity work must preserve.

## Scope

- Seam: adventuring viewport clicks vs inventory/clothing/mouth/eye click dispatch.
- Primary for shared V1 mechanics remains ReDMCSB. This pass is CSB-specific lineage evidence, so it uses only N2-local CSB references:
  - `/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin/Mouse.cpp`
  - `/home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Mouse.cpp`
- Verifier: `tools/verify_csb_v1_viewport_inventory_mouse_source_lock.py`.

## Source anchors

### CSBWin mirror

- `CSBWin/Mouse.cpp:1031-1086` — `HandleClickInViewport` computes the front cell from `d.partyX`, `d.partyY`, `d.partyFacing`, `d.DeltaX`, and `d.DeltaY`; door-facing clicks route through `ViewportObjectButtons[5]`, `QueueSound(1, ...)`, and `CreateTimer(TT_DOOR, ...)`.
- `CSBWin/Mouse.cpp:1096-1160` — empty-hand viewport clicks scan `ViewportObjectButtons[0..5]`; button `5` routes to `TouchWallF1()` when not facing an alcove, while buttons `0..4` route to `TAG01a148(D5W)`.
- `CSBWin/Mouse.cpp:1162-1228` — object-in-hand viewport clicks are ordered as drop areas, alcove drop (`DropObject(4)`), fountain fill (`FacingWaterFountain`), then wall touch.
- `CSBWin/Mouse.cpp:1600-1668` — click dispatch keeps inventory/clothing (`28..65`, `7..11`, `83`), mouth (`70`), eye (`71`), and viewport (`80`) as separate routes; viewport calls `HandleClickInViewport` only while `d.ClockRunning`.

### CSB/src lineage

- `CSB/src/Mouse.cpp:1148-1165` — CSB lineage adds the empty-hand fountain-drink case on viewport button `5`: selected champion water increases by `1600`, gulp sound `8` queues, then ordinary alcove/wall touch remains below it.
- `CSB/src/Mouse.cpp:1771-1839` — CSB lineage preserves the same inventory/clothing, mouth, eye, and viewport dispatch partition as the CSBWin mirror.

## Parity contract locked by this pass

1. Inventory/clothing/mouth/eye click routes must stay outside viewport hit handling.
2. Viewport route `80` is only active while the clock is running.
3. Viewport click handlers must derive the front-cell coordinate from party position/facing before door/wall/object interactions.
4. Empty-hand viewport button `5` is special: door switch/wall touch/fountain-drink behavior hangs off this source seam.
5. Object-in-hand wall clicks must preserve source ordering: drop areas before alcove/fountain/wall-touch handling.

## Gate

```sh
python3 tools/verify_csb_v1_viewport_inventory_mouse_source_lock.py \
  --csbwin-source /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin \
  --csb-source /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src
```
