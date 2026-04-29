# N2 viewport/world source-lock follow-up: D3C draw order

Target: one narrow source-locked render target, the center far square `M600_VIEW_SQUARE_D3C` in ReDMCSB `DUNVIEW.C`.

## Primary source audited

Source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

- `DUNVIEW.C:6642-6834` — `F0118_DUNGEONVIEW_DrawSquareD3C_CPSF`, the D3C square renderer.
  - `6664`: calls `F0172_DUNGEON_SetSquareAspect(...)` to classify square contents.
  - `6697-6720`: wall case; draws D3C wall bitmap (`6699-6715`) and only proceeds to objects when the wall ornament is an alcove (`6716-6718`); otherwise returns (`6720`).
  - `6721-6747`: front-door case; floor ornament first (`6722`), objects/creatures pass 1 behind frame (`6723`), door frame/button/door (`6725-6744`), then pass-2 order set (`6746`).
  - `6748-6816`: pit/teleporter/corridor case; visible pit drawn before fall-through (`6751-6760`), corridor/teleporter order set to `C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT` (`6813`), floor ornament drawn (`6814`), then objects/creatures/projectiles/explosions drawn via `F0115...` (`6816`).
  - `6818-6834`: visible teleporter field overlay is drawn after objects via `F0113_DUNGEONVIEW_DrawField(...)` (`6825-6831`).
- `DUNVIEW.C:4547-4582` — `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` function header comment documents its internal order: objects, creature(s), projectiles, explosions/fluxcage.
- `DUNVIEW.C:4794-4800` — `F0115` decodes door-front pass nibble and decides whether this is alcove rendering.
- `DUNVIEW.C:4819-5191` — per-cell object loop; cell ordinal decoded at `4826-4830`, object rendering path blits at `5181-5184`.
- `DUNVIEW.C:5201-5644` — creature rendering after objects; creature blit at `5511-5514` / zone blit at `5627`.
- `DUNVIEW.C:5645-5904` — projectile rendering after creatures, including object-style projectiles returning through `T0115171_BackFrom...`.
- `DUNVIEW.C:5916-6136` — explosion pass starts only after all cells are processed.
- `DUNVIEW.C:3940-4006` — `F0108_DUNGEONVIEW_DrawFloorOrnament`, used by D3C at `6722` and `6814`.
- `DUNVIEW.C:4218-4316` — `F0111_DUNGEONVIEW_DrawDoor`, used by D3C door case at `6741/6744`.

## Probe artifact

Added `d3c_draw_order_probe.py`, a source-only verification script. It hashes `DUNVIEW.C`, locates the D3C and `F0115` anchors above, and asserts the source order for the target.

Run output saved to `probe_output.txt`:

```text
source=/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C
dunview_sha256=6e1cc4f4aafdaf4ef93ea022fcea8b6f93a1d20e3d3f0db89f64419fe8c1c8c2
F0118_D3C_range=6642-6834
F0118_D3C_block_sha256=9f90e014ae9c56d81f7a6497bcd8f8a3a6338421e82b3fc2725cae9b76e859b3
verified=d3c_source_order_anchors_ok
```

Full output: `verification-m11/n2-viewport-d3c-source-lock-20260429/probe_output.txt`.

## Evidence paths

- `verification-m11/n2-viewport-d3c-source-lock-20260429/d3c_draw_order_probe.py`
- `verification-m11/n2-viewport-d3c-source-lock-20260429/probe_output.txt`
- `verification-m11/n2-viewport-d3c-source-lock-20260429/report.md`

No screenshots were produced; this lane was intentionally source-locked, not emulator-based.
