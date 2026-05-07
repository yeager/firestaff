# Pass242 — DM1 V1 DUNVIEW/TCC INT6 build blocker

Status: `PASS_DUNVIEW_TCC_INT6_BLOCKER_CLASSIFIED`

## Finding

The N2 ReDMCSB I34E build is not merely missing a later TLINK/map step. It dies at the authentic `DUNVIEW.C` compile: `DUNVIEW.OBJ` is zero bytes, `FIRES.MAP` is zero bytes, and the DOSBox-X stdout tail contains repeated CPU illegal interrupt 6 reports while MKII.LOG records `dunview.c` errors.

## Evidence

- `MKII.BAT` uses `TCPP101` for this route: line `10`.
- `DUNVIEW.C` compile command: line `473`.
- `FIRES.MAP` TLINK output line: line `510`.
- ReDMCSB toolchain doc names Turbo C++ 1.01 for PC: line `236`.
- pass236 compiled object entries: `28`; `DUNVIEW.OBJ` size `0`; `FIRES.MAP` size `0`.
- pass236 stdout-tail illegal interrupt 6 count: `190`; MKII error excerpt `dunview.c` count: `7`.

## Exact blocker

Produce an emulator/toolchain environment where Turbo C++ 1.01 can finish the `MKII.BAT` I34E `DUNVIEW.C` compile. Until `DUNVIEW.OBJ` is non-zero, the authentic TLINK line cannot emit `FIRES.MAP`, so the DM1 V1 public-symbol route remains blocked.

## Next practical routes

1. Try the same HARDDISK tree under a different DOS emulator/VM/real DOS instead of current DOSBox-X.
2. Debug DOSBox-X INT6 behavior around the TCPP101 `DUNVIEW.C` compile.
3. Obtain a trusted `FIRES.MAP` from an authentic ReDMCSB I34E build with the same `MKII.BAT`/`I34E.LNK` order.

Evidence manifest: `parity-evidence/verification/pass242_dm1_v1_dunview_tcc_int6_blocker/manifest.json`.
