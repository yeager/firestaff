# Firestaff DONE - NEXUS

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-10 - Nexus Saturn input-hook timing correction

- The external Mednafen capture patch now applies the deterministic START
  frame selection at `SMPC_StartFrame` and applies the pad byte after
  `IODevice::UpdateInput` has refreshed the host port, so the emulated Saturn
  actually consumes the injected state.
- The injected Saturn pad bit now follows Mednafen's active-high host-port
  representation: the button mask is set during the press window and cleared
  at its end. This removes a real polarity defect from the capture harness;
  it does not manufacture a menu transition.
- The complete patch applies cleanly to the pinned Mednafen source and the
  external Saturn-only build links with the verified M68K object.
- A 1,800-frame J-BIOS 1.01 / English retail-disc run passes raw-envelope
  validation and observes VDP1 activity, but frames 300, 900 and 1,500 retain
  the same `NBG1` bitmap state and one-draw VDP1 chain. The raw witness still
  has no source-owned startup-to-menu transition, so no MENU.BPK/FONT256 or
  startup-to-menu admission is claimed.
- A 600-frame rerun with the corrected post-poll injection changes the real
  Saturn witness: VDP2 switches from the initial all-layer character setup to
  the later `NBG2/NBG3` composition, while VDP1 changes from the initial chain
  to the direct-colour command chain. MENU.BPK/FONT256 byte ownership is not
  yet proven for those spans, so renderer admission remains closed.
- The same-session VDP1 writer trace records 9,260 authenticated VRAM writes.
  The bulk data corridor is observed at SH-2 PC `0x060135e8`/`0x060135f4`,
  while the Saturn command-list writer is observed at `0x06001782`. A code
  snapshot at the `0x10a00` target confirms the bulk corridor, but without a
  source-read/CD-origin join it does not identify MENU.BPK, FONT256 or a
  DGN face; no renderer gate is opened.

## 2026-08-10 - Nexus VDP2 tilemap register-order correction

- VDP2 NBG1-tilemap capture now preserves big-endian `TVMD=0x0080` register
  envelopes instead of routing them through the little-endian probe.
- Regression coverage now exercises both legacy big-endian and native
  little-endian register serializations. The change affects only authenticated
  capture decoding; it does not open an unverified menu or production route.
- Verification: `test_nexus_v1_vdp2_tilemap_capture_compositor`,
  `test_nexus_v1_vdp2_runtime_tilemap`, `test_nexus_v1_vdp2_capture_compositor`,
  and `test_nexus_v1_vdp12_capture_composition` PASS.

## 2026-08-10 - Nexus same-session VDP1/VDP2 + SCSP capture pipeline

- Mednafen-byggscriptet applicerar nu SCSP main-/sound-CPU-spårningen och
  launcher-scriptet propagaterar samt hashnoterar trace-filerna.
- En extern kallstart med J-BIOS 1.01 och hashbunden English/Merged-disc gav
  1 200 råa VDP1/VDP2-frame-block och 1 033 icke-idle VDP1-observationer.
- Validatorn följer den faktiska V2-state-raden och payload-storleken från
  capture-patchen. Event-selector, SAL-codec, voice-route och playback är
  fortfarande blockerade där samma-session-bevis saknas.

## 2026-08-10 - Nexus SLEV/SAL/SDDRVS runtime-corridor binder

- `nexus_v1_scsp_runtime_join()` binder nu separat verifierad main-SCSP-
  producenttrace, sound-CPU-trace och retail `SDDRVS.TSK`-disassembly under
  hashbundna SLEV/SAL/MAP/SDDRVS-identiteter.
- Bindningen exponerar endast den bevisade command-handler/SCSP-voice-route-
  korridoren. Event-selector, SAL-codec, sample-rate och host-playback är
  fortsatt explicit `0`/blockerade.
- Den autentiska capture som finns monterad saknar voice-register-write i
  samma trace och avvisas därför av den nya produktionsbindningen; testet
  verifierar både denna fail-closed väg och en separat bounded contract-fixture.
- Verifiering: `test_nexus_v1_slev_scsp_runtime_join`,
  `test_nexus_v1_scsp_trace`, `verify_nexus_production_source_boundary.py`.

## 2026-08-10 - Nexus VDP1 Structure3 owner receipt

- VDP1:s DGN-materialresolver traverserar nu den hashbundna Structure3-
  directory/face-tabellen och räknar exakta face-rader vars råa fill-selector
  matchar den bytejoinade Structure2-bilden.
- `Nexus_V1_Vdp1CaptureCompositeReceipt` och sekvensreceiptet behåller denna
  owner-join separat från bild/CLUT-joinen. Tvetydiga nollträffar öppnar inte
  renderer eller scenägarskap.
- Retail-kedjans frame-/transform-/culling-semantik förblir capture-gated;
  focused VDP1 build och resolver/compositor-tester passerar.

## 2026-08-10 - Nexus VDP1 direct-colour DGN owner join

- VDP1 colour mode 5 kan nu bindas till en unik DGN Structure2 encoding
  `28h`-bild med exakt Saturn little-endian VRAM ↔ canonical big-endian
  byteordning.
- Direct-colour capture receipt rapporterar `source_join_verified`; ingen
  CLUT eller host-palette uppfinns och `renderer_permitted` förblir stängd.
- Tester täcker både `28h`-ägarskapsjoin och direct-colour-capture med
  source-join receipt. Fokuserad Nexus-build passerar.

## 2026-08-10 - Nexus FONT256 raw VDP2 Page-span join

- `nexus_v1_font256_vdp2_capture_join` jämför nu den exakta råa Page-regionen
  med den hashattesterade `FONT256.S2D`, tillsammans med Character Generator
  och Palette.
- Ändrad Page avvisas av `test_nexus_v1_font256_vdp2_capture_join`.
- `text_code_mapping_proven=0` och `semantic_admission_blocked=1` behålls:
  detta är source/capture-proveniens, inte en gissad FONT256-text- eller
  meny-renderare. Fokuserad Nexus-build och real-data-tester passerar.

## 2026-08-10 - Nexus DGN Structure2 source decoder

- Promoted the DMWeb-authenticated Structure2 decoder into the production
  engine's source lane for canonical LEV00-LEV15 data. Encoding `08h` now
  decodes MSB-first 4bpp pixels and resolves zero-offset palette reuse by
  Palette ID; encoding `28h` preserves exact big-endian Saturn 15-bit words
  in the source-only `direct_pixels` lane instead of quantizing them to a
  guessed host palette.
- The real LEV01 engine test confirms every descriptor decodes and that
  `animated_floor_material_route_valid` remains disabled. Existing VDP1/CLUT,
  transform, culling and viewport gates remain closed.
- External-disk build and focused CTest (`dgn_geometry_readiness`,
  `boot_file_hash_scan`): PASS.

## 2026-08-10 - Nexus FONT256 source-word retention

- `Nexus_V1_FontS2dSourceWords` retains the exact big-endian Page (4096),
  Palette (256), and Attribute (242) words from the authenticated
  `FONT256.S2D` decode in the production engine object. The retention API is
  bounded by the DMWeb SCR region helpers and does not assign glyph, palette,
  text, VDP2 placement, or framebuffer meaning.
- The real-data FONT256 decoder test now verifies that the retained words
  agree with the direct source helpers. The production engine keeps
  `font_loaded == 0`; no unsupported Saturn text route is opened.
- Verification: external-disk targeted build of `firestaff_nexus`,
  `test_nexus_v1_boot_file_hash_scan`, and `test_nexus_v1_font_s2d`; targeted
  CTest 4/4 passed with the user-owned Nexus data root. `git diff --check`
  passed.

## 2026-08-10 - Nexus MENU.BPK source-pixel API

- Added `nexus_v1_menu_bpk_decode_source_surface()`. It revalidates the
  canonical MENU.BPK source and package hash, then exposes one DMWeb PRS3
  surface as exact indexed bytes through the production engine boundary.
  PALT remains raw/opaque and the API does not permit VDP1 upload, palette
  interpretation, or menu presentation.
- The real renamed-MENU.BPK boot test decodes entry 1 (16×15, 240 indexed
  pixels) through this engine API while retaining the Saturn presentation
  blocker. External-disk build and focused CTest: PASS.

## 2026-07-11 - Nexus FACE.BIN readiness gate

- `FACE.BIN` now has a byte-evidenced descriptor: a 56-byte `FACE` header,
  20 variable PRS3 frames, 128..131-byte opaque prefixes, 3,136-byte declared
  output per frame, and a two-byte container tail. PRS3 opcode and prefix
  palette semantics remain unproven. `nexus_ui_expand_face_record_48x48()`
  therefore leaves every canonical PRS3 frame blocked and
  `nexus_ui_load_face_record()` leaves no surface allocated on this path.
- M11 now honors the Nexus launcher asset receipt for title, save, and champion
  startup input instead of replaying blocked actions through ungated host facts.
  Canonical compact FACE media remains at title with `blocked-faces` and cannot
  reach champion rendering.
- `test_m11_nexus_startup_gate` and the new skip-safe
  `firestaff_nexus_v1_face_media_probe` read staged canonical Saturn bytes,
  validate exact first/final PRS3 frame bounds, and prove the non-crashing,
  no-synthesis gate. Verification: focused Ninja build; both direct probes
  pass with local Saturn media; strict C11 `-Wall -Wextra -Werror` syntax
  checks pass for the FACE descriptor and startup loader.
- ✅ 2026-07-11 CSB DSA source `LOAD` opcode-family handoff: added a bounded
  CSBWin DSA-word decoder/executor for one complete `DSACMD_LOAD` family,
  operating only on checksum-authenticated, runtime-owned `DSAAction` words
  after resume. It implements `A..Z`, `INTEGER`, `ABS`, `DOLLAR`, and
  `INTEGER32`, including CSBWin's signed five-bit next-state and `-16`
  extension word. `LOAD_ABS32` is retained as source-illegal because
  `DSA.cpp` has no execution case for its declared selector. Unsupported
  opcodes do not run or receive substitute behavior. Focused DSA unit and
  authenticated resume-handoff checks pass. Source: CSBWin `Data.h:1686-1708,
  1947-1984`; `DSA.cpp:1074-1189`; `SaveGame.cpp` DSA read boundary.
- ✅ 2026-07-11 DM2 GDAT fallback removal (`DM2-GDAT-FB-01`, `DM2-GDAT-FB-02`): real boot-profile frames now mark `GRAPHICSSET/<MapGraphicsStyle>` floor/ceiling and selected `WALL_GFX` failures as explicit blocked no-draw material receipts instead of painting the old gray/brown planes or aggregate/per-panel wall rectangles. Synthetic and no-data renderer paths keep their deterministic fallback behavior. Runtime frame ownership carries the blocked count/mask and refuses a full GDAT frame while any required base material is blocked. Source lock: skproject `SKWIN/SkWinCore.cpp` `MapGraphicsStyle()`/`GDAT_CATEGORY_GRAPHICSSET` queries and `DRAW_MAP_CHIP` `GDAT_CATEGORY_WALL_GFX` `QUERY_DUNGEON_MAP_CHIP_PICT` route. Coverage: the runtime handoff smoke test forces missing floor, ceiling, and wall GDAT assets and verifies receipts plus untouched framebuffer pixels; the hash-verified boot-profile gate requires zero blocked material receipts on the real frame. Verification: `cmake --build build --target test_dm2_v1_runtime_handoff_smoke test_dm2_v1_boot_profile_smoke --parallel 4` completed clean; `ctest --test-dir build -R '^dm2_v1_runtime_handoff_smoke$' --output-on-failure` passed.

- ✅ 2026-07-11 Theron Track02 repeatable nonstartup-region catalog: scanned
  both hash-verified raw JP/US Track 02 BIN sector streams after excluding the
  six indexed all-zero logical containers. The new strict locator retains 11
  repeatable nonzero MODE1 user-data runs only when their sector spans differ
  by the observed one-sector physical offset and their full user-data byte
  counts, nonzero counts, and hashes agree. It records both variants' raw and
  sector bounds plus rejected nonrepeatable runs as negative evidence. The
  catalog accepts no same/unknown variant, exports no bytes, assigns no
  semantics, and remains opaque/promotion-blocked. Verification: strict
  `cc -std=c11 -Wall -Wextra -Werror` compile of `theron_v1_track02.c`; direct
  focused `firestaff_theron_v1_track02_nonstartup_sector_receipt_probe` passed
  against the staged raw JP/US media (0 failures, 0 skips). The normal CMake
  target remains blocked by unrelated CSB undeclared DSA opcode symbols.
- ✅ 2026-07-11 DM2 G1 map-owned `GenericRecord::w0` traversal gate: replaced
  the over-broad whole-pool direct-link census with the source-shaped walk
  rooted only in `c_map` tile links. The new validator resolves each
  map-reachable ObjectID through the proven `READ_DUNGEON_STRUCTURE`
  text-adjacent 16-pool bases, rejects invalid DB type/index and record bounds,
  and rejects repeated record addresses per chain. This follows skproject
  `SKWINSPX/src/v4/skcore.cpp` `GET_ADDRESS_OF_RECORD` / `GET_NEXT_RECORD_LINK`
  lines 1184-1224 and `SKWIN/DME.h` `GenericRecord::w0` lines 831-847; it does
  not turn unused pool words into inferred links. Verification: focused target
  build passed; `test_dm2_v1_dungeon_loader_first_map_gate` passed 70/70,
  including unreachable-invalid, reachable-out-of-range, and reachable-cycle
  gates; the hash-verified PC G1 real-data probe passed 44/44 against
  `~/.firestaff/data/dm2/DUNGEON.DAT`. Honest scope: real G1 remains
  non-traversable because a map-reachable `w0` still fails strict validation.
# 2026-07-11 - Nexus Structure1G animation declaration handoff

- ✅ 2026-07-11 CSB F0115 native weapon/scroll/food composition: extended the CSB-only native resolver from chest/potion rows to all 46 PC34 weapon subtypes, the scroll, and junk food types 29-36. Mappings are source-locked to ReDMCSB `DUNGEON.C F0141:1147-1154`, `G0237` object-info rows 23-68 and 156-163, and `DUNVIEW.C G0209:1216-1307 / F0115:4923-5073`; `GRAPHICS.DAT` indices are generated only from those G0209 native-relative rows. The native compositor now applies horizontal mirroring only for source-marked `GraphicInfo & 0x0001` art, retains chest alcove suppression, and rejects unsupported indices. Focused CSB test covers resolver boundaries, right-lane no-flip behavior, and every admitted real-asset graphic when `FIRESTAFF_CSB_GRAPHICS_DAT` is staged.

- ✅ Nexus now validates the documented Structure1G declaration grammar using
  the canonical LEV00-LEV15 corpus: counted 8-byte descriptors, a terminal
  `FF` descriptor whose trailing bytes remain unconstrained, Structure2 image
  instructions, backward `FF FE` gotos, and `FF FF` sequence ends. The parser
  no longer assumes named Structure1 header pointers are address ordered.
  Real-media evidence is 14 present/valid tables, 51 declarations/sequences,
  154 image instructions, and 51 backward gotos.
- ✅ The source-evidenced Structure1B low-nibble animated-floor form is now
  carried into the DGN render plan with its animation ID and first Structure2
  image index. Only LEV08 uses it: 41 cells, all bound to declared animation
  ID 0. There is no inferred timing, flag execution, model-face animation, or
  static-material substitution. Since no verified Structure2-to-DMDF/BPK
  material bridge exists, any visible animated floor blocks the real DGN plan
  with fallback disabled.
- Verification: strict C11 `-Wall -Wextra -Werror` syntax checks;
  `test_nexus_v1_dgn_geometry_readiness` with local canonical LEV00-LEV15;
  focused CTest `nexus_v1_dgn_geometry_readiness` (1/1); and
  `firestaff_nexus_v1_dgn_material_corpus_probe` against local Track 1 media.
- ✅ 2026-07-11 DM1 V1 PC34 champion-panel runtime pixels: reordered M11's
  normal V1 panel lane to match ReDMCSB `STARTUP2.C` (`CASTER.C F0394`,
  `ACTIDRAW.C F0387`, then `MENUDRAW.C F0395`), corrected the real
  `G0498[12] -> C04` empty-hand palette remap, and kept C011's two
  12-scanline source rows non-overlapping at y=50..61 and y=62..73. The
  headless probe now skips unrelated launcher screenshot-gallery I/O before
  opening its selected real-data entry. Verification:
  `firestaff_dm1_v1_champion_panel_pixels_runtime_probe` passed against real
  DM1 PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT`; all four C089..C092 empty-hand
  icons matched all 256 source pixels, and C011 lines 2 and 3 retained all
  168 checked source pixels.
# ✅ 2026-07-12 CSB F0276/F0270 C10 local skill XP: a real-format C004 LocalEffect value 10 now follows ReDMCSB's immediate F0269 path instead of being treated as a deferred sensor rotation. Object-triggered C49 materialization divides the original 300 Steal XP by party count, skips dead champions after division, and credits both hidden Steal (8) and base Ninja skill XP. Source: ReDMCSB `MOVESENS.C F0269` lines 1038-1078, `F0270` lines 1088-1094, and `CHAMPION.C F0304` lines 879-906. Verification: dedicated live C49 regression passed 8/8; the manually rebuilt focused nine-binary F0267/F0276 group passed while shared CMake regeneration was blocked by two unrelated missing Theron probe sources.

# ✅ 2026-07-12 CSB F0276 C002 party route: party movement now evaluates C002 floor Theron/party/creature sensors at the live F0267 destination. It follows ReDMCSB's party/no-group eligibility, then uses the existing F0272/F0268 remote effect route. Source: ReDMCSB `MOVESENS.C F0267` lines 792-857 and `F0276` lines 1686-1689. Verification: `test_csb_v1_f0276_party_c002_sensor_pc34_compat` passed 5/5 through `MOVE_FORWARD`; the manually rebuilt focused ten-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C002 group route: the shared C002/C007 group eligibility route now has a dedicated live C04 move regression. A C04 group landing on a C002 floor Theron/party/creature sensor publishes the normal F0272/F0268 fakewall SET event. Source: ReDMCSB `MOVESENS.C F0267` lines 800-867 and `F0276` lines 1686-1689. Verification: `test_csb_v1_f0276_group_c002_sensor_pc34_compat` passed 4/4; the manually rebuilt focused eleven-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C001 group route: live C04 group movement now admits the ReDMCSB C001 floor Theron/party/creature/object branch only when party, ordinary objects, and another group are absent. A C04 group landing on C001 publishes the normal F0272/F0268 fakewall SET event. Source: ReDMCSB `MOVESENS.C F0276` lines 1678-1685. Verification: `test_csb_v1_f0276_group_c001_sensor_pc34_compat` passed 4/4; the manually rebuilt focused twelve-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C001 party route: live party movement now evaluates C001 floor Theron/party/creature/object sensors only after a true F0267 move into an empty, group-free destination. Same-square F0284 turns pass `PartySquare` and correctly suppress C001. Source: ReDMCSB `MOVESENS.C F0267` lines 792-857 and `F0276` lines 1678-1685. Verification: `test_csb_v1_f0276_party_c001_sensor_pc34_compat` passed 6/6 through `MOVE_FORWARD` and `TURN_RIGHT`; the manually rebuilt focused thirteen-binary F0276 group passed with strict runtime compilation.

# 2026-07-13 Nexus DGN static-material selector guard

The DGN runtime no longer treats Structure1B bytes 3/4 as direct
`SN_WALL.MNS` material IDs. The real LEV00-LEV15 corpus contains values beyond
the bank's 0..14 descriptor range, so a hash-bound MNS pair now requires an
explicit selector-binding proof before it can promote a DGN render plan. The
existing material-raster test supplies that proof only as a controlled host
fixture; production remains blocked without a Saturn executable/capture route.
Verification: Ninja build plus `test_nexus_v1_dgn_material_raster` and
`test_nexus_v1_dmdf_embedded_blocks` against the real local MNS asset.

# Nexus MNS TEXT Atomic Material Route (2026-07-13)

- `nexus_v1_dmdf_decode_text_material_bank()` now fails closed for the whole
  authenticated MNS TEXT bank when any descriptor cannot occupy a unique
  256-entry host slot, allocation fails, or a surface exceeds the indexed
  palette capacity. The DGN route cannot use a partial original material bank.
- `test_nexus_v1_dmdf_embedded_blocks` covers the structurally valid but
  out-of-bank source-ID rejection and passes against both local canonical
  `SN_FLOOR.MNS` and `SN_WALL.MNS` assets.

# Nexus MENU.BPK PALT / WARNING.BIN correlation (2026-07-15)

The canonical MENU.BPK PALT trailer and WARNING.BIN resource 0's documented
DGT2 CLUT share 224 identical indexed big-endian 16-bit words. This establishes
only a source-owned BGR555 word-encoding correlation. The other 32 PALT words
remain raw source values; no PRS3 entry association, palette application, pixel
decode, or rendering route is enabled.

The documented DGT2 BGR555 reader's low-15-bit colour mask also matches 255 of
256 indexed PALT words: 31 raw mismatches are bit-15-only, while index 130 is
one actual colour-word difference. This strengthens the byte correlation but
does not declare PALT to be DGT2 or promote any decoder/palette route.

# Nexus PRS3 V1 nonzero control-byte path (2026-07-15)

The canonical DM.BIN loader now has a source-locked SH-2 receipt for its
nonzero low-bit fallthrough: shifted R11 is tested against mask 1, `BT` selects
the separate zero-side corridor, and the nonzero path guards input, reads one
byte through `@R12+`, then reaches the existing R2 byte store. This proves only
a bounded direct-byte path. It does not establish the zero-side grammar, token
names, buffer ownership, termination, palette use, PRS3 pixels, or rendering.

# Nexus PRS3 V1 zero-side termination condition (2026-07-15)

The hash-bound zero-side corridor now retains the exact SH-2
`CMP/EQ R1,R10; ADD #1,R10; BF/S` control sequence and branch target. It
proves that the repeat loop returns only while the compared registers differ.
That is a static termination condition, not a claim that either register is a
run length or that the zero side is a backreference. No PRS3 output, palette,
or renderer route is enabled.

# Nexus PRS3 V1 zero-side two-byte source span (2026-07-15)

The verified zero-side corridor now records its adjacent `MOV.B @R12+` reads:
one source byte, its sign extension, then the immediately following source
byte. This proves an exact two-byte sequential input span before the raw merge.
It does not assign fields, history access, output, token, palette, or pixel
semantics, and cannot enable decoding or rendering.

# Nexus PRS3 V1 zero-side merge order (2026-07-15)

The source-bound DM.BIN receipt now proves the full static merge sequence:
both sequential inputs are zero-extended, the second byte is copied and shifted
four bits, masked with the PC-relative `0x0f00`, and ORed into the first byte;
the source then masks the second register with immediate `15`. This fixes the
raw merge order as `byte0 | ((byte1 << 4) & 0x0f00)`. It does not assign the
word a PRS3 semantic or permit decoding, palette application, or rendering.

# Nexus PRS3 V1 post-merge control edge (2026-07-15)

The zero-side receipt now binds the actual post-merge sequence: mask R7 with
15, add 2, add merged R4, `CMP/GT R7,R4`, then `BT` back to the PRS3 control
re-entry. It proves that exact static branch condition and target only. No
register role, token rejection, termination, output, palette, decoder, or
renderer meaning is inferred.

# Nexus PRS3 V1 source-counter terminal path (2026-07-15)

The hash-bound DM.BIN receipt now proves that the refill, nonzero, and zero-side
source-counter guards all branch to one shared epilogue. That path writes zero
to R0 before the routine's RTS. It is intentionally recorded only as a static
failure-shaped terminal path: it does not prove a PRS3 end marker, externally
observed status, successful termination, decoder output, palette, or rendering.

# Nexus PRS3 V1 branch-local source consumption (2026-07-15)

The source-locked DM.BIN receipt now records the two exact SH-2 R14 counter
debits that precede direct stream reads: the nonzero low-bit corridor executes
`ADD #-1,R14` before its one-byte `@R12+` read, and the zero-side corridor
executes `ADD #-2,R14` before its two adjacent `@R12+` reads. The receipt
rejects either changed instruction. This establishes only static input-byte
consumption by branch; it does not name PRS3 tokens, fields, output/history
behavior, palette use, decoded pixels, or a renderable surface.

# Nexus PRS3 V1 indexed-byte control operands (2026-07-15)

The source-locked zero-side receipt now records the exact static operand flow
from `MOV.B @(R0,R13),R1` through `CMP/EQ R1,R3` and then `CMP/EQ R1,R10`,
which is immediately followed by the existing `BF/S` repeat decision. The
receipt rejects a changed indexed load or compare corridor. This identifies no
PRS3 token, history buffer, copied/output byte, length, terminator, palette,
pixel, or drawing behavior; those meanings still need an authenticated Saturn
execution trace.

# Nexus PRS3 V1 nonzero post-store control route (2026-07-15)

The retail DM.BIN receipt now locks the complete static nonzero post-store
corridor. After the guarded `R2 -> @(R13,R0)` byte store it executes
`ADD #1,R6`, `AND R5,R6`, and branches to the shared PRS3 control re-entry.
The receipt rejects a changed update, mask, or re-entry branch. This is a
source-control fact only: it does not establish a literal token, R6/R13
allocation, output byte order, decoded data, palette, pixels, or rendering.

# Nexus PRS3 V6 dynamic control-operand capture (2026-07-15)

The VDP1 trace schema now admits V6 only when it includes one ordered dynamic
operand witness for each low-bit branch: the real nonzero and zero R14 debit
PCs, before/after R14 values, before/after R12 cursor values, and decrement to
input-read sequence order. Source binding rechecks both PCs against the locked
DM.BIN receipt. The witness remains unauthenticated capture evidence and does
not prove PRS3 token grammar, output semantics, palette, pixels, or drawing.
# Nexus PRS3 V7 nonzero source/output witness (2026-07-15)

V7 now requires one bounded MENU.BPK byte witness for the nonzero control
corridor: payload offset and byte, the source-locked nonzero output-store PC,
observed output byte/address, and ordered write sequence. Asset binding checks
the input byte against the selected real stream and rejects an output byte that
does not match the source-owned direct-byte route. This is one capture-bound
transfer candidate, not a complete PRS3 literal/token grammar, decoder,
palette/pixel interpretation, or render permission.

# Nexus PRS3 V8 zero-side source/merge witness (2026-07-15)

V8 now admits a bounded zero-side witness only when two adjacent MENU.BPK
source bytes match the selected real stream, their read PCs match the retail
DM.BIN corridor, and their observed merge equals `byte0 | ((byte1 << 4) &
0x0f00)`. The zero-side corridor has no proven direct output store, so V8
deliberately records no fabricated output pair. This does not identify a
token, offset, history copy, length, output, palette, decoded pixel, or draw.
# Nexus Structure1F direct face capture bridge (2026-07-15)

One active canonical Structure1F owner can now write an atomic capture-producer
manifest containing its exact LEV identity, Structure1A owner/model/rotation,
selected Structure3 face ordinal, face/vertex/normal row fingerprints, parsed
vertex indexes, and raw Structure1A transform-table fingerprints. The writer
requires the existing source-bound geometry and transform receipts and emits
only `original_saturn_capture_required=1` and `no_draw_only=1`. It does not
decode materials or establish transforms, culling, VDP1, palette, pixels, or
rendering.
# Nexus Structure1F direct face host manifest gate (2026-07-15)

The package boundary now consumes a direct face capture manifest only after
rebuilding the active canonical Structure1F target and matching its LEV hash,
owner location, Structure1A/Structure3 references, face/vertex/normal row
fingerprints, vertex indexes, and transform-table fingerprints. Any malformed
or changed field is fail-closed. Acceptance preserves `no_draw_only`; it is
not evidence of a Saturn transform, culling result, material, VDP1 command,
palette, pixels, or rendering.

# Nexus Structure1F capture target trace-route binding (2026-07-15)

Direct-owner transform trace intake now has five distinct files: the canonical
direct-face target, debugger manifest, raw trace, transform snapshot, and
independent attestation. It consumes and validates the target first, so a
changed or malformed package target blocks trace admission even when the other
sidecars remain valid. An accepted trace remains opaque and no-draw; it does
not establish transform, material, palette, pixels, culling, VDP1, or rendering.
# Nexus Structure1F launcher host capture gate (2026-07-15)

The launcher now exposes a direct-face capture intake that independently
rechecks its initialized, active, canonical LEV source before delegating to
the engine manifest gate. This carries the exact accepted request to the host
boundary without allowing a caller to substitute another level's data. The
receipt remains capture-only and no-draw: no Saturn transform, face material,
palette, VDP1 command, pixel, or rendering claim is added.

# Nexus Structure1F VDP1 capture prerequisite (2026-07-15)

The direct-face launcher gate now also retains the canonical `DM.BIN` static
SH-2 VDP1 state-write receipt. Its proven raw fields include the `0x04 = 2`
control write, the VDP1-VRAM base handoff, and `0x06 = 0x8000`, `0x08 = 0`,
and `0x0a = 0xffff`; a changed or
unverified executable blocks capture intake. This is a capture prerequisite,
not a claim that those registers select a Structure1F face or prove a command,
transform, material, palette, pixel, or rendering result.

# Nexus Structure1F direct material corpus gate (2026-07-15)

All canonical LEV00--15 files now pass a direct-owner material audit. It finds
zero source-proven Structure1F-to-Structure2 static material links through the
current owner/model/face relation, so the engine cannot turn a shared selector
into a texture or VDP1 claim. The missing link remains an original-Saturn
capture requirement; no material or renderer path was enabled.

# Nexus Structure1F raw VDP1 capture host route (2026-07-15)

`nexus_v1_launcher_dgn_direct_face_raw_capture_intake()` now routes an
authenticated six-lane Structure3 capture through the initialized canonical
LEV host only after the direct Structure1F manifest and the static DM.BIN VDP1
capture prerequisite pass.
`nexus_v1_engine_bind_structure1f_direct_face_raw_capture()` compares the full
DGN/Structure3 face candidate (source, mesh corpus, entry, ordinal,
face/vertex/normal rows, and selector) before retaining opaque capture bytes.
Missing or altered lanes, attestations, source identity, binder receipts, or
face fields fail closed. The route is explicitly no-draw and proves no VDP1
command, texture, palette, transform, or raster semantics.
Verification: `test_nexus_v1_direct_static_material_capture` against local
canonical LEV00--15 and `test_nexus_v1_structure3_capture_manifest` pass.

# Nexus Structure1F VDP1 texture/palette material link (2026-07-15)

`nexus_v1_engine_bind_structure1f_vdp1_material_capture()` now requires the
runtime-owned direct-face capture lanes to be byte-identical to their
authenticated six-lane source. It additionally requires a documented texture
command, a complete VDP1-VRAM snapshot with the exact CMDSRCA texture window,
and a unique copy of that 32-byte command in VDP1 VRAM. The receipt retains
the command's raw `CMDCOLR` word and the exact palette-lane hash together, but
does not interpret either as CRAM/CLUT addressing, colour format, texel order,
or pixels. The launcher consumes this receipt after the opaque capture handoff
and remains fail-closed/no-draw. Verification:
`test_nexus_v1_direct_static_material_capture` and
`test_nexus_v1_dgn_geometry_readiness` pass.

# Nexus Structure1F VDP1 mode-1 lookup decoder (2026-07-15)

The new `nexus_v1_vdp1_decode_mode1_lookup_texture()` implements only the
documented VDP1 mode-1 lookup route: four-bit texture samples are consumed
high nibble then low nibble, and `CMDCOLR * 8` addresses the 32-byte,
16-entry lookup table in the same authenticated VDP1 VRAM snapshot. It emits
raw 16-bit VDP1 colour codes, never RGBA pixels or a draw command. The engine
enters this path only after the direct Structure1F capture, byte-identical
runtime lanes, command, and CMDSRCA window gates pass. An optional witness
comparison is byte-for-byte but cannot authenticate a caller-supplied witness.
This follows Sega's VDP1 User's Manual, sections 6.3--6.5; VDP2/CRAM and
game-specific palette semantics remain blocked. Verification:
`nexus_v1_vdp1_lookup_decode` and
`nexus_v1_direct_static_material_capture` pass.

# Nexus Structure1F VDP1 mode-1 VDP2 palette-chain verifier (2026-07-15)

`nexus_v1_vdp1_resolve_mode1_palette_capture()` now consumes the documented
mode-1 lookup result only alongside an explicitly attested full VDP2 CRAM
capture (4 KiB) and its raw register image. It preserves the VDP1 source-index
rules from `CMDPMOD`: index 0 is transparent when SPD is clear, and index F
ends a source scanline when ECD is clear. A lookup value with bit 15 set is
decoded as RGB555; otherwise the verifier adds captured `SPCAOS` to the
11-bit sprite dot-colour value, applies captured `RAMCTL/CRMD` address rules,
and reads the selected RGB555 or RGB888 CRAM entry. Prohibited CRMD=3,
partial state, and unattested captures fail closed. The output is a
source-independent inspection receipt, never a host draw or a claim of final
Saturn composition. The focused test verifies transparent, direct-RGB,
CRAM-addressed, end-code, post-end suppression, and capture-attestation
rejection paths. It does not claim an actual Nexus output witness: a real
capture's final VDP1/VDP2 pixel stream must still byte-match before rendering
can be enabled. Verification: `nexus_v1_vdp1_lookup_decode`,
`nexus_v1_direct_static_material_capture`, and
`nexus_v1_dgn_geometry_readiness` pass.
# ✅ 2026-07-15 Nexus PRS3 nonzero transfer debit gate

The external SH-2 PRS3 nonzero transfer binder now requires the real
DM.BIN-proven nonzero R14 source-counter debit PC (`0x14dd2`) before it accepts
an opaque MENU.BPK byte-transfer candidate. The previous `zero_branch + 2`
delay-slot shortcut (`0x14dce`) is rejected by regression coverage, so a stale
capture cannot satisfy the nonzero path. This remains evidence-only: no PRS3
opcode grammar, decoder, output ownership, palette, pixel, VDP1, or menu draw
route is promoted. Verification: `nexus_v1_prs3_capture_trace_schema`,
`nexus_v1_bpk_archive`, `nexus_v1_bpk_surface_class`, and
`nexus_v1_bpk_prs3_payload_evidence` pass.

# Nexus multi-level direct-LEV capture-plan gate (2026-07-17)

- The operator-only multi-level capture launcher now requires a valid,
  direct-file-only LEV00--LEV15 discovery receipt and exact per-level DGN FNV
  agreement before emitting a capture job. The receipt retains identities
  only; it does not materialize LEV payloads, decode graphics, or enable draw.
  Focused launcher coverage and the full `firestaff` build are green.

# Nexus direct SLEV/SAL no-runtime launch provenance (2026-07-17)

- Nexus now discovers only direct, hash-verified `SLEV00`--`SLEV15`,
  `SNDLEV` SAL/MAP, and `SDDRVS.TSK` files and retains path, size, MD5, and
  FNV identity only. M11 accepts an active-level receipt only when those
  identities match its current auxiliary sources and the active card/MENU.BPK
  route epoch. Missing, mixed, stale, and cross-level inputs fail closed;
  script dispatch and SFX playback remain blocked. Focused discovery and M11
  lifecycle CTests plus the full `firestaff` build are green.

# Nexus direct SDDRVS dungeon admission (2026-07-17)

- M11 dungeon admission can now carry a direct, hash-verified `SDDRVS.TSK`
  identity only when the existing direct-LEV, card, MENU.BPK package, route
  epoch, and active level-aux receipts still agree. The driver file is
  rehashed and FNV/size-checked at consumption, so missing or mutated bytes
  invalidate the prior discovery receipt. This remains no-draw and
  no-dispatch: it does not parse SDDRVS, execute scripts, decode audio, or
  play sound. Focused stale/mutated/missing coverage and the full `firestaff`
  build are green.

# Nexus direct SAL/SLEV dungeon admission (2026-07-17)

- M11 can now retain one no-draw dungeon receipt for a level-local direct
  SLEV/SAL/MAP identity only when its direct LEV, card, MENU.BPK package,
  epoch, and active auxiliary receipt agree. Every direct asset is rehashed
  and checked against its stored size/FNV at consumption; missing, mutated,
  and stale identities fail closed. The route still blocks script dispatch,
  codec promotion, and SFX playback. Focused fixture coverage and the full
  `firestaff` build are green.

# Nexus direct SAL container provenance (2026-07-17)

- Direct SAL consumption now accepts only the source-backed `dsp01.EX`
  preamble, exact full-file FNV/size, and one bounded opaque payload interval
  with its own FNV. M11 SAL/SLEV dungeon admission requires that receipt and
  rejects malformed headers, source drift, and empty/out-of-bounds intervals.
  No codec, sample format, decode, or playback is implied. Focused container
  CTests, the full `firestaff` build, and `git diff --check` are green.

# Nexus direct SNDLEV MAP table provenance (2026-07-17)

- Direct MAP consumption now requires exact source FNV/size, its documented
  24-byte opaque header, bounded 8-byte row table, and `ff ff` terminator.
  M11's SAL/SLEV dungeon receipt requires this no-playback provenance beside
  the direct SAL container receipt; malformed, unterminated, or drifted maps
  fail closed. No selector/event meaning, codec, or playback was added.
  Focused parser coverage, full `firestaff`, and `git diff --check` are green.

# Nexus M11 SNDLEV MAP row no-playback route (2026-07-17)

- M11 now admits a selected opaque MAP row only after it rehashes the direct
  MAP source and exactly binds the row index, offset, fixed length, row FNV,
  table FNV, level, package, card, and route epoch to the existing no-draw
  dungeon receipt. Mutated/stale sources and rows outside the bounded table
  reject. This does not interpret MAP selector/event bytes or permit codec
  promotion, script dispatch, or audio playback. Focused MAP provenance
  CTests and the full `firestaff` build are green.

# CSBWin active timer-queue M11 handoff (2026-07-17)

- M11 resume coverage now follows CSBWin `GAMEBLOCK2.NumTimer`, not the
  serialized TimerQueue storage capacity. The verified fixture retains only
  its two active queue slots in source order, with `timeline_queue.gameTick`
  equal to the restored save tick; the third stored slot remains inactive.
  `csb_v1_m11_csbwin_timer_queue_resume` verifies that atomic valid handoff
  and rejects a checksum-corrupt queue before M11 publishes a session. The
  full `csb_v1_m11_startup_resume_gate` is now green. Verification: both
  focused CTests PASS; isolated `firestaff` build and `git diff --check`
  PASS.

# CSBWin DSA restored-TIMER owner receipt (2026-07-17)

