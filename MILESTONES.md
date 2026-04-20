# ReDMCSB original-compatible milestones

This is the working milestone ladder from the current verified state to a meaningfully complete original-compatible port.

Rule: a milestone is only checked off when it is verified against real original data or real original control flow, not by synthetic wrappers alone.

---

## M0. Foundation and probeable seams

Status: ✅ DONE

Done means:
- real `GRAPHICS.DAT` access works through the compat path
- load/decompress/expand/present path exists end-to-end
- visible export works on broad real-data ranges
- startup/runtime/boot-sequence/boot-program/boot-plan seams exist and run on real original data
- metadata-aware classification exists for:
  - bitmap-safe
  - bitmap-suspicious
  - special non-bitmap
  - empty
  - zero-sized placeholder

---

## M1. Honest runtime accounting

Status: ✅ DONE

Done means:
- `completed`, `skipped`, and `published` are separated honestly
- placeholder vs special skip reasons survive through:
  - single-frame runtime
  - multi-frame runtime
  - stateful runtime
  - boot-sequence
  - boot-program
  - boot-plan
  - scripted boot-plan suite
- ordinary probes report the richer story directly, not only nested flat totals

---

## M2. Early-file structure map

Status: ✅ DONE

Done means:
- early and mid early ranges are classified with real-data verification
- known placeholder boundaries are identified and treated honestly
- script catalog has reusable verified windows for early startup-adjacent ranges

Currently established:
- clean windows through `25..79`
- placeholder cluster `80..85`
- clean windows through `86..125`
- long placeholder wall `126..245`
- resumed clean run `246..670`
- crash band in ordinary runtime probe: `671..675`, `677..685`, `687`
- later placeholder singletons: `676`, `686`
- known placeholder entries/boundaries so far:
  - `12`
  - `21`
  - `24`
  - `80..85`
  - `126..175`

---

## M3. Full `GRAPHICS.DAT` structure map

Status: ✅ DONE

Done means:
- later-file ranges are mapped far enough that the file has a believable structural segmentation
- long clean windows, placeholder walls, and special regions are documented
- script catalog covers representative windows across the file, not just startup-adjacent ones
- real-data suite expectations stay green while coverage grows

Verified outcome:
- the file now has a believable segmented shape with clean runs, placeholder walls, placeholder singletons, a later crash band, and a known mixed/special late region
- this segmentation was sufficient to choose title/menu-relevant candidates from evidence instead of guessing

Exit criteria:
- enough of the file is structurally segmented that title/menu-relevant candidates can be chosen from evidence instead of guessing

---

## M4. Title/menu candidate set

Status: ✅ DONE

Done means:
- a shortlist of likely title/menu-related windows exists
- those windows are promoted into named scripted boot-plan sequences
- the sequences are grounded in actual verified indices, not speculation
- placeholder-heavy or special-heavy regions are explicitly excluded unless intentionally exercised

Verified outcome:
- the candidate search converged on a stable backdrop-side lead around `368..391` and a stable title/menu-side lead around `272..287` then `304..319`
- those windows were promoted into repeatable named scripts with stable expectations on real original data

Current best composite follow-up:
- `overlay_title_b_then_menu_a` = `272..287` then `304..319`

Current M6/M7 lead scripts:
- `m6_boot_attempt_a` = `368..391` then `272..287` then `304..319`
- `m7_reachability_a` = `368..391` then `272..287` then `304..319` then `304..319`

Exit criteria:
- we can run a small suite of title/menu-like scripted sequences repeatedly on original data with stable expectations

---

## M5. Missing decode/asset-coverage gaps closed

Status: ⏳ TODO

Done means:
- any real asset classes needed for title/menu reachability that still fail decode are identified
- missing IMG3/source-buffer cases are implemented only where real data proves they are needed
- suspicious-but-exportable bitmap cases are reviewed and either promoted, isolated, or documented

Exit criteria:
- title/menu candidate sequences are not blocked by avoidable decode gaps in the currently selected windows

