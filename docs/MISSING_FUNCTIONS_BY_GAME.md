# Missing Functions and Verification by Game

Status: 2026-11-01. This is a summary of Firestaff's remaining gaps, not a
list of every historical TODO or symbol name.

For Theron, the current evidence summary is
[`THERON_CAPTURE_READINESS.md`](theron/THERON_CAPTURE_READINESS.md); dated
source-lock reports are supporting records, not parallel status pages.

Each item is classified as follows:

- **Missing code** – the function or its production route is not implemented.
- **Missing original data** – the source format or a sufficient corpus is
  unavailable.
- **Missing verification** – the route exists, but needs authentic runtime,
  pixel, or capture evidence before it can be considered complete.
- **Intentionally closed** – a synthetic or uncertain route must not be
  reopened.

The respective game TODO is the source for detail: [DM1](../TODO-dm1.md),
[DM2](../TODO-dm2.md), [CSB](../TODO-csb.md), [Nexus](../TODO-nexus.md), and
[Theron](../TODO-theron.md).

## DM1

### Missing Original Data or Evidence

- **Authentic C13 save:** no local PC34 save is verified to contain the
  requested C13 event. Existing saves provide C03/C04 data, but do not replace
  a C13 corpus.
- **Original-versus-Firestaff pixel pairs:** broader pairs are missing for the
  viewport, creature chain, champion HUD, panels, launcher, effects, and
  macOS/app runs.
- **Original capture of C13/HoC/top-row/action routes:** capture scripts and
  source gates exist, but coverage is incomplete.
- **FM Towns:** some TownsOS EGB pixel semantics and mouse/input routing still
  depend on capture/BIOS evidence. This concerns original parity, not
  Firestaff's native launch route: the authentic FM Towns ZIP starts without a
  BIOS.

### Missing or Incomplete Functionality

- Broader sensor, removal, and DSA interactions still need source-owned
  original-save/runtime evidence.
- The creature and combat chain has verified parts, but broader group layout,
  creature combat, and additional timeline cases are not fully proven.
- DM1 V2.2 lacks a reviewed, authentic material package and complete
  source-owned modern presentation. Placeholder/procedural art must not be
  activated as a replacement.

### Not a Remaining Production Gap

DM1 V1's normal PC34 viewport, HUD, inventory, and action routes have source
gates and fail-closed behaviour. The word “synthetic” in receipts, negative
probes, or ReDMCSB's own `synthetic wall` terminology does not by itself mean
that production code draws synthetic DM1 pixels.

**Native startup matrix (verified 2026-08-29):** Firestaff reads the real media
directly in memory and reaches `dm1-runtime` with `levelLoaded=1` from both
the CLI and start menu for ten supplied release routes: English DOS 3.4 ZIP,
French DOS after manual RAR2 extraction, Atari ST English outer/nested ZIP,
Atari ST German and French preservation ZIPs, Amiga HD and 2.0 ZIP → ADF,
and FM Towns JA/EN ZIP. The focused `dm1_v1_*_cli_boot` CTests check the
canonical asset MD5 and never use an emulator or BIOS at runtime. The
authentic French `DMSAVE.DAT`/`.BAK` pair additionally passes a source-backed
F0435 → F0433 → F0435 round trip against its original `DUNGEON.DAT`; party,
C03/C04 timeline and active groups remain intact. This is startup/save
evidence, not a claim of complete original pixel or game-system parity.

## DM2

### Missing Code or an Incomplete Production Route

- **Database ownership:** Firestaff's DM2 model still needs fully validated
  original record pools, links, maps, and relocation semantics instead of the
  reduced save-state layout.
- **Input and dialog:** the original resume selector, keyboard/mouse ordering,
  held-button semantics, modal dialog, text, cancel, and event-queue behaviour
  are incomplete.
- **Creature drop/AI:** AI tables and parts of the drop route exist, but
  source-owned `CREATURE_AI` records must be bound to graphics, possession,
  death, and cooldown before the route can be considered complete.
- **Save/load and GAME_LOAD:** the SKSAVE format and remaining runtime-state
  ownership need a broader original corpus and packaged-app verification.
- **CCM and cell effects:** advanced `DM2_PROCEED_CCM`, complete cell-content
  digest/map changes, and teleporter effects are not fully implemented.
- **V2.2 rendering:** real material/pixel consumption, clipping, additional
  depth/outdoor routes, and runtime wiring are missing.
- **V2 HUD:** authentic text/bitmap assets and more widgets, including
  inventory quick-view and action prompts, are missing.

### Missing Original Data or Verification

- Weather needs original save/memory snapshots that bind the timer record to
  its correct owner and change.
- A complete DOS framebuffer/blitter capture is needed for palette/clipping
  parity.