- Tightened the existing checksum-bound DSA/save runtime handoff with an
  immutable per-execution TIMER owner receipt. The receipt now binds the
  original TimerQueue slot and TIMER index to the exact CSBWin Timer.cpp
  function, saved action/position bytes, target location, concrete type-47
  actuator location, and already authenticated DSA action. Currentness
  rebuilds that owner selection under the existing save, Dungeon.dat, startup
  session and runtime-tick handoff before M11 may commit the outcome. Any
  function/action/slot/location/action-selection drift rejects without rerun
  or mutation. Unknown opcode bodies remain blocked by the existing source
  verifier; no wrapper, timer synthesis, or new DSA semantics were added.
  Verification: `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MESSAGE queue transaction (2026-07-17)

- Tightened the source-proven `DSACMD_MESSAGE`, `MESSAGE32`, and
  `DESSAGE32` execution family around CSBWin DSA.cpp's
  `QueueDSASwitchAction` boundary. Decoded message operands now remain a
  bounded pending request until the entire checksum-authenticated DSA action
  accepts; only then can the existing runtime queue owner publish its source
  timer event. A following unowned opcode leaves the queue unchanged. This
  retains the original M/D route, delay, action and target operands without
  inventing an event, target-room mapping, wrapper, or general interpreter.
  Verification: `csb_v1_dsa_pure_control_pc34_compat` and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA COPYTELEPORTER transaction (2026-07-17)

- Made source-proven `DSACMD_COPYTELEPORTER` and `COPYTELEPORTER32` atomic
  with the rest of an authenticated DSA action. The exact decoded source and
  destination location operands now remain pending until all later action
  words accept, then invoke only the existing runtime DB1/CELLFLAG copy
  owner. A later rejected or unowned opcode cannot leave a partial dungeon or
  save-runtime mutation. Missing owner data, unknown actions and unsupported
  opcode bodies remain closed; no teleporter, cell, link or event is created.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DB3 COPY runtime transaction (2026-07-17)

- Tightened source-proven `STKOP_Copy` (DSA.cpp:4696-4721) at the live
  runtime boundary. A new optional owner callback receives the staged source
  and destination Things together, re-resolves both as complete DB3 records,
  and requires the current source bytes to equal the staged six-byte image
  before writing the destination. The regular runtime path uses that callback
  inside its existing candidate Dungeon/save transaction, so source drift,
  wrong type, unavailable owner, or a later unknown opcode cannot publish a
  partial mutation. Pure stack consumers retain the existing staged fallback;
  no generic copy API, timer event, wrapper, or synthetic action was added.
  Verification: `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA GeneratorDelayStore owner transaction (2026-07-17)

- Tightened `STKOP_GeneratorDelayStore` (DSA.cpp:2876-2915) at its DB3
  runtime boundary. Its pending request now retains the source
  GeneratorDelay@ value, and the candidate runtime re-reads the same loaded
  type-6 generator or source-defined type-0 fallback before committing the
  byte-eight delay. A changed/missing owner or a following unknown opcode
  leaves the candidate and live Dungeon unchanged. This reuses the existing
  save/runtime candidate handoff only; it creates no generator, timer, queue
  event, wrapper or inferred opcode behavior. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA GeneratorDelayStore DB3 runtime receipt (2026-07-17)

- Extended the existing source-owned `STKOP_GeneratorDelayStore` transaction
  with an atomic DB3 receipt: selected location, original delay, committed
  delay, and whether CSBWin selected the type-6 generator instead of its
  defined type-0 fallback. This data publishes only after the existing
  re-read/compare commit succeeds. A saved dispatch retains its exact
  TimerQueue/TIMER scope through the pre-existing restored-timer receipt; this
  package adds no queue route, timer body, generator allocation, wrapper, or
  synthetic save behavior. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MissileInfoStore DB14/TIMER transaction (2026-07-17)

- Tightened `STKOP_MissileInfoStore` (DSA.cpp:2824-2846) around its paired
  DB14 and TIMER ownership. The action now retains the original four-value
  DB14/TIMER image before applying stack operands. Candidate commit re-reads
  that exact image, then uses the existing strict runtime path that confirms
  the serialized TIMER is valid and represented by exactly one active queue
  event before updating DB14 and TIMER direction together. Stale/mixed DB14,
  TIMER or queue evidence and later unknown actions remain non-mutating. No
  missile, timer, event, wrapper or inferred opcode behavior was added.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MissileInfoStore runtime receipt (2026-07-17)

- Extended the source-owned `STKOP_MissileInfoStore` receipt after its
  existing atomic DB14/TIMER commit. It now records the concrete DB14 Thing
  and all four source words before and after mutation; a saved dispatch adds
  the already strict unique TimerQueue/TIMER scope only after that receipt is
  complete. The regression locks the original DB14 range/damage/direction
  image, the committed image, and the fail-closed stale-owner behavior. No
  generic missile, queue, timer, or opcode interpreter route was introduced.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DB14/TIMER owner receipt (2026-07-17)

- Hardened `STKOP_MissileInfoStore` publication with its separate DB14-linked
  TIMER owner. The candidate must preserve the DB14 timer index and one
  identical serialized TimerQueue slot while revalidating TIMER function,
  time, and the raw position byte before/after direction mutation. The
  execution receipt now carries those facts alongside the existing DB14
  Thing and four-word images. Missing, duplicate, reordered, stale, or mixed
  owner data rejects before live Dungeon or receipt publication. This adds no
  timer body, missile route, queue synthesis, or fallback opcode behavior.
  Verification: `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA StoreExCellFlg EXPOOL receipt (2026-07-17)

- Extended the existing source-owned `STKOP_StoreExCellFlg` transaction with
  a full EXPOOL before/after receipt. The opcode now requires the current
  eight-word cell-flags owner before staging its source flag byte, and a
  committed action records the packed location and all eight words before and
  after replacement. Missing or stale EXPOOL data and later invalid opcode
  bodies reject before any save-tail publication. Saved TIMER/queue scope
  remains exclusively the existing restored-dispatch receipt; no allocator,
  timer, queue, or generic interpreter was added. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA StoreExCellFlg save-tail identity receipt (2026-07-17)

- Completed the runtime side of `STKOP_StoreExCellFlg` (CSBWin
  `DSA.cpp:3298-3328`). Its existing DB11/EXPOOL candidate receipt now
  includes the admitted tail FNV before and after the source eight-word
  replacement. The runtime adapter accepts only `set_extended_cell_flags ==
  1`, so the source API's negative failure result cannot be treated as a C
  truthy commit. A tail whose FNV has drifted rejects before action execution
  or runtime receipt publication. The common restored-dispatch path remains
  the only owner of saved TimerQueue/TIMER scope; no queue, timer, allocator,
  wrapper, or synthetic cell state was introduced. Verification:
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_runtime_skin_expool_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

- The common restored-dispatch handoff now also revalidates an EXPOOL-mutating
  execution receipt before it records or exposes saved TimerQueue/TIMER scope.
  Its post-write tail must still be valid, complete, and equal to both the
  declared and receipt FNV; drift clears the saved handoff and makes the
  execution and runtime-chain readers reject. This remains an identity gate,
  not a DB11 allocator, TIMER constructor, or opcode interpreter. Verification:
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, `csb_v1_dsa_localstate1_save_handoff`,
  and `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA TalentsStore wing save receipt (2026-07-17)

- Bound high-bit `STKOP_TalentsStore` (CSBWin `DSA.cpp:4291-4338`) to the
  existing `CHARDESC::SaveToWings` DB11/EXPOOL owner. A completed action now
  publishes its actual wing fingerprint, talents word before/after, and the
  complete tail FNV before/after only after the candidate re-reads the same
  eight `EDT_Character` records on both sides. A missing wing remains the
  source no-op; a later unsupported word leaves the candidate tail unpublished.
  The common receipt-currentness gate rejects tail drift before the execution
  receipt or restored TimerQueue/TIMER handoff can be consumed. No character
  creation, swap, DB11 allocation, generic save write, queue, or interpreter
  route was added. Verification: `csb_v1_wing_expool_runtime`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA TalentsStore party receipt (2026-07-17)

- Completed low-bit `STKOP_TalentsStore` publication from the actual imported
  CSBWin owner. The focused regression serializes the `DSA::Read` action
  table, validates its RCS, imports and selects the saved `(18, 1, 0, 0)`
  action in file order, then confirms that the source `AMPERSAND2` action
  updates the live party CHARDESC talents word. The execution receipt binds
  champion count, fingerprints, and talents before/after; any later talents
  drift makes it unavailable. This fixes only the lost context-to-runner
  party-talents return path. No synthetic DSA action, character/wing mutation,
  timer, queue, save allocation, or general interpreter route was added.
  Verification: `csb_v1_dsa_queued_localstate2_timer` PASS; isolated
  `firestaff` build and `git diff --check` PASS.

# CSBWin DSA FalsePit DB1 receipt (2026-07-17)

- Added a distinct runtime receipt for source `STKOP_FalsePit` (CSBWin
  `DSA.cpp:2859-2876`). It is admitted only after an RCS-authenticated imported
  DSA action selects one existing `roomPIT`, changes only CELLFLAG bit zero,
  and preserves the complete five-word DB1/CELLFLAG image before and after.
  Mixed cell writes, a non-pit state, room-type drift, and later CELLFLAG
  drift fail closed before a save/runtime handoff can consume the receipt. The
  implementation adds no pit traversal, timer, queue, renderer, or inferred
  cell semantics. Verification: `csb_v1_dsa_queued_localstate2_timer` PASS;
  isolated `firestaff` build and `git diff --check` PASS.

# CSBWin DSA ExperiencePlus CHARDESC receipt (2026-07-17)

- Extended the admitted `STKOP_ExperiencePlus`/`Magic.cpp::AddToSkill`
  candidate with a source-owned save receipt. It records the selected
  CHARDESC skill and its source basic-skill pair, increment, and both values
  before/after. Before publication the runtime rederives the exact UI16
  increment, cap, and no-LevelUp result from the unmodified party image and
  compares it to the candidate. Later selected/basic-skill drift makes the
  execution receipt and restored TimerQueue/TIMER handoff unavailable. The
  unimplemented LevelUp transaction, random/stat/UI work, generic XP writes,
  and inferred timer behavior remain closed. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA LevelUp prerequisite receipt (2026-07-17)

- Added a read-only, source-bound preflight for the existing
  `Magic.cpp::AddToSkill` LevelUp boundary. It publishes only when the exact
  proposed selected/basic CHARDESC skill pair crosses the unadjusted-mastery
  threshold, retaining UI16 increment, both values before/after, and mastery
  before/after. The receipt is current only while the source skill pair still
  equals its precondition. It deliberately executes no LevelUp code and
  commits no XP, random, stat, UI, save, or timer result. A future complete
  original LevelUp owner must consume this prerequisite atomically. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MonsterStore DB4 runtime receipt (2026-07-17)

- Bound the already source-owned `STKOP_MonsterStore` (`DSA.cpp:4075-4125`)
  commit to an atomic DB4 runtime/save receipt. Staging now preserves the
  complete eight-word group image before the first write. Before the cloned
  Dungeon is published, runtime re-reads that exact DB4 Thing on both sides
  and requires complete before/after equality, along with the original
  selected-field write mask. The published receipt expires if the live DB4
  image later drifts, which also closes the existing restored timer handoff.
  A later unsupported source word leaves DB4 and the receipt untouched. No
  group allocation, link traversal, timer or queue creation, generic opcode
  handling, or LevelUp RNG/stat/UI route was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA CellStore CELLFLAG/DB0/DB1 runtime receipt (2026-07-17)

- Bound source-owned `STKOP_CellStore` (`DSA.cpp:3837-3956`) to the existing
  byte-map and first matching DB0/DB1 record owner. The pending write keeps
  the complete five-word `CellFetch` image before its first mutation. Before
  cloned Dungeon bytes publish, runtime re-reads the same location from both
  Dungeon images and requires complete before/after equality plus the source
  write mask. The receipt becomes unavailable after drift in any field that
  the CellFetch owner exposes. A later unsupported opcode leaves CELLFLAG,
  DB0/DB1, and receipt untouched. No cell, record, Thing link, timer, queue,
  inferred room rule, or generic interpreter path was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA CopyTeleporter DB1/CELLFLAG runtime receipt (2026-07-17)

- Extended the existing `DSACMD_COPYTELEPORTER` source-owner transaction with
  complete five-word CellFetch images for source, destination-before and
  destination-after. Candidate publication re-reads all three images across
  the cloned Dungeon boundary; receipt currentness rejects later source or
  destination DB1/CELLFLAG drift. The focused fixture uses two actual
  byte-map teleporter squares, their source-first-Thing entries, and two DB1
  records. It proves atomic payload/CELLFLAG copy and destination drift
  rejection. No teleporter, record link, timer, queue, or generic cell route
  was added. Verification: `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA object-property runtime receipt (2026-07-17)

- Bound the existing CSBWin DSA object-property family (`SetCurse`,
  `SetBroken`, `SetCharges`, `SetPoisoned`, and `SetSubType`) to its raw
  DB5/DB6/DB8/DB10 owner at the staged commit boundary. The receipt retains
  Thing, source property kind, and normalized value before/after; it is
  current only while the same raw property still equals the recorded result.
  The DB5 fixture proves a real SetCharges commit and charge-field drift
  rejection. Unsupported Thing/property pairs and unknown opcode paths remain
  closed; no generic object mutation, item allocation, or timer route was
  added. Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA Random GAMEBLOCK2 runtime receipt (2026-07-17)

- Bound `STKOP_Random` to the existing authenticated CSBWin
  `GAMEBLOCK2.RandomNumber` owner with exact seed before/after in the runtime
  receipt. Receipt currentness requires the verified GAMEBLOCK2 summary and
  exact post-action seed, so later seed drift closes runtime/save consumption.
  The focused fixture verifies the source seed transition and drift rejection.
  No host RNG, timer, queue, or random-dependent opcode route was added.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DisableSaves GAMEBLOCK2 runtime receipt (2026-07-17)

- Bound `STKOP_DisableSaves` to the existing staged CSBWin save-policy owner
  with exact before/after state in the runtime receipt. Receipt currentness
  rejects later policy drift. The focused fixture proves the zero-to-one
  transition and drift rejection. No save operation, timer, queue, fallback
  policy, or broader GAMEBLOCK2 mutation was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DiscardText DB2/F0168 runtime receipt (2026-07-17)

- Bound `STKOP_DiscardText` to the existing TT_OPENROOM DB2/F0168 one-message
  receipt. The runtime receipt retains the admitted message identity before
  the authenticated clear and the exact cleared after-image; a replacement
  text receipt invalidates later runtime/save consumption. The focused fixture
  proves the clear and replacement-drift rejection. No text queue, log,
  renderer fallback, or generated text was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA SETSKIN EXPOOL receipt and cache admission (2026-07-17)

- Tightened `AMPERSAND2/SETSKIN` (CSBWin `DSA.cpp:3122-3135`) at the existing
  EXPOOL/DB11 candidate boundary. The opcode now requires a readable current
  `EDT_Skins` owner before staging and, after the complete authenticated
  action commits, records its packed location and exact skin byte before and
  after the write. Missing ownership and a later unsupported action leave the
  candidate, live save state, and receipt untouched. The existing restored
  dispatch remains solely responsible for appending saved TimerQueue/TIMER
  scope; no timer, queue, allocator, wrapper, or synthetic skin is added.
- The live HUD skin consumer now compares its saved-tail FNV/size receipt on
  every grid admission and clears cached columns on tail drift, truncation, or
  absence. It cannot present a prior save's EXPOOL bytes as a fallback.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_runtime_skin_expool_handoff`, `csb_v1_phase7_verification`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA ModifyMessage timer-scope receipt (2026-07-17)

- Bound `STKOP_ModifyMessage` (DSA.cpp:4931-4947) to the authenticated
  runtime execution receipt. Its source-clamped SET/CLEAR/TOGGLE triplet is
  now exposed only when the existing timer-scope runner has completed a
  verified action; callers cannot fabricate the values through a generic
  timer, queue, or save field. The triplet remains transient and does not
  mutate a TIMER or serialize a replacement save. Verification:
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA ModifyMessage saved-timer receipt (2026-07-17)

- Connected the existing `STKOP_ModifyMessage` effect to the real restored
  `ProcessTimers` handoff. Each admitted saved-timer dispatch starts with
  CSBWin's source 0/1/2 SET/CLEAR/TOGGLE map; a completed DSA action may
  publish its clamped map only alongside the revalidated queue slot, TIMER
  index, function, action, position, time, and authenticated DSA identity.
  Failed or stale queue input clears the transient execution receipt. This
  remains an observational runtime receipt: it creates neither a queue nor a
  serialized timer and does not infer any unknown opcode body. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin queued LocalState=1 save-transition receipt (2026-07-17)

- Aligned the LocalState=1 save handoff with the imported CSBWin DSA record:
  `m_state`, `m_localState`, `m_groupID`, and `m_stateCount` are four
  source words, not the obsolete two-byte state plus byte flags layout. The
  RCS-validated candidate now writes only the source `m_state` word and the
  execution receipt records its before/after state and post-write tail FNV.
  A queued restored timer adds its exact queue slot, TIMER index, time and
  authenticated DSA action identity to that receipt. Corrupt RCS or stale
  queue evidence publishes neither a state transition nor a receipt. No
  generic state machine, queue, or synthesized save was added. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin queued LocalState=0 DB3 state receipt (2026-07-17)

- Added the source-owned `DSA.cpp::PutState` LocalState=0 path. It writes
  only the selected type-47 DB3 actuator's `DSAstate` nibble in word2 after
  revalidating the admitted Extended Features RCS/FNV tail, selected action,
  and source actuator identity. The execution receipt identifies DB3 storage,
  state before/after, tail FNV, and the existing exact saved queue/TIMER
  scope. A corrupt tail rejects before either DB3 or receipt publication; no
  generic state, queue, or fallback action was introduced. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin queued LocalState=2 ParameterB receipt (2026-07-17)

- Completed the existing compact `DSA.cpp::PutState` LocalState=2 path with
  a source-bound runtime receipt. A committed type-47 DB3 `ParameterB` state
  transition now records its storage kind, state before/after, authenticated
  Extended Features tail FNV, and the exact restored TimerQueue/TIMER scope.
  The regression proves compact state one to two and proves a corrupted DSA
  RCS rejects before either DB3 or receipt publication. No widened ParameterB
  record, generic state machine, queue, or fallback opcode was added.
  Verification: `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# Nexus WARNING.BIN PP source-faithful execution (2026-07-17)

- Audited Sega Saturn/32X Graphic References ST-124-R1 section 6 and bound
  its PP contract to the canonical SHA-256-attested `WARNING.BIN` resource 0:
  a six-byte PP header, 256 big-endian BGR555 CLUT words, and one indexed byte
  per 240x96 image pixel. `nexus_v1_warning_dgt2_pp_execute()` rechecks every
  admitted source/prefix/body/trailing FNV and exact boundary, copies only the
  23,040 source index bytes and 256 unconverted BGR555 words to exact-size
  caller buffers, then requires an explicit presentation callback. Missing,
  mutated, undersized, or callback-rejected input publishes no execution
  receipt. The executor contains no default image, RGBA conversion, CLUT
  replacement, trailing-byte interpretation, or visual fallback. Verification:
  `nexus_v1_warning_dgt2_source_admission`,
  `nexus_v1_warning_dgt2_descriptor_admission`,
  `nexus_v1_warning_dgt2_pp_payload_admission`, and
  `nexus_v1_warning_dgt2_pp_execution` PASS against the local canonical file;
  isolated `firestaff` build PASS.

# Nexus WARNING.BIN PP M11 presentation (2026-07-17)

- Connected only canonical `WARNING.BIN` resource 0 to M11's real 320x200
  indexed presentation surface. Every warning-frame route rechecks the
  engine's exact source identity, reopens the original bytes, and rebuilds the
  source/descriptor/payload/execution receipt before writing the 240x96 index
  plane. The host palette is built only from the source's 256 BGR555 words in
  ST-124 section-6 order, `B4..B0/G4..G0/R4..R0`, using exact 5-to-6 bit
  replication for M11's RGB6 palette API. Body drift, a noncanonical source,
  an incorrect host surface size, or any receipt failure leaves the cleared
  frame untouched; the former title/UI-surface warning alternatives are not
  used. No synthetic texture, title/menu image, colour substitution, decoder,
  or visual fallback was added. Verification:
  `nexus_v1_warning_dgt2_source_admission`,
  `nexus_v1_warning_dgt2_descriptor_admission`,
  `nexus_v1_warning_dgt2_pp_payload_admission`,
  `nexus_v1_warning_dgt2_pp_execution`, and
  `nexus_v1_warning_dgt2_m11_presentation` PASS against the local canonical
  file; isolated `firestaff` build PASS.

# Nexus Structure1F direct directory admission (2026-07-17)

- Added `nexus_v1_structure1f_directory_admit()` on top of the existing DGN
  parser and direct LEV corpus identity route. It rehashes each canonical
  direct LEV00--LEV15 file, rejects source/package drift, and retains only the
  final Structure1F directory span plus its six parser-observed count-derived
  record spans, fixed record sizes, source tags and raw FNV witnesses. A
  mutated directory byte, changed size, out-of-range span or cross-level
  identity produces no receipt. The result is explicitly no-draw and grants no
  face, mesh, transform, material, texture, palette or host-placement meaning.
  Verification: `nexus_v1_structure1f_provenance` and
  `nexus_v1_structure1f_directory_admission` PASS against the local 16-level
  direct corpus; isolated `firestaff` build PASS.

# Nexus Structure1F first-family typed record admission (2026-07-17)

- Added a strict admission for the first parser-observed Structure1F family
  only: tag `0x10`, eight-byte rows, and the documented byte-ordered `x` and
  `y` fields at offsets 1 and 2. It retains the arithmetic `y * 64 + x` cell
  ordinal and copies bytes 3--7 as opaque data. The admission rehashes the
  source package, requires the existing direct-corpus directory identity and
  exact family span/FNV, and rejects cross-level identity reuse, altered row
  tags, and out-of-range row indices. It grants no item behavior, face, mesh,
  transform, material, texture, palette, decoder, or draw meaning.
  Verification: `nexus_v1_structure1f_directory_admission` and
  `nexus_v1_structure1f_item_admission` PASS against the local LEV00--LEV15
  corpus; isolated `firestaff` build PASS.
# Nexus Structure3 entry framing admission (2026-07-17)

- Added `nexus_v1_structure3_entry_admit()` above the existing direct-source
  Structure3 target receipt. It requires the rehashed ordinary file, level,
  package FNV, target FNV and 40-byte header to agree before retaining the raw
  tag, two counts, three contiguous count-bounded 12-byte spans and their FNV
  witnesses. Header-boundary, target, package or source drift rejects with no
  receipt. The admission remains explicitly no-draw and grants no geometry,
  normal, material, texture, transform, palette, pixel or rendering meaning.
  The fixture covers positive framing, target drift and a malformed boundary
  after the file identity is refreshed. Verification:
  `nexus_v1_structure3_target_admission_fixture` and
  `nexus_v1_structure3_entry_admission_fixture` PASS; full
  `build-nexus-codex` build PASS.

# Nexus VDP2 writer candidate receipt (2026-08-07)

- ✅ Added `scripts/analyze_nexus_vdp2_writer_candidates.py` for the
  authenticated Mednafen VDP2 code-window trace. It verifies the trace header,
  the hash-verified `TM.BIN`/`DM.BIN` inputs, and reports aligned longest
  partial matches without promoting a speculative source owner.
- ✅ The primary runtime writer window remains evidence-only: four words in
  `DM.BIN` and three in `TM.BIN`; the longest ten-word candidate is shared by
  both files at another PC. Tilemap/CLUT ownership and menu/HUD/viewport
  composition remain blocked.

# Nexus VDP2 layer register receipt (2026-08-07)

- ✅ Added `scripts/analyze_nexus_vdp2_composition.py`, using the raw
  authenticated Saturn witness rather than synthetic register fixtures.
- ✅ The European one- and eight-frame captures independently report display
  enabled and only `NBG1` enabled. The decoded `CHCTLA` proves bitmap mode,
  colour code `1`, bitmap-size code `0` and bitmap palette `0`; `PNCN1` and
  `MPOFN` are not treated as active tilemap selectors for this frame. The
  retail bitmap/CLUT consumer and host composition remain blocked.

# Nexus NBG1 bitmap source join (2026-08-07)

- ✅ Added `scripts/analyze_nexus_vdp2_bitmap_source.py`. It verifies the
  authentic capture and retail hashes, decodes DMWeb PRS3 output, extracts the
  real FONT256 character-generator tiles, and compares them in the active
  NBG1 bitmap byte domain.
- ✅ Both real samples report a 131072-byte span at VRAM `0x000000`, 162
  decoded MENU.BPK surfaces plus 242 FONT256 tiles examined, and zero
  non-zero exact matches. The result remains source-unbound and no-draw;
  title/STABG/dungeon bitmap and CLUT joins remain open.

# Nexus title bitmap/CRAM negative join (2026-08-07)

- ✅ Extended the authentic NBG1 comparator with hash-verified DMWeb
  `TITLE.BIN` MAPD/TIBG records, all five 64×28 title maps, and the real
  `TITLE.CG` 4bpp character generator. The title palette is compared against
  captured VDP2 CRAM in both byte orders.
- ✅ The one-frame and frame-7 eight-frame European captures examine 409
  bounded retail sources, report zero non-zero exact bitmap matches, and report
  no title-palette CRAM match. This is negative source-join evidence only;
  STABG/dungeon/CLUT ownership and host composition remain blocked.

# Nexus second VDP1 source-span capture (2026-08-07)

- ✅ Ran and validated an additional external E-BIOS/French START+A capture
  (eight active frames; raw SHA-256 `d648bd88…`). Every frame has active VDP1
  state and the same bounded type-2 command shape. The later source span is
  16bpp, 33280 bytes at VDP1 VRAM `0x63e00`, with observed hashes
  `5cca9793…` and `58afb9c9…`.
- ✅ An exact-byte scan against the local Nexus data directory finds no file
  owner. VDP2 registers/VRAM/CRAM are unchanged, so this is negative VDP1
  provenance only; no DGN/MNS/ITEM/HUD/viewport route was opened.

# Nexus STABG bitmap/CRAM negative join (2026-08-07)

- ✅ Extended the comparator with the hash-verified DMWeb STMP decode for the
  first 40×21 `STABG.BIN` map: tile cells, horizontal flips, 791 8×8 indexed
  tiles, and the 256-entry palette are retained as authentic source bytes.
- ✅ Both European NBG1 captures examine 410 bounded sources, report zero
  non-zero exact bitmap matches, and report no STABG palette match in CRAM in
  either byte order. The result remains source-unbound and no-draw; dungeon
  bitmap, CLUT ownership and final HUD composition remain open.

# Nexus MENU palette/CRAM negative join (2026-08-07)

- ✅ Added the authenticated 256-entry `MENU.BPK` PALT payload to the same
  VDP2 CRAM comparison, preserving its raw BE16 bytes and a word-swapped
  comparison form.
- ✅ The real one-frame sample reports no MENU palette match in either byte
  order; the NBG1 source join remains unbound and semantic admission blocked.

# 2026-08-07 Nexus VDP1 PC trace negative receipt
- ✅ Added a reproducible operator-only Mednafen patch and analyzer for
  SH-2-PC-addressed VDP1 VRAM writes. The European source-span probe reached
  the live window but resolved only colour/framebuffer fills at
  `0x06026260`/`0x06026270`; no DGN texture owner was admitted. Production
  menu/HUD/viewport composition remains capture-gated.

# 2026-08-07 Nexus SCSP/68K runtime corridor receipt
- ✅ Added external SCSP mailbox tracing with main SH-2 and sound-CPU 68K PCs.
  The real European gameplay run records `0x06001652 -> 0x100400 = 0x02`,
  followed by nonzero 68K writes from PCs that resolve inside the authenticated
  `SDDRVS.TSK` image at load base `0x1000`. SLEV/MAP/SAL meaning and host
  playback remain explicitly blocked.

# 2026-08-07 Nexus capture scan scheduling
- ✅ Added the operator-only `FIRESTAFF_NEXUS_NO_WAITING=1` Mednafen producer
  flag and launcher propagation. The external build compiles and installs with
  the flag; it changes scheduler waiting only and never promotes raw VDP/SCSP
  bytes to production presentation or playback.

# 2026-08-07 Nexus SDDRVS handler join
- ✅ Added a source-window receipt for the authentic SCSP trace PC `0x3224`.
  It verifies the corresponding `SDDRVS.TSK` command-byte/driver-state/SCSP
  register-family handler at `+0x2220`; SLEV selector, MAP/SAL identity and
  host playback remain explicitly blocked.

# 2026-08-07 Nexus active VDP1 raw witness

- ✅ Captured the European retail Nexus image with the instrumented external
  Mednafen producer at the late startup window, using the source-confirmed
  active-low `A+START` mask (`0x30`). The one-frame raw artifact is
  `c1ec48ac1b55c05ef573225b2820e3051fa2735968d286ea3b58ec9984da2712` and
  validates at 1,577,645 bytes with `PTMR=02`, `EDSR=03`, `COPR=00000c`.
- ✅ Added `--require-vdp1-activity` to the raw validator. It proves only a
  non-idle VDP1 state plus nonzero captured VRAM/framebuffer payload; it keeps
  PRS3, menu, HUD, viewport-owner, CLUT, DGN and SLEV/SAL semantic admission
  blocked. The external framebuffer witness is retained on the external disk.
- ✅ Extended the external witness to two gameplay frames. Both contain the
  same real VDP1 systemclip/local-coordinate/type-2 textured command/END
  window (`PMOD=0x0028`, `SRCa=0xc7c0`, `SIZE=0x28b4`), while VDP1 VRAM,
  framebuffer 1 and the draw-buffer selector change. The raw artifact is
  `549e03856163899381d4b6a03f65ef989fadbeccb338579eb87876e00f30e362`;
  the command source still lacks an authenticated DGN/ITEM/MNS join.

# Nexus MENU.BPK retail PRS3 decode census (2026-08-07)

- ✅ `test_nexus_v1_bpk_archive` now loads the explicit
  `FIRESTAFF_NEXUS_DATA_DIR` corpus and decodes all 162 retail PRS3 menu
  surfaces, checking that every output has the declared `width × height` and
  that no retail stream fails. This proves the DMWeb-bounded byte decoder on
  real MENU.BPK data; it does not promote pixels to Saturn VDP1/VDP2 upload.

# Nexus English FONT256 opaque subrecord receipt (2026-08-07)

- ✅ The real European/English `FONT256.S2D` (`25,012` bytes, SHA-256
  `764a2d6c…`, source FNV-1a64 `0x90c4ce611bd5f5fe`) no longer skips the
  subrecord grammar test merely because its opaque section-2 composition
  differs from the canonical corpus.
- ✅ Its measured section-2 profile is bound separately: `857` populated
  16-byte blocks in `68` runs, with byte counts `0x00=8890`, `0x03=3498`,
  `0x0f=3100`, `0xff=16`. Section 0/4/6 receipts remain shared and all
  values stay opaque source measurements.
- ✅ Synthetic canonical regression and the real English test both pass.
  This removes a false skip only; glyph mapping, pixel decode, VDP2 text
  placement and production drawing remain capture-gated.

# Nexus Saturn raw witness region analysis (2026-08-07)

- ✅ Added `scripts/analyze_nexus_saturn_runtime_capture.py`, which validates
  the existing raw capture layout, prints SHA-256 identities for each VDP1 and
  VDP2 region, and supports an explicit adjacent-frame change requirement.
- ✅ The real four-frame French/E-region startup witness changes VDP1 FB0,
  VDP1 VRAM and the draw-buffer selector. The sample's VDP2 register/VRAM/CRAM
  regions remain unchanged. The tool keeps semantic admission blocked: this
  is runtime observation only, not menu/HUD/viewport or CLUT ownership proof.

# Nexus Saturn VDP1 state witness (2026-08-07)

- ✅ Extended the external capture patch to VDP1 raw format V2 and rebuilt the
  Saturn-enabled Mednafen binary. The real E-region two-frame witness records
  `PTMR=02`, `EDSR=03`, `LOPR=0008`, `COPR=000008`, `RET=ffffffff`, and a
  framebuffer selector change from `1` to `0`.
- ✅ V1 raw witnesses remain accepted by the validator. V2 state is evidence
  only; no active command-list source, CLUT, placement or production draw route
  is inferred.

# Nexus Saturn VDP1 command-window receipt (2026-08-07)

- ✅ Added `scripts/analyze_nexus_vdp1_command_window.py`. Against the real
  E-region two-frame witness, `COPR=8` maps to VDP1 VRAM offset `0x40`, whose
  observed record is END; preceding bounded records at `0x00` and `0x20` have
  control types `0x09` and `0x0A`.
- ✅ This remains a raw hardware-state receipt only. No MENU.BPK/DGN owner,
  CLUT, destination or production draw route is inferred.

# Nexus Saturn authentic startup draw-command receipt (2026-08-07)

- ✅ A later real E-region frame now verifies one VDP1 distorted-sprite command
  at `0x0040` (`PMOD=0x0028`, `SRCa=0xc7c0`, `SIZE=0x28b4`) followed by END at
  `0x0060`. The command-window analyzer now computes the bounded source VRAM
  span and SHA-256 for this record.
- ✅ This remains a raw source-join target. No TITLE.CG, MENU.BPK, PRS3,
  STABG, FACE or DGN ownership is inferred, and no production draw route is
  enabled.
- ✅ The captured source span is retained as a negative join receipt:
  SHA-256 `0a87c97db9dcaf9e74df11cb85b35084edc0e37daa74e6012ce1fc131a2d5575`,
  with no exact-file or first-32-byte match against the local retail
  `TM.BIN`, `TITLE.CG`, `TITLE.BIN` or `STABG.BIN` files. It remains authentic
  runtime evidence, not a synthetic replacement.

# Nexus TM.BIN VDP owner disassembly receipt (2026-08-07)

- ✅ Added `scripts/analyze_nexus_tm_bin_vdp_owner.py`. Against the real
  `TM.BIN` (`160044` bytes, SHA-256 `d87485fe…`), it finds the complete
  observed VDP1 literal set (`0x25d00000`, `02`, `06`, `08`, `0a`, `10`) and
  VDP2 register literals through SH-2 PC-relative loads.
- ✅ This is static code-owner evidence only. It does not promote the runtime
  VDP1 draw command, source span, CLUT or startup asset into production.

# Nexus SLEV SH-2 static owner receipt (2026-08-07)

- ✅ Added `scripts/analyze_nexus_slev_sh2_owner.py`. It verifies all 16 real
  `SLEV##.BIN` SHA-256 values and scans their big-endian SH-2 words. The
  corpus is 111,776 bytes with shared entry word `0x2fe6`, 1,271 `RTS`, 2,220
  `JSR`, 5,164 immediate, 948 branch and 3,536 PC-relative-load observations.
- ✅ Eight exact PC-relative literal rows in `SLEV02`, `SLEV03`, `SLEV11` and
  `SLEV15` reach the observed `0x25/0x26` address corridors; the tool prints
  each instruction/literal offset and value. No event selector, callback ABI,
  SDDRVS handoff or runtime dispatch is inferred; semantic admission stays
  blocked.

# Nexus SDDRVS 68k disassembly receipt (2026-08-06)

# Nexus spell-table production boundary (2026-08-06)

- ✅ Removed the inferred `nexus_v1_magic.c` spell/mana implementation from
  the production archive. It remains available to explicit source-study
  tests, while production links a fail-closed adapter returning no spell,
  no mana preview and no damage.
- ✅ This prevents authenticated table bytes from being mistaken for a
  captured Saturn spell dispatcher or SLEV/SFX effect consumer.

# Nexus combat production boundary (2026-08-06)

- ✅ Removed the DM1-shaped combat/RNG/wound/XP implementation from the
  production archive. It remains available to explicit formula-study tests;
  production now links a state-preserving fail-closed adapter.
- ✅ Added a production-boundary regression proving attack, damage, RNG and
  XP calls cannot mutate champion or creature state without a Saturn action
  and writeback capture.

# Nexus rest/status production boundary (2026-08-06)

- ✅ Removed the DM1-shaped rest regeneration and poison/status mutation
  implementations from `firestaff_nexus`. Their original source remains in
  the explicit fixture library; production now links a state-preserving
  fail-closed adapter.
- ✅ Added a production-boundary regression proving rest timers, status
  effects and champion state cannot mutate before Saturn action/timing/HUD
  consumers are captured.

# Nexus action/world production boundary (2026-08-06)

- ✅ Removed the DM1-shaped action-timer, door, trap and projectile state
  machines from `firestaff_nexus`. Their original implementations remain in
  explicit study targets; production now rejects inferred admission and
  writeback through a fail-closed ABI adapter.
- ✅ Added a production-boundary regression proving those four routes do not
  create or advance state without the captured Saturn command, transition,
  trigger or projectile-DMA consumers.

# Nexus light production boundary (2026-08-06)

- ✅ Removed the DM1-shaped torch/FUL/ambient light state machine from
  `firestaff_nexus`. Its original implementation remains in the explicit
  study target; production now links a state-preserving fail-closed adapter.
- ✅ Added a production-boundary regression proving light setters, torch/FUL
  activation and ticking cannot mutate state before the Saturn light command,
  timer writeback and HUD/VDP consumers are captured.

# Nexus light-overflow M11 boundary (2026-08-06)

- ✅ Closed the remaining M11 init, launcher-handoff and tick calls into the
  data-free light-overflow host timeline while the Saturn action gate is
  closed. A retail Nexus session now keeps that runtime absent rather than
  advancing inferred F0238/F0257 state.
- ✅ Kept the standalone overflow/save model available only to explicit
  diagnostic probes; no Saturn execution or save ownership is implied.

# Nexus experience production boundary (2026-08-06)

- ✅ Removed the DM.BIN-shaped XP award, level-up and class-table implementation
  from the production archive. It remains available to explicit source-study
  tests; production now links a state-preserving fail-closed adapter.
- ✅ Added a production-boundary regression proving XP, level-up and stat/
  skill queries cannot expose or mutate inferred runtime state before the
  Saturn actor-death and champion writeback consumers are captured.

# Nexus startup VDP2 source-reference receipt (2026-08-06)

- ✅ The authenticated European `DM.BIN` startup/menu source regression now
  also verifies `yam\\vdp2.c` at `0x38CF4` and the six exact source-address
  literal slots at `0x28098`, `0x28640`, `0x28778`, `0x2887C`, `0x289E0` and
  `0x28E1C`.
- ✅ The same receipt now verifies the nine retail SH-2 PC-relative `MOV.L`
  loads at `0x27FE6`, `0x28002`, `0x285C6`, `0x28710`, `0x287AA`, `0x2880A`,
  `0x2885A`, `0x288B2` and `0x28D76` that reach those literal slots.
- ✅ This extends byte-level source ownership evidence only; VDP2
  register/VRAM writes, tilemap/CLUT placement and runtime presentation remain
  capture-gated.

# Nexus V2 production placeholder boundary (2026-08-06)

- ✅ Removed procedural V2 lighting, smooth-movement and touch/controller
  runtime sources from `firestaff_nexus`; the original implementations remain
  available only to explicit probes.
- ✅ Added fail-closed production adapters and a CTest boundary proving that
  none of these routes exposes active state, queue translations, ticks, or a
  synthetic viewport/HUD presentation path.

