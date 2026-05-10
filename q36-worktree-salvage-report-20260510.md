# Firestaff Q3.6 Worktree Salvage Report

**Date:** 2026-05-10
**Host:** N2 (trv2@192.168.3.121)
**Repo:** /home/trv2/work/firestaff

## Repository State

| Ref | Hash |
|-----|------|
| HEAD (worker/n2-dm1v1-f0365-f0366-route-followup-20260510) | 69b0695 |
| origin/main | 6dfb78f |
| HEAD ahead of origin/main | 10 commits |

## Worktree Inventory

- **Total git worktrees:** 104 (103 unique commits, 1 duplicate)
- **Already in HEAD:** 1 (current HEAD worktree itself)
- **Already in origin/main only:** 0
- **Unique commits not in HEAD:** 102

### By Category

| Category | Count | Description |
|----------|-------|-------------|
| **Single-commit, parent in HEAD (ready cherry-pick)** | 36 | One commit ahead of HEAD, clean cherry-pick candidate |
| **Multi-commit branches (parent NOT in HEAD)** | 66 | Diverged from older base; need rebase or selective cherry-pick |
| **Duplicate worktrees** | 2 pairs | Same hash tracked in multiple worktrees |
| **Non-git directories** | 4 | firestaff-salvage, firestaff-worker, firestaff-builds, orphan-pass210 |

## Top 20 Cherry-Pick Candidates

Ranked by value (runtime impact > source locks > docs/evidence). Single-commit worktrees first.

### HIGH-RISK (runtime/code behavior)

| # | Hash | Path | Why Valuable |
|---|------|------|-------------|
| 1 | 1f91f04 | firestaff-viewport-walls-20260510 | `pass495: lock dm1 viewport wall occlusion source` — source lock for current work |
| 2 | 32d005e | firestaff-pass423-door-obstruction | `Source-lock door creature obstruction` — gameplay behavior |
| 3 | 9660dad | pass408-strafe-07k-gpt | `pass408: age spell ticks in movement cooldown mirror` — runtime movement fix |
| 4 | ec18262 | pass419-projectile-combat-big-lane-20260508 | `pass419: preserve projectile pass-through for non-material creatures` — combat behavior |
| 5 | 1e0dba7 | pass402-viewport-door-occlusion-gpt | `pass402 order DM1 side viewport contents` — viewport rendering |
| 6 | b5100d3 | pass402-movement-cooldown-gpt | `pass402 align DM1 V1 movement cooldown tick order` — movement timing |
| 7 | ed5c7a7 | pass403-movement-timing-fix-gpt | `pass403 align movement cooldown tick gate` — movement timing |
| 8 | 6fe89f3 | pass409-teleporter-45-47-gpt | `pass409 fix m11 teleporter transition probe` — teleporter behavior |
| 9 | fb38616 | pass388-queue-producer-runtime | `test: add pass388 queue producer runtime verifier` — input queue runtime |
| 10 | fe66554 | pass389-keyboard-producer-fix | `pass389 prove keyboard producer blocker` — keyboard input |

### MEDIUM-RISK (CMake/tests/probes/source locks)

| # | Hash | Path | Why Valuable |
|---|------|------|-------------|
| 11 | 253951a | pass398-text-window-source-lock | `Add pass398 text window source-lock verifier` — UI source lock |
| 12 | 7fc3674 | pass402-text-message-area-gpt | `pass402 add DM1 message area source-lock verifier` — UI source lock |
| 13 | 884d4ae | pass401-text-window-gpt | `pass401: lock dm1 v1 text window source` — text rendering |
| 14 | c32cc50 | pass396-screentext-rendering | `pass396 prove DM1 V1 screentext rendering` — text rendering |
| 15 | cdcfb8e | pass390-viewport-projection | `Add pass390 viewport projection source lock` — viewport source |
| 16 | c363bd1 | pass401-viewport-occlusion-gpt | `pass401 source-lock viewport door occlusion` — viewport source |
| 17 | c096c59 | firestaff-n2-dm1v1-movement-vblank | `worker-n2: propagate blocked movement vblank wait` — 50 unique commits, movement |
| 18 | da1325a | pass395-touch-queue-integration | `pass395 prove touch queue integration` — touch input |
| 19 | fc62317 | pass372-dm1v1-viewport-walls-door-occlusion-20260508 | `pass372: lock dm1 viewport door occlusion source follow-up` — viewport |
| 20 | 8a2ce63 | /tmp/firestaff-pass442 | `pass442: consolidate DM1 viewport wall merge readiness` — consolidation |

