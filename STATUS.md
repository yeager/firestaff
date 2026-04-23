# ReDMCSB PC 3.4 status

## Product direction

There will be three distinct versions of this project:

1. **Original-compatible version**
   - stays as faithful and untouched as possible
   - requires the original game files
   - is the version currently being built

2. **High-resolution original-required version**
   - uses improved/replacement graphics and improved audio
   - still requires the original game files
   - is therefore not planned as a fully standalone redistributable build
   - can follow the original game's naming/product identity much more closely than a standalone rewrite
   - is deferred until the original-compatible version is solid

3. **3D original-required version**
   - follows the same product stance as version 2: improved presentation, but still requires the original game files
   - extends that original-required direction into 3D presentation
   - is explicitly the last major version and should not distort current milestone choices

For the current workstream, assume the target is **version 1, the original-compatible version**, unless a note explicitly says otherwise.

Product/legal stance update:
- versions 2 and 3 are no longer planned as standalone legal-to-distribute builds
- instead they should require the original game, which gives a much stronger basis for using the real game name/product line
- that is still a practical product/legal strategy, not a formal legal opinion or guarantee

## Current state

The porting work has moved from pure mapping into a first working layer structure, and the active M9 work now has a real beta-facing harness instead of only isolated probes.

Bounded V2 note:
- the opt-in `FIRESTAFF_V2_VERTICAL_SLICE=1` path now includes a first shared four-slot party HUD strip expansion on top of the earlier status-box and party-HUD-cell slice assets; this is a bounded presentation pass only, not a full portrait or typography system
- a first bounded V2 initial-4K render path now exists via `firestaff_m11_v2_initial_4k_capture` + `tools/render_v2_initial_4k.py`, using the Wave 1 4K UI shells and a first-pass skeleton creature family for an honest offline in-game composition

### Available now
- reproducible syntax probes
- auto-flattening for internall `.C` includes
- a runnable original-data beta harness:
  - `redmcsb_m9_beta_harness.c`
  - `run_redmcsb_m9_beta_harness.sh`
- extracted frontend files:
  - `image_frontend_pc34.c`
  - `base_frontend_pc34.c`
- real compat layers:
  - `byteops_pc34_compat.c`
  - `image_backend_pc34_compat.c`
- explicit layer headers:
  - `byteops_pc34_compat.h`
  - `image_backend_pc34_compat.h`
  - `image_frontend_pc34.h`
- tests:
  - `test_byteops_pc34_compat.c`
  - `test_image_backend_pc34_compat.c`

