# DM1 V1 F0128 targeted D0 transactions

Status: implemented and locally verified.

ReDMCSB `DUNVIEW.C` F0125, F0126 and F0127 complete D0L, D0R and D0C as
separate terminal square transactions. Firestaff now invokes the verified
per-square scheduler callback with those exact three square identities in that
order. D0L's field tail therefore completes before D0R begins, and D0R
completes before D0C's pit/stair/ceiling material.

D0C remains partitioned exactly where the source requires it: primitives,
F0115 floor items/projectiles/restarted explosions, then F0113 field. Raster
consumers continue to resolve mounted retail PC 3.4 GRAPHICS.DAT material; this
change adds no synthetic pixels and performs no asset extraction.

Verification:

- `dm1_v1_f0128_d0_material_order_source_gate`
- `dm1_v1_f0128_square_material_source_gate`
- `m11_dm1_hoc_orientation_runtime_pc34`

This does not claim pixel-perfect comparison against an original DOS capture.

The same audit found and removed the last direct broad stairs call after the
per-square D3--D1 transactions. Because an invalid scheduler plan returns
before viewport background composition, the fallback could never run; all
admitted F0104 stairs material is now exclusively owned by its exact F0128
square callback.
