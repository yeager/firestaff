# Firestaff project status

**Last status review: 2026-10-31.** This page is the
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
| Theron's Quest | Verified bounded JP runtime and US routes; active real-media bring-up | Direct native launch enters Theron's title/startup path without Firestaff's Run menu or a System Card; the US Debug boot probe stops before dungeon loading. Native JP fresh start reaches a loaded dungeon; an isolated cold Mednafen route confirms the four-second RUN hold and New Game navigation, but its saved 2 KiB BRAM matches the known empty JP image. Authentic US Track 02 plus Ak-Tu-Ba-complete BRAM reaches dungeon 2 through Continue and exercises bounded movement and item transactions. Native M11 also rejects the authentic empty JP BRAM as Continue; no progressed JP BRAM is available to verify positive JP progress restoration. All seven dungeon sources load from authentic media; raw/assembled-ISO equivalence covers 34 maps and 2,269 linked source-object records per region; separate world ledger retains 2,266 ground-reference occurrences. Three JP PCE Fast UP runs share an event sequence from the guest poll at `$44C1` (event 2342) through `$D334` reading `$2912=00` (3000), `$D349` writing `$10` (3001), and `$D334` reading `$10` (3023); three neutral controls show no such consumer/writer path. A follow-up same-state UP/neutral/DOWN check faces a wall in every reference screenshot and sees no position write at the bounded `$2040/$2041` hooks or `$C1FC` TII; this is a blocked-wall result, not movement evidence. The event ordering still does not prove causality, field meaning, or party/render effect. The US save-state mouse replay records guest reads and candidate command-consumer activity but has no runtime CD provenance or proven action. A cold US follow-up validates three scripted inputs at System Card controller polls; the `$E009` dispatch has no game-owned poll or runtime CD source-read join. | No source-bound dynamic viewport or paired retail image; game-owned cold handoff, initial pose/map join, reachable-position movement join, later progression, JP progress-save round-trip, broader save semantics, and complete gameplay remain open. The event ordering does not prove causality, and `$2912` semantics/effect remain unknown. The empty emulator BRAM is not progression evidence. The Japanese cold RUN trace reaches BIOS only in its traced run; the US `$E009` receipt remains unjoined. See the [current capture-readiness summary](theron/THERON_CAPTURE_READINESS.md). |

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
