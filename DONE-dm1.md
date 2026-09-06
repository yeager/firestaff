# Firestaff DONE — DM1

## 2026-09-06 — Non-vacuous scroll font check

- The real-scroll raster test now requires a loaded original font and a
  nonzero ink-pixel count for every tested scroll. Original/V2.1 and the
  complete original-object corpus pass with these guards.

## 2026-09-06 — Original scroll text raster placement

- The PC3.4 scroll eye corpus reconstructs the C023 panel and scroll-font
  ink with a separate character-remapping and pixel-placement loop. Its
  panel-area comparisons pass in Original and V2.1 alongside the full
  object corpus.
- Text layout and M653 pixel decoding remain shared with production. The
  comparison preserves actual pixels behind transparency, so it does not
  independently prove those pixels, font decoding or full emulator parity.

## 2026-09-06 — Original scroll panel border

- Every PC3.4 original scroll eye transaction now compares the visible
  two-pixel C023 outer border with the loaded original panel at its native
  destination. Original and V2.1 pass with the full object corpus.
- This is border/material evidence only: transparent pixels and the inner
  text area are excluded, and glyph parity remains open.

## 2026-09-06 — Original scroll text ownership

- Every original PC3.4 scroll tested through the eye now requires M11's
  decoded source text to match that scroll's own C02 reference with F0341's
  SCROLL/DECODE_EVEN_IF_INVISIBLE flags. Original and V2.1 pass with the
  full original-object corpus.
- This verifies owner/reference selection using the shared text decoder;
  it does not independently validate decompression, localization or glyph
  pixel parity.

## 2026-09-05 — Three-mode legacy launch matrix

- Added V2.0 to the existing Original/V2.1 original-media CLI/menu matrix.
  All 36 launches pass across Atari EN/DE/FR, Amiga EN and FM Towns EN/JP,
  retaining the requested presentation and reaching a loaded dungeon.
- Japanese FM Towns also asserts its own program/graphics fingerprints.
  These checks do not establish full gameplay, animation timing or pixel
  parity, and do not cover every historical edition of these platforms.

## 2026-09-05 — Japanese title media failure classification

- Missing archives skip explicitly, but unreadable existing archives or
  archives without a readable BIN now fail the JDM title test. Verified
  original media passes, an existing non-archive returns 1, and a missing
  path returns 77. No new game data or extracted files were created.

## 2026-09-05 — FM Towns launch and Japanese title evidence

- Rebuilt Firestaff passes the original ZIP CLI/menu English and Japanese
  launch/input matrix, including selected program/graphics hashes and CDDA
  track status. This is not an emulator timing or full-frame comparison.
- The original JDM title-receipt test passes. Its assertions now stay active
  under NDEBUG, and unavailable media returns CTest's skip code 77 instead
  of a false pass. CMake recognizes that skip explicitly.

## 2026-09-05 — Original chest owner transitions

- The original PC3.4 corpus now refreshes the same open chest after each
  pickup, requiring holes to survive, and switches to another chest while
  holding the first resident. It verifies the held Thing, old owner's full
  remaining chain and unchanged next owner's head after close.
- Original and V2.1 pass together with the complete original-object corpus.
  Test placements are restored in memory; save/resume remains out of scope.

- Bound the stripped Japanese FM Towns `JDM.EXP` title path to its unique
  disassembly fingerprints at `0xc428`, recovered rectangle/data owners, and
  exact C12/C13/C14 RGB6 records. The native title presenter now accepts the
  Japanese title only through the real in-memory ZIP/BIN/ISO receipt; it has
  no default/VGA palette fallback. See
  `parity-evidence/dm1_fmtowns_jdm_title_palette_binding.md`.

- Fixed the native English FM Towns title palette handoff. The authenticated
  EDM.EXP P3 load image now supplies the exact C12 PRESENTS and combined C13
  DUNGEON/C14 MASTER RGB6 transactions used at their original title-frame
  boundaries; stale host/PC palettes are no longer allowed. See
  `parity-evidence/dm1_v1_fmtowns_title_palette_transactions.md`.

## 2026-09-04 — F0125–F0127 targeted D0 transactions

