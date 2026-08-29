# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Complete the native PC-DOS WIELD command owner before promoting the retail
  `SKSAVE1` creature-drop regression: ZIP-backed resume, pit, stair and door
  paths run in RAM, and the real save's DB4 records and WIELD commands are
  admitted. The current diagnostic target sweep is deliberately not an
  authentic player trajectory: it positions the restored party beside source
  DB4 records to inspect command binding. The read-only receipt now uses the
  source `DM2_USE_DEXTERITY_ATTRIBUTE` route (SKWINSPX v5 `skhero.cpp`
  1965--1998) rather than the raw saved dexterity byte; the supplied
  `SKSAVE1` currently reaches an effective dexterity of 21 on its inspected
  `0x140c` → `0x1116` route, but legitimately misses the selected real target.
  It must not be promoted as a death/drop regression until an original
  input-to-CD/RAM trace supplies a valid encounter, weapon choice and combat
  timing. Do not replace it with a generated weapon, creature, save, or
  combat result.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
