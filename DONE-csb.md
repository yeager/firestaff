# Firestaff DONE — CSB

## 2026-09-05 — Atari inventory mouse release

- Fixed same-slot release undoing an Atari inventory pickup. The release
  guard now resolves Atari's native slot coordinates as well as F31's.
- Reproduced the failure and verified the fix in Original, V2.0 and V2.1
  with the original STX and utility MINI.DAT. The utility champion starts
  unequipped; the test temporarily places an existing dungeon weapon in a
  backpack slot and restores the empty slot afterward. No media is changed.
- This proves that pickup/replacement roundtrip, not every Atari item or
  chest slot. Full Atari/Amiga inventory coverage remains open.

## 2026-09-05 — F31 chest owner transitions

- Original EN/JP archive tests pass same-owner reopening with an empty slot
  and switching to another chest while holding the first chest's resident.
  They compare both original linked lists and the held Thing in Original
  and V2.1, using controlled in-memory placement of original records.
- The expected behavior follows ReDMCSB CHEST.C F0333, lines 30–75:
  retain same-owner slots, close the previous owner before loading another.
  These checks do not establish save/resume or pixel-level parity.

## 2026-09-05 — F31J F0168/F0646 inscription byte pipeline

- Implemented the distinct ReDMCSB F31J second decoding pass that restores the
  packed Shift-JIS stream from F0168's A..P representation, including literal
  prefix and terminal inscription-marker rules.
- Implemented bounded F0646 line selection with exact 16-pixel Shift-JIS,
  8-pixel ANK, zero-width control and explicit-break semantics. Truncated or
  malformed pairs fail closed.
- Verified the selected real FM Towns CD ZIP in both sessions: F31E exposes 41
  visible C02 strings with no high bytes; F31J exposes 46 with 557 high bytes.
  The selected CHTWE/CHTWJ dungeon is retained instead of borrowing English.
- Kept F0644 glyph rasterization closed. The retail CD calls the FM Towns EGB
  system font and contains no glyph ROM, so no game-media-only pixel-parity
  claim or M648 substitution was made.

- Bound FM Towns F31 C017 inventory drawing and pointer hit-testing to the
  selected retail `GRAPHICS.DAT` item 696. Both CDATA and CJDATA provide the
  same thirty C507..C536 children of the C105 16x16 record; boot retains the
  decoded same-session receipt and F31 fails closed rather than borrowing the
  PC/Atari table. Real EN/JA archive tests cover the receipt.

## 2026-09-05 — Side-aware F0172 unreadable inscriptions

- Added a source-owned F0172 wall-aspect receipt which selects C02 by the
  exact right/front/left F0107 view wall rather than by map square alone.
- Routed distant/side M615 through the existing wall-ornament blit and applied
  ReDMCSB G0190/G0204 one-to-three-line `0x4000` clipping semantics. D1C stays
  on the readable M648 transaction and unsupported or mismatched faces fail
  closed.
- Added focused face-selection and all-depth shift-table regressions; no
  synthetic runtime surface or post-render overlay was introduced.

## 2026-09-05 — F0373 levitating front-cell group parity

- Replaced the conservative all-group pickup rejection with the authentic
  ReDMCSB F0175 → F0144/F0264 → F0176 chain. A real C04 now blocks a
  front-square object only when its creature lacks the G0243 levitation bit
  and occupies the clicked cell; levitating groups no longer hide reachable
  floor objects (`parity-evidence/csb_v1_floor_pickup_f0373.md`).
- Party-map F0176 resolves C04 byte 5 as `ActiveGroupIndex` through the
  F0145/F0147 owner and reads effective Cells/Directions from that valid
  active slot; a nonzero-index regression prevents the former raw-byte bug.

## 2026-09-05 — Native C02 inscription decode ownership

- Visible wall TextStrings now decode from the selected CSB dungeon with the
  correct Atari/FM Towns and reversed Amiga bitfields. Invisible records fail
  closed and the C07 scroll offset path shares the corrected platform rule;
  no DM1 `world.things` text fallback is used
  (`parity-evidence/csb_v1_visible_wall_inscription_f0168.md`).