- Replaced the class-wide terminal D0 callback pass with three explicitly
  targeted scheduler transactions in ReDMCSB order: D0L, D0R, then D0C.
  Each side square now completes its F0104/F0115/F0113 tail before the next
  square begins; D0C retains its split around F0115 and the final F0113 field.
- The implementation continues to use the mounted PC 3.4 GRAPHICS.DAT
  consumers. No generated bitmap, extracted runtime asset, or second replay
  was introduced.

## 2026-09-04 — Removed broad post-scheduler stairs ownership

- Deleted the remaining broad D3--D1 stairs fallback after square replay.
  The authenticated-plan failure path already returns before viewport
  background composition, so that branch was unreachable; retaining it gave
  F0104 stairs a second, source-invalid ownership path.
- D3--D1 stairs are now exclusively consumed from each owning square's
  authenticated foreground callback using mounted retail GRAPHICS.DAT data.

## 2026-09-04 — F0128-owned F0104 door frames

- Split native centre and side door-frame material out of F0111 into the
  scheduler's explicit F0104 door-frame callback phase. The live order is now
  F0108, DOORPASS1, frame, optional F0110, F0111 panel, DOORPASS2.
- F0111 no longer redraws frame pixels. Open doors retain their frames, and
  the same source plan covers normal side, centre, and D3L2/D3R2 routes. The
  real PC 3.4 HoC probe requires separate frame and panel receipts.

- Modern temporal post-processing now applies movement blur, phosphor
  persistence, and the next-frame history snapshot in one full-resolution
  traversal. The operation order and idle-frame history semantics are
  unchanged, while the all-effects path no longer performs three separate
  high-resolution memory passes per presented frame. Original mode never
  enters this optional filter path.

## 2026-09-04 — F0128-owned F0110 door-button routes

- Added an explicit F0110 scheduler step only to the four ReDMCSB call sites:
  F0117 D3R and the centre F0118/F0121/F0124 routes. The callback consumes it
  between the plan's frame and F0111 steps; D3L, D2L/D2R and D1L/D1R remain
  source-authentic no-button-call routes.
- Deleted the direct D3R and centre button replay. The real PC 3.4 HoC probe
  finds retail map 0 door `(1,2)` from party `(0,2)` facing east and proves
  one callback-owned F0108, DOORPASS1, F0110 and F0111 step in that order.

## 2026-09-04 — Door-front F0108 source boundary

- Split door-front floor ornaments out of the generic post-door foreground
  phase. Their authenticated scheduler step now rasterizes before
  `DOORPASS1`, followed by the source frame/F0111/DOORPASS2 transaction.
  Corridor, pit and stair F0108 steps remain in their ordinary plan-ordered
  foreground path; no primitive-class batch or second ornament draw exists.
- The real PC 3.4 HoC test finds an unmodified retail door and requires one
  pre-door F0108 callback receipt before accepting its DOORPASS1 and F0111
  receipts. F0113 remains owned by the same square after its final F0115
  consumer.

## 2026-09-04 — F0128-owned F0107 ornament projections

- Moved the authenticated 13-row D3--D1 wall-ornament family into the
  per-square callback. D3/D2 side routes consume their side-facing F0107
  projection followed by their front-facing projection; centre and D1 routes
  consume their sole projection. The three broad hand-written ornament
  replay calls are gone.
- Corrected the scheduler's F0107 cardinality from an unconditional pair to
  the exact ReDMCSB function bodies: two calls only for D3/D2 normal side
  squares, one for outer, centre, and D1 walls. The real PC 3.4 HoC gate now
  requires callback-owned F0107 material alongside wall, D0, and foreground
  work. F0676/F0677 outer walls and F0122/F0123 D1 side walls now also ignore
  the F0107 return exactly as source does, preventing a false alcove Thing
  pass on routes that immediately return. MEDIA720 D3L2/D3R2 now use the
  separately authenticated item-696 C1004 layout anchors; they do not index
  or infer nonexistent rows from the legacy 13-row G0205 source table.

## 2026-09-04 — F0128-owned F0111 door transactions

- Moved every D3--D1 F0111 door transaction from the hand-written viewport
  replay into the authenticated per-square scheduler callback. Center, side,
  and exceptional D3L2/D3R2 routes now consume their native frame, panel,
  ornament, mask, flip and opening-state material only when the owning plan
  span emits `F0111_DOOR`; no duplicate F0111 raster path remains.
