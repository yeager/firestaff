# Firestaff TODO — CSB

2026-10-13 current-main retest on the Mac mini M4, source commit `e7119f2ed`:
authentic Atari STx, Amiga and FM Towns CLI/start-menu regressions all passed
(`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_native_cli_boot`,
`csb_v1_fmtowns_native_cli_boot`). The tests use SDL dummy video/audio drivers;
they establish the tested startup/handoff routes, not native-window parity,
physical input or audible output.

2026-10-13 current-source M4 regression: the authentic Amiga 3.1/3.3 CLI
suite, Amiga 3.3 loose-folder route and A31E archive route all passed against
installed original media (three CTests; 72.63 seconds wall time). The A33M
negative case with `KAOS.FTL` removed was rejected by both scanning and direct
launch; complete A33M/A31M packages and A31E reached their source-owned startup
receipts. This verifies the edition-specific media gate and headless launch
routes, not native-window presentation or audible output.

2026-10-13 local startup matrix on the Mac mini M4 host, source commit
`22bc77676`: the locally built binary passed authentic Atari STx, Amiga and FM
Towns CLI/start-menu regressions (`csb_v1_atari_stx_native_cli_boot`,
`csb_v1_amiga_native_cli_boot`, `csb_v1_fmtowns_native_cli_boot`). GitHub
Actions run `38352901234` passed all jobs on Ubuntu, macOS and Windows. These
local routes used SDL's dummy video/audio drivers; they do not verify the M4's
native Retina window, physical input, audio output or visual parity.

2026-10-13 extended original-media archive matrix on the Mac mini M4, source
commit `458bf820e`: the Atari nested ZIP, original Atari archive, and Amiga ADF
CLI/start-menu tests passed. The French Atari preservation ZIP test was skipped
because `chaos_strikes_back_ftl_france.zip` is not present in the CSB data
directory; the available original Atari archive contains English v2.0/v2.1
game and utility disks only. Do not replace the missing French source with
synthetic media. These headless runs do not verify native display or audio.

2026-10-12 shared FTL decoder bounds: the ReDMCSB `DECOMPDU.C` F0455 token
reader now fails on truncated input instead of synthesizing zero bits after
EOF. The malformed-stream regression and the authentic Amiga `DUNGEON.DAT`
probe pass. This hardens normal FTL dungeon intake; it does not decode the
separate CTRaw `PACK` representation described below.

2026-10-12 v3.0.379 startup regression retest: the released macOS arm64
binary passed authenticated return-to-menu and all-five-game rescan checks
for FM Towns, Atari ST and Amiga media. Direct `--game csb` exit behavior
passed with original FM Towns media. CLI diagnostics confirmed FM Towns AUTO
priority when original Amiga media was also installed. These runs use SDL's
dummy video driver; native M5 Retina input and pixel parity remain unverified.

Reviewed 2026-09-05. Only open work is listed here.

2026-10-13 A35E source verification: on the Mac mini M4, the current local
build passed the authentic Amiga 3.5 English ADF path after byte-preserving ZIP
transport. Explicit Amiga and automatic platform detection passed direct CLI
and production M12 startup; both M12 routes reached first movement at `(9,1)`
with a nonzero viewport receipt. The dedicated A35E M12-to-M11 boundary test
reported 45 passed, 0 failed, 0 skipped. The original DMWeb archive SHA-256
remained unchanged. These headless checks do not verify native display output
or audible playback. Native CTRaw PACK decoding remains unsupported.

2026-10-12 A35E original-ADF retest: the published v3.0.379 macOS arm64
binary authenticated the English Amiga 3.5 original ADF from the DMWeb
software archive (MD5 `d1808d54faf36c6ab7f054908ce7ebd5`). A byte-preserving
ZIP wrapper was accepted by direct `--game csb --platform amiga`: the CLI
reached the native entrance and then the first runtime movement at `(9,1)`.
The released v3.0.380 binary also passed bare `--game csb` and M12 AUTO with
no platform selection: both routes selected authenticated `amiga35-en`
(`Graphics.DAT` MD5 `291e1bc6803e3dc4b974c60117ca5d68`) and reached the first
runtime movement at `(9,1)`. The real-media regression now covers both AUTO
routes so they remain independent of saved platform preferences.
The explicitly pinned production M12 menu also selected the authenticated
`amiga35-en` `Graphics.DAT` (MD5
`291e1bc6803e3dc4b974c60117ca5d68`) and reached the native entrance. A follow-up
menu run now waits through the original entrance, accepts C200, and verifies the
first runtime movement at `(9,1)` with a nonzero viewport receipt. The regression
uses `wait:` in loop frames, not milliseconds. These runs used SDL's dummy
video/audio drivers. The dedicated A35E M12-to-M11 boundary check subsequently
passed 45 assertions on the Mac mini M4. This ADF route is separate from native
CTRaw decoding.

