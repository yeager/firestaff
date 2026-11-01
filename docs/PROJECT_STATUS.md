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
| Theron's Quest | Verified bounded native JP and US routes; active real-media bring-up | Native direct launch skips Firestaff's Run menu/System Card; US Debug boot probe ends at `levelLoaded=0`. Native JP fresh start reaches a loaded dungeon; authentic US Track 02 plus matching Ak-Tu-Ba-complete BRAM reaches dungeon 2 through Continue. The eight selected regional real-media/native startup, later-dungeon and input CTests passed three consecutive loops (24 executions); SDL coverage includes mouse-panel/touch input, movement, wait, champion selection and US item pickup/reselect/drop. A fresh native macOS Debug build also passed the authentic US SDL event-loop suite three times (348.18 seconds total); see the [input run receipt](source-lock/theron-native-sdl-input-real-media-20261101.md). Native M11 rejects the authentic empty JP BRAM; positive JP progress restoration remains open. Source-only loaders admit all seven dungeons in both regions. Authenticated US/JP full-ISO projections now bind the same eight creature-pointer and five regular-spawn zone records as their raw BINs; pointer records 0–4 resolve in order to those zones, and records 5–7 have zero offsets. This is source metadata only, not evidence of runtime pointer selection, category semantics, or spawning. The native `--debug` receipt matches all 8 pointer and 5 zone records in three loops per region from authenticated raw BINs and three more per region from authenticated split MODE1/2048 ISO files; the reusable verifier passes all twelve loaded-level runs. This verifies the diagnostic, not original spawn selection or behavior. See [receipt verification](source-lock/theron-track02-spawn-debug-receipt-20261101.md). An earlier authenticated JP state-only replay consumed five scripted controller events but recorded zero spawn-consumer/register entries, zero RNG samples, and no dungeon transition; it does not establish movement or spawn semantics. The latest matched JP state-only UP/control pair verified a guest read at `$44C1`, then recorded 256 identical authentic-source-matched data reads per run before the map arrays; no movement-command write was observed and movement/render semantics remain open. See the [trace report](source-lock/theron-disassembly/theron-jp-state-only-up-control-data-read-20261101.md). A cold JP reference capture confirms four-second Button I holds on NEW GAME and FILE 1 advance into Akutuba; its `.mc0` is an emulator device snapshot, not BRAM, and the captured 2 KiB BRAM remains empty. The latest visual cold JP follow-up remained at the PC Engine Super CD-ROM System splash after a PID-targeted RUN attempt; the input trace showed only released-button samples and the wrapper rejected the package because the System Card status trace had no `$1800` reads. An earlier PID-bound run recorded 15 presses and active-low RUN/I at BIOS poll `$E4C5`, but no verified `$E11D` handoff, game-owned poll, CD IRQ, authenticated CD-to-RAM receipt, or LBA 3674 read; the exact-LBA tool has not yet captured an accepted game load. Its data-port verifier now cross-checks authenticated raw BIN and MODE1/2048 ISO layouts and can require complete byte coverage of the descriptor/map source span; format-only regression fixtures do not add gameplay evidence. Three JP PCE Fast pairs show repeatable guest-read-conditioned `$C1FC` TII transfers from `$20B4` to candidate `$2040/$2041` bytes; a 128-instruction repeated context includes `$C1D6` call/return and conditional fall-through. The static source associates the TII with teleporter destination copying, so the dynamic path does not establish player movement. An isolated authentic US cold Drator replay reached `start`, `left-event` and reentry and recorded 132 `$C852` fetches; its first five 512-byte runtime windows at `$0DC852` exactly match authentic Track 02 raw offset `$A3AD2`. Its same-session trace joins writes and reads of generated row offsets 6–8 and records a later ten-byte TII copy to a work buffer. A strict verifier accepts constructed `$CBC6` and `$45C3` fetch windows read from the authenticated US BIN and lists four and three matching raw offsets, respectively; these tooling rows do not bind the live reader instructions to a unique runtime LBA/occurrence. The filtered observer reached its cap after the row reads; route reentry was recorded separately. The 8,388,608-row BIOS trace is explicitly truncated; its streaming verifier processed it at low memory and correctly rejected the truncation marker. The wrapper rejected the capped capture package. A later authentic JP state-only trace reached the 8,192-row `$3879` cap before `$C852`; its separate first-hit sidecar reported zero rows (`not_observed`), and JP `$C85E` was opcode `$A6` at `$0D285E` with no `$5039`. A later authentic US cold trace records ordered `$C852` → `$C85E` `JSR $5039` → `$5039` dispatch under MPR2 `$68`; live bytes at `$5039` differ from the static candidate. Its wrapper package was rejected because live `$45C3` did not match retail Track 02 and status/consumer traces hit their caps. A hash-gated scan found no contiguous 8 KiB match for the 102 unique MPR2 `$68` windows in raw US Track 02 or Track 19; a separate authentic `$23AD` output matched 0/102 windows, while other decode or runtime-write sources remain possible. C852 schema v3 and the all-writer trace share a per-run capture ID, with wrapper-receipt hashes and full-page completeness checks. The verifier has fixture coverage, but no authentic same-session capture has exercised it. Generated-row semantics remain open. A later authentic run used the source-scoped BIOS and Drator-row filters; the capture still failed source validation and reached trace caps. Separate authentic-state capture verifies bounded VRAM trace ordering after a guest read only. An authentic-source disassembly maps the `$50D2` reader window to one raw JP Track 02 occurrence and identifies a candidate list region. Three further authentic JP state-only runs recorded 1,677, 1,513 and 1,556 dynamic map-array reads across 17 source-matched byte addresses in Map 0 and Map 3, without reaching the 4,096-row sample cap. The observed PCs and values do not establish the candidate list’s role, active-map selection or a tile-consumer join; see the [reader report](source-lock/theron-disassembly/theron-jp-akutuba-map-index-reader-20261101.md) and [map-array capture report](source-lock/theron-disassembly/theron-jp-state-only-map-array-read-capture-20261101.md). The JP active-map capture now has a concrete acceptance chain from authenticated Track 02 reads through runtime selection and a pose-bound tile consumer to atomic VDC/VCE/VRAM state; no capture has joined that chain yet. Active-map identity, field semantics, collision, visible effect, state-driven renderer, gameplay CDDA routing, cold CD handoff and native parity remain unproven. The current isolated PCE Fast reference instrumenter has been rebuilt against standalone SDL 2.32.10, but its latest capture is not yet accepted. Production CDDA currently plays title Track 01; test-only decoding of tracks 03–18 and static `$E03F` candidates do not establish a gameplay event-to-track route. | Full wrapper-verified handoff capture; generated-row consumer semantics; downstream row-buffer semantics; source-bound dynamic viewport and paired retail frame; active-map/position semantics; collision and pose-to-render joins; authenticated cold game-owned handoff; later progression; positive JP progress-save round-trip; broader save, audio and complete gameplay parity. See [capture readiness](theron/THERON_CAPTURE_READINESS.md), [ordered US call-window report](source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md), [Drator capture boundary](source-lock/theron-us-drator-c852-cold-capture-20261031.md), [runtime spawn capture](source-lock/theron-disassembly/theron-runtime-spawn-capture.md), [state comparison](../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md), [active-map capture chain](../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain), and [instruction context](source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md). |