- MIDI and some runtime sound owners lack instruction-level traces and
  source-bound save/runtime evidence.
- Demo and non-PC extracts need separate version/container classification.

### Intentionally Closed

`examples/dm2_hud_widget_synthetic/` and procedural V2.2 art are gate fixtures
only. They must not replace the real DM2 GDAT source or be presented as
completed Skullkeep graphics.

**Native startup matrix (verified 2026-08-29):** authentic DOS English and
French, Amiga, FM Towns, and Macintosh ZIPs are read in memory. Both DOS
editions retain their own authenticated GDAT/GAME_LOAD startup owner through
SKULL and New Game; Amiga follows its TITLE stream to runtime; FM Towns follows
AUTOEXEC/TWANIM to GDAT's New Game rectangle and the source-owned champion
selection click; Mac starts the verified retail profile. DM2 DOS MVE can also
be verified headlessly with source-ordered presentation frames in
`dm2_v1_dos_native_cli_boot`. This is startup/runtime evidence, not complete
parity for saves, rendering, AI, or audio.

## CSB

### Missing Original Data or Evidence

- **CSBWin DSA corpus:** a checksum-validated DSA-bearing CSBWin save with
  index/action records is needed for positive breadth.
- **Save corpus per media/version:** a verified original save and round-trip
  proof are needed for every claimed media/version branch.
- **Real capture:** broader original capture of the HUD, viewport, title,
  doors, and macOS/app/other media branches is missing.
- **Audio:** complete audio/runtime parity with source-owned dispatch is not
  complete.

### Missing or Incomplete Functionality

- Deeper end-to-end gameplay parity and playability without DM1 assumptions.
- More DSA timer, generator, teleporter, and sensor transactions with
  authentic save/dungeon pairs.
- Broader source-locked viewport placements, masks, material bindings, and
  draw order for CSB-specific D2/D3 routes.
- V2.2 needs a real PC34 GRAPHICS.DAT-based material/pixel binding before
  modern art can be re-enabled.

### Intentionally Closed

The synthetic dungeon-loader/world fixture and experimental launch fixtures
may continue to test lifecycle and negative branches. They are not substitutes
for CSB's real dungeon, save, or viewport data.

**Native startup matrix (verified 2026-08-27):** Firestaff reads real CSB media
in memory and performs the original title → runtime → first movement sequence,
as well as the start-menu launch handoff, for Atari ST (raw STX and nested
ZIP), Amiga ADF archives, and FM Towns CD archives (English and Japanese
program chains). `csb_v1_{atari_stx,atari_nested_zip,amiga,fmtowns_{en,ja}}_native_cli_*`
proves these routes. Startup evidence does not replace the open original-capture and DSA/save-parity
requirements above.

## Nexus

### Missing Code or an Incomplete Production Route

- **Structure2 material:** complete descriptor, UV, texture, and palette
  semantics are missing for binding raw descriptors to renderable material.
- **Animated material:** payload grammar, sequence semantics, flags, and
  timing are incomplete.
- **Saturn runtime/capture:** a real executable/emulator trace and frame
  capture are required for pixel position, mode, palette, and timing. Static
  ISO inspection is insufficient.
- **Audio:** CUE-declared retail BIN tracks are now source-bound; Saturn's
  dispatcher, native decoder, literal sample/trigger evidence, and playback
  are still missing.
- **Structure1F/VDP1:** several material, texture/palette, and replay gates
  remain capture- or host-route-dependent.
- **VDP2 NBG1 tilemap:** corrected Saturn word/byte address and character-stride
  math was merged in PR #19. A frame-239 capture-only plane decoder now records
  the 40×28 visible 8bpp page. The authentic raw-snapshot test decodes 148
  nontransparent samples, all within eight RGB levels of the retained Ymir
  screenshot (maximum error seven). A capture-only VDP1/VDP2 stack now also
  matches all 286,720 doubled screenshot samples exactly for this frame, using
  its hash-pinned VDP1 FBS and the Ymir software renderer's priority tie rule.
  Arbitrary-frame composition, retail asset ownership, text/page mapping,
  placement, and production rendering remain unresolved.
- **VDP2 NBG2 tilemap:** a bounded frame-239 decoder now measures the 40×28
  visible 8bpp capture plane and matches 9,258 of 11,232 nontransparent CRAM
  samples to the Ymir screenshot within eight RGB levels. The exact frame-239
  full-stack capture match resolves layer priority and color selection only
  for this measured state; complete color calculation, retail source ownership
  and production rendering remain unresolved.
- **VDP2 NBG0 bitmap:** frame 239's existing bounded 8bpp decoder matches
  3,954 of 3,968 nontransparent CRAM samples within eight levels. The exact
  frame-239 full-stack capture match verifies NBG0/NBG1/NBG2 ordering for this
  captured state, but does not establish arbitrary-state color calculation or
  an asset owner.

