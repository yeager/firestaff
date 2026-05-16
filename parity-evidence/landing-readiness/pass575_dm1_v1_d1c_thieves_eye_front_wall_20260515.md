# Pass575 landing readiness manifest

Status: LANDABLE

Branch: worker/pass575-dm1v1-front-wall-thieves-eye-source-lock-20260515-codex
Commit: a3ea6a49
Base: origin/main f280fc75
Worktree inspected: /home/trv2/work/firestaff-worktrees/pass575-dm1v1-front-wall-thieves-eye-source-lock-20260515-codex

## Scope

Pass575 adds source-lock evidence for DM1 V1 D1C front-wall Thieves Eye handling. It is a metadata/verifier/test-surface branch only; no renderer behavior change is claimed.

## Artifact Sweep

- build-pass575: absent in inspected worker worktree
- untracked files: none reported by git status --porcelain --untracked-files=all
- ignored build artifact check: no build-pass575 path present

## ReDMCSB Audit Anchor

Source root: /home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/
File: DUNVIEW.C
SHA256: a1eb2774b6e3962e51361aac67da50c2cfb40daf25211c449586dc3ffdfd8846

Function and lines:
- DUNVIEW.C:7727 F0124_DUNGEONVIEW_DrawSquareD1C starts.
- DUNVIEW.C:7784 enters case C00_ELEMENT_WALL.
- DUNVIEW.C:7789 checks G0407_s_Party.Event73Count_ThievesEye.
- DUNVIEW.C:7790 caches C001_DERIVED_BITMAP_THIEVES_EYE_VISIBLE_AREA.
- DUNVIEW.C:7805 copies the hole bitmap dimensions into the Thieves Eye visible-area derived bitmap.
- DUNVIEW.C:7819 saves the viewport visible area into the derived bitmap.
- DUNVIEW.C:7820 composites C041_GRAPHIC_HOLE_IN_WALL over the saved visible area with flesh transparency.
- DUNVIEW.C:7834 draws G2107_WallSet[C04_WALL_D1C] to C712_ZONE_WALL_D1C for I34E/I34M/P31J.
- DUNVIEW.C:7842 tests whether the front wall ornament is an alcove.
- DUNVIEW.C:7843 draws objects/creatures/projectiles/explosions in C0x0000_CELL_ORDER_ALCOVE for that alcove case.
- DUNVIEW.C:7866 restores the Thieves Eye visible area to G0296_puc_Bitmap_Viewport with C09_COLOR_GOLD transparency.
- DUNVIEW.C:7868 re-adds the derived bitmap to cache.
- DUNVIEW.C:7869 releases the derived bitmap block.
- DUNVIEW.C:7872 returns before C17_ELEMENT_DOOR_FRONT handling begins at DUNVIEW.C:7873.

## Original Game Data

No dungeon.dat or graphics.dat files were read for this readiness pass, so no original-data hash lock is required.

## Verification

Commands run in the inspected pass575 worktree:

- git status --short --branch
  - clean, ahead of origin/main by 1 commit
- git diff --check origin/main...HEAD
  - pass, no output
- python3 tools/verify_pass575_dm1_v1_d1c_thieves_eye_front_wall_source_lock.py
  - PASS575_DM1_V1_D1C_THIEVES_EYE_FRONT_WALL_SOURCE_LOCKED
- git status --porcelain --untracked-files=all
  - pass, no output

## Landing Readiness

Ready for landing as a source-lock evidence/test branch. No cleanup commit was needed on the pass575 branch because the previously reported build-pass575 artifact is absent and the worktree is clean.
