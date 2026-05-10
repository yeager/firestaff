# pass465_dm1_v1_endgame_original_route_gate

- status: `BLOCKED_ORIGINAL_RUNTIME_ROUTE_MISSING_F0444_TRACE`
- parity claim: **not made**
- promoted original runtime route: `false`
- manifest: `/home/trv2/work/firestaff/parity-evidence/verification/pass465_dm1_v1_endgame_original_route_gate/manifest.json`

## ReDMCSB-first source audit anchors

- ENDGAME.C:94 — defines F0444_STARTEND_Endgame — ok=True
- ENDGAME.C:961 — F0446 fuse sequence enters F0444(true) — ok=True
- PROJEXPL.C:1123 — fuse action path calls F0446 — ok=True
- MENU.C:1499 — champion action dispatch handles Fuse — ok=True
- MENU.C:1504 — Fuse dispatch calls group fuse action — ok=True
- TIMELINE.C:1338 — end-game sensor can enter F0444(true) — ok=True
- GAMELOOP.C:329 — game loop calls F0444 after party death/win loop — ok=True
- STARTUP2.C:1453 — PC34 startup load-failure route calls F0444(false) — ok=True
- TITLE.C:12 — TITLE.C defines title draw anchor — ok=True
- TITLE.C:94 — TITLE.C loads/draws C001_GRAPHIC_TITLE — ok=True

## N2 PC34 original data identity

- DM.EXE size=11471 sha256=4c79b43276f1eb31… exists=True
- FIRES size=94779 sha256=ebf84045c3edbce7… exists=True
- VGA size=4503 sha256=4d9815e777e135bf… exists=True
- TITLE size=12002 sha256=adc7f1916eeef343… exists=True
- DATA/DUNGEON.DAT size=33357 sha256=d90b6b1c38fd17e4… exists=True
- DATA/GRAPHICS.DAT size=363417 sha256=2c3aa836925c64c0… exists=True

## Symbol-map F0444 audit

- symbol map: /home/trv2/work/firestaff/data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json
- verified_runtime_hit: False
- matching entries: 0

## Blocker

The source route to `ENDGAME.C:F0444_STARTEND_Endgame` is locked, including `TITLE.C` title anchors, but this pass did not find a promotable stock PC34 runtime route/save-state because the required runtime evidence is absent or unverified:

- missing `/home/trv2/work/firestaff/parity-evidence/verification/pass465_dm1_v1_endgame_original_runtime_trace/trace.ndjson`
- missing `symbol_map:F0444_STARTEND_Endgame confidence=verified_runtime_hit`

Required next evidence: a stock `FIRES` debugger trace or save-state route whose symbol map marks `F0444_STARTEND_Endgame` as `verified_runtime_hit`. Screenshots or Firestaff/ReDMCSB rebuilt execution are insufficient.

Non-claims: no pixel/video parity claim, no pushed changes, no DANNESBURK data, no raw debugger log promotion.