# Nexus startup/menu SH-2 source corridor (2026-08-06)

- ✅ Extended the authenticated European `DM.BIN` receipt to validate the
  startup/menu routine's SH-2 prologue/return envelope and exact PC-relative
  literal targets for `MENU.BPK`, `STABG.BIN` and the retained hardware
  literal.
- ✅ Kept the result source-ownership evidence only: no menu placement,
  FONT256 consumer, VDP2 register write, HUD composition or viewport pixel
  was promoted without Saturn capture.
- ✅ Real-data `test_nexus_v1_startup_menu_source` passes.

# Nexus Saturn capture launcher executability (2026-08-06)

- ✅ Marked all five Nexus Mednafen capture launchers executable so the
  operator-only capture workflow can be invoked directly.
- ✅ Real European BIOS/CUE and MENU.BPK/DM.BIN/LEV00.DGN identity preflight
  passes; stock Mednafen still exits 78 before writing a manifest because the
  Firestaff capture hook is absent.
- ✅ No synthetic trace or presentation evidence was admitted.
# Nexus TITLE.BIN/TITLE.CG real map join (2026-08-06)

- ✅ Extended the real TITLE.BIN MAPD/TIBG regression to load the authenticated
  TITLE.CG atlas and decode all five 64x28 source maps.
- ✅ The regression checks every map is populated and has non-empty source
  pixels, while explicitly retaining the no-presentation boundary.
- ✅ No VDP2 tilemap, CLUT, timing, or framebuffer permission is inferred from
  this format-only join.

# Nexus English FONT256.S2D revision admission (2026-08-06)

- ✅ Admitted the documented English Saturn FONT256.S2D SHA-256 revision for
  common SCR framing, section-chain and source-tile receipts.
- ✅ Kept the canonical-only subrecord grammar gate because the English
  revision's byte counts differ; it remains explicitly skipped rather than
  being forced through canonical statistics.
- ✅ No text glyph mapping, VDP2 placement or framebuffer permission changed.

# Nexus startup PLRD animation quarantine (2026-08-06)

- ✅ Removed the host-side 12-frame cursor blink and guessed palette metadata
  from authenticated European PLRD render rows.
- ✅ Kept the legacy blink/colors only in the isolated ASCII compatibility
  fixture lane used by unit tests.
- ✅ Real PLRD rows now expose only verified layout and TABL/FONT256 glyph
  payload until the Saturn VDP2 cursor consumer and original frame timing are
  captured.
- ✅ `test_nexus_v1_champion_plrd` covers the no-synthetic-animation contract.

# Nexus champion menu text quarantine (2026-08-06)

- ✅ Champion startup presentation now mirrors the save route: its title,
  subtitle, row labels and footer remain bounded `DRAW_NONE` capture slots.
- ✅ No host English `DRAW_TEXT` command can reach the startup command package;
  real PLRD/TABL/FONT256 text remains closed until the Saturn VDP2 consumer and
  placement are captured.
- ✅ `test_nexus_v1_startup_menu_pc34_compat` asserts the no-text-command
  boundary while retaining the 20 real PLRD/portrait source route.

# Nexus Structure3 VDP1 permission quarantine (2026-08-06)

- ✅ The geometry-only Structure3 scene planner no longer treats a bare
  `vdp1_consumer_evidence_available` boolean as presentation evidence.
- ✅ Texture submission stays blocked until a complete Saturn VDP1
  trace/CLUT/VRAM/owner receipt is bound; the real LEV01 Structure1F→Structure3
  plan remains geometry-only and no-draw.
- ✅ `test_nexus_v1_dgn_scene_runtime_plan` now sets the legacy flag and proves
  that texture and raster submission remain blocked.

# Nexus generic loot-drop quarantine (2026-08-06)

- ✅ Caller-supplied `nexus_floor_drop()` no longer creates a live floor item.
- ✅ Real floor records continue to enter only through
  `nexus_floor_drop_source()` from authenticated DGN Structure1Fa data, with
  raw attributes and source entry ordinal retained.
- ✅ Inventory and click-route fixtures now call the explicit source-admission
  lane and verify the generic drop mutator remains blocked.

# Nexus SLEV selector quarantine (2026-08-06)

- ✅ `nexus_sound_set_event_selector()` no longer accepts a host-supplied MAP
  selector while Saturn event-dispatch provenance is absent.
- ✅ The 16-level SAL/MAP metadata route remains available for opaque source
  receipts, but neither selector diagnostics nor playback can be promoted by
  a numeric caller hint.
- ✅ `test_nexus_v1_sound_gameplay` covers the inert setter contract.

# Nexus descriptor-0008 fixture provenance (2026-08-06)

- ✅ Corrected the DGN readiness fixture so a special ITEM.IBS floor image
  carries the production binder's `0xFF` regular-palette/image sentinels.
  This removes one false failure without authorizing decode or VDP1 drawing.
- ✅ The real-data probe still reports the independent open Structure3,
  Structure1C and Saturn VDP1/viewport capture gaps; those remain blocked.

# Nexus MENU.BPK raw PALT source lane (2026-08-06)

- ✅ Added a bounded API that copies all 256 real PALT entries as
  big-endian 16-bit source words and verifies the European trailer payload
  FNV-1a64 `0ec4e98ca3a18f85`.
- ✅ The API intentionally does not label the words BGR555/RGB555 or assign a
  CLUT bank; colour conversion and VDP1 ownership still require source-backed
  Saturn capture.

# Nexus MENU.BPK bounded surface decode regression (2026-08-06)

- ✅ `test_nexus_v1_bppk` now reads the real European `MENU.BPK` through
  `FIRESTAFF_NEXUS_DATA_DIR` and decodes all 162 PRS3 surfaces via the
  bounded public surface decoder. Every output has the declared indexed-8bpp
  size and non-zero source pixels; the directory reports 164 entries/162
  PRS3 entries.
- ✅ The test remains source/pixel evidence only. CLUT ownership, VDP1 upload,
  destination placement and menu presentation remain capture-gated.

# Nexus startup real-corpus gate audit (2026-08-06)

- ✅ `test_m11_nexus_startup_gate`, `test_nexus_v1_startup_menu_pc34_compat`
  och `test_nexus_v1_startup_media_gate` passerar mot
  `/Users/bosse/.firestaff/data/nexus`, inklusive TITLE, WARNING, GAMEOVER,
  STABG, LOGOBG och alla 20 FACE-ytor.
- ✅ Källbundna ytor laddas endast som receipts/source surfaces; FONT256
  glyph mapping, Saturn VDP1/VDP2-placering och startup-animationens
  presentation är fortsatt capture-gated. Ingen hårdkodad timing öppnar
  rendering.

# Nexus real MENU.BPK PRS3 decode audit (2026-08-06)

- ✅ De verkliga `MENU.BPK`-proberna passerar för den europeiska korpusen:
  162 PRS3-ytor, 8-bitars indexed output och korrekta deklarerade
  pixel-/payloadstorlekar. `test_nexus_v1_prs3_decode`, decoder-admission,
  loader-control-flow och SH-2-subset-proven passerar.
- ✅ DMWebs bytegrammatik används i avkodaren och ogiltiga framtida
  backreferenser avvisas fail-closed. Ingen CLUT-bindning, VDP1-upload,
  skärmplacering eller menyroute har öppnats utan Saturn-capture.

# Nexus SAL provenance real-corpus regression (2026-08-06)

- ✅ `test_nexus_v1_sal_container_provenance` now admits all 16 European
  `SNDLEV00.SAL` through `SNDLEV15.SAL` files through their exact direct
  identities and verifies the retail `dsp01.EX` header plus opaque descriptor
  interval. The test explicitly keeps codec proof and playback disabled.
- ✅ Existing MAP provenance and audio receipt checks remain unchanged; no
  selector meaning, SAL codec, CD-DA handoff or SFX playback was inferred.

# Nexus launcher placeholder removal and media-gate ordering (2026-08-06)

- ✅ Removed the last procedural Nexus card fallback (Saturn ring, obelisk,
  stairs and runes) from the M12 launcher. With no authenticated title
  framebuffer, the card now remains a neutral source-lock panel labelled
  `CAPTURE LOCKED`.
- ✅ Moved missing-data handling ahead of runtime/presentation readiness so a
  missing Nexus ISO/BIN/CUE produces the actionable `Dungeon Master Nexus`
  recovery popup instead of a misleading presentation error.
- ✅ Extended `verify_nexus_production_source_boundary.py` to reject the old
  `NEXUS ART` label and require the explicit capture-locked branch. The
  missing-media regression, `firestaff` build, verifier and real Nexus data
  scan pass.

# Nexus HUD depth provenance (2026-08-06)

- ✅ The M11 Nexus HUD handoff no longer supplies a synthetic maximum depth of
  15; diagnostics now use source-defined `NEXUS_MAX_LEVELS` (16). Production
  HUD pixels remain no-draw until Saturn widget placement and VDP1/VDP2
  ownership are captured.

- ✅ Nexus level-MD5 lookup, level loading and current-level validation now
  share the source-defined `NEXUS_MAX_LEVELS` bound instead of duplicated
  literal limits.

# Nexus capture launcher collision guard (2026-08-06)

- ✅ The generic Saturn capture launcher now rejects identical trace and
  manifest output paths before creating the manifest, preventing one artifact
  from overwriting the other.

# Nexus startup animation capture gate (2026-08-06)

- ✅ Startup animation readiness now has a separate
  `saturn_presentation_capture_bound` requirement. Correct timing, real
  package assets and capture frame numbers are insufficient without an
  original Saturn VDP1/VDP2 presentation frame bound to the route.
- ✅ Added and registered
  `test_nexus_v1_startup_presentation_animation_receipt`; it verifies that
  source assets plus timing remain no-draw until that binding exists.
- ✅ `firestaff`, the startup-menu regressions, the M11 startup gate and the
  production source-boundary verifier pass. This does not claim the animation
  capture itself is complete.

# Nexus English/French ISO provenance profiles (2026-08-06)

- ✅ Added exact container-hash profiles for the English fan-translation v2
  ISO (`cf158b32f342c168fc570d36a0f1c637`) and French fan-translation ISO
  (`2efb0e8c41f01dea3faa41328ce87f46`) found in the real Nexus data root.
- ✅ The scanner now prefers those container identities before the shared
  inner `DM.BIN` hash and suppresses the competing Japanese label when the
  virtual marker came from one of those ISOs. A standalone authenticated
  `DM.BIN` remains classified as Japanese extracted media.
- ✅ This follows DMWeb's separate Japanese-retail/English-fan/French-fan
  media classification; it does not claim fan translations are original
  Saturn releases or open a runtime presentation path.

# Nexus ITEM.IBS and TITLE.CG host-render quarantine (2026-08-06)

- ✅ Removed the standalone ITEM.IBS and TITLE.CG RGBA writers from the retail
  Nexus library. They accepted source bytes plus caller palettes but had no
  authenticated Saturn VDP1/VDP2 command, CLUT or placement owner.
- ✅ Added both decoders explicitly to their receipt tests and strengthened
  the production boundary verifier. Real ITEM.IBS/TITLE.CG data remains
  available as source evidence; no host item/title pixels are promoted.

# Nexus CPU rasterizer production quarantine (2026-08-06)

- ✅ Removed the textured host CPU rasterizer from `firestaff_nexus`; the
  production viewport now links a lifecycle-safe no-op adapter that clears and
  retains receipts but emits no DGN/MNS pixels.
- ✅ Kept the real rasterizer only in the explicit material fixture target and
  strengthened the source-boundary verifier. Retail DGN/MNS presentation
  remains gated on Saturn VDP1 command, CLUT/VRAM and owner capture.

# Nexus FONT256 host-draw seam quarantine (2026-08-06)

- ✅ Removed `nexus_v1_saturn_font.c` from the retail `firestaff_nexus`
  library. Its indexed glyph writer was a host-framebuffer fixture, not a
  captured Saturn page/tilemap/attribute/VDP2 consumer.
- ✅ Added the source explicitly to deterministic parser/render probes and
  corrected the API comments. Real FONT256 source receipts remain available;
  production startup/HUD text stays capture-gated.

# Nexus Phase 4 rendering-document fidelity correction (2026-08-06)

- ✅ Corrected the source-lock rendering document so it no longer describes
  gray/zero-padded UI fallback or completed Nexus presentation.
- ✅ It now matches the implementation: invalid/short surfaces reject, source
  pixels remain receipt-only, and VDP1/VDP2 plus DGN/MNS presentation stays
  capture-gated with no generated fallback.

# Nexus launcher synthetic card-art quarantine (2026-08-06)

- ✅ Removed the compiled procedural Nexus card from the M12 startup and
  missing-media views. Nexus now reports `SATURN TITLE SOURCE (CAPTURE
  LOCKED)` and stays image-less unless a caller supplies an actual card file.
- ✅ The public generated-card lookup also rejects `nexus`, so no other M12
  caller can resurrect that legacy procedural bitmap accidentally.
- ✅ This keeps authentic `TITLE.CG`/`TITLE.BIN` available as source media
  without pretending their Saturn VDP2 tile-map/CLUT placement is a launcher
  framebuffer. `m12_nexus_missing_media_popup_gate`, the Nexus startup-menu
  regression and the startup-media gate pass; no runtime Nexus presentation
  route was opened.

# Nexus MENU.BPK revision gate (2026-08-06)

- ✅ The PRS3 loader-media probe no longer hardcodes the Japanese 89,060-byte
  `MENU.BPK` revision. It now admits the documented Japanese, English
  (`a6f2272a4f6cb3c6b3b33012bc5b15ed`, 87,684 bytes) and French
  (`fcf8a00fbb92593ed9ae908f8e285cda`, 87,820 bytes) retail identities.
- ✅ With `/Users/bosse/.firestaff/data/nexus`, the full 163-test Nexus matrix
  passes; capture-dependent tests remain explicitly skipped. This changes
  verification coverage only: PRS3 decoding, CLUT/VDP1 ownership and
  presentation remain capture-gated.
# Nexus European TITLE.BIN revision admission (2026-08-06)

- ✅ The documented English Saturn ISO `TITLE.BIN` identity is now admitted
  alongside the canonical capture revision: SHA-256
  `a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44`
  (`MD5 0b293be24d06eb550b27442ac9e8924c`, 112,216 bytes). Unknown hashes
  remain rejected.
- ✅ Real English `RES*`, DGT2, MAPD/TIBG and CNFD admissions now pass. The
  English TITL receipt records its real prefix split (records 0, 2 and 3
  share the 512-byte prefix; record 1 differs) and plane non-zero census
  `15187/410/1572/885`; it is not conflated with the canonical profile.
- ✅ Verification with `/Users/bosse/.firestaff/data/nexus`: five TITLE.BIN
  real-data admissions, `nexus_v1_title_mapd_real` and
  `nexus_v1_startup_media_gate` all pass. No VDP2 placement or host draw route
  was opened.

# Nexus retail SLEV corpus receipt (2026-08-06)

- ✅ Added `nexus_v1_slev_task_corpus_receipt`, which rehashes and reads all
  16 authenticated `SLEV00.BIN`–`SLEV15.BIN` files from the real Nexus
  directory. It verifies the common SH-2 entry spine, bounded PC-relative
  literals, exact word count and the fail-closed `rules=0`/no-dispatch state.
- ✅ The test passes against `/Users/bosse/.firestaff/data/nexus`; the
  result proves an encoding/profile receipt only, not task semantics or an
  event dispatcher.

# Nexus retail readiness and historical-format fence (2026-08-06)

- ✅ Built the actual `firestaff` executable and ran the runtime screenshot
  readiness gate against `/Users/bosse/.firestaff/data/nexus`. The real
  Track 1 launch is authenticated and reaches the Nexus runtime; the result
  is correctly `BLOCKED_CAPTURE` with valid 320×200/960×540 BMP geometry and
  zero pixels because Saturn VDP1/VDP2 presentation evidence is absent.
- ✅ Marked the old H2321 format report as a historical snapshot and listed
  the current bounded retail receipts for DGN, SMAP, MAP/SAL, SLEV, ITEM,
  FACE, STABG, TITLE, MENU.BPK and FONT256. The note explicitly preserves
  the remaining capture gates and removes the obsolete CD-track-map claim.
- ✅ Focused tests: `nexus_production_source_boundary` and
  `nexus_v1_track1_real_screen_capture_readiness` passed; the runtime gate
  returned `BLOCKED_CAPTURE` rather than a false failure or promotion.

# Nexus production text-raster fence (2026-08-06)

# Nexus production source boundary (2026-08-06)

- ✅ Added `nexus_production_source_boundary`, a CTest verifier that keeps
  synthetic V2 HUD/renderer modules and unproven text/MNS presentation paths
  out of `firestaff_nexus`. This protects the retail fail-closed boundary
  during future CMake/source-list changes.

# Nexus CI source boundary (2026-08-06)

- ✅ The cross-platform CMake workflow now runs
  `nexus_production_source_boundary` as a hard check after building the Nexus
  library. `docs/nexus_ci.md` no longer reports the obsolete zero-test state;
  it documents the actual data-free CI boundary and the remaining private
  retail/capture requirements.

# Nexus direct Structure2 decode gate (2026-08-06)

- ✅ Closed the remaining public retail bypass: the active canonical DGN
  level now rejects direct Structure2 texture/palette decoding as well as
  the LEV-load path. DMWeb descriptor and format receipts remain available
  as no-draw evidence; only isolated non-DMWeb fixtures may use the helper.
- ✅ Added a real-DGN regression proving the retail decode receipt stays
  invalid with zero decoded surfaces until Saturn VDP1/CLUT capture exists.

# Nexus retail Structure2 decode fence (2026-08-06)

- ✅ Closed a retail material-promotion leak: loading a hash-verified LEV no
  longer decodes the DMWeb 08h/28h Structure2 hypotheses into host pixel and
  palette surfaces. The bounded descriptor/format receipt remains available
  as no-draw provenance; compatibility fixture lanes are unchanged.
- ✅ Verification: `test_nexus_v1_dgn_geometry_readiness` and the real Nexus
  boot/hash scan. No game data was copied or committed.

# Nexus boot-profile synthetic feature fence (2026-08-06)

- ✅ Removed unproven behavior claims from the default Nexus profile. It now
  enables only `USE_SATURN_CD`; historical champion-limit, stat, rune, party,
  and renderer flags remain explicit opt-in compatibility settings. This
  matches the evidence boundary: RLOWFIX/PLRD proves 20 mirror records, while
  DGN/Structure3 and the action/UI consumers still require source binding.
- ✅ Added a smoke assertion for the exact default flag set. No game data was
  copied or committed.

# Nexus retail mechanics mutation fence (2026-08-06)

- ✅ Closed the remaining retail mechanics leak: `NEXUS_SRC_EXTRACTED` and
  `NEXUS_SRC_ISO` now return before the DM1-shaped movement/turn loop can
  mutate party pose or consume commands. The explicit `NEXUS_SRC_NONE` lane
  remains available for isolated compatibility tests. Updated the integration
  regression to prove decoded floor geometry alone cannot move the party.
- ✅ Verification: `test_nexus_v1_tick_integration` (20 tests), including
  retail hunger/status/light/door/teleport/HUD and movement immutability.
  No game data was copied or committed.

# Nexus startup ASCII-label provenance fence (2026-08-06)

# Nexus CDDA selection-only status (2026-08-06)

# Nexus startup TITLE.BIN truncation gate (2026-08-06)

- ✅ Hardened `nexus_title_load` so an incomplete `TITLE.BIN` cannot reach the
  DMWeb MAPD/TIBG offset through the already-loaded title-surface path. This
  keeps startup source handling fail-closed; it does not promote decoded title
  tiles to Saturn VDP2 presentation. Verification: rebuilt `firestaff_nexus`
  and `test_nexus_v1_title_mapd_real`; the aggregate `firestaff` link remains
  blocked by unrelated concurrent Theron symbols. No game data was copied or
  committed.

# Nexus ITEM.IBS floor identity and attribute provenance (2026-08-06)

- ✅ Corrected the retail floor-item handoff. DMWeb Structure1Fa byte 4 is
  now kept as the ITEM.IBS declaration ID; ITEM.IBS word 20 remains only an
  inventory-image association and can no longer substitute a different item.
- ✅ Retained Structure1Fa attribute bytes 5 and 7 and the source-entry ordinal
  on floor records. This preserves real LEV01 torch and waterskin charge bytes
  without guessing their action semantics.
- ✅ Verification: rebuilt `test_nexus_v1_boot_file_hash_scan` and
  `test_nexus_v1_inventory_gameplay`; the real European Nexus corpus passed
  the declaration-ID/attribute provenance checks and the inventory suite
  passed 67 tests. No game data was copied or committed.

# Nexus boot-profile capability audit (2026-08-06)

# Nexus startup event and PRS3 corpus identity gates (2026-08-06)

- ✅ Hardened `test_nexus_v1_event` so the 61-name `DM.BIN` receipt cannot
  silently accept a filename-matched or synthetic replacement; it now requires
  the authenticated European MD5 `e88d60859f65f08fa622e1992b02280f`.
- ✅ Hardened `test_nexus_v1_bpk_prs3_payload_evidence` to honor
  `FIRESTAFF_NEXUS_DATA_DIR` and require the authenticated English/French
  `MENU.BPK` MD5 catalog before walking the 162 retail PRS3 entries. The
  decoder, upload, and Saturn presentation routes remain explicitly blocked
  pending instrumented Saturn capture.
- Verification: `test_nexus_v1_event`,
  `test_nexus_v1_bpk_prs3_payload_evidence`, and focused CTest both pass with
  `/Users/bosse/.firestaff/data/nexus`.

# Nexus HUD no-draw probe contract (2026-08-06)

- ✅ Corrected the V2 HUD runtime and overlay probes, which still expected the
  procedural diagnostic surface to paint when its phase gate was enabled.
  They now verify the actual source-lock contract: state setters remain safe
  for diagnostics, but no framebuffer pixels are written without authenticated
  retail HUD/VDP1/VDP2 ownership. Added the DMDF/DGN format caveat to the
  evidence string so the probe citation is complete.
- Verification: `firestaff_nexus_v2_hud_runtime_probe` 25/25, focused
  `nexus_v2_hud_overlay` and `nexus_v2_hud_runtime_integration` CTest pass.

# Nexus real SAL/MAP runtime receipt gate (2026-08-06)

- ✅ Extended the 16-level authenticated SAL/MAP corpus regression to run the
  runtime SFX receipt after every real pair load. It now proves that canonical
  SAL/MAP metadata is retained while playback remains blocked, with no ready
  status or host voice, until Saturn event-dispatch and SDDRVS ownership are
  captured.

# Nexus spell action capture gate (2026-08-06)

- ✅ Closed the M11 Nexus Light/Torch/Darkness bridge while Saturn spell
  action ownership remains uncaptured. Recognized rune sequences no longer
  mutate the shared compatibility light timeline; the regression now proves
  that the runtime preserves `MagicalLightAmount` and reports the missing
  Saturn action-dispatch capture.

# Nexus ISO-only corpus inventory (2026-08-06)

- ✅ Compared the authenticated English Track 1 ISO directory with the local
  loose corpus. The 137 ISO entries resolve to 131 loose game resources plus
  six deliberate ISO-only members: three DMN text files and three DMV video
  files. The inventory now records that they remain virtual until a real
  consumer is bound, with no placeholder materialization.

# Nexus SLEV/SAL/MAP retail identity gate (2026-08-06)

- ✅ Hardened the 16-level SLEV/SAL/MAP corpus regression with the production
  MD5 catalog before accepting bounded sound records. The 154 MAP records and
  existing playback/event no-op gates remain intact; SDDRVS/event-consumer
  capture is still required before runtime audio dispatch.

# Nexus DGN level retail identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_dgn_level_content` with the production MD5
  catalog for all 16 European `LEV00-15.DGN` files before accepting its
  item/decoration/sensor census. The real corpus remains diagnostic evidence;
  Saturn object, loot, trigger and viewport consumers are still capture-gated.

# Nexus Structure1F ITEM.IBS retail identity gate (2026-08-06)

- ✅ Hardened the real Structure1F→ITEM.IBS coverage regression with the
  authenticated European `ITEM.IBS` MD5 before accepting regular-item and
  descriptor-0008 floor-image coverage. The retail item source remains
  diagnostic/no-draw evidence; Saturn action, loot and pickup consumers remain
  capture-gated.

# Nexus HUD champion-panel retail identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_champion_panel` with the authenticated European
  DM.BIN MD5 before accepting stat-bar, inventory-slot and equipment-slot
  geometry. The HUD remains no-draw/input-capture-gated after this source
  receipt.

# Nexus startup/menu source retail identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_startup_menu_source` with the authenticated
  European DM.BIN MD5 before accepting its startup/menu loader, FONT256,
  STABG and VDP2-register receipts. Saturn menu order, text placement and
  VDP1/VDP2 composition remain capture-gated.

# Nexus startup media retail identity gate (2026-08-06)

- ✅ Added exact retail MD5 checks to the startup-media regression for
  STABG.BIN, WARNING.BIN, GAMEOVER.BIN, TITLE.CG and LOGOBG.DG2. Their
  STMP/DGT2/atlas format receipts and no-draw assertions still pass; no host
  presentation path was reopened without Saturn VDP capture.

# Nexus MENU.BPK retail identity gate (2026-08-06)

- ✅ Hardened the real MENU.BPK archive regression with the authenticated
  English/French retail MD5 identities before accepting its 163-entry,
  162-PRS3 directory census. PRS3 decompression and Saturn menu composition
  remain intentionally capture-gated.

# Nexus HUD DM.BIN retail identity gate (2026-08-06)

- ✅ Hardened the 80-entry HUD layout and 40-entry hit-rectangle regressions
  with the production DM.BIN MD5 before accepting real geometry. The parser
  and source anchors remain verified; Saturn input dispatch, VDP2 ownership
  and final HUD composition remain capture-gated.

# Nexus FACE.BIN retail identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_face_bin` with the authenticated European
  FACE.BIN MD5 before accepting the 20-record portrait decode and UI-surface
  census. No portrait draw path was reopened; Saturn VDP1 destination,
  scaling, flip and command order remain capture-gated.

# Nexus ITEM.IBS retail identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_item_ibs` with the authenticated European retail
  MD5 before accepting the decoder census. Same-sized synthetic or renamed
  ITEM.IBS data can no longer count as the 243-item/223-image source corpus.
  No runtime item-use or loot mutation was enabled; Saturn action/VDP1
  provenance remains capture-gated.

# Nexus MNS retail corpus identity gate (2026-08-06)

- ✅ Hardened `test_nexus_v1_mns` so all 30 expected retail `.MNS` files must
  be present under `FIRESTAFF_NEXUS_DATA_DIR` and match the production MD5
  identity catalog before decode/texture checks count. This closes the
  same-named/synthetic corpus loophole; 815 real source textures and the
  existing animation checks still pass. No game data is tracked.

# Nexus startup handoff fixture re-authorization (2026-08-06)

- ✅ Re-authored `test_m11_nexus_startup_runtime_handoff` around the current
  source-faithful capture gate. Synthetic title/save/champion fixture state no
  longer expects host draw promotion; the test now proves no-draw behavior and
  preserves the DGN fail-closed route until Saturn VDP1/VDP2 capture exists.
  Verification: focused test passes; no retail data is changed or committed.

# Nexus viewport provenance quarantine (2026-08-06)

- ✅ Closed the remaining generic creature billboard raster path. The public
  API now remains no-draw because its host texture and inferred gameplay flags
  did not prove Saturn VDP1 command/CLUT/placement or DMDF/MNS ownership. The
  phase-4 source-lock document no longer advertises DM1 door/projectile
  geometry or gray creature placeholders as Nexus visuals. Build and relevant
  Nexus provenance tests pass; no retail data is changed or committed.

- ✅ Closed the remaining direct UI host-copy path. `nexus_ui_blit_surface*`
  no longer writes a framebuffer, and `nexus_ui_surface_remap_pal`/
  `nexus_ui_surface_darken` no longer mutate retained retail pixels. The
  startup-media regression proves source and framebuffer preservation until
  Saturn VDP1/VDP2 placement and composition are captured.

- ✅ Removed the default `SATURN_CDDA_AUDIO` capability from
  `src/nexus/nexus_v1_boot_profile.c`. The retail disc's Track 2–9
  declaration remains provenance only; level selection, CDDA handoff and
  playback are still unbound, so the default launcher profile must not
  advertise that route. Updated the public header and CDDA API comment to
  state the capture gate, and added a regression to
  `tests/nexus_v1_boot_profile_smoke.c`. Verification: 27/27 smoke checks,
  `git diff --check` clean.

- ✅ The same audit removed `RESTRICTED_DOOR_CLOSES` from the default Nexus
  runtime flags. Retail door transition/timer ownership is still absent from
  the Saturn capture, so a default boot must not claim the DM1-shaped timed
  door behavior. The flag remains available only for explicit compatibility
  profiles. The smoke regression now covers both uncaptured capabilities.

# Nexus PLRD provisions quarantine (2026-08-06)

- ✅ 2026-08-06 DM2 PC-DOS source-AI baseline restoration: the retail
  executable's original 63 × 36-byte `v1d296c.dat` table is now retained as
  loaded source data before optional GDAT `CREATURE_AI` overrides. Source:
  `SKProject/SKULLWIN/dm2data.cpp::c_dm2data::init` and
  `c_record.cpp::DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD`. The
  `CREATURES[type & 0xff].word(0x05)` mapping now accepts all 256 original
  record type keys without expanding or fabricating the 63-row AI table.
  In the mounted eight-file PC-DOS corpus all eight direct-root streams now
  decode with their source masks. `c_querydb.cpp::DM2_QUERY_GDAT_CREATURE_WORD_VALUE`
  returns scalar zero for the absent type-54 (twice) and type-127 row-5
  fields, which selects the genuine `v1d296c[0]` table row; no GDAT mapping
  is fabricated. The real-data regression asserts both raw absences and the
  exact 8/0/0 decoded/blocked/malformed corpus outcome.
  Verification: `test_dm2_v1_drops_gdat_real_data`,
  `test_dm2_v1_creature_animation_gdat_real_data`,
  `test_dm2_v1_save_load_real_data` (85/85), and `test_dm2_v1_save_load`
  pass. No
  game data is copied, unpacked, or used to admit runtime resume.

- ✅ 2026-08-06 DM2 canonical GRAPHICS.DAT probe correction: the real-data
  creature-animation regression now actually opens the DOS-uppercase
  `GRAPHICS.DAT` after its lowercase candidate fails. Previously it changed
  the candidate path but returned a misleading no-data skip without rereading
  it. The mounted PC-English file is now parsed before the test accurately
  reports that its current AI classification is not admitted. No game data is
  copied or unpacked.

- ✅ 2026-08-06 DM2 original-SKSAVE creature-mask gate: the isolated
  `DM2_READ_RECORD_CHECKCODE` reader no longer substitutes the default DB4
  SUPPRESS mask when SKProject's
  `DM2_QUERY_CREATURE_AI_SPEC_FLAGS` decision is unavailable. A source-owned
  `CREATURES[type]` binding must exist before `v1d647f` or `v1d648f` is
  selected; otherwise the shared bitstream stops fail-closed. The later
  `v1d296c` baseline restoration above supplies the PC-DOS path where the
  binding exists, while types without an original row remain blocked. This is
  diagnostic-only and does not admit a resume.
  Verification: `test_dm2_v1_save_read_record_checkcode`,
  `test_dm2_v1_save_load_real_data` (80/80), and
  `test_dm2_v1_save_load` (26/26).

- ✅ 2026-08-06 DM2 V2.2 local-art cache removal: removed the dormant
  `v22_inplace_cache.bin` parser and its invented wall/floor/creature RGBA
  cache admission from the production-linked compatibility module. Its public
  API remains an explicit no-op/no-draw boundary, so local generated art is
  neither opened nor retained as possible DM2 material. The verified V1 GDAT
  route remains the only visual owner. Verification:
  `test_dm2_v22_inplace_draw_pc34` (17/17),
  `test_dm2_v22_viewport_swap_wireup_pc34` (10/10), and the full `firestaff`
  target build pass.

- ✅ 2026-08-06 DM2 external BPP8 screenshot palette correction:
  `M11_Screenshot_CaptureCurrent` no longer folds framebuffer indices through
  `0x0f` after M11 has installed a source-owned 256-colour palette. This makes
  the external capture retain the same physical GDAT index and dtPalIRGB RGB6
  colour as the live DM2 title/menu/credits surface. The legacy 16-colour
  route retains its historic mask. Source: SKProject
  `startend.cpp::DM2_SHOW_MENU_SCREEN` and `DM2_SHOW_CREDITS`; both present
  BPP8 TITLE pages with physical indices. Verification:
  `test_m11_screenshot_capture_delivery` now checks high index `0x83`, and a
  dummy-SDL `firestaff --game dm2 --boot-probe` run against the supplied
  PC-English corpus emitted the 320×200 indexed capture plus the actual
  post-presentation RGBA capture.

- ✅ 2026-08-06 DM2 static-menu readiness correction: the Tier 1 strict boot
  probe now expects `titleReady=1` for `dm2-startup-menu`. SKProject
  `SKWIN/startend.cpp::DM2_SHOW_MENU_SCREEN` draws the static
  `TITLE/0/dt07/4` menu before `MessageLoop(true)`, so it is immediately
  interactive rather than waiting for a title-animation frame. It also
  verifies that terminal menu state rather than pressing Enter and demanding
  an unsupported `GAME_LOAD` handoff. A fresh read-only boot of the supplied
  PC-DOS data reports the expected phase, ready state, raw 320×200 capture
  and post-presentation capture; game data was neither copied nor unpacked.

- ✅ 2026-08-06 Theron production archive boundary: the archive regression now
  checks every CMake-excluded inferred/procedural Theron translation unit,
  not only the legacy creature table. A future source binding must therefore
  be explicit and reviewable before it can enter the production library.

- ✅ 2026-08-06 Theron production source list cleanup: removed the duplicate
  `theron_v1_track02_spell_descriptors.c` entry from `firestaff_theron`.
  The production archive now has one explicit source entry for that
  source-bound descriptor module; no runtime behavior or fixture route was
  changed.

- ✅ 2026-08-06 Theron synthetic combat-spawn quarantine: removed the
  dungeon/coordinate replay seed from the production spawn path. Authenticated
  Track 02 monster records remain in the source ledger, while live creature
  publication stays blocked until the original PCE bank-switched RNG consumer
  at overlay `$4644/$4667` is bound. The diagnostic category formula helper
  remains available only to isolated tests. The combat regression now proves
  that a real occurrence cannot become a synthetic live creature, and the
  production combat archive keeps its required link symbols.

`RLOWFIX.BIN/PLRD` is now treated as the source of its authenticated raw
name/TABL references, statistics and equipment ordinals only. The production
parser no longer seeds food/water with the inherited DM1 value `1500`; those
fields remain unbound until the Saturn new-game/save consumer is captured.
The real-data PLRD regression passes, and the historical save/inventory notes
were corrected to stop presenting DM1 provisions as Nexus data.

- ✅ 2026-08-06 DM1 C13 M11 runtime fixture correction: the focused runtime
  regression now initializes its admitted champion with live current HP before
  the first M11 tick. The previous fixture used `hp.maximum = 100` with
  `hp.current = 0`, so the source `m11_check_party_death()` gate correctly
  stopped all later input and made C13 appear not to advance. With the fixture
  representing a live champion, the source C13 step-2 -> step-1 -> F0283
  step-0 chain advances through the expected M11 ticks. The focused C13 test,
  2/2 original PC34-backed save round trips, and the real-corpus probe pass.
  This fixes test setup only; fixture-free original saves containing C13 events
  are still required.

- ✅ 2026-08-06 DM1 full-asset audit path correction: the 713-record
  `GRAPHICS.DAT` audit now accepts both a direct install root and the standard
  PC34 `DATA/GRAPHICS.DAT` layout. The previous test-only path assumption
  reported a false open failure for the real extracted DOS package. Ninja
  rebuild and the full real 713-record audit pass: 543 bitmap records, 0
  suspicious bitmap, 35 non-bitmap records, 4 empty records and 131
  zero-sized records.

- ✅ 2026-08-06 DM1 source SND3 binding: M11 now rebinds the original 35-event
  SND3 bank to the exact hash-admitted PC3.4 `GRAPHICS.DAT` path when the DM1
  startup graphics receipt is applied. This prevents an unrelated default
  search-root file from supplying sound while the visual runtime uses another
  installation. Missing or malformed source samples remain silent; no
  procedural marker is used by authenticated DM1 events. Real DM1 object/audio
  regressions pass against the extracted PC3.4 data.

- ✅ 2026-08-06 Nexus creature runtime quarantine: production mechanics and
  engine ticks now keep creature AI, spawner admission, and projectile motion
  fail-closed while `nexus_v1_action_semantics_proven()` is false. Direct
  creature helpers remain available for diagnostics; integration tests now
  verify that uncaptured actors do not move or damage the party. Stale combat
  and creature docs are labelled historical/diagnostic instead of claiming
  source-locked live parity.

- ✅ 2026-08-06 Nexus retail tick-state quarantine: ISO/extracted engine ticks
  no longer advance unbound action cooldowns, door animation or trap timers;
  retail movement no longer applies the inherited DM1 step-stamina mutation,
  and a local door record cannot open from movement without the Saturn action
  receipt. Creature death/XP/script/spawner follow-up, damage-display/message
  timers and game-over transitions are also closed. Fixture behavior remains
  available for isolated tests. The focused tick integration now covers the
  retail no-mutation boundary (20 tests), including pending teleport and level
  transition writes.

- ✅ 2026-08-06 Nexus retail provision mutation quarantine: the mechanics tick
  no longer decrements or penalizes unbound PLRD food/water in ISO/extracted
  engines. The 14-test tick integration confirms the real-source path leaves
  stamina and provision timers untouched; fixture-only DM1 behavior remains
  isolated to `NEXUS_SRC_NONE`.

- ✅ 2026-08-06 Nexus real startup probe correction: the Track 1 probe now
  accepts the complete 20-record FACE.BIN source receipt while separately
  requiring VDP1 placement for drawing, and treats the retained FONT256.S2D
  bytes as non-draw evidence (`font_loaded == 0`) until Saturn page/attribute
  mapping is captured. Real-data launch probe: 57/57.

- ✅ 2026-08-06 Nexus startup-menu text boundary: the runtime save-menu
  handoff now removes generic host ASCII `DRAW_TEXT` commands before M11/M12
  consumption. The layout builder remains available for isolated tests, while
  real startup text stays blocked pending authenticated TEXT4/TABL/FONT012
  Saturn consumer and placement evidence.

- ✅ 2026-08-06 Nexus shop-instance quarantine: retained the real DM.BIN price
  receipt while blocking synthetic shop registration, stock, open and lookup
  routes. Buy/sell remain no-op until the Saturn shop-object consumer and
  inventory/gold dispatch are authenticated; focused shop-manager tests pass.