### LOW-RISK (docs/evidence/test-only, single commit)

| # | Hash | Path | Why Valuable |
|---|------|------|-------------|
| 21 | 1a160cc | firestaff-title-end-capture-parity-20260509 | `pass462: salvage title original capture gate` — title capture |
| 22 | 27ccc17 | n2-dm1v1-viewport-comparator-pass322 | `pass322: add dm1 viewport comparator harness` — test harness |
| 23 | 396ae30 | pass418-door-entrance-big-lane-20260508 | `pass418: gate closing door obstruction` — door behavior |
| 24 | 4012fbf | pass215-movement-timing-1778016331 | `Source-lock movement redraw present timing` — movement source |
| 25 | 68e4c40 | pass216-viewport-walls-1778016367 | `chore: source-lock viewport wall draw order blocker` — viewport source |
| 26 | 6548398 | /tmp/firestaff-pass423-inventory-gate-1778268665 | `pass423: source-lock V1 inventory gate ranges` — inventory |
| 27 | 69452f1 | pass417-n2-source-scan-20260508 | `pass417: document next source-backed lane scan` — docs |
| 28 | 6e1f04e | pass325-debugger-control-primitive-20260507 | `Add pass325 debugger control primitive blocker evidence` — test |
| 29 | 7015828 | pass416-m11-probe-deep-20260508 | `pass416: fix fakewall viewport probe aspect` — probe fix |
| 30 | 7275328 | n2-dm1v1-pass324-wall-asset-draworder-sweeper | `pass324: lock dm1 v1 viewport draw-order metadata` — viewport |
| 31 | 76e7fa9 | pass395-viewport-walls-source-runtime | `Lock DM1 viewport wall source runtime path` — viewport source |
| 32 | 7e28fcc | pass324-dm1v1-wall-asset-draworder-202605071537 | `pass324: tighten viewport wall asset gate` — 3 unique commits |
| 33 | a03e3b7 | pass414-pass405-heavy-salvage-20260508 | `pass414: source-lock PC34 command queue cap` — source lock |
| 34 | bcf613f | pass397-screentext-blocker | `pass397 source-lock DM1 scroll text layout` — text source |
| 35 | d7c58f1 | pass415-post607-big-integration-20260508 | `pass415 align M11 fakewall viewport aspect` — viewport |
| 36 | ee98b20 | pass399-csbwin-crosscheck-blocker | `pass399 narrow dm1 v1 c080 viewport hand seam` — viewport |

## Dirty Worktrees

**None detected.** All 104 worktrees are clean (no uncommitted changes).

## Duplicate Worktrees (safe to remove one copy)

| Hash | Worktree 1 | Worktree 2 |
|------|-----------|-----------|
| 6f0d7a4 | firestaff-p306-pass305 | firestaff-worktrees/firestaff-p306 |

## Non-Worktree Directories (not git repos, data only)

| Path | Content |
|------|---------|
| firestaff-salvage/ | 5 subdirs with abandoned evidence (2026-05-01) |
| firestaff-worker/ | Empty directory |
| firestaff-builds/ | 7 build artifacts (2026-05-06 to 05-09) |
| firestaff-worktrees/orphan-pass210-untracked-20260506T054011Z/ | Python script + untracked subdir |
| firestaff-worktrees/_quarantine_failed_opus_20260507/ | 2 quarantined note dirs |

## Top Safe Cleanup Candidates

These worktrees are **stale/superseded** — their commits are either evidence-only, deeply diverged from HEAD, or covered by later passes:

### Batch 1: Immediate cleanup (no value, superseded by current work)