## 2026-09-05 — Platform-owned inscription material plan

- Locked Atari S20/S21/F20E to MEDIA020 M648 graphic 120 and authentic fixed
  G0203 geometry, and Amiga A31/A35 plus English FM Towns F31E to MEDIA720
  M648 graphic 258 and F0635 geometry. All admitted glyphs are authentic 8x8
  C10-transparent source material.
- FM Towns Japanese fails closed because ReDMCSB F0107 owns it through F0644
  and a different selected-media font pipeline. No English M648 or DM1 asset
  substitution is permitted.
- Added live F0172 publication from the native CSB front-wall Thing chain,
  including the retail BUG0_76 last-visible-C02 behavior. The Atari MEDIA020
  Original renderer now consumes that receipt and authentic graphic 120 in
  the candidate-page transaction, with CSB gettext applied only at the final
  presentation boundary and decoded retail English as fallback.
- Wired Amiga A31/A35 and FM Towns F31E to raw selected-container graphic 696,
  F0639 range parsing and strict F0635 C1000..C1003 anchors. Their Original
  front-wall draw now consumes selected M648 graphic 258 without PC/Atari
  rectangle or DM1 asset substitution.

## 2026-09-05 — Native F0349 water-potion mouth transaction

- Command 70 now applies the source-proven C15 water-potion branch atomically
  to the CSB runtime: water gain/cap, in-place C08 empty-flask transformation,
  statistics redraw, and leader-hand retention. Unsupported F0349 branches
  fail closed instead of touching the DM1 world mirror
  (`parity-evidence/csb_v1_mouth_water_potion_f0349.md`).
- The deterministic C09 food branch now uses the eight exact G0242 food
  amounts, caps Food at 2048, detaches and consumes the real Thing, clears the
  CSB leader hand, adjusts leader load, and requests the source swallow sound.
- C09 waterskins now use the exact subtype/icon/ChargeCount branch: +800 water
  capped at 2048, in-place charge decrement, charge-dependent load/icon
  update, retained leader hand, statistics redraw, and C08 swallow request.
- The remaining C08 potion family now implements F0348/F0349 stat, stamina,
  mana, health, wound-RNG, antivenin, and stacked YA timeline behavior. Every
  admitted potion becomes an empty flask with retained Power and corrected
  hand load; C72 expiry subtracts its own `B.Defense` rather than clearing all
  stacked shield defense.

## 2026-09-05 — F0375 left/right leader-hand throw cells

- Propagated F0375's explicit left/right side through F0329 into the native
  CSB projectile record. Left and right viewport halves now produce distinct
  source cells while restoring the action hand and clearing the leader hand
  (`parity-evidence/csb_v1_leader_hand_throw_side_f0375.md`).

## 2026-09-05 — Source-owned viewport floor pickup

- Routed C080 floor-pile clicks through the CSB-owned F0373 dungeon-chain
  mutation. Visible Atari/Amiga/FM Towns objects now move from the authentic
  square record into the leader hand instead of falling through the DM1-only
  world snapshot (`parity-evidence/csb_v1_floor_pickup_f0373.md`).

## 2026-09-05 — Live F0302 inventory transaction input

- Ordinary equipment and backpack clicks now refresh the CSB runtime-owned
  M516 party receipt before reading either possession. This closes the stale
  C017-panel path that could write an old mirrored Thing back over a current
  runtime slot; slot and leader-hand writes remain in the CSB runtime.

## 2026-09-05 — Native Eye/scroll text decoding

- C07 scrolls inspected through Eye now decode their platform-correct C02
  reference and authentic dungeon text pool in the CSB runtime. The final
  panel no longer depends on DM1 `world.things`, and localization uses the
  CSB domain (`parity-evidence/csb_v1_eye_scroll_f0341.md`).

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native direct-CLI and start-menu matrix against the staged
  original media. Atari ST/STX, nested and French preservation ZIP routes;
  Amiga ZIP→ADF routes; and English and Japanese FM Towns routes all reached
  the native campaign handoff. The Atari and Amiga M12→M11 source handoffs
  were also exercised from the selected in-memory media owners.

