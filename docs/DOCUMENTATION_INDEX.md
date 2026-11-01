# Firestaff documentation

This is the documentation map for the Firestaff project. It covers
all five supported games, the shared launcher and engine layers, data setup,
CI, source-lock evidence and release packaging.

## Current project boundary

The public status is intentionally conservative. A parser, fixture or
source-lock test proves a narrow contract; it does not by itself prove
original-media playability or pixel parity.

| Game | Verified today | Active boundary |
|---|---|---|
| Dungeon Master (DM1) | Playable PC 3.4 V1 route; separately verified Atari ST, Amiga and FM Towns starts | Continued original-route captures and V2 visual polish |
| Chaos Strikes Back (CSB) | Bounded native real-media routes on Atari ST, Amiga and FM Towns; Amiga F0107 uses source MEDIA720 scales and authentic C696 anchors, and A31M/A33M SWSH resources have a verified playback path | Matched Amiga frame, palette/light, inscription, floor trapdoor, native intro timing/audio, wider campaign, save and viewport evidence |
| Dungeon Master II: Skullkeep | Bounded real-media routes on DOS, Amiga, FM Towns and Macintosh | Full record/save ownership and wider source UI, audio and mechanics parity |
| DM Nexus | Bounded presentation of retail MAPD title assets from Saturn CUE/BIN, plus source-owned disc/format work. Japanese retail and English-version Ymir cold boots reach the native title menu; the English screenshot replay is pixel-exact with 126 informative retail-byte matches among 137 textured draws. Capture-only Japanese title-prompt work binds selected `TITLE.BIN` pixels and command 171; menu frame 119 binds five visible commands to `TITLE.BIN`; the VDP1/VDP2 stacks for gameplay frames 239 and 397 match all 286,720 doubled screenshot samples exactly. These are reference-emulator results, not Firestaff gameplay support. | Full Firestaff Saturn title composition and title-to-menu parity; menu, HUD and dungeon display-consumer evidence; bounded capture decoders do not change production readiness |
| Theron's Quest | Native direct launch skips Firestaff's Run menu and System Card. Authentic JP Track 02 reaches a loaded dungeon through fresh start; authentic US Track 02 plus matching BRAM reaches dungeon 2 through Continue. Eight selected regional real-media/native startup, later-dungeon, keyboard and SDL input CTests passed three consecutive loops (24 executions); SDL coverage includes mouse-panel/touch input, movement, wait, champion selection, and US item pickup/reselect/drop. Authenticated US/JP ISO projection pointer records 0–4 statically resolve to the five regular-spawn zone records; pointers 5–7 have zero offsets, with no runtime spawn meaning inferred. An authentic US Drator replay reached `start`, `left-event` and `reentered-event`; the rebuilt reference hook captured 132 `$C852` fetches at physical `$0DC852`, with the first five 512-byte runtime windows exactly matching the authentic US Track 02 candidate at raw offset `$A3AD2`. The same session proves writes and reads of generated row offsets 6–8 at physical `$1F093B-$1F093D`, plus a later ten-byte TII copy to a work buffer. A strict verifier matches constructed `$CBC6` and `$45C3` windows from authenticated US Track 02 bytes, finding four and three raw offsets respectively; these are not live fetched windows and do not bind a unique runtime LBA/source occurrence. The filtered trace reached its cap after these reads; the route subsequently reached reentry. The BIOS trace is explicitly truncated; its streaming verifier correctly rejects the marker, and the wrapper rejected the capture package because of the BIOS trace cap. The JP active-map capture now has an explicit source-to-consumer acceptance chain, but no capture has yet joined descriptor/map reads, runtime selection, a tile read at the captured pose, and atomic VDC/VCE/VRAM state. USE effects, positive JP progress save/restore, later campaign play, combat, audio, and visual parity remain open. See [capture readiness](theron/THERON_CAPTURE_READINESS.md), the [Drator capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md), the [active-map capture chain](../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain), and the [active Theron TODO](../TODO-theron.md). |