---

## M6. First meaningful title/menu boot attempt

Status: ✅ DONE

Done means:
- the runtime can move beyond startup-adjacent proof-of-life into a title/menu-like scripted path
- multiple consecutive asset loads and publications work under real original data
- the path stays alive without fake progress assumptions
- visible output is stable enough to inspect as a real higher-level boot result

Current verified result:
- `m6_boot_attempt_a` = `368..391` then `272..287` then `304..319`
- verified on real `GRAPHICS.DAT` with `completed=56`, `skipped=0`, `published=56`

Exit criteria:
- one repeatable title/menu-like boot script exists and completes cleanly with expected outputs

---

## M7. Title/menu reachability

Status: ✅ DONE

Done means:
- the program reaches an actual recognizable title/menu state
- title/menu graphics, panels, and text are placed plausibly
- the runtime survives there across multiple frames
- asset loading behavior around that state is stable

Current verified reachability-oriented probes:
- `m7_reachability_a` = `368..391` then `272..287` then `304..319` then `304..319`
- `m7_reachability_b` = `368..391` then `272..287` then `304..319` then `312..319`
- `m7_reachability_c` = `368..391` then `272..287` then `304..319` then `316..319`
- all three verify cleanly on real `GRAPHICS.DAT`
- `m7_reachability_a`: `completed=72`, `skipped=0`, `published=72`
- `m7_reachability_b`: `completed=64`, `skipped=0`, `published=64`
- `m7_reachability_c`: `completed=60`, `skipped=0`, `published=60`
- visual comparison against `m6_boot_attempt_a` showed that the full repeated menu tail in `m7_reachability_a` mostly repeated ambiguous/noisy menu evidence rather than settling into a more convincing stable state
- direct comparison between the selective tail variants shows that `m7_reachability_b` remains the better current M7 lead: repeating `312..319` gives a clearer held-state dwell, while the tighter `316..319` tail in `m7_reachability_c` trims too much and weakens the latched/stable feel slightly
- new stateful boot-plan reachability seam added above the scripted boot-plan runner and verified on `m7_reachability_b`
- the new probe reports explicit higher-level phases and reaches:
  - `phase=4` (`MENU_HELD`)
  - `advancedPhaseCount=4`
  - `reachedMenuEstablished=1`
  - `reachedMenuHeld=1`
- this is the first M7-era result that elevates the current best scripted sequence into an explicit stateful reachability judgment instead of only a long flat publication chain
- the newer backdrop-aware composition path now also shows that the preloaded full-screen `dialogGraphicIndex=1` asset already contains the large recognizable title card, while the viewport/title-side path contributes only a small local patch on top of that baked-in title screen
- on the composed-backdrop export path, the held `m7_reachability_b` tail remains visibly recognizable and stable across repeated frames: the large title/logo card stays plausibly placed and the only changes are minor local patch fluctuations rather than layout collapse or re-randomization
- the late-tail patch behavior is now step-mapped explicitly: the residual diff against the baked-in backdrop stays `304`-like across the tail, while the visible local patch follows a repeatable `304/319` alternation and ends on `319`; the repeated `312..319` tail in `m7_reachability_b` reproduces the same stable pattern rather than collapsing
- first M8-oriented loop/hold probe now also exists:
  - `m8_loop_stability_a` = `368..391` then `272..287` then `304..319` then `312..319` then `312..319`
  - verifies cleanly on real `GRAPHICS.DAT` with `completed=72`, `skipped=0`, `published=72`
  - through the stateful reachability seam it reports `holdCompletedStepCount=16`, `holdCycleSize=8`, `holdCycleCount=2`, `reachedMenuHeld=1`
- that is not honest M8 completion yet, but it is the first explicit evidence that the current held menu-side state can survive more than one selective hold cycle without collapsing

Verified outcome:
- a real recognizable title/title-card-like state is now reached on original data with plausible placement, stable repeated hold behavior, and explicit stateful reachability evidence (`reachedMenuEstablished=1`, `reachedMenuHeld=1`) on the current best lead `m7_reachability_b`