- ✅ 2026-08-06 DM1 V2 synthetic-effects framepath quarantine: removed the
  production step that converted viewport projectiles, explosions and
  teleporters into procedural particles or dynamic-light sources after the
  authenticated ReDMCSB draw. The source bitmap/palette renderer remains the
  only DM1 visual owner until a real V2 asset corpus exists. Updated the
  framepath probe to verify that V1, V2.0 and V2.2 leave unbound V2 state alone.
  Ninja `firestaff` build and focused CTest pass.

- ✅ 2026-08-06 Nexus container/loot quarantine: removed the synthetic
  DM1-shaped chest/crate mutation route from production. Retail DGN item and
  location records remain diagnostic until a Saturn container owner, content
  chain, key dispatch and loot writeback are authenticated. Focused container
  and real ITEM.IBS inventory tests pass with the route blocked.

- ✅ 2026-08-06 Nexus combat claim quarantine: corrected the V1/V2 phase-gate
  metadata so the DM1-shaped combat helper is described as diagnostic only.
  The production action-semantics gate remains fail-closed while Saturn
  attack dispatch, target admission, RNG and effect-write capture are absent.

- ✅ 2026-08-06 Nexus stale-doc quarantine: marked the old testing, armor,
  potion and combat-item pages as historical/diagnostic snapshots. They no
  longer present DM1-derived formulas or the former “no tests/all scaffolding”
  planning text as current Nexus parity, and point readers to the strict
  fidelity inventory and Saturn capture gates.

- ✅ 2026-08-06 DM2 FM Towns English ZIP companion: an explicitly selected,
  hash-verified PC-English `GRAPHICS.DAT` now accepts the DOS archive's real
  `DATA/GRAPHICS.DAT` member spelling as well as lower-case virtual paths.
  The Japanese FM Towns CD remains the game-data owner and the companion is
  read solely in RAM. The real-media test verifies the complete English text
  overlay without extracting game data, including M12's scanned archive
  provenance handoff. Loose companion bytes are re-hashed after their RAM
  read before they may reach the GDAT text parser.
- ✅ 2026-08-06 Theron Mednafen capture-module routing: the live original-media
  capture launcher now passes `-force_module pce`, preventing a mixed-audio
  CUE from entering Mednafen's CD-DA player instead of the HuC6280 PCE
  module. The corrected capture was run against the real US Track 02 BIN,
  authentic System Card 3.0 and source-derived audio/Track 19 media. It
  emitted valid 65,536-byte VDC VRAM and 1,024-byte VCE snapshots and 256
  authenticated raw sectors, while still proving no game-owned post-startup
  PCE-CD read (`non_system_card_pcecd=0`) or `$2600` handoff. Dungeon,
  object, palette and viewport promotion therefore remain correctly blocked.

- ✅ 2026-08-06 Theron capture input/media boundary: the instrumented original
  runner observed a real PID-bound macOS key-down/up pair, proving host input
  reaches the capture boundary. Firestaff's source-locked intake continues to
  require US `INDEX 01 = 225`; the raw BIN also contains a valid MODE1 sync at
  that authenticated offset, while an earlier sync-like span at sector 75 is
  not promoted. Mednafen still reports uncorrectable sectors for the raw/CUE
  pairing before a game-owned consumer read. No level, object, tile, palette
  or viewport semantics were promoted, and no guessed pregap normalization was
  added.

- ✅ 2026-08-06 Theron split-ISO intake: the raw-media regression passed against
  the supplied retail `TQUS.cue` with `TQUS19.iso` and `TQUS02End.iso`. The
  production materializer rebuilt the canonical 3,221-sector US Track 02 ISO
  with MD5 `ceb02343868f80cec899e9b239aff2da` and the expected MODE1/2048
  receipt; this verifies media intake only, not later game-owned consumers.

- ✅ 2026-08-06 Theron targeted real-data regression: the available rebuilt
  binaries passed the 653-case startup-flow probe, all seven US and seven JP
  Track 02 level-bank checks, the authenticated US/JP Track 19 level-offset
  checks, and the 57-case M11/M12 launcher handoff boundary. The raw split-CUE
  case remains an explicit skip when `FIRESTAFF_THERON_CUE` is unset.

- ✅ 2026-08-06 Theron ISO capture intake: the live Mednafen launcher now
  distinguishes authenticated MODE1/2048 Track 02 ISO CUEs from raw
  MODE1/2352 BIN CUEs and admits the canonical US/JP ISO hashes separately.
  `bash -n` and the capture-script regression pass; no consumer, level,
  object, bitmap or palette semantics are promoted by this route alone.

- ✅ 2026-08-06 Theron chapter-marker loot parity: removed the duplicated,
  incorrectly ordered quest-item table from the chapter marker and dungeon
  progression diagnostic. Both now consume the authenticated US Track 02
  retrieval table, restoring the real order `Shield Defiant`, `Taza Boots`,
  `Taza Poleyn`, `Soulcage`, `Taza Armour`, `Tazahelm`, `Retaliator`.
  The chapter-marker probe passes 65 checks with 0 failures.

- ✅ 2026-08-06 Theron forcefield companion-record handoff: production
  `enter_forcefield_with_roster()` no longer advances empty companion slots
  when the US text consumer is unavailable. Selected Soul Room mirrors now
  bind the real Track 02 champion records for stats, skills and equipment;
  display names remain empty unless a source text receipt supplies them.
  The real-US M12/M11 launcher boundary is 57 passed, 0 failed, 1 skipped.

- ✅ 2026-08-06 DM2 title/credits identity gate: the real PC-DOS M11 startup
  test now verifies separate bounded GDAT receipts for the `TITLE/0/dt07/4`
  menu and `TITLE/0/dt07/1` credits payloads, together with the original
  `dtPalIRGB` palette. This prevents the credits surface from being accepted
  as the startup menu when palette-index values happen to overlap.

- ✅ 2026-08-06 DM1 macOS runtime captures: captured clean Entrance and
  post-Entrance dungeon frames from the built Firestaff executable using the
  verified PC DOS 3.4 data directory and original save. Added the real images
  to `docs/screenshots/` and README. HoC/HUD capture remains explicitly open;
  these frames are not promoted as proof for those outstanding routes.

- ✅ 2026-08-06 Nexus title-capture admission: separated the decoded retail
  TITLE.CG/TITLE.BIN source receipt from the missing Saturn VDP1/VDP2 title
  capture. Production engines leave the explicit capture seam closed, so a
  full-start package cannot advertise title capture or host display ownership
  from atlas bytes and timing alone. The synthetic positive startup fixture
  now names its external capture witness explicitly; the retail startup-menu
  regression asserts title art loaded but title capture unavailable.

- ✅ 2026-08-06 DM1 keyboard potion-use source route: removed the fabricated
  generic M11 potion effects and routed keyboard/use-item consumption through
  the ReDMCSB PANEL.C-backed PC34 live transaction. The route now uses the
  source formulas, writes the real raw potion record, converts to type 20
  while preserving Power, and passes the focused consumable/live-transaction
  tests. Full HoC packaged capture remains open.

- ✅ 2026-08-06 Nexus fountain placeholder quarantine: removed the unproven
  public DM1-shaped fountain registration/effect path. Caller-supplied water,
  health, mana and poison values can no longer mutate a champion; the manager
  remains an empty provenance seam until a real Saturn fountain record and
  action/effect consumer are authenticated.

- ✅ 2026-08-06 DM1 YA shield expiry: potion consumption now mirrors the
  source C72 status timeout in the runtime timeline, including the applied
  defense delta, C72 event fields, expiry delay, and lifecycle shield mirror.
  The existing M10 timeline dispatcher therefore owns the later subtraction;
  no second host countdown is introduced.

- ✅ 2026-08-06 Theron US roster/text audit: real US Track 02 still proves
  only the `GO AWAY AND RESURRECT THERON` prompt at `0xa0722`; the JP ASCII
  roster cluster is absent from the US receipt. Real US Track 19 item names,
  item properties, level labels, and startup envelope remain independently
  validated, but the bank-$1f/stage-2 disassembly still has no executing US
  roster/text consumer or `$2600` RAM join. No host champion labels were
  restored. See `docs/source-lock/tqr_v1_us_roster_consumer_audit_2026-08-06.md`.

- ✅ 2026-08-06 Theron bitmap-route provenance audit: the real US/JP Track 02
  indexed samples remain available as byte-level diagnostics, but the startup
  route labels are now documented as layout-catalog candidates rather than
  VDC/VCE screen ownership. Palette binding and RGBA output remain blocked;
  the required consumer/LBA/VDC/VCE proof is recorded in
  `docs/source-lock/tqr_v1_bitmap_route_provenance_audit_2026-08-06.md`.
  The real-US M12/M11 launcher boundary remains green at 52 passed, 0 failed,
  1 skipped.

- ✅ 2026-08-06 Theron forcefield Enter admission: fixed the production
  no-roster startup API, which previously entered the forcefield base state
  and immediately reset to Soul Room/`NOT_READY`. It now preserves the
  source-owned Theron-only admission without inventing companions, and the
  M12/M11 boundary proves keyboard/pointer input reaches the explicit
  level/VDC/VCE capture gate. Dungeon promotion remains fail-closed until the
  original consumer capture exists.

- ✅ 2026-08-06 DM2 unowned-shop state removal: the production shop API no
  longer retains caller-provided gold, negotiation skill or inventory when no
  source-owned `SHOP_GLASS` transaction exists. The empty catalog already
  blocked normal M11 access, but exported helper calls could still create a
  private host state; they now reject without mutation. The focused ownership
  gate and the real PC-DOS M11 startup gate pass.

- ✅ 2026-08-06 Theron palette admission cleanup: raw Track 02 asset loads no
  longer initialize the procedural stone palette. Verified media therefore
  begins with an empty, non-renderable palette until a captured HuC6260 span
  and consumer are explicitly bound; fixture tests that need a palette still
  initialize it locally. Theron asset-loader and rendering tests pass.

- ✅ 2026-08-06 DM2 coordinate-only door-action removal: M11 Action could
  formerly advance a door state by writing its G1 tile directly. The route
  lacked SKProject `DM2_ACTUATE_DOOR`'s DB0 record, actuator/timer context,
  collision, sound and follow-up scheduling, so it is now explicitly
  non-mutating and rejected. Door state remains readable from the original
  dungeon; production actuation stays blocked until the complete source
  transaction is owned.

- ✅ 2026-08-06 Nexus CUE external-media completeness gate: `nexus_iso_open_cue`
  still selects only the authenticated Nexus data track, while the new
  `nexus_iso_cue_media_receipt` checks every CUE `FILE` payload independently.
  The real European loose CUE can therefore be reported as missing its
  external CDDA files instead of being mistaken for a complete runnable disc;
  this is a readiness correction only and does not claim audio playback.

- ✅ 2026-08-06 Nexus MENU.BPK prerequisite-status correction: a canonical
  stored or bounded-decoded route remains blocked as `SATURN_PRESENTATION`
  when the PALT/VDP1 capture is absent. The final handoff normalization no
  longer relabels that route as `READY_STORED`; the dedicated renderer-handoff
  regression covers both blocked routes and the explicitly admitted capture
  route.

- ✅ 2026-08-06 DM2 square-actuator failure contract: the public
  coordinate-only DB3 entry now returns failure when its original DB3/DB14
  payload, link and timer transaction are unavailable. It already made no
  mutation, but no longer reports a misleading successful no-op to a caller.
  Real G1 actuator receipt and PC-DOS startup gates remain the active proof.

- ✅ 2026-08-06 DM2 original-SKSAVE fail-closed cleanup: removed the
  unreachable partial `GAME_LOAD` publication branch, including its
  synthetic session, timer-owner and raw-dungeon handoff helpers. Public
  resume now has one explicit no-mutation contract until the complete
  SKProject `GAME_LOAD` record/hero/actuator/timer ownership chain is
  implemented. The real PC-DOS eight-save corpus regression proves each
  unmodified payload is still decoded for diagnostics but cannot change live
  party state or create a raw-save handoff. Focused real-corpus and save/load
  regressions pass.
- ✅ 2026-08-06 DM1 synthetic-path audit: checked the active M11 production
  target against ReDMCSB ownership and the local real PC34 corpus. M564
  object names, raw Thing/G0237 icon and charge resolution, decoded
  GRAPHICS.DAT viewport material, C127/C026 mirrors and F0702 held-object
  cursor paths are source-backed; missing material fails closed. The legacy
  generic viewport renderer is not linked into the `firestaff` M11 executable.
  Added `docs/parity/DM1_V1_SYNTHETIC_PATH_AUDIT.md`. Remaining work is real
  Mac capture, C13-bearing original saves and broader original-pixel pairs.

- ✅ 2026-08-06 DM1 original PC34 save recheck: two operator-supplied
  48,561-byte `DMSAVE.DAT` files from `Downloads/` passed
  `test_dm1_v1_original_save_pc34_backed_corpus_roundtrip` using the
  hash-resolved real DM1 `DUNGEON.DAT`. The verified inputs were
  `26ccd1591ccf6ec9e53186e994f73924185143f82055312cafd474ed7abc9437` and
  `ab7bb4a34b77bba033d7b6c31db32e7198a962b0e55c0644c0486f50bb361ecb`.
  No synthetic save bytes were used. This expands authentic PC34 evidence;
  C13-bearing saves and packaged Mac capture remain open.

- ✅ 2026-08-06 DM2 no-disk-materialization enforcement: removed the stale
  M12 PC-DOS archive and renamed-loose-file cache path that could write
  `GRAPHICS.DAT`, `DUNGEON.DAT`, music and alternate dungeon sidecars under
  `asset-cache/dm2`. PC ZIP/ISO and renamed loose data now retain their
  matched source paths as diagnostics and block launch until a bounded
  in-memory PC reader is complete. FM Towns and Amiga still use their existing
  selected-media RAM owners, so their verified archive launches remain intact.
  ZIP, ISO, renamed-loose and real FM Towns direct/English-companion ZIP

- ✅ 2026-08-06 Nexus PRS3 readiness correction: a successful bounded DMWeb
  decode of the 162 real MENU.BPK streams no longer promotes a runtime upload
  or renderer handoff. Runtime decode/upload receipts stay `BLOCKED_PRS3`,
  expose only diagnostic byte counts/hashes, and emit no host pixels until
  Saturn CLUT ownership, VDP1 upload framing and destination placement are
  authenticated. Real-corpus, boot-hash, surface-class and renderer-handoff
  regressions pass.

- ✅ 2026-08-06 Theron text/disassembly boundary verification: the real
  US/JP Track 02 HuC6280 bank-$1f receipt passes with the authenticated
  decompressor fragment, caller output-size contract and stage-2 resource
  handler. The real Track 02 text corpus still produces unresolved brace
  control codes, so world-text publication remains rejected and diagnostic
  output remains available for the future original text-consumer match.

- ✅ 2026-08-06 Theron US roster placeholder removal: the production Track 02
  receipt no longer reports the eight host literals (`MARA`, `LINOS`, etc.) as
  US source data. The authenticated US BIN currently proves the startup prompt
  but not the champion-name/title payload, so the roster catalog now returns
  `NOT_FOUND` and the menu remains fail-closed until the real encoded text
  consumer is recovered. JP roster decoding remains byte-verified.

- ✅ 2026-08-06 DM2 FM Towns English corpus-coverage gate: the real-media
  launcher regression now iterates every non-empty text key in the selected
  Japanese CD GDAT and requires a non-empty value at the identical key in the
  explicitly selected, hash-verified PC-English GDAT companion. It covers
  both a direct companion file and the original DOS ZIP member, using only
  RAM-held source buffers. A single familiar label can therefore no longer
  conceal missing English companion text. This verifies corpus coverage, not
  the still-unbound original GUI/dialogue consumers.

- ✅ 2026-08-06 DM2 FM Towns English launch coverage: the same complete-key
  check is now an M11 boot gate rather than test-only evidence. A session
  reaches the Japanese CD's original startup media only when the selected,
  hash-verified PC-English corpus supplies non-empty text for every non-empty
  native GDAT text key. Both corpus readers remain RAM-only; this adds no
  translation or fallback text, and it does not claim unbound GUI consumers
  are already rendered.

- ✅ 2026-08-06 Theron forcefield menu input: pointer activation now uses the
  same boot-layer admission route as keyboard Enter/Action. When the real
  Track 02 dungeon capture is still missing, the Soul Room remains visible
  and reports `AUTHENTIC CAPTURE ADMISSION REQUIRED` instead of silently
  returning to the launcher. Dungeon promotion remains fail-closed.

- ✅ 2026-08-06 Theron Track 01 real-audio consumer: the authentic CUE
  handoff now accepts the supplied CUE-declared WAV names when the matching
  local original OGG transcode is present, resolves the split Track 02 ISO
  alias, and decodes 44.1 kHz stereo Vorbis into the existing SDL3 audio
  stream when `vorbisfile` is available. Raw 2352-byte CDDA remains unchanged;
  platforms without Vorbis fail closed instead of treating OGG bytes as PCM.
  The real extracted US CUE/OGG/ISO corpus starts and pumps through the audio
  stream under the dummy SDL audio driver.

- ✅ 2026-08-06 Theron split-CUE parity coverage: the raw Track 02 intake
  regression now exercises the authentic Japanese CUE alias route as well as
  the US route. It verifies that `TQJP02.iso` resolves only to the supplied
  hash-verified `TQJP02End.iso` (`397039af02d50d15c70b74088eb8a1cb`, 149
  MODE1/2048 sectors), without promoting any dungeon or tile semantics.
  The real extracted US/JP CUE corpus passes the focused test.

- ✅ 2026-08-06 DM2 original-SKSAVE record-link audit: traced the supplied
  PC-DOS primary and backup corpus through SKProject
  `sksvgame.cpp::DM2_READ_RECORD_CHECKCODE`. The existing callback transcript
  is not admitted as a save reader: it omits the source DB4
  `CREATURES → AIDefinition` mask selection at lines 880-881, which desyncs
  every real stream after champion-item/leader roots. Continue and Load remain
  correctly blocked; no partial record graph or inferred session was kept.

- ✅ 2026-08-06 DM2 silent startup/action receipt regression: the real
  PC-DOS startup smoke route now verifies that boot preserves structured
  `ACTION` and startup handoff receipts while leaving their player-visible
  host status strings null. This matches the current fail-closed
  `c_gui_draw`/dialogue ownership boundary: no English replacement status is
  reintroduced merely to make a menu or front-cell action look playable.
  Verified against the authenticated local PC-DOS `GRAPHICS.DAT` and
  `DUNGEON.DAT` route.
- ✅ 2026-08-06 DM1 ReDMCSB G0190 wall-ornament bitmap selection: replaced
  the generic depth heuristic with the exact 13-entry PC34/I34E derived
  bitmap increment table from `DUNVIEW.C`. This corrects D3L-front/D3C-front
  and D1C/front-mirror selection while preserving the source G0205 zones,
  palette ownership and flips. `dm1_v1_wall_ornament_pc34_compat` passes all
  13 native-index assertions against the source table.

- ✅ 2026-08-06 DM1 M653 font identity gate: removed the unverified font index
  and unique-size heuristic from the shared GRAPHICS.DAT loader. Only the
  ReDMCSB PC34/legacy M653 records 695/557 are admitted; unrelated 768-byte
  records now fail closed. Focused action/spell source-gate, F0342 and F0662
  tests pass.

- ✅ 2026-08-06 DM1 inventory portrait source gate: authenticated DM1/CSB
  inventory panels no longer draw the host-generated face silhouette when
  C026 or save-owned M516 portrait pixels are unavailable. The panel now
  uses only original portrait material or leaves that area untouched; the
  fallback remains diagnostic-only for non-source fixtures.

- ✅ 2026-08-06 F10 all-game runtime coverage: the compact graphics/cheats
  popup is now regression-covered for DM1, CSB, DM2, Theron's Quest and
  Nexus as distinct M11 source kinds. The test exercises live presentation
  switching plus mouse tab/row clicks and per-game cheat/speed slots; the
  start-menu, in-game menu, DM2, Theron and Nexus input docs now describe the
  shortcut and its source-data boundary.

- ✅ 2026-08-06 DM2 FM Towns English save-dialogue labels: a selected,
  hash-verified PC-English `GRAPHICS.DAT` companion now supplies the two
  `c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL` GDAT labels to the real FM
  Towns dialogue command. The selected Japanese CD still owns the panel image,
  raw4 layout, palette and font; the English companion is bounded RAM-only
  data, including `archive.zip::data/graphics.dat`, and is never unpacked to
  disk. Native GDAT remains the fallback unless the authenticated FM Towns
  runtime overlay is active. The fixture dialogue receipt, PC real-data
  viewport route, and FM Towns real-media direct/ZIP companion regressions
  pass.

- ✅ 2026-08-06 Nexus current-main production audit: rechecked the startup,
  menu, HUD, viewport and SLEV/SAL routes against the real European corpus in
  `/Users/bosse/.firestaff/data/nexus`. Focused retail regressions pass for
  DM.BIN startup/HUD geometry, MENU.BPK, TITLE MAPD, STABG, FACE, SLEV/SAL,
  save round-trip and the 29/0 Track-1 readiness probe. The audit confirms
  synthetic BPX/text/MNS/V2 presentation modules remain test/probe-only and
  the linked viewport remains no-draw without authenticated Saturn capture;
  no guessed asset or screenshot was promoted.

- ✅ 2026-08-06 Nexus Mednafen launcher option audit: verified against the
  Mednafen 1.32.1 Saturn source that the USA/Europe BIOS setting is
  `ss.bios_na_eu`, then corrected the Nexus VDP1, PRS3, Structure3, SLEV/SAL
  and replay launchers plus their shell regressions. Real retail CD startup
  was confirmed with the extracted merged English CUE and the owned European
  BIOS; the absent Firestaff capture hook still correctly blocks promotion.
  Raw ISO/BIN capture inputs now fail closed before manifest creation because
  they do not carry the Saturn CDDA track layout Mednafen requires.

- ✅ 2026-08-06 Theron Track 02 monster source ledger: the authentic category-4
  thing-list records from all loaded dungeons are now copied into world state
  with source references, level coordinates, raw type/position, health words,
  number and direction flags. Verified Track 02 levels no longer use the old
  synthetic random placement/type table; live creature promotion remains
  closed until the original graphics, AI and combat consumers are bound.

- ✅ 2026-08-06 Theron Track 02 generator source ledger: map-reachable
  category-3 actuator type-6 records are now retained with source references,
  coordinates, value, effect/timing flags and targets. Verified worlds no
  longer consult the legacy DMWeb/DM1 generator table or random placement;
  generator execution remains closed pending the original timing, re-enable
  and spawn consumer.

- ✅ 2026-08-06 Theron Track 02 source-object bank: all decoded map-reachable
  non-host thing records are now retained in world state with their raw bytes,
  chain links, category/index, position and exact level coordinates. No guessed
  inventory or host-item semantics were promoted; the source records survive
  beyond the temporary loader result for the next proven consumer.

- ✅ 2026-08-06 Theron forcefield menu admission: the startup layout now keeps
  `ENTER FORCEFIELD` actionable in both Soul Room and READY, so starting with
  Theron alone no longer gets trapped on the mirror list. The runtime still
  fails closed with the authenticated-capture status when the dungeon handoff
  capture is absent. Direct-launch and launcher-handoff regressions pass.

- ✅ 2026-08-06 Theron generator budget integrity: production generator ticks
  now increment a respawn budget only after the source spawn bridge creates a
  live record. Legacy DMWeb/DM1 labels that are not yet bound to Theron's
  source-zone identity no longer consume generator capacity or create false
  respawn state. The focused source combat/runtime regression passes.

- ✅ 2026-08-06 CSB Atari MSA admission: standard Magic Shadow Archiver
  sector images now decode their documented big-endian header, track order and
  RLE runs into the same bounded GEMDOS reader used for raw `.st` images.
  This admits the supplied CSB Atari v2.0 original `.msa` even when nested in
  a `.7z`, without renaming or synthetic disk content; raw and nested tests
  cover discovery and materialization.

- ✅ 2026-08-06 CSB Amiga KryoFlux archive scan: archive members named
  `<track>.<side>.raw` are now recognized as flux tracks rather than hashed
  as loose ISO/game-file payloads. This prevents a supplied Amiga archive's
  hundreds of raw tracks from delaying the usable ADF path, while a normal
  `.raw` member and top-level raw CD-image support remain unchanged.

- ✅ 2026-08-06 External archive cache regression: the hash-scanner fixture
  now runs under an isolated home and asserts that a real `.7z` member writes
  its complete virtual path to the persisted cache. Fixture cleanup also
  removes its nested Atari archive after debugger/interrupted runs.

- ✅ 2026-08-06 External archive scan cache: hash-verified external archive
  members now cache their digest under the complete virtual member path and
  containing archive mtime/size. Repeated game-profile searches therefore
  reuse authentic member identities while an archive replacement invalidates
  the cache; no filename-based admission was added.

- ✅ 2026-08-06 M12 missing archive-tool recovery: when present game media is
  inside an archive whose reader is unavailable, the launcher now shows a
  localized popup naming the required extractor and asks the player to rescan
  after installation, instead of presenting the generic no-data message. The
  new three-line message is translated for all 19 shipped locales; the focused
  launcher and hash-scanner tests plus PO-layout validation pass.

- ✅ 2026-08-06 F10 runtime graphics and cheats panel: extended the existing
  all-game modal popup with a fourth CH page backed by the real shared
  launcher cheat toggle and live slower/normal/faster scheduler. Keyboard and
  mouse page/row controls persist per-game settings and apply speed changes
  without restart. README and `docs/runtime_graphics_and_cheats.md` now
  document F10, the start-menu relationship, all controls and the source-data
  boundary. The focused runtime popup and DM1 real-data tests pass.

- ✅ 2026-08-06 DM1 F0374 keyboard drop route: `M11_GameView_DropItem`
  now consumes a held real PC34 object from the transient leader/mouse hand
  before searching champion inventory, drops it onto the current party square
  with rollback on chain failure, and preserves the source M564 name. The
  real `Dungeon-Master_DOS_EN.zip` GRAPHICS.DAT/DUNGEON.DAT regression now
  proves leader-hand drop followed by pickup back into G4055.

- ✅ 2026-08-06 Theron startup frame cleanup: when verified Track 02 atlas
  pixels and font tiles are present, M11 no longer overlays its host-generated
  border on top of them. The unbound frame remains absent until the original
  HUD tile bank and layout are captured. `test_theron_v1_m11_launcher_handoff_boundary`
  (46/46) and `test_theron_rendering` (25/25) pass.

- ✅ 2026-08-06 CSB FM Towns legacy RAR admission: the supplied retail RAR
  contains the same original MODE1/2352 track as the Redump ZIP, but its
  compression method is unreadable by the installed 7-Zip build. External
  RAR extraction now prefers `unrar`, including shared hash discovery and
  virtual-path materialization. CSB stages the real `.bin`, verifies its
  `CDATA` pair and reports READY without a renamed loose-file copy.

- ✅ 2026-08-06 Nexus retail Saturn boot receipt: the supplied European BIOS
  (`96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`) and
  the real English merged BIN/CUE from the user-owned archive booted in stock
  Mednafen 1.32.1. The emulator identified SGID `T-9111G`, `DUNGEON MASTER
  NEXUS`, region U, PAL scanlines and CD tracks 1–9. Firestaff's own scan over
  `/Users/bosse/.firestaff/data/nexus` independently found the hash-verified
  `DM.BIN` inside the English ISO and reported Nexus READY. This proves the
  retail boot/media boundary only; stock Mednafen has no Firestaff VDP1/VDP2,
  CRAM or SLEV/SAL trace hook, so presentation and runtime-consumer gates stay
  closed.

- ✅ 2026-08-06 Nexus PRS3 retail-vector receipt refresh: corrected the real-data
  admission tests to the current `MENU.BPK` corpus and DMWeb indexed-output
  contract. Entry 1 is 16x15x1 (`240` output bytes) and remains blocked at
  `237/240`; entry 5 is 54x31x1 with a `560`-byte stream, `529` input bytes,
  `1674` output stores, output FNV `290a9d13c0224cc6` and control FNV
  `f305b1060657bb06`. The differential trial records one MSB exact frame plus
  108 trailing matches, so the simple decoder remains unproven. Focused real-
  corpus tests for admission, loader control flow, subset trace and Structure2
  ABI pass; no runtime decoder, pixel intake, palette upload or VDP1 handoff was
  opened.

- ✅ 2026-08-06 Nexus Structure1B material-owner quarantine: the retail
  LEV00–LEV15 selector census disproves direct ordinal lookup into the 15-entry
  MNS TEXT banks. Production no longer emits a valid direct
  Structure1F→Structure2 material target while the selector-transform proof is
  absent. M11's no-draw handoff path now binds the independently source-bound
  Structure3 face and Structure2 descriptor directly, preserving opaque capture
  windows without reintroducing the invalid material-owner claim. Focused DGN,
  Structure3, material-provenance and campaign-ingress checks remain green;
  Saturn selector/VDP1 semantics and all drawing remain closed.

- ✅ 2026-08-06 CI Windows warning-as-error fix: confined the POSIX-only
  case-insensitive SKSave filename variant matcher to non-Windows builds.
  Windows no longer compiles an unused helper under `-Werror`; the focused
  `dm2_v1_dynamic_creature_material_plan` target builds and its test passes.

- ✅ 2026-08-06 DM2 ZIP scan-to-launch handoff: a complete pair of
  hash-verified PC-DOS archive members now materializes to ordinary
  `asset-cache/dm2/GRAPHICS.DAT` and `DUNGEON.DAT` files. The scanner no
  longer reports both rows FOUND and then labels DM2 MISSING. Focused ZIP
  regressions cover renamed and nested-deflated members, cache paths and
  byte-identical payloads.

- ✅ 2026-08-06 M12 version-catalog capacity: raised the scanner's stored
  profile bound from seven to sixteen, matching the declared 15-profile DM1
  matrix and preventing later profiles from being silently omitted or read
  past the status array.

- ✅ 2026-08-06 CI latest-revision scheduling: restored the branch-scoped
  GitHub Actions concurrency guard with cancellation enabled. Rapid pushes to
  `main` now retain the newest full matrix instead of queuing obsolete commits
  ahead of it; pull-request runs remain isolated by ref.

- ✅ 2026-08-06 Theron raw nonstartup/bank probe paths: real-data probes now
  discover `theron/TQUS02.bin` and `theron/TQJP02.bin` directly. Nonstartup
  sector receipts pass with zero skips, and bank evidence passes with all six
  regional descriptor/span anchors plus the three US and three JP audio-bank
  IDs; the unavailable composed US ISO remains an explicit skip.

- ✅ 2026-08-06 Theron descriptor-role probe path correction: the real US/JP
  raw BIN role probe now discovers `theron/TQUS02.bin` and `theron/TQJP02.bin`
  from the standard data root. It verifies all six raw descriptor anchors,
  RTS/zero-fill boundaries and MODE1/2352 user-data bridges with `fail=0`;
  unsupported/absent ISO compositions remain explicit skips.

- ✅ 2026-08-06 Theron startup real-asset probe path correction: the receipt
  probe now discovers the supplied authentic `theron/TQJP02.bin` and
  `theron/TQUS02.bin` files instead of obsolete `theron-extras` filenames.
  Both real BIN variants now exercise the 313-pass receipt path, including
  regional bitmap routes, 32×27 startup candidate, descriptor/text/roster
  bytes and hash-bound boot profile; unsupported ISO names remain explicit
  skips.

- ✅ 2026-08-06 Theron real palette-bind coverage: the startup palette bind
  regression now discovers the supplied standard-root `TQUS02.bin` and
  `TQJP02.bin` automatically, verifies both regional hash profiles and checks
  the decoded black/white endpoints. This proves the production US/JP offset
  selection is exercised with real media; VCE ownership and semantic render
  promotion remain capture-gated.

- ✅ 2026-08-06 Theron Track 02 warning cleanup: removed unused raw-offset and
  joypad probe paths and made the POSIX canonical-path capacity contract
  explicit. `firestaff_theron` and the authentic US/JP font-tile regression
  build and pass without changing the source-bound media or admission gates.

- ✅ 2026-08-06 Theron M11 forcefield input gate: removed the premature
  startup-wide atlas rejection that returned to the launcher before keyboard
  input was dispatched. Enter now reaches the forcefield admission result;
  missing source-owned post-startup capture still blocks dungeon promotion.
- ✅ 2026-08-06 DM1 archive-backed runtime handoff: hash-discovered ZIP
  members were previously passed directly to the ordinary-file ReDMCSB
  loaders, so a real archive could be detected but not launched. M11 now
  materializes the verified DM1 `DUNGEON.DAT` and sibling `GRAPHICS.DAT` into
  the per-user runtime cache before startup. No filename-only fallback was
  added. Verification against `~/.firestaff/data/dm1`: real M564 object-name
  test passes and the 611-record object corpus passes.

- ✅ 2026-08-06 DM1 creature-name source ownership: removed M11's duplicate
  27-entry display-name table and routed runtime names through the ReDMCSB
  source-locked creature-render module. Invalid type IDs now report `UNKNOWN`
  without inventing a creature label. Verification:
  `test_dm1_v1_creature_render_pc34_compat` 14/14 and
  `m11_dm1_runtime_source_capture_receipt` pass.

- ✅ 2026-08-06 Theron M11 Enter-forcefield regression: added an end-to-end
  keyboard ACCEPT test from title → stage select → Soul Room → forcefield.
  The Enter path reaches the same startup handoff as pointer/ACTION input and
  now explicitly verifies the authentic-capture admission message when the
  local Track 02 lacks the required System Card/host capture; no fallback
  dungeon is promoted.

- ✅ 2026-08-06 Nexus ITEM.IBS floor-palette reuse: fixed the real floor-image
  renderer to resolve DMWeb's `palette_offset == 0` descriptors from the
  previously declared palette with the same palette ID, without falling back
  to palette 0. The retail regression exercises one of the 75 reused-palette
  descriptors and matches the decoder's source hash. Item action/pickup
  semantics and Saturn VDP1 presentation remain capture-gated.

- ✅ 2026-08-06 Nexus MNS runtime admission: added canonical MD5 identities for
  all 30 retail MNS files. The real readiness probe now loads
  `SCORPION.MNS` into the DMDF model pool and passes 17/17 checks; the prior
  failure was an incomplete hash catalog, not missing game data. VDP1 command
  order, creature model placement and final viewport presentation remain
  capture-gated.

- ✅ 2026-08-06 Nexus creature metadata quarantine: production creature init
  no longer inserts unverified English display labels or duplicates the MNS
  filename table. It uses the authenticated retail MNS roster, while the
  complete AI/sentinel table is verified against `DM.BIN+0x0383A8`; CRET stats
  remain bound only from retail `RLOWFIX.BIN`.

- ✅ 2026-08-06 Theron authentic regional palette receipt: registered the
  focused Track 02 palette test in CMake and made it discover
  `.firestaff/data/theron/TQUS02.bin` and `TQJP02.bin` automatically. The
  authenticated HuC6260-shaped windows are `0x2A06A0` (US) and `0x29FD70`
  (JP); the startup binder now selects the correct regional offset. Both
  strict decodes pass while semantic/VCE ownership and production rendering
  remain fail-closed. See
  `docs/source-lock/tqr_v1_track02_palette_offset_receipt_2026-08-06.md`.

- ✅ 2026-08-06 Nexus MNS roster provenance: the complete 30-entry
  `nexus_v1_creature_names` table is now checked against the mounted European
  retail `DM.BIN` string table at `0x0385F0`, rather than relying only on
  hardcoded spot-checks. The roster remains source metadata; no creature stats,
  AI semantics or model drawing were promoted from filenames.

- ✅ 2026-08-06 Theron real-data probe discovery: descriptor-table and
  level-handoff probes now use the supplied standard-root
  `.firestaff/data/theron/TQUS02.bin` and `TQJP02.bin` paths. Verification
  passes all three US and JP raw descriptor anchors and binds each authentic
  32×27 startup candidate; unresolved dungeon/object semantics remain
  explicitly no-claim.

- ✅ 2026-08-06 Theron regional level-descriptor receipt: the authenticated
  logical Track 02 span at UD `0x619900` now distinguishes the real US
  53-record table (`318` bytes, FNV-1a `7aa82bc7`) from the real JP
  zero-filled span (`318` bytes, FNV-1a `63d8ddfd`). JP cannot be decoded
  through the US table; the focused test verifies both outcomes from the
  supplied MODE1/2352 BINs. Referenced payloads and runtime semantics remain
  capture-gated.

- ✅ 2026-08-06 Nexus supplemental ISO MNS source receipt: the extracted
  European retail boot now authenticates missing `SN_FLOOR.MNS` and
  `SN_WALL.MNS` directly inside its co-located Track 1 ISO before admitting
  their bytes to the real DMDF material decoder. This closes a provenance
  mismatch between the read path and the receipt path; Structure1B material
  ownership, palettes, transforms and VDP1 drawing remain blocked.

- ✅ 2026-08-06 Nexus stale issue-page audit: marked the old
  `nexus_issues.md`, `nexus_regression.md` and `nexus_bugs.md` scaffolding/
  no-disc/no-tests claims as historical snapshots and linked the current
  strict-fidelity inventory. No runtime status was changed.

- ✅ 2026-08-06 Nexus real launch-smoke regression: the 17-pass European
  retail launch probe now also asserts that supplemental-ISO
  `SN_FLOOR.MNS`/`SN_WALL.MNS` hashes and DMDF routes are bound, while the
  real TEXT4/TABL/FONT012 receipts leave the Saturn text-consumer gate closed.

- ✅ 2026-08-06 Nexus SAL/MAP retail corpus verification: the real
  `SNDLEV00-15.SAL/.MAP` corpus passes `test_nexus_v1_sal_map_corpus` with
  16 pairs, 154 bounded eight-byte MAP records and no out-of-bounds SAL
  windows. The decoder retains 720 tone candidates as diagnostic source
  evidence; event dispatch, SDDRVS ownership and host playback remain gated.

- ✅ 2026-08-06 Nexus SLEV retail task-profile verification: `test_nexus_v1_script_vm`
  passes the complete real `SLEV00.BIN`–`SLEV15.BIN` corpus. Each file retains
  the same 36-byte SH-2 entry spine and bounds-checked in-file literal
  receipts; task-body opcodes, callback ownership and dispatch remain opaque
  and fail-closed.

- ✅ 2026-08-06 Nexus DGN corpus geometry receipt: fixed
  `nexus_v1_inspect_dgn_material_corpus()` to count the real Structure3 mesh
  extraction receipt instead of the unrelated post-grid/collision
  `geometry_info.mesh_ready` bit. The European retail corpus now verifies
  `readable=16 parsed=16 geometry=16`; floor/ceiling/wall material promotion
  and VDP1 presentation remain blocked. `firestaff_nexus_v1_dgn_material_corpus_probe`
  passes against `/Users/bosse/.firestaff/data/nexus`.

