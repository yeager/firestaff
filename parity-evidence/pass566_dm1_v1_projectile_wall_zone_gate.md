# pass566 DM1 V1 projectile wall-zone gate

This evidence gate ties three source-locked facts together:

- PROJEXPL.C:F0219 checks cross-square projectile destination walls, closed fake walls and stairs-to-stairs before calling F0267_MOVE_GetMoveResult_CPSCE.
- MOVESENS.C:F0267 has its own party/group projectile-impact precheck before dungeon movement commits.
- DUNVIEW.C:F0115 draws surviving projectile/thrown-object things only through the G2028/C2900_ZONE_ viewport row/cell zones, while wall-square draw functions return before F0115 unless a front alcove explicitly branches to the thing layer.

Local coverage:
- test_dm1_v1_viewport_3d_pc34_compat.c::test_projectile_wall_zone_movement_visibility_gate
- tools/verify_dm1_v1_projectile_wall_zone_gate.py
- parity-evidence/verification/pass566_dm1_v1_projectile_wall_zone_gate/manifest.json

Non-claims: no original runtime launch, no DUNGEON.DAT/GRAPHICS.DAT read, no pixel parity claim.