- Preserved F0110 exactly where ReDMCSB places it before F0111, including the
  exceptional D3R button; it is now owned by its explicit scheduler step.
  The real PC 3.4 HoC runtime test searches the
  unmodified mounted dungeon for a door and now requires both `DOORPASS1` and
  F0111 callback receipts from that original square.

## 2026-09-04 — PC 3.4 SND3 event-table source closure

- Replaced the guessed priority, distance and period fields in the 35-entry
  DM1 sound table with the exact ReDMCSB I34E `DATA.C` MEDIA719/MEDIA712
  rows. This includes the distinct period for entrance-door event 3 and the
  source periods for party damage, War Cry, Blow Horn and movement sounds.
- The real-media gate reads `DATA/GRAPHICS.DAT` directly inside the supplied
  PC 3.4 ZIP, proves every event maps to its source `SND3` item, and decodes
  all 35 event aliases from the authentic 33-sample bank at 6000 Hz. Invalid
  host emission indices now fail closed instead of entering the audio route.
  No game-data member is extracted and no procedural sound is accepted as
  proof.

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native direct-CLI and start-menu matrix against the staged
  original media. PC DOS 3.4; DOS English and French; Atari ST English,
  German, French and nested archive routes; Amiga HD and v2.0; and FM Towns
  all reached their intended native handoff in memory. The French RAR 2.0
  package retained its explicit unsupported-format diagnostic rather than a
  false missing-media result.

## 2026-08-30 — Amiga v2.0 original save-disk provenance

- The startup menu's Continue entry and direct CLI `--save` now accept the
  authenticated `ZIP → ZIP → ADF → DMGAMEG.DAT` path. M12 validates the
  actual Amiga F0435 envelope in RAM, and M11 routes the virtual path straight
  to the format-specific loader instead of attempting `fopen`. The direct
  Amiga boot probe resumes the real save at tick 292 with no media extraction.
- Added an in-memory receipt for the ordinary save disk retained inside the
  supplied Amiga v2.0 preservation ZIP. The selected ZIP → ZIP → ADF is
  SHA-256 `5679f789655ba3f53f6275137fc80f59eb798b03b88f801e260aed352b6709c9`
  and contains the original `DMGAMEG.DAT` and `DMGAMEG.BAK`, each 49,002
  bytes; no member is written to disk.
- Its primary header verifies as a ReDMCSB-compatible original save family
  (format 5, platform 3, dungeon 10) with a valid header checksum. This
  proves real saved-session material is present, while correctly keeping
  framebuffer/Copper-palette evidence capture-gated.

## 2026-08-30 — Native Amiga RGB4 palette producer

- Recovered the exact dynamic producer from the supplied English Amiga v2.0
  `dm` program without extracting the ADF: the real executable has one
  producer at `0x14306`, copies the caller source table to its work table,
  changes each RGB4 component by one or two, and invokes the Copper-list
  builder through the original `0x14434 → 0x14140` call.
- Added a native, source-gated eight-frame implementation. It accepts only
  verified caller-owned 16-word Amiga RGB4 tables, never invents a palette,
  and is regression checked against the original executable's in-memory
  control flow.

Reviewed 2026-08-29. Completed work only.

- The supplied Amiga 2.0 ZIP→ADF `graphics.dat` receipt now decodes an
  authenticated big-endian IMG1 record into original 4-bit palette indices
  in memory. The Amiga wrapper selects the legacy decoder's big-endian path
  explicitly, preventing an FM Towns/PC byte-order fallback.
- With that original ZIP→ZIP→ADF source present, the Amiga graphics test no
  longer builds a structurally valid replacement `GRAPHICS.DAT`. Positive
  decode and format coverage comes only from the authenticated ADF member;
  compact malformed-header checks remain solely as rejection boundaries.
- The supplied FM Towns ZIP now has a RAM-only CDDA payload receipt. It
  follows the source CUE's first audio index from the MODE1/2048 data region
  into the shared raw-audio BIN, validates every track interval, observes
  PCM in music tracks, and proves documented track 20 silence.
