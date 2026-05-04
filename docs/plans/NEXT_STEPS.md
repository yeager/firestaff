# ReDMCSB PC 3.4 - next steps

## Version focus

Current priority:
- build **version 1, the original-compatible version** first
- assume original game files are available locally, but keep them separate from the port tree
- do not mix in replacement/custom graphics or audio in this phase

Deferred later:
- **version 2**, a high-resolution version with improved graphics/audio that still **requires the original game files**
- **version 3**, a 3D version that also **requires the original game files**
- because versions 2 and 3 depend on the original game, they can follow the original-compatible naming/product line much more closely than a fully standalone redistributable build
- this is a materially better legal/product position for using the game's real name, but it should still be treated as a product strategy decision, not as a blanket legal guarantee
- version 3 is explicitly last, so current implementation choices should not bend around 3D prematurely

## Already established layers

### Core / probe-carrying layers
- gameplay/UI core via the probe files
- text/font layer
- lightweight draw layer

### Frontend layers
- `image_frontend_pc34.c`
- `base_frontend_pc34.c`

### Compat/backend layers
- `byteops_pc34_compat.c`
- `image_backend_pc34_legacy_contract.md`
- `byteops_backend_pc34_legacy_contract.md`

## Important verified facts

- `IMAGE.C` can stand separately from `IMAGE2/3/4/5`
- `BASE.C` can stand separately from `COPYBYTE/CLEARBYT`
- byteops compat is both syntax-verified and simple-tested
- the combined probe with core + frontend layers passes

## Recommended work order for the next pass

## Current milestone checkpoint

- **M7 is honestly complete**: the composed-backdrop path shows a recognizable, plausibly placed, and stable held title/title-card state on original data, and the stateful reachability seam reports `reachedMenuEstablished=1` and `reachedMenuHeld=1` on `m7_reachability_b`.
- **M8 is now effectively complete**: loop/hold stability survived real repeated cycles, the former `304..319` mystery was reduced to a DM PC v3.4 front-lock-family split, and the remaining `308/312` exception lane now has a concrete content/decode explanation rather than a vague runtime mystery.
- Closing M8 causal test: rewriting `kind6` to `kind4` on real raw IMG3 data flips `308` from strongly positive (`+831`) to negative (`-259`) and substantially weakens `312` (`+858 -> +555`), which is good enough to treat the motif -> decode -> patch chain as operationally demonstrated.
- **M9 is now frozen/verified** as the first honest original-compatible beta slice.
- Unified boot is stable inside the trusted beta harness, the best current title hold can be produced directly via `--title-hold` at `graphic 313`, and `run_redmcsb_m9_verify.sh` is now the top-level verification gate.
- Immediate focus: post-M9 widening, meaning richer submenu/game-state consequences, broader interaction coverage, and better title/menu/submenu fidelity without destabilizing the frozen beta slice.

### 1. Make the frontend files less like extracts and more like real layers
- clean up auto-extraction artifacts where needed
- start replacing direct original structure with clearer layer boundaries
- optionally add includes for:
  - `image_frontend_pc34.h`
  - `byteops_pc34_compat.h`

### 2. Build a probe that uses the new layer names consistently
Goal:
- minimize the dependency on one-off build scripts
- make `image_frontend_pc34.c` and `base_frontend_pc34.c` the primary working files
- keep `image_backend_pc34_compat.c` as an explicit module, not just a hidden implebuttation detail

### 3. Take the next small implementation step in the active beta/runtime path
Already done:
- `F0685_IMG3_LineColorFilling`
- `F0686_IMG_CopyFromPreviousLine`
- first falsecolor/index-aware title exporter
- first composed-screen seam
- first backdrop+viewport title composition path
- reusable late-tail step-identity analysis via `analyze_redmcsb_title_tail_step_identity.py`

Best candidates now:
- keep `m7_reachability_b` and `m8_loop_stability_a` as the stable title/menu baselines, but treat `--title-hold` at `313` as the current best user-facing title artifact
- use `redmcsb_m9_beta_harness` as the main beta-facing artifact instead of stitching together separate probes by hand
- preserve the current M9 beta slice as frozen and honest
- the first M9+ extension is now started as an explicit submenu consequence seam above the current consequence layer: `memory_graphics_dat_submenu_consequence_pc34_compat.{c,h}`
- use `run_redmcsb_m9_verify.sh` as the top-level M9 verification gate, and `run_redmcsb_m9_submenu_matrix.sh` plus `submenu-matrix/submenu_matrix.md` and `submenu-matrix/submenu_invariants.md` as the submenu-specific probe when deepening submenu behavior
- keep the new submenu behavior signals honest: `lastSubmenuBehaviorClass` is last-event semantics, while `submenuCumulativeBehaviorClass` and `submenuBehaviorMask` are cumulative sequence semantics
- keep that new seam conservative for now, then deepen it there instead of widening random assets or reopening boot work
- treat the current command/input stack as exhausted for this purpose: it only distinguishes `ADVANCE -> TICK` vs everything else -> `NONE`, so richer submenu behavior must keep growing above the current consequence layer
- improve title/menu/submenu fidelity now that the runtime no longer dies at unified boot handoff
- preserve and extend the new persistent menu/session state instead of falling back to prefix replay or wider fake wrappers
- document honestly what is still unsupported or fragile for the next beta increment
- investigate the crash band at `671..675`, `677..685`, and `687` separately when it blocks beta readiness
- widen the strict real-candidate window beyond the currently verified dialog `1..3` and viewport `0..15` only when that directly helps beta-facing interaction or stability
- keep going on the MEMORY side only where real beta-facing probes show a concrete missing seam, instead of adding more speculative wrappers

### 4. Stay away from the whole `IMAGE3/5` blob until there is a narrower plan
That is where the asm hell lives.

## If you want fast value
The best next real implementation now is no longer another tiny seam in isolation. It is whichever small step most honestly improves the new beta harness, especially:
- deeper persistent runtime state above the startup-frame driver
- broader menu/submenu semantics on top of the existing persistent session state, with verify-gated sequence semantics now split cleanly into last-event class vs cumulative class/mask
- cleaner user input on top of the now-working session model