- ✅ 2026-08-06 CSB Amiga TITL.DAT palette receipt: the strict ANIM container
  reader now follows ReDMCSB `ANIM.C` F1179's ByteCount boundary and decodes
  the real `PL` step exactly as F1181's sixteen indexed Amiga 4-bit RGB
  components. The opt-in real-media test verifies the 32-frame/606-VBL title
  schedule and original palette values. `EN`/`DL` pixels remain fail-closed
  until the separate Amiga GRF1 expansion route is implemented.

- ✅ 2026-08-06 CSB Amiga TITL.DAT EN-frame receipt: `TITL.DAT` now expands
  the authentic 320×200 base `EN` image through the distinct Amiga GRF1
  command stream, preserving fills, literals and previous-line copies from
  ReDMCSB `EXPAND.C` F0466. The real-media regression proves all 64 000
  indexed pixels and the source-faithful 254-byte read, including the six
  bytes consumed from the following `DL` record because `ANIM.C` F1204 passes
  the expander only an address, not ByteCount. This does not claim `DL` delta
  frame expansion or M11 presentation binding, which remain open.

- ✅ 2026-08-06 CSB Amiga TITL.DAT partial DL-frame receipt: the first 30
  complete Amiga delta streams now apply to the preceding real frame through
  the GRF1 F1205 copy-before-draw model. The decoder preserves `0xA?` and
  `0xE?` transparent advances, replaces only commanded pixels, and supports
  literals plus previous-line copies from ReDMCSB `EXPAND.C` F0466. The
  real-media regression locks all 30 decoded frames to a final indexed-frame
  hash, rather than accepting a fixture or generated pixels. The final
  282-VBL DL remains deliberately rejected because it reads beyond the
  on-disk FTL item; `ANIM.C` F1177's exact-size, non-clearing MEM1 allocation
  provides no source-defined tail. Rejection is transactional, so the caller
  retains its preceding real frame and no partial delta reaches presentation.

- ✅ 2026-08-06 CSB Amiga TITL.DAT real-media gate: the title regression now
  verifies the registered Amiga 3.1 MD5 before it accepts an extracted file.
  The canonical `5b590ea3a6f5eed513b5678b01468ee4` member materialized from
  the supplied ADF passes; an ADF image or same-shaped non-title input fails
  before parsing. This keeps fixtures from being reported as genuine title
  evidence.

- ✅ 2026-08-06 Nexus startup FONT256/TEXTTABL receipts: the real DM.BIN
  startup regression now verifies the literal-pool pointer at `0x18BF4` to
  retail `FONT256.S2D` and the adjacent `TEXTTABL` marker at `0x294C0`.
  These strengthen loader/table provenance only; the Saturn glyph consumer
  and VDP2 placement remain capture-gated.

- ✅ 2026-08-06 Nexus HUD VDP2 owner receipt: the real `DM.BIN` HUD regression
  now verifies the adjacent `yam\\vdp2.c` owner marker at `0x38CF4` and its
  six big-endian literal references, alongside the existing `menuctrl.c`
  layout table. This is disassembly/source ownership evidence only; no VDP2
  layer, palette bank or runtime HUD presentation was enabled.

- ✅ 2026-08-06 Nexus stale-claim documentation audit: added
  `docs/NEXUS_STALE_CLAIM_AUDIT.md` and corrected the high-traffic data,
  content, intro, graphics and audio audits so historical host-stub wording
  cannot be mistaken for Saturn parity. The documents now point to the real
  retail receipts and preserve the no-draw/capture gates for startup, menu,
  HUD, viewport, text and SLEV/SAL routes. No runtime route or synthetic asset
  was enabled.

- ✅ 2026-08-06 DM1 Atari STX asset-pipeline handoff: the hash-first asset
  pipeline now accepts each of the six catalogued retail Atari ST disk-image
  identities, including STX members found inside supported archives. It
  validates the RSY v3 image, extracts the original FAT12
  `GRAPHICS.DAT`/`DUNGEON.DAT` bytes, and tags the bundle as Atari STX so a
  PC34 renderer cannot consume it accidentally. Direct-file verification
  against the real DM1 STX passed; the Atari dungeon/runtime join remains
  open in TODO.

- ✅ 2026-08-06 DM1 Atari ST IMG1 decoder: the real 563-record DMCSB1
  Atari-LZW/raw handoff now feeds a shared original IMG1/IMG2 nibble decoder.
  The decoder preserves the Atari big-endian dimensions and rejects invalid
  bounds; it does not reinterpret records through the PC34 IMG3 path. Focused
  Atari and legacy graphics tests pass. Runtime launch remains gated until
  the extracted STX record is joined to the Atari dungeon/runtime owner.

- ✅ 2026-08-06 DM1 V2 unknown-field VFX fail-closed: removed the last
  success-valued no-op fallback from the extended field effect API. Unknown
  pits/stairs/teleporter/fake-wall families now return no-draw, matching the
  ReDMCSB source boundary; authenticated V1 field bitmaps remain the only
  admitted visual owner. `test_dm1_v2_extended_field_vfx_pc34` passes.

- ✅ 2026-08-06 DM1 F0190 creature-attribute handoff: fixed the M11
  killed-all group route to resolve the immutable ReDMCSB/PC34 creature
  profile before planning possessions. The previous zero-filled attribute
  input could suppress source fixed possessions and use the wrong C040 death
  smoke attack domain; M11 now passes the profile's size/drop bits. The
  integration audit verifies the source profile and rejects the old zero
  substitute. `test_dm1_v1_f0190_c040_m11_integration_audit` and the full
  Ninja `firestaff` build pass.

- ✅ 2026-08-06 DM1 V2.1 real PC34 palette binding: replaced the viewport
  renderer's hard-coded EGA-like palette and linear shade calculation with
  ReDMCSB `VIDEODRV.C`'s six independently tuned PC34 VGA rows
  (`G8149/G8151-G8156`, `G9010_auc_VgaPaletteAll_Compat`). The indexed
  framebuffer brightness nibble now selects the corresponding source row,
  so wall, item and creature colours are source-owned at every light level.
  Updated the direct-renderer CMake test targets and pixel signature. Verified
  with `dm1_v2_source_route_state_hash_pc34`,
  `dm1_v2_launch_smoke_pc34`, `dm1_v2_viewport_materials_pc34` and
  `dm1_v2_per_mode_material_signatures_pc34` (4/4 pass).

- ✅ 2026-08-06 DM2 mixed-platform launch ownership: M12 now resolves the
  selected matched version to its own original-media owner before the M11
  handoff. A shared data root containing PC-DOS, FM Towns and Amiga media can
  no longer boot the first catalogue match when the player has selected a
  different verified edition. FM Towns and Amiga retain their user-owned ZIP
  handoff for the existing RAM-only boot readers; DOS retains the directory
  containing its hash-verified pair. The real FM Towns and Amiga M12 probes
  pass against both direct archive paths and the shared DM2 data root. No
  game data was unpacked, cached or staged.

- ✅ 2026-08-06 DM1 original PC34 save corpus backed roundtrip: both
  operator-supplied `DMSAVE.DAT` files were classified as authentic PC34
  saves, loaded through F0435 against the hash-verified original
  `DUNGEON.DAT`, exported through F0433, and loaded again through F0435.
  Party position/direction, champion roster, game tick, creature-AI count,
  active-group count and timeline count survived the roundtrip. The boot
  receipt remains fail-closed when no save corpus is configured; no save
  parts or hashes are inferred from a live dungeon. Verification:
  `test_dm1_v1_original_save_pc34_backed_corpus_roundtrip` passed with 2/2
  candidates.

- ✅ 2026-08-06 DM2 inventory HUD original-data receipts: restored and
  activated the previously missing `DRAW_HAND_ACTION_ICONS` and
  `DRAW_ITEM_SURVEY` material receipts. They now bind the exact
  `INTERFACE_GENERAL/4/dtImage/2..5` or `INTERFACE_CHARSHEET/0/dtImage/1`
  source record, source-expanded rectangle, raw payload, decoded pixels and
  16-colour local palette at both issue and consumption time. The new
  in-place PC-English regression verifies all 64 hand/side/position/direction
  combinations plus the survey frame (`identity-hash=be2f4362`); altered raw
  bytes, a different rectangle, unsupported possession or a transparency mode
  all fail closed. This adds no inventory layout, host icon or generated
  pixels: M11 inventory presentation remains unavailable until its complete
  original event/layout route is bound. Source: SKProject
  `SKWIN/c_gui_draw.cpp::DM2_DRAW_HAND_ACTION_ICONS` (2341–2386) and
  `DM2_DRAW_ITEM_SURVEY` (2072–2106).
- ✅ 2026-08-06 Nexus startup surface raw-byte provenance: the real
  TITLE.CG, WARNING.BIN, GAMEOVER.BIN and STABG.BIN loaders now retain an
  FNV-1a-64 receipt and exact source-byte size on each decoded surface.
  `test_nexus_v1_startup_media_gate` asserts the receipt against the retail
  files in `.firestaff/data/nexus`. This is provenance only; the Saturn
  VDP1/VDP2 capture gate and no-draw presentation boundary remain unchanged.

- ✅ 2026-08-06 Nexus LOGOBG.DG2 source decode: the real 72,198-byte PP
  startup layer now loads into its own indexed 320×224 UI surface, preserving
  all 256 big-endian BGR555 palette words, RGBA expansion, palette receipt and
  raw-byte provenance. It remains optional and no-draw until VDP2 layer and
  placement capture proves the original startup composition.

- ✅ 2026-08-06 Nexus STONE.BIN image-local PP decode: implemented the
  missing documented `nexus_palette_stone_pp_receipt()` and
  `nexus_palette_decode_stone_pp_record()` APIs. The real 4,400-byte corpus
  verifies eight 550-byte `pp` records, each 32×32 with 16 big-endian BGR555
  words and 512 packed 4bpp bytes; selected records decode into caller-owned
  buffers without global palette promotion. `test_nexus_v1_raw_bin` verifies
  the retail corpus.

- ✅ 2026-08-06 Nexus manifest revision correction: the live-data verifier
  now accepts only the documented SHA-256-authenticated English/French
  `MENU.BPK` revisions and English `RLOWFIX.BIN` revision when their sizes
  differ from the original canonical extraction. The stale 4,096-byte
  `STONE.BIN` description is corrected to the real eight-record 4,400-byte
  corpus. Container-only files remain reported as missing rather than being
  falsely treated as loose-file verification passes.

- ✅ 2026-08-06 DM2 M10 IMG9 decoder ownership: corrected a link boundary
  exposed by the full real GDAT census. `dm2_v1_asset_loader.c` is owned by
  `firestaff_m10`, but its source IMG9 decoder had only been emitted through
  the higher-level DM2 archive. The decoder is now compiled with the loader,
  so bounded M10-only real-data consumers do not fail unresolved on
  `dm2_v1_decode_img9`; the duplicate DM2 archive object is removed. Verified
  with the GDAT creature-table, CCM source-alignment and entire PC-English
  visual-corpus regressions. No image substitute or data materialization was
  introduced.

- ✅ 2026-08-06 Theron fresh FIFO-origin capture audit: rebuilt the patched
  Mednafen binary against real SDL 2.32.8 and ran authenticated US Track 02
  media. The replay receipt records 161 raw sector spans, 51 SCSI reads, 25
  CD IRQs and 4,096 main-RAM consumer reads, while the six `$e009` windows
  contain zero game-owned CD data reads. The two FIFO-origin rows are bounded
  BIOS/CD-routine reads at `$21e7`/`$21e9`, so no `$2600`, object, level, tile,
  palette, HUD or viewport semantics were promoted. Cocoa/Quartz input also
  proved eight host events reached the emulated polling path without changing
  that result. The capture script now accepts `run` as an alias for physical
  `return`/Run input.

- ✅ 2026-08-06 DM2 full PC-English visual corpus census: added the real-data
  `test_dm2_v1_gdat_visual_corpus_real_data` regression. It reads the mounted
  `GRAPHICS.DAT` only, walks every exact `dtImage` ENT1 row and decodes each
  unique RAW payload directly through the source IMG3/U4/U8/IMG9 routes. The
  verified corpus contains 5,676 image rows, 4,031 distinct image payloads
  and 18,633,937 decoded source pixels; all pass without a cache, substitute
  image, data extraction or fallback. Greatstone's 5,624 exported visual-item
  count remains documented as a different presentation catalogue domain.

- ✅ 2026-08-06 DM2 IMG9 real-data decoder dispatch: the active GDAT loader
  no longer treats every non-mode-2 C8/IMG9 payload as mode 3. It delegates to
  the complete SKProject `c_gfx_decode.cpp::decode_img9` port and accepts only
  original modes 1, 2 and 3. The real DOS `GRAPHICS.DAT` G1 graphics-set gate
  now passes all five referenced styles and rejects selector 0. Greatstone's
  PC 1.0 English catalogue was cross-checked without copying game data:
  5,624 exported visual items are correctly recorded as a different domain
  from the authenticated file's 11,854 ENT1 rows. See
  `docs/reference/audits/DM2_PC10_EN_GREATSTONE_CROSSCHECK.md`.

- ✅ 2026-08-06 DM2 M11 real-data gate environment alignment: the M11
  startup/profile regression now recognizes `FIRESTAFF_DM2_DATA_DIR`, matching
  the shared PC-DOS corpus convention used by boot, GDAT and SKSave probes.
  With `/Users/bosse/.firestaff/data/dm2/dos_extract/data`, the gate reaches
  the verified source startup boundary. Its no-environment fallback now points
  directly at the mounted PC-DOS owner directory, and a watchdog expiration
  exits nonzero rather than being misreported as a passing skip. This changes
  verification discovery only; no synthetic profile or runtime route was
  enabled.

- ✅ 2026-08-06 DM2 SKProject function-coverage audit: reconciled the old
  “31 missing functions” report with the current named-symbol audit. All
  `DM2_SOUND1`–`DM2_SOUND7`, applicable `c_move.cpp`, and source-owned
  `c_map.cpp` entries are `IMPLEMENTED_PARITY`; the remaining `c_dialog.cpp`
  and `c_eventqueue.cpp` rows are explicitly `NOT_APPLICABLE_ARCH` under the
  M11 UI/event architecture, not production fallbacks. The audit contains
  1,118 `IMPLEMENTED_PARITY`, 73 `NONAPPLICABLE`, and 560
  `NOT_APPLICABLE_ARCH` rows. No disabled callback/test transcript was
  promoted into a playable DM2 route.

- ✅ 2026-08-06 DM2 `c_eventqueue` source-edge correction: the retained
  test-only queue transcript now preserves `QUEUE_EVENT`'s saturated `0x02`
  one-shot effect on the following `0x04` capacity, the source 7/9 entry
  limits, and `QUEUE_0x20`'s seven-entry cap without inventing a zero y
  coordinate. It also distinguishes `init()`'s zero sentinels from
  `event_1031_098e()`'s `-1` flush reset. No callback event queue is linked
  into M11 or production DM2 input. Verification:
  `test_dm2_v1_eventqueue_pc34_compat` PASS. Source:
  `SKULLWIN/c_eventqueue.cpp::init`, `QUEUE_EVENT`, `QUEUE_0x20`.

- ✅ 2026-08-06 DM2 c_1c9a callback placeholder removal:
  `DM2_1c9a_09b9` now implements the exact SKProject
  `c_1c9a.cpp:5404-5413` DB4 record-link predicate: resolve the low-16-bit
  record through its owner callback, read source word `+8`, and compare it
  with the low-16-bit argument. Missing owners remain false. The source is
  still excluded from the production AI archive until the surrounding
  DB4/CAII/CCM ownership chain is complete, so this introduces neither an AI
  fallback nor synthetic creature state. The focused regression passes
  54/54.

- ✅ 2026-08-06 Nexus WARNING DGT2 production quarantine: removed the direct
  WARNING.BIN M11 presentation/resource callback modules from the
  `firestaff_nexus` production library. They remain explicit real-data test
  sources for byte/CLUT regression, while no exported production route can
  copy decoded warning pixels into a host framebuffer without Saturn capture.

- ✅ 2026-08-06 Nexus capture producer preflight: the PRS3, VDP1 and SLEV/SAL
  Mednafen launchers now require their advertised Firestaff output hook before
  writing a manifest or starting an external run. Stock Mednafen exits 78 with
  an explicit instrumented-build message and leaves no misleading artifact.

- ✅ 2026-08-06 Nexus startup pointer provenance gate: M11 no longer accepts
  the planner's fixed save/champion hit rectangles as live Saturn input. Save
  selection, roster changes and the champion START footer now require the
  route-specific capture and exact input-matrix receipt; compatibility
  geometry remains available only to isolated tests.

- ✅ 2026-08-06 Nexus scene-owner fallback removal: removed the production
  heuristic that selected the first bounded Structure3 model when no active
  Structure1F face owner existed. Real LEV00 adjacent-cell facts still parse,
  while geometry, material and M11 promotion remain blocked until the
  source-owned Structure1F → Structure1A → Structure3 chain is bound.
  Added `docs/NEXUS_RUNTIME_CAPTURE.md` to document the artifact boundary and
  the difference between ordinary video evidence and VDP/runtime provenance.

- ✅ 2026-08-06 Nexus visible-owner selection: scene planning now filters
  Structure1F candidates by their authenticated Structure1A owner coordinates
  against the active party/forward/left/right cells. Real `LEV01.DGN` still
  finds a positive owner chain across the camera scan; arbitrary off-screen
  faces are not selected for the no-draw geometry plan.

- ✅ 2026-08-06 Nexus M11 startup capture gate: removed the remaining direct
  `TITLE.CG` and `WARNING.BIN` host-framebuffer/palette copies from the M11
  startup executors. The real European ISO still loads and advances the
  source-backed timing/receipt state, while the framebuffer stays no-draw until
  an original Saturn VDP1/VDP2 destination and CLUT capture is bound.

- ✅ 2026-08-06 Nexus item-use provenance gate: removed the remaining DM1-derived
  food/potion/status mutations and fallback magnitudes from the exported item
  API. Real `ITEM.IBS` declarations remain available for source/material
  receipts, but no item is advertised or consumed until the Saturn
  action/event consumer is captured. `test_nexus_v1_item_use` now proves that
  unbound declarations leave champion and status state unchanged.

- ✅ 2026-08-06 Theron executed HuC6280 consumer window: added a strict
  code-fetch verifier for the authentic Mednafen main-RAM sidecar and recorded
  the real `$2c54–$2c69` instruction window in
  `docs/source-lock/theron-main-ram-consumer-disassembly-2026-08-06.md`.
  The regression rejects a mutated window and requires logical/physical
  reader-PC equality for every byte. This proves executed code provenance, not
  level/object/tile/palette meaning; semantic publication remains blocked.

- ✅ 2026-08-06 Theron raw VDC/VCE exact-size admission: tightened the
  in-memory snapshot loader to require exactly 65,536 VRAM bytes and 1,024 VCE
  bytes, matching the file-backed capture boundary. Oversized concatenated
  buffers are now rejected, with regression coverage; the authenticated BAT
  preview and all semantic no-draw gates remain unchanged.

- ✅ 2026-08-06 Nexus startup title host no-draw gate: the public
  `nexus_render_title` entry point no longer copies authentic WARNING.BIN or
  TITLE.CG pixels/CLUT into a host framebuffer without a verified Saturn
  VDP1/VDP2 capture binding. The real startup media tests still decode and
  provenance-check those source bytes; presentation remains explicitly
  capture-gated.

- ✅ 2026-08-06 CSB FM Towns Game-session provenance gate: the F31E CDATA
  pair now has an opt-in real-media regression that scans the exact
  `405b757038eea3c263e60f240854d6de` GRAPHICS.DAT and
  `83c56cf1b779e7460a55c9299ebeb04b` DUNGEON.DAT identities before opening
  the startup session. It proves C001--C005 entrance and C017/C040 HUD
  surfaces are decoder-bound to original FM Towns media, rather than an
  identically named PC, Atari or Amiga cache file. `CHTWE.EXP` presentation,
  audio and save ownership remain capture-gated.

- ✅ 2026-08-06 CSB FM Towns F31J runtime handoff: a selected CJDATA runtime
  cache now keeps its verified top-level GRAPHICS.DAT/DUNGEON.DAT pair ahead
  of the preserved CDATA sidecar. This prevents the recursive boot scan from
  silently changing the Japanese F31J profile into F31E. The opt-in original
  CD regression proves `TITLE.ANM` → `SWITCHTW` → `CHTWJ.EXP` → C004,
  C002/C003 door opening, C017 HUD and F0128 live viewport. ReDMCSB
  `COMPILE.H` EXEID60/61 is the program-ownership reference; audio/CDDA,
  Utility, ending and save handoff remain explicitly open.

- ✅ 2026-08-06 Nexus HUD layout envelope gate: the real DM.BIN 80-entry
  `menuctrl.c` layout parser now rejects non-sentinel coordinates outside the
  Saturn 320×224 display envelope, matching the existing hit-rectangle
  admission boundary. The authentic 80-entry layout and 40-entry hit table
  both pass with the local European corpus; no host/off-screen or synthetic
  placement can enter the HUD receipt.

- ✅ 2026-08-06 Nexus documentation provenance correction: updated the
  source-locked data audit so real LEV00–LEV15 Structure1B/Structure3
  receipts, 16-file SLEV task profiles, SAL DataID-0/MAP metadata, the
  authenticated 26,610-byte SDDRVS.TSK identity and the 20-record FACE.BIN
  corpus are no longer incorrectly documented as “not parsed” or hypothetical.
  The documentation preserves the actual Saturn-capture gates: no SLEV event
  semantics, SAL playback, portrait placement or DGN/VDP1 presentation is
  claimed from bounded file parsing alone.

- ✅ 2026-08-06 Nexus viewport handoff return status: the verified legacy
  DGN material route now returns success from
  `nexus_viewport_dgn_host_route_receipt` when its receipt is
  `ready-rendered-mesh`. Previously the receipt admitted presentation but
  returned API failure, so launcher/M11 callers discarded an otherwise valid
  viewport. Saturn-capture-gated Structure3/no-draw routes remain unchanged.

- ✅ 2026-08-06 Theron VDC BAT binding retention: the authenticated 64 KiB
  VRAM/1 KiB VCE viewport mount now persists each admitted BAT word's
  source tile/palette atlas index instead of discarding the mapping after
  tile population. Duplicate tile/palette pairs share the same real atlas
  entry, invalid cells remain `-1`, and a public query exposes only the raw
  BAT receipt. The loader regression and real Mednafen snapshot test pass;
  no dungeon-square, object, or synthetic rendering semantics were inferred.

- ✅ 2026-08-06 Nexus extracted-file provenance gate: canonical filenames no
  longer outrank identity hashes in `nexus_v1_read_extracted_file`. A wrong
  `DM.BIN`, FACE, SLEV, SAL/MAP or other known-name payload is rejected and a
  hash-verified renamed retail file is selected instead; unknown DMDF-family
  model discovery remains unchanged. This closes the direct filename
  injection path used by startup, HUD and level auxiliary loaders.

- ✅ 2026-08-06 Nexus interaction provenance gate: production
  `NEXUS_CMD_INTERACT` no longer toggles DGN-derived switches, opens
  containers, drinks from fountains, or performs altar rituals while the
  Saturn action dispatcher is uncaptured. The isolated data modules remain
  available for parser tests; the runtime tick regression now proves real
  switch/container state stays unchanged until an authenticated action trace
  binds the semantics.

- ✅ 2026-08-06 Nexus PLRD footer provenance cleanup: the champion-start
  footer no longer injects the host English `PARTY/ACCEPT/ADD/ACTION/START`
  string for real European RLOWFIX/PLRD records. It remains available only
  for the isolated ASCII compatibility roster; the real PLRD regression now
  requires an empty footer until Saturn TEXT/FONT256 placement is captured.

- ✅ 2026-08-06 Nexus level-source receipt correction: engine and launcher
  startup/resume receipts now retain the exact hash-resolved `LEV##.DGN`
  source path, including renamed extracted files and ISO `::member` paths,
  instead of reconstructing a misleading canonical filename. The real boot
  hash regression verifies the renamed level path survives the runtime handoff.

- ✅ 2026-08-06 Nexus HUD rectangle admission: the DM.BIN parser now rejects
  non-empty zero-width/zero-height regions while preserving the retail
  table's exact all-zero unused entries. The real 40-entry HUD corpus and
  malformed-region regression both pass; no hit-region semantics or pixels
  are promoted beyond the source-bound raw table.

- ✅ 2026-08-06 Nexus DGN actor CRET provenance gate: a Structure1A/3 model
  signature no longer promotes an actor to a live creature type when its
  RLOWFIX CRET record is absent. Such actors remain untyped/idle instead of
  receiving zero-health roster defaults; authenticated CRET data still binds
  the normal production path. This closes a real-data-to-viewport/runtime
  leak without changing the isolated fixture `spawn_on_level` route.

- ✅ 2026-08-06 Theron VDC capture preview: added an explicit raw-BAT preview
  that copies only authenticated VRAM tile pixels through the retained
  BAT→atlas binding into the production framebuffer. The real snapshot test
  now proves nonzero preview pixels, while world-driven dungeon/HUD drawing
  remains blocked until the HuC6280 consumer mapping is proven.

- ✅ 2026-08-06 Theron VDC snapshot intake hardening: file-backed VRAM/VCE
  mounting now requires exactly 65536 and 1024 bytes respectively, matching
  the native Mednafen capture contract. Oversized or concatenated snapshots
  are rejected before any real palette or tile data is mounted.

- ✅ 2026-08-06 Nexus startup PLRD label provenance cleanup: authenticated
  RLOWFIX/PLRD champion rows no longer receive a host-generated ASCII/HP/MP
  label when the retail record carries only TABL/FONT256 glyph codes. The
  source glyph sequence remains available for the future Saturn text capture;
  legacy labels remain limited to compatibility fixtures. The real PLRD test
  now asserts that no host label is emitted.

- ✅ 2026-08-06 Nexus level provenance gate: removed the filename-only
  `LEV%02d.DGN` fallback from `nexus_v1_game_load_level`. Level state now
  accepts only an MD5-resolved Saturn payload, while renamed authentic bytes
  remain supported. The boot/hash regression now proves that wrong bytes under
  the canonical `LEV00.DGN` name are rejected before entering game state.

- ✅ 2026-08-06 Nexus ITEM raw-declaration provenance cleanup: the legacy
  40-byte ITEM.IBS binder now retains `carry_locations` as source metadata
  without inferring a consumable gameplay flag from bit 0. The authenticated
  `Nexus_V1_ItemIbsBank` route was already fail-closed and remains unchanged;
  a regression covers the raw route, and the real ITEM.IBS/inventory suite
  passes 67 gameplay checks plus the decoder corpus. No item action or armor
  semantics were promoted.

- ✅ 2026-08-06 Nexus SCR no-draw boundary cleanup: removed the unreachable
  flat-glyph decode/raster branch from `nexus_v1_screen_text_draw_s2d_bytes`.
  Real `FONT256.S2D` input now has one explicit, auditable `GLYPH_MAP`
  failure path until Saturn page/tilemap/attribute evidence is bound; the
  isolated synthetic `draw_indexed` layout probe remains unchanged. The
  screen-text surface probe passes 27/27 and real input leaves its framebuffer
  untouched. No game data was tracked.

- ✅ 2026-08-06 Nexus shop catalog provenance: the runtime shop manager now
  binds the eight `(item_id, price)` rows and `0xFFFF` terminator directly
  from hash-verified `DM.BIN` at `0x037210` instead of relying only on a
  detached C table. Real-data coverage verifies every row and engine startup
  exposes the bound catalog; legacy unbound manager tests retain their
  compatibility lookup. No item action/combat semantics were inferred.
- ✅ 2026-08-06 CSB FM Towns Switch executable inventory: the real
  `SWITCHTW.EXP` resource chain now supplies both original 320x200 Switch
  pages plus all four registered button images, with their actual IMG2 stream
  boundaries, `C26_SWITCH` palette and ReDMCSB `SWITCH.C` F2279/F2280
  coordinates retained. The parser rejects unrelated 320x200 executable
  streams. M11 now transfers from the completed source-owned `TITLE.ANM`
  playback into the `AUTOEXEC.BAT` `SWITCHTW JAPAN` route, preserves
  SWITCH.C's sixty-VBlank reveal, and displays the executable's authenticated
  palette/page/button pixels. M11 also reproduces Switch's language toggle and
  starts the selected-language `STORY.ANM` handoff, returning to the matching
  Switch loop at animation EOF. The focused original-media Switch test passes
  18/18 and `firestaff_m11` builds successfully.
  payloads unless the complete F31E/F31J sequence decodes. Its four source
  rectangles now retain the original language-dependent exit statuses for
  `AUTOEXEC.BAT` Story, utility, game and language-toggle handling. The M11
  Story route is implemented; Utility and Game remain intentionally unbound
  until their distinct CEDT/Game executables have a source-captured handoff.

- ✅ 2026-08-06 Nexus Saturn BGR555 channel-order correction: the real
  `SMAP00-15.BIN`, `FACE.BIN`, `ITEM.IBS` and `.MNS` palette decoders now map
  Saturn BGR555 bits 14..10/9..5/4..0 to host R/G/B consistently with the
  shared VDP1 palette path. Added a synthetic asymmetric-word regression and
  reran all 16 real SMAP decodes, 20 FACE portraits, 243 ITEM declarations,
  30 MNS models and the SMAP runtime binding. No capture or fallback gate was
  opened; no game data was tracked.

- ✅ 2026-08-06 DM1 generic dungeon ornament placeholder removal: the legacy
  wall/floor ornament bridge no longer returns zero for every map square. It
  now parses the real PC34 map ornament counts and source metadata, applies
  the original F0169/F0170 seed/formula and face/type gates, and returns the
  verified local ordinals used by the source renderer. The real
  `Dungeon-Master_DOS_EN_Version-34.zip` `DUNGEON.DAT` receipt verifies
  wall ordinal 3 at map 0 `(13,8)` and floor ordinal 3 at `(4,2)`; no game
  data was tracked. Sensor overrides and external Mac/app capture remain in
  `DM1-HOC-ORNAMENT-RENDER-CAPTURE`.

- ✅ 2026-08-06 DM1 direct-loop synthetic start removal: `fs_game_load_assets`
  no longer continues after missing, incomplete or unparsable DM1 media and
  no longer installs the fixed `(11,29,N)` Hall-of-Champions fallback. The
  direct loop now enters only after the source `DUNGEON.DAT` parser supplies
  the initial party pose; failure is reported as a startup data error. Full
  macOS viewport/HUD capture remains tracked separately in
  `DM1-DIRECT-LOOP-CAPTURE`.

- ✅ 2026-08-06 DM1 host-font fallback removal: source-backed DM1 text no
  longer silently falls back to Firestaff's built-in 5x7 diagnostic font when
  M653 is unavailable. The real PC3.4 object corpus test now requires the
  authenticated original font as well as the 611 source-backed names/icons;
  the real corpus passes. Missing source font material is now no-draw instead
  of a visually different replacement. Broader text capture remains tracked
  in `DM1-M653-FONT-CAPTURE`.

- ✅ 2026-08-06 Nexus ITEM equipment placeholder removal: deleted the old
  `20..26` item-ID armor mapping and the unknown-armor-to-torso fallback from
  the inventory helper. ITEM.IBS declarations remain source-owned, while
  armor-slot mutation now fails closed until Saturn action/slot evidence is
  captured. Verification: real ITEM.IBS inventory regression and production
  action gate remain green; no game data was tracked.

- ✅ 2026-08-06 Nexus startup FACE receipt hardening: both launcher full-start
  receipt paths now require `faces_loaded == faces_expected` and zero fallback
  portraits. The former arithmetic `loaded + fallback == expected` could mark
  incomplete source coverage as real-ready. Verification: explicit 19/20
  partial-coverage blocker plus real European startup media/menu tests pass.

- ✅ 2026-08-06 Nexus FACE startup-count placeholder removal: production
  startup loading no longer uses the inherited 24-portrait bound. It now
  checks the authenticated 20-record FACE.BIN layout from the real European
  corpus, so the loaded startup set is exactly the source-owned 20×56×56
  portraits. Verification: real startup media gate and startup-menu runtime
  test pass; no game data was tracked. Saturn VDP1 placement remains gated.

- ✅ 2026-08-06 Theron real VDC/VCE capture receipt: the instrumented native
  SDL2 Mednafen build now snapshots the authentic US Track 02 session at exit
  into exact 65,536-byte VDC VRAM and 1,024-byte VCE palette-RAM files. The
  real-data loader test verifies 8,315 non-zero VRAM bytes, 123 non-zero VCE
  bytes, 219 BAT tile bindings and 512 palette entries. No snapshot or game
  media is tracked; the receipt remains opaque and does not admit guessed
  viewport, object or level semantics.

- ✅ 2026-08-06 Theron production capture binding: the production viewport
  lifecycle now accepts the authenticated VDC-VRAM/VCE snapshot pair through
  explicit `FIRESTAFF_THERON_VRAM_SNAPSHOT` and
  `FIRESTAFF_THERON_VCE_SNAPSHOT` paths, loads the real BAT/tile and palette
  data, and releases it through the normal viewport owner. The real-capture
  test now exercises this production init path; square-to-tile, object/level,
  and dungeon/UI rendering remain fail-closed pending the HuC6280 consumer.

- ✅ 2026-08-06 Nexus MENU.BPK receipt aligned with the real European retail
  corpus: the structural probe now recognizes the verified canonical,
  English and French archive identities, checks the observed 536-byte
  outer/header delta, and compares the directory trailer against the live
  final offset-table entries instead of stale hardcoded offsets. Verification:
  real 87,684-byte `MENU.BPK`, 51/51 probe checks. No game data was tracked;
  menu VDP1/VDP2 drawing remains capture-gated.

- ✅ 2026-08-06 Nexus PLRD/TABL source-name handoff: the real 20-record
  European RLOWFIX corpus now propagates each champion's bounded FONT256 glyph
  codes (TABL indices, stopping at the verified `0x0005` terminator) into the
  startup row model. No ASCII/JIS name is invented; visible text remains
  capture-gated at the Saturn VDP2 consumer. Verification: real PLRD/TABL
  regression plus startup-row glyph assertions; no game data was tracked.

- ✅ 2026-08-06 Nexus FACE source-owned startup decode: the real European
  `FACE.BIN` records now pass the DMWeb 64-entry BGR555 + PRS3 decode contract
  and load as 20 indexed 56×56 UI surfaces. The host portrait entry point
  remains no-draw because Saturn VDP1 destination, scale, flip and command
  order are still uncaptured. Verification: all 20 real portraits decode and
  load, plus the Nexus startup-media gate; no game data was changed or tracked.

- ✅ 2026-08-06 Nexus FACE invalid-layout fallback removal: rejected or
  incomplete FACE.BIN bytes no longer report the legacy synthetic 48×48
  portrait geometry. Valid DMWeb FACE.BIN data still reports its authenticated
  20-entry, 56×56 PRS3 layout; invalid input now remains 0×0 and cannot leak a
  portrait size into startup diagnostics. Verification: real startup-media
  gate and M11 Nexus startup gate pass against the European Nexus corpus.
  No game data was changed or tracked.

- ✅ 2026-08-06 DM1 synthetic damage-frame removal: authenticated PC34 DM1
  frames no longer draw Firestaff's host-made red viewport border when a
  champion is hit. ReDMCSB's damage feedback remains source-owned through the
  C015/C016 champion-panel redraw and source audio; the border is retained only
  in the unauthenticated diagnostic renderer. Verification: `firestaff` builds,
  `test_m11_dm1_damage_indicator_source_gate`, the 142-case champion layout
  test, the 283-case sound integration test, and the real 611-record object
  corpus pass. An unrelated existing parry assertion in
  `test_m11_rest_runtime_pc34_compat` remains open.

- ✅ 2026-08-06 DM1 damage-number palette correction: the original-font
  damage-number route now uses PC34 `C15` foreground with `C08` red
  background, matching ReDMCSB `CHAMDRAW.C F0623` instead of the old orange
  host palette slot. Verification: Firestaff builds and the focused DM1
  damage, champion-layout, and sound tests pass.

- ✅ 2026-08-06 DM1 G0237 object-aspect tail restoration: restored the four
  missing ReDMCSB rows for junk subtypes 49..52 (`77, 78, 74, 41`), covering
  Lock Picks, Magnifier, Zokathra Spell and Bones. The previous 176-value
  initializer for the declared 180-row table silently zero-filled these
  records and could select unrelated object art. The real PC3.4 corpus test
  now asserts the four tail aspects and passes all 611 object records.

- ✅ 2026-08-06 DM1 G0237 object-aspect alignment correction: compared the
  complete `kObjectInfoAspect[180]` sequence against ReDMCSB `G0237` row by
  row. Four missing `62` entries for the Emerald, Ruby, Ra and Master Keys
  had shifted Boulder and every later junk/object record, so appending the
  final four values alone was insufficient. Restored the exact 180-row
  sequence and expanded `test_m11_dm1_real_object_corpus` to assert the
  affected key/object tail (subtypes 21..52). Verification: direct source
  comparison 180/180 with zero mismatches; Ninja target build passed; the
  real PC3.4 corpus passed all 611 records.

- ✅ 2026-08-06 DM1 PC34 dungeon-map offset correction: replaced the legacy
  generic `DUNGEON.DAT` reader's EOF/fallback raw-map offset with the real
  ReDMCSB/DMWeb layout: 44-byte header, 16-byte MAP descriptors, cumulative
  column SFT bases, square-first-thing table, text words, and G0235 thing
  records. Map bytes are now read from each descriptor's raw offset in
  column-major order, with the lower five attribute bits retained for door
  state. This prevents object records from becoming false walls/doors and
  removes the hardcoded fallback map interpretation. Verification: extracted
  PC3.4 `DUNGEON.DAT` passed 14-map, 18x19 map-0, start-position, door-type,
  and door-state checks in `test_firestaff_dm1_dungeon_state_real_data`; the
  no-argument CTest form remains skip-safe when original data is unavailable.

- ✅ 2026-08-06 DM2 Amiga nested-media intake: the real-media receipt no
  longer shells out to `unzip` or `bsdtar`. A bounded ZIP reader now accepts
  an already-resident ZIP byte buffer, so the supplied outer archive, its
  original M3 disk ZIPs and their ADFs are traversed entirely in memory before
  the six LZX parts are joined. Stored and deflated entries retain the same
  strict bounds checks as the regular ZIP reader. Verification runs the full
  outer-ZIP → disk-ZIP → ADF → LZX → GRAPHICS/DUNGEON/CD chain against the
  real supplied archive. No game data was unpacked, copied or tracked.

- ✅ 2026-08-06 DM2 Amiga LZX in-memory decoder: implemented the bounded
  64 KiB-window LZX solid-stream decoder required by the authentic Amiga
  installer archive. It honors the original swapped-byte bitstream, block
  modes, delta Huffman tables and per-entry CRC before releasing any decoded
  bytes to the caller. The real six-disk corpus now decodes `GRAPHICS.DAT`
  (3,493,879 bytes), `DUNGEON.DAT` (39,411 bytes) and `CD.DAT` (176 bytes) in
  RAM; `CD.DAT` also reaches the existing original MOD-map parser. The media
  remains non-launchable until M12 applies its existing MD5 identity gate to
  these in-memory outputs. No game data was unpacked, copied or tracked.