- The supplied FM Towns ZIP also supplies the TMenu input receipt directly:
  Firestaff follows ZIP→CUE→BIN→ISO9660 to `TMENU.EXP` in RAM and validates
  the Phar Lap header plus original poll, initialization, and TBIOS entry
  bytes used by the native input schema.
- The supplied DM1 Amiga 2.0 preservation chain now has a direct real-media
  graphics-format receipt. `test_dm1_v1_amiga_graphics_dat` reads the
  selected ZIP → ZIP → ADF `graphics.dat` member in RAM through the native
  AmigaDOS OFS reader, validates the actual 575-entry Amiga layout, and
  identifies it as the known English 2.0 format. It never copies game data
  to disk. Rendering/pixel comparison remains separate active work.
- The real Amiga ZIP → ZIP → ADF and Atari ZIP → ZIP → STX start-menu paths
  now publish their admitted source decoder (`IMG2` and `DMCSB1` respectively)
  only after the original graphics and dungeon pair have both bound.
- The authentic PC DOS 3.4 ZIP start-menu path likewise publishes its admitted
  `IMG3` source decoder only after its original graphics and dungeon pair bind.
- The supplied DOS-EN archive's nested lowercase `dungeon-master/dmaster/DATA`
  layout is now covered independently.  It binds the same authenticated PC
  3.4 graphics/dungeon pair and reaches native CLI, menu, and movement without
  unpacking the original archive.
- The manually unpacked authentic French PC DOS `EUDATA` route now receives the
  same direct start-menu `IMG3` handoff check; its unsupported RAR 2.0 wrapper
  remains a separate diagnostic boundary.
- The authentic German Atari ST 1.2 and French Atari ST 1.3 packages each now
  require their direct start-menu `DMCSB1` handoff before their existing native
  movement assertions run.
- The Amiga 2.0 preservation package now has its own real-media CTest. It
  verifies the exact ZIP → original ZIP → ADF selection, the `IMG2` handoff,
  and a post-menu native movement result rather than relying on the separate
  HD package's coverage.
- The supplied Atari preservation collection now selects only its `[!]`
  original member (`ZIP → ZIP → STX`) and reads its `GRAPHICS.DAT` and the
  release-specific `DUNGEON.DAT` identity entirely in RAM. Direct CLI, the
  startup menu and a native movement probe are covered by real-media CTests;
  cracked sibling images are never admitted.

- PC DOS 3.4 authentic archive startup reaches native DM1 runtime with a
  hash-verified real-media regression.
- M12 resolves the authenticated PC 3.4 data owner and production retains
  fail-closed behavior when required source data is absent.
- DM1 Amiga 2.0 English supplied as ZIP → ZIP → ADF is hash-verified and
  read entirely in RAM. Both direct CLI launch and the startup-menu route
  reach the bounded native runtime without extracting game data to disk.
- The supplied Amiga HD package (preservation ZIP → original HD ZIP → ADF)
  is admitted using the same authenticated Amiga 2.0 graphics/dungeon pair
  and reaches native runtime through direct CLI and the startup menu entirely
  in memory.
- Atari ST original media is source-locked and read in RAM: the supplied
  English 1.0a/1.2, German 1.2 and French 1.3 variants reach native runtime
  through direct CLI and start menu. The German 1.2 protected STX uses the
  verified image identity `0eff1c902ea155f19e4a177bb2ccac7d`, graphics hash
  `2bdc5f431f84c0ece738f54dbd787c3b` and dungeon hash
  `cea11d6e9f7e1698fc95329fe3fb0899`.
- The supplied FM Towns JA/EN archive is verified through both direct CLI
  and start menu, reaching `dm1-runtime` without media extraction.  The
  boot receipt requires the source-bound `TMENU.INF` selection and selected
  `EDM.EXP`/`JDM.EXP` MD5, rather than promoting generic DM1 movement as a
  native FM Towns handoff.
- The same original FM Towns ZIP now verifies the complete public input
  matrix from independent native sessions: forward, backward, turn left,
  turn right, both strafes, and action.  Every check follows
  ZIP → CUE → BIN → ISO9660 → TMENU → `EDM.EXP` in memory and asserts the
  observed initial runtime position, level ownership, and CDDA title track.