Exit criteria:
- a real title/menu state is reached reliably enough to treat it as a platform milestone, not a probe stunt

---

## M8. Interaction and loop stability

Status: ✅ DONE

Done means:
- basic input/event handling is wired enough to interact with title/menu state
- frame/tick progression is stable enough that the state does not freeze or collapse immediately
- repeated loads, cache behavior, and loop transitions do not obviously corrupt the session

Currently established:
- `m8_loop_stability_a` is the first clean real-data hold/loop probe above the M7 baseline and verifies with `completed=72`, `skipped=0`, `published=72`
- through the stateful reachability seam it reports `holdCompletedStepCount=16`, `holdCycleSize=8`, `holdCycleCount=2`, and `reachedMenuHeld=1`, which is explicit evidence of surviving more than one selective held menu-side cycle
- the runtime-near local probe stack centered on `redmcsb_runtime_tail_local_probe.c` reproduces the same mixed late-tail behavior across both `m6_boot_attempt_a` and `m7_reachability_b`, so the `304/319` patch pattern is stable across the original late tail and the held-repeat tail
- raw viewport metadata, nonzero-bbox checks, synthetic menu-mini seams, startup/runtime flags, and ST-side binding theories were all falsified as leading explanations for the front-lock split
- the former broad `304..319` mystery is now locked to ordinary DM PC v3.4 `IMG3` front-lock assets, not title/intro tail content
- inside that front-lock family, `308` (`Topaz Front`) and `312` (`Gold Front`) were shown to be the row-repetitive / copy-heavy exception lane, while `306` remains the separate explicit-color-10 / `kind7` outlier
- motif-level repetition, decode-side `kind6` dominance, and visible-side positive patch outcome now form one coherent chain: repeated adjacent motif rows correlate positively with `img3Kind6Permille` (`+0.57`) and very strongly with `patchSignedDelta` (`+0.86`), while correlating negatively with `img3Kind4Permille` (`-0.654`)
- a closing `kind6 -> kind4` ablation on real raw IMG3 data directly demonstrated the causal link for `308`: baseline `patchSignedDelta` `+831` collapsed to `-259` after rewriting the `kind6` commands. The same ablation on `312` reduced `patchSignedDelta` from `+858` to `+555`, strongly supporting the same mechanism even though `312` retained some favorable structure

Verified outcome:
- the reachable title/menu-side state now survives repeated held cycles on original data, and the remaining late-tail behavior has a concrete content/decode explanation rather than a vague runtime mystery. M8 is therefore complete at the milestone level; any further `312`-specific ablation work is polish, not a blocker

Exit criteria:
- basic interaction with the reachable title/menu state works and remains stable for repeated runs

---

## M9. Original-compatible beta

Status: ✅ FROZEN / VERIFIED

Done means:
- original files are required and used cleanly
- boot to title/menu is repeatable
- basic interaction works
- known unsupported paths are documented honestly
- the port is usable as a real beta of the original-compatible branch

Currently established:
- a single runnable beta harness now exists: `redmcsb_m9_beta_harness.c` via `run_redmcsb_m9_beta_harness.sh`
- it boots through `m7_reachability_b` to a held menu state on original data and publishes visible PGM frames for both boot and interaction steps
- it supports both one-shot event sequences and line-buffered session mode (`--session`)
- the menu-side interaction path now carries real persistent session state across steps for:
  - current selection
  - current screen (`menu` / `submenu`)
  - current render graphic
  - last activated selection/graphic
