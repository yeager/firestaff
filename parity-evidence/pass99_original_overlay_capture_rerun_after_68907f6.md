# Pass 99 — original overlay/capture rerun after `68907f6` on N2

Date: 2026-04-28
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
HEAD during rerun: `848f69d` (`Fix M12 modern menu mouse probes`)
Relevant prior fix: `68907f6` (`Fix original graphics probe bitmap buffer sizing`)
Run directory: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-0650-lane4-overlay-capture-rerun-after-68907f6/`
Original PC 3.4 data: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT`

## Scope

Lane 4 evidence-only rerun for original overlay/capture after `68907f6`. No viewport/HUD/gameplay/V2 code was changed.

## Commands rerun

| Command | rc | Evidence |
| --- | ---: | --- |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | `capture_smoke.out`: `In-game capture smoke PASS: 6 screenshots`. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh` | 0 | `build_visible_dispatch.out`: built wrapper successfully. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh` | 0 | `build_present.out`: built wrapper successfully. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh` | 0 | `build_screen.out`: built wrapper successfully. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index78.pgm" 78` | 0 | `ok`, `kind=BITMAP_SAFE`, `width=224`, `height=97`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 78` | 0 | `ok`, `width=224`, `height=97`, `presentedBytes=10864`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 78` | 0 | `ok`, `width=224`, `height=97`, `screenCopiedBytes=10864`. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index79.pgm" 79` | 0 | `ok`, `kind=BITMAP_SAFE`, `width=224`, `height=39`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 79` | 0 | `ok`, `width=224`, `height=39`, `presentedBytes=4368`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 79` | 0 | `ok`, `width=224`, `height=39`, `screenCopiedBytes=4368`. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index0.pgm" 0` | 0 | `ok`, `kind=BITMAP_SAFE`, `width=224`, `height=136`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 0` | 0 | `ok`, `width=224`, `height=136`, `presentedBytes=15232`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 0` | 0 | `ok`, `width=224`, `height=136`, `screenCopiedBytes=15232`. |
| `python3 tools/pass84_original_overlay_readiness_probe.py` | 0 | Saved as `parity-evidence/pass99_original_overlay_capture_readiness_after_68907f6.json`. |

## Captured original-data artifacts

| Artifact | File type | SHA256 |
| --- | --- | --- |
| `$RUN_DIR/original_visible_dispatch_index0.pgm` | PGM 224x136 | `3a607a1dc601664fb3a2183c984d81cdf9cab351e65c4f3098f82df939fa5ea5` |
| `$RUN_DIR/original_visible_dispatch_index78.pgm` | PGM 224x97 | `a76168baf4c4f7b4d78ecb0bacbb4060b230296003ece56544466b1c364f8b1b` |
| `$RUN_DIR/original_visible_dispatch_index79.pgm` | PGM 224x39 | `7ae3b0a77c5e7e85ce4e3e32b03f42029b66cbd4eda94808c6fb69e680d76250` |

## Result

The `68907f6` buffer-sizing fix is verified green on N2 for the original-data visible dispatch, present, and screen seams, including the previously failing default graphic index `0`. The older index-0 heap abort/presentation mismatch recorded in pass 97 did **not** reproduce in this rerun.

Remaining blocker is higher-level semantic route readiness, not this low-level original graphics/capture seam: the readiness probe still reports `ready_for_overlay_comparison=false` because pass74/pass78/default-route artifacts and semantic original runtime shot labels are not yet landable.
