# CSB V1 Bootstrap Scout

Scope: evidence-only planning for a future **CSB V1 parity** path. This pass does not change rendering, M10 behavior, or launch support.

## Current anchors

- `config_m12.h` already reserves three game slots: DM1, CSB, and DM2.
- `menu_startup_m12.c` maps `csb` to game slot 1, shows CSB in the startup catalog, and preserves per-game options.
- `asset_status_m12.c` has known CSB graphics MD5s and accepts `GRAPHICS.DAT` or `CSBGRAPH.DAT` names for the CSB slot.
- `probes/m12/firestaff_m12_startup_menu_probe.c` explicitly asserts that CSB is not launchable yet, even when asset scanning reports a match.

## Parity gap against DM1 V1

DM1 V1 has a source/evidence spine that CSB does not yet have:

1. DM1 source lock: `tools/greatstone_dm1_source_lock_check.py` verifies the DM1 `GRAPHICS.DAT`/`DUNGEON.DAT` source pair and writes evidence.
2. DM1 original capture path: `scripts/dosbox_dm1_capture.sh` and follow-up DM1 capture/compare tools exist.
3. DM1 layout assumptions: `tools/extract_zones_layout_696.py`, `m11_game_view.c`, and M11 probes are tied to DM1 PC 3.4 / layout-696 / ReDMCSB-backed constants.
4. DM1 graphics runtime: existing V1 loaders, palette code, and M11 presentation code are DM1-first even when some low-level comments mention DM/CSB shared PC-era VGA data.

CSB currently has only catalog/asset-awareness. It does not have a CSB source lock, original capture harness, dungeon-data identity check, layout variant proof, or runtime boot/rendering path.

## Shared graphics/assets assumptions to verify before code changes

Do not assume CSB can reuse DM1 V1 geometry just because the launcher recognizes `CSBGRAPH.DAT`.

- Verify the exact CSB media variant first: graphics file, dungeon file, language/version, and any paired data required by ReDMCSB or the original binary.
- Confirm whether the CSB version being targeted uses the same graphics container, palette interpretation, and zone/layout records as the DM1 V1 path.
- Treat `asset_status_m12.c` CSB hashes as catalog readiness only. It currently requires one matched file, so it is not enough evidence to launch or render CSB.
- Keep `assets-v2/` and `tools/v2_asset_store.py` out of the CSB V1 truth chain.

## First safe probe/evidence step

Add a CSB source-lock probe before enabling any launch/rendering path:

1. Define the target CSB media set in tracked source: expected graphics file names, dungeon/save-data file names, hashes, labels, and provenance.
2. Verify **both** graphics and dungeon/runtime data files by hash. A graphics-only match should remain insufficient.
3. Emit a short evidence report under `parity-evidence/` with relative paths, hashes, and the selected CSB variant.
4. Only after that, add a non-rendering boot-readiness probe that proves the launcher can produce a CSB launch intent behind an explicit experimental gate.

This keeps the next step bounded and avoids accidental M10 semantic changes or DM1 V1 rendering regressions.

## Probe added in this scout

`tools/csb_v1_bootstrap_scout.py` is a static evidence probe. It checks that CSB is cataloged but launch-gated, that CSB asset status is graphics-only, and that DM1 has source-lock/capture tooling that CSB lacks.