- The DM1 V2 movement/viewport regression now reads the canonical PC 3.4
  `DATA/DUNGEON.DAT` directly from its original ZIP in RAM. It no longer
  depends on an extracted corpus for its positive dungeon decode path, and
  the verified raw-map composition is used as the real-data evidence.
- The supplied Amiga HD preservation ZIP now verifies the complete first
  input matrix from independent original ZIP → ZIP → ADF sessions. Forward,
  backward, both turns, both strafes and action all retain the native runtime
  and observed source party state without a generated save or map.
- The English Atari ST v1.2 preservation chain now verifies the same complete
  first input matrix through fresh native ZIP → ZIP → STX sessions. The
  expected party states are observed from original media; no disk image or
  dungeon replacement is synthesized.
- The canonical PC DOS 3.4 ZIP now also verifies the complete initial input
  matrix through fresh native archive sessions. Its authenticated IMG3 title,
  dungeon and party state are retained in memory for every direction, strafe
  and action check; no extracted fixture or generated save is used.
- The complete PC34 `F0381_MENUS_PrintMessageAfterReplacements` producer set is
  now closed. ReDMCSB `MENU.C` has exactly two calls for `C005_ACTION_FLIP`;
  the bounded F0407 plan owns their exact HEADS/TAILS source strings, rejects
  values outside `M005_RANDOM(2)`, and passes the selected msgid through the
  DM1 translation domain into the cyan C015 message area with the original
  70-tick lifetime. Both outcomes, Swedish translations, the live M11 action
  tail, and the original PC 3.4 in-memory media identity are regression-gated.
- Original-mode C015 presentation is Unicode-capable without replacing its
  preservation font. ASCII cells still come from hash-identified PC34 M653;
  translated Latin glyphs are decoded once per UTF-8 codepoint into the same
  six-pixel TEXT.C advance. A real in-memory PC 3.4 ZIP regression combines
  the retail GRAPHICS.DAT/M653 owner with Swedish U+00C5 and proves its exact
  raster and single-cell placement on the bottom C015 row.
- Original-mode wall inscriptions now have the same bounded final-presentation
  boundary. F0168's packed TextString span, decoded glyph bytes, hashes and
  M648 raster receipt remain untouched and are always the fallback. A
  media-and-index-qualified gettext key may replace only the final readable
  text; supported UTF-8 Latin glyphs occupy centered 8x8 cells while ASCII
  continues to come from retail M648, using M648's own opaque palette index.
  The canonical PC 3.4 ZIP test visits all 56 visible inscriptions, then proves
  TextString 4 `HALL OF / CHAMPIONS` as Swedish `HJÄLTARNAS / SAL`, including
  the exact U+00C4 raster and retained original material receipt.
- The DM1 V2 side-by-side viewport comparison now accepts the decoded
  original PC 3.4 dungeon state and renders its actual initial composition
  into both lanes. The real-data viewport regression no longer promotes the
  old hard-coded entry fixture as positive composition evidence.
- `dm1_v2_side_by_side_seed_pc34` now uses that same native in-memory ZIP
  path for every positive lane, layout, RGBA export, and region comparison.
  It skips when the optional local PC 3.4 archive is absent instead of
  silently falling back to a hard-coded dungeon fixture.
- `dm1_v2_v1_v2_side_by_side_seed_pc34` now independently reads the original
  PC 3.4 ZIP member in RAM and verifies V1/V2 disabled-presentation framebuffer
  parity at the authentic entry coordinates in all four directions. It no
  longer uses the legacy entry-state fixture for positive pixel evidence. The
  fixture implementation and its data-free V2 screenshot probes were removed,
  so this family has no synthetic-dungeon fallback.
- The supplied authentic French DOS `DMSAVE.DAT` and `DMSAVE.BAK` (48,561
  bytes each; SHA-256 `494d081ee5175b2dccc900d5ea3f25230c8bb3b0f20828d311b8fc5bdfb82d21`
  and `a760234408bf27946b1586ecf396be72e648bd8f3d18abee90a18c2c7e94421f`)
  now pass a native backed-save roundtrip. Each source save is loaded through
  F0435 against its supplied original French `EUDATA`, staged through F0433,
  exported, reloaded through F0435, and checked for party, C03/C04 timeline,
  active-group and dungeon ownership preservation. No save fixture is used.