- ✅ 2026-08-06 DM2 Amiga installer-media index: Firestaff now joins the
  authentic `dm2_arcsplit1`…`dm2_arcsplit6` corpus strictly in RAM and parses
  its original `DM2_archive.LZX` index without extracting or publishing a
  file. The real supplied six-disk corpus verifies 35 entries and locates the
  original `GRAPHICS.DAT` (3,493,879 bytes), `DUNGEON.DAT` (39,411 bytes),
  `CD.DAT` and `music/SK00.MOD`…`SK09.MOD` receipts. The corpus stays
  non-launchable: compressed LZX payloads must still be decoded in memory and
  the resulting GRAPHICS/DUNGEON pair must pass the original hashes. The
  real-media test streams nested ADFs through RAM only; no game data was
  unpacked, copied or tracked.

- ✅ 2026-08-06 Nexus menu inventory hardening: removed the DM1/CSB-derived
  menu-flow diagram and unsupported 24-champion claim from `docs/nexus_menu.md`.
  The document now records only the verified startup assets, DM.BIN hit-table
  handoff and 20 PLRD records; retail menu order, text consumption and
  Saturn VDP1/VDP2 composition remain capture-gated. No runtime behavior or
  game data was changed.

- ✅ 2026-08-06 Nexus startup label hardening: M12 no longer advertises
  `V1 / V2` for Nexus while its V2 HUD is still a procedural no-op and the
  Saturn presentation capture is missing. The game card now states
  `V1 / SATURN CAPTURE GATED`. The boot-readiness regression now also checks
  that the unbound prompt remains closed; no runtime pixels or game data were
  changed.

- ✅ 2026-08-06 Nexus startup title geometry: replaced the production
  320x200 reveal-height assumption with `NEXUS_FB_H` (224), matching DMWeb's
  64x28 TITLE.BIN map geometry and the verified Saturn HUD envelope. Added a
  320x224 regression; VDP2 tilemap/CLUT ownership remains capture-gated.

- ✅ 2026-08-06 DM2 platform documentation: corrected the obsolete claim that
  DM2 had no Amiga release. The variant guide now records the documented
  European 1.0 EN/FR/DE port, its six-floppy installer and original
  `dm2_arcsplit1`…`dm2_arcsplit6` → `DM2_archive.LZX` → `unlzx` installation
  order. It explicitly keeps the corpus non-launchable until that operation
  can be reproduced entirely in memory and a matching original data pair is
  hash-verified. Sources: DMWeb and Greatstone; no game data was extracted,
  copied or tracked.

- ✅ 2026-08-06 DM2: retired the obsolete `dm2_v1_wall_door_local_palette_gate`
  fixture. It fabricated wall and door plans, pixels, palettes, RAW receipts,
  and destination geometry, and had become incompatible with the source-owned
  RAW4 route: a source-required door now obtains its placement from the real
  GDAT loader before it can draw. The gate could therefore neither prove the
  current renderer nor exercise genuine game data. The maintained real-media
  tests `test_dm2_v1_gdat_wall_plan_viewport_real_data` and
  `test_dm2_v1_gdat_door_overlay_plan_real_data` cover the actual
  `GRAPHICS.DAT` command/palette receipts and their fail-closed M11 consumers.
  No production fallback was introduced.

- ✅ 2026-08-06 Nexus SFX provenance wording correction: `NEXUS_SFX_*`
  enum names and diagnostic labels are now explicitly host-side requests,
  not claimed retail `SNDLEV##.MAP` event IDs. Runtime selector binding and
  playback remain fail-closed pending authentic SLEV/SDDRVS dispatch capture.

- ✅ 2026-08-06 Nexus DGN real texture census guard: the hash-verified
  `LEV00.DGN`–`LEV15.DGN` regression now requires the exact 1,678 decoded
  Structure2 images (1,553 indexed-4bpp and 125 direct-555) and aggregate
  non-zero pixel/palette output. This prevents a zero-filled diagnostic buffer
  from masquerading as texture coverage while keeping source verification and
  VDP1 promotion separate. Verification: `test_nexus_v1_dgn_texture_decode`.

- ✅ 2026-08-06 Nexus European BIOS capture preflight: the supplied
  `Sega Saturn BIOS (E) (1.00).bin` was verified as 524,288 bytes with
  SHA-256 `96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`.
  Mednafen 1.32.1 accepts it with forced European region settings and boots
  the authentic merged English Nexus cue/ISO to `T-9111G` / `DUNGEON MASTER
  NEXUS`. The BIOS remains temporary and untracked; no pixels or VDP1/VDP2
  ownership are promoted from the stock capture.

- ✅ 2026-08-06 Nexus HUD rectangle envelope hardening: the DM.BIN-derived
  40-entry parser now rejects signed negative origins as well as inverted or
  out-of-screen corners. The real 320x224 table remains unchanged and the
  regression checks the viewport/compass/movement-pad coordinates plus both
  malformed classes. Verification: `test_nexus_v1_hud_hit_rects` passes with
  the real `/Users/bosse/.firestaff/data/nexus/DM.BIN`; no game data added.

- ✅ 2026-08-06 Nexus retail MAP minimum-size admission: `SNDLEV##.MAP`
  parsing now recognizes DMWeb's byte-zero retail record table even when the
  file contains only one eight-byte record and its `FF FF` terminator; the
  legacy 24-byte fixture grammar remains isolated. Added a regression that
  preserves the opaque DataID/selector and bounded SAL window without
  inventing event semantics. Verification: `test_nexus_v1_sal_map_corpus`
  passes against all 16 real SAL/MAP pairs plus the minimal retail case.
  No game data was added to the repo.

- ✅ 2026-08-06 Nexus MNS DMDF envelope gate: `nexus_v1_mns_decode` now
  requires DMWeb's exact DMDF block/file size and validates the declared MOTN
  and TEXT section spans before accepting animation tables or texture pixels.
  TEXT descriptors and pixel payloads must remain inside TEXT; malformed
  prefixes are rejected. The real `/Users/bosse/.firestaff/data/nexus` corpus
  remains intact: 30/30 MNS models decode, 815 source textures render, and
  SCORPION/ROCKPILE/VEXIRK/D_GOLD retail-count assertions pass. Verification:
  `test_nexus_v1_mns` (summary fail=0). No game data was added to the repo.

- ✅ 2026-08-06 GitHub Actions Windows-buildfix: UCRT64 no longer sees the
  POSIX-only external-archive helpers as implicit declarations. Windows keeps
  the existing fail-closed archive behavior through explicit stubs for nested
  ADF reads/path publication. Local full CMake build passes; the fix targets
  `cmake-build (windows-2022)` failure at `asset_find_by_hash.c`.

- ✅ 2026-08-06 Theron Track 02 object-record handoff: full-dungeon loading now
  retains decoded source records alongside every authentic category 4–10, 14
  and 15 occurrence, including the missile/cloud payloads. Tests verify the
  decoded category and linked reference against the raw record across all
  seven real US dungeons. No host object or synthetic inventory mapping was
  introduced.

- ✅ 2026-08-06 DM2 loose-install admission: M12 now prefers an ordinary,
  hash-verified DM2 installation over an identical ZIP/ISO member and hands
  M11 the actual directory containing `GRAPHICS.DAT` and `DUNGEON.DAT`.
  Case-insensitive macOS install names such as `graphics.dat` remain ordinary
  files rather than being mistaken for cache candidates. A mounted PC-DOS
  corpus under `.firestaff/data/dm2/dos_extract/data` now reports `READY` when
  scanning its parent DM2 directory and reaches `dm2-startup-menu` through the
  parent-root boot probe. Virtual ISO entries remain launch-blocked and no
  game data was unpacked, copied or tracked. Verification:
  `test_dm2_v1_missing_graphics_profile_gate`,
  `test_asset_status_dm2_iso_required_cache_gate`,
  `test_dm2_v1_m11_startup_profile_gate`, and the real-data boot probe.

- ✅ 2026-08-06 DM2 unowned projectile-route isolation: removed the projectile
  dispatch, per-tick step and creature-collision adapters from the production
  DM2 archive. They own a private F0810-compatible list, but no M11 or DM2
  runtime call site supplies the original CCM, timer, creature and DB-pool
  transaction that creates it. Dedicated tests compile the modules directly;
  product code cannot present their fixture-capable list as live gameplay.
  No game data was unpacked, copied or tracked.

- ✅ 2026-08-06 Nexus STABG DMWeb table hardening: the retail
  `DecodeSTABGBIN`-shaped offset table must now show its zero terminator; a
  full bounded table without that marker is rejected instead of being
  accepted as complete. The public surface loader also rejects negative byte
  counts before size conversion. Real `STABG.BIN` still decodes its 11 maps,
  40×21 first map, 320×168 receipt surface and 512-byte palette. Verification:
  `test_nexus_v1_startup_media_gate` with the mounted Nexus corpus.

- ✅ 2026-08-06 Nexus startup-surface allocation bounds: `nexus_ui_surface_load`
  now validates width/height products with `size_t` before comparing source
  bytes or allocating. Overflowing dimensions remain unavailable instead of
  becoming a negative/truncated requirement; the real WARNING/GAMEOVER/TITLE
  and STABG startup media path still passes. Verification:
  `test_nexus_v1_startup_media_gate` against
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- ✅ 2026-08-06 DM2 original-SKSave runtime gate: M11 no longer promotes the
  decoded `DM2_V1_SessionState` subset into a playable runtime. SKProject
  `DM2_GAME_LOAD` continues the original stream with record pools, timers,
  actuator-generator initialization, map selection and entrance placement;
  the incomplete subset now fails atomically and leaves the startup menu
  active. This prevents real save files from being misrepresented as fully
  imported gameplay. Verification: rebuilt `firestaff_m11`; real PC-DOS
  `test_dm2_v1_m11_startup_profile_gate` passes with
  `FIRESTAFF_DM2_DATA_DIR=/Users/bosse/.firestaff/data/dm2/dos_extract/data`.
  No game data was unpacked, copied or tracked.

- ✅ 2026-08-06 Nexus MENU.BPK PRS3 span hardening: bounded compressed-size
  inspection now stops at each directory entry's `next_offset`, including the
  final entry's authenticated `PALT` boundary, instead of scanning to the end
  of the archive. The real 162-entry PRS3 corpus remains intact and the
  regression asserts every reported payload stays inside its own entry span.
  Verification: `test_nexus_v1_bpk_archive` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-08-06 CI link fix: `test_dm2_v1_scene_weather_light_runtime_chain_real_data`
  now compiles its `dm2_v1_GRAPHICS_DATA_OPEN_receipt` implementation into the
  test target. The full real-data runtime-chain test passes after the fix.
- ✅ 2026-08-06 Nexus TITLE.BIN MAPD/TIBG bounds: corrected the DMWeb
  section minimum from `0x8c70` to `0x8c74`, covering five 64×28 tilemaps
  followed by all sixteen big-endian palette words. A truncated palette table
  is now rejected before any out-of-range read. The real
  `/Users/bosse/.firestaff/data/nexus/TITLE.BIN` + `TITLE.CG` corpus still
  decodes all five maps and passes the truncation regression in
  `test_nexus_v1_title_mapd_real`; Saturn VDP1/VDP2 placement remains capture-
  gated.

# 2026-07-11 Nexus DGN dungeon-start host route

- ✅ 2026-07-11 Theron Track02 evidence-boundary verification: reviewed the available local source-lock corpus and staged media state before continuing non-startup work. No hash-verified Track 02 corpus is present locally and no original HuC6280 loader/disassembly trace currently identifies a non-startup level/object record boundary or a palette/bitmap binding, so no decoder or fallback was added. Ninja rebuilt the focused Theron Track02 targets and CTest passed `theron_v1_track02_level_handoff`, `theron_v1_track02_nonstartup_sector_receipt`, and `theron_v1_track02_descriptor_entry_semantic` (3/3). The existing gates continue to prove only the hash/anchor-gated startup route and opaque, promotion-blocked post-descriptor container evidence.

- ✅ 2026-07-11 Theron Track02 six-container MODE1 prerequisite: added a hash/index-gated typed sector descriptor for the six real non-startup JP/US raw-BIN containers. Real-media analysis proves every 0x400 raw window crosses a MODE1 sector boundary and then the next sector's 16-byte sync/header: five profiles are user/tail/header/user (720 user bytes + 288 EDC/ECC-tail bytes), while anchor 2 entry 6 is tail/header/user (256 tail + 752 user). The probe verifies both MD5-locked regions and the matching physical profile at all three anchors. The descriptor exports no payload bytes and assigns no compression, table, object, level, bitmap, palette, text, or runtime role; it is solely the prerequisite that prevents a later decoder from consuming MODE1 framing bytes as payload. Verified by strict `cc -std=c11 -Wall -Wextra -Werror` compilation of `theron_v1_track02.c`, focused target rebuild, direct real-media probe, and focused Track02 CTest suite (4/4).

- ✅ 2026-07-11 Nexus Structure1B height-aware host render plan: copied DGN commands now project the already decoded signed 1/32-unit floor corners, their derived ceiling corners, and the corresponding front/side wall endpoints into plan quads. The material viewport and M11 host plan therefore share non-flat DGN silhouettes instead of letting the host flatten real slopes. Regression coverage fixes the exact floor and ceiling quad coordinates for a bounded Structure1B slope fixture. No post-grid row payload received semantics.

- ✅ 2026-07-11 Nexus 0x30 row-flag provenance consumption: the verified 0x30 typed-prefix high-bit flag now has per-level flagged-row count and first/last ordinal receipts from the real LEV00-LEV15 corpus. Those measurements propagate through geometry information, renderer handoff and the DGN render plan without assigning a flag meaning, decoding other row bytes, or treating Structure1B packed 12-bit `post_grid_0x30_ref` values as row ordinals. The corpus gate locks all 16 levels' observed flag distributions, while synthetic coverage proves a flag reaches the render receipt independently of a cell reference.

- ✅ 2026-07-11 Theron SRM authenticated body-evidence boundary: bounded SRM inflate now verifies the gzip CRC32 and ISIZE trailer before any envelope decoder can consume the body. Successful payloads carry only neutral evidence (container flags, authenticated size/CRC, and whole/prefix/suffix fingerprints); this supports real Save Disk corpus comparison without assigning unknown Sphenx/Greatstone bytes to party, inventory, or progression fields. Trailer mismatches remain non-decodable and cannot create evidence or enable Continue. The SRM body probe reports configured real-slot fingerprints when artifacts are staged and skips cleanly otherwise. Verified with Ninja targets `test_theron_v1_srm_body_decode_pc34`, `firestaff_theron_v1_srm_body_decode_probe`, `test_theron_v1_srm_classifier_pc34`, and `test_theron_v1_startup_save_resume_pc34`; focused CTest passed 4/4.

- ✅ 2026-07-11 Theron SRM strict single-member container boundary: the authenticated Save Disk import path now rejects gzip reserved flag bits, invalid optional FHCRC fields, trailing bytes, and concatenated gzip members. The CRC32/ISIZE trailer is located immediately after the one DEFLATE member that zlib consumed, so a later member's matching trailer cannot authenticate an earlier body. Rejected containers publish no body evidence and cannot enable Continue; this adds no Sphenx/Greatstone body-field mapping. Regression coverage exercises concatenated valid members and an invalid FHCRC. Verified with Ninja `test_theron_v1_srm_body_decode_pc34`, `test_theron_v1_srm_classifier_pc34`, `firestaff_theron_v1_srm_body_decode_probe`, and `firestaff_theron_v1_srm_classifier_probe`; direct tests passed 65/65 and 111/111, focused CTest passed 4/4.

- ✅ 2026-07-11 Theron SRM authenticated body-corpus receipt: startup now retains opaque fingerprint groups for every authenticated one-member gzip body across the five Save Disk slots. The catalog uses only verified trailer metadata and existing whole/prefix/suffix fingerprints, keeps unauthenticated bodies outside groups, and does not interpret real Sphenx/Greatstone bytes or affect the first fully decoded Continue selection. The SRM body probe prints the staged-artifact corpus receipt when real slots are available. Verified with Ninja `test_theron_v1_srm_body_decode_pc34`, `test_theron_v1_startup_save_resume_pc34`, and `firestaff_theron_v1_srm_body_decode_probe`; direct tests passed 71/71, 319/319, and probe 20 pass with one expected no-artifact skip.

- ✅ 2026-07-11 Theron SRM authenticated two-corpus delta topology receipt: comparable trailer-authenticated bodies now retain a bounded, position-bound layout of differing byte runs, including total and retained run counts, longest run, truncation state, each retained offset/length pair, and a topology-bound checksum. The receipt first revalidates the supplied payloads against existing authenticated whole/prefix/suffix evidence, assigns no field semantics, and remains outside decoder, Continue, object/runtime, palette, route, and synthetic-content paths. Regression coverage proves separate and contiguous change runs; the configured two-root real-media probe emits only this neutral topology when staged corpora are available. Verified with Ninja `test_theron_v1_srm_body_decode_pc34` and `firestaff_theron_v1_srm_body_decode_probe`, plus focused CTest.

- ✅ 2026-07-11 Theron SRM decoded-slot selection: startup Save Disk scanning now retains the first gzip-recognized slot for manifest evidence while selecting the first ascending slot that completes the existing bounded envelope decode. An unknown gzip body cannot block a later valid supported envelope, and the startup state receipt publishes only that decoded slot as Continue-active. This does not infer Sphenx/Greatstone body fields. Verified with Ninja `test_theron_v1_startup_save_resume_pc34` and focused CTest.

- ✅ 2026-07-11 DM1 D3L/D3R F0116/F0117 source-lock markers: restored the explicit M11 nearest-blocking-center-depth query for the split F0115 side-content and deferred-explosion passes, and materialized F0096 wallset variants with the ReDMCSB `M646 + wallSet * M647` formula before F0116/F0117 consume D3L/D3R graphics. Verified with the focused DM1 viewport source-lock CTest and side-contents center-blocker probe. No parity/capture or combat work was included.

- ✅ 2026-07-11 Nexus BPK/PRS3 material-route correction: updated the local-data boot hash-scan contract for the real `MENU.BPK` PRS3 decoder route. The test now requires all 162 real PRS3 surfaces to decode and upload successfully rather than preserving the retired PRS3 blocker expectation. FLOORS/WALLS BPK host import is now atomic: a mixed archive containing valid prefix surfaces plus a later truncated or undecodable surface leaves the destination material bank unchanged, so DGN cannot consume a partial archive. Regression coverage in `test_nexus_v1_bpk_surface_class` verifies the mixed valid/truncated case; `test_nexus_v1_dgn_material_raster` keeps the material-to-viewport path covered. Verified with focused Ninja/CTest targets.

- ✅ 2026-07-11 Theron Track02 strict dungeon route catalog: added exact catalog selection and an atomic catalog-backed stairs transition for complete Track02 level/object/bitmap transactions. Catalog entries must belong to one dungeon, carry unique valid levels, and form a contiguous sequence beginning at level 0; gaps, duplicates, mismatches, and rejected routes have no selected fallback and leave the queued world transition unchanged. The compact object records remain receipt-only and are not projected into the generic world object database. Verified with the focused Track02 descriptor semantic probe and CTest.

- ✅ 2026-07-11 Theron Track02 validated level transition: complete Track02 dungeon routes now retain their dungeon and sub-level identity, and queued stairs transitions atomically install only a matching complete target route. The boundary requires validated level, compact object-table and bitmap-route receipts on both sides, rejects missing or mismatched routes without mutating the world, and keeps raw object rows out of the generic world-object database until their real-media semantics are known. Verified with Ninja `firestaff_theron_v1_track02_descriptor_entry_semantic_probe` and focused CTest `theron_v1_track02_descriptor_entry_semantic` 1/1.

- ✅ 2026-07-11 Theron Track02 compact-row ordinal consensus: accepted compact-row evidence now retains each level byte's first and last table ordinal plus a stable FNV-1a hash over ordinal and raw row bytes. Cross-anchor consensus requires those positions as well as count and bytes, so a reordered otherwise identical row set is reported as a bounded mismatch. This remains receipt-only: it assigns no object-field meaning and does not alter object/runtime/Continue readiness, synthetic-menu blocking, or palette promotion. Verified with Ninja `test_theron_v1_startup_save_resume_pc34` and `firestaff_theron_v1_track02_descriptor_entry_semantic_probe`, then focused CTest.

- Added a DGN-backed new-game start receipt in `nexus_v1_game`: the verified level-0 `(11,29,N)` request records its decoded Structure1B square/collision/mesh facts and blocks runtime without fallback when the cell is invalid, out of bounds, a wall, or collision-blocked.
- `nexus_v1_load_level()` now resolves and applies that receipt before initializing level-0 mechanics, so host, mechanics, and later material-plan rendering share the same accepted DGN start pose. `test_nexus_v1_dgn_material_raster` covers ready consumption and collision-blocked rejection.
- ✅ 2026-07-11 Theron Track02 palette production path: added a bounded HuC6260 16-colour 9-bit palette decoder and indexed-atlas-to-RGBA route builder. The decoder's channel mapping is B=bits 0..2, R=3..5, G=6..8; the route accepts only an explicit 32-byte palette payload with reserved bits clear and at least one nonblack entry. Invalid, zero, or incomplete inputs leave the RGBA receipt unpublishable, so bitmap samples cannot promote a guessed fallback palette. Extended the existing Track02 semantic probe with component, colourization, and malformed-payload coverage.
- ✅ 2026-07-11 DM1 transactional automatic backup resume: `DM1_LoadGameWithBackup()` now loads both Firestaff-native and original PC34 candidates away from the live runtime world, tries `.bak` only after a primary open failure, and promotes the backup only after complete validation. A rejected present primary preserves the active world, caller header, backup file, and `usedBackup=0`. Verification: Ninja `test_dm1_v1_save_load` passed 15/15; direct original handoff test passed; CTest `dm1_v1_save_load_source_lock` and `dm1_v1_original_save_pc34_handoff` passed.
- ✅ 2026-07-11 DM2 live GDAT sprite/scene/light/weather material consumption: expanded the boot-owned `GRAPHICSSET` receipt with the real `SCENE_RAIN`, `MISTY_MAP`, `THUNDER_POSITION`, and `AMBIANT_DARKNESS` typed words. Dungeon map-chip sprites and HUD images now use the same validated `dtPalIRGB`/`dtPalette16` material blit as floor, wall, and door-frame imagery; scene-light consumption is recorded per material pixel. Weather commands only execute when their matching real GDAT scene material is present, so rain, mist, and thunder no longer receive an unconditional procedural route. Runtime ownership receipts expose all scene words and sprite/light/weather consumption. The DM2 live-sidecar layout advances to v3 so restored frames retain the added scene words. Verified with Ninja `test_dm2_v1_boot_profile_smoke`, `test_dm2_v1_runtime_handoff_smoke`, and `test_dm2_v1_save_load`; focused CTest passed 3/3.
- ✅ 2026-07-11 DM2 M11 GDAT title/menu palette presentation: M11 now installs the verified `INTERFACE_GENERAL/0`, field `0xFE` `dtPalIRGB` table as the active raw 8-bit presentation palette for DM2. Startup GDAT title/menu blits map logical image indices through the paired `dtPalette16` table before writing the indexed framebuffer, while a real menu GDAT frame suppresses all rect/text fallback draws. This follows skproject `SkWinCore::INIT` lines 55606-55615 and `SHOW_MENU_SCREEN`, with `MapGraphicsStyle()` kept solely on the active `GRAPHICSSET` scene route. Regression coverage verifies the exported IRGB table and a title-source pixel after `dtPalette16` mapping. Ninja built `test_dm2_v1_m11_startup_profile_gate`; focused CTest `dm2_v1_boot_profile_smoke|dm2_v1_runtime_handoff_smoke` passed 2/2. The broad M11 startup gate retains one pre-existing composite real-visual-capture receipt failure.
- ✅ 2026-07-11 DM2 outdoor GDAT runtime: replaced the V1 procedural sky-gradient and ground-fill branch with the active map `GRAPHICSSET` ceiling/floor GDAT materials, routed through the existing `dtPalIRGB`/`dtPalette16` material blitter. The runtime host receipt now distinguishes outdoor sky and ground consumption and accepts an outdoor frame without indoor walls only when both source materials, HUD, and zero fallback draws are present. `test_dm2_v1_runtime_handoff_smoke` covers the material fetches, pixels, and host receipt. Verification: Ninja-built targeted DM2 tests passed directly, 154/154 and 81/81; this build tree has no CTest registration.

- ✅ 2026-07-11 DM2 live map-scene material route: floor, ceiling, outdoor sky and outdoor ground now encode and fetch the current `Map_definitions::MapGraphicsStyle()` `GRAPHICSSET` record instead of silently reading index 0. The boot asset provider caches decoded ceiling/floor surfaces per map graphics-set, and frame ownership records the selected scene material index plus both consumed planes. The legacy `-1`/`-2` material addresses remain diagnostic-only. Source-locked to skproject `DME.h MapGraphicsStyle()` and `SkWinCore.cpp` `glbMapGraphicsSet` GDAT material queries. Verified with Ninja `test_dm2_v1_runtime_handoff_smoke` 155/155 and `test_dm2_v1_boot_profile_smoke` 81/81; focused CTest passed 2/2.
# ✅ 2026-07-13 Nexus Structure1Fa ITEM.IBS special-floor palette consumer

`nexus_v1_item_ibs_parse_verified()` now validates the documented special
floor-image descriptor table, local/inherited 16-colour BGR555 palettes and
bounded raw payload spans. `nexus_v1_dgn_bind_structure1f_item_materials()`
binds that original palette/payload receipt to the matching floor command.
The unproved `0008` pixel codec remains no-draw and cannot fall back to an
inventory icon. Focused `nexus_v1_dgn_geometry_readiness` passes.

# ✅ 2026-07-13 Nexus ITEM.IBS special-floor packed-4bpp corpus gate

The canonical `ITEM.IBS` corpus now proves that descriptor encoding `0008`
has a packed `width * height / 2` 4bpp span, with local 16-colour BGR555
palettes interleaved before later payloads. The parser maps the on-disc
0..108 floor ordinals into the combined 223..331 image space, preserves the
positive packed payload, and treats legal `FFFF` inventory associations as
no-draw. The raw nibble order and world placement remain intentionally
blocked. Verified against `~/.firestaff/data/nexus/ITEM.IBS` through
`nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-13 Nexus ITEM.IBS 0008 DGN command-material consumer

Verified ITEM.IBS `0008` floor payloads now reach an explicit, command-indexed
DGN material consumer with their exact packed bytes, local BGR555 palette,
dimensions, and source provenance. The consumer rejects a non-floor or
out-of-range command and never authorizes drawing: original nibble order and
3D placement remain unproven, with no inventory-icon or synthetic fallback.
The focused `nexus_v1_dgn_geometry_readiness` target passes against the local
retail ITEM.IBS corpus.

# ✅ 2026-07-13 Nexus Structure1F ITEM.IBS retail-coverage gate

`nexus_v1_dgn_structure1f_item_ibs_coverage()` now validates every direct
Structure1F item against the authenticated ITEM.IBS bank before it can reach a
material path. The local LEV00–LEV15 corpus proves 446 item records and 174
separate descriptor-`0008` references, with no missing or unsupported source
descriptor. The receipt remains no-draw and fail-closed: it does not claim a
Saturn texel order or world placement. Verified by
`nexus_v1_dgn_geometry_readiness` with the retail corpus.

# ✅ 2026-07-13 Nexus ITEM.IBS 0008 VDP1 codec-provenance gate

The new `nexus_v1_item_ibs_decode_0008_vdp1_4bpp()` keeps the Saturn VDP1
high-nibble-first rule behind four independent provenance facts: verified
ITEM.IBS identity, original VDP1 command stream, 16-colour mode, and the
byte/nibble route. Retail ITEM.IBS descriptors therefore remain blocked and
no-draw when only their own data is available. Focused
`nexus_v1_dgn_geometry_readiness` verifies both the blocked retail route and
the source-gated decoder contract without a fallback.

# ✅ 2026-07-13 Nexus DM.BIN PRS3 marker catalog

`nexus_v1_prs3_dm_bin_catalog_verified()` now reads only bounded, literal
PRS3 framing from hash-verified original DM.BIN bytes. The retail corpus has
two markers: an unclassified executable occurrence and one complete V1 record
with target `4096` and first frame word `997`. Truncated records fail closed;
the catalog never promotes a PRS3 opcode decoder or a render route. Verified
by `nexus_v1_prs3_capture_trace_schema` against local retail DM.BIN.

# ✅ 2026-07-13 Nexus PRS3 DM.BIN/MENU.BPK outer-frame receipt

`nexus_v1_prs3_cross_asset_frame_receipt_verified()` now compares only the
hash-verified V1 outer-frame fields shared by original `DM.BIN` and
`MENU.BPK`. The local retail corpus proves one complete DM.BIN V1 record and
162 complete MENU.BPK V1 frames, each with a nonzero first frame word. It does
not infer any opcode grammar, control-bit order, termination rule, decoded
pixel output, or menu render route: decoder promotion, menu handoff, and
fallback visuals remain disabled. Verified by the focused
`nexus_v1_prs3_capture_trace_schema` CTest with the local retail corpus.

# ✅ 2026-07-13 Nexus PRS3 V1 SH-2 execution receipt

`nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified()` imports the exact
instruction-level facts already isolated from the hash-verified retail
`DM.BIN`: the V1 control test at `85450`, R12 post-increment byte read at
`85460`, R13/R0 byte store at `85464`, and loop branch at `85472`. Any changed
anchor rejects the receipt. These are loader control/dataflow facts only; no
live `MENU.BPK` frame binding, VDP1 command observation, opcode grammar, or
decoder/menu route is promoted. Verified by
`nexus_v1_prs3_capture_trace_schema` against local retail `DM.BIN`.

# ✅ 2026-07-14 Nexus PRS3 SH-2-to-VDP1 capture gate

`nexus_v1_prs3_vdp1_capture_schema_parse()` and its asset-binding companion
now define a strict future-capture contract for one exact `MENU.BPK` PRS3
frame: hash-bound BPK/DM.BIN bytes, bounded packed input span, SH-2 input and
output address ranges, complete output fingerprint, and a later VDP1 command
whose texture source is that exact output range. Partial or inconsistent
traces reject atomically. The gate records an observed handoff only; it never
claims an opcode grammar, enables generic PRS3 decoding, or permits fallback
visuals. Verified by `nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 VDP1 command/palette capture contract

The original-capture schema now accepts V3 evidence for one hash-bound
`MENU.BPK` PRS3 frame. In addition to the existing SH-2 input/output and VDP1
texture-read witnesses, V3 requires contiguous raw VDP1-command and palette
read spans with ordered sequence numbers, byte counts, addresses, and FNV
witnesses. Binding remains tied to the exact MENU.BPK/DM.BIN input, and a
changed palette span rejects the capture. This establishes no PRS3 opcode,
texture-pixel, palette-format, or VDP1 field semantics, and it never permits
rendering or fallback visuals. Verification:
`test_nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 external-capture validator

`firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN` now
imports a read-only V3 candidate trace only after the two supplied ordinary
files match the canonical Track 1 MD5 identities. The validator binds the
trace's MENU.BPK span to real bytes and reports the VDP1-command/palette
witnesses, while leaving runtime import, decoder promotion, and fallback
visuals disabled. It does not manufacture a trace or attest the capture
producer. Verification: `nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 raw-sidecar admission