Across every game, work is ordered as **startup → menu → HUD → viewport**.
Later presentation claims remain blocked when an earlier source or real-data
handoff is not proven.

## Documentation roles

Use the following hierarchy when documentation appears to disagree:

1. **Current public status:** this index, [project status](PROJECT_STATUS.md),
   [platform status](PLATFORM_STATUS.md), and the corresponding checked-in
   wiki pages are the current support contract.
2. **Technical reference:** format, reverse-engineering and source-lock pages
   record the evidence, implementation details and known boundaries behind a
   current claim.
3. **Planning and historical records:** dated audits, reports, release notes
   and files under `docs/plans/` preserve decisions at the time they were
   written. They are not current status unless a current-status page links to
   the specific conclusion.

Current public-status pages must describe only source-owned or real-media-backed
behaviour. They must not contain local machine paths, private captures,
disassemblies, BIOS/firmware, game archives, saves, or other copyrighted source
material. A README screenshot may show Firestaff itself, but is never evidence
for an original game's output. Historical records are being retained as dated
research and must not be copied into current status or wiki pages.

Per-game technical notes are grouped under `docs/dm1/`, `docs/csb/`,
`docs/dm2/`, `docs/nexus/` and `docs/theron/`. Shared engine, menu and
gameplay notes are under `docs/shared/`; cross-game status and project guides
remain at the top of `docs/`.

## Start here

- [README](../README.md) — user-facing overview, supported platforms and quick start
- [Project status](PROJECT_STATUS.md) — short cross-game status matrix
- [Five-game startup support gap analysis](STARTUP_SUPPORT_GAP_ANALYSIS.md) — authenticated CLI coverage and unresolved launcher/runtime gaps
- [Platform status](PLATFORM_STATUS.md) — current status for every game and platform
- [Active top-10 native-parity queue](parity/ACTIVE_TOP_10.md) — current
  evidence-backed priorities and explicit data blockers