### Verified
- `IMAGE.C` is separable from `IMAGE2/3/4/5`
- `BASE.C` is separable from `COPYBYTE/CLEARBYT`
- the byteops compat layer passes tests (`ok`)
- the image backend compat layer passes tests (`ok`)
- the combined probe with both frontend layers works
- the image backend compat layer now contains nibble reading, pixel-count decoding, and destination writing
- the image backend compat layer now has a first small command executor for the IMG3 same-stride case
- the image backend compat layer now also handles padded-width / line-break logic for single IMG3 commands
- the image backend compat layer now has a first small multi-command loop for IMG3
- the image backend compat layer can now expand a minimal IMG3 source buffer end-to-end
- the literal-color case in IMG3's end-to-end path is now verified, including count ordering
- same-stride copy-with-count and padded literal-with-count are now verified through a real source buffer
- the compat core is now connected to a small probe/integration path via a wrapper around the ported IMG3 expander
- that wrapper is now lifted into a real adapter module: `image_expand_pc34_compat.{c,h}`
- the compat core is now also connected through a larger EXPAND-like call-site layer: `expand_frontend_pc34_compat.{c,h}`
- the next real caller above that now exists as a narrow SWSH path: `swsh_frontend_pc34_compat.{c,h}`
- the simplest bitmap/screen split on the SELECTOR side now also exists as a narrow compat path: `selector_frontend_pc34_compat.{c,h}`
- EXPAND/SELECTOR/SWSH now share a common bitmap-call layer: `bitmap_call_pc34_compat.{c,h}`
- a MEMORY-like caller above EXPAND now also exists as a compat path: `memory_frontend_pc34_compat.{c,h}`
- the DIALOG backdrop path above MEMORY now also exists as a compat path: `dialog_frontend_pc34_compat.{c,h}`
- the ENDGAME credits path above the shared bitmap chain now also exists as a compat path: `endgame_frontend_pc34_compat.{c,h}`
- the first cache/MEMORY-adjacent build-and-return path now also exists as a compat path: `memory_cache_frontend_pc34_compat.{c,h}`
- the build-or-reuse pattern on the MEMORY side now also exists as a compat path: `memory_cache_reuse_pc34_compat.{c,h}`
- a simple block-metadata edge on the MEMORY/cache side now also exists as a compat path: `memory_cache_block_pc34_compat.{c,h}`
- the block-index cache-hit/miss path on the MEMORY side now also exists as a compat path: `memory_cache_index_pc34_compat.{c,h}`
- the first derived-bitmap cache path now also exists as a compat path: `derived_bitmap_cache_pc34_compat.{c,h}`
- the bitmap copy helpers from `MEMORY.C` now also exist as a compat path: `bitmap_copy_pc34_compat.{c,h}`
- the apply step in the MEMORY load/decompress/expand flow now also exists as a compat path: `memory_load_expand_pc34_compat.{c,h}`
- the bytecount/allocation wrappers above that same load/decompress/expand flow now also exist as a compat path: `memory_load_wrappers_pc34_compat.{c,h}`
- used-list / usage-count bookkeeping on the cache side now also exists as a compat path: `memory_cache_usage_pc34_compat.{c,h}`
- native/derived index access is now connected to the bookkeeping layer via `memory_cache_access_pc34_compat.{c,h}`
- reset-time / clock-gating for usage counts now also exists as a compat path: `memory_cache_timegate_pc34_compat.{c,h}`
- the time gate is now also connected in front of native/derived access via `memory_cache_gated_access_pc34_compat.{c,h}`
- `F0489` now also has its early graphic-vs-bitmap branch in `memory_native_or_graphic_pc34_compat.{c,h}`
- the cache allocation boundary before the real allocator/defrag now also exists in `memory_cache_allocation_boundary_pc34_compat.{c,h}`
- the first allocator / unused-block seam now also exists in `memory_cache_unused_blocks_pc34_compat.{c,h}`
- the focused reuse/split path above the unused-block seam now also exists in `memory_cache_reuse_split_pc34_compat.{c,h}`
- the top-of-cache-vs-reuse branch in `F0483` now also exists in `memory_cache_allocate_or_reuse_pc34_compat.{c,h}`
- the defrag-vs-reuse decision when the largest unused block is too small now also exists in `memory_cache_defrag_branch_pc34_compat.{c,h}`
- the first focused `F0483` orchestrator now also exists in `memory_cache_allocator_pc34_compat.{c,h}`
- the post-defrag result now also exists in `memory_cache_defrag_result_pc34_compat.{c,h}`
- the allocator orchestrator is now composed with the post-defrag result via `memory_cache_allocator_defrag_pc34_compat.{c,h}`
- the first used-block compaction seam now also exists in `memory_cache_compact_used_pc34_compat.{c,h}`
- the first defrag loop/orchestrator for mixed used/unused segments now also exists in `memory_cache_defrag_loop_pc34_compat.{c,h}`
- the `F0482` miniature now also exists as a composed defrag-loop + apply path in `memory_cache_defrag_apply_pc34_compat.{c,h}`
- the segment scan before the defrag loop now also exists in `memory_cache_segbutt_scan_pc34_compat.{c,h}`
- the small `F0482` orchestrator now also exists in `memory_cache_defrag_orchestrator_pc34_compat.{c,h}`
- `F0490` now also has its first real session / buffer orchestration in `memory_load_session_pc34_compat.{c,h}`
- `F0490` now also has a first transaction shell in `memory_load_transaction_pc34_compat.{c,h}`
- the wrapper layer above `F0490` is now also connected to the transaction seam via `memory_load_wrappers_transaction_pc34_compat.{c,h}`
- the load/decompress side now also has a small pipeline orchestrator in `memory_load_pipeline_pc34_compat.{c,h}`
- the first real `GRAPHICS.DAT` refcounted file/session seam now also exists in `memory_graphics_dat_pc34_compat.{c,h}`
- the first file-backed `F0490` transaction path now also exists in `memory_graphics_dat_transaction_pc34_compat.{c,h}`
- the metadata-driven offset seam above `GRAPHICS.DAT` access now also exists in `memory_graphics_dat_metadata_pc34_compat.{c,h}`
- real `GRAPHICS.DAT` header parsing into per-graphic metadata tables now also exists in `memory_graphics_dat_header_pc34_compat.{c,h}`
- metadata-driven graphic selection now also exists in `memory_graphics_dat_select_pc34_compat.{c,h}`
- parsed-metadata composition into the file-backed transaction path now also exists in `memory_graphics_dat_composed_transaction_pc34_compat.{c,h}`
- reusable runtime metadata state for `GRAPHICS.DAT` now also exists in `memory_graphics_dat_state_pc34_compat.{c,h}`
- runtime-state-driven file-backed transactions now also exist in `memory_graphics_dat_runtime_transaction_pc34_compat.{c,h}`
- persistent loaded-graphic slots above the runtime metadata state now also exist in `memory_graphics_dat_slots_pc34_compat.{c,h}`
- a persistent dialog-box style special-graphic preload path now also exists in `memory_graphics_dat_special_pc34_compat.{c,h}`
- a dialog-facing bitmap caller above that special preload path now also exists in `memory_graphics_dat_dialog_path_pc34_compat.{c,h}`
- a viewport-facing graphic caller driven by runtime `GRAPHICS.DAT` metadata now also exists in `memory_graphics_dat_viewport_path_pc34_compat.{c,h}`
- a MEMORY-owned bitmap path above the viewport/runtime chain now also exists in `memory_graphics_dat_bitmap_path_pc34_compat.{c,h}`
- an ownership-reuse seam above that MEMORY-owned bitmap path now also exists in `memory_graphics_dat_bitmap_reuse_pc34_compat.{c,h}`
- a block-index cache lookup/fallback seam above that path now also exists in `memory_graphics_dat_block_index_pc34_compat.{c,h}`
- a used-list / usage-bookkeeping seam above that block-index path now also exists in `memory_graphics_dat_used_list_pc34_compat.{c,h}`
- an allocator-boundary seam above that used-list path now also exists in `memory_graphics_dat_allocator_boundary_pc34_compat.{c,h}`
- an allocator-entry seam above that boundary gate now also exists in `memory_graphics_dat_allocator_entry_pc34_compat.{c,h}`
- a defrag-entry seam above that allocator admission now also exists in `memory_graphics_dat_defrag_entry_pc34_compat.{c,h}`
- a defrag-loop/orchestrator seam above that defrag-entry path now also exists in `memory_graphics_dat_defrag_loop_pc34_compat.{c,h}`
- an allocator-orchestrator seam above that defrag-loop path now also exists in `memory_graphics_dat_allocator_orchestrator_pc34_compat.{c,h}`
- a first small startup-asset seam above the new `GRAPHICS.DAT` runtime chain now also exists in `memory_graphics_dat_startup_pc34_compat.{c,h}`
- a first tiny boot/init control-flow seam above that startup-asset phase now also exists in `memory_graphics_dat_boot_pc34_compat.{c,h}`
- a higher startup-wiring seam above that boot/init flow now also exists in `memory_graphics_dat_startup_wiring_pc34_compat.{c,h}`
- a first-frame dispatch seam above that startup-wiring layer now also exists in `memory_graphics_dat_first_frame_pc34_compat.{c,h}`
- a startup-dispatch seam above that first-frame layer now also exists in `memory_graphics_dat_startup_dispatch_pc34_compat.{c,h}`
- a main-loop-entry seam above that startup-dispatch layer now also exists in `memory_graphics_dat_main_loop_entry_pc34_compat.{c,h}`
- a startup-tick seam above that main-loop-entry layer now also exists in `memory_graphics_dat_startup_tick_pc34_compat.{c,h}`
- a first tiny command-facing seam above startup-tick/main-loop entry now also exists in `memory_graphics_dat_main_loop_command_pc34_compat.{c,h}`
- a tiny repeated-command-loop seam above that command layer now also exists in `memory_graphics_dat_main_loop_command_loop_pc34_compat.{c,h}`
- a tiny queued-command seam above that loop layer now also exists in `memory_graphics_dat_main_loop_command_queue_pc34_compat.{c,h}`
- a first typed-command queue seam above that queue layer now also exists in `memory_graphics_dat_main_loop_typed_command_queue_pc34_compat.{c,h}`
- a first explicit input-to-command queue seam above that typed queue now also exists in `memory_graphics_dat_input_command_queue_pc34_compat.{c,h}`
- a first explicit event-dispatch seam above that input queue now also exists in `memory_graphics_dat_event_dispatch_pc34_compat.{c,h}`
- a first explicit menu-state seam above that event dispatcher now also exists in `memory_graphics_dat_menu_state_pc34_compat.{c,h}`
- a first explicit menu-render-effect seam above that state layer now also exists in `memory_graphics_dat_menu_render_effect_pc34_compat.{c,h}`
- those command/input/menu/activate seams are now packaged into a runnable M9 harness that:
  - boots to held menu on original data
  - accepts user-provided event sequences
  - publishes visible PGM frames for boot and interaction steps
  - supports line-buffered session mode
  - carries persistent menu/session state across steps for selection, screen, render graphic, and last activation
  - now also supports unified boot publication plus direct title-hold entrypoints (`--unified-boot`, `--unified-boot-stop`, `--title-hold`)