The V3 validator now optionally accepts three read-only capture sidecars:
decoder output, raw VDP1 command bytes, and raw palette bytes. Admission
requires each sidecar's exact recorded size and FNV witness to match the
hash-bound V3 trace after canonical `MENU.BPK`/`DM.BIN` validation. The CLI
accepts all six inputs and reports each binding separately. It does not claim
that a file was produced by an original Saturn/emulator, decode any sidecar,
or permit runtime import, rendering, or fallback. Verification:
`nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 provenance-ledger gate

The raw-sidecar admission can now be accompanied by a strict text ledger that
hash-binds the V3 trace, output, VDP1-command, palette sidecars, and capture
producer binary. Missing or changed files reject the ledger. This records
reproducible provenance only: no local authentic Nexus trace/log was found,
producer authentication remains false, and runtime import stays disabled.
Verification: `nexus_v1_prs3_capture_trace_schema`.
# ✅ 2026-07-14 Nexus active DGN Structure3 mesh source route

The Nexus engine now exposes a caller-buffered Structure3 mesh-entry route
only from the exact canonical `LEVxx.DGN` bytes it currently owns. It requires
the active level identity, canonical hash, byte-binding receipt, and current
source bytes to agree before returning typed signed 16.16 vertices, face rows,
and paired normals. Mutated or stale level bytes return no partial mesh. This
is a source-routing boundary only: no transform, texture, palette, VDP1, or
draw semantics are granted, and the no-draw barrier remains active.
Verification: `test_nexus_v1_dgn_geometry_readiness` covers the active route
and mutation rejection.

# ✅ 2026-07-14 Nexus active LEV Structure3 directory receipt

The engine now exposes the active canonical LEV's bounded Structure3 directory
with the exact retained source-byte FNV for capture tooling. This is a
source-owned no-draw catalog only; it does not decode texture, palette, VDP1,
transform, or drawing semantics. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus complete DGN material source gate

The complete Structure3 source scene now consumes the active canonical
Structure2 payload-anchor traversal. Every descriptor must retain an image
anchor, each nonzero palette anchor is kept separately, and the full anchor
count must be consumed from the same LEV bytes before the scene is complete.
These remain bounded capture candidates only: image/palette lengths, texel
order, palette format/addressing, VDP1 mode, decoder, and draw semantics all
remain fail-closed. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus animated DGN payload-anchor route

Every declared non-control Structure1G image instruction for an active `08xx`
face now resolves through the active Structure2 payload-anchor traversal. The
route requires the matching image anchor and, where present, the matching
palette anchor from the exact same canonical LEV bytes. It remains no-draw:
candidate interval bounds are not pixel spans, palette format/addressing,
texel order, VDP1 mode, timing, decoder, or drawing proof. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus static DGN face payload intervals

Every active static Structure3 material face now carries the bounded
next-anchor candidate interval for its exact Structure2 image payload and,
when present, its palette payload. The viewport refuses a static source packet
without those intervals. This is capture framing only: neither interval is an
image or palette length, and no pixel codec, palette format, VDP1 mode,
transform, or draw path is inferred. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus descriptor capture windows

Every Structure2 descriptor capture target now contains the exact bounded
image candidate window and, where present, palette candidate window from the
canonical LEV. The raw-trace admission manifest must bind these hashes and
offsets before provenance is considered. This enables a real capture producer
to state its source-read targets without claiming that it observed, decoded,
or drew them. Pixel/palette/VDP1 semantics and rendering remain fail-closed.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 descriptor capture target

The active canonical LEV route can now build and atomically write an external
capture request for one exact Structure2 descriptor. It carries only source
identity, descriptor byte offset/raw fields, and FNV fingerprints for that
20-byte descriptor and the bounded post-FFFF span. The target requires an
original Saturn capture and remains no-draw; it is not a pixel decoder,
palette format, animation rule, VDP1 command, or runtime fallback.
Verification: `test_nexus_v1_dgn_geometry_readiness` against the hash-verified
retail LEV corpus, including an emitted LEV00 request.

# ✅ 2026-07-15 Nexus active DGN face/material selector receipt

`nexus_v1_current_level_structure3_face_material_receipt()` now carries the
active canonical LEV's bounded Structure3 face topology together with its
complete documented Structure2/Structure1G selector joins. Hash, byte size,
and FNV identity are required; a stale source receipt withdraws the route.
Selectors remain identifiers only: material bytes, pixels, palettes, UVs,
VDP1 commands, and drawing remain unavailable, with no fallback visuals.
Verification: `test_nexus_v1_dgn_geometry_readiness` against the hash-verified
retail LEV00.DGN through LEV15.DGN corpus.

# ✅ 2026-07-15 Nexus active Structure1A owner-chain receipt

`nexus_v1_current_level_structure1a_owner_chain_receipt()` now consumes the
complete Structure1F index to unique Structure1B owner to Structure1A row to
raw Structure3 model/face-selector chain from the active hash-bound LEV. The
receipt withdraws on stale source identity. It assigns no placement,
transform, material, pixel, palette, VDP1, or draw semantics and permits no
fallback visuals. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus active Structure2 descriptor-envelope receipt

`nexus_v1_current_level_structure2_descriptor_receipt()` now consumes the
active canonical LEV's bounded Structure2 descriptor table and post-FFFF
opaque span, together with complete optional Structure1G global-to-local
descriptor bindings. Source hash/size/FNV identity and the measured aligned
descriptor-offset envelope are required; stale identity withdraws the route.
No payload encoding, pixels, palette, animation, VDP1, or drawing semantics
are assigned, and fallback visuals remain disabled. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus active LEV Structure3 mesh semantic receipt

The engine now publishes the active canonical LEV's bounded Structure3
topology, signed-vector, and face/normal evidence only when the retained bytes
still match the package-bound source receipt. The receipt withdraws on a stale
level or any byte mutation. It remains explicitly no-draw: no original capture,
texture, palette, transform, VDP1, or renderer handoff semantics are claimed.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus active LEV Structure3 face framing receipt

The engine now binds Structure3 entry-header boundaries and face-row local
vertex-index evidence to the exact active canonical LEV bytes. Any stale or
mutated source withdraws the receipt. This remains a no-draw framing boundary:
it proves neither Saturn transforms nor surfaces, materials, textures,
palettes, VDP1 commands, or rendering. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus active LEV transform/camera framing receipt

The engine now binds the active party cell and direction to the exact
canonical LEV byte receipt, alongside the existing bounded Structure1A raw
transform-selector receipt. A stale level, invalid pose coordinate, or byte
mutation withdraws it. This is no-draw camera-input provenance only: no
Saturn camera matrix, transform order/unit, culling, or rendering semantics
are inferred. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus PRS3 V3 provenance-bundle validator route

The read-only V3 capture validator now accepts an authentic capture bundle's
trace, three raw sidecars, provenance ledger, and producer binary in one
invocation. It can verify their FNV links, but reports producer authentication
and runtime import as false by design. No PRS3 opcode grammar, pixel/palette
decode, synthetic surface, or draw route is enabled. Verification:
`test_nexus_v1_prs3_capture_trace_schema` and the validator target build.

# ✅ 2026-07-14 Nexus PRS3 V3 producer-attestation workflow

The V3 validator can now additionally check a strict Mednafen SH-2/VDP1 bus
trace workflow attestation against the complete artifact bundle and producer
binary. Its original-Saturn execution line is deliberately only a claim, so
the result always requires independent authentication and cannot permit
runtime import, decoding, fallback pixels, or rendering. Verification:
`test_nexus_v1_prs3_capture_trace_schema` and the validator target build.

# ✅ 2026-07-14 Nexus PRS3 V3 capture-bundle ledger writer

The V3 capture tool now writes the deterministic provenance ledger from an
externally acquired trace, canonical MENU.BPK/DM.BIN, raw output/VDP1/palette
sidecars, and the producer binary only after the existing byte-bound admission
passes. It writes hashes, never copies capture or game bytes, and cannot
authenticate a producer, decode PRS3, import runtime data, or render.
Verification: `test_nexus_v1_prs3_capture_trace_schema` and validator build.

# ✅ 2026-07-14 Nexus active LEV renderer-source receipt

The DGN viewport now consumes an active-LEV renderer receipt that carries the
canonical package-bound LEV byte count/FNV and Structure3 payload hash to the
renderer boundary. It withdraws the receipt when retained bytes change. When
an original-capture packet is admitted, the receipt names each opaque source
span while retaining independent blockers for texture decoding, palette
application, VDP1 command semantics, and transform/culling semantics. The
route stays no-draw with fallback visuals disabled; it creates no pixels or
host interpretation of Saturn state. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

- ✅ 2026-07-14 CSB PC V1 startup decode: literal assets bypass LZW and
  compressed assets use ReDMCSB-compatible chunk-width LZW before IMAGE3
  expansion. This restores original C001 title and entrance assets. Verification:
  real-data title/import and launch probes.

The coalesced original Mednafen receipt now requires the later `$e009`
dispatch's observed local-RAM destination plus a 32-byte post-return RAM
fingerprint. Firestaff compares that fingerprint with the selected MODE1
user-data prefix, so a supplied authentic capture can establish a bounded
record-to-RAM transfer for `0x0b52`. The contract rejects a missing,
misordered, mismatched, or non-local span. It does not establish a game
transition or assign dungeon, object-tail, bitmap, palette, or payload
semantics. Verification: focused raw-loader CTest and capture-order script.

# 2026-07-14 Nexus Structure3 face-pair multiplicity corpus receipt

The DGN face receipt now partitions each entry-local unordered vertex pair by
whether it co-occurs in one or multiple bounded face rows and retains the
maximum local occurrence count. The hash-verified LEV00.DGN through LEV15.DGN
retail corpus validates the partition. This is no-draw row incidence only; it
does not establish an edge, winding, surface, normal-plane, transform,
texture, palette, or drawing behavior.

# 2026-07-14 Nexus Structure3 retail source-only capture gate

`test_nexus_v1_dgn_face_mesh_corpus` now submits source-only capture input for
each hash-verified LEV00.DGN through LEV15.DGN level and requires all 16 to
remain blocked before candidate framing, complete source binding, or renderer
handoff. This proves only that the installed DGN corpus lacks the separate
captured texture span, palette state, VDP1 state/command, transform, culling,
and ordered original-Saturn provenance required by the binder. It does not
decode a texture or palette, assign a transform, or authorize DGN drawing.
Verification: `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus
./build-nexus/test_nexus_v1_dgn_face_mesh_corpus`.

# 2026-07-14 Nexus ITEM.IBS 0008 VDP1 capture-binding gate

The documented packed-4bpp parser now requires an atomic capture receipt before
it expands any descriptor-`0008` texels: hash-verified complete `ITEM.IBS`
bytes, selected descriptor metadata, exact packed span and BGR555 palette,
VDP1 state/command fingerprints, texture-source extent, and strict
texture-before-command sequence all have to match. The codec remains no-draw
and retail ITEM.IBS remains blocked because no original Saturn packet is
present. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# 2026-07-14 Nexus ITEM.IBS VDP1 command-packet shape gate

The descriptor-`0008` capture binder now parses a complete 32-byte
little-endian VDP1 command record before it can authorize high-nibble-first
expansion. It requires the captured texture-source word, 4bpp colour-bank
mode, and declared width/height to agree with the selected ITEM.IBS descriptor,
in addition to the pre-existing hash and sequence checks. The focused
`nexus_v1_dgn_geometry_readiness` fixture proves a self-consistent but
different source word remains blocked. This is only a documented hardware
packet-shape check: no original Saturn command packet was added, so retail
ITEM.IBS stays no-draw and no texture, palette, placement, or VDP1 ordering
claim is promoted.

# Nexus Structure3 Selector Reuse Receipt (2026-07-14)

`nexus_v1_level_structure3_face_material_receipt()` now retains per-level
unique and reused bounded face-selector occurrences for both documented
Structure2 (`00xx`) and Structure1G (`08xx`) joins. The focused retail
LEV00.DGN through LEV15.DGN corpus test requires complete reuse accounting.
The aggregate corpus retains 1,291 unique and 16,110 reused Structure2
selector occurrences plus 44 unique and 376 reused Structure1G occurrences.
This is identifier provenance only: payload decoding, dimensions, UVs,
palettes, animation, transforms, and VDP1 drawing remain blocked pending
original Saturn evidence.

# 2026-07-14 Nexus Structure3 typed mesh corpus identity receipt

`test_nexus_v1_dgn_face_mesh_corpus` now serializes only the bounded typed
Structure3 vertex, face, and normal rows in the hash-verified retail
LEV00.DGN through LEV15.DGN corpus. The source receipt is `d3f42b1f`, alongside
the existing 1,144 entries, 18,478 face/normal pairs, and selector-join
coverage. It deliberately does not read or associate the separate `FACE.BIN`
asset, decode texture pixels, assign palette/VDP semantics, choose a transform,
or authorize drawing. Verification: focused
`test_nexus_v1_dgn_face_mesh_corpus` against
`/Users/bosse/.firestaff/data/nexus`.
# 2026-07-27 - Nexus blocked PRS3 launcher return

- M11 now returns to the launcher when the authenticated Nexus MENU.BPK path
  is blocked on missing PRS3/Saturn decoder evidence, rather than entering a
  permanent black no-draw dungeon state. Updated runtime handoff coverage
  verifies keyboard and pointer champion starts.
# Nexus FACE.BIN diagnostics now honor `FIRESTAFF_NEXUS_DATA_DIR`; the real
# corpus path is no longer silently replaced by `$HOME/.firestaff/data/nexus`.
# Nexus ITEM.IBS diagnostics now honor `FIRESTAFF_NEXUS_DATA_DIR`; the real
# floor images. Nexus HUD documentation now identifies the procedural V2
# Nexus portrait placement boundary

2026-08-06: Removed the startup champion renderer's guessed 10×10 FACE.BIN
portrait rectangles and borders. The verified portrait ordinal remains in the
opaque render command, but its destination is zero-sized and cannot draw while
Saturn VDP1 destination/scale evidence is absent.

# Nexus roster provenance boundary

2026-08-06: Audited the Nexus champion paths against the real European
`RLOWFIX.BIN`. Production engine and launcher use the verified PLRD importer
and clear the pool on absent or malformed data; the 24-entry hardcoded roster
remains isolated to compatibility fixtures. PLRD health/stamina/mana,
attributes, equipment ordinals, and six TABL indices/codes are source-backed.
Rendered names remain intentionally unavailable until the Saturn
TEXT/FONT256 consumer is captured, so no synthetic names are promoted.
- ✅ 2026-08-06 DM1 FM Towns/Amiga real IMAGE1/IMAGE2 support: replaced the
  legacy decoder's incorrect byte-command interpretation with the DMWeb and
  ReDMCSB nibble RLE algorithm, including literal, previous-row, long-run and
  transparent-run commands. Added the DM1 legacy raster index boundary
  (0-20, 22-532) so shared 575-entry tables cannot send COD/SND/TXT/FNT or
  unused records through the bitmap cache. The new
  `test_dm1_v1_legacy_graphics_real_corpus` reads a real FM Towns MODE1/2048
  track through its ISO DATA/JDATA entries and a real Amiga ADF-extracted
  `GRAPHICS.DAT`; both decode all 532 original image records with stable
pixel digests and reject every non-raster index. No generated pixels or
platform substitution was introduced. Atari ST IMG1/IMG2 pixel binding and
STX extraction remain explicitly open in TODO.
- ✅ 2026-08-06 CSB FM Towns nested-CD intake: the launcher now streams the
  retail ZIP's raw MODE1/2352 image to a temporary file, reads ISO sectors on
  demand and verifies the original English and Japanese GRAPHICS/DUNGEON hash
  pairs before launch. It materializes the selected runtime pair together with
  title, executable and portrait sidecars in the normal CSB cache, without a
  507 MB heap allocation. The local real-media regression covers both language
  receipts and the resulting ordinary runtime paths; the game image remains
  user-supplied and untracked.
- ✅ 2026-08-06 CSB FM Towns CDDA filström: originalets 30-spårs CUE och
  råa MODE1/2352-bild kan nu leverera ett valt 44,1 kHz stereo-CDDA-spår
  sektorvis till en vanlig PCM-fil. Den sista spårlängden bestäms av den
  fysiska bildens slut, precis som minnesvägen, utan att läsa in hela
  507 MB-bilden. Realt CUE/IMG-test bekräftar spårantalet och spår 2:s
  CUE-härledda längd. Uppspelningens M11-bindning är fortfarande öppen.
- ✅ 2026-08-06 DM2 Amiga boot and M12 media handoff: the authentic Amiga AGA
  installer can now reach the normal DM2 boot owner through outer ZIP → disk
  ZIP → OFS ADF → six `dm2_arcsplit` parts → LZX entirely in RAM. Boot admits
  GRAPHICS.DAT and DUNGEON.DAT only when their known Amiga pair hashes pass,
  and admits the 176-byte CD.DAT MOD map only when its own original hash
  passes. M12 invokes that same boot-owned verifier, retains nested virtual
  provenance, passes the unchanged ZIP pathname to runtime, and never creates
  a DM2 cache. Selecting the original Amiga archive directly selects that
  platform even beside a PC install. The real-media boot and M12 regressions
  both pass against the supplied archive; no game data was unpacked, copied or
  tracked.
# 2026-08-06 Nexus strict container manifest

- ✅ `verify_nexus_v1_asset_manifest.py` now excludes the local
  `FILE_LISTING.txt` provenance artifact, streams direct ISO members through
  7-Zip, and accepts only exact canonical or documented retail SHA-256
  identities. The supplied European English ISO verifies all 137 disc assets
  with 131 loose files plus six authenticated ISO members; no game data is
  extracted or committed. Nested ISO files inside 7z remain explicitly
  uninspected.

# 2026-08-06 Nexus mixed extracted/ISO runtime source

- ✅ `nexus_v1_init()` now retains the hash-verified extracted corpus as the
  authoritative source while admitting a co-located valid retail ISO as a
  supplemental reader. Missing exact-name members are read from that ISO
  without changing source identity or enabling synthetic fallbacks. The real
  English root boot smoke verifies `DMN_ABS.TXT` (210 bytes) through this path;
  the nested ISO inside the 7z archive remains intentionally uninspected.

# 2026-08-06 Nexus startup/menu/viewport documentation correction

- ✅ Replaced stale Nexus overview, language, startup, champion, feature,
  title, menu and graphics documents with evidence-bound status. They now
  distinguish real retail byte/format receipts from unproven Saturn VDP1/VDP2,
  text, HUD, mesh, gameplay and audio consumers. The corrupt startup document
  was replaced with valid UTF-8; no runtime claim was expanded.

# 2026-08-06 Nexus HUD DM.BIN disassembly anchor

- ✅ The real-data HUD regression now verifies the `yam\\menuctrl.c` owner
  string, the 80-entry table at `DM.BIN+0x376D0`, its exact FNV-1a64 receipt,
  and seven occurrences of the SH-2 runtime address `0x060476D0`. This is a
  stronger disassembly/source-ownership receipt; it does not infer VDP1/VDP2
  drawing or event-command semantics.

# 2026-08-06 Nexus startup/menu DM.BIN resource anchor

- ✅ The real-data startup/menu regression now verifies the adjacent retail
  loader strings `MENU.BPK`, `yam\\menu.c`, `FONT256.S2D` and `STABG.BIN` at
  `DM.BIN+0x373B4` through `DM.BIN+0x373D8`. Their exact SH-2 pointer-reference
  counts are 1/10/1/1. This records resource ownership only; it does not infer
  menu order, text semantics or Saturn VDP1/VDP2 composition.
  The regression also pins the `0x18B60` SH-2 routine/literal-pool receipt
  (`FNV-1a64 0xF6D5CC046BAB98C7`) and its `yam\\menu.c`/`STABG.BIN` targets.
- ✅ 2026-08-06 Theron's Quest HuC6280 decompressor caller receipt: the
  hash-locked US/JP bank-$1f images now verify the byte-identical `$2386-$23A3`
  caller tail (30 bytes, FNV-1a `699e8da1`) in addition to the 382-byte
  `$23AD-$252A` decoder. The receipt records the source-owned output-length
  measurement through `$3B7C/$3B7D` without promoting unknown input, bank or
  level/object semantics into production. `test_theron_v1_huc6280_disassembly`
  passes against both authentic regional ISOs.

# 2026-08-06 Nexus production roster quarantine

- ✅ Removed the inferred 24-name Nexus roster from the production
  `firestaff_nexus` archive. Legacy tests/probes that intentionally exercise
  the compatibility API now link `tests/nexus_v1_champions_fixture.c` through
  `firestaff_nexus_test_fixtures`; the production library contains no old
  roster strings. The real European RLOWFIX/PLRD parser remains the sole
  production champion source, and `test_nexus_v1_champion_plrd` passes against
  `/Users/bosse/.firestaff/data/nexus`.

# 2026-08-06 Nexus PRS3/VDP1 static-state audit

- ✅ The real European `MENU.BPK` PRS3 route passes all 162 retail surfaces;
  the combined launch-smoke and DGN corpus probes also remain green. Audited
  the hash-bound `DM.BIN` VDP1 register/state receipts and confirmed they stay
  no-draw evidence: no PRS3 execution, CLUT upload, command emission,
  destination placement or menu/viewport ownership is promoted without an
  instrumented Saturn/Mednafen capture. Added the boundary to TODO so future
  work cannot mistake the decoder receipt for VDP1 presentation proof.

# 2026-08-06 Nexus startup menu text-consumer gate

- ✅ Added an explicit `menu_text_consumer_bound` production gate. The real
  TEXT4/TABL/FONT012 bytes are retained, but host-generated chrome strings no
  longer suffice to open the save/champion menu route. Until Saturn text
  placement is capture-bound, the route reports
  `menu-text-consumer-capture-required` and remains fail-closed.
  The compatibility test opts into this seam explicitly; initialized retail
  engines leave `startup_menu_text_consumer_capture_verified` clear.
  The real DM.BIN startup receipt also records one occurrence each of the
  SH-2-visible constants `0x25F00006` and `0x25F80000`; these remain address
  receipts, not proof of text-layer placement.
# 2026-08-06 Nexus RLOWFIX startup text source handoff

- ✅ Engine initialization now retains the authenticated European RLOWFIX
  `TEXT` resource 4 (15 strings), 216-entry `TABL` receipt and FONT012
  #0/#1/#2 (291/250/710 glyphs) beside the real PLRD champion records. The
  launch smoke probe verifies this source handoff;
  it does not promote the bytes into Saturn text pixels or open the menu gate.

- ✅ 2026-08-06 Theron's Quest Track 02 resource framing: the level-block
  receipt now applies the authenticated `$23AD` contract to all seven US and
  seven JP spans, retaining each exact six-byte header and bounded
  `LE16(+2)-5` bitstream slice. It rejects short, underflowing, or overrun
  frames and also passes the Track 19 ISO projections. This is a real
  disassembly-backed framing boundary only; bank mappings, decoder output and
  tile/map/palette semantics remain fail-closed.
- ✅ 2026-08-06 Theron stage-2 resource-handler disassembly receipt: the
  authentic US/JP HuC6280 handler at `$4C3F` (162 bytes, FNV-1a `46360d97`)
  now verifies the four-entry MPR table publication and the source-window to
  destination-register contract (`$3004/$3005`, length `$3006/$3007`). The
  focused disassembly test passes for both retail ISO variants. This remains
  generic source ownership; no level/object/tile/palette semantics were
  enabled without an executing command and source-LBA join.
- ✅ 2026-08-06 DM1 F0115 raw-Thing object icon ownership: floor and alcove
  item rendering now uses `dm1_v1_dungeon_get_object_subtype_pc34()` for the
  live Thing before selecting the real PC34 GRAPHICS.DAT aspect. Decoded
  candidate metadata cannot override a changed junk/torch/food record, and
  mismatched Thing types fail closed. Verified with real PC34 object-name,
  F0115 floor-material/pickup, and alcove-material tests.

- ✅ 2026-08-06 DM2 PC-DOS champion SUPPRESS correction: original save
  import now decodes the exact 263-byte `c_hero` record using
  `SKWINDOS/src/dm2data.cpp::table1d6356`, retaining the raw source records
  apart from Firestaff's older 261-byte convenience view. Proven name,
  formation, stats and hero-type fields are copied only after that decode;
  the old all-ones 261-byte mask is explicitly diagnostic-only. Synthetic
  D2RS envelopes are rejected before save admission. The real eight-file
  PC-DOS corpus passes the source receipt census; complete `GAME_LOAD`,
  inventory/possession and live resume remain fail-closed.
- ✅ 2026-08-06 DM2 original inventory/leader-hand fail-closed correction:
  replaced the false flat 32-bit leader-hand and 30-slot inventory save
  helpers with rejection boundaries. SKProject `LeaderPossession` is a
  22-byte runtime cursor, but only its 16-bit ObjectID reaches SKSAVE through
  `WRITE_RECORD_CHECKCODE`; `c_hero::item[30]` likewise contains 16-bit DB
  links. The source-session route no longer publishes the old cache, M11
  cannot re-inject it, and inventory swap returns unavailable until the real
  record-chain importer/allocator exists. Verified with utility, real
  eight-save corpus, M11 startup/profile and Phase A probes.

- ✅ 2026-08-06 DM2 save-resume documentation audit: corrected stale TODO
  claims which described diagnostic D2RS/raw-SKSave parsing and fabricated
  inventory caches as a playable restore path. The documented state now
  matches the enforced gate: original corpus parsing is receipt-only; no
  source session, possession graph, inventory, or leader hand is published
  before the SKProject record allocator/append path is implemented.

- ✅ 2026-08-06 DM2 `READ_RECORD_CHECKCODE` ownership receipt: the isolated
  decoder now mirrors SKProject `sksvgame.cpp:808-974` and
  `skrecord.cpp:63-112` at the allocation boundary. Its explicit callback
  contract appends every allocated link to the authentic parent root or tile
  coordinates, initializes nested `uw_02` roots before recursive reads, and
  preserves the source two-bit record-link placement field. Unit coverage
  proves ordered chain ownership and placement retention; the real eight-save
  PC-DOS corpus traverses every hero-item and party root (72 PASS). This is a
  test-only source receipt, not a playable restore path: production remains
  fail-closed until a genuine G1 DB/tile/possession/timer allocator exists.

- ✅ 2026-08-06 CI CSB V2 touch/controller link correction: added
  `vga_palette_pc34_compat.c` to the standalone CSB test target that compiles
  the V2 viewport renderer. The focused CMake build and CTest pass locally;
  the main GitHub matrix remains the cross-platform verification.

- ✅ 2026-08-06 DM2 creature-door data correction: removed the active
  hard-coded zero door-attribute fallback from the G1 field bridge. The
  current-map DB0 door root selects map-header slot 0/1, then the real
  `DOORS/dtWordValue/0x0d` record supplies the closed-door creature rule,
  matching SKProject `GET_GRAPHICS_FOR_DOOR` and `GET_DOOR_STAT_0D`
  (`skdoor.cpp`). Missing G1, map-header, or GDAT ownership now returns no
  field result instead of inventing a blocking attribute.

- ✅ 2026-08-06 DM2 flat inventory ABI closure: the residual public
  leader-hand and champion-inventory setters no longer mutate the retired
  32-bit cache, and their getters cannot expose a fixture-written handle.
  This matches SKProject `LeaderPossession`/`WRITE_RECORD_CHECKCODE` and
  `c_hero::item[30]`: the original route owns 16-bit DB links and the cursor,
  neither of which can be reconstructed from a host handle. The focused
  save/load regression proves these calls stay fail-closed; M11's real
  PC-DOS startup gate still passes.
✅ 2026-08-06 Nexus SFX-diagnostiken visar inte längre syntetiska händelsenamn.
Hostenumret är kvar som intern begäran, medan retail-MAP-selectors förblir
opaka tills en Saturn-capture binder event-dispatchen. Playback och övriga
no-draw/capture-gates är oförändrade.

- ✅ 2026-08-06 DM2 source-gated movement: removed the headless movement
  fallback that had treated missing dungeon data as a generic floor. Runtime
  move and turn now require the same hash-verified boot-owned GRAPHICS.DAT,
  DUNGEON.DAT and GDAT callback binding used by the renderer; fixture-only
  dungeons cannot alter party position or facing. The collision decoder stays
  isolated and explicitly tested outside the live gameplay boundary.

- ✅ 2026-08-06 DM2 legacy-loop input correction: `fs_game_tick_v1()` no
  longer interprets DM2 keyboard/touch commands by mutating its generic
  DM1-style party fields. It sends movement and turning to the verified DM2
  boot/runtime boundary, then mirrors only the returned source state; absent
  boot state drains stale commands without creating a session.

- ✅ 2026-08-06 DM2 source-owned HUD stat pairs: M11 now reads the three
  current/maximum pairs directly from authenticated PC-DOS `c_hero` records
  (offsets 54/56, 58/60 and 62/64) and applies SKProject's effective-max-MP
  rule through the existing champion-stat bridge. The old convenience record
  never supplied the stamina/mana maxima, so it can no longer create an
  apparently complete dynamic HUD; unbound records remain no-draw.

- ✅ 2026-08-06 DM2 spell-feedback text gate: removed the synthetic English
  failure labels from the runtime status accessor. SKProject's
  `PROCEED_SPELL_FAILURE` preserves C068--C070 panel state and draws the
  NEED_FLASK GDAT image for class `0x30`; Firestaff now retains only that
  source failure class until those original consumers are bound.

- ✅ 2026-08-06 DM2 movement-cadence no-fabrication correction: removed the
  runtime's post-commit one-frame `glbIsPlayerMoving` substitute. SKProject
  renders the saved old pose while a walk-delay countdown runs, then commits
  through `PERFORM_MOVE`; the active V1 state lacks those source-owned hero,
  inventory and spell-effect inputs, so it now renders the settled pose rather
  than applying the real 700/701 plane offsets at a false time. The isolated
  party walk-delay helper now delegates to the source-locked
  `DM2_CALC_PLAYER_WALK_DELAY` receipt instead of a conflicting local formula.

- ✅ 2026-08-06 DM2 PC-DOS menu image-route provenance correction: renamed
  the `TITLE/0/4` decoded-image receipt that had been called a fallback. The
  established PC-DOS profile uses that real 320×200 GDAT image when no raw
  `SHOW_MENU_SCREEN` record exists; the startup gate still rejects every
  generated menu overlay and every missing original route.

- ✅ 2026-08-06 DM2 startup host-text closure: removed active hard-coded
  English startup, new-game, resume and load status strings from the M11
  receipt path. Menu actions preserve their source-gated structured result,
  including the `GAME_LOAD` control-flow boundary, but M11 now leaves status,
  inspect and log text empty until an original GUI/dialogue text owner is
  connected.
- ✅ 2026-08-06 DM2 startup/runtime GDAT-label closure: removed M11's
  `STARTUP GDAT`, credits, frame-blocked and `RUNTIME GDAT` labels, plus
  runtime-bind ready/failed text. Real TITLE and runtime pixels remain
  source-gated and fail closed when unavailable; their structural receipts are
  retained without a host-authored status panel.
- ✅ 2026-08-06 DM2 FM Towns English companion gate: English requests for the
  Japanese FM Towns CD now require an explicit, canonical PC-English
  `GRAPHICS.DAT` companion selected by M12. The companion is MD5-gated,
  consumed only in RAM and only for decoded GDAT text; there is no sibling-path
  lookup, disk extraction or generated translation. The real-media test proves
  the Towns CD stays the runtime owner while `FIGHTER` is read from the
  authenticated PC text corpus.
- ✅ 2026-08-06 DM2 FM Towns text-query handoff: the source-locked
  `c_gfx_str.cpp::DM2_QUERY_GDAT_TEXT` bridge can now consume a bounded,
  already-decoded companion entry before the selected GDAT cipher path. It
  leaves the native entry untouched when no companion post exists and still
  runs the shared original `FORMAT_SKSTR` consumer.
- ✅ 2026-08-06 DM2 QueryDB GDAT-text relay: the formerly empty text-query
  stub now forwards byte-validated keys to the original callback contract from
  `skcore.cpp::QUERY_GDAT_TEXT` (2636:02F8). It preserves the caller-owned,
  decoded and `FORMAT_SKSTR`-expanded buffer, rejects out-of-range keys rather
  than wrapping them to unrelated data, and is covered with a `FIGHTER` text
  callback proof.
- ✅ 2026-08-06 DM2 creature viewport rect index: restored the previously
  missing `DM2_QUERY_CREATURE_BLIT_RECTI` from SKProject's
  `skgdtqdb.cpp:4995` and its `util.cpp:147` 5×5 rotation. The QueryDB test
  now covers the identity and all three clockwise rotations used by source
  creature placement.
- ✅ 2026-08-06 DM2 runtime text sanitization: removed host-authored action,
  shop, door, movement, inventory and quicksave labels from the live M11/boot
  route. Runtime receipts, save-writer refusal and real door/movement state
  remain intact, but no status or inspector replacement text is shown until a
  matching original GDAT/dialogue owner is wired.
✅ 2026-08-06 Nexus startup menu text sanitization: production save/chrome
builders no longer emit hoststrängarna `DUNGEON MASTER NEXUS`, `LOAD GAME`,
`NEW GAME` eller `LOAD SLOT ##`. Riktig radgeometri och källans slot-identitet
behålls som receipts; textfälten är tomma tills Saturns TEXT4/TABL/FONT012-
konsument och placering är capture-bundna.
- ✅ 2026-08-06 Nexus DGN Structure1B material census and bounds gate:
  hashverifierade europeiska LEV00–LEV15 visar selektorer `0x01..0x7D`,
  medan både `SN_FLOOR.MNS` och `SN_WALL.MNS` har 15 TEXT-deskriptorer.
  Direkt selector→MNS-ordinal är därmed motbevisad och förblir capture-gated;
  materialplaneraren avvisar nu även framtida material-/Structure2-index utanför
  den dekoderade bankens bounded surface-count. Retail MNS/material-regression
  passerar.
- ✅ 2026-08-06 Nexus CDDA selector quarantine: retail CUE/ISO evidence still
  admits the eight CD-DA tracks (2–9), but no source-owned level selector was
  found in the retained DM.BIN/disassembly. The former `level / 2` mapping was
  removed from runtime and audio receipts; unknown level→track selection now
  remains `-1` and playback stays gated. Related stale music docs are marked
  metadata-only.
- ✅ 2026-08-06 CSB FM Towns ZIP scanner crash: initialized the M12 version
  catalog before the special raw-CD admission path records its verified
  `CDATA`/`CJDATA` language variant. A retail FM Towns ZIP as the only CSB
  candidate now completes the scan, reports CSB READY and materializes the
  hash-verified English pair instead of dereferencing an uninitialized
  `versionId`. Verified against the original 484 MiB MODE1/2352 image and
  both the file-backed ISO parser and scanner path.
- ✅ 2026-08-06 Theron regular creature spawn boundary: corrected the real-data
  mechanics probe so it no longer links `theron_v1_compat.c` over the
  production archive. The five Track 02 spawn-zone/category tables remain
  source receipts, while live creature publication, combat and loot stay
  blocked until the bank-switched RNG consumer is captured. The probe now
  verifies the production no-spawn/no-combat/no-drop boundary against the
  authentic JP/US Track 02 level-0 grid.
- 2026-08-06 Nexus event-owner quarantine: removed the production path that
  inferred live door, teleporter, pit, and stairs routes from DGN square
  values plus Structure1F destination fields. The verified SDDRVS asset is a
  sound-driver task, while SLEV/SAL event dispatch remains capture-gated;
  explicit source-bound registries and no-draw behavior remain available for
  future Saturn evidence.
- ✅ 2026-08-06 Nexus CDDA readiness wording: corrected the audio status table
  to say that tracks 2–9 are a disc-layout receipt only, and added a runtime
  regression proving that manual track selection does not claim playback,
  invent a level binding, or produce a ready receipt.
- 2026-08-06 Nexus startup/menu regression: corrected the inverted exact-row
  assertion in `test_nexus_v1_launcher_bpk_no_draw_presentation`. A validated
  PRS3 row is now tested as admitted opaque no-draw evidence, while payload,
  compression, mode and bounds drift remain rejected. BPK no-draw presentation,
  M11 host, and Saturn-card startup tests all pass.
- ✅ 2026-08-06 DM1 HoC C127 D3 side/depth material: fixed the mismatch
  between ReDMCSB's raw D3L2/D3R2 `-2/+2` offsets and M11's normalized
  F0128 `-1/+1` viewport offsets. Real PC34 D3/D2/D1 coverage now retains
  the authenticated C346 wall backing at every admitted side/depth view;
  no C026 portrait or procedural fallback is introduced away from D1C.
  Verification: `test_dm1_v1_champion_mirror_pc34_compat` 68/68,
  `test_m11_dm1_hoc_mirror_side_depth_material_receipt`, and
  `test_m11_dm1_hoc_real_mirror_viewport_material` pass against the real
  PC34 `GRAPHICS.DAT`/`DUNGEON.DAT` corpus.
- 2026-08-06 Nexus stale-claim quarantine: corrected the linked world and
  provisional script-VM comments so native save/event/timer state no longer
  claims ReDMCSB or SDDRVS source equivalence. The runtime's existing
  authenticated-dispatch gate remains unchanged; no unproven gameplay action
  was enabled.
# Nexus FONT256 real-data probe correction (2026-08-06)

- ✅ Removed the stale real-data assertions that sent `FONT256.S2D` through
  the flat 1bpp fixture parser and reported 256 drawable glyph slots.
- ✅ Track-1 launch/readiness probes now use `nexus_v1_font_s2d_decode()` and
  `nexus_v1_font_load_from_s2d()`, verifying the named retail regions and
  exactly 242 real 8x8 character-generator tiles. Page/attribute character
  mapping and HUD framebuffer placement remain blocked.
- ✅ The S2D glyph-byte probe now treats its byte-window map as fixture-only;
  its real branch verifies CG-region bytes and deterministic 242-tile source
  handoff. The real text-layout branch records source regions without drawing.
- ✅ Real-data verification: Track-1 phase launch 57/57, screen readiness
  29/29, glyph-byte probe 250/250 and runtime layout probe 172/172 passed.

# Nexus explicit real-data menu/HUD CTest gates (2026-08-06)

- ✅ Added `nexus_v1_bpk_surface_class_real` and `nexus_v1_stmp_real`. They
  select the external `FIRESTAFF_NEXUS_DATA_DIR`, require the real `MENU.BPK`
  and `STABG.BIN` files, and return skip-safe code 77 when the private corpus
  is unavailable.
- ✅ The real MENU.BPK path verifies all 162 PRS3 surfaces and keeps runtime
  decode/upload blocked; the real STABG path verifies the STMP receipt. The
  data-free tests remain separate and continue to run without game media.
- ✅ Focused CTest: 10/10 passed against `/Users/bosse/.firestaff/data/nexus`;
  missing-data probes returned 77 as intended. `git diff --check` passed.

- ✅ 2026-08-06 DM1 FM Towns English title runtime consumer: after the
  selected legacy GRAPHICS.DAT is bound, M11 validates the selected EDM.EXP
  directory receipt and presents the real graphic-1 PRESENTS frame, the
  source-bound 18-step zoom and MASTER frame. Missing or mismatched FM Towns
  startup media fails closed rather than entering the PC34 title path.
  Japanese JDM and the native FM Towns menu/TBIOS/CD-audio consumers remain
  explicitly open in TODO.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT bootstrap receipt: the F31 Game
  handoff now records the selected retail CD bootstrap independently of
  user saves. It authenticates `CDATA/MINI.DAT` (42 776 bytes, FNV-1a
  `494999c9`) for English and `CJDATA/MINI.DAT` (43 208 bytes, FNV-1a
  `284799d1`) for Japanese, following ReDMCSB `CEDTDATA.C` G2297 and
  `LOADSAVE.C` F0435's native header path. It deliberately does not pass
  either file to the Atari/Amiga GAMEBLOCK decoder or advertise Resume.
  Real English and Japanese F31 Switch→Game handoff tests pass.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT header verification: the F31 Game
  receipt now runs the selected retail bootstrap's first 512 bytes through
  ReDMCSB `CEDTINC6.C` F7061 with CSB key word 29, then requires its
  decrypted header to be C5, the family that includes FM Towns CSB. The
  English `CDATA/MINI.DAT` key is `0x340f`; Japanese `CJDATA/MINI.DAT` uses
  `0xf77d`. This authenticates the native header without treating its body as
  an Atari/Amiga save or enabling Resume. Real F31E and F31J handoff tests
  pass.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT header ownership: after F7061 admits
  the real header, F31E now requires its F7 English platform marker and F31J
  requires F8 Japanese, with both retaining the C13 CSB-Game dungeon marker.
  This makes the bootstrap receipt reject a language-crossed or wrong-dungeon
  header before any later save-body work.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT save-part receipt: after the native
  header, the Game handoff verifies the original F7057 checksums for
  GlobalData, active groups, champion/party data, events and timeline. Both
  retail files expose one party champion, 60 active-group slots, 436 event
  slots and end the authenticated part sequence at byte 8 236. This is a
  source-backed corpus check, not a dungeon-tail decoder or Resume path.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT dungeon-tail receipt: the F31 Game
  handoff now follows ReDMCSB `CEDTINCA.C` F7063's native tail order after
  the four external portraits. Both retail files verify 11 maps, 296 columns
  and their trailing F7059 byte-sum checksum (English `0x62df`, Japanese
  `0x6671`). The result remains an admission receipt, not a live save restore.

- ✅ 2026-08-06 CSB MAP origin correction: the source loader now reads
  ReDMCSB `DEFS.H` MAP `OffsetMapX/Y` from bytes 6/7, not the unrelated
  byte-4/5 padding. The first authentic F31 MINI map therefore retains its
  real origin `(17,14)` in the Game receipt instead of a synthetic `(0,0)`.

- ✅ 2026-08-06 CSB FM Towns MINI dungeon consumer: the F7063-authenticated
  tail is now copied only through a receipt-bound API and opens in the real
  CSB dungeon loader. The F31E corpus proves all 11 maps and its first map's
  `(17,14)` origin; no raw save bytes are promoted to a live resumed world.

- ✅ 2026-08-06 CSB FM Towns MINI party-pose receipt: the F7057-decrypted
  `GLOBAL_DATA` now retains GameTime and the original party pose, and F7063
  rejects a pose outside its selected map. F31E proves tick 82 at map 4
  `(22,18,S)`; F31J proves tick 88 at the same pose. Champion-body decoding
  and live restoration remain deliberately separate.

- ✅ 2026-08-06 CSB FM Towns Utility P3 boundary: `UTILE.EXP` and
  `UTILJ.EXP` now must pass their original Phar Lap level-1 P3 envelope in
  addition to the full-file identity gate. The receipt records the real
  384-byte header, 512-byte load-image offset, English 151 875-byte / EIP
  `0xfe00` and Japanese 151 987-byte / EIP `0xfeb0` program boundaries.
  ReDMCSB `COMPILE.H` EXEID 63/64 identifies the pair as C06_CEDT. This
  intentionally does not substitute the existing PC34 utility flow for the
  native TBIOS editor or its save transactions. Real F31E and F31J handoff
  tests pass.

- ✅ 2026-08-06 CSB FM Towns C06 menu byte receipt: disassembly of the
  verified P3 load images identifies the first Utility menu pool and binds
  it by raw offset, length and FNV-1a. F31E exposes its six original labels
  (`LOAD CHAMPIONS`, `SAVE CHAMPIONS`, `MAKE NEW ADVENTURE`, `REVERT`,
  `UNDO`, `QUIT`) from virtual `0x11578`; F31J exposes the corresponding
  68-byte Shift-JIS pool from `0x11628`. The receipt keeps the Japanese text
  as original bytes and does not manufacture translated host labels. English
  and Japanese real-media handoff tests pass.

# Nexus level-bound consistency (2026-08-06)

- ✅ Nexus sound-bank loading and mechanics level admission now use the
  canonical `NEXUS_MAX_LEVELS` bound instead of duplicated literal `15`
  checks.
- ✅ Real European SAL/MAP corpus verification still passes for all 16 levels;
  event dispatch and playback remain fail-closed.
- ⚠️ The aggregate build remains blocked later by the unrelated DM2 FM-Towns
  animation-stream link gap recorded in `TODO.md`; the Nexus archive itself
  builds.
# Nexus Font256 production section-parser link (2026-08-06)

- ✅ `firestaff_nexus` now links the existing real SEGA SATURN SCR section
  parser required by Font256 admission.
- ✅ `FIRESTAFF_NEXUS_PRODUCTION` compiles out the unproven flat glyph loader
  and host framebuffer writer; no synthetic text pixels enter production.
- ✅ Font256 section-witness, first-section and corpus targets build, and the
  real 25,012-byte `FONT256.S2D` section-table probe passes 55/55.
- ✅ The production-source boundary verifier now checks the compile guard and
  parser inclusion instead of requiring the whole translation unit to be
  excluded.
- ℹ️ The aggregate project build now passes the Nexus archive and stops later
  at an unrelated DM2 FM-Towns animation-stream link gap.
# 2026-08-06 Nexus UI-event dispatch boundary

- ✅ Retail ISO/extracted Nexus now rejects host UI events before the Saturn
  SLEV/SDDRVS producer, queue and state-write contract is captured. This closes
  direct automap, inventory, save, leader, throw and drop mutations while
  retaining the source-less compatibility lane.
- ✅ Added the production-boundary regression for automap and command-state
  immutability.
- ✅ The public level-transition helper now shares the same retail gate, so a
  pending compatibility transition cannot bypass the tick boundary and load a
  synthetic retail DGN level. The production regression covers the rejected
  call and output state.
# Nexus Saturn raw VDP1/VDP2 runtime witness (2026-08-06)

- ✅ Built the patched Mednafen 1.32.1 Saturn producer on the external disk.
  The binary contains the `ss` module and the Firestaff raw-capture hook.
- ✅ Ran the European BIOS against the European DM Nexus ISO through a
  data-only CUE and retained a two-frame, 3,155,092-byte raw witness outside
  the repository. Mednafen identified `T-9111G`, `DUNGEON MASTER NEXUS`, and
  the European area.
