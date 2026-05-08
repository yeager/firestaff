# DM1 V1 viewport explosion pass timing gap — source lock (2026-05-08)

Scope: DM1 V1 movement/viewport/walls only. This is verifier/blocker-narrowing evidence, not a renderer rewrite and not a pixel-parity claim.

## ReDMCSB anchors

- `DUNVIEW.C:4547-4582` defines `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: each packed view-cell pass draws objects, then one creature cell, then projectiles; explosions are a later phase.
- `DUNVIEW.C:5679-5683` restarts the square thing list for the projectile pass and filters `C14_THING_TYPE_PROJECTILE` for the current view cell.
- `DUNVIEW.C:5915-5933` closes `} while (L0130_ul_RemainingViewCellOrdinalsToProcess);`, then enters `/* Draw explosions */`, restarts from `L0146_T_FirstThingToDraw`, and filters `C15_THING_TYPE_EXPLOSION`.

## Firestaff timing gap characterized

Current `m11_game_view.c` coalesces projectiles and explosions through `m11_draw_effect_cue()`: it draws `cell->summary.projectiles > 0` and then `cell->summary.explosions > 0` in the same per-open-cell cue. `m11_draw_wall_contents()` calls that cue as a local “Layer 3: Projectiles and explosions”.

That preserves *within-cell* projectile-before-explosion order, but it does not preserve ReDMCSB F0115 timing: explosions must wait until after all packed cells have finished their object/creature/projectile phases.

## Minimal fix proposal

Split the current effect cue into two source-owned phases:

1. Per packed/open cell: draw projectile cues/sprites only at the existing point after creatures.
2. After the packed cell traversal completes: run one deferred explosion pass over the same ordered visible cells and draw explosion/fluxcage cues/sprites.

Do not change projectile/explosion gameplay state, thing summaries, wall occlusion, or bitmap family selection in this lane. The only intended behavior change is moving explosion drawing out of the per-cell cue and into the F0115 after-all-cells pass.

## Status

Blocker narrowed: current renderer timing is definitively different from ReDMCSB F0115 for explosion layering across multiple visible cells. The next safe implementation is a small draw-pipeline split, gated by the source-lock verifier added with this evidence.