Reviewed 2026-08-29. Completed work only.

- Atari ST CSB gameplay now remains on the `CHANGE7_01_FIX` VBlank path
  after `ANIM.C` hands off to `FTLCODE`: every 50 Hz gameplay tick delivers
  the source VBlank model, whose palette-start callback installs the already
  verified `GRAPHICS.DAT` C232 light palette. The title's original P4B1
  palette remains title-owned. This is in-process scheduling and palette
  selection only; it neither changes simulation cadence nor creates media.

- `SWITCH.DAT` is now verified from the supplied CSB Utility Disk itself:
  `csb_v1_atari_switch_dat_real_media` opens the original STX, retains its
  7,405-byte `SWITCH.DAT` member only in process memory, validates the
  checksum/header/options/palette, and decodes every enabled source graphic.
  The separate compact fixture remains limited to malformed-input boundaries;
  it is not positive game-data evidence and neither route extracts media.

- M11's CSB query-world handoff now consumes the exact verified
  `CSB_V1_DungeonData::raw_data` bytes retained by the selected boot reader.
  It no longer attempts to reopen an STX/ADF/archive locator as a loose
  `DUNGEON.DAT`. Real Atari STX and Amiga ZIP → ADF launcher regressions prove
  title/start-menu handoff, native mirror/candidate flow, and the first
  runtime frame with no extracted game-data file; FM Towns uses the same
  bounded-memory source contract.

- The supplied CSB Utility Disk's supported single-member LZMA2 7z profile
  is now decoded by Firestaff in bounded memory. Its Atari STX member and
  nested files, including `START.PRG`, can be read without `7z`, `bsdtar`, or
  another external tool at runtime and without materializing game data on
  disk. The reader validates both 7z header CRCs, the extracted member CRC,
  declared sizes, and the member name; unsupported 7z structures remain
  closed rather than falling back to an external extractor. The M12
  regression runs against the supplied archive with external archive tools
  disabled.
- The supplied Amiga ZIP→ADF `Graphics.DAT` now has an in-memory native
  runtime-family receipt: inventory/panel, pit and field, stairs, wall and
  floor ornaments, and doors all decode through the big-endian IMG1 consumer
  without a PC3.4 fallback or an extracted game-data file.
- The active CSB Amiga runtime-graphics CTest is now that ZIP → ADF → OFS
  receipt itself (`csb_v1_amiga_runtime_graphics_real`), rather than a
  separate loose-`GRAPHICS.DAT` test that skipped against the supplied media.
- The supplied CSB Amiga A-disk now has a direct native graphics-format
  receipt. `test_csb_v1_amiga_graphics_dat` reads `Graphics.DAT` from the
  original ZIP → ADF chain in RAM through the AmigaDOS OFS reader, validates
  its Amiga record table and known language/version identity, then decodes
  the source C017 inventory panel at its original 224×136 dimensions. This
  is source-format evidence only; composed-screen capture parity remains
  active work.
- Since this original ADF route is staged, its graphics test no longer
  constructs positive IMG1 or `Graphics.DAT` stand-ins. Positive parsing and
  decode evidence comes only from the admitted ADF member; malformed-header
  checks remain as fail-closed boundaries.
- The supplied Amiga A31 archive now has the same first-runtime input matrix
  evidence as the Atari and FM Towns releases. Each direction, strafe and
  action is launched in a fresh native ZIP → ADF session, with the original
  title owner and campaign state retained in memory; no generated save or
  replacement dungeon is used.
- An authentic FS-UAE/Kickstart 1.3 comparison found and fixed Amiga RGB4
  palette quantization. CSB title, entrance, credits and runtime/HUD surfaces
  now expand original 0x0RGB registers directly to RGB8 instead of passing
  through the VGA six-bit DAC path; the screenshot writer also preserves the
  exact presented table. The real A31M handoff regression checks all title
  entries plus dungeon and C005 credits presentation. See
  `parity-evidence/csb_v1_amiga_rgb4_original_capture_20260905.md`.
