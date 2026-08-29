# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Fix the native PC-DOS `SKSAVE1` → M11 Fireball admission path using the
  source `DM2_CAST_SPELL_PLAYER` receipt. The retail ZIP boots and resumes in
  RAM, and `YA FUL IR` resolves to fixed spell 16, but every eligible saved
  hero is rejected by the source skill/mana/cast-threshold gate. Do not alter
  hero stats, mana, runes or save bytes to make this pass; expose the actual
  receipt and implement the missing source owner instead.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