2026-10-11 A35 original-ADF startup verification: the DMWeb software archive
also contains a multilingual Amiga 3.5 original ADF, explicitly labeled
"Original (Not working)." Its extracted bytes match MD5
`4d148dea3fdc6d7098b6a1bbfce7b5c9`; a ZIP stored with no compression retains
the exact bytes. Firestaff identifies its A35M `Graphics.DAT` (MD5
`cefaddfdf5651df2c91f61b5611a8362`) and `Dungeon.DAT`. The direct CLI and
production M12 menu routes both selected `amiga35-multi`, passed the APPB
language selection and entrance, and reached the first runtime movement at
`(9,1)` with a nonzero viewport receipt. The original DMWeb archive remained
unchanged. This verifies this ADF source through ZIP transport in Firestaff;
it does not establish that the disk image works in its original hardware.
Native CTRaw support remains a separate open item: no CTRaw PACK decoding or
sector recovery is implemented, and the ADF member is not a decoded CTRaw
stream.

2026-10-11 local startup retest: the current source passed the authentic
FM Towns, Atari ST and Amiga ADF CLI/start-menu regressions
(`test_csb_v1_fmtowns_native_cli_boot.sh`,
`test_csb_v1_atari_nested_zip_cli_boot.sh` and
`test_csb_v1_amiga_adf_archive_cli_boot.sh`). The checks reached the Towns
MINI.DAT party, Atari runtime movement and Amiga source entrance/runtime
movement. This does not verify native M5 Retina window/input behavior or full
visual parity.

2026-10-12 follow-up: the authentic Amiga 3.5 multilingual ADF, preserved
byte-for-byte in ZIP transport, now also passes bare `--game csb` direct CLI
and M12 AUTO routes without `--platform`. Both routes select A35M, pass the
English language selector and reach first movement. This closes the A35M AUTO
coverage gap for that ADF source; the separate original CTRaw archive still
has no legal/documented PACK decoder and remains unsupported.

