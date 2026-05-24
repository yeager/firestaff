# DM1 V1 Bug Tracking Audit
## Source-Lock: ReDMCSB WIP20210206 / Toolchains/Common/Source/

## ReDMCSB Bug Index (BUG0_* in DUNVIEW.C and other sources)

BUG0_00: Useless code - PRESENT in Firestaff (various, no gameplay consequence)
BUG0_01: Coding error without consequence - PRESENT in Firestaff
BUG0_02: Timeline 24-bit overflow at ~850-1000 hours - GAME HANGS - NOT FIXED
BUG0_03: VBlank interrupt timing graphical glitch - MINOR - NOT FIXED
BUG0_04: Creature drawn with unexpected colors - NOT FIXED
BUG0_05: Champion portrait sensor visible on all wall sides - NOT FIXED
BUG0_06: Projectile/explosion blit on left viewport edge (flipped) - NOT FIXED
BUG0_07: Explosion blit on left viewport edge (unflipped) - NOT FIXED
BUG0_64: Floor ornaments drawn over open pits - PRESENT as documented draw order
BUG0_83: Thieves Eye door hole moves with opening door - NOT FIXED
BUG0_86: Champion portrait/resurrect/rename graphics missing on custom dungeons - NOT FIXED

## Firestaff Status

BUG0_02 (Timeline overflow): CRITICAL - Game hangs after ~850 hours.
Fix suggestion from ReDMCSB: subtract fixed amount (like 15000000) from time values
when time reaches maximum (like 16000000). No fix implemented, no test exists.

BUG0_04 (Creature colors): Low severity. creature_render_pc34_compat.c handles palette
loading but no isolated test for all creature types.

BUG0_05 (Portrait sensor z-order): Low severity. No test isolates portrait drawn
over wall ornament on same wall square.

BUG0_06/07 (Projectile/explosion blit): NOT FIXED. No BUG0_06/07 comments in
dm1_v1_projectile_explosion_render_pc34_compat.c - fix not ported. Graphical glitch
only on left-edge cropped projectiles/explosions.

BUG0_64 (Floor ornaments over pits): PRESENT. viewport_3d_pc34_compat.c has
comments referencing DUNVIEW.C:6275-6278 etc. with BUG0_64 note as source-lock
documentation. Actual render order may match original bug.

BUG0_83 (Thieves Eye hole): NOT FIXED. No specific test. Door/wall rendering in
viewport_3d_pc34_compat.c handles door ornaments but Thieves Eye animation tied
to door state not independently tested.

BUG0_86 (Champion portrait graphics): NOT FIXED. Affects custom dungeons with
champion mirror maps and creature types. dm1_v1_portrait_panel_pc34_compat.c handles
portrait rendering but no test exercises memory-limitation condition.

## Summary Table

| Bug  | Severity | Firestaff | Test |
|------|----------|-----------|------|
| BUG0_02 Timeline overflow | CRITICAL | NOT FIXED | NO TEST |
| BUG0_03 VBlank timing | MINOR | NOT FIXED | NO TEST |
| BUG0_04 Creature colors | LOW | NOT FIXED | PARTIAL |
| BUG0_05 Portrait sensor z-order | LOW | NOT FIXED | NO TEST |
| BUG0_06 Projectile blit left edge | LOW | NOT FIXED | NO TEST |
| BUG0_07 Explosion blit left edge | LOW | NOT FIXED | NO TEST |
| BUG0_64 Floor ornaments over pits | LOW | MAYBE PRESENT | PARTIAL |
| BUG0_83 Thieves Eye hole animation | MEDIUM | NOT FIXED | NO TEST |
| BUG0_86 Champion portrait graphics | MEDIUM | NOT FIXED | NO TEST |