- ✅ Added `scripts/validate_nexus_saturn_runtime_capture.py`, which checks the
  capture magic, ordered frame markers, and exact VDP1/VDP2 payload lengths.
  It explicitly reports semantic admission as blocked: PRS3, SLEV/SAL/SDDRVS,
  HUD, and viewport routes remain gated.
- ⚠️ The supplied European CUE references missing Japanese audio-track files;
  the capture used a temporary data-only CUE pointing at the same European ISO.
# Nexus European Saturn startup capture correction (2026-08-07)

- ✅ Confirmed media provenance with the supplied E-BIOS: the English ISO is
  `SGAREA U`, the merged English image is `SGAREA J`, and the French ISO is
  `SGAREA E`. Only the French ISO is used for the European capture chain.
- ✅ The E-BIOS + French-media raw captures validate through the external
  VDP1/VDP2 validator. At later frame windows they show authentic TrueMotion
  publisher graphics and a changing orange startup animation in the VDP1
  framebuffer. No host pixels or semantic menu/HUD/viewport admission was
  added.
- ✅ The Japanese BIOS attachment was hash-verified separately for the J-region
  comparison path; its evidence remains separate from the European chain.

# Nexus Saturn startup input capture route (2026-08-07)

- ✅ Added an external-only Mednafen SMPC route for a bounded active-low START
  pulse, selected by emulated frame and hold length. It writes no VDP/SH-2
  state and records the input window in the operator manifest.
- ✅ Rebuilt the existing external Saturn producer’s SMPC object and relinked
  the instrumented binary; patch dry-run and launcher regression pass.
- ⚠️ E-region tests at frame 1000/60 frames and frame 4500/2 frames still show
  authentic intro imagery after the input window. Menu, HUD, viewport and
  PRS3/SLEV/SAL/SDDRVS semantic admission remain blocked pending a proved
  transition and source-owned consumer bindings.
- ✅ Extended the operator route with the Saturn gamepad A-bit mask (`0x20`)
  and combined START+A mask (`0x30`); launcher, patch dry-run and relinked
  external binary checks pass. No menu claim was made from the frame-7500
  intro capture.

# Nexus Saturn A/START+A runtime window (2026-08-07)

- ✅ Ran the real European ISO with E-BIOS, a 60-frame combined START+A
  window at emulated frame 6500, and raw VDP1/VDP2 capture beginning at frame
  8000. The four captured frames remain authentic intro/fire imagery; no menu
  transition was observed.
- ✅ Re-ran the real DM.BIN startup/menu resource-anchor test and startup-media
  gate against `/Users/bosse/.firestaff/data/nexus`; both passed. These prove
  source ownership and asset admission, not Saturn menu placement.
# Nexus Saturn capture window follow-up (2026-08-07)

- ✅ Fixed the raw-capture launcher’s instrumented-binary check so `pipefail`
  cannot turn a valid `strings` match into a false exit-78 rejection.
- ✅ Captured additional authentic E-BIOS/French-media VDP1/VDP2 windows at
  intro frame offsets 2400, 3000, and 4200. They remain raw transport/layout
  witnesses: the validator passes, while PRS3, menu, HUD, viewport, and
  SLEV/SAL/SDDRVS semantic admission remains correctly blocked.

# Nexus SCSP read-trace boundary (2026-08-07)

- ✅ Added a reproducible Mednafen sound-CPU SCSP-read producer with bounded
  address/PC filters and a strict trace analyzer.
- ✅ Ran it against the authenticated European French gameplay window. The
  100-row receipt contains real shared-RAM/driver reads, but no mailbox read
  at `0x100400..0x100401` and no `0x3224`-filtered read; SLEV/SAL semantics
  and host playback therefore remain fail-closed.

# Nexus VDP1 source-writer corridor (2026-08-07)

- ✅ Extended the VDP1 write-trace analyzer with exact PC and address-range
  requirements.
- ✅ Verified an authentic European startup window with 4,601 writes from
  runtime PC `0x06013098` into `0x47c00..0x49ffe`; known framebuffer/colour
  writers remain separately classified. The runtime writer is not promoted
  to a named retail asset or production draw route without source identity.

# Nexus VDP1 writer code-window receipt (2026-08-07)

- ✅ Added a reproducible Mednafen producer patch that captures the live SH-2
  code window around the VDP1 writer PC, plus a strict validator.
- ✅ Captured the authentic European runtime PC `0x06013098` with VRAM target
  `0x47c00` and 48 code words. Little-endian SH-2 disassembly shows the live
  branch at `0x06013098` to `0x06012f52`.
- ✅ Kept source-file identity, VDP1 command/CLUT ownership and production
  rendering blocked: the relocated code window is runtime evidence, not yet
  a byte-for-byte DM.BIN/TM.BIN join.
# Nexus relocated-code loader receipt (2026-08-07)

- ✅ Added a bounded Mednafen SH-2 high-RAM write producer and validator.
- ✅ Verified 3,080 authentic writes into `0x06013000..0x06013fff` from the
  runtime loader PC `0x00002368` in the European startup window.
- ✅ Kept the retail source member unbound: BIOS/runtime-loader ownership does
  not prove DM.BIN, TM.BIN, a video asset, or any VDP1/VDP2 consumer contract.

# Nexus Saturn CDB sector-trace receipt (2026-08-07)

- ✅ Added a bounded Mednafen Saturn-CDB read hook at the actual
  `src/ss/cdb.cpp` data-sector path, with an external-only patch and no game
  data copied into the repository.
- ✅ Compiled the instrumented Mednafen build and captured the BIOS window:
  1,024 reads covering LBA `0..16`; this is authentic BIOS/CD startup traffic,
  not yet a retail Nexus member read.
- ✅ Kept source identity, relocated-code admission, SLEV/SAL playback and
  VDP1/VDP2 production composition blocked until a later CDB window reaches
  and joins an authenticated ISO file span.

# Nexus Saturn retail CDB join receipt (2026-08-07)

- ✅ Re-ran the European-BIOS/French-media startup through the corrected
  SMPC input hook, which now runs after Mednafen's virtual-port update. The
  bounded CDB trace contains 50,000 authentic data-sector reads over LBA
  `0..59951`; ISO9660 joining resolves all six required members and reports
  `DM.BIN` (8,212 reads), `TM.BIN` (173), `ITEM.IBS` (72), `MENU.BPK` (44),
  `SLEV00.BIN` (61), and `SDDRVS.TSK` (14), with `LEV00.DGN` and
  `SNDLEV01.SAL` also observed.
- ✅ Added `scripts/analyze_nexus_cdb_read_trace.py` as a reproducible,
  read-only ISO9660/LBA join gate. It emits `retail_lba_join=verified` but
  deliberately ends with `semantic_admission=blocked`.
- ✅ The same session retained 3,080 high-RAM writes in
  `0x06013000..0x06013fff` from runtime loader PC `0x2368` and one authentic
  Saturn runtime frame. This strengthens the temporal capture receipt only;
  it does not identify the bytes' producer or prove VDP1/VDP2 draw order,
  CLUT ownership, HUD composition, SLEV/SAL dispatch, or SFX playback.
- ⛔ No VDP1 writer trace was emitted in this combined bounded run. Keep
  production face/mesh/texture, HUD/viewport, SLEV/SAL/SDDRVS and PRS3
  consumer admission closed until a trace joins a live writer/consumer to
  the authenticated retail bytes.

# Nexus Saturn runtime source-to-VDP1 provenance receipt (2026-08-07)

- ✅ Corrected the capture invocation to use the actual VDP1 trace variables
  (`FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN/MAX`). The authentic E-BIOS/French
  run now records 39,936 VDP1 VRAM-write rows and a writer code window at
  runtime PC `0x06013098` targeting `0x47c00`; eight raw frames pass the
  transport validator with non-idle VDP1 activity.
- ✅ Added the external-only SH-2 source-read/source-write witness and its
  strict `scripts/analyze_nexus_sh2_source_trace.py` ISO join. In the same
  `skip3000`/frame-1000 startup window, complete contiguous 4 KiB runtime
  chunks match `TM.BIN` at ISO offset `0x74f3000` and `DM.BIN` at
  `0x5e000`; `0DMSTRT.BIN` and `SWTCHR.BIN` also match exactly. Partial
  prefixes and inferred addresses are rejected.
- ✅ The same bounded run joins 50,000 CDB reads to all required retail
  members, including `DM.BIN` (8,212), `TM.BIN` (173), `ITEM.IBS` (72),
  `MENU.BPK` (44), `SLEV00.BIN` (61), and `SDDRVS.TSK` (14).
- ✅ The eight-frame raw witness was decoded with
  `scripts/analyze_nexus_vdp1_command_window.py`: `COPR=0x00000c` exposes the
  four-record Saturn chain (system `0x09`, system `0x0a`, type-2 bitmap draw,
  END). Frame 7 carries `PMOD=0x0028`, `SRCa=0x8f80`, `SIZE=0x28b4`; its
  encoded source address is VDP1 byte offset `0x47c00`, matching the live
  writer corridor at PC `0x06013098`. The observed source span is 33,280
  bytes through `0x4fe00`.
- ✅ Added the external-only VDP2 write witness and analyzer. The same
  authenticated run records 15,365 VDP2 register writes, 183,355 VRAM writes
  and 1,280 CRAM writes; the trace covers the three hardware lanes and passes
  `scripts/analyze_nexus_vdp2_write_trace.py`. The strengthened hook now
  records nonzero SH-2 PCs: dominant VRAM writers are `0x06011924` (65,538),
  `0x060118fc` (40,448) and `0x06002fc4` (22,914), while register writes are
  dominated by `0x0600231c` (14,400). This is executing-code ownership, not
  yet a decoded tilemap/CLUT or production presentation proof.
- ✅ Added the VDP2 writer code-window witness and strict retail comparison.
  The authenticated run records 64 unique SH-2 windows; the primary
  `0x06011924` window contains `25fe 0000 25fe 007c ...`, and the setup
  window at `0x06001416` contains the VDP-register literal pairs. The exact
  48-word windows do not occur verbatim in hash-verified `TM.BIN`/`DM.BIN`,
  so the analyzer keeps source identity and semantic promotion blocked.
- ⛔ This proves retail byte provenance and a live VDP1 writer, not the
  writer's decoded face/mesh/texture consumer, VDP2 tilemap/CLUT ownership,
  HUD/viewport draw order, or SLEV/SAL/SDDRVS event semantics. Production
  semantic admission therefore remains fail-closed.
# Nexus MENU.BPK external-root regression (2026-08-13)

- ✅ Corrected `test_nexus_v1_bppk` so its primary MENU.BPK decode uses
  `FIRESTAFF_NEXUS_DATA_DIR`; the HOME-relative path remains only as the
  documented default when no root is configured. With the external real Nexus
  corpus, the test verifies 164 archive entries, 162 PRS3 surfaces and 162/162
  successful indexed-surface decodes.
- ✅ Re-ran with `HOME=/tmp/firestaff-nexus-no-home` and the external data root;
  the same real-data result passes, proving the test does not read a stale
  HOME-local or synthetic MENU.BPK copy.

# Nexus startup/menu external-root regression (2026-08-07)

- ✅ Updated the real-data `FONT256.S2D`, `FACE.BIN`, `TITLE.CG`/RES* and
  `STABG.BIN` probes to prefer `FIRESTAFF_NEXUS_DATA_DIR`, retaining the HOME
  path only as a compatibility fallback. This keeps startup/menu provenance
  on the mounted external corpus and does not enable any capture-gated pixels.
- ✅ Verified all four probes with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus` and
  `HOME=/tmp/firestaff-nexus-no-home`: real FONT256, 20 authenticated FACE
  portraits, TITLE/RES archives and STABG all pass.

# Nexus viewport/audio corpus external-root regression (2026-08-07)

- ✅ Extended the external-root-first contract to the legacy real-data probes
  for all 16 `LEVxx.DGN` levels, `LOGOBG.DG2`, raw Saturn binaries including
  `DM.BIN`/`SDDRVS.TSK`, and all 16 `SNDLEVxx.SAL`/`.MAP` pairs. HOME remains a
  compatibility fallback only when the explicit root is absent.
- ✅ With `HOME=/tmp/firestaff-nexus-no-home`, the external corpus verified all
  16 DGN decodes, LOGOBG 320x224 source geometry, raw-binary receipts and all
  real SAL/MAP metadata profiles. SAL playback, SDDRVS dispatch and VDP1/VDP2
  presentation remain capture-gated.

# Nexus DMDF/MNS real TEXT material corpus (2026-08-07)

- ✅ Extended `test_nexus_v1_mns` to consume the authenticated external Nexus
  corpus through the production DMDF `TEXT` descriptor and BGR555 material-bank
  route. All 30 retail MNS models retain matching descriptor counts; the corpus
  decodes 815 source textures, with 23 indexed material banks and 587 BGR555
  surfaces verified.
- ✅ The two static Saturn material sources, `SN_FLOOR.MNS` and `SN_WALL.MNS`,
  both decode completely. Seven creature banks remain explicit source-only
  descriptor receipts because their colour cardinality exceeds the current
  indexed host bank; no lossy palette or placeholder surface was introduced.
- ✅ Verification used
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus` with
  `HOME=/tmp/firestaff-nexus-no-home`; `test_nexus_v1_mns` passed, including
  real OBAKE MOTN sampling and 75 transformed source vertices. VDP1/VDP2
  placement and final viewport presentation remain capture-gated.

# Nexus MNS exact direct-colour source lane (2026-08-07)

- ✅ Updated the production DMDF `TEXT` material-bank decoder so real textures
  with more than 256 unique BGR555 colours are retained losslessly as exact
  `uint16_t` source pixels. The decoder no longer rejects those seven creature
  banks and never invents a quantized palette.
- ✅ Kept indexed and direct-colour ownership separate: direct-colour surfaces
  have no indexed `pixels` buffer, so the existing viewport admission gate
  cannot mistake them for render-ready VDP1 materials. The two static banks
  remain indexed and fully decoded.
- ✅ Retail regression now verifies 30 complete TEXT banks and 815 surfaces,
  including seven direct-colour source banks, plus the DGN material raster and
  face/material retail corpus tests. Verification used the external Nexus data
  root with an isolated `HOME`; all focused tests passed.

# Nexus Saturn capture link-path repair (2026-08-07)

- ✅ Closed the remaining Mednafen witness-chain link gap: the SH-2 source
  trace now carries its own `FirestaffGetSH2PC()` declaration/definition and
  its source-trace/PC helper hunks have valid insertion counts. A fresh tree
  applies the complete Saturn capture chain, and the external producer links
  successfully with the real VDP1/VDP2/CD hooks.
- ✅ Kept this as producer evidence only. No runtime asset, menu, HUD or
  viewport admission is changed by the link repair.

# Nexus Saturn capture toolchain repair (2026-08-07)

- ✅ Repaired the ordered Mednafen 1.32.1 Saturn witness patch chain: CD reads,
  SH-2 source/memory peeks and VDP2 VRAM/CRAM/register writes now apply cleanly
  to a fresh upstream tree without placing hooks in unrelated read paths.
- ✅ Verified the clean chain against the external Mednafen source and compiled
  the affected `vdp2.o`, `cdb.o` and `ss.o` objects. This proves the producer
  toolchain is buildable; it does not claim a new authenticated runtime capture.

# 2026-07-31 Nexus STABG retail-yta till startup-media

- ✅ `nexus_ui_load_stabg()` materialiserar DMWeb:s verifierade första STMP-karta
  från den riktiga `STABG.BIN`-filen som 320×168 indexyta och sparar filens 256
  Saturn-paletteord samt deterministisk RGBA-expansion. Startup räknar därmed
  ytan som laddad i stället för fallback; ingen host-palette eller syntetisk
  HUD-grafik används. VDP1/VDP2-placering och runtime-state-bindning är fortsatt
  blockerade tills de kan bevisas från Saturn-källan.
  Källa: DMWeb `DecodeSTABGBIN` och lokal retail `STABG.BIN`.
  Verifiering: `test_nexus_v1_startup_media_gate` mot
  `/Users/bosse/.firestaff/data/nexus`.
# 2026-07-31 Nexus PRS3-headergräns

- ✅ PRS3-headerns komprimerade storlek kontrolleras nu utan signerad
  heltalsaddition som kan wrap:a vid korrupta eller mycket stora fält.
  DMWebs little-endian-bitflöde och offsetregler är oförändrade. Den riktiga
  `MENU.BPK`-proben fortsätter att avkoda alla 162 PRS3-ytor, medan separat
  Saturn-palette-/VDP1-/menysemantik fortfarande krävs innan render-gaten kan
  öppnas.
  Verifiering: `test_nexus_v1_bpk_surface_class` med lokal retailfil.
# 2026-07-31 Nexus FACE.BIN retailpalette till uppstart

- ✅ Uppstartens FACE-loader skickar nu hela DMWeb-frameprefixet till
  porträttytan i stället för att kasta bort de första 128 bytesen. De 64
  källägda big-endian BGR555-orden per porträtt sparas och expanderas till
  RGBA; den tidigare hårdkodade `192..207`-palette-lanen används inte längre.
  PRS3-pixlarna är fortfarande no-draw tills champion-index och Saturn
  VDP-placering är bevisade.
  Verifiering: `test_nexus_v1_face_bin` avkodar alla 20 retailporträtt.
# 2026-07-31 Nexus SAL DataID 0 directory provenance

- Added the DMWeb `DMNDataFileDecoder.vbs` `DecodeSNDLEVxxMAP` tone-bank
  parser to the Nexus sound runtime. It walks the real MAP-owned SAL parts,
  locates DataID 0, validates its big-endian offset table and entry bounds,
  decodes the four variable entries plus `4 + 32*n` entries, and records
  PCM width/source-control and sample-payload metadata.
- The runtime still refuses playback because Saturn event→selector ownership
  and the `SDDRVS.TSK` ABI are not authenticated. No synthetic sample or
  fallback audio was introduced.
- Verification: `test_nexus_v1_sal_map_corpus` and
  `test_nexus_v1_sound_runtime_receipt` pass against
  `/Users/bosse/.firestaff/data/nexus`.
# 2026-07-31 Nexus real-data viewport boundary audit

- Ran the DGN multi-level parser, material-raster, material-corpus and launch
  probes against `/Users/bosse/.firestaff/data/nexus`.
- All 16 `LEV*.DGN` files parse and the launch smoke reaches level 0, but the
  real material corpus reports `geometry_ready_level_count=0`, incomplete
  ceiling/wall host coverage, and no authenticated MNS/BPK host route.
- Kept the viewport fail-closed; no procedural or fixture material was
  promoted. The remaining owner is authenticated Saturn VDP1/VDP2 submission.

# 2026-07-31 Nexus item-mechanics provenance audit

- Audited the real `ITEM.IBS` binding against the live movement/item paths.
- `ITEM.IBS` proves declaration category, weight, image and string ordinals;
  it does not prove action, equipment, protection or creature-drop semantics.
- Recorded the remaining raw-ordinal `65/80` water/fire gate and dormant gold
  helper as explicit gaps in `TODO.md`. No guessed item meaning or synthetic
  loot/HUD label was promoted.
# 2026-08-05 Nexus MNS retail corpus verification

- Materialized the original English ISO's MNS model files into the configured
  local Nexus data root; all 30 documented roster models decode as DMDF.
- The real MNS test rendered 452 source textures and exercised OBAKE MOTN
  animation, transforming 75 vertices with `0` failures.
- No MNS pixels were promoted into the blocked DGN/VDP1 viewport route.

# 2026-08-05 Nexus DGN material-surface admission hardening

- ✅ The real DGN viewport now validates every selected MNS/BPK/Structure2
  surface before palette access or rasterization: bank bounds, `valid`, pixel
  ownership and positive dimensions are required. An authenticated animated
  Structure1G/Structure2 reference cannot silently fall back to a static
  Structure1B tile when its image is absent. Invalid material admission leaves
  the route blocked with no procedural substitute and records the first missing
  material command.
- Verification: `test_nexus_v1_dgn_material_raster` and
  `git diff --check` pass; no game data was added to the repository.

# 2026-08-05 Nexus ITEM.IBS gameplay placeholder removal

- ✅ The live ITEM.IBS bank now preserves byte-2 carry locations as raw
  declaration data instead of inventing `NEXUS_ITEMF_CONSUMABLE` flags.
- ✅ Real-data mechanics no longer dispatch the fixed DM1 item-ID potion,
  armour-slot or unarmed-power paths. Those compatibility helpers remain
  isolated from the authenticated Nexus route until Saturn action/combat
  semantics are bound from DM.BIN disassembly or an authenticated capture.
- Verification: `test_nexus_v1_item_ibs`,
  `test_nexus_v1_inventory_gameplay`, `test_nexus_v1_item_use`,
  `test_nexus_v1_tick_integration`, Nexus mechanics build and
  `git diff --check` pass; no game data is committed.
- ✅ 2026-08-05 Theron Track 02 thing-data loader hardening: reject an
  oversized ground-reference count before narrowing it into the source-shaped
  16-bit receipt field or calculating the copy span. Regression coverage now
  proves the overflow boundary fails closed; no real-data semantics are
  inferred or promoted.
- ✅ 2026-08-05 Theron M11 integration: production `firestaff_theron` now
  links the source-bound `theron_v1_viewport.c` lifecycle/presentation path
  instead of the total viewport no-op. Dungeon tiles, unverified chrome, and
  inferred mappings remain fail-closed; the verified Track 02 font and future
  authenticated palette/VRAM routes are now reachable by the real M11 path.
- ✅ 2026-08-05 DM1 HoC object presentation: restored ReDMCSB's D2 palette
  remap for D1/D0 wall ornaments, preventing authentic torch-holder and
  ornament pixels from becoming black silhouettes. Corrected the C00/C01
  ready/action hand slot masks so valid objects can be placed in either hand.

- ✅ 2026-08-05 DM1 leader-hand cursor: after pickup, the framebuffer draws
  the source PC34 16x16 object icon at the tracked pointer position, using
  the same F0033/F0038 icon resolver as inventory and action cells.
# ✅ 2026-07-15 Nexus Structure1F/Structure3 runtime correlation

# ✅ 2026-07-15 Nexus PRS3 zero-side static corridor identity

The retail `DM.BIN` zero-side SH-2 path is now retained as one exact 64-byte
source corridor from its branch target through its outer-loop branch. The
receipt requires FNV-1a `e0cc325e85a0e63f` before a zero-side external trace
can bind, so mutation of an otherwise unnamed intermediary instruction fails
closed. It establishes no PRS3 token, copy/backreference, palette, pixel, or
decoder semantics. Verification: `test_nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-15 Nexus FACE PRS3 capture targets

`nexus_ui_face_prs3_capture_target()` now exposes one exact canonical
`FACE.BIN` PRS3 frame for an external capture producer only after a
caller-owned source-hash gate. Prefix, PRS3 header, and compressed stream
hashes remain separate, preserving the source lanes needed for a later Saturn
loader trace without inventing palette, token, pixel, or portrait semantics.
The target remains no-draw with fallback visuals disabled. Verification:
`test_m11_nexus_startup_gate`.

# ✅ 2026-07-15 Nexus FACE PRS3 capture campaign

`nexus_ui_face_prs3_capture_campaign()` now covers every canonical FACE.BIN
frame in producer order, with a separate ledger over target framing and its
prefix/stream source lanes. The campaign rejects missing or malformed frames
before external trace analysis, but establishes no loader execution, token,
palette, pixel, or menu-placement semantics. It remains no-draw with fallback
visuals disabled. Verification: `test_m11_nexus_startup_gate`.

# ✅ 2026-07-15 Nexus direct Structure1F static-material route

`nexus_v1_engine_build_structure1f_direct_static_material_capture_target()`
now joins a documented active `Structure1F -> Structure1A -> Structure3`
face selection with the same face's exact static Structure2 descriptor and
bounded payload windows. It fails closed for non-static or unresolved faces;
the resulting target is capture-only and cannot assign texture, palette,
transform, VDP1, or draw semantics. Verification:
`test_nexus_v1_dgn_geometry_readiness` against LEV00--LEV15.

# ✅ 2026-07-15 Nexus direct Structure1F raw-fill face route

`nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target()`
conditionally joins a direct owner with its exact non-textured Structure3
face, vertices, normal, and opaque fill-selector bytes. It cannot assign a
flat colour, palette, transform, VDP1 command, or draw behavior, and textured
faces remain unavailable through this route. Verification:
`nexus_v1_direct_static_material_capture` against canonical LEV01.

# ✅ 2026-07-15 Nexus direct Structure1F 08xx material route

`nexus_v1_engine_build_structure1f_direct_animated_material_capture_target()`
conditionally joins a direct owner with its exact Structure3 08xx /
Structure1G material declaration. The route is source-only: it does not
execute the image sequence, decode a payload, assign palette or VDP1
semantics, or draw. Verification:
`nexus_v1_direct_static_material_capture` against canonical LEV01.

# ✅ 2026-07-15 Nexus active Structure1F face/mesh receipt

`nexus_v1_current_level_structure1f_face_mesh_receipt()` now consumes the
documented Structure1F -> Structure1A -> Structure3 model/face/normal ordinal
attachment only from the active, hash-bound retail LEV bytes. The focused
retail corpus verifies the receipt for every admitted level. It contains no
transform, material, texture, palette, VDP1, or draw semantics and therefore
remains no-draw with fallback visuals disabled. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 raw Saturn trace admission gate

`nexus_v1_engine_admit_structure2_descriptor_capture_trace()` now binds an
external raw capture manifest to the active hash-verified LEV source, one
exact Structure2 descriptor, its opaque post-FFFF payload, and the supplied
raw-trace bytes. The caller must independently attest original Saturn
provenance; unverified input remains blocked even after every hash matches.
Opaque admission authorizes neither a decoder nor draw, keeping the renderer
fail-closed until pixel, palette, and VDP1 semantics are actually captured.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure3 static face-to-Structure2 capture target

`nexus_v1_engine_build_structure3_static_material_capture_target()` now joins
one bounded, texture-flagged Structure3 face from the active canonical LEV to
the exact matching static Structure2 descriptor, with independent hashes for
the LEV source, the 12-byte face row, descriptor row, and opaque payload. It
also resolves the exact image-payload byte anchor and, when nonzero, the
palette-payload byte anchor from the observed Structure2-relative offsets.
The focused corpus test verifies this against real hash-verified LEV00–LEV15
package data. The target is capture-producer input only: pixels, palette
format, UVs, VDP1 state, transforms, and drawing remain unproved and blocked.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 retail format-evidence gate

`nexus_v1_current_level_structure2_format_evidence_receipt()` now consumes
the active canonical LEV and validates every descriptor's image anchor plus
any nonzero palette anchor against the opaque payload. The hash-verified
LEV00–LEV15 corpus fixes the observed split at 1,553 `0x0008` descriptors and
125 `0x0028` descriptors; all `0x0028` rows lack a palette anchor. The gate
keeps pixel span, palette addressing, VDP1 format, decoder permission, and
drawing false. This is concrete format evidence, not a format inference.
Verification: `test_nexus_v1_dgn_geometry_readiness`.
- ✅ 2026-07-15 DM1 GROUP F0181: added the exact current-map group-event
  deletion primitive. It scans event records and removes the complete
  C29..C41 range at the requested square through the existing F0237 heap
  repair path; other squares and maps remain. The DM1 event-timer regression
  covers both deletion boundaries and every retention gate. Source: ReDMCSB
  `GROUP.C` F0181:340-371.
- ✅ 2026-07-15 DM1 GROUP F0194: added source-defined all-active-group
  retirement. It composes F0194's active-slot scan with F0184's loaded C04
  writeback: Cells, low packed Direction, Behavior >= C4 to wander, then
  inactive slot. The regression covers sparse active slots and malformed raw
  references failing before any mutation. Source: ReDMCSB `GROUP.C`
  F0194/F0184.
- ✅ 2026-07-15 DM1 GROUP F0195: initial DUNGEON.DAT startup now consumes
  the current map's loaded SFT/C04 chains in original X-major/Y-minor order.
  Each first C04 receives F0181 exact-square C29..C41 deletion, the F0183
  active-state bridge with the PC3.4 60-slot limit, and F0180 C37 scheduling
  at GameTime + 1. The regression covers a mixed C03/C04 chain, raw C04
  cells/directions, event retention, and both wandering events. Source:
  ReDMCSB `GROUP.C` F0180/F0181/F0183/F0195 and `NEWMAP.C` F0003.
- ✅ 2026-07-15 DM1 GROUP F0197-F0199: added the real sight/smell square
  predicates and source route walk. Closed opaque C3/C4 doors, fakewall
  imaginary-state distinction, diagonal branch blocking, and Manhattan route
  result are covered; non-adjacent paths require a loaded-map callback.
  Source: ReDMCSB `GROUP.C` F0197-F0199.
- ✅ 2026-07-15 DM1 GROUP F0200: added the complete route-backed visible
  party decision. It preserves per-creature facing deduplication, side attack,
  invisibility and night-vision range changes, adjacent random range, then
  delegates the final line to F0199. Regression covers facing, side attack,
  invisibility, and an actual map-route blocker. Source: ReDMCSB `GROUP.C`
  F0200/F0227.
- ✅ 2026-07-15 DM1 GROUP F0201: added live direct party scent consumption.
  It calls F0199 through an F0198-backed map callback before considering the
  supplied original stored scent, preserving the source ordering and range
  gate. Regression covers clear route priority and blocked-route stored scent.
  Source: ReDMCSB `GROUP.C` F0201/F0198/F0199.
- ✅ 2026-07-15 DM1 GROUP F0202: removed the legacy clear-destination
  assumption. F0202 now accepts only supplied decoded destination facts and
  otherwise blocks movement, while retaining source terrain, Fluxcage,
  teleporter, party, door, and group ordering. Regression fixtures explicitly
  provide decoded empty squares. Source: ReDMCSB `GROUP.C` F0202.
- ✅ 2026-07-15 DM1 GROUP F0203: added the live tested-direction scan. Each
  cardinal direction is marked before its F0202 evaluation, including a
  blocker, and later directions remain untouched after a successful choice.
  Source: ReDMCSB `GROUP.C` F0203/F0202.
- ✅ 2026-07-15 DM1 GROUP F0204: added the archenemy double-movement gate.
  A first-step Fluxcage stops the source branch before the second step; the
  second step consumes its own loaded F0202 facts. Regression covers Fluxcage,
  blocked second square, and a verified clear second square. Source: ReDMCSB
  `GROUP.C` F0204/F0202.
- ✅ 2026-07-15 DM1 GROUP F0205/F0206: removed the pseudo-random opposite
  turn from the legacy helper. Opposite turns now require the live RNG form,
  which preserves F0205 one-step correction and F0206's per-creature gates.
  Source: ReDMCSB `GROUP.C` F0205/F0206.
- ✅ 2026-07-15 DM1 GROUP F0207: removed non-source projectile substitutions.
  Lord Order and Grey Lord now reject original BUG0_13's undefined projectile
  Thing without consuming RNG, and the unproven Trolin spell palette is gone.
  Verified original projectile types remain unchanged. Source: ReDMCSB
  `GROUP.C` F0207 BUG0_13.
- 2026-07-15 DM2 skproject weather material integrity: final
  `QUERY_TEMP_PICST` consumption now verifies the receipt's decoded
  ENVIRONMENT pixels and `QUERY_GDAT_IMAGE_LOCALPAL` hash. A substituted
  same-sized image or palette blocks the entire weather transaction.
- 2026-07-15 DM2 skproject door material integrity: M11 now rehashes every
  source-owned `DRAW_DOOR` decoded plane and local palette immediately before
  presentation. Altered same-sized GDAT material blocks the complete pass.

# ✅ 2026-07-15 Nexus canonical PALT capture target

The canonical MENU.BPK handoff can now write a source-bound original-Saturn
capture target for PALT's exact record and raw-table fingerprint. It requests
only PALT-memory-read, palette-state, and VDP1-command observations, leaving
the PALT-to-palette relation unproved and all decoder/draw routes blocked.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus animated Structure3 image-source route

`nexus_v1_current_level_visit_structure3_animated_material_images()` now
walks every declared non-control `Structure1G` instruction associated with an
active `08xx` Structure3 face and binds it to the exact local Structure2
descriptor capture target. The viewport consumes this as a separate no-draw
source lane. `FF FE` is not followed, no image is selected as a frame, and no
animation timing, Saturn pixel/palette/VDP1 semantics, decoder, or fallback
visual path is enabled. The focused DGN test covers one bounded 08xx face and
its source descriptor, plus the existing hash-verified retail-corpus route
when local data is available. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus canonical PALT trace admission

`nexus_v1_engine_admit_menu_bpk_palt_trace()` now admits an externally
verified Mednafen trace only after it binds the active canonical MENU.BPK,
exact PALT record fingerprint, raw trace, PALT-memory bytes, palette-state
bytes, and VDP1-command bytes. The engine retains the result solely as an
opaque no-draw receipt. It does not infer that PALT produced the palette, nor
any palette format, PRS3 relationship, CLUT behavior, decoder, or drawing.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus complete animated DGN source gate

The complete active Structure3 scene now includes the full `08xx`
Structure1G image-instruction route. A scene cannot be complete until every
declared image instruction resolves to an exact bounded Structure2 source
descriptor from the same canonical LEV bytes. This is no-draw source coverage
only: no GOTO execution, frame selection, timing, texture payload, palette,
VDP1, transform, decoder, or fallback has been introduced. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus owner/material DGN capture bundle

# ✅ 2026-07-15 Nexus owner/material Saturn capture target

The owner/material bundle can now be written atomically as
`FIRESTAFF_NEXUS_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_V1`. One
producer request carries the active canonical LEV fingerprint, Structure1F/
Structure1A ownership facts, exact typed Structure3 face fingerprints, and
the bounded selected Structure2 image/palette candidate windows. It requests
one original-Saturn source-read, palette, VDP1 VRAM/command, transform, and
culling observation set. It creates no pixels, decoder contract, model-entry
mapping, or draw permission. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus atomic owner/material trace consumption

`nexus_v1_engine_admit_structure1a_structure3_material_capture_trace()` now
consumes an external trace only when it names the deterministic atomic target
fingerprint, the active Structure1F/1A owner, the exact Structure3 face, and
the selected Structure2 descriptor before delegating to the existing
source-window trace admission. A trace remains opaque even after independent
original-Saturn provenance is supplied: no pixel, palette, VDP1, transform,
decoder, or draw semantics are inferred. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Mednafen owner/material trace collector

`firestaff_nexus_v1_saturn_owner_material_trace_collector` accepts only an
existing atomic target and a nonempty raw Mednafen debugger trace. It copies
the target's required identity fields, adds the raw-trace FNV-1a witness, and
writes an unauthenticated intake manifest for the atomic admission route. The
tool does not launch an emulator, generate trace bytes, or attest original
Saturn provenance. The atomic target now includes the Structure2 opaque-payload
fingerprint required by that route. Verification:
`test_nexus_v1_dgn_geometry_readiness` and direct collector compilation.
2026-08-10 - Nexus J/English cold-start witness inventory

En separat autentiserad 1 200-frames Mednafen-witness med J-BIOS 1.01 och
English/Merged-discen validerades på extern disk. `SGAREA=J`, raw VDP1/VDP2-
layout och frame-envelope passerar; jämförelse mot hashbundna MENU.BPK,
FONT256.S2D, TITLE och DGN lämnar startup→meny-ägarskapet obundet. Resultatet
är sparat som negativt proveniensbevis och öppnar ingen placeholder- eller
fallback-rendering.
# ✅ 2026-08-10 Nexus VDP1 source-buffer provenance join

En autentiserad J-BIOS 1.01/English-Merged Mednafen-körning kopplar nu en
sammanhängande 0x4000-byte SH-2 hög-RAM-skrivkedja från PC `0x00205f18` till
`TM.BIN`: destinationsintervallet `0x06027000..0x0602b000` matchar exakt
ISO-medlemmen `TM.BIN` vid medlems-offset `0x17000`. VDP1:s senare bulk-rutin
vid PC `0x060135f4` använder samma buffert; dess `R0=0x06027874` ligger
`0x874` byte in i den verifierade kedjan och motsvarar därmed `TM.BIN+0x17874`.
Beviset är byteexakt och hashbundet till den autentiska ISO:n. Det tilldelar
inte ännu `TM.BIN` till MENU.BPK/PRS3, FONT256, CLUT eller en färdig
produktionsrenderare. Verifiering: `scripts/analyze_nexus_sh2_source_trace.py`
med `--require-member TM.BIN --require-destination-range
0x06027000:0x0602b000 --require-pc 0x00205f18`.
# ✅ 2026-08-10 Nexus VDP1 source-to-VRAM copy direction

Samma-sessionens registervittne och råa VDP1/VDP2-capture bevisar nu
kopieringsriktningen för den autentiska direct-colour-bufferten: vid
`R0=0x06027874`, `R5=0x800`, VDP1-målet `0x10a00` och writer-PC
`0x060135f4` är alla `0x800` byte identiska efter den dokumenterade
16-bitars Saturn-byteordningen. Verifieringsverktyget
`scripts/analyze_nexus_vdp1_source_to_vram.py` passerar mot frame 350 i
den externa 400-frame-capturen. Detta bevisar transport och byteordning,
inte pixelkodning, CLUT/palett, kommando-typ eller produktionsrendering.

# ✅ 2026-08-14 Nexus VDP1 source-to-VRAM strömverifiering

Verifieraren för source-to-VRAM läser nu råfångster sekventiellt i stället
för att lägga hela fångsten i minnet. Den går ändå till filslut och avvisar
trunkerade eller påhängda data. En ny testkörning täcker en giltig vald
bildruta, utanför intervallet och ett otillåtet efterhäng. Den externa
400-bildrutorsfångsten vid frame 350 passerar fortsatt med källadress
`0x06027874`, mål `0x10a00` och writer-PC `0x060135f4`.
# ✅ 2026-08-10 Nexus VDP1 command-to-texture span

Frame 350:s autentiska COPR-kedja `0x05280..0x05300` består av fyra
type-2 distorted-sprite-poster och en END-post. Alla fyra draw-poster har
`colour_mode=5`, `srca=0x2140` (texture-offset `0x10a00`) och `256x4`
direct-colour-payload på 2048 byte. Det nya verktyget
`scripts/analyze_nexus_vdp1_command_source_join.py` verifierar kedjan mot
samma 400-frame-capture. Detta stänger endast command→texture-framing;
palette/CLUT-ägare, TM.BIN-transformens semantik och produktionsrendering är
fortsatt separata gates.
# ✅ 2026-08-10 Nexus VDP2 PND writer transport

Frame 350:s autentiska VDP2-write trace visar att PC `0x0601184c` skriver
PND-ord till `0x10000`; registervittnet har samtidigt `R3=0x0601121c` som
källpekare. De första 64 writer-posterna matchar byteexakt VDP2-VRAM i samma
frame efter capture-formatets native-little-endian ordning. Verktyget
`scripts/analyze_nexus_vdp2_pnd_writer.py` verifierar detta mot samma
400-frame witness. FONT256/text/palette-ägare och hostkomposition är inte
identifierade av transportbeviset och förblir blockerade.