- the former unified boot blocker is now resolved in the harness: unified boot publication plus post-boot interaction works on original data
- verified unified one-shot path `fnnaf` reaches the known result: `finalSelectionIndex=2`, `activatedGraphicIndex=13`, `finalScreen=1`
- longer unified interaction sequences also complete without crashing, so the old blocker has moved from runtime survival to fidelity of submenu/title semantics
- a direct `--title-hold` mode now exists and can freeze the current best practical title hold at graphic `313`
- repeated `--title-hold` frames are pixel-identical, so the current title/title-card state is a real stable hold, not just a lucky single-frame capture
- this is now strong enough to call M9 a done-enough first beta slice for the original-compatible branch, not just a milestone in progress
- submenu drift has been removed in favor of conservative honest submenu state until a richer submenu seam exists
- final reporting now distinguishes current-event vs cumulative sequence semantics (`activationTriggered` vs `anyActivationTriggered`, `lastScreenChanged` vs `screenChanged`)
- a single top-level verification gate now exists as `run_redmcsb_m9_verify.sh`
- that verification gate now checks title-hold semantics, repeated title-hold identity, submenu artifact existence, submenu invariant PASS state, and expected submenu-matrix row coverage
- the submenu verification surface now also carries explicit last-event behavior classes plus cumulative behavior masks, so the frozen beta slice is guarded by both structural invariants and compact semantic submenu signals
- the submenu verification surface now also carries an explicit cumulative behavior class for sequence semantics, so mixed rows can be verified as honest mixed submenu flows without overloading the last-event class
- the main remaining gaps are now follow-on beta improvements: richer submenu/game-state consequences, broader interaction coverage, and higher title/menu/submenu fidelity

Exit criteria:
- someone other than us could run it, understand what works, and not be misled about what does not

---

## M10. 100% original-compatible

Status: ⏳ TODO

This is the finish line, not the next target.

Done means:
- no known original-content path is missing for the intended original-compatible scope
- graphics/data/audio/control-flow all behave compatibly enough to call the branch complete
- remaining differences are polish-level, not structural missing pieces
- the project is ready to stop being treated as a porting experiment and start being treated as the finished original-compatible version

---

## Immediate active checkpoint

Active milestone: **Post-M9 beta widening**

Immediate sub-goal:
- keep `m7_reachability_b` and `m8_loop_stability_a` as the stable original-data title/menu baselines
- treat M9 as frozen and verified via `run_redmcsb_m9_verify.sh`
- build outward from the new `memory_graphics_dat_submenu_consequence_pc34_compat.{c,h}` seam instead of adding more isolated probes
- focus on richer submenu/game-state consequences, broader interaction coverage, and higher title/menu/submenu fidelity
- keep the crash band around `671..687` separate unless it becomes directly relevant

## What to beta av next

1. Use `m7_reachability_b` as the current stable held-state baseline and `m8_loop_stability_a` as the first loop-stability-oriented probe
2. The first command/input-facing seam now exists above the startup/main-loop-entry chain as `memory_graphics_dat_main_loop_command_pc34_compat.{c,h}`
3. Additional tiny control-flow seams now also exist above it as `memory_graphics_dat_main_loop_command_loop_pc34_compat.{c,h}`, `memory_graphics_dat_main_loop_command_queue_pc34_compat.{c,h}`, `memory_graphics_dat_main_loop_typed_command_queue_pc34_compat.{c,h}`, `memory_graphics_dat_input_command_queue_pc34_compat.{c,h}`, `memory_graphics_dat_event_dispatch_pc34_compat.{c,h}`, `memory_graphics_dat_menu_state_pc34_compat.{c,h}`, `memory_graphics_dat_menu_render_effect_pc34_compat.{c,h}`, `memory_graphics_dat_menu_activate_pc34_compat.{c,h}`, and `memory_graphics_dat_menu_activate_consequence_pc34_compat.{c,h}`
4. Those seams are now packaged into a single runnable beta harness that proves boot, user-provided input, visible frame publication, and persistent menu/session state across steps on original data
5. The blocker is now narrower: what remains missing is not basic menu interaction but richer menu semantics, deeper persistent runtime state above the startup-frame driver, and real game-state consequences above the tiny post-activate seam
6. Keep `671..675`, `677..685`, and `687` as a separate crash-band task when needed
7. Only return to M5 decode-gap work if M7/M8/M9 evidence points to a real missing asset-class blocker