- DM1 PC 3.4 F0446 endgame cadence now follows `ENDGAME.C`: every ordered
  victory-message F0445 redraw owns its immediately following 780-tick wait,
  and the final 600-tick wait runs only after the last message. These waits
  freeze gameplay time. Victory music fails closed unless the selected
  installation's authentic `SONG.DAT` is admitted, and the victory path keeps
  restart disabled exactly as F0446 requires. A focused runtime test locks the
  event/delay order and the 780 + 780 + 600 total.
- The authenticated PC 3.4 F0444 victory presentation now has distinct
  source-ordered phases: champion mirrors/portraits wait for input, accepted
  input starts authentic SONG.DAT C3 and C006 `THE END`, the 300-tick hold
  freezes gameplay time, and the victory route then presents authentic C005
  credits. All phases own the full logical screen, and missing F0444 material
  or SONG.DAT fails closed instead of selecting synthetic presentation.
- The separate PC 3.4 F0444 restart request no longer escapes to a generic
  host relaunch. It invokes the native F0435-compatible quick-load consumer
  in place: a valid admitted save clears the terminal/death state and resumes
  the restored world, while a failed load clears the request and returns to
  C3/C006 `THE END` for the source 300-tick hold using retained authenticated
  endgame material. If that material is unavailable the failure path stops
  closed. Focused decisions cover load success, load failure/re-entry, and
  missing-material rejection.
- M11 DM1 status/readout localization now has an explicit extraction
  boundary. Six live pickup/drop/spell literals are marked as DM1-owned,
  included in the 578-entry template, and translated into Swedish. The CI
  gate checks catalog presence, nonblank Swedish coverage, and active-game
  domain dispatch so shared CSB/DM2/Nexus/Theron literals cannot inflate the
  DM1 completion number.
- The next explicit M11 localization group covers the source-locked DM1
  CLIKVIEW.C fountain path: `FOUNTAIN`, `DRANK WATER`, `CONTAINER FILLED`,
  and `FLASK FILLED WITH WATER`. All four runtime literals are marker-gated,
  Swedish-translated, and included in the now 581-entry DM1 template without
  importing shared-file strings owned only by another game.
- The shared required-file catalog retains the verified Amiga 2.0
  `DUNGEON.DAT` identity alongside the Atari additions.  The post-scan
  recovery now republishes a selected ZIP → ZIP → ADF source owner before
  calculating availability, so an Atari preservation scan cannot make a
  valid Amiga launch unavailable.
## DM1 PC 3.4 MEDIA720 outer-D3 wall ornaments

- Recovered all sixteen authentic D3L2/D3R2 layout records directly from
  real `GRAPHICS.DAT` item 696 and wired them into F0128's per-square F0107
  callbacks. The native route preserves record alignment types, C30/C14
  scaling, D3 palette mapping, C10 transparency, and D3R2 horizontal flip.
- Ruled out item 558 rather than guessing from D3L/D3R: the admitted PC 3.4
  file has SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`;
  entry 558 starts at `0x25eed`, is 38 bytes compressed/decompressed, has a
  16x7 header, and hashes to
  `df28c5d26e9a7b87903ac817a21675c65ad52216e5deede4148f5286f4223e23`.
- Corrected Hall of Champions mirror admission for the source-owned outer
  D3 records. Internal view indices 13/14 now admit authentic C127 mirrors
  and consume base ornament C345 at the item-696 anchors; they are no longer
  suppressed as invalid aliases of G0205 rows 0/1. The real PC 3.4 archive
  regression covers all 15 matching mirror placements without fixtures.

## DM1 FM Towns Japanese native ZIP start

- Added an explicit `--dm1-fmtowns-ja` selection backed by the canonical
  FM Towns retail disc identities. The native in-memory route now selects
  TMENU -> `JDM.EXP`, reports its exact executable and Japanese GRAPHICS MD5,
  applies the recovered JDM title/palette owners, and reaches the runtime.
- Added the source-specific F20J `JDATA/DUNGEON.DAT` reader. The authentic
  33,931-byte Japanese file ends exactly after raw map data and has no F0434
  checksum trailer; all ordinary dungeon and save readers remain strictly
  checksum-validating. Real-media tests prove both the rejection boundary
  and successful 14-map Japanese materialization without extracting to disk.