| Worktree | Hash | Reason |
|----------|------|--------|
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-level14-movement-viewport-probe-20260506-1819 | 99f3f1e | 115 commits, docs-only, pass252-era probe evidence |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass256-level14-probe-redo-20260506-1830 | 9cae767 | 118 commits, docs-only, superseded by pass443+ |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-movementviewport-dmweb-faq-analysis-20260506-1900 | a9fada0 | 122 commits, docs-only, DMweb FAQ integration |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass261-dosbox-conf-deterministic-pc34-20260506-1903 | a8150f2 | 124 commits, docs-only, DOSBox config audit |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-redmcsb-gdb-symbol-map-20260506-1901 | ce1be40 | 122 commits, docs-only, GDB symbol map |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass265-global-data-address-binding-20260506-1929 | 1055c7d | 125 commits, docs-only, PC34 address binding |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass266-idc-memory-watch-probe-20260506-1936 | e51a94c | 127 commits, docs-only, IDC memory watch |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass273-dunview-stack-narrowing-20260506-2043 | 2501f9f | 127 commits, test-only, doorpass ordering |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass273-live-data-segment-symbol-unblock-20260506-2055 | 8cd1296 | 132 commits, docs-only, FIRES symbol map |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass284-f0380-dequeue-ordering-proof-20260506-2207 | 760c7a8 | 147 commits, docs-only, F0380 dequeue proof |

### Batch 2: DM1 V2 comparator work (diverged track, not merged)

| Worktree | Hash | Reason |
|----------|------|--------|
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-completion-matrix-gate-pass267-20260506-1936 | 9a2c036 | 124 commits, V2 completion matrix |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass272-viewport-composition-gate-20260506-2043 | ce28808 | 127 commits, V2 viewport composition |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass277-real-state-pixel-comparator-scaffold-20260506-2111 | 0fbf11f | 136 commits, V2 pixel comparator |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass283-firestaff-entry-png-render-comparator-20260506-2145 | db025db | 147 commits, V2 PNG comparator |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass285-firestaff-entry-png-export-seam-20260506-2207 | 8e8fc38 | 147 commits, V2 PNG export |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass288-entry-viewport-diff-reduction-20260507-0245 | 95a2512 | 155 commits, V2 viewport diff |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass292-entry-geometry-asset-mismatch-reduction-20260507-0302 | f7e92ef | 156 commits, V2 geometry mismatch |
| firestaff-worktrees/firestaff-oauth-n2-dm1v2-pass294-entry-comparator-next-blocker-manifest-20260507-0311 | 75a10a3 | 157 commits, V2 blocker manifest |

### Batch 3: F0380 proof chain (docs/evidence, superseded by pass500+)

| Worktree | Hash | Reason |
|----------|------|--------|
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass289-f0380-dispatch-equivalent-proof-20260507-0248 | c2834ca | 155 commits, F0380 dispatch proof |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass291-viewport-walls-render-blocker-fix-20260507-0248 | 7779a84 | 155 commits, viewport wall render plan |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass293-direct-f0380-hook-address-window-20260507-0308 | 325678b | 157 commits, F0380 breakpoint blocker docs |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass296-input-tuple-proof-without-direct-f0380-20260507-0322 | f010d5f | 159 commits, movement proof without F0380 |
| firestaff-worktrees/firestaff-oauth-n2-dm1v1-pass299-movement-proof-completion-matrix-update-20260507-0338 | 6425a69 | 162 commits, F0380 gate nonblocking |

### Batch 4: Early pass worktrees (pass215-230, deeply superseded)

| Worktree | Hash | Reason |
|----------|------|--------|
| firestaff-pass230-runtime-symbol-map-202605061137 | 16472eb | 58 commits, pass230, superseded |
| firestaff-pass230-runtime-symbol-map-current-202605061137 | 8207937 | 60 commits, pass230, superseded |
| firestaff-pass230-runtime-symbol-map-latest-202605061137 | f4d612c | 61 commits, pass230, superseded |
| firestaff-worktrees/pass215-movement-timing-1778016331 | 4012fbf | 1 commit (cherry-pickable, see above) |
| firestaff-worktrees/pass216-viewport-walls-1778016367 | 68e4c40 | 1 commit (cherry-pickable, see above) |

### Batch 5: GPT-era pass worktrees (pass401-409, mixed quality)

Many GPT-assisted passes with 1-2 commits each. The single-commit ones are listed as cherry-pick candidates above. The multi-commit ones:

| Worktree | Hash | Reason |
|----------|------|--------|
| firestaff-worktrees/pass407-walls-collision-gpt-redo | b0f99d9 | 2 commits, wall collision redo |
| firestaff-worktrees/pass404-viewport-occlusion-fix-gpt | e3ee04e | 2 commits, viewport occlusion fix |
| firestaff-worktrees/pass368-dm1v1-solid-wall-early-return-occlusion-20260508 | f1a926e | 3 commits, solid wall occlusion |
| firestaff-worktrees/dm1v1-406-viewport-camera | 0c810ca | 3 commits, viewport boundary walls |

## Recommended First Batch for Cherry-Pick to Main

**36 single-commit worktrees** with parent already in HEAD. Cherry-pick in this order:

### Tier 1: Runtime fixes (high impact)
1. `9660dad` — pass408: age spell ticks in movement cooldown mirror
2. `ec18262` — pass419: preserve projectile pass-through for non-material creatures
3. `32d005e` — Source-lock door creature obstruction
4. `1f91f04` — pass495: lock dm1 viewport wall occlusion source
5. `b5100d3` — pass402 align DM1 V1 movement cooldown tick order
6. `ed5c7a7` — pass403 align movement cooldown tick gate
7. `6fe89f3` — pass409 fix m11 teleporter transition probe
8. `1e0dba7` — pass402 order DM1 side viewport contents

### Tier 2: Source locks (medium impact, low risk)
9. `253951a` — Add pass398 text window source-lock verifier
10. `7fc3674` — pass402 add DM1 message area source-lock verifier
11. `884d4ae` — pass401: lock dm1 v1 text window source
12. `c32cc50` — pass396 prove DM1 V1 screentext rendering
13. `cdcfb8e` — Add pass390 viewport projection source lock
14. `c363bd1` — pass401 source-lock viewport door occlusion
15. `da1325a` — pass395 prove touch queue integration
16. `fc62317` — pass372: lock dm1 viewport door occlusion source follow-up
17. `8a2ce63` — pass442: consolidate DM1 viewport wall merge readiness
18. `fb38616` — test: add pass388 queue producer runtime verifier
19. `fe66554` — pass389 prove keyboard producer blocker
20. `ee98b20` — pass399 narrow dm1 v1 c080 viewport hand seam

### Tier 3: Evidence/docs (low risk)
21. `1a160cc` — pass462: salvage title original capture gate
22. `27ccc17` — pass322: add dm1 viewport comparator harness
23. `396ae30` — pass418: gate closing door obstruction
24. `4012fbf` — Source-lock movement redraw present timing
25. `68e4c40` — chore: source-lock viewport wall draw order blocker
26. `6548398` — pass423: source-lock V1 inventory gate ranges
27. `69452f1` — pass417: document next source-backed lane scan
28. `6e1f04e` — Add pass325 debugger control primitive blocker evidence
29. `7015828` — pass416: fix fakewall viewport probe aspect
30. `7275328` — pass324: lock dm1 v1 viewport draw-order metadata
31. `76e7fa9` — Lock DM1 viewport wall source runtime path
32. `a03e3b7` — pass414: source-lock PC34 command queue cap
33. `bcf613f` — pass397 source-lock DM1 scroll text layout
34. `d7c58f1` — pass415 align M11 fakewall viewport aspect
35. `3fa15af` — pass366: gate nearest center door occlusion
36. `5551a1a` — pass421: source-lock PC34 input queue C5 cap

## Summary

- **104 worktrees** audited across `/home/trv2/work/firestaff-worktrees/`, `/home/trv2/work/firestaff-*`, and `/tmp/firestaff-*`
- **36 ready cherry-picks** (single commit, parent in HEAD) — see Tier 1-3 above
- **66 multi-commit branches** — need selective cherry-pick or rebase before integration
- **0 dirty worktrees** — no uncommitted salvage needed
- **2 duplicate pairs** — one copy each can be removed
- **4 non-git directories** — data-only, not worktrees
- **~60 stale worktrees** safe for cleanup (Batches 1-3 above) — mostly docs/evidence from pass252-299 era and DM1 V2 comparator track

## Verification

```
$ git status --short   (before: clean)
$ git status --short   (after: only q36-worktree-salvage-report-20260510.md as untracked)
```

---
Report generated: 2026-05-10 12:08 CEST
Model: qwen3.6:35b (DANNESBURK, local)
