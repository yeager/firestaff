# Firestaff project status

**Last status review: 2026-11-01.** This page is the
concise status source for the README and wiki. The full documentation
map is in [`DOCUMENTATION_INDEX.md`](DOCUMENTATION_INDEX.md).

This is the short, user-facing status summary for the whole project. Detailed
source audits and capture receipts remain in the linked game documentation.

| Game | Current status | What is verified | Main open boundary |
|---|---|---|---|
| Dungeon Master 1 | Playable PC DOS 3.4; bounded native routes for other editions | PC 3.4 runtime, viewport, HUD, input, combat and saves; Apple IIGS 2.0/2.1 original-media CLI/M12 first-world startup; Apple Silicon macOS boot smoke and original-data gates | Broader original-vs-Firestaff capture, IIGS title/entrance and DOC playback, and V2 material; the unreleased historical Macintosh prototype has no local authenticated data |
| Chaos Strikes Back | Verified bounded real-media routes; active hardening | Atari STX 50 Hz title-to-FTLCODE/runtime route, start-menu CLI, Amiga and FM Towns routes; FM Towns C28 entrance palette and MINI.DAT bootstrap state; Amiga F0107 source MEDIA720 scale and C696 anchor selection from authentic Graphics.DAT; SWSH palette/logo/sample parsing and playback path | Matched Amiga frame, palette/light, inscription and trapdoor verification; native intro timing and audible output; DSA/save corpus, wider campaign and viewport evidence |
| Dungeon Master II: Skullkeep | Verified bounded real-media routes; active hardening | M12 source-owner selection, English/French DOS menu-to-runtime routes, Amiga title/New Game/runtime, FM Towns in-memory title/menu/runtime slices, source-owned viewport/HUD and focused mechanics | Edition-matched title palette/timing, input, audio and dungeon-composition parity across editions |
| DM Nexus | Active real-data bring-up | Saturn DMDF/DGN data, CUE-owned CDDA and STABG source selection, source-bound decoding and bounded rendering of five authentic MAPD title planes, capture-only VDP1/VDP2 decoders, and an NBG1 tile-fetch address correction tested against an authentic Japanese startup frame in merged PR #19. Firestaff does not yet reproduce the native Saturn title composition or menu. Japanese retail and English-version Ymir cold boots reach the native title menu; the English screenshot replay is pixel-exact, and its source join has 126 informative matches among 137 textured draws. These are reference-emulator results only. Extracted JP/EN/FR owners use regional `RLOWFIX.BIN` plus localized UI hashes; supplemental discs must match the owner. Authentic JP and EN menu tests passed, including rejection of a Japanese CUE for an extracted EN owner. The authentic Japanese CUE/BIN-only layout now reaches the same capture-gated title through CLI and menu admission. | Full Firestaff Saturn title composition and title-to-menu parity; menu/face, HUD and dungeon source-to-VDP material semantics; native event/audio playback and full playability; production readiness remains 0% |
| Theron's Quest | Verified bounded native JP and US routes; active real-media bring-up | Native direct launch skips Firestaff's Run menu/System Card; US Debug boot probe ends at `levelLoaded=0`. Native JP fresh start reaches a loaded dungeon; authentic US Track 02 plus matching Ak-Tu-Ba-complete BRAM reaches dungeon 2 through Continue. The eight selected regional real-media/native startup, later-dungeon and input CTests passed three consecutive loops (24 executions); SDL coverage includes mouse-panel/touch input, movement, wait, champion selection and US item pickup/reselect/drop. Native M11 rejects the authentic empty JP BRAM; positive JP progress restoration remains open. Source-only loaders admit all seven dungeons in both regions. Authenticated US/JP full-ISO projections now bind the same eight creature-pointer and five regular-spawn zone records as their raw BINs; pointer records 0–4 resolve in order to those zones, and records 5–7 have zero offsets. This is source metadata only, not evidence of runtime pointer selection, category semantics, or spawning. A cold JP reference capture confirms four-second Button I holds on NEW GAME and FILE 1 advance into Akutuba; its `.mc0` is an emulator device snapshot, not BRAM, and the captured 2 KiB BRAM remains empty. Three JP PCE Fast pairs show repeatable guest-read-conditioned `$C1FC` TII transfers from `$20B4` to candidate `$2040/$2041` bytes; a 128-instruction repeated context includes `$C1D6` call/return and conditional fall-through. The static source associates the TII with teleporter destination copying, so the dynamic path does not establish player movement. An isolated authentic US cold Drator replay reached `start`, `left-event` and reentry and recorded 132 `$C852` fetches; its first five 512-byte runtime windows at `$0DC852` exactly match authentic Track 02 raw offset `$A3AD2`. Its same-session trace joins writes and reads of generated row offsets 6–8 and records a later ten-byte TII copy to a work buffer. A strict verifier accepts constructed `$CBC6` and `$45C3` fetch windows read from the authenticated US BIN and lists four and three matching raw offsets, respectively; these tooling rows do not bind the live reader instructions to a unique runtime LBA/occurrence. The filtered observer reached its cap after the row reads; route reentry was recorded separately. The 8,388,608-row BIOS trace is explicitly truncated; its streaming verifier processed it at low memory and correctly rejected the truncation marker. The wrapper rejected the capped capture package. Generated-row semantics remain open. Source-scoped BIOS and Drator-row filters pass tooling checks, but no new authentic run has used them. Separate authentic-state capture verifies bounded VRAM trace ordering after a guest read only. The JP active-map capture now has a concrete acceptance chain from authenticated Track 02 reads through runtime selection and a pose-bound tile consumer to atomic VDC/VCE/VRAM state; no capture has joined that chain yet. Active-map identity, field semantics, collision, visible effect, cold CD handoff and native parity remain unproven. | Full wrapper-verified handoff capture; generated-row consumer semantics; downstream row-buffer semantics; source-bound dynamic viewport and paired retail frame; active-map/position semantics; collision and pose-to-render joins; authenticated cold game-owned handoff; later progression; positive JP progress-save round-trip; broader save, audio and complete gameplay parity. See [capture readiness](theron/THERON_CAPTURE_READINESS.md), [Drator capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md), [state comparison](../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md), [active-map capture chain](../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain), and [instruction context](source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md). |