- The native Atari STX CLI route verifies original title startup, runtime and
  start-menu entry using the supplied campaign media. Its source-owned input
  regression now also covers backward movement, both turns, both strafes and
  action from independent original STX sessions, each retaining a nonzero
  native viewport receipt. The unchanged initial strafe/action position is a
  recorded source result, not synthetic content.
- The supplied French Atari preservation ZIP now follows its original
  `ZIP → STX` path in RAM.  Its protected sector descriptors retain their
  logical order even when capture offsets are skewed, so the verified shared
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair reaches title, start menu, and first native
  movement without a replacement image or disk extraction.
- The M12/M11 Atari STX route now retains the hash-verified original
  `ANIMATE.SCR`/`ANIMATE.DAT` container when the selected runtime cache holds
  only `GRAPHICS.DAT`/`DUNGEON.DAT`; 50 Hz VBlank cadence, final FTLCODE
  handoff and first native HUD/viewport frame are exercised against that media.
  The completed 224×136 source-owned viewport publishes a nonzero FNV-1a
  receipt without being promoted as a PC F0128 runtime-session receipt.
- On that verified Atari route, native Enter/Accept now crosses the retained
  ANIM.C → FTLCODE handoff instead of being lost in the unrelated PC startup
  dispatcher. A requested CSB PC platform is explicitly rejected before any
  media or cache selection, because no original DOS/PC release exists.
- The Atari M12/M11 handoff regression now fails safely and precisely when a
  selected package cannot be opened, instead of cascading or crashing.
- The FM Towns F31 start-menu receipt now identifies its source-owned
  `TITLE.ANM` palette/frame handoff with a nonzero frame hash. An explicit
  F0435 user-save launch is kept distinct and proves the admitted C03
  executable handoff instead of claiming that it replayed the title.
- The current real-media launch matrix covers Atari STX, Amiga ZIP → ADF, and
  both English and Japanese FM Towns selection. Each route was exercised from
  its original source through title, normal start-menu launch, and the first
  native `UP` movement into the campaign. This is launch/runtime evidence,
  not a claim of complete campaign playthrough parity.
- Direct Amiga ZIP → ADF launches now identify A31M from `TITL.DAT` in the
  exact same virtual ADF as the selected `GRAPHICS.DAT`, rather than requiring
  an M12 cache leaf. The start menu publishes the source-owned `TITL.DAT`
  boundary and hash, while unrelated outer-archive or host files remain closed.
- Amiga A31E and A31M original ZIP → ADF media are read entirely in RAM.  The
  A31E direct C03 handoff verifies `APPB.FTL` and `BJELoad_R` through the same
  selected ADF as `GRAPHICS.DAT`, reaches `csb-entrance-0` with the original
  A31E hash, and does not create an asset-cache copy.  A31M's original
  `TITL.DAT`, `APPB.FTL` language page and `KAOS.FTL` continuation now use
  the same source locator and pass the real CLI and start-menu route into
  runtime after the old extracted cache is absent. The focused real-media
  regression uses virtual source locators rather than the legacy
  materialization API.
- The Atari R1 Hint Oracle now reads its hash-discovered `MINI.DAT` member
  directly into RAM before native GAMEBLOCK decoding.  It no longer writes an
  extracted Utility Disk save into an asset cache; a real STX CLI regression
  covers the direct `--csb-hint-oracle` route.
- CSB Utility Disk import now verifies the original archive member directly
  in bounded RAM.  UTIO.C sector 7 is read through the native STX transport
  reader when required, so the supplied Atari Utility Disk neither needs nor
  creates a transient ADF cache.
- The retired game-media disk-materialization switch now fails configuration
  and the media-admission source rejects direct activation.  Packed CSB
  formats therefore remain source-owned and in-memory only; a format without
  a native reader fails closed rather than creating a cache copy.
- A31M's source-owned Utility Disk DB2 instruction is decoded from the live
  selected dungeon, catalogued without the adjacent encoded champion-stat
  payloads, and translated only at the CSB PO presentation boundary. The
  real ZIP → ADF → M11 test proves the Swedish result and original fallback.