- Add a source-preserving launch path for the supplied Amiga 3.5 CTRaw media.
  On 2026-10-10, `--game csb --platform amiga --boot-probe` against the
  supplied `Chaos Strikes Back for Amiga 3.5 CTRaw.7z` failed with `game
  unavailable for --game: csb`. The 7z contains four CAPS/SPS CT Raw disk
  images (English and English/French/German game and utility disks), rather
  than the ADF images accepted by the current Amiga file path. Repacking those
  four byte-identical original members into ZIP and retrying the same CLI probe
  produced the same failure; ZIP handles the container but not the disk-image
  format. The known-good A31 menu launch script, run against the original
  A35 archive with `FIRESTAFF_FAIL_IF_NO_LAUNCH=1`, also ended with `no launch
  reached before exit`. ReDMCSB COMPILE.H:274-298 defines distinct A35E/A35M
  executable and language-selection routes, so A31/A33 startup tests do not
  cover this edition. Implement bounded in-process access to the original
  media or another source-preserving intake path, then verify direct CLI,
  start-menu selection, A35E/A35M handoff and first runtime input against the
  authentic disks. Do not treat renamed or converted test images as proof of
  native CTRaw support, and keep emulator or BIOS dependencies out of runtime.
  The DMWeb software archive has since yielded a separate authentic A35M ADF
  member (tested through ZIP transport; see the verification note above), but
  it is not a decoded sector image from the CTRaw archive. A local search found
  no independently decoded A35 CTRaw sector image or usable CAPS reader, so
  the filesystem type is not yet established. The third-party
  `rsn8887/capsimg` source examined for format research carries a
  non-commercial restriction in its
  [license](https://github.com/rsn8887/capsimg/blob/master/LICENCE.txt); do not
  bundle or adapt it without a separate license review.
  A clean-room decoder-source audit on 2026-10-10 confirmed that no
  independently reusable CTRaw PACK implementation or complete PACK grammar
  was found. Greaseweazle's permissively licensed CAPS adapter calls
  `CAPSLockImage`, `CAPSLoadImage` and `CAPSLockTrack`; it does not decode the
  CT Raw payload itself. The independent disk-utilities frontend also relies
  on CAPS/SPS, and the published IPF document describes IPF records rather
  than CTRaw PACK records. The local ReDMCSB tree contains authentic A35E/A35M
  files (`APPB.FTL`, `VDEO.FTL`, `ANIM.FTL` and `KAOS.FTL`) and documents the
  distinct edition routes, but supplies no CTRaw decoder or PACK grammar.
  Preserve those files as future authentic cross-checks; their presence does
  not prove that a disk stream has been reconstructed. Existing
  `firestaff_caps_raw_archive_stream` coverage validates four original
  members' CAPS/DATA/TRCK/PACK framing and packed-payload CRCs only; it does
  not decode PACK payloads or recover sectors. Continue with a clean-room
  decoder only after obtaining a documented grammar or a legally usable
  decoder oracle. References: [Greaseweazle CAPS adapter](https://github.com/keirf/greaseweazle/blob/master/src/greaseweazle/image/caps.py),
  [Greaseweazle Unlicense](https://github.com/keirf/greaseweazle/blob/master/COPYING),
  [official SPS Decoder Library listing](https://kryoflux.com/?page=download),
  and [IPF v1.6 documentation](https://www.kryoflux.com/download/ipf_documentation_v1.6.pdf).
  A 2026-10-13 review of the public-domain `disk-utilities` README confirmed
  that its `disk-analyse` CTRaw reader must be built with `caps=y` and CAPS v5.
  This confirms a possible development-time oracle exists only when that
  external library is available; it does not provide a standalone PACK codec
  for Firestaff. Neither `disk-analyse` nor a CAPS image library was found in
  the current development environment. Do not add that dependency to the
  Firestaff runtime. See the [disk-utilities build requirements](https://github.com/keirf/disk-utilities#ipf--ct-raw-support-the-capssps-ipf-support-library).
  Any decoder acceptance run must compare reconstructed files against the
  existing original-media hashes in `data/asset_validator_checksums_m12.json`
  and `parity-evidence/csb_v1_l10n_source_catalog.md`.

- Restore the original French Atari ST preservation ZIP before revalidating
  `csb_v1_atari_french_preservation_zip_cli_boot`. The test currently skips:
  `/Users/bosse/.firestaff/data/csb/chaos_strikes_back_ftl_france.zip` is absent
  from the configured data directory. Do not treat the earlier French ZIP
  evidence as a current local-media run until the authentic archive is present.

- Verify the natural F31 closed-door melee path on the reported M5. Its
  C04 sound now schedules ReDMCSB's C20 event one tick later, and EN/JA
  original-media tests prove that an injected C20 event selects its sample
  at the due tick. The test does not exercise a naturally triggered melee
  action or audible device output.

- Capture a post-Entrance FM Towns dungeon frame on the reported M5 and
  compare an authentic ornament-bearing pose with its source material. The
  live F0128 path now binds MINI.DAT floor-ornament selectors and seed, and
  English/Japanese F31 tests verify that metadata transaction. Those checks
  do not establish pixel-level dungeon correctness or native input behavior.

- Reconcile the Atari ST SND1 decoder with ReDMCSB SOUND.C F0060/F0061 before
  claiming complete audio coverage. Against the supplied
  `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` hard-disk GRAPHICS.DAT, table
  rows 1, 12 and 16 load but the bounded F0060 decoder returns -2. A new
  preserved-media probe compared all 22 SND1 records in the hard-disk image
  and retail v2.1 STX floppy; sizes and hashes match for every record, including
  the three rejected rows (534: 39 bytes/100 samples, 544: 408/1019, 546:
  452/962). ReDMCSB SOUND.C advances through packed samples until the declared
  sample count ends without checking the source graphic boundary. Do not pad
  or fabricate those streams: resolving them needs an original Atari runtime
  memory-boundary/capture trace. The all-zero PSG-to-PCM fix is separately
  covered by a source-format edge test; that does not establish the three
  rejected sounds or audible parity.
  The separate Atari Timer-A preemption bug is fixed: authentic title cues
  at VBlanks 1107/1128 now replace queued PSG output. That transport proof
  does not resolve these three rejected source streams.

- Complete the remaining C37 wander bridge after its shared-RNG admission was
  corrected. It now consumes persistent `G0349` exactly once for the
  `M005_RANDOM(2)` movement gate and, when admitted, once for absolute
  `M004_RANDOM(4)` direction; its bounded four-direction scan also consumes
  the prior-square one-in-four `M004` gate only when applicable. The remaining
  C37 now uses F0202's creature-owned open-pit/levitation and imaginary
  fakewall gates plus its non-material door exception. The F0202
  archenemy-Fluxcage gate and the C31 party-bump attack transition are
  covered by raw-format regression tests. F0267 now also detaches a C04
  before C60/C61 owns a blocked destination and performs its later placement
  from the detached record, matching TIMELINE.C's
  `CM1_MAPX_NOT_ON_A_SQUARE` route. C29 and C30's bounded reaction branches
  are covered by raw-format regression tests. Remaining work is authenticated
  platform trace evidence for those paths and full F0200 perception input.
  Do not reintroduce map/tick-derived reseeding. See
  `docs/parity/DM1_RAW_RNG_CONSUMER_AUDIT.md` for scan scope.

- Extend the source-profile C37 levitation/pit checks to original platform
  captures and exact RNG evidence. Audit fall-damage Defense=255 semantics
  and moving-group aftermath separately; C25 immunity is not proof of those
  paths. Verify levitation across chained teleporter/pit destinations.

- Extend chest verification to Atari/Amiga pointer geometry, chest switches,
  runtime-driven owner changes and native save boundaries. F31 EN/JP now
  verifies open-slot persistence, pickup and permitted replacement in both
  presentation modes, with synchronized runtime chains. Original oversized
  residents remain forbidden to reinsert; do not loosen G0038 slot masks to
  make such placements appear supported.

- Verify completed-event history for FM Towns distance-volume production.
  The real F31 archive now verifies all 35 source PCM events, the native
  5500 Hz transport, and every 1..127 driver-volume step; direct local events
  use 127. Two different authentic sounds now survive one tick in the F31
  pending queue for English and Japanese. The runtime request branch implements MEDIA551's
  division-before-multiplication distance formula. What remains is an
  end-to-end completed-event receipt at a non-local distance, followed by an
  original capture comparison and physical speaker check.

- Add authentic inscription-bearing Atari, Amiga and F31E runtime fixtures or
  captures and assert the candidate framebuffer pixel delta for the now-wired
  C02/M648/F0635 paths, including the now-wired distant/side zones. Obtain an
  allowed authentic F31J glyph-raster strategy: its exact F0168 second decode
  and F0646 substring route are wired, but F0644 pixels remain fail-closed
  because the retail CD delegates them to the FM Towns EGB system font and
  contains no font ROM. English M648 is never borrowed. Original-capture pixel
  parity remains unclaimed.

- Obtain checksum-verified DSA-bearing saves and per-edition save corpora;
  use them to extend native gameplay, timer and transaction coverage.
- Add original-data HUD, viewport, title, door and audio capture comparisons
  for Atari ST, Amiga and FM Towns. Do not use CSBWin as a PC game route.
  The native Atari ST runtime viewport in `src/engine/m11_game_view.c`
  (`m11_csb_present_atari_st_runtime_viewport`) draws the original ceiling,
  floor, stone walls, pits, stairs and doors, but still lacks general visible
  square thing-chain traversal and most object, creature, projectile and
  explosion rendering. ReDMCSB `DUNVIEW.C` F0115 draws these in the D3
  through D0 cells (for example lines 6816, 7368, 7937 and 8294). The
  original Atari M12-to-M11 startup route only checks the prison map's
  nonzero static viewport;
  that map contains no object or creature things, so its viewport hash cannot
  verify this missing pass. The available M11 sprite callbacks use PC3.4
  F0115 source zones and DM1 placement geometry. Bind the Atari-native
  graphics indices and placement tables before reusing any of them; then use
  an authentic thing-bearing pose and pixel-level assertions. Do not draw
  PC3.4 sprites as an Atari fallback. The original Atari Utility STX
  `MINI.DAT` contains 11 maps and offers a controlled candidate: map 6,
  party (21,17) facing south sees floor square (21,18), whose first thing is
  DB5 weapon `0x1423` (record `fe ff a7 00`, subtype 39). ReDMCSB
  `DUNGEON.C` G0237 row 62 maps that subtype to ObjectAspectIndex 11;
  the Atari MEDIA009 `DUNVIEW.C` G0209 row 11 has
  FirstNativeBitmapRelativeIndex 12. Atari `DEFS.H` sets M612 to 360, so the
  source item is 372. It decodes from the original Atari Game STX
  `GRAPHICS.DAT` as a 64x4 indexed surface through the existing Atari loader.
  Entry 510, used by the PC3.4 mapping, also decodes but belongs to a
  different Atari graphic category and must not be used for this weapon.
  The Atari presenter now uses G0237/G0209 data for all 46 DB5 weapon
  subtypes in the bounded open D1C front/back cells and D0C back cell,
  first matching DB5 per visible cell, using source G0218 coordinates and G0217/G0223
  first-item offsets. F0159 traversal now reaches DB5 records behind other
  things; authentic MINI map 3 (14,11) has a DB3/DB5/DB5 chain verified in
  both D1C cells by the original-media test. D1C executes within the F1 wall-command pass; D0C
  executes after the nearer side-wall passes, matching F0115 ordering.
  All 27 distinct Atari graphics selected by those subtypes decoded from
  the original Game STX at the source dimensions. The original Atari M12-to-M11
  test resumes authentic MINI data in diagnostic D1C and D0C poses, checks
  graphic-372 pixels on screen, and confirms they disappear when F0267 unlinks the real
  weapon from the decoded source square in test memory. A missing or
  malformed admitted graphic rejects the Atari viewport frame instead of
  silently omitting that weapon. The D1C front pass follows Atari F0129's
  fixed-point D2 scaling and G0214 palette; a second genuine MINI map-9
  pose compares all 78 opaque scaled pixels with original graphic 372 and
  checks their disappearance after F0267. No mapped DB5 weapon uses Atari's
  native flip flag. Far view cells, complete mixed thing chains, piles, DB6–DB10 objects, creatures,
  projectiles and explosions still need their native draw passes and
  real-media receipts; they remain invisible in the Atari viewport.
  Amiga RGB4 register expansion is now compared and fixed from an authentic
  FS-UAE/Kickstart 1.3 title route; retain as open only a same-source-VBlank
  image pair plus the remaining HUD/viewport/door/audio comparisons. A
  genuine TOS 1.62 capture boot reaches the protected Atari retail Entrance;
  the ordinary M12-to-M11 menu route now reaches the live Atari dungeon
  runtime after the authentic `ANIMATE.SCR` sequence and FTLCODE handoff. The
  tested source start is map 0 at (9,0), facing south, with zero champions.
  ReDMCSB LOADSAVE.C F0435 initializes a new CSB game with an empty party;
  CLIKMENU.C F0366 documents that empty-party movement is valid on the original
  maps containing champion mirrors and no creature types. An authentic Atari
  M12-to-M11 test drives production input from the untouched spawn to C127
  ordinal 4, selects its C026 portrait, and confirms it through source C160.
  The supplied Atari save-disk MSA has no root files, so there is no authentic
  campaign save available to substitute. This verifies title/menu-to-runtime,
  two consecutive C127/C160 champion recruitments, and movement after the
  second confirmation, not sustained campaign play or capture. The second
  source-authentic route approaches the C127 wall at (12,8) from the open
  corridor at (11,8).
  The real-media CLI route now also sends source F1 and F2 inventory keys
  after the second recruitment and verifies an open inventory panel. A
  separate authentic M12-to-M11 receipt verifies the selected inventory
  ordinal and unchanged leader. Further item handling remains open: both
  recruited champions have empty inventories, and a read-only census of the
  original 20x16 prison map 0 found no object or teleporter things. ReDMCSB
  PANEL.C F0355 closes an open chest when switching inventories; verify that
  transition only after reaching an authentic chest beyond the prison map.
  Legacy loose-file capture tests that labeled CSB as PC3.4 have been removed;
  replacements must name and authenticate one of the three supported retail
  platforms. On 2026-10-01, `csb_v1_atari_original_archive_cli_boot` was added
  and passed against the original Atari ST preservation archive
  `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` (authenticated asset MD5
  `ebf6a57af3f27782e358c0490bfd2f2e`). It covers the `ANIMATE.SCR` title,
  direct CLI runtime movement, C127/C160 recruitment through ordinal 4, then
  a source-verified corridor route to ordinal 5, its C160 confirmation, and
  resumed movement to (11,7), with a nonzero viewport receipt. Host loop-frame
  `wait:32` steps let movement cooldowns expire; these waits are not original
  Atari VBlanks. Sustained campaign play remains open.
  Atari capture/audio comparisons remain open. The separate nested
  ZIP test still skips when its nested ZIP package is not installed.
- Extend source-text extraction beyond the now-bound object/action names and
  reviewed Utility Disk DB2 instruction. Atari ST now
  supplies M564 from item 556 and the G0490 subtable from C560 item 560 to
  both the live runtime and catalog generator;
  FM Towns also binds English/Japanese M564 and DYNA_BUTTONS to its live
  runtime directly from the packed CD. Amiga A31M binds M564/C699 from its
  selected big-endian DMCSB2 media, while A31E/A35E bind compiled G0490 only
  from the hash-locked APPB on the same admitted disk. The live selected
  dungeon now promotes the reviewed utility instruction to the CSB POT while
  rejecting structured champion records and their encoded statistics. The
  native Eye path now walks a real C07 scroll to its platform-specific C02
  reference and authentic text pool without consulting DM1 `world.things`.
  Continue walking square-chain relations at their native presentation
  consumers before promoting further messages and inscriptions.
  Add executable-owned dialogs and Hint Oracle text the same way; never restore
  invented semantic keys or translate encoded champion-stat payloads.
  The final main-draw message-area pass now preserves the live F0168-decoded
  C02/DSA receipt and cannot overwrite it with M11 telemetry; extend the same
  source ownership to the remaining inscription consumers.
  Its persistence is also source-timed: supported Atari/Amiga/FM Towns rows
  remain visible for 70 game ticks and disappear at the F0046 deadline.
- Keep V2.2 presentation closed until a real source-owned material/pixel
  binding exists.
## Deferred original-data corpus

DSA-bearing saves and visual/audio captures are deferred while native Atari,
Amiga and FM Towns media paths continue to be improved from the supplied
original packages. The external FM Towns corpus contains an authenticated,
language-matched F31J `CSBGAME-JP.DAT`, which passes the full native reader and
now restores into M11 through both direct CLI launch and M12 Quick Resume.
The authentic-media FM Towns regression passes with that corpus supplied and
asserts the restored map, party pose, direction and champion count in both
direct launch and the live M12-to-M11 runtime route. The local CTest registers
the corpus automatically when the authenticated save is present.
Its English `CSBGAME.DAT` candidate is incoherent and remains rejected. The
Atari ST preservation archive does include
`Floppy Disks MSA/Chaos Strikes Back for Atari ST Save Disk.msa`, but that
authentic 720 KiB MSA decodes to a formatted disk with zero root files; its
SHA-256 is `bca3db90f795c633fcb0cc7a10a4811dae616b7d8e7eb8b65b4f59af10598d29`.
The real-media `csb_v1_atari_msa` decoder regression confirms it contains no
campaign save. No Atari/Amiga DSA-bearing campaign save or CSBWin extended
save is currently available. A real Atari TOS 1.62 + protected
campaign-disk run reaches the `PRISON / RESUME` Entrance but cannot establish
a champion-bearing dungeon state without that missing source material. Later
work must record media hashes and provenance before using newly supplied
captures; it must never generate a replacement corpus.

The ordinary M12 Quick Resume path now carries fully validated CSB saves even
when the optional DSA-corpus identity is absent. Wire the separate
`M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery` receipt into the real
CSBWin DSA-bearing save path before claiming DSA resume parity; only the unit
test currently consumes that discovery receipt.

The separately obtained CSBWin compatibility corpus is also not an admissible
substitute: a private non-sanitized CSBWin reference build reaches its
graphics-table decoder but resolves an impossible graphic identifier and then
fails. It therefore cannot be used to manufacture a DSA save, a platform
capture, or any parity claim. Retest only when a corpus is supplied with
edition and source-media provenance that binds its `dungeon.dat`,
`graphics.dat`, and `CSBgraphics.dat` together.
# CSB C30 projectile-hit reaction uses the raw C04 group and source-shaped
# F0199 route. ReDMCSB GROUP.C F0209:2024-2044 first ignores attacking/fleeing
# groups, makes a 3/4 M004 gate, turns without moving if the party is not on
# the unblocked route, and otherwise makes a second 3/4 gate before falling
# through to C29's danger-movement route. The native tick regression now
# covers attack-ignore, blocked-route direction-only, and visible-party C29
# fallthrough branches, including their exact G0349 draw counts. Remaining work
# is full F0200,
# rather than another generic visibility predicate: it must bind each real
# creature's direction(s), sight range, invisibility capability, palette-light
# adjustment, and source-owned range RNG before C30/C32--C37 can claim full
# perception parity. C30 now applies F0200's per-creature facing,
# side-attack, C71 invisibility, complete source range word, source RNG
# jitter, and F0337/G0304 palette-row reduction before F0199. The palette
# row is published directly by the original renderer path, never inferred
# from host brightness. Remaining work is authenticated platform trace
# evidence for this complete consumer.