- verified unified one-shot `fnnaf` reaches the known result: `finalSelectionIndex=2`, `activatedGraphicIndex=13`, `finalScreen=1`
- verified session mode with `fn` then `af` accumulates honestly across chunks and stays latched on the submenu-side activated render
- the former unified boot crash/blocker is now resolved inside the trusted beta harness rather than worked around by external probes
- the best current practical title hold is now fixed at `graphic 313`, and repeated hold frames are pixel-identical, making it a real stable title demo state instead of a single-frame accident
- a top-level verification gate now exists as `run_redmcsb_m9_verify.sh`, checking title-hold semantics, repeated title-hold identity, submenu artifact production, submenu invariant PASS state, and expected submenu-matrix row coverage in one run
- that submenu-side verification now also carries explicit last-event behavior classes plus cumulative behavior masks, so the frozen beta slice is guarded by compact semantic submenu signals instead of raw counters alone
- a first explicit menu-activate seam above that render layer now also exists in `memory_graphics_dat_menu_activate_pc34_compat.{c,h}`
- a first explicit post-activate consequence seam above that activate layer now also exists in `memory_graphics_dat_menu_activate_consequence_pc34_compat.{c,h}`
- the submenu consequence seam now also exposes a cumulative behavior class alongside the existing last-event class and cumulative mask, so mixed submenu sequences can be reported honestly as mixed at sequence scope without lying about the last event
- a first original-data-driven startup probe now also exists in `redmcsb_memory_graphics_dat_original_startup_probe.c`
- a strict original-data-driven startup probe using the real expand/apply path now also exists in `redmcsb_memory_graphics_dat_original_startup_strict_probe.c`
- a wider strict original-data startup scan now also exists in `scan_redmcsb_memory_graphics_dat_original_startup_strict.py`
- a first real original-data presentation probe now also exists in `redmcsb_memory_graphics_dat_original_present_probe.c`
- a first simple screen/framebuffer seam above that presentation path now also exists in `screen_bitmap_present_pc34_compat.{c,h}` and `redmcsb_memory_graphics_dat_original_screen_probe.c`
- a first visible-output/export seam above that screen path now also exists in `screen_bitmap_export_pgm_pc34_compat.{c,h}` and `redmcsb_memory_graphics_dat_original_visible_probe.c`
- a wider visible real-asset scan now also exists in `scan_redmcsb_memory_graphics_dat_original_visible.py`
- late-file visible export failure `694` is now isolated and tentatively identified as a non-bitmap/text-like asset rather than a normal bitmap decode failure
- the late region `688..712` now looks like a mixed/special asset block rather than a clean homogeneous bitmap tail
- the late region now also has a concrete working taxonomy: bitmap-safe, metadata-suspicious-but-exportable, clearly special/non-bitmap (`694`), and empty placeholders (`697..700`)
- a taxonomy-aware full-file filtered visible export pass now succeeds for 708 entries with 0 failures and 5 intentional skips
- a minimal host video backend seam now also exists in `host_video_pgm_backend_pc34_compat.{c,h}` and is verified against real original data
- a minimal startup/runtime driver seam now also exists in `startup_runtime_driver_pc34_compat.{c,h}` and is verified against real original data
- a minimal multi-frame runtime seam now also exists in `multi_frame_runtime_driver_pc34_compat.{c,h}` and is verified against real original data
- a minimal stateful startup/runtime seam now also exists in `stateful_runtime_driver_pc34_compat.{c,h}` and is verified against real original data
- a minimal boot-sequence orchestrator now also exists in `boot_sequence_runtime_pc34_compat.{c,h}` and is verified against real original data
- a minimal ordered boot-program layer now also exists in `boot_program_runtime_pc34_compat.{c,h}` and is verified against real original data
- a minimal late special-entry dispatch seam now also exists in `late_special_dispatch_pc34_compat.{c,h}` and is verified
- that late special-entry dispatch seam is now also wired into a real visible export flow via `redmcsb_memory_graphics_dat_original_visible_dispatch_probe.c`
- the dispatcher is now also wired into the broader filtered whole-file export pass, which still succeeds for 708 entries with 0 failures and 5 dispatcher-driven skips
- the startup runtime driver now also applies the dispatcher before the normal viewport path, so known late special entries can be skipped above export
- the ordered boot-program layer now also tolerates dispatcher-skipped steps and records skipped-vs-published counts honestly
- the higher boot-sequence runtime layer now also distinguishes completed, skipped, and published frames, and can complete honestly without fake publication counts
- a minimal top-level boot-plan orchestrator now also exists in `boot_plan_runtime_pc34_compat.{c,h}` and is verified against real original data
- a reusable scripted boot-plan layer now also exists in `boot_plan_script_pc34_compat.{c,h}` and is verified against real original data with multiple named scripts plus catalog listing
- the scripted boot harness now also has a suite-level real-data probe that runs the full registered script catalog with 3/3 passing
- the scripted boot harness now also carries per-script expected completed/skipped/published counts, and the suite probe asserts those expectations with 0 mismatches
- the script catalog now also includes a more startup-like `steady_startup` script (`0,0,0`) grounded in the previously verified steady startup path
- the script catalog now also includes a more progressive early-startup `startup_ramp` script (`0,0,1,1,2,3`) that passes cleanly on real original data
- the script catalog now also includes a later early-window `late_early_ramp` script (`4,5,6,7`) built from explicitly checked bitmap-safe real indices
- the script catalog now also includes an `early_extended_ramp` script (`8,9,10,11,13,14,15`), while intentionally excluding suspicious `12` (`0x0` output)
- the script catalog now also includes a `mid_safe_ramp` script (`16,17,18,19,20,22,23`), while intentionally excluding suspicious `21` (`0x0` output)
- the script catalog now also includes a later boundary script (`24..31`), a dense later placeholder-cluster script (`80..87`), a long later placeholder-wall region split into five verified scripts (`126..143`, `144..175`, `176..207`, `208..239`, `240..245`), a massive later clean run carried by sixty-five verified clean windows through `656..670`, plus two later placeholder singletons (`676`, `686`), extending verified scripted coverage well beyond the previous `0..23` startup-adjacent range while staying honest about the unstable crash band in `671..687`
- metadata probing now shows that `12`, `21`, `24`, the full cluster `80..85`, and the long later wall `126..245` are genuine zero-sized placeholder entries (`compressed=0`, `decompressed=0`, `0x0`, sharing the next entry's offset), not ordinary decode failures, after which a new clean bitmap-safe run resumes at `246`
- a metadata-aware entry classifier now also exists in `graphics_dat_entry_classify_pc34_compat.{c,h}`, operationalizing `ZERO_SIZED_PLACEHOLDER` separately from late `EMPTY` and `SPECIAL_NON_BITMAP`
- that metadata-aware entry classifier is now also wired into the real visible export probe, so placeholders like `12` skip operationally as placeholders rather than as vague failures
- the startup runtime driver now also uses the metadata-aware entry classifier, so placeholders like `12` skip above export as `ZERO_SIZED_PLACEHOLDER` instead of falling through the normal viewport path
- the boot-program layer now also carries richer skip accounting, distinguishing placeholder skips from late special skips in mixed multi-step sequences
- the repeated multi-frame runtime layer now also distinguishes placeholder-skipped frames from special-skipped frames instead of flattening them into generic skips
- the stateful startup runtime summary now also exposes placeholder-vs-special frame skip counts at its own top level instead of forcing callers to inspect only the nested multi-frame result
- the higher boot-sequence layer now also carries placeholder-vs-special frame skip counts upward instead of only a flat skipped-frame total
- the ordinary boot-sequence probe now also reports those top-level placeholder-vs-special frame skip counts instead of only a flatter nested published-frame summary
- the ordinary single-frame runtime probe now also reports classifier kind plus dispatcher-skip state, and the ordinary multi-frame probe now also reports placeholder-vs-special skip counts directly
- the scripted boot-plan suite now passes with 106/106 scripts after expanding through M4 finalists, M6 boot-attempt variants, and M7 reachability variants, with expectation checks still green on real original data
- the M4 candidate search converged on a stable backdrop-side lead around `368..391` and a stable title/menu-side lead around `272..287` then `304..319`
- the strongest current M6 lead is `m6_boot_attempt_a` (`368..391` then `272..287` then `304..319`), which verifies cleanly with `completed=56`, `skipped=0`, `published=56`
- the strongest current M7 frame-chain lead is `m7_reachability_b` (`368..391` then `272..287` then `304..319` then `312..319`), which verifies cleanly with `completed=64`, `skipped=0`, `published=64`
- a new stateful boot-plan reachability seam now also exists in `stateful_boot_plan_reachability_pc34_compat.{c,h}` plus `redmcsb_original_stateful_boot_plan_reachability_probe.c`
- that new stateful probe upgrades the current M7 lead into explicit higher-level reachability phases and, on real `GRAPHICS.DAT`, reaches `phase=4`, `reachedMenuEstablished=1`, and `reachedMenuHeld=1`
- the title/render investigation now also has a first palette-aware falsecolor exporter in `screen_bitmap_export_ppm_falsecolor_pc34_compat.{c,h}`, which preserves local index semantics better than grayscale but still does not recover the final original RGB palette
- the startup/runtime export path now also has a first composed-screen seam and a first multi-asset backdrop-aware composition path: the preloaded `dialogGraphicIndex=1` asset is a real `320x200` title-card-like backdrop, and composing it with the viewport/title-side bitmap produces the first credible full-screen title-card exports instead of tiny isolated fragments
- the large recognizable title card is now strongly evidenced to be mostly baked into `graphic 1`, while the viewport/title-side path contributes only a small local patch on top of that backdrop
- composed-backdrop exports of the held `m7_reachability_b` tail show a recognizable, plausibly placed, and stable title/title-card-like state across repeated frames, with only minor local patch changes rather than collapse or re-randomization
- the late-tail local patch is now step-mapped with a reusable probe: the residual diff against the baked-in backdrop stays `304`-like across the tail, while the visible local patch follows a repeatable `304/319` alternation and ends on `319`; repeating `312..319` in `m7_reachability_b` reproduces the same mixed pattern instead of collapsing it away
- a reusable analysis script now exists as `analyze_redmcsb_title_tail_step_identity.py`, generating `exports_2026-04-17_patch_candidate_match/tail_step_identity_map.csv` as a stable baseline for future runtime-near tail probes
- this is enough to treat M7 as honestly complete: there is now a recognizable stable title/title-card state with plausible placement and explicit held-state reachability evidence on original data
- a first M8-oriented loop/hold script now also exists as `m8_loop_stability_a`, extending the selective held menu tail by a second hold cycle
- that loop/hold script verifies cleanly on real original data with `completed=72`, `skipped=0`, `published=72`
- through the new stateful seam it reports `holdCompletedStepCount=16`, `holdCycleSize=8`, and `holdCycleCount=2`, which is the first explicit evidence of surviving more than one held menu-side cycle
- the late-tail M8 investigation now also has a reusable runtime-near local probe in `redmcsb_runtime_tail_local_probe.c` (plus build/run wrapper), tied directly to the real boot-plan tail on original data rather than only post-hoc image export comparisons
- that probe confirms that the repeated `312..319` tail in `m7_reachability_b` reproduces the same local numeric behavior already seen in `m6_boot_attempt_a`, so the mixed `304/319` patch pattern is stable across both the original late tail and the held-repeat tail
- raw viewport metadata, nonzero bbox checks, and synthetic menu mini seams were all tested and did not explain the large-overlay split; they are now honest dead ends rather than open speculative branches
- the first real rendering-side discriminator is now signed delta against the baked-in backdrop for the fixed visible patch window `(153,90)..(167,109)`: large `319`-leaning outliers `308` and `312` are strongly net-positive there (`+831`, `+858`), unlike the large `304`-leaning cases
- that signed-delta result also survives spatial splitting: `308` and `312` stay positive across left/right halves, top/bottom halves, and all three row bands, while the large `304`-leaning overlays remain mixed or negative in one or more major regions
- below overlay geometry, the late tail now also has a real IMG3 decode-path seam: raw command-stream metrics from the exact loaded viewport bytes show `308` as a clear decode outlier and `312` as a partial one, and those decode metrics reproduce identically across both M6 and M7
- the strongest current decode-side read is now: inside the kind6-heavy large-overlay branch, `306` differs from `308/312` because it is much more explicit-color / `kind7` weighted and much less `kind3+5` weighted; the direct `img3Kind35To7MilliRatio` is now the tightest tiny decode-side separator for that narrow `306` vs `308/312` question, sharper than the earlier `img3Kind35Minus7Permille` contrast
- the new branch summary confirms that, among the current tiny decode metrics, `img3Kind35To7MilliRatio` tracks the strong positive-delta outlier branch best, while `patchSignedDelta` remains the strongest visible-side separator; `kind3` and `kind5` do not share one common sub-kind signature, so the honest common decode-side description remains structured `kind3+5` mass beating explicit-color `kind7`
- the spoiler pass sharpened this further: high `img3Kind35To7MilliRatio` alone is not enough, because `314`, `316`, and `318` still land visible-`304`; the current best read is now conjunctive, where the `319`-leaning outliers are the cases whose favorable decode mix actually turns into broad multi-band positive replacement across the fixed patch rather than a weak or patchy one
- additional higher seams have now been checked and closed without separating the branch: the synthetic menu-state mini counters are flat, the startup/runtime transaction flags are flat, and even the more tail-connected startup frame-driver readiness/classification seam is flat across `304..319`; the remaining missing seam is therefore not in the currently exposed helper/driver state flags
- the best current content-level seam is now structured `kind4`, not just `kind4` share: `308/312` have only tiny `kind4` runs (max `2..3`), while spoilers carry real blocky `kind4` walls (`314` max `31`, `316` max `155` front-loaded, `318` max `28` tail-loaded)
- spatial profiling tightened that further: `308/312` show only tiny local `kind4` noise on a few rows/columns, while spoilers show broad almost-full-width `kind4` fields with distinct placement profiles (`316` early wall, `318` late staircase wall, `314` split-cluster structure)
- `kind4` placement now also lines up with the visible negative-band damage pattern strongly enough to look mechanistic: `316` behaves like an early-band spoiler, `318` like a late-band spoiler, and `314` like a mixed two-cluster spoiler, while `308/312` avoid structured `kind4` almost entirely
- the full-tail stress test (`304..319`) held up hard: visible-`304` rows average `k4Top/k4Mid/k4Bot = 77.0/21.5/59.8` and `k4Permille = 193.3`, while visible-`319` rows average just `0.8/2.6/0.5` and `k4Permille = 10.1`; spoiler modes vary inside the visible-`304` family, but the family-level split is now very strong
- the finer band-correlation pass says `kind4` is strongest as a family-level separator, not yet a perfect per-band rule: `k4Top -> negTop = +0.510` and `k4Bot -> negBot = +0.574` are real, but the mapping is messy inside visible-`304`, and `306` remains the clearest exception to any `kind4`-alone story
- focused analysis of `306` now points to a second spoiler mode: unlike the structured-`kind4` cases, `306` has very low `kind4` but extremely high `kind7` (`107 permille`), strongly negative `kind35Minus7` (`-76`), and a catastrophic all-band negative patch profile, suggesting an explicit-color / `kind7`-heavy spoiler path
- whole-tail `kind7` scan supports that this is a true exception mode, not a hidden subfamily: `306` is the only row combining very high `kind7` with low `kind4` and negative `kind35Minus7`; `308` has elevated `kind7` but succeeds, and `314` has elevated `kind7` but still looks primarily like a structured-`kind4` spoiler
- direct `kind7` parsing strengthens the `306` exception further: it has large structured `kind7` blocks in a mirrored early/late layout (big runs at the start and near the end), while comparison rows keep `kind7` in the small-fragment regime; this now looks like a genuine structured `kind7` spoiler mechanism, not just a scalar outlier
- but the 306 damage-coupling check is not a clean spatial clone: `kind7` is top/bottom-heavy `(36,5,37)` while the visible negative bands are worst in the middle `(297,419,282)`, so the second mechanism is real but probably indirect or mixed rather than a simple band-for-band transfer
- probing `kind6` as the missing middle layer only partly helps: `306` does have a broad mid-heavy `kind6` field `(70,137,108)` that fits its worst mid-band damage better, but successful rows `308/312` also have large broad `kind6` fields, so `kind6` alone does not explain the exception either
- opening the explicit-color payload sharpens the `306` path substantially: its structured `kind7` is overwhelmingly a single color (`10`) in large mirrored early/late blocks (`72/78` kind7 pixels), unlike the small mixed payloads in `308/312/314`; the second mechanism now looks specifically like a monochromatic structured `kind7` spoiler, not just a generic kind7-heavy outlier
- color `10` itself also appears to have a semantic identity in the extracted/original frontend code as `C10_COLOR_FLESH`, which makes the `306` exception read even tighter: a likely flesh-colored explicit-color wall, not just an opaque palette index anomaly
- source reading of the IMG3 expander pinned down the command semantics: `kind0..5` use the 6-entry local palette, `kind6` copies from the previous line, and `kind7` writes a raw explicit color nibble directly. That means the `306` wall is a true direct color-10 explicit-color structure, not a local-palette aliasing artifact
- tracing post-expand flow narrowed the remaining slot-10 question: the compat bitmap present/overlay path treats color `10` like any other 4-bit nibble and does not reinterpret it after expansion, so whatever makes slot `10` special must live in higher palette/replacement semantics, not a hidden branch in bitmap presentation
- higher source-level palette replacement for slots `9/10` now looks explicitly creature-scoped: ADGE gates the custom slot-10/9 rewrites behind `itemnum >= 446 And itemnum < 532 Then ' creatures`, while our late-tail targets (`306/308/312/314/316/318`) sit below that range. So the monster `ColorA/ColorB` replacement system does not directly explain the late-tail seam
- the DM PC v3.4 item identity for the former "late tail" is now locked by two independent PC-oriented sources (`DMExtract v1.01/MAPS/GRAPHICS.DAT DM PC v3.4 English.map` and Greatstone's `dm_pc_34/graphics.dat` page): title/menu/interface assets are early (`0001..0006`), while `0303..0320` are a regular wall-ornate lock sequence (`Stone`, `Cross`, `Topaz`, `Skeleton`, `Gold`, `Tourquoise`, `Emerald`, `Ruby`, `Ra`) in `Left Side / Front` pairs
- the runtime-tail problem is therefore no longer best described as a generic `304..319` mystery block or a title/intro tail; it is a front-lock-family split inside ordinary PC `IMG3` assets
- left-side lock rows sampled in `m7_reachability_b` stay comparatively small and uniformly `319`-like, while the front rows are much heavier in composed footprint / command count / `kind6` mass and carry the real branch split
- inside that front-lock family, `308` (`Topaz Lock`, Front) and `312` (`Gold Lock`, Front) are the surviving `319`-like exceptions, while `306` (`Cross Lock`, Front) remains the separate explicit-color-10 / structured-`kind7` outlier
- the decoded probe bitmaps themselves now show the same split without relying on IMG3 labels: `308/312` are dense and relatively even, `314/316/318` are internally torn/blocky or hollow, and `306` still sits in its own exception lane. That strengthens the content-level seam again and weakens remaining placement/metadata theories even further
- Greatstone item-image comparison now supports the same read visually: `308` (`Topaz Front`) and `312` (`Gold Front`) are the most row-reuse-friendly front-lock motifs, with stronger symmetry, flatter interiors, and less row-to-row disruption than `304/314/316/318`
- simple motif-mask metrics make that visible read quantitative: `308/312` have many more identical adjacent motif rows and fewer distinct row shapes than the other front-lock images, with `308` showing perfectly stable motif span and `312` collapsing to only `7` distinct mask rows in the whole image
- that motif repetition now correlates directly with the runtime/decode split inside the front-lock family: repeated adjacent motif rows correlate positively with `img3Kind6Permille` (`+0.57`) and very strongly with `patchSignedDelta` (`+0.86`), while correlating negatively with `img3Kind4Permille` (`-0.654`)
- the current best M8 blocker is therefore even narrower: explain why `Topaz Front` and `Gold Front` were drawn as the most row-repetitive front-lock motifs, because that content structure now looks like the clearest upstream cause of the copy-heavy / low-`kind4` / positive-patch exception lane
- a small bitmap-only rule now reproduces the family structure: low row/column spread isolates the clean dense `308/312` pair, dual-high spread isolates torn/blocky spoilers `314/316`, a strong edge-vs-middle hollow score isolates `318`, and the remaining leftover is `306` as the explicit-color-10 exception lane
- when run across the full late tail `304..319`, that bitmap rule generalizes into broader families rather than collapsing: `307/308/311/312/319` all land in the clean-dense lane and all are visible `319` winners, while `314/316/318` remain the strongest structured spoiler family and all are visible `304` winners. The remaining rows behave like intermediate/special cases rather than disproving the rule
- the leftover `other` rows also now show shape instead of pure noise: `305/309/313/315/317` cluster as a thin column-biased but still visible-`319` intermediate lane, `304/310` read as 304-side anchor/milder-spoiler forms, and `306` still stands apart as the explicit-color-10 exception
- the lane model also has sequence logic: visible-`319` rows are exactly the union of `C319` and `I319`, visible-`304` rows are exactly `A304`, `S304`, or `X306`, and the tail end `313..319` falls into an almost metronomic `I319/S304/I319/S304/I319/S304/C319` alternation before resolving at final clean `319`
- staging-vs-emergence now leans clearly toward staging: the per-row visible/signed behavior across `304..319` is identical on `m6_boot_attempt_a`, `m7_reachability_b`, and `m8_loop_stability_a`, and the chosen held repeat tail `312..319` preserves exactly the strongest structured resolving segment instead of collapsing to a trivial final state. That looks more like an authored local-patch progression on top of a stable backdrop than a one-off emergent artifact
- same-format neighbor comparison sharpened the picture again: DMWeb confirms `304..319` are ordinary DM PC v3.4 `IMG3` items inside the big `246..670` block, but raw item attributes show the tail is still unusually choreographed within that ordinary family, with a tight large/small asset alternation (`32x26`/`16x19`-style) that lines up with the discovered `I319` vs `S304/C319` sequence rhythm
- a size-only classifier now adds a second simple layer on top of the bitmap rule: `14x19`/`16x19` strongly predict the 319-side small-support lanes, `16x19` is a pure `I319` signature, `24x26` and `32x28` isolate the `A304` lane, `28x26` isolates `306` as the explicit-color exception, and `30x28` isolates `318` as an `S304` spoiler. The main unresolved size-only ambiguity is `32x26`, which still needs bitmap/content analysis to split `312` from `314/316`
- those two simple layers now combine into a compact full-tail classifier that gets `304..319` exactly right (`16/16`): dimensions provide the scaffold, and bitmap spread resolves the only real same-size ambiguities (`14x19` and `32x26`). That is the first small explicit decision model that reconstructs the whole five-lane tail taxonomy
- the lane taxonomy also compresses into a surprisingly small sequence grammar: the full tail reads as `[A I X] [C C I] [A] [C C I] [S I S I S C]`, while the selected hold tail `312..319` compresses further to `[C] [(I S) x 3] [C]`. That makes the hold-tail staging look even less accidental
- the current M8 blocker is therefore much narrower than before: explain why the `kind6`-heavy overlays with decisively `kind3+5`-over-`kind7` decode mix (`308`, `312`) produce a broad all-band positive replacement profile against the baked-in backdrop while the other large overlays do not
- the new original-data-driven main-loop-command probe now also verifies the first tiny command-facing runtime handoff with `commandStage=2`, `commandReady=1`, `commandHandled=1`, and `processedCommandCount=1`
- the new original-data-driven main-loop-command-loop probe now also verifies a tiny repeated-command loop with `requestedCommandCount=2`, `completedCommandCount=2`, `handledCommandCount=2`, and `commandLoopCompleted=1`
- the new original-data-driven main-loop-command-queue probe now also verifies a tiny queued command sequence with `requestedQueueLength=3`, `completedQueueLength=3`, `handledCommandCount=3`, and `queueCompleted=1`
- the new original-data-driven typed-command-queue probe now also verifies a first tiny semantic split in that queue with `noopCommandCount=1`, `tickCommandCount=2`, and `handledTickCommandCount=2`
- the new original-data-driven input-command-queue probe now also verifies the first explicit input mapping layer with `requestedInputCount=3`, `mappedInputCount=3`, `advanceInputCount=2`, `idleInputCount=1`, and `handledTickCommandCount=2`
- the new original-data-driven event-dispatch probe now also verifies the first explicit event layer with `requestedEventCount=3`, `dispatchedEventCount=3`, `frameEventCount=1`, `advanceEventCount=2`, and `handledTickCommandCount=2`
- the new original-data-driven menu-state probe now also verifies the first tiny state transition layer with `advanceTransitionCount=2`, `frameCount=2`, `initialSelectionIndex=0`, and `finalSelectionIndex=2`
- the new original-data-driven menu-render-effect probe now also verifies the first observable state-driven render choice with `renderVariantCount=3`, `selectedRenderVariant=2`, and `highlightedGraphicIndex=3`
- the new original-data-driven menu-activate probe now also verifies the first explicit activate/select behavior with `activateEventCount=1`, `activationTriggered=1`, `activatedSelectionIndex=2`, and `activatedGraphicIndex=13`
- the new original-data-driven menu-activate-consequence probe now also verifies the first explicit post-activate state consequence with `initialScreen=0`, `finalScreen=1`, and `screenChanged=1`
- the top-level boot-plan layer now also carries richer skip accounting, so placeholder-vs-special skip reasons survive all the way up to the boot-plan result
- the scripted boot-plan catalog now also carries expected placeholder-vs-special skip counts, and the suite asserts them against real original data
- the script catalog now also includes a real mixed-classifier script (`11,12,694,13,21`) that verifies two placeholder skips plus one late special skip at the reusable top-level harness

## Next reasonable implementation
Keep widening the frozen M9 beta slice from the submenu side. The immediate focus is now richer submenu/game-state consequences above `memory_graphics_dat_submenu_consequence_pc34_compat.{c,h}`, especially explicit return/exit semantics or similarly honest state changes that can be guarded by `run_redmcsb_m9_verify.sh`. Keep the new split clear: last-event behavior class stays event-scoped, while cumulative behavior class and mask carry sequence semantics. Investigate the ordinary-runtime crash band at `671..675`, `677..685`, and `687` separately only when it blocks this beta-facing work, while continuing to treat `12`, `21`, `24`, `80..85`, the long wall `126..245`, and later placeholder singletons `676` and `686` as real non-published boundaries.

## Files to start with
- `MILESTONES.md`
- `NEXT_STEPS.md`
- `redmcsb_pc34_port_roadmap_2026-04-16.md`
- `redmcsb_boot_checklist_2026-04-16.md`
- `redmcsb_original_data_inventory_2026-04-16.md`
- `redmcsb_image_split_plan_2026-04-16.md`
- `redmcsb_base_split_plan_2026-04-16.md`