### Missing Original Data or Verification

- A complete Saturn run, including the BIOS/emulator/capture chain, is the
  primary external gap.
- Original `LEV*.DGN`, `SLEV*.BIN`, `SNDLEV*.SAL`, and `*.MNS` files are
  available locally, but raw bytes are insufficient evidence for semantic
  material, gameplay, or animation interpretation.

### Intentionally Closed

Generated DGN/DMDF/save fixtures may be used for parser and round-trip tests.
They must not be promoted to a playable Nexus world or used as Saturn pixel
evidence.

## Theron

Firestaff has bounded native real-media routes for both admitted regions: JP
fresh start reaches source-verified Ak-Tu-Ba level 0; US Track 02 with matching
Ak-Tu-Ba-complete BRAM reaches dungeon 2 through Continue and exercises
movement, champion selection, wait, and US item pickup/reselect/drop through
the native SDL input loop. USE is dispatched as a command, but authentic door
and object effects remain fail-closed until the T900 consumer is source-bound.
Native M11 rejects the authentic empty JP BRAM; positive JP progress
restoration remains unverified. Source-only loaders cover all seven dungeons
per region. Authenticated US and JP pointer records 0–4 resolve by their
source-relative offsets to the five regular-spawn zone records in order;
pointer records 5–7 have zero offsets. This static source join does not prove
runtime pointer selection or spawning. These results are recorded in the
[capture-readiness summary](theron/THERON_CAPTURE_READINESS.md); they do not
establish complete retail gameplay or visual parity.

A matched three-run JP PCE Fast state-only comparison records Button II+UP
guest reads followed by repeatable `$C1FC` transfers from `$20B4` to candidate
bytes `$2040/$2041`; three Button II-only controls retain the prior bytes.
Source-locked analysis also associates `$C1FC` with copying teleporter
destination coordinates. A follow-up three-run capture records the same
128-instruction path through `$C1D6` and into `$C1FC`, but does not source-bind
the tested fields, establish gameplay purpose, or identify an active-map
object. It therefore cannot establish that the transfer is player movement.
Map identity, byte semantics, collision outcome, visible response, cold CD
handoff and native parity remain open. See the [dynamic instruction-context
capture](source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md).
The separate short-format US
`$44CA/$44DB` trace is still ambiguous because standard-PCE and PCE Fast share
an instrumentation path and label.

### Missing Original Data or Verification

- **Save body layout:** authentic US BRAM Continue and its bounded campaign and
  champion fields are verified, and the authentic empty JP BRAM is rejected.
  Positive JP progress restoration and the complete cross-region BRAM layout
  are not sufficiently source-locked. The separate community `.srm` Save Disk
  body remains opaque and must not be conflated with in-game BRAM or `.tqsv`.
- **Original/Firestaff paired capture:** the shared v2 comparator now requires
  hash-bound authentic-media and `--debug` build receipts per checkpoint, but
  a same-state original-versus-Firestaff image pair and reviewed receipt
  producers are still missing.
- **Startup media parity:** source-bound decoding and paired pixel/audio
  evidence for Track 02 startup art and text are missing; bounded native
  startup routes already exist.
- **Post-$3800 chain:** the continuation consumer after the authenticated
  `$3800` boundary needs further live capture.
- **JP continuation:** authentic Japanese Continue/save semantics and broader
  Japanese campaign behavior remain unverified; do not derive them from US
  data.
- **Broader original corpus:** more authentic CUE/BIN/ISO combinations are
  needed for version and media breadth.

### Missing or Incomplete Functionality

- **Native gameplay breadth is limited:** the verified JP fresh-start and US
  Continue paths cover bounded launch and input slices, not the full campaign
  or every gameplay system.
- Complete source-owned semantics for later levels, objects, champion data,
  and save/load are incomplete.
- Authentic runtime traces are needed for doors, pits, teleporters, altars,
  combat, drops, and sounds outside the already verified level-0/table slices.
- A complete Track 02 bitmap/material decoder and production-bound viewport/UI
  presentation are missing.
- V2.2 lacks an authenticated Track 02 material package; procedural/AI art
  must remain fixture-only.

### Intentionally Closed

No-op, fixture-start, and synthetic parser routes must not create a replacement
level when authentic Track 02 records are unavailable. Production remains
capture-gated.

## Shared Priority Order

1. Obtain or verify the missing authentic save, media, or capture source.
2. Bind bytes to the correct original function, record owner, and runtime
   route.
3. Add a source lock and real-data regression.
4. Add original-versus-Firestaff pixel or timing pairs where the function is
   visual.
5. Keep uncertain or missing data fail-closed; do not create synthetic saves,
   frames, roster data, or material to fill a gap.
