# CSB V1 parity surface matrix — 2026-05-06

Scope: first countable CSB V1 parity surface matrix after the inventory/startup boundary. This is evidence-only: no CSB launch, rendering, gameplay, save compatibility, or menu/runtime behavior is enabled.

## Surfaces

| surface | status | boundary |
| --- | --- | --- |
| startup/load assets | SOURCE_LOCKED_BLOCKED_RUNTIME | Requires Atari ST v2.x `GRAPHICS.DAT` + `DUNGEON.DAT` identity and CSB support-file proof before startup/load can count. |
| dungeon load/state | SOURCE_LOCKED_BLOCKED_RUNTIME | Requires CSB dungeon identity/state proof from the curated Atari dungeon payload. |
| prison/champion route | SOURCE_LOCKED_BLOCKED_RUNTIME | Must reproduce the CSB Utility/Make-New-Adventure route from CSB state, not DM1 champion routing reuse. |
| viewport/HUD rendering | SOURCE_LOCKED_BLOCKED_CAPTURE | Needs a CSB original capture path with stable frame/state anchors tied to the Atari v2.x renderer lane. |
| input/mode routing | SOURCE_LOCKED_BLOCKED_RUNTIME | Must prove CSB mode-specific mouse/keyboard routing, including Utility/reincarnate modes. |
| save/new-adventure | BLOCKED_MISSING_SAMPLE | Still blocked without a curated extracted `CSBGAME*.DAT`/`.BAK` sample or generated-save harness with provenance. |

## Source anchors

The verifier checks local N2 references only:

- ReDMCSB `DEFS.H`, `CEDTINCH.C`, and `DUNVIEW.C` for save/dungeon IDs, New Adventure gates, and viewport boxes.
- CSB lineage `README`, `Graphics.cpp`, `Chaos.cpp`, and `Mouse.cpp` for required payloads, graphics open boundary, Utility buttons, and input modes.
- CSBWin `Game/readme.txt`, `SaveGame.cpp`, `CSBwin.cpp`, and `data.cpp` for play workflow, dungeon-index usage, viewport trace hook, and keyboard mode state.
- Firestaff blocker `parity-evidence/blocker-n2-csb-sample-save-search-20260430.md` for the sample-save boundary.

## Non-claims

- No CSB launch, rendering, gameplay, or save compatibility is enabled or claimed.
- No `menu_startup_m12.c` or launch/runtime code was modified.
- Atari ST and Amiga CSB payloads remain separate; this matrix anchors the current Atari ST v2.x lane only.

## Verification

```sh
python3 -m py_compile tools/verify_csb_v1_parity_surface_matrix.py
./tools/verify_csb_v1_parity_surface_matrix.py
python3 -m json.tool parity-evidence/verification/csb_v1_parity_surface_matrix.json >/dev/null
```

JSON output: `parity-evidence/verification/csb_v1_parity_surface_matrix.json`