- [Game data setup](DATA_SETUP.md) — legal data intake and scanner behavior
- [Runtime-media ownership](DATA_SETUP.md#runtime-media-ownership) — original
  container ownership, bounded-memory reads and the no-extraction rule
- [Game-data formats](GAME_DATA_FORMATS.md) — verified containers, record families and save boundaries
- [CI guide](CI.md) — GitHub Actions checks and local reproduction
- [Verified hashes](VERIFIED_HASHES.md) — canonical data identities
- [Historical technical gap analysis](FIRESTAFF_GAP_LIST.md) — preserved
  research; use project/platform status and per-game TODO files for active work
- [Missing functions by game](MISSING_FUNCTIONS_BY_GAME.md) — code, data and verification gaps
- [Synthetic data by game](SYNTHETIC_DATA_BY_GAME.md) — fixtures, placeholders and real-data replacements
- [Preservation and provenance](wiki/Preservation.md) — evidence levels, capture manifests and external data rules
- [Preservation status (reviewed 2026-08-12)](PRESERVATION_STATUS_2026-08-11.md) — cross-game real-media, format and disassembly boundary
- [External preservation archive](PRESERVATION_ARCHIVE.md) — moved historical runs and checksum manifest
- [Archived project history](archive/HISTORY-archived-2026-08-08.md) — older mixed completion log
- [Legacy DM1 tracking list](archive/DM1-tracking-list-2026-05-26.md) — superseded by the current per-game TODO and DONE logs
- [Latest versioned release summary](releases/RELEASE_3.0.390.md) — older summaries are grouped in the same directory; see the [current changelog](https://github.com/yeager/firestaff/blob/main/RELEASE_NOTES.md)
- [DMWeb and Greatstone references](DMWEB_REFERENCE.md) — external provenance index
- [Theron input controls](theron/theron_input.md) — native keyboard mappings, visible in-dungeon mouse movement panel, and Button I/II behavior
- [Theron real-data inventory](theron/THERON_REALDATA_INVENTORY.md) — Track 02/19 media identities and placeholder boundaries
- [Theron ISO spawn-record projection](source-lock/theron-track02-spawn-record-iso-projection-20261101.md) — hash-verified US/JP ISO offsets and raw-BIN parity; pointer records 0–4 statically resolve to the five zone records, without inferring runtime spawn semantics
- [Theron System Card run-path trace filter](../scripts/instrument_mednafen_theron_syscard_io_ram_trace.py) — optional `run-path` capture retains authenticated `$EA4D`, `$EA50`, `$E162` and callback `$2259` rows; three-loop tooling checks pass, but no game run has used it yet
- [Theron Drator row-consumer capture filter](../scripts/capture_theron_mednafen_live_trace.sh) — opt-in PCE Fast `drator-row` mode retains observed US reads at `$CBC9`/`$0DABC9` and `$45C3`/`$0D05C3` for `$1F093B-$1F093D`; the v2 parser checks address provenance only, three tooling loops pass, and no authentic run has used the filter
- [Theron capture readiness](theron/THERON_CAPTURE_READINESS.md) — canonical current summary of native `--debug` evidence, reference captures, evidence limits, the cold JP route and empty BRAM result, and the JP active-map source-to-consumer acceptance chain
- [Theron JP Akutuba source-sector trace plan](source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md) — identity-verified descriptor and four arrays map to raw Track 02 sector 308 / disc LBA 3674; the authentic JP `.mc0` source span maps to HuC CD-RAM page `$74`, while runtime selection remains unknown
- [Theron Mednafen PCE Fast state inspector](../tools/inspect_theron_mednafen_pce_fast_state.py) — source-locked reader for authentic 1.32.1 `.mc0` sections, CD-RAM/MPR mapping, and embedded VCE/VDC0/VRAM/SAT snapshot inventory
- [Theron JP Akutuba trace-target planner](../tools/plan_theron_jp_akutuba_map_trace.py) — derives raw offsets and disc LBAs from the authentic JP Rev. 1 BIN after checking its full media identity; optional `--dump-grids` prints descriptor fields and all four maps as opaque raw tile bytes
- [Theron reference-capture runbook](theron/THERON_MAC_SDL_MEDNAFEN_LOCAL.md) — current diagnostic capture constraints; retired machine-specific build paths have been removed
- [Theron native Debug direct launch](source-lock/theron-native-debug-direct-launch-20261031.md) — confirms the Firestaff Run menu is bypassed; the startup probe did not load a dungeon
- [Theron PCE Fast `$C1FC` control-flow trace](source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md) — bounds a repeated 128-instruction context; input-conditioned transfer, teleporter association, and unknown map/field semantics do not prove player movement
- [Theron cold System Card RUN trace](source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary) — joins the observed port bytes to a BIOS branch but does not observe a confirmed game handoff
- [Theron authentic empty JP BRAM M11 regression](../tests/test_theron_v1_m11_jp_empty_bram.c) — native UI rejection of the captured empty JP slot; positive JP progress restoration remains unverified
- [Theron JP PCE Fast input receipt](source-lock/theron-disassembly/theron-jp-pce-fast-scripted-input-receipt-20261031.md) — three authentic-state runs bind a scripted direction mask to its same-frame PCE Fast guest read
- [Theron JP PCE Fast input-gated VRAM trace](source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md) — corrected PC/raw gate admits 4,096 bounded VRAM commits after an authentic-state guest read; event ordering does not prove causality or parity
- [Theron `$C852` fetch-trace hook](../scripts/instrument_mednafen_theron_pce_fast_c852_fetch_trace.py) — bounded opt-in reference instrumentation; the isolated 2026-10-31 cold US run reached Drator `start`, `left-event` and `reentered-event`, recording 132 fetches. Its first five 512-byte windows match authentic Track 02 raw offset `$A3AD2`. Same-session writes and reads of offsets 6–8 are proven, and PC `$45C3` later copies the ten-byte row to a work buffer. Generated-row consumer semantics remain open.
- [Theron US `$C852` HuC6280 disassembly](source-lock/theron-disassembly/theron-us-c852-a3ad2-20261031.asm) — 512 source-matched bytes from authenticated US Track 02 at raw offset `$A3AD2`; source mapping is proven, while register semantics and generated-row consumer ownership remain open
- [Theron US Drator cold capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md) — records route reachability, the same-session row-byte join, the `$A3AD2` runtime source match, and the capture-verification boundary
- [Theron US Drator row-reader source candidates](source-lock/theron-disassembly/theron-us-drator-row-reader-source-map-20261101.md) — disassembles the `$CBC9`/`$45C3` reader candidates; the strict verifier matches constructed 512-byte windows against authentic BIN bytes (four and three offsets), while a live runtime LBA/source-occurrence join remains open
- [Theron JP PCE Fast state comparison](source-lock/theron-disassembly/theron-jp-pcefast-input-state-compare-20261031.md) — three UP/control pairs share an event sequence from the `$44C1` guest poll through the `$D334`/`$D349` `$2912` transaction; ordering is repeatable, while causality and gameplay semantics remain unproven
- [Theron JP blocked-wall movement probe](source-lock/theron-disassembly/theron-jp-up-input-latch-blocked-wall-probe-20261031.md) — UP, neutral and DOWN leave the authentic F5 wall image unchanged; bounded position hooks record no movement, so this is not movement or renderer evidence
- [Theron JP state-only input/position comparison](../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md) — three Button II+UP runs repeat candidate-position TII transfers while matched Button II-only controls retain the prior bytes; map identity, field meaning, collision behavior and visual response remain unresolved
- [Theron JP title-action capture](source-lock/theron-jp-four-second-title-action-capture-20261031.md) — authentic cold JP route confirms four-second Button I holds on NEW GAME and FILE 1 advance into Akutuba; `.mc0` is an emulator state, not BRAM, and no game-owned loader join or native hold-timing parity is claimed
- [Theron PCE Fast input/state comparison verifier](../scripts/verify_theron_pcefast_input_state_comparison.py) — checks a hash-bound UP/control trace manifest and ordered RAM evidence
- [Theron US PCE mouse-held capture](source-lock/theron-disassembly/theron-us-pce-mouse-held-command-capture-20261031.md) — repeated guest mouse-read and candidate command-consumer receipts; save-state replay has no runtime CD provenance
- [Theron US saved-state scripted-input boundary](source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md) — non-BIOS read PCs and changed snapshots, but ambiguous standard-PCE/PCE Fast trace records; no action or renderer mapping is established
- [Theron US cold `$E009` capture boundary](source-lock/theron-disassembly/theron-us-cold-select-i-hold-e009-capture-20261031.md) — earlier raw-mask-verifier result, retained as historical evidence
- [Theron US returned-mask capture](source-lock/theron-disassembly/theron-us-cold-scripted-input-returned-mask-20261031.md) — three scripted inputs reach System Card polls; no game-owned poll or runtime CD source provenance was observed
- [Theron System Card RUN trace](source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary) and [bounded `$EA3B` disassembly](source-lock/theron-disassembly/theron-system-card3-ea3b-20261031.asm) — callback byte and BIOS branch evidence; game handoff remains unobserved

## Per-game technical notes

Focused implementation, format and parity notes are grouped by game. These
pages provide starting points; current support claims remain in the status
documents above and in the per-game TODO/DONE logs.

| Folder | Starting points |
|---|---|
| DM1 (`docs/dm1/`) | [V1 bug audit](dm1/DM1_V1_BUG_AUDIT.md), [placeholder inventory](dm1/DM1_PLACEHOLDER_INVENTORY.md) |
| CSB (`docs/csb/`) | [graphics gaps](csb/csb_gap_graphics.md), [mechanics gaps](csb/csb_gap_mechanics.md) |
| DM2 (`docs/dm2/`) | [overview](dm2/dm2_overview.md), [platform variants](dm2/dm2_variants_platform.md) |
| Nexus (`docs/nexus/`) | [startup](nexus/nexus_startup.md), [file classification](nexus/NEXUS_FILE_CLASSIFICATION.md), [frame 119 menu source joins](nexus_vdp1_menu_frame119_source_join.md), [Ymir gameplay VDP priority composition](nexus_ymir_vdp_priority_composition.md), [cold-boot frame 13162 composition](nexus_vdp1_coldboot_frame13162_composition.md) |
| Theron's Quest (`docs/theron/`) | [capture readiness](theron/THERON_CAPTURE_READINESS.md), [real-data inventory](theron/THERON_REALDATA_INVENTORY.md) |
| Shared engine and UI (`docs/shared/`) | [launcher options](shared/menu_options.md), [DM1 original entrance reference](shared/menu_startup.md), [champions](shared/champ_classes.md), [AI](shared/ai_creature.md) |

## Active per-game work logs

These files record implementation evidence and the remaining gaps for each
game. Read them with the status matrix above: a completed focused check closes
only the behavior and edition it names.

- [Dungeon Master TODO](https://github.com/yeager/firestaff/blob/main/TODO-dm1.md) and [DONE log](https://github.com/yeager/firestaff/blob/main/DONE-dm1.md)
- [Chaos Strikes Back TODO](https://github.com/yeager/firestaff/blob/main/TODO-csb.md) and [DONE log](https://github.com/yeager/firestaff/blob/main/DONE-csb.md)
- [Dungeon Master II TODO](https://github.com/yeager/firestaff/blob/main/TODO-dm2.md) and [DONE log](https://github.com/yeager/firestaff/blob/main/DONE-dm2.md)
- [Nexus TODO](https://github.com/yeager/firestaff/blob/main/TODO-nexus.md) and [DONE log](https://github.com/yeager/firestaff/blob/main/DONE-nexus.md)
- [Theron's Quest TODO](https://github.com/yeager/firestaff/blob/main/TODO-theron.md) and [DONE log](https://github.com/yeager/firestaff/blob/main/DONE-theron.md)

## Wiki map

The checked-in pages under [`docs/wiki/`](wiki/) are the source for the GitHub
wiki and are copied by `scripts/sync_wiki.sh` during release work.

### CSB Atari ST provenance

- [CSB Atari Mania reference](csb/CSB_ATARIMANIA_REFERENCE.md) — local Atarimania
  print-PDF provenance, stated product facts and strict evidence boundary.

### Per-game pages

- **DM1:** [technical reference](wiki/DM1-Technical-Reference.md), [PC34 internals](wiki/DM1-PC34-Internals.md), [reverse engineering](wiki/DM1-Reverse-Engineering.md), [FM Towns guide](wiki/DM1-FMTowns-Guide.md)
- **CSB:** [technical reference](wiki/CSB-Technical-Reference.md), [DSA and saves](wiki/CSB-DSA-and-Save-Internals.md), [FM Towns guide](wiki/CSB-FMTowns-Guide.md), [reverse engineering](wiki/CSB-Reverse-Engineering.md)
- **DM2:** [technical reference](wiki/DM2-Technical-Reference.md), [GDAT internals](wiki/DM2-GDAT-Internals.md), [reverse engineering](wiki/DM2-Reverse-Engineering.md), [platform status](PLATFORM_STATUS.md)
- **Nexus:** [technical reference](wiki/Nexus-Technical-Reference.md), [DGN/PRS3](wiki/Nexus-DGN-and-PRS3-Internals.md), [SAL/MAP](wiki/Nexus-SAL-MAP-Internals.md), [reverse engineering](wiki/Nexus-Reverse-Engineering.md)
- **Theron:** [technical reference](wiki/Therons-Quest-Technical-Reference.md), [Track 02 internals](wiki/Therons-Quest-Track02-Internals.md), [reverse engineering](wiki/Therons-Quest-Reverse-Engineering.md), [input notes](theron/theron_input.md), [capture readiness](theron/THERON_CAPTURE_READINESS.md)

### Shared pages

- [Wiki home](wiki/Home.md)
- [Reverse-engineering index](wiki/Reverse-Engineering-Index.md)
- [Game data](wiki/Game-Data.md)
- [Game-data formats](wiki/Game-Data-Formats.md)
- [Architecture](wiki/Architecture-Overview.md)
- [Building and installing](wiki/Building-and-Installing.md)
- [Parity evidence](wiki/Parity-Evidence.md)
- [Platform guides](wiki/Platform-macOS.md)
- [Release process](wiki/Release-Process.md)

## Evidence vocabulary

- **Source-locked:** behavior is anchored to a reference source or disassembly and guarded by a focused test.
- **Real-data verified:** the tested asset is identified by its canonical hash and the relevant parser/loader contract passes.
- **Runtime proven:** the real-data path reaches the claimed runtime milestone.
- **Playable:** the public route has end-to-end original-data runtime proof. At present this label is reserved for the DM1 PC 3.4 V1 route.
- **Fixture-only:** useful for a narrow parser or state-machine contract, but not evidence for original-media rendering.

## Rebuilding the documentation

Documentation changes are ordinary repository changes. Before publishing a
release, run the focused tests for the affected game, check Markdown links and
run the build/CI commands in [CI.md](CI.md). Do not commit original game data,
emulator saves, private capture logs or generated placeholder art.

**Last reviewed:** 2026-11-01. The current status reflects the source and
authenticated-media evidence recorded on the linked status and per-game
work-log pages. The complete clean-source Mednafen 1.32.1 patch chain passed
three loops, and the `$C852` instrumented PCE Fast binary built against real
SDL2 2.33.0 and passed isolated runtime verification. A rebuilt cold US replay
reached Drator `left-event` and reentry, and captured 132 `$C852` fetches at physical
`$0DC852`; the first five 512-byte windows exactly match Track 02 offset `$A3AD2`. The filtered
consumer trace hit its cap after recording the same-session row write/read
match and a later ten-byte TII copy. Callback-state verification passed. The
8,388,608-row BIOS trace is explicitly truncated; its streaming verifier
processed the file at about 18 MB RSS and correctly rejected the truncation
marker. The runtime `$C852` source is mapped to Track 02 raw offset `$A3AD2`; downstream buffer semantics remain open; the wrapper rejected the capped BIOS trace. A corrected PCE Fast state capture validates the
input-gated VRAM trace
after the authentic guest read; its bounded 4,096-row receipt does not establish
causality or a renderer join. A cold US follow-up validates three scripted inputs at System Card
controller polls, while its `$E009` dispatch remains unjoined to game-owned
polling and runtime CD data; native support status is unchanged. Each receipt
remains bounded to its cited edition, route and test; it does not imply
complete parity.

The reference capture wrapper now offers a BIOS run-path PC filter for the
source-locked `$EA4D`, `$EA50` and `$E162` accesses and callback target
`$2259`. Its instrumenter, streaming verifier and wrapper regressions passed
three loops; no original-game capture has used the filter yet, so this tooling
change does not alter the current Theron evidence or support status. See
[capture readiness](theron/THERON_CAPTURE_READINESS.md).

The 2026-11-01 ISO projection receipt also binds the same eight creature-pointer
and five regular-spawn zone records from the authenticated US and JP full ISO
projections as from their regional raw BINs. Pointer records 0–4 resolve in
order to the five zones in both variants; records 5–7 have zero offsets.
Native ISO dungeon startup admits these records as source metadata. This static
join does not establish runtime pointer selection, category semantics, runtime
consumers or RNG behavior. See the [projection source lock](source-lock/theron-track02-spawn-record-iso-projection-20261101.md).