An offline scan of an authentic JP `.mc0` state found the exact 1,260-byte
Akutuba descriptor-and-map source span in the inflated payload at `0x6633b`.
All four maps are present, so the snapshot does not identify the active map or
map that file offset to guest RAM/MPR. The state scan does not change the native
parity boundary; see the [source-match report](source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md#offline-save-state-source-match-2026-11-01).

A newer JP PCE Fast state-only reference capture validates 4,096 bounded VRAM
commits after an authentic-state guest read at `$44C1` (shared events 58758–58761).
It confirms trace-gate operation and event ordering only: the run reports
`transition=missing` and does not prove causality, a visible response, cold
media handoff, or Firestaff parity. See the [source-locked capture report](source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md).

The opt-in `$C852` fetch hook has now been used in an authentic US cold replay
that reached Drator start, left-event and reentry. Its first five 512-byte
runtime windows exactly match US Track 02 raw offset `$A3AD2`. The same-session
trace proves a write/read match for generated-row offsets 6–8 and a later
ten-byte TII copy to a work buffer. A strict source-byte verifier matches
constructed `$CBC6` and `$45C3` windows against the authenticated US BIN and
finds four and three matching raw offsets, respectively. These constructed
windows do not identify a live fetched occurrence or bind it to an LBA. The
8,388,608-row BIOS trace is explicitly
truncated; its streaming verifier rejects the cap marker. Generated-row
semantics remain open; the wrapper completed but rejected the capped BIOS trace. See
[capture readiness](theron/THERON_CAPTURE_READINESS.md) and the
[source-locked capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md).

The latest local M12 real-media sweep separately exercised keyboard and mouse
selection for all five game cards. Its first-runtime route passed for DM1,
CSB, DM2 and Theron US; a direct Theron JP selection also passed. Nexus
selection remains an authentic title handoff only; it does not pass the Saturn
menu capture gate or start gameplay. The detailed commands, data-preservation checks and limits are in
the [startup support gap analysis](STARTUP_SUPPORT_GAP_ANALYSIS.md#latest-local-real-media-menu-check-2026-10-27).

## Completion reporting

Firestaff does not assign invented percentages to a game, version or platform.
Completion is reported by the strongest evidence-backed status in the platform
matrix: **Playable**, **Verified route**, **Data path**, **Preservation**, or
**Unsupported**. A verified boot probe is therefore not reported as completed
gameplay, and a parser or synthetic fixture never raises a row above its
real-media evidence.

| Game | Completion status | Platform source of truth |
|---|---|---|
| Dungeon Master 1 | Playable on PC DOS 3.4; other editions remain separately scoped. | [DM1 platform rows](PLATFORM_STATUS.md#dungeon-master) |
| Chaos Strikes Back | Verified bounded routes on Atari ST, Amiga and FM Towns; no original DOS/PC release. | [CSB platform rows](PLATFORM_STATUS.md#chaos-strikes-back) |
| Dungeon Master II: Skullkeep | Verified bounded routes on DOS, Amiga, FM Towns and the listed Macintosh editions. | [DM2 platform rows](PLATFORM_STATUS.md#dungeon-master-ii-the-legend-of-skullkeep) |
| DM Nexus | Verified Saturn data and bounded MAPD title-plane render; Japanese retail and English-version Ymir references reach the native menu, but Firestaff does not yet reproduce that native title/menu or start Nexus as a playable game. The measured implementation coverage is documented separately. | [Nexus platform row](PLATFORM_STATUS.md#dm-nexus), [Nexus completion](nexus/NEXUS_COMPLETION.md), [English cold-boot evidence](nexus/nexus_startup.md#english-cold-boot-reaches-the-native-title-menu-2026-10-27) |
| Theron's Quest | Firestaff runs the game in its native C/SDL runtime. Direct native launch enters Theron's title/startup path without Firestaff's Run menu or a System Card; the dedicated US Debug boot probe reported `levelLoaded=0`. Separately, JP reaches a loaded dungeon with native ticks and rendered frames, and authentic US Continue reaches dungeon 2. The viewport is not yet proven to update from authenticated game state, and neither dungeon route has a paired retail image. Host save persistence and campaign-state translation remain distinct evidence boundaries. | [Theron platform rows](PLATFORM_STATUS.md#therons-quest), [capture readiness](theron/THERON_CAPTURE_READINESS.md), [direct-launch evidence](source-lock/theron-native-debug-direct-launch-20261031.md) |

Nexus is the sole exception with an explicitly defined numeric measurement:
44.0% implementation coverage across its six measured domains, 33.1% for its
weighted startup-to-HUD chain, and 0% production grade. Those values describe
only the documented Saturn capture model and must not be averaged with the
status labels above.

The dated [preservation status](PRESERVATION_STATUS_2026-08-11.md) records
the current source, format and real-media boundary for every game.

## CSB boundary in brief

CSBWin is retained as a source/disassembly reference for engine and format
research. It is not game data, not a Firestaff PC route, and cannot make CSB
into a DOS title: CSB has no original DOS release and `--platform pc` remains
closed. PC-9801 and X68000 are also unsupported CSB platforms.

## Shared presentation priority

Across all five games, presentation work is sequenced in the same order:

1. **Startup** — authentic media admission, title/intro route and launch gate.
2. **Menu** — source-bound navigation, options and game selection.
3. **HUD** — real bitmap, palette, font and layout ownership.
4. **Viewport** — source-backed map, object, creature, lighting and camera output.

An earlier stage must be stable before a later stage is described as complete.
This keeps a working launcher or synthetic HUD fixture from being mistaken for
full original-data gameplay.

## Evidence rules

“Verified” means that the relevant source or real-data receipt exists and its
focused checks pass. A synthetic fixture can prove a parser or state-machine
contract, but it cannot prove original-media parity. Screenshots in public
documentation must be real runtime captures; generated or fallback art is not
promoted as game evidence.

## Runtime dependency boundary

Production Firestaff starts only from user-supplied game media. It neither
requires nor reads BIOS, firmware, or System Card files, and it never launches
an external emulator. Historical capture and reverse-engineering material may
refer to those tools as evidence provenance, but it is not a runtime input or
installation requirement.

## Build and CI

The canonical local build is documented in [`DATA_SETUP.md`](DATA_SETUP.md)
and the CI checks in [`CI.md`](CI.md). GitHub Actions runs strict warnings,
asset hygiene, native builds on three operating systems, headless probes and a
cross-platform determinism comparison. Rapid pushes to `main` cancel obsolete
runs, so status must always be read from the newest commit.

The [Firestaff wiki](https://github.com/yeager/firestaff/wiki) expands this
summary with per-game format, hardware and reverse-engineering notes. The
checked-in wiki source is under [`docs/wiki/`](wiki/).