Latest reference-capture update: an authentic cold JP map-byte attempt was rejected without the `$E11D` launch branch, an LBA 3674 data read, or verified consumption of every scripted input event. The corrected input verifier now distinguishes controller slot from serial shift index, but still blocks this capture. It establishes no loader provenance or gameplay input; see the [cold JP map-byte capture](source-lock/theron-disassembly/theron-jp-cold-map-byte-writer-capture-20261101.md).


An offline scan of an authentic JP `.mc0` state found the exact 1,260-byte
Akutuba descriptor-and-map source span in the inflated payload at `0x6633b`.
A source-locked Mednafen 1.32.1 inspector maps it to HuC CD-RAM physical page
`$74` (`$0E891D`); the saved MPRs do not select that page. All four maps are
present, so the active map remains unknown. The state includes an embedded
preview and named VCE/VDC0/VRAM/SAT snapshot variables, but no pose-bound
consumer. This does not change the native parity boundary; see the [source-match report](source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md#offline-save-state-source-match-2026-11-01).

A read-only comparison of the authenticated US standard-PCE state
(`f17f377df210b4a3ae904a13fb85a7f0`) and US Track 02 maps all four authentic
Akutuba arrays to `HuC.SysCardRAM` offsets `0x18ad7`, `0x18afa`, `0x18bd2`
and `0x18cf2`. Saved MPR3 selects page `$74`, which maps the page into
`$6000–$7fff`; the four arrays remain resident together, so this does not
identify the active map. Two addresses from an older movement trace align
with bytes in map 0 under the saved mapping, but that trace lacks a
contemporaneous MPR record. See the [US state map binding report](source-lock/theron-us-authentic-standard-pce-state-map-binding-20261101.md).

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
| Theron's Quest | Firestaff runs the game in its native C/SDL runtime. Direct native launch enters Theron's title/startup path without Firestaff's Run menu or a System Card; the dedicated US Debug boot probe reported `levelLoaded=0`. Separately, JP reaches a loaded dungeon with native ticks and rendered frames, and authentic US Continue reaches dungeon 2. A recent state-only comparison verifies the guest UP read but not movement; a source-locked `$50D2` disassembly and bounded data-read trace identify a candidate pointer-like list, not an active-map or tile-consumer join. Three authentic JP state-only traces recorded 1,677, 1,513 and 1,556 dynamic map-array reads across 17 source-matched byte addresses in Map 0 and Map 3, with no 4,096-row cap reached; active-map and pose-to-render semantics remain open. Hash-verified raw-BIN windows disassemble the four reader paths; the v4 PCE Fast hook now has a wrapper-accepted state-only UP/control capture with same-event opcode/register/Page1/MPR context and 946 map-array reads. One byte at `$0E8B14` differs from the authenticated BIN in the autoloaded state; an audit found the original target-write trace excluded that address, so its zero-row result is inconclusive. An isolated rebuilt instrumenter includes it, but cold loader provenance, active-map and pose-to-render joins remain open. See the [map-index context capture](source-lock/theron-disassembly/theron-jp-state-only-map-index-context-capture-20261101.md). The viewport is not yet proven to update from authenticated game state, and neither dungeon route has a paired retail image. A separate authentic US reference trace records `$C852` → `$C85E` `JSR $5039` → `$5039` under MPR2 `$68`; live target bytes differ from the static candidate, and the wrapper rejected the package because live `$45C3` failed source matching and bounded traces hit their caps. Host save persistence and campaign-state translation remain distinct evidence boundaries. | [Theron platform rows](PLATFORM_STATUS.md#therons-quest), [capture readiness](theron/THERON_CAPTURE_READINESS.md), [US ordered call-window report](source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md), [runtime-page source scan](source-lock/theron-disassembly/theron-us-mpr2-68-runtime-page-source-scan-20261101.md), [direct-launch evidence](source-lock/theron-native-debug-direct-launch-20261031.md) |

The completion matrix also includes evidence-coverage scores for individual
game targets; those scores describe documented gate coverage, not a percentage
of a finished game. Nexus separately reports its six-domain implementation
coverage (44.0%), weighted startup-to-HUD chain (33.1%), and production grade
(0%) under its Saturn capture model. Neither set of figures replaces the
evidence-backed status labels above.

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
