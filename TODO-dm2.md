# Firestaff TODO — DM2

Reviewed 2026-08-25. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
- Complete the Amiga outdoor HUD command route: authentic startup currently
  reaches map 0 with the source floor, ceiling and `INTERFACE_GENERAL/0`
  palette, but its frame is correctly rejected until all source HUD command
  materials are bound. Do not relax `dm2_v1_amiga_native_cli_boot`.
