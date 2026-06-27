# Firestaff Gap List — Meta-Analysis (2026-06-20)

Honest, source-cited inventory of what remains between Firestaff
HEAD (`216b0b67`) and full cross-game coverage of all five
supported games (DM1, CSB, DM2, Nexus, Theron), assembled from
the dmweb-free-fr + greatstone surveys, the existing per-game
gap docs, and the stale `docs/parity/COMPLETION_MATRIX.md`.

This doc does NOT replace the per-game FINAL_GAPS files. It
sits above them as a cross-game index, prioritized for action.

Classification:
- **FIXED** — exists in main HEAD, source-locked.
- **PARTIAL** — partially implemented; some sub-items in main, others not.
- **OPEN-BOUNDED** — tractable, fits in a focused commit.
- **OPEN-LARGE** — would need a separate milestone (weeks, not days).
- **BLOCKED-DATA** — cannot close without real game data we don't have.
- **OUT-OF-SCOPE** — explicitly out of Firestaff (e.g. modding tools).

---

## A. Cross-cutting gaps

### A1. Asset format coverage

| Gap | Source | Status |
|---|---|---|
| IMG1/IMG2 RLE 16-color image decoder | ReDMCSB, sck, DMWeb DM Apple IIGS/FM Towns/PC-9801 edition pages | FIXED (`image_backend_pc34_compat.c` `IMG3_Compat_ExpandFromSource`); remaining Apple IIGS proof is PO/ProDOS media handoff for 2.0/2.1, while remaining FM Towns and PC-9801 proof is real media handoff, version/hash classification, and IMG2 asset receipts |
| IMG3/IMG4 4bpp local-palette image | dmweb Data Files | FIXED (same code path) |
| **IMG5 4bpp planar image (Amiga, SNES)** | greatstone d_items.html / greatstone d_articles_snes_multipal.html / DMWeb DM Amiga and SNES edition pages | **FIXED in v2.9.2 for raw IMG5 decode** (`firestaff_img5_decode.c`, commit `216b0b67`); remaining Amiga proof is real-media handoff across DM Amiga 2.0/2.1/2.2/3.6. Remaining SNES proof is cartridge-ROM handoff plus SCK mapfile palette-selection metadata (`PALSEL`, `TILESBYITEM`, `PALSEL_INCR`) for correct per-tile item/portrait colors; this is palette/presentation evidence and console-runtime classification, not the bounded decoder itself |
| LZW-compressed items (DM Atari ST, CSB Atari ST) | dmweb Data Files + DM Atari ST edition page | PARTIAL — decoder is contract-verified (`m11_gfx_lzw_decompress`, `dm1_lzw_round_trip` PASS 1/1 on 2026-06-21); DMWeb's DM Atari ST page now pins the source-media set for real handoff (STX originals for English 1.0/1.1/1.2, German 1.2, French 1.3, plus separate cracked/hacked media); real Atari ST asset handoff remains BLOCKED-DATA |
| **FTL container format (Amiga, X68000, MegaCD)** | greatstone d_ftl.html / DMWeb DM Amiga and X68000 edition pages | PARTIAL / OPEN-BOUNDED — `firestaff_ftl_container_unit` PASS 1/1 on 2026-06-22; parses the 20-byte common header, 12-byte hunk headers, BSS/DATA/CODE hunk discovery, BSS metadata, and verifies documented common/BSS/DATA/uncompressed-CODE checksums. `firestaff_ftl_hunk_data_zero_run_unit` PASS 11/11 on 2026-06-25; bounded HUNK_DATA area_1 zero-run decompression per greatstone d_ftl.html "Note 7" with explicit bounds checks (rejects truncated run headers, odd input length, and declared uncompressed-size mismatches against the HUNK_BSS area_1 in-memory size field). **2026-06-25 X68000 <-> FTL handoff added:** `firestaff_x68k_ftl_handoff_unit` PASS 5/5 cross-module test that builds a synthetic FTL container with a chosen `data_area1_memory_size`, parses it with `firestaff_ftl_container`, and asks the X68000 HDM classifier whether the declared area_1 fits inside the DMWeb-documented 1232 KB 2DHD disk; companion `firestaff_x68k_media_classify_unit` PASS 12/12 covers the DMWeb X68000 geometry constants and copy-protection-sentinel detection. DMWeb's DM Amiga page adds the practical media/protection boundary for official SPS IPF, unofficial IPF, protected ADF, cracked ADF, hard-disk patch, and port media. DMWeb's DM X68000 page adds the HDM/floppy boundary for a v3.0 original image missing copy-protection sectors, a cracked image, and a blank save disk. Remaining: HUNK_CODE 0x5223 decompression/checksum verification, mapfile item extraction (greatstone d_mapfile.html), real FTL corpus gates, and runtime asset loading. |
| **PAK container format (Atari ST)** | greatstone d_pak.html + DM Atari ST edition page | FIXED — `firestaff_pak_decode_unit` PASS 1/1 on 2026-06-21; parses 28-byte Atari ST executable header plus nibble-coded table/literal compression. Remaining proof is real-media extraction only: DMWeb's DM Atari ST page records `START.PAK`-adjacent STX/ST/MSA/RamDisk provenance and sector `#F7` copy-protection context |
| **Apple IIGS PO/ProDOS media import** | DMWeb DM Apple IIGS edition page | OPEN-BOUNDED — DMWeb identifies the 1989 Apple IIGS 2.0/2.1 English line, original PO images that cannot boot because the copy-protection sector is absent, Computist/ACS cracked variants, a hard-disk-patched image, an empty save disk, 1 MB RAM requirement, 2.0 ROM03 `->002C` main-dungeon failure, and the 2.1 fix. Remaining: PO/ProDOS extraction, canonical-vs-cracked-vs-hard-disk classification, hash coverage, IIGS keyboard/audio hotkey gates, ROM01/ROM03 memory-boundary evidence, and emulator/runtime launch proof. |
| **DM1 FM Towns CD-image import** | DMWeb DM FM Towns edition page | OPEN-BOUNDED / PARTIAL-EVIDENCE — DMWeb identifies the Japan v2.0 English/Japanese CD line, November 1989 release, redump BIN/CUE masters T1/T2, an ISO/CUE archive, CD audio tracks 02-20 (with tracks 04/07/20 unused and track 20 silent), and FM Towns-specific keyboard behavior. **Bounded redump-layout classifier added 2026-06-25** (`firestaff_fmtowns_cd_classify`): parses redump-style CUE sheets in memory, extracts the disc track list (FILE/TRACK/INDEX/PREGAP), distinguishes MODE1/2352 vs MODE1/2048 vs AUDIO tracks, optionally reads an ISO 9660 PVD signature at the standard data-track byte offset, and scores candidate game/version families from the documented CD-audio track tables for DM1 (17..19 audio), CSB (28..30 audio), and DM2 (5..7 audio + silent track 8). `firestaff_fmtowns_cd_classify_unit` CTest gate covers DM1/CSB/DM2 fixture layouts, single-ISO MODE1/2048 layout, malformed-CUE rejection, LF-only line endings, REM/';' comments, PREGAP/INDEX 00 markers, synthetic ISO 9660 PVD detection, and a no-match return for tiny discs. Remaining: BIN/CUE or ISO/CUE extraction, EN/JP hash classification, real `GRAPHICS.DAT`/`DUNGEON.DAT` and IMG2 receipts, CD-audio track receipts, FM Towns input gates, and emulator/runtime launch proof. |
| **DM1 PC-9801 HDM/floppy media import** | DMWeb DM PC-9801 edition page | OPEN-BOUNDED — DMWeb identifies the Japanese v2.0 PC-9801 line, 1990-02-09 release, 3.5-inch and 5.25-inch floppy editions, `2.0a` original media that cannot boot without its copy-protection sector, a cracked `2.0a`, and a newer bug-fixed non-copy-protected `2.0b` original. Remaining: HDM/floppy extraction, 2.0a-vs-2.0b and original-vs-cracked classification, real `GRAPHICS.DAT`/`DUNGEON.DAT` and IMG2 receipts, 8-bit/16-bit presentation evidence, PC-98 keypad/Alt-S input gates, and emulator/runtime launch proof. |
| **DM1 X68000 HDM/floppy media import** | DMWeb DM X68000 edition page + DMWeb copy-protection page (Sharp X68000 section) | OPEN-BOUNDED / PARTIAL-EVIDENCE — DMWeb identifies the Japanese v3.0 X68000 line, 1990-01-26 release, 31 kHz screenshot set, HDM original image that cannot boot without copy-protection sectors, cracked image, and blank save disk. **2026-06-25 X68000 media classifier added:** `firestaff_x68k_media_classify_unit` PASS 12/12 (geometry constants, empty input, too-small input, single-side / full-disk / oversize size classes, blank save-disk detection, `HPR-0007` Track 1 Side 1 Sector 9 sentinel detection, FTL-payload magic detection at offset 0, FTL handoff fits / overflow, unprotected-disk flag, NULL safety). **2026-06-25 X68000 <-> FTL container handoff added:** `firestaff_x68k_ftl_handoff_unit` PASS 5/5 cross-module test that builds a synthetic FTL container with a chosen `data_area1_memory_size`, parses it with `firestaff_ftl_container`, and asks the X68k HDM classifier whether the declared area_1 fits inside the DMWeb-documented 1232 KB 2DHD disk. **2026-06-26 real-media receipt + cross-module FTL-handoff added:** `firestaff_x68k_media_receipt_real_corpus_probe` (skip-safe CTest `firestaff_x68k_media_receipt_real_corpus`, 10/10 PASS on the local public DMFiles DM1 X68000 v3.0 HDM `~/.firestaff/data/dm1-extras/x68000-3.0-jp/DungeonMasterX68000version30Japanese.hdm`, 1,261,568 bytes exact DMWeb 2DHD geometry) verifies the DMWeb-documented receipt surface (full-disk class, no FTL magic at offset 0, Hudson Soft boot block at offset 0, MFM-fill Track 1 Side 1 Sector 9 region, no `HPR-0007` sentinel at the DMWeb sector-9 offset); `firestaff_x68k_ftl_handoff_real_corpus_probe` (skip-safe CTest `firestaff_x68k_ftl_handoff_real_corpus`, 8/8 PASS) scans the real HDM for embedded FTL resources via `FirestaffFtlContainer_Parse` and finds 2 parseable FTL containers at offsets 73,728 (0x12000) and 460,800 (0x70800), verifies the first FTL's BSS `data_area1_memory_size` (24,720 bytes) fits the on-disk HDM media class, and cross-checks the FTL-handoff helper's documented boundary cases. Known limitation surfaced by real-data receipt: the classifier's `ftl_magic_candidate_count` field scans only the first 32 KiB (per its header contract for single-resource `.FTL` payloads) but the real DMFiles X68000 HDM embeds FTL resources at offsets > 32 KiB, so the candidate-count field reads 0 even though the full-HDM scan finds 2 parseable FTL containers; the receipt explicitly surfaces both numbers as notes rather than asserting a false-positive guarantee. Remaining: HDM/floppy extraction on a real preserved master, original-vs-cracked/save-disk classification (off-axis `HPR-0007` strings embedded in `DMGame.bak` backup labels are a separate finding), expanding the FTL-magic scan window past 32 KiB or making it window-configurable, real `GRAPHICS.DAT`/`DUNGEON.DAT` and FTL-container receipts, 31 kHz presentation evidence, X68000 keypad/arrow input gates, and emulator/runtime launch proof. Copy-protection/sector gaps remain honest: public DMFiles HDMs lack the `HPR-0007` sentinel and the only operational check is documented as broken on DM and CSB alike (DMWeb Sharp X68000 section). |
| **DM1 SNES / Super Famicom ROM import** | DMWeb DM Super Famicom / Super NES edition page | DEFERRED-LARGE — DMWeb identifies four SMC cartridge versions (English NTSC 1.0, English PAL 1.0, Japanese NTSC 1.0, Japanese NTSC 1.1), Japanese/English screenshots and videos, 19 SPC music dumps, bsnes MP3 track evidence, slow-rendering behavior, and SNES-specific bugs. Remaining: cartridge-ROM import/classification, tile/palette rendering, SPC/music receipt handling, controller/input mapping, version-specific bug gates, and emulator/runtime launch proof before any native SNES claim. |
| **DM1 PC 3.4 DOS presentation/input receipts** | DMWeb DM PC edition page | OPEN-BOUNDED / EVIDENCE-ONLY — PC 3.4 EN and multilingual are already hash-verified runtime targets. DMWeb pins the surrounding DOS edition boundary: USA English, Europe English/French/German, VGA/EGA screenshots, PC-only ending animation, entrance music excerpted from FM Towns track 05, DOSBox/Smacker capture provenance, numeric-keypad / Alt-S / Ctrl-Q-Ctrl-A input, analog joystick sensitivity, Alt-keypad mouse simulation, and a Spanish fan translation that must stay outside canonical hash claims. Remaining: focused VGA/EGA, ending/music, and PC input receipts. |
| **DM1 overview version-comparison receipts** | DMWeb DM overview page | OPEN-BOUNDED / EVIDENCE-ONLY — DMWeb's overview page records cross-version behavior not owned by one edition page: two dungeon-view perspective families, PC/Atari ST/X68000 mono sound versus Amiga/Apple IIGS/SNES stereo, missing Atari ST/X68000 creature-move/War Cry/Blow Horn sounds, early-version fountain/wall-click behavior, Kid Dungeon only on Apple IIGS 2.0/2.1 and Amiga 2.2 EN, PC-only entrance music/endgame animation, Atari ST 1.0 Lock Pick placement plus v1.2 regeneration changes, X68000 larger view, PC-9801 reduced palette/three light levels, SNES redrawn graphics/music/introduction, FM Towns CD audio, and Atari ST-vs-PC dungeon deltas. Remaining: turn these into focused receipt gates as each platform's real media becomes available. |
| **HTC hint oracle text format (CSB)** | dmweb Hint Oracle Files / ReDMCSB HINTHTC.C,HINTLZW.C | PARTIAL / OPEN-BOUNDED — 2026-06-22 added data-free `csb_hint_oracle_htc_unit`: read-only HCSB.HTC parser for big-endian format word/dungeon id/header skip, location records, hint records, page compressed-length pool, content slicing, level/X/Y matching including the 255/255 any-XY rule, and bounded HTC LZW decompression with the ReDMCSB 0x90 repeat marker. **2026-06-25 real-asset scan/cache handoff:** `csb_hint_oracle_htc_real_scan` + `firestaff_csb_v1_hint_oracle_real_htc_scan_probe` (CTest `csb_v1_hint_oracle_real_htc_scan`, label `tier2;csb;hint_oracle;real_data;skip_safe`) hash-discovers a real Utility Disk HCSB.HTC in the user's data tree, materializes virtual container paths into `~/.firestaff/asset-cache/csbbin/`, owns the parsed view, and round-trips hint-name + first-page decompression against the file. **2026-06-26 runtime-adjacent UI/binding gate:** `csb_hint_oracle_ui_runtime_binding` (include + src/csb) + `firestaff_csb_v1_hint_oracle_ui_runtime_binding_probe.c` (CTest `csb_v1_hint_oracle_ui_runtime_binding`, label `tier2;csb;hint_oracle;real_data;skip_safe`) prove a decoded HCSB.HTC page or page-slice can reach a Firestaff-facing oracle/hint surface: `format_report()` writes a multi-line diagnostic/oracle report (matched MD5/label + format word 2 + dungeon id 13 + location/hint/page counts + hint 0 first-page binding smoke + level-0 (255,255) wildcard binding smoke); `format_hint(hint_index)` writes the hint name + first-page decoded text; `resolve_location(level, x, y)` resolves a (level, x, y) into the first matching hint's name + first-page decoded text. Data-free unit `csb_hint_oracle_ui_runtime_binding_unit` 6/6 PASS (synthetic in-memory fixture, no real asset). Real-asset probe 32/32 invariants PASS on the local Atari ST 2.x HCSB.HTC: `format_hint(0)` writes the parsed hint 0 "FULYA PIT" header + the first-page text starting `"This is where Lord Chaos mined the deadly Mana-absorbing Corbum.."`; `resolve_location(level=0, 255, 255)` resolves to hint 197 "CREATURES" and writes the first-page text starting `"Learn well the shapes and forms dread Chaos make"`; second scan + binding is byte-identical to the first (deterministic). The binding surface is intentionally small: it does not draw a graphical Hint Oracle overlay, it does not bind into the M11 game loop or M12 launch flow, and it does not claim parity for every Utility Disk release variant — the known-hash list in `csb_hint_oracle_htc_real_scan` still gates which HCSB.HTCs we accept; the binding surface itself is variant-agnostic. Verified end-to-end against `~/.firestaff/data/csb-atari-st-2x/HCSB.HTC` (Atari ST 2.x PP 2009-02-22 hard-disk variant, md5 `8ce69b54cf255a15e98e909bb45b9742`, 66172 bytes). The registered MD5 list is intentionally narrow (Atari ST 2.x + Amiga 3.3 FR Meynaf hard-disk utility variants only — both locally verified) so we do not falsely claim hashes for CSB Utility Disk English R1/R2/R3, French R1, or German R1/R2 releases; the list is meant to grow as those variants are classified. Skip-safe: hosts without a known HCSB.HTC exit 0 with a SKIP message. Remaining: draw the actual Hint Oracle graphical overlay, parse English/French/German Utility Disk release variants, and capture a real Hint Oracle screen using a verified HCSB.HTC before this row can move past OPEN-BOUNDED. |
| **CMP portrait image format** | sck tutorial | FIXED — `firestaff_cmp_decode_unit` + `csb_v1_cmp_import_pc34` PASS 2/2 on 2026-06-21; decoder parses the 496-byte CSB Utility Disk champion portrait format and import glue writes it into CSB V1 champion/party structures |
| **AMG sound format (CSB utility disk)** | dmweb Data Files / sck tutorial / DMWeb CSB Amiga edition page | FIXED for documented CSB Utility Disk sound-effect files — `firestaff_amg_decode_unit` parses the single-item Amiga SND2 `.AMG` shape (`TELE2`, `SWIPE`, `MAGEXPLO`, `EXPLOS1`, `DRAGON`): big-endian sample count, signed 8-bit mono PCM, 0..3 trailing bytes, plus source-cited PAL/NTSC rate helpers. DMWeb's CSB Amiga page pins the utility-disk media split by English releases 1-3, French release 1, and German releases 1-2; runtime playback/rate binding and non-SND2 `NAKED.AMG` are not claimed. |
| **MVE (Interplay, DM2 PC)** | dmweb Animations | OPEN-LARGE — DOS-stub + Interplay MVE binary |
| **QuickTime `MooV` / `.mov` (DM2 Macintosh)** | greatstone d_articles_mac.html / dmweb Animations | OPEN-LARGE — Mac QuickTime 1.x/2.x movies may be split into data + resource fork; known video codecs include Cinepak (`cvid`) and Animation (`rle`). Greatstone documents flattening Mac `MooV`/`.qtr` into Windows `.mov`, then modern MP4/H.264/AAC conversion. |
| **DMDF/DGN (Nexus Saturn)** | DMWeb Nexus pages / local verified assets | PARTIAL — DMDF parser exists (`src/nexus/`), DGN partially. DMWeb's Nexus page confirms the Saturn-only Japanese release, true 3D engine, 15-level structure, and Masaaki Shibata map material for levels 2-12; these should bound future DGN/world-loader evidence |
| **MNS (Nexus monster/spell files)** | locally verified | PARTIAL — handled in launcher/profile detection, runtime sparse |
| **S2D (Nexus font files)** | locally verified | PARTIAL — parser exists, has a bounded bitmap-to-indexed-framebuffer renderer, and now has a bounded real-on-disk SEGA SATURN SCR section-table parser + helpers. **2026-06-21 SEGA SATURN SCR parser determinism probe added:** `firestaff_nexus_v1_saturn_font_determinism_probe` (commit `b2157a62`, ctest `nexus_v1_saturn_font_determinism`) covers load/free/get_glyph + dimension inference (16x16 / 12x12 / 8xN buckets) + NULL-safety + 50-repetition determinism. **2026-06-22 render gate added:** `firestaff_nexus_v1_saturn_font_render_probe` (ctest `nexus_v1_saturn_font_render`) expands parser-exposed 1bpp glyph bytes, draws into a synthetic indexed framebuffer, and verifies glyph dimensions, expansion, clipping, transparent/background behavior, deterministic framebuffer hash, NULL/bounds handling, plus an optional local `FONT256.S2D` parser-to-render handoff when the verified 25,012-byte asset is present. **2026-06-25 SCR section-table parser added:** `firestaff_nexus_v1_saturn_font_scr_sections_probe` (ctest `nexus_v1_saturn_font_scr_sections`, 55/55 PASS) walks the 32-entry SEGA SATURN SCR section table at offset 0x20 of an SEGA SATURN SCR payload via `nexus_v1_font_load_sections()` + `nexus_v1_font_section_count()` + `nexus_v1_font_get_section()` + `nexus_v1_font_section_table_index()` + `nexus_v1_font_section_in_bounds()`. Synthesizes a small SCR fixture to lock NULL-safety, too-small-buffer rejection, invalid-magic rejection, all-reserved/zero entry skipping, single-populated-entry handling, out-of-bounds section skipping, table-overlap skipping, four-populated FONT256.S2D-shape layout (indices 0/2/4/6), in-bounds validation, original-index preservation across skips, and a fully-bound four-populated real-asset chain (entries 0x0120+0x2010 / 0x2130+0x3c90 / 0x5dc0+0x0210 / 0x5fd0+0x01e4 form a contiguous non-overlapping partition that ends at file_size). When `~/.firestaff/data/nexus/FONT256.S2D` is present the optional gate locks all four real section windows and the [0]->[1]->[2]->[3] chain end at 0x61b4 inside the verified 25,012-byte asset; entries 28..31 (non-zero but out-of-bounds tail padding the flat 1bpp loader silently consumes) are skipped by the parser without rejecting the whole table. Remaining: bind text layout/runtime drawing, decode which bytes inside each section hold the actual glyph payload, and capture an actual Nexus screen using the real font before calling this FIXED. |
| **TAI/SAL/MAP (Nexus level data)** | locally verified | PARTIAL — loaders exist; TLINK metadata and rendering sparse |
| **BPX/BPK (Nexus compressed archives)** | locally verified | PARTIAL — `nexus_v1_bpk_archive` is CTest-gated and locks the observed Nexus `MENU.BPK` BPPK/BMPD directory shape: `BPPK` outer size, `BMPD` section header, 163 big-endian candidate offsets, 162 `PRS3` payload markers, one raw payload, monotonic bounds checks, and local MENU.BPK receipt checks when the file is present. `nexus_v1_bpx_bpk_archive_boundary` also recognizes the verified `nexus/MENU.BPK` size/hash marker, keeps unknown real `.BPK/.BPX` payloads unsupported, and locks a bounded synthetic `BPX0` table contract for entry bounds, lookup, stored-entry extraction, and unsupported-compression reporting. Remaining: inspect real MENU.BPK bytes/disassembly, identify PRS3/BPK/BPX stream semantics, and hand packed payloads into renderable Nexus menu graphics. |
| **Theron's Quest Track 02 BIN/ISO** | locally verified | FIXED for boot/bank-anchor coverage, PARTIAL for broader semantic Track 02 decoding — JP canonical ISO, JP extras BIN, and US extras BIN launch-tested by `tier1_strict_boot_probe` (Theron rows PASS, 2026-06-21) plus `theron_v1_launcher_scan_reuse` and `theron_v1_track02_bank`. 2026-06-22: `theron_v1_runtime_screenshot_readiness` records real Firestaff runtime-probe/BMP hash receipts when those Track 02 paths are present, without promoting public screenshots or claiming full dungeon-bank parity. 2026-06-25: `theron_v1_track02_decode_descriptor_table()` (CTest `theron_v1_track02_descriptor_table` PASS) reads the 9-word LE stride table at every JP/US anchor (`US ISO 0x1584`, `US raw 0x70be06/0x70e2c6/0x710904`, `JP raw 0x70b4d6/0x70d996/0x70ffd4`) with shape-only validation (entries 0x0020..0x2020, stride 0x0400, half-open range `[0x0020, 0x2420)`). The decoder is shape-driven only: it does NOT claim per-entry semantic type, dungeon-level binding, runtime loader handoff, or level-descriptor semantics — per-entry semantic binding and broader decoder coverage remain open. |

### A2. Mapfile system

| Gap | Source | Status |
|---|---|---|
| YAML/TOML mapfile parser for arbitrary item description | greatstone d_mapfile.html | FIXED — 2026-06-22: added bounded shared `firestaff_mapfile` parser/API plus data-free `firestaff_mapfile_unit` CTest for Greatstone mapfile 2.5-style header properties, six-field item rows (`number,type,attributes,description,long_description,comment`), raw item descriptions, CRLF/whitespace/quoted header values, `SIZE` attribute extraction, line-numbered errors, and capacity/malformed-row rejection. Added `firestaff_sck_mapfile` parser/CTest for Greatstone/SCK `type name offset size` rows, including comments, blank/CRLF lines, decimal/hex offsets, name lookup, u32 rejection, trailing-token rejection, and target-file bounds validation. The mapfile-to-asset-loader handoff remains tracked separately below. |
| Mapfile-to-Firestaff-asset-loader bridge | greatstone d_mapfile.html | PARTIAL — 2026-06-22: `tools/fetch_greatstone_sck_mapfiles.sh` now reproduces the bounded curl/unzip handoff from Greatstone's SCK release into a local `db/map/*.map` corpus, and `firestaff_sck_mapfile` now parses real SCK 2.x comma rows (`item,type,attrs,description,long,comment`), captures `FORMAT`/`ENDIAN`, preserves ordinal/type/label metadata, extracts `SIZE=` attributes, and builds bounds-checked asset slices for offset/size-backed rows. **2026-06-25 live selection bridge added**: bounded `firestaff_sck_asset_bridge` parses the SCK `_mapping.xml` index into `(md5, path, file, games[])` rows, performs MD5+filename lookup, and selects a single concrete asset slice from a referenced `.map` by item number or description substring with an `IMG`/`RAW`/etc. type prefix filter; CTest `firestaff_sck_asset_bridge` PASS 1/1 covers synthetic `_mapping.xml` with one self-closing row, one nested `<game>` row, and a SCK 2.x `dm_atari_demo`-shape mapfile carrying IMG1+`SIZE=` slices (select `012288` `IMG1` @12288 size 2560; rejects unsized IMG3 rows as `NOT_SIZED`; rejects oversized slices; rejects malformed mapfiles). Real-corpus probe `firestaff_sck_asset_bridge_real_corpus_probe` PASS when `$FIRESTAFF_GREATSTONE_SCK_DIR` (or `~/.cache/firestaff/greatstone-sck-mapfiles/db/map`) carries `_mapping.xml` and `dm_atari_demo.map`, parsing 231 rows and selecting `012288` `IMG1` `Dungeon Graphics` @12288 size 2560. Remaining: per-asset-type decoder wiring (IMG1/IMG3/IMG5 still go through the existing image backend; RAW/FTL/PAL/SND items remain selector-visible but no Firestaff-side decoder is wired), live menu/launch profile integration, and broader coverage beyond `SIZE=` slices. |

### A3. Engine behaviour parity

| Gap | Source | Status |
|---|---|---|
| CSB-specific hidden-code items 558-562 (Amiga executable blobs) | greatstone d_items.html | FIXED — `csb_hidden_code_skip_table_unit` + `csb_v1_graphics_hidden_item_skip_pc34` PASS 2/2 on 2026-06-21; remaining CSB real-asset rendering work is tracked under CSB runtime/graphics rows |
| Atari ST hidden code skip | greatstone d_items.html | FIXED — same skip-table/loader gates cover Atari ST items 21/538/548 and 558-562 |
| Champion panel portrait loading from CSB utility disk | CSB docs | FIXED for current Utility Disk portrait handoff — CMP decode/import, Utility Disk flow, imported-party inventory handoff, runtime load/attribute gates, and data-free runtime portrait render-source selection are now CTest-backed. `csb_v1_portrait_render_handoff_pc34_compat` proves synthetic `.CMP` → `CSB_V1_PartyState` → runtime party snapshot → exact `CSB_V1_Champion.Portrait` render source with ReDMCSB `PANEL.C:F0354` / `CHAMDRAW.C:F0292` source lock. Real CSB viewport/HUD captures and pixel parity remain tracked under CSB graphics/runtime rows. |
| Savegame format (DM1, CSB) | ReDMCSB + dmweb | FIXED for DM1 (`dm1_v1_save_load.c`), PARTIAL for CSB |
| Savegame format (DM2) | skproject source | PARTIAL |
| Savegame format (Nexus .sav) | locally verified | PARTIAL |
| Savegame format (Theron .SRM) | locally verified | PARTIAL — 2026-06-25: synthetic `.tqsv` slot round-trip, rejection, verification, and cross-slot export/import remain CTest-gated; new `theron_v1_srm_classifier` adds a bounded 5-slot Save Disk classifier (`theron_v1_srm_classifier_probe` + `test_theron_v1_srm_classifier_pc34`) that recognizes the gzipped-deflate save body dmweb references, reports ABSENT on the current host (no real `.srm` staged), and accepts a real file when one is placed under `$HOME/.firestaff/data/theron/save/` or `$FIRESTAFF_THERON_SRM_DIR`. Real `.srm` payload decoding and cross-slot import to `Theron_DungeonProgression`/champion blocks remain out of scope. |
| Custom dungeon import (DM1 dungeon.dat, CSBWin dmsave/csbgame) | dmweb Custom Dungeons page | PARTIAL / BLOCKED-DATA — 2026-06-22: M12 has a disk-backed `custom_dungeon_m12_real_path` CTest for `data/custom/<name>/DUNGEON.DAT` discovery, mixed-case community filenames, header validation, sorted listing, invalid-entry rejection, and valid-entry selection. `custom_dungeon_import` also covers the existing M12 launcher scanner and DM1 V1 engine scanner with synthetic `DUNGEON.DAT` fixtures: valid header/map-count parsing, alphabetical launcher list, case-insensitive `dungeon.dat`/`graphics.dat` discovery, compressed-signature rejection, too-small rejection, valid-entry selection, and invalid-entry refusal. **2026-06-27 CSBWin `dmsave`/`csbgame` loader-boundary contract gate landed:** `csb_v1_csbwin_save_loader_boundary_pc34_compat` (14 documented shapes: 3 accept + 11 reject; per-shape `csb_v1_import_csb_save_buffer()` loader-boundary verdict; CTest `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` PASS 79/79 + skip-safe real-asset probe `csb_v1_csbwin_save_loader_boundary` PASS). Remaining blocker: real community/custom dungeon corpus handoff, the CSBWin 512-byte XOR-pad obfuscation-key decoder, the DM1→CSB raw-save conversion path, and the end-to-end CSBWin save importer wiring. |

### A4. i18n / l10n (post v2.9.1)

| Gap | Source | Status |
|---|---|---|
| 19-launcher-locale cycle | po/validate_po_layout.sh | FIXED in v2.9.2 |
| DM1 native translations (17 non-Swedish locales) | po/dm1_translations_complete.py | PARTIAL — validator now reports 492-540/547 native entries across the 17 non-Swedish catalogs; remaining gaps are fallback/blank entries plus native QA |
| CSB native translations | po/csb_translations | PARTIAL — `sv/fr/de/ja/zh` are 33/33 native; the other shipped non-English catalogs are currently fallback-only |
| DM2 native translations | po/dm2_translations | OUT-OF-SCOPE — DM2 slice not implemented |
| Nexus native translations | po/nexus_translations | PARTIAL — validator reports 28-30/30 native entries across every shipped non-English catalog; remaining work is native QA/runtime rendering, not empty scaffolding |
| Theron native translations | po/theron_translations | PARTIAL — `sv` now has 38/38 native runtime entries; `fr/de/ja/zh` have 35-37/38 native entries; 13 shipped catalogs remain fallback-only and native QA/runtime rendering is still open |
| **Native-vs-fallback separation in validator** | po/validate_po_layout.sh | FIXED — `validate_po_layout.sh` now reports `nonblank` coverage separately from `native` coverage and marks fallback-only catalogs as `FALL`; `bash -n po/validate_po_layout.sh && bash po/validate_po_layout.sh` PASS on 2026-06-21 |
| Native QA on terminology / runtime rendering | po/ | OPEN-LARGE — needs native speakers |

### A5. Tooling

| Gap | Source | Status |
|---|---|---|
| compare_to_greatstone.py SHA256 probe | tools/asset-validate/ | FIXED in v2.9.2 (commit `0d89adc6`) |
| PLATFORM_MATRIX.md version support map | docs/PLATFORM_MATRIX.md | FIXED in v2.9.2 (commit `32dcf76c`) |
| DMWEB_REFERENCE.md consolidated reference | docs/DMWEB_REFERENCE.md | FIXED in v2.9.2 (commit `b54b52c4`) + EXTENDED 2026-06-20 — now mirrors dmweb /community/documentation/ (43 pages) at `reference/dmweb-community-docs/`. 19 → 62 pages surveyed, see Section I. |
| **Reproducible game-archive extraction from `~/Downloads/`** | new | DONE 2026-06-20 (commit `4b097f54`) — `reference/extract-game-archives.sh` extracts 73 archives → 71 `<game>-extras/<version>/` directories without touching canonical staging. |
| **`--scan-data` smoke reports real READY-path:er** | existing | FIXED for the current gate — `asset_validate_coverage_by_game` is wired in CTest and PASS; `tier1_strict_boot_probe` is wired in CTest and PASS for all present in-scope launch paths on 2026-06-21, including CSB canonical and CSB Amiga 3.3 Meynaf FR via the `CSB READY` marker. Nexus virtual-ISO launch remains tracked as a Tier 4 runtime/launcher gap, not a Tier 1 path-discovery gap. **2026-06-25 data-dir picker / platform-matrix data-free regression added:** `firestaff_data_dir_picker_platform_matrix_probe` (ctest `data_dir_picker_platform_matrix`) builds a synthetic data root that mirrors the `tier1_strict_boot_probe` sub-layouts (`dm1/`, `dm1-multilingual/`, `dm1-extras/legacy-dos/`, `csb/`, `csb-extras/legacy-amiga-dms/`, `dm2/`, `dm2-extras/{dos-en,dos-fr,pc-fr,pc-de}/`, `theron/Theron's Quest (Japan) (Track 02).iso`, `theron-extras/{japan,usa}/track02.bin`) and locks: empty-root no-availability + single search-root + zero duplicate-root skips; DM1 (via `pc34-multi`), CSB (via `pc34-en`), DM2 (via `pc-en`), and Theron (via `pce-jp`) availability, while Nexus stays unavailable without a DM.BIN; per-game first-matched version determinism; `asset_find_by_md5` recursive discovery for every nested leaf; negative-discovery rejection of an unknown hash; re-scan clearing availability when the new folder has no match; and re-scan idempotence for the same root. Locks the M12 layer under `tier1_strict_boot_probe` so the data-dir picker stays honest across the platform-matrix rows. |
| **Real-data regression tests (greatstone db_data)** | greatstone sck tool | PARTIAL / OPEN-BOUNDED — `tools/verify_greatstone_db_data_paths.py` is now CTest-gated as `greatstone_db_data_paths_probe` and the OFFLINE fixture at `tests/fixtures/greatstone_db_data_paths/index.json` carries only derived metadata (status code, content-type/length hints, `<title>` text + SHA-256, link count). 10 documented-current db_data paths (DM `dm_pc_34/graphics.dat/graphics.dat.html`, DM `dm_snes_11_jp_ntsc/smc/smc.html`, CSB `csb_atari_21_en_stx/graphics.dat/graphics.dat.html`, CSB `csb_amiga_udr2_en/hcsb.htc/hcsb.htc.html`, DM2 `dm2_pc10_en/graphics.dat/graphics.dat.html`, DM2 `dm2_amiga_10_enfrge/lang.ftl/lang.ftl.html`, DM2 `dm2_segacd_10_en/stry.dat/stry.dat.html`, plus the three `g_dm/g_csb/g_dm2` index pages) and 3 obsolete `c_dm_*` / `c_csb_*` / guessed-DM2 404-regression paths are all locked at the gap-list level. `--online --write` (opt-in via env `FIRESTAFF_GREATSTONE_PROBE=1`) refreshes the evidence + fixture in one step, drops the body after extracting metadata, and never persists copyrighted bytes (`tools/asset-validate/no_game_data_in_git.py` PASS). Remaining: keep `compare_to_greatstone.py` aligned with `VERIFIED_HASHES.md` plus local `*-extras/` alternatives and broaden the curated path set as more reachable db_data examples are pinned. |
| **Lefthook in PATH for CI** | build/CI hygiene | FIXED — `.github/workflows/verify.yml` installs Go, installs `lefthook`, exports `$(go env GOPATH)/bin`, and runs `lefthook run ci`; local dev machines may still no-op gracefully when Lefthook is absent |

### A6. Build / CI

| Gap | Source | Status |
|---|---|---|
| Strict warnings `-Wall -Wextra -Werror` | CI verify.yml | FIXED |
| Phase A probe (headless invariants) | CI | FIXED (23/23) |
| Audio probe | CI | FIXED |
| Cross-platform determinism | CI | FIXED |
| `_G2157_` undefined symbol in firestaff_m10 | CMakeLists.txt | FIXED — `image_backend_pc34_compat_globals.c` provides the globals, see commit 3588798f |
| **MD5 vs SHA256 inconsistency** | `asset_find_by_hash.c` vs `VERIFIED_HASHES.md` | FIXED — see docs/MD5_SHA256_HARMONIZATION.md and `tools/asset-validate/compare_md5_to_sha256.py` |

---

## B. DM1 gaps (specific to game 1)

Source: `docs/FINAL_GAPS.md` (188 lines), `docs/DM1_V1_BUG_AUDIT.md`,
dmweb Game Page for Dungeon Master, ReDMCSB decompilation.

### B1. Capture-gap pairs (from prior TODO audit)

| Gap | Status |
|---|---|
| Original DOSBox/FIRES keyboard buffer transcript for I34E route keys | PARTIAL — 2026-06-21: pass513 transcript scaffold at `verification-screens/pass1052-dm1-original-route-24h-turncycle/pass513_i34e_route_key_transcript_scaffold.json` source-locks route tokens (`kp5`/`kp4`/`kp6`/`kp8`/`kp2`) against ReDMCSB F0361/G0459 (movement table), F0365 (turn dispatch), F0128 (tuple draw), F0097 (viewport present). **Promoted transcript (2026-06-21):** `tools/pass513_i34e_route_key_transcript_field_completer.py` walks the canonical pass1052 route and fills all 23 missingOriginalRuntimeFields deterministically from ReDMCSB source contracts. Promoted artifact at `verification-screens/pass513-dm1-v1-promoted-transcript/promoted_transcript.json` is accepted by `verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py` with status `PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_PROMOTABLE` (was `SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS`). **Pass1072 provenance guard (2026-06-21):** `pass1072_dm1_v1_keyboard_buffer_live_provenance_readiness` now fingerprints the deterministic transcript, checks its capture hashes and explicit non-live boundary, audits the ReDMCSB M528/F0361/F0380/F0128/F0097 source anchors, and records status `BLOCKED_ORIGINAL_I34E_KEYBOARD_BUFFER_LIVE_DEBUGGER_OBSERVATION_MISSING` because it finds 0 debugger-observed rows and this host is missing `dosbox-debug`. **Honest boundary on real debugger observation (2026-06-21 19:32):** the source-locked fill above is NOT a live I34E debugger observation. A real observation requires `dosbox-debug` (a custom-built DM1 + DEBUG.EXE build with break-point support) plus `Xvfb` + `xdotool`; none of which is installed in this session host. The host has `dosbox-staging` only. `tools/run_dosbox_debug_pty.py` would run the live debugger if those tools were installed, but at the time of this session the prerequisite tools check returns `missing tools: dosbox-debug, Xvfb, xdotool`. The deterministic fill remains the best the current host can produce; a real I34E debugger session is still required to validate memory-observed runtime values such as G0433/G0434 actual pointer addresses, G2153 live count under scheduler jitter, and M527 keyboard buffer byte observations. pass623/pass625/pass626/pass1072 still CTest-lock the Firestaff-side/readiness bridge; live-runner handoff gates `dm1_v1_original_capture_route_handoff` + `dm1_v1_original_capture_live_row_gate` + `dm1_v1_capture_runbook_consistency` keep the Firestaff-side evidence reproducible. |
| Paired original viewport screenshot | PARTIAL — 2026-06-21: pass1052 captured 4 viewport crops (`01_party_hud`, `02_left_1_wall`, `03_left_2_view`, `04_left_3_view`) via `scripts/dosbox_dm1_original_viewport_reference_capture.sh` (DOSBox route: `wait:9000 enter enter wait:1800 click:276,140 wait:2200 one wait:2500 kp5 wait:1200 shot:party_hud kp4 kp4 kp4 shot:left_1_wall_/_left_2_view_/_left_3_view`). Pass1054 nearest-neighbour pairing over Firestaff 24-pose Hall capture: 4 rows, MAE range 0..12.9, exact_pixel_match count = 1 (wall row). Scout rows useful for route work but not same-state parity claims. Firestaff-vs-original diff PNGs at `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/pairs/`. Pass1056 pairing-gate manifest at `parity-evidence/verification/pass1056_dm1_v1_pass1052_firestaff_pairing_gate/manifest.json`: status=PASS, exact_match_count=1, row_count=4, all_artifacts_present=true, all_pair_hashes_match=true. Additional pass376 measurement-only overlays now cover 6 original-vs-Firestaff viewport rows under `parity-evidence/overlays/pass376_firestaff_pairing/`; deltas remain 72.7055%..93.4874%, so this is visual-debug evidence only, not promoted parity. |
| Paired original wall screenshot | PARTIAL — 2026-06-21: 1 wall exact pixel match confirmed (pass1054 row `02_02_left_1_wall_original_viewport_224x136_vs_hall_1_4_dirE_viewport_224x136`: MAE=0.0, changed_pixels=0, sha256=`8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c`, Firestaff DUNVIEW.C wall_clip gate matches ReDMCSB DUNVIEW.C:436-440 + 3048-3076 + 3394-3470 + 8446-8542 source-locked via pass512 audit). Source-only center-wall pixel coverage now CTest-locks D3C/D2C/D1C `G2107_WallSet` selection, row-local parity flip, blit clipping, and opaque `CM1_COLOR_NO_TRANSPARENCY` copy semantics via `dm1_v1_center_wall_parity_opaque_pixel_probe`. Source-only side-wall coverage now CTest-locks clipped D3/D2/D1 side-wall blit bounds and D0L/D0R 16-byte edge wall spans via `dm1_v1_side_wall_pixel_clip_probe` and `dm1_v1_d0_side_wall_edge_pixel_probe`; negative lane coverage also locks D0C's no-wall-blit path and the absence of D1L2/D1R2 draw/write lanes via `dm1_v1_d0c_wall_absence_pixel_slice_probe` and `dm1_v1_d1l2_d1r2_absence_pixel_slice_probe`. Additional source-only viewport contract CTests now cover D0C F0111 door-panel behavior, D0C F0111 partly-open door, D0C stairs/pit dispatch, D1C F0107 wall-ornament routing, D1C F0111 door behavior, D1C F0108 floor-ornament occlusion, D1C F0111 partly-open door routing, D1C F0115 door-frame ordering, D1C stairs/pit dispatch, and D1C wall ownership. Remaining work (out of 24h scope): broaden to multi-state wall route (door/wall/fakewall/fountain/wall-ornament 35). |
| Paired original collision transcript | PARTIAL — 2026-06-21: pass1055 closed-door stasis capture: 3 byte-identical frames (`door_before`, `after_viewport_click`, `after_kp5`) all sha256=`a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6` (raw 320x200 + 224x136 viewport crops). Status PASS1055_ORIGINAL_CLOSED_DOOR_STASIS_CAPTURED, now CTest-gated by `pass1055_dm1_v1_original_closed_door_collision_capture`, which also runs the matching `firestaff_dm1_v1_pass1055_closed_door_pair_probe`: `MOVE_BLOCKED_DOOR`, no movement, final tuple `(0,6,9,3)`. Firestaff-side broadening now adds `firestaff_dm1_v1_extended_collision_pair_probe`: same pass1055 target route plus repeated closed-door stasis and wall/door/fakewall/pit/teleporter element substitution through the V1 movement pipeline. Source-only special-square coverage also gates the D1C door-button press/unpress frame transition plus open-pit fall damage and landing-sensor emission through synthetic M10 fixtures. Source-only viewport pixel-owner coverage now CTest-locks D2C open-pit -> F0108 floor-ornament -> F0115 thing-layer ordering and D2-before-D1 stair/pit shared-lane ownership via `dm1_v1_stair_pit_occlusion_pixel_gate`. Remaining work (out of 24h scope): original pixel-pair this view, add party-occupied/diagonal-approach blocked original rows, and promote a broader wall/door/fakewall transcript. |
| Paired original creature-chain screenshot | BLOCKED-DATA — 2026-06-21: requires original DM1 PC 3.4 level-1 route to a visible creature chain. Per pass1058 keypad atlas, the corrected level-1 route reaches a distinct `stair_entry` state and then the first selected target door using the `kp8`/`kp4`/`kp2` corrected DOSBox keypad mapping, but the target remains a closed/inert door: Enter, Space, two door clicks, and a forward key all leave the raw frame hash unchanged. `dm1_v1_creature_chain_original_capture_gate` now CTest-locks the future required rows (`creature_chain_d2c_trolin_front`, `creature_chain_d1c_trolin_front`), canonical hashes, source anchors, and non-claim boundary while preserving BLOCKED_ON_REFERENCE status. 2026-06-21 capture-harness follow-up: `dosbox_capture_session.py --post-dungeon-route` now lets operators append a bounded keypad route after the first live movement proof and writes an operator-local `dosbox_capture.post_dungeon_route.json` receipt with hashes, classifier states, route keys, and local frame paths. 2026-06-21 route-token recovery: pass1058 now preserves the exact corrected start-pose route and door-probe actions as redacted text in its CTest manifest, without proprietary frames; because the post-dungeon hook starts after pass1073's first movement proof, future replay must align the starting pose rather than paste the old route blindly. Tracking pass remains BLOCKED-DATA until a reviewed run produces creature rows and paired Firestaff evidence; downstream pass86 classifier + semantic-promotion gates are ready and waiting. |
| Paired original champion-panel screenshot | PARTIAL — 2026-06-21: pass1053 promoted existing pass455 `candidate_select` (`e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e`) + terminal/HUD after C160 (`7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f`) from `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509` (`probe-initial-south-corrected`). Status PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_EVIDENCE_TRACKED, now CTest-gated by `pass1053_dm1_v1_original_champion_candidate_panel_gate`: 3 original 320x200 frames, 12 crops, ReDMCSB `COMMAND.C`/`MOVESENS.C`/`REVIVE.C` anchors, and Firestaff-side reference captures are reproducible. pass1071 adds a machine-checkable readiness gate, `pass1071_dm1_v1_champion_panel_pairing_readiness`, that fingerprints the pass1053 package and Firestaff HUD PPMs while preserving status `BLOCKED_ORIGINAL_FOUR_CHAMPION_HUD_AND_SINGLE_STATUS_PANEL_CAPTURE_MISSING`. Original captures at `verification-screens/pass1053-dm1-original-champion-candidate-panel/{start_before_portrait_click.png, candidate_select_after_click_111_82.png, resurrect_terminal_hud_after_click_130_115.png}` + crops under `crops/`. Firestaff-side V1 captures exist at `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm` + `party_hud_statusbox_gfx_vga.ppm`. Remaining work (out of 24h scope): full four-champion party HUD + single-champion status-panel original capture with deterministic route + pass86 pass86_pass=true promotion. |

**2026-06-21 live-runner note:** `docs/parity/tools/dosbox_capture_session.py --live` now reaches DM1 PC 3.4 `dungeon_gameplay` again on this macOS host by launching the DOSBox Staging app binary directly, passing `DM.EXE` as the executable PATH argument, waiting for startup, and using `Return` on the entrance wall. The local run produced `01_ingame_start.png`, `02_ingame_step_forward.png`, and a movement receipt with a viewport-hash change after `Keypad-5`. pass1073 now CTest-locks a redacted receipt for that run: two `dungeon_gameplay` frame hashes, DOSBox conf pins, C070 no-change, Keypad-5 viewport change, and explicit non-claims. The live runner also accepts `--post-dungeon-route Keypad-5:label,...` for follow-up operator-local route receipts after the first movement proof. This is capture-harness and receipt recovery only: the proprietary frames remain operator-local, and none of the B1 rows move to FIXED until the frames are promoted into tracked evidence and paired against Firestaff.

### B2. Per-domain DM1 gaps

| Gap | Doc reference | Status |
|---|---|---|
| DM1 Amiga media/input/protection coverage | DMWeb DM Amiga edition page | PARTIAL / BLOCKED-DATA — DMWeb now pins the Amiga release/media boundary: 2.0 EN/FR/DE, 2.1 EN, 2.2 EN/DE, 3.6 EN/FR/DE, demo media, official SPS IPF 3.6, unofficial IPFs, protected original ADFs, cracked ADFs, hard-disk patches, and Meynaf's Atari ST-to-Amiga port. Future work needs original-vs-crack-vs-patch hash classification, real `GRAPHICS.DAT`/`DUNGEON.DAT` extraction, Amiga 2.x vs 3.6 keyboard gates, Kickstart/RAM assumptions, and copy-protection provenance without committing patched executables. |
| Champion stats F0308, F0202, F0229 | FINAL_GAPS §Group 1 | FIXED |
| Magic-map C80-83 | FINAL_GAPS | FIXED |
| Teleporter rotation | FINAL_GAPS | FIXED |
| Kinetic pass-through F0816 | FINAL_GAPS | FIXED |
| Fire/spell shield subtraction F0321 | FINAL_GAPS | FIXED |
| C6 wisdom factor | FINAL_GAPS | FIXED |
| Trolin anti-mage palette F0823 | FINAL_GAPS | FIXED |
| DM_SAVE_HEADER noise/keys/checksums | FINAL_GAPS | FIXED |
| Hall of Champions 4-mirror + wall-mirror zones | FINAL_GAPS | FIXED -- 2026-06-22 Hall champion-mirror placement regression: C127 portraits now route through the fixed C346 D1C champion-mirror frame, not the map's last wall ornament or the generic wall-ornament pass; `firestaff_dm1_v1_champion_mirror_capture_probe` now requires both portrait pixels and mirror-frame pixels for reported floating cases plus wrong-wall cases. 2026-06-22 runtime probe matrix covers visibility, walkpath routing, z-order, re-blt, actual poses, resurrect round-trip, and candidate-panel suppression; fixture-mismatch SKIPs on known reference-data positions remain tracked as SKIPs, not regressions. 2026-06-23 follow-up probes on `main` cover ordinal 0/DAROOU, HALK/ordinal 1 front-north-entry, ordinal-1 south_return, and ordinal-04 south_return (`firestaff_dm1_v1_hall_of_champions_champion_portrait_04_south_return_portrait_rect_position_probe`) without claiming DOS pixel parity. |
| M12 launcher extras (3/5 wired) | FINAL_GAPS | FIXED (3 of 5) |
| **M12 launcher extras (spell reference + map viewer)** | FINAL_GAPS | OUT-OF-SCOPE -- pass1060 audit: `docs/FINAL_GAPS.md` marks both as lacking a data source; `src/ui/menu_startup_m12.c` keeps both disabled while bestiary/items/screenshots/changelog are wired |
| Chest runtime detail coverage | TODO.md | FIXED -- 2026-06-21 chest matrix PASS 38/38 via `ctest --test-dir build -R 'chest|Chest' --output-on-failure`; covers open/close, action-hand, full-hand, stack split/merge, scroll-wheel pickup, non-leader, party-rotate, candidate-panel, cross-champion, drop-to-floor, empty-reopen, and pass797/pass652/pass799/pass803/pass804/pass810/pass811/pass812/pass822/pass836/pass849/pass850 verifier gates. Remaining visual/pixel polish is tracked under B1 capture pairs and B3 V2 material gates, not this runtime-detail row. |
| Inventory route parity for all item types | `docs/dm1_gap_inventory_items.md` | FIXED -- pass1070 audit verifies equip slots, backpack/chest routes, consumables, mouth/eye routes, panel-slot routing, object interaction, and M11 inventory runtime/pixel gates together |
| Champion portrait sensor parity | `docs/dm1_gap_portrait_sensor.md` | FIXED -- pass1059 audit: C127 `sensorData` is a 0..23 C026 atlas ordinal, not 0..7; M11 runtime already clamps to `mirrorCatalog.count`; resurrection test keeps index 23 valid |
| Hall of Champions C127 portrait front-wall ownership | user report 2026-06-22 | FIXED -- `m11_front_cell_mirror_ordinal()` now applies the ReDMCSB PC34/I34E front-side filter (`DUNGEON.C:2573`, `DUNGEON.C:2608-2612`, `DEFS.H:2552`) before accepting C127 `sensorData`, so wrong-side mirror sensors no longer draw/click as front portraits. Runtime probes now cover wrong-wall negatives for old Leif/Mophus poses plus source-positive Leif/Mophus/Halk/Sonja/Zed/Wuuf routes. 2026-06-22 south_return portrait_rect_position slice row: ordinal 16 (CHANI / "SAYYADINA SIHAYA" on PC 3.4 EN) on the south_return route locked via `firestaff_dm1_v1_champion_mirror_ordinal_16_south_return_portrait_rect_position_runtime_probe` (ctest 11/11 PASS, C026 col 0 row 2 source rect, D1C portrait rect painted at (96, 35, 32, 29) with warm_count=322, no floating on side walls, per-ordinal strict dominance 475 vs 86 second-best). 2026-06-23: ordinal-9 west_negative sibling probe `firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative_portrait_rect_position_runtime_probe` (18/18 invariants PASS on local PC 3.4 DUNGEON.DAT) extends wrong-wall corridor-band coverage to ordinal 9 (ZED / "DUKE OF BANVILLE"); cross-checked against ordinal-10 GANDO at `(1,5) DIR_NORTH` (100% pixel match, 4% ordinal-9 drift) so the row=1 portrait band is alive at the source-locked rect without painting the slice target. Companion to the existing ordinal 0/1/2/3/4/6/7/8/9/10 south_return portrait_rect_position probes. |
| Per-champion C01-C24 stats | `docs/dm1_gap_c01_c24_stats.md` | FIXED -- pass1063 audit/test: Hall recruitment uses decoded mirror records via `F0606`/`F0652`/`F0673`, not flat `m11_stats_add_champion()` defaults or G0243 creature data |
| C25-C26 Lord Order/Grey Lord projectile fallback | `docs/dm1_gap_c25_c26.md` | FIXED -- pass1064 audit/test: ReDMCSB BUG0_13 leaves the original projectile thing undefined for custom dungeons; Firestaff names both C25/C26 cases and uses a deterministic Fireball fallback |
| Touch zones for inventory | `docs/dm1_touch_inventory.md` | FIXED -- pass1065 verifies C507..C536 inventory/chest/panel coordinates through the source-locked touch matrix and mouse-command queue |
| Touch zones for champion panel | `docs/dm1_touch_champion.md` | FIXED -- pass1065 verifies C151..C218 status/name/hand zones through champion status-box and touch queue gates |
| Touch zones for menu | `docs/dm1_touch_menu.md` | FIXED -- pass1065 verifies movement/action/spell/menu touch zones through source-ordered mouse tables and V1 command dispatch gates |
| AI pathfinding | `docs/ai_pathfinding.md` | FIXED -- pass1066 adds a data-free CTest for the ReDMCSB F0798/F0799 one-step greedy cascade: primary, RNG-gated secondary, door blocking, opposite fallback, and blocked idle |
| Champion AI/autoplay | `docs/ai_champion.md` | OUT-OF-SCOPE -- DM1 V1 has no autonomous champion AI; champion movement/actions remain player-command driven |
| Creature grouping/coordination | `docs/ai_grouping.md` | FIXED -- 2026-06-21 grouping matrix PASS 8/8: ordered attack cells, group move/removal, C006 unused group slot, creature AI behavior, pathfinding, perception/target, stairs/group timing, and pass803 ordered-cells verifier. Broader real-runtime creature-chain screenshot evidence remains B1 capture-data work, not an AI grouping implementation gap. |
| Creature AI aggro/reaction/spell behavior | `docs/ai_creature.md`, `docs/ai_aggro.md` | FIXED -- pass1067 gates F0790-F0796 perception, visibility/smell, aggro transition, determinism, and target selection; pass1069 adds reaction-event creation, projectile-hit search turn, danger movement/stop-attacking, and existing caster projectile table/payload gates |

### B3. DM1 V2

| Gap | Status |
|---|---|
| V2.0/V2.1/V2.2 runtime pipeline | FIXED |
| V2.2 modern asset pipeline (gpt-image-2) | FIXED — 19 PBR hero variants, 29 asset pack entries |
| **Real in-place V2.2 drawing via m11_draw_dm1_\* passes** | PARTIAL / OPEN-LARGE — live M11 now initializes the optional DM1 V2.2 in-place cache and prefers `m11_v22_inplace_render_pass()` before falling back to the placeholder overlay; `dm1_v22_inplace_render_probe` proves synthetic-cache load, wall/floor/pit/stairs material routing, exact per-cell palette centers, 4-direction 4x8 material-backed sweeps, and no wrong-wall fallback for teleporter fields. Remaining: finished real-art assets, per-cell `m11_draw_dm1_*` material swaps, and material/pixel diffs. |
| V2 modern UI overlay polish (inventory/champion/rune/action) | PARTIAL / OPEN-LARGE — 2026-06-21: V2 HUD interaction now mirrors the source-owned V1 champion/action/rune routes, including spell parent/caster/rune/cast/recant (`C100`, `C101`..`C109`) via `dm1_v2_hud_interaction_pc34`; CTest/source-lock group PASS 4/4: touch matrix, HUD interaction, HUD interaction source lock, and UI overlay affordance routes. The HUD overlay itself now has data-free presentation state and pixel gates for champion HP/stamina/mana summaries, leader/spell-ready cues, action active/flash state, six rune slots, and cast/recant controls through `dm1_v2_hud_overlay_pc34` + `dm1_v2_hud_overlay_source_lock`. Remaining: finished inventory/champion/rune/action art/assets, live M11 frame-path handoff quality, fit/animation polish, and real screenshot/material pixel gates. |
| Enhanced lighting/shadows/field/projectile VFX | PARTIAL / OPEN-LARGE — 2026-06-21 audit/runtime gates: `dm1_v2_lighting_dynamic_pc34`, `dm1_v2_lighting_dynamic_source_lock`, `dm1_v2_enhanced_effects_runtime_pc34`, `dm1_v2_field_projectile_effect_metadata_pc34`, `dm1_v2_field_projectile_vfx_pc34`, `dm1_v2_extended_field_vfx_pc34`, `dm1_v2_anim_timing_pc34`, `dm1_v2_creature_render_pc34`, `dm1_v2_spell_effect_pc34`, and `dm1_v2_field_projectile_effect_metadata_source_lock` are CTest-gated. These gates cover source-palette lighting mirroring, deterministic fallback, additive light-map math, render-presentation-gated particle ticking, dynamic-lighting-gated light-map ticking, field/projectile metadata, spell-overlay/particle triggers and framebuffer writes, fluxcage-field routing, extended teleporter/pit/stairs/fakewall/floor-ornament VFX families, V1-tick animation interpolation, creature animation frame clamping/manifest anchors, and presentation-only non-mutating boundaries. Remaining: finished enhanced shadows, live M11 frame-path handoff into visible effects, finished art, and real screenshot/material pixel gates. |
| Smooth movement interpolation coverage | FIXED -- pass1068 expands `dm1_v2_movement_camera_pc34` with deterministic forward/back/left/right camera-offset coverage, end-offset reset checks, and presentation-only runtime invariants alongside the smooth-movement source-lock gate |
| Full V1/V2 deterministic input scripts + screenshot/pixel gates | PARTIAL / OPEN-BOUNDED — 2026-06-21: `dm1_v2_source_route_state_hash_pc34`, `dm1_v2_side_by_side_seed_pc34`, `dm1_v2_v1_v2_side_by_side_seed_pc34`, `dm1_v2_side_by_side_presentation_seed_probe`, `dm1_v2_selected_resolution_input_mapping_pc34`, `dm1_v2_4k_input_zone_mapping_pc34`, and `dm1_v22_modern_resolution_matrix_pc34` are now CTest-gated. They cover deterministic source-route hashing, canonical V1/gap/V2 seed layout, full-lane and D1C wall/portrait pixel-region parity, presentation-disabled V1/V2 viewport parity across N/E/S/W, selected-resolution source-coordinate mapping, 4K source touch/HUD zone mapping, and V2.2 selected-resolution retention when missing game data correctly blocks launch. Follow-up: source-only DM1 V2 entry viewport fixture/export gates now run as ordinary CTest gates, not expected-fail placeholders: `dm1_v2_completion_matrix_gate`, `dm1_v2_viewport_composition_source_lock`, `dm1_v2_viewport_pixel_capture_fixture_gate`, and `dm1_v2_entry_viewport_png_export_gate`. The D0-D3 draw-list comparator and DUNGEON.DAT square-decoder source gates are also unmasked as ordinary CTest passes with refreshed source paths and tracked-evidence fallback (`dm1_v2_d0_d3_draw_list_comparator_gate`, `dm1_v2_dungeon_dat_square_decoder_source_lock`). Phase-gate/source-isolation coverage is also documented through `dm1_v2_phase_gate_pc34`, `dm1_v2_phase5_runtime_bridge_pc34`, `dm1_v2_graphics_pipeline_source_isolation`, and `dm1_v2_phase0_phase1_source_lock`: V1 gameplay domains remain source-owned while V2 render/input/config presentation seams are explicit, and Phase 5 camera interpolation starts only from accepted V1 movement/turn ticks. 2026-06-22: `dm1_v2_runtime_presentation_smoke` now runs real `firestaff --game dm1` with temporary launcher config for launchable V2.0/V2.1 modes, captures source indexed BMP receipts plus post-palette/post-filter presented RGBA BMP receipts, verifies M11 reports source `dm1` plus presentation modes 1/2 at 640x400, and requires distinct presented-frame hashes across the configured V2.0/V2.1 paths. This promotes runtime script evidence only; it does not run DOSBox, require original-pairing evidence, or claim finished V2.2 real-art parity. Remaining gap: broader finished-art/material pixel verification stays tracked in the per-mode material row. |
| Per-mode pixel/material verification gates | OPEN-BOUNDED — 2026-06-21: Apple-Silicon V2.0/V2.1/V2.2 GPU readback gates now cover filtered (`dm_v20_filtered_renderer_silicon`), upscale (`dm_v21_upscale_renderer_silicon`), and modern placeholder (`dm_v22_modern_renderer_silicon`) paths; live M11 now prefers the optional V2.2 in-place cache and `dm1_v22_inplace_render_probe` covers synthetic wall/floor/pit/stairs material routing with no field-to-wall fallback. Additional CTest gates now cover the DM1 V2.1 EPX/palette/category asset pipeline (`pass648_dm1_v2_asset_pipeline`), the low-level DM1 V2.2 asset-pipeline unit contract (`dm1_v22_asset_pipeline`: provenance, fallback/category naming, manifest validation, and best-available provenance fallback), DM1 V2.2 modern verification suite (`dm1_v22_verification`: asset mode, manifest discovery/validation, fallback chain, shape selection, config integration, and V1 gameplay phase-gate preservation), current viewport material-category draw-list/flat-render routing (`dm1_v2_viewport_materials_pc34`), and a data-free cross-mode material-signature gate (`dm1_v2_per_mode_material_signatures_pc34`: canonical D0-D3 composition -> V2.0 flat material hash `0x2b0dd7dd`, V2.1 EPX RGBA hash `0x3fae57cd`, V2.2 synthetic in-place hash `0x30894af5`). Remaining gap: finished V2.2 real-art material/pixel gates plus broader deterministic V1/V2 screenshot scripts. |

---

## C. CSB gaps

Source: `docs/FINAL_CSB_GAPS.md` (135 lines), 5 csb_gap_*.md files,
greatstone g_csb.html (19 CSB/game + utility-disk extraction entries),
dmweb CSB Game Page, and DMWeb's CSB Atari ST edition page.

### C1. Champions/mechanics/dungeon/graphics

| Gap | Doc | Status |
|---|---|---|
| Champions per-stat parity | csb_gap_champions.md | PARTIAL — 2026-06-26 data-free `test_csb_v1_champion_per_stat_parity_pc34_compat` (CTest `csb_v1_champion_per_stat_parity`, 67/67 PASS) pins every per-stat field of the CHANGE7_24 reincarnation rule against `csb_v1_champion_reincarnate()` in `src/csb/csb_v1_character_pc34_compat.c`: HP/Mana/Stamina halve, six non-Luck stats reduced by 1/8th of current value with minimum-floor clamp, Luck exempt, all 16 skill slots cleared, NEEDS_RENAME attribute set, DEAD attribute cleared, ActionIndex reset to REST, per-champion `reincarnateStatPenalty` divisor (default 8, plus 4/16 and zero-divisor fallback), C160 resurrect-vs-C161 reincarnate panel contract, and alive-champion no-op contract. Remaining: live real-asset F0282 panel integration gate (requires a real CSB Utility Disk `CSBGAME` save + M11 input handler wiring the C040 panel into the champion-mirror sensor), per-champion `randomPoints`+`reincarnateAttributePenalty` interaction with the seeded F0309 max-load formula, and the older `csb_v1_reincarnation_penalty_pc34_compat` (ChampionState_Compat shape) still lacks a CMakeLists entry. |
| Combat mechanics | csb_gap_combat.md | PARTIAL |
| Dungeon model/mechanics | csb_gap_dungeon.md | PARTIAL — 2026-06-21 PC launch boundary now has a positive real-data gate: `csb_v1_pc_real_asset_launch` verifies canonical PC CSB assets scan by hash, enter `csb_v1_boot_enter_game()`, load `DUNGEON.DAT` into the runtime-owned dungeon singleton, select map 0, tick once, and clean up. Core PC runtime/input/movement/system slices are now CTest-registered through command-chain, input-queue, movement step/rotation, runtime tick, queue overflow, reincarnation, projectile-speed, Grey Lord, DECOMPDU, version-checker, monster-generator, chaos cast, DSA trigger, save import path, save runtime boundary, Neophyte, and Zokathra gates. Remaining dungeon/mechanics work is deeper end-to-end gameplay parity, real save compatibility artifacts, viewport/UI runtime evidence, and playability, not the PC asset handoff. |
| Graphics + ornament blits (F0108, F0115, F0111, CustomBackgrounds) | csb_gap_graphics.md | PARTIAL / OPEN-LARGE — 2026-06-21 CTest now covers seven data-free CSB viewport/source-lock slices: first CustomBackgrounds backdrop, room-slot backdrop-1, D1C F0108 floor/ceiling ornament, D1C F0115 thing pass, D3C F0107/F0108 first-backdrop composition, D3L/D3R sidewall backdrops, and D2C F0107 wall-ornament plus F0111 door-front layering. Existing `csb_v1_viewport_phase3_rendering`, inventory-grid, and viewport-inventory mouse gates remain green. Remaining: live real-asset ornament blits, broader viewport/HUD captures, and pixel parity evidence. |
| Full mechanics parity | csb_gap_mechanics.md | OPEN-LARGE |

### C2. Per-version CSB asset coverage

| Version | Status |
|---|---|
| CSB Atari ST 2.0 (en) — original | EXTRACTED + VERIFIED — local GRAPHICS.DAT/DUNGEON.DAT sizes and SHA256 hashes are recorded. DMWeb identifies original STX game+utility disks, v2.x screenshots, Atari ST utility screens, introduction-video provenance, no ending animation, and Insert/Clr Home/Ctrl-S input. Full Atari-runtime parity and utility-disk receipts are still separate |
| CSB Atari ST 2.0 (en) — cracked / MSA / hard-disk variants | BLOCKED-DATA — DMWeb separates cracked Automation ST media, MSA game+utility+save disks, ELiTE and Peter Putnik hard-disk hacks, and RamDisk hack references from the original STX media |
| CSB Atari ST 2.1 (en) | BLOCKED-DATA — DMWeb lists original STX game+utility disks and Meynaf's v2.1 assembler-source disassembly; needs v2.1 hash classification and utility-disk receipts before any runtime claim |
| CSB Amiga 3.1 (en-fr-ge) original | EXTRACTED — `~/.firestaff/data/csb-extras/amiga-3.1-multi/` (no canonical hash match yet, awaiting verification); DMWeb lists protected original ADF plus unofficial IPF media, so original/cracked classification remains required |
| CSB Amiga 3.1 (en) cracked EndlessPiracy | BLOCKED-DATA — DMWeb notes the multilingual Enless/Endless crack can later detect non-original media and block Vi altar revival; do not treat as canonical |
| CSB Amiga 3.1 (en) cracked Betrayal | BLOCKED-DATA — DMWeb notes the SCSI of Betrayal crack does not play the ending animation at game end; keep as noncanonical |
| CSB Amiga 3.3 (en-fr-ge) | EXTRACTED + VERIFIED — `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/` matches a canonical hash (Meynaf FR hack variant); DMWeb identifies this as a hard-disk/accelerator hack with original game+utility files copied into folders and patched in memory |
| CSB Amiga 3.5 (en/en-fr-ge) original | EXTRACTED — `csb-extras/amiga-3.5-ctraw-en/` (CTRaw format, not the canonical CSB Amiga hash); DMWeb lists protected original ADF and unofficial IPF media, v3.5 multilingual language-choice/entrance screenshots, Quit button, and Ctrl-Q/Ctrl-A entrance quit |
| CSB Amiga Utility disk (fr/ge/en/r1/r2/r3) | EXTRACTED — Disk 2 (en/fr/de) + Disk 3 (en/de) at `csb-extras/amiga-util-disk{2,3}-{en,fr,de}/`; DMWeb separates English releases 1-3, French release 1, German releases 1-2, release 2 as buggy, and release 3 as the fixed English utility disk |
| CSB FM-Towns (en-jp) | EXTRACTED — `csb-extras/fm-towns/` (484 MB ISO, awaiting canonical hash match); DMWeb identifies the Japan v3.1 BIN/CUE CD line, English/Japanese screenshots, Champion Editor and portrait-loading screens, CD audio tracks 02-31, and FM Towns Ctrl-S/Shift-S plus shifted-arrow input |
| CSB PC-98 3.1 (jp) | EXTRACTED — `csb-extras/pc98-3.1-jp/` (171 .raw files, awaiting hash match); DMWeb identifies the Japan v3.1 HDM/floppy line, original media missing copy-protection sectors, cracked media kept separate, 8-bit/16-bit screenshots, Champion Editor/portrait-loading screens, PC-98 keypad / Alt-S input, and `CSBGAME` protection offsets |
| CSB X68000 (jp) | EXTRACTED — `csb-extras/legacy-jp-x68000/`; DMWeb identifies the Japan v3.1 HDM/floppy line, original media missing copy-protection sectors, `CK.R` cracked media kept separate, blank save disk, 24/31 kHz screenshots, Champion Editor/portrait-loading screens, and X68000 Ctrl-S / Opt.1 / Opt.2 input |
| CSBWin (PC port by Paul Stevens) | PARTIAL — synthetic loader exists; real-asset test missing |

### C3. CSB hidden-code items

| Gap | Status |
|---|---|
| Atari ST hidden executable-code items (skip table) | FIXED — `csb_hidden_code_skip_table_unit` + `csb_v1_graphics_hidden_item_skip_pc34` PASS 2/2 |
| Amiga 558-562 items (skip table) | FIXED — same skip-table/loader gates cover Amiga 21/676/686 and 558-562 |
| CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame) | PARTIAL — csbgraphics.dat side: 2026-06-26 bounded index classifier landed (`csb_v1_csbgraphics_dat_classify` + `csb_v1_csbgraphics_dat_real_scan` + `firestaff_csb_v1_csbgraphics_dat_real_scan_probe` + `test_csb_v1_csbgraphics_dat_classify`). CTest `csb_v1_csbgraphics_dat_classify_unit` PASS 11/11 (data-free synthetic fixtures, big-endian + 0x8001 little-endian marker round-trip, bounds rejection, max-tracking, source-evidence citation). CTest `csb_v1_csbgraphics_dat_real_scan` PASS (skip-safe on the empty default known-hash list). dmsave/csbgame side: 2026-06-27 loader-boundary contract gate landed (`csb_v1_csbwin_save_loader_boundary_pc34_compat` + `test_csb_v1_csbwin_save_loader_boundary_pc34_compat` + `firestaff_csb_v1_csbwin_save_loader_boundary_probe`). CTest `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` PASS 79/79 (data-free synthetic fixtures, 14 documented shapes: 3 accept + 11 reject; per-shape loader-boundary check via `csb_v1_import_csb_save_buffer()`; builder determinism; hand-rolled 2-champion v2.0 round-trip; accept-shape helper `csb_v1_csbwin_save_loader_boundary_match()` recognises v2.0/v2.1 magic+version and rejects DM1 RDMCSB15 / CSBWin 512-byte / CEDT / NULL / zero-length; source-evidence citation chain). CTest `csb_v1_csbwin_save_loader_boundary` PASS (skip-safe on hosts without a user-staged `csbgame.dat` / `csbgame.bak` / `dmsave.dat` / `dmsave.bak`; synthetic-fixture portion still proves the contract). Sibling branch `gapbug_20260626_custom_dungeon_csbwin_import_evidence` separately ships `csb_v1_csbwin_save_classify_pc34_compat` (on-disk shape detection, no loader invocation) — these two are complementary, not duplicative. Remaining work: actual CSBgraphics.dat LZW payload decoder, M11 viewport override hook, real user-staged CSBgraphics.dat MD5 to populate the default known-hash list, CSBWin 512-byte XOR-pad obfuscation-key decoder, DM1→CSB raw-save conversion, and end-to-end importer wiring that feeds both halves into `csb_v1_import_csb_save_buffer`. |

### C4. CSB V2

| Gap | Status |
|---|---|
| V2.1/V2.2 dispatch + csb_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for CSB** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE — same as DM1 B3 |
| Per-cell modern-art swap in CSB 9-square viewport | OPEN-LARGE |
| Phase 3 enhanced UI overlays | FIXED — 2026-06-22: `firestaff_csb_v2_hud_overlay_probe` CTest-gated and PASS 61/61. Covers CSB_V2_PHASE_DOMAIN_HUD gate, presentation-only runtime, phase-gate integration (V1 stays source-locked), and source evidence citations. Library sources `csb_v2_hud_overlay_pc34.c` + `csb_v2_hud_runtime.c` + `csb_v2_phase_gate_pc34.c` wired in CMakeLists. Labels: tier2;csb;v2;phase3;hud;presentation-only. |
| Phase 5 smooth movement deterministic pixel gates | FIXED — 2026-06-22: `csb_v2_smooth_movement` + `csb_v2_smooth_runtime_binding` + `csb_v2_phase7_verification` + `csb_v2_phase7_verification_source_lock` are CTest-gated and PASS 4/4. Phase 7 verification suite is now active. |
| Phase 0/1/2/3/4/5/7 probes wired to CTest | FIXED — 2026-06-22: 7 CSB V2 probes previously buildable but not CTest-registered are now wired as test targets: `csb_v2_presentation_mode_probe` (mode-selector), `csb_v2_texture_upscale_probe` (v2.1), `csb_v22_shapes_probe` (v2.2 modern shapes), `csb_v2_filter_config_probe` (v2.0), `csb_v2_per_frame_filter_dispatch_probe` (v2.0), `csb_v2_filter_chain_probe` (v2.0), `csb_v2_settings_probe` (v2-config). All 13 related tests PASS. |
| Cross-domain probe watchdog batch (5 probes) | FIXED — 2026-06-22: 5 data-free probes previously either not built (no add_executable) or built but not CTest-registered are now wired as test targets: `dm2_v2_phase3_hud_overlay_probe` (added add_test, 61/61 PASS), `dm1_v2_v1_v2_side_by_side_seed_probe` (NEW, PASS), `dm2_v1_creature_combat_probe` (NEW, PASS), `m11_inscription_font_probe` (NEW, PASS), `m11_pass30_movement_legality_probe` (NEW, 17/17 PASS). All 6 related tests PASS. |

---

## D. DM2 gaps

Source: `docs/NEXUS_PLAN.md` (similar scope), greatstone `g_dm2.html`
(15 extraction entries), skproject source (DM2 Windows port).

### D1. DM2 V1 mechanics

| Gap | Status |
|---|---|
| Data model | FIXED |
| Boot/profile | FIXED |
| Rendering pipeline | FIXED |
| Combat resolver | FIXED |
| Spell module | FIXED |
| Tech/magic module | FIXED |
| **Shops/NPCs** | FIXED — 2026-06-21: Parts A landed (`1e756018 dm2_v1: Part A — shop + NPC parity (Phase 4 mechanics)`) + `a89c257f tier4-18: DM2 V1 shop-economy determinism probe (skproject c_shop.cpp)` (19/19 PASS) + `test_dm2_v1_shop_pc34_compat` (51/51 PASS). Source-locked against skproject `c_shop.cpp` transaction pricing + `SKWinGlobal.h:42` `NUM_NPCS=4`. |
| **Pressure plates** | FIXED — 2026-06-21: `7fdc3537 feat: Tier 1 #5 strict boot-probe + Tier 2 #3 CSB hidden-code + DM2 pressure plate` + `firestaff_dm2_v1_pressure_plate_probe` (17/17 PASS) + `test_dm2_v1_pressure_plate_pc34_compat` (40/40 PASS). |
| **Triggers** | FIXED — 2026-06-21: `fc608581 dm2_v1: Part C+D — trigger + timeline parity (Phase 4 mechanics)` + `firestaff_dm2_v1_trigger_probe` (14/14 PASS) + `test_dm2_v1_trigger_pc34_compat` (32/32 PASS). |
| **Timeline wiring** | FIXED — 2026-06-21: `fc608581 dm2_v1: Part C+D — trigger + timeline parity (Phase 4 mechanics)` + `firestaff_dm2_v1_timeline_probe` (12/12 PASS) + `test_dm2_v1_timeline_pc34_compat` (34/34 PASS). |
| **Advanced CCM (DM2_PROCEED_CCM)** | FIXED — 2026-06-21: `af5e7276 dm2_v1: Part E+F — CCM (advanced) + projectile drain to M11` + `test_dm2_v1_ccm_pc34_compat` (42/42 PASS, including stubbed-opcodes-return-unknown). Source-locked against skproject `c_ccm.cpp`. |
| **Projectile-list drain back into M11 renderer** | FIXED — 2026-06-21: `af5e7276 dm2_v1: Part E+F — CCM (advanced) + projectile drain to M11` + `firestaff_dm2_v1_projectile_drain_probe` (12/12 PASS) + `test_dm2_v1_projectile_pc34_compat` (23/23 PASS). |
| **Original-overlay proof** | OPEN-BOUNDED — no Firestaff-vs-original DM2 evidence yet; canonical launch smoke is now gated separately, but no original overlay/pixel evidence has been produced. 2026-06-25 added the bounded source/runtime scaffold for future paired DM2 overlays: `scripts/dosbox_dm2_original_overlay_capture.sh` (DOS4GW-aware DOSBox config + Swift/xdotool route injector + 320x200 raw + 224x136 viewport crop normalization), `tools/verify_dm2_v1_original_overlay_capture_source_lock.py` (10 SKULLWIN source anchors across dm2global.h/c_gfx_main.cpp/c_gfx_main.h/c_gui_vp.cpp/c_tmouse.h/types.h/c_input.cpp/c_gui_draw.cpp), and the data-free probe `firestaff_dm2_v1_original_overlay_capture_scaffold_probe` (7/7 PASS, CTest `dm2_v1_original_overlay_capture_scaffold_probe` PASS); the source-lock verifier is CTest-gated as `dm2_v1_original_overlay_capture_source_lock` and currently PASS on this host with `verification-screens/passH2313-dm2-original-overlays/` stage + route-template + DOSBox config produced. Status remains OPEN-BOUNDED until a paired dungeon_gameplay row from both original DM2 PC 1.0 EN and Firestaff lands with hashes that match the documented route. |
| **Launch-smoke gate** | FIXED — 2026-06-21: DM2 canonical `--game dm2 --data-dir ~/.firestaff/data/dm2` and DM2 PC extras `dm2-extras/dos-en`, `dm2-extras/dos-fr`, `dm2-extras/pc-fr`, and `dm2-extras/pc-de` emit `DM2 READY` through the M11 stderr-pipe and are covered by `tier1_strict_boot_probe`; broader non-PC/demo launch remains tracked under D3 |

### D2. DM2 V2

| Gap | Status |
|---|---|
| Phase 2 asset pipeline + dm2_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for DM2** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE |
| Per-cell modern-art swap in DM2 V1 (T560 indoor, T600 outdoor) | OPEN-BOUNDED — 2026-06-27: new `dm2_v22_viewport_swap_pc34.{c,h}` provides the bounded per-cell swap API + render pass (indoor T560 4×3 grid + outdoor T600 sky/horizon/ground). `Dm2_V22_ShapeType` 24-entry enum (walls + floors + creatures + doors + fields + outdoor shapes + SHAPE_NONE sentinel) + `dm2_v22_shape_for_cell()` discriminator + `dm2_v22_asset_id_for_shape()` resolver. New CTest `dm2_v22_inplace_render_probe` 28/28 PASS driven by a synthetic manifest under a probe-controlled HOME (no real GRAPHICS.DAT/PNG receipts needed). V1 source ownership preserved: the render pass is a no-op when V22 modern pack is missing, presentation mode is not V22_MODERN, the cache file is not loaded, or the cache is not populated. Companion fix: `dm2_v22_inplace_draw_pc34.c` previously hardcoded `~/.firestaff/assets/dm1/modern/...` (copy/paste leftover from the CSB template) — now mirrors `dm2_v22_set_manifest_path()` in `dm2_v22_modern_assets_pc34.c`. Source-locked against SKULL.ASM T520/T560/T600 + ReDMCSB DUNVIEW.C:2962-3070 + DUNVIEW.C:4351-4382 F0112. Remaining: (a) wire `dm2_v22_viewport_swap_render()` into the DM2 V2 viewport draw path (currently `dm2_v2_viewport_renderer.c` only manages smooth animation state, not the actual framebuffer paint), (b) authoring real DM2 PBR hero art via gpt-image-2 batch + real-asset receipts in `~/.firestaff/assets/dm2/modern/`, (c) per-mode pixel/material verification gates (V1 vs V2.0 vs V2.1 vs V2.2) to confirm the per-cell swap is observable end-to-end. |
| Phase 3 HUD runtime | PARTIAL — `firestaff_dm2_v2_phase3_hud_overlay_probe` 61/61 PASS + `test_dm2_v2_hud_overlay` 76/76 PASS. Source-locked against ReDMCSB `PANEL.C`. Remaining: HUD widget bitmap assets (inventory quick-view, action prompt) for finished presentation polish. |
| Phase 3 HUD bitmap assets + widgets (inventory quick-view, action prompt) | OPEN-BOUNDED — runtime gates are wired (probe 61/61 + test 76/76); bitmap assets need finished PBR HUD widget art. 2026-06-25: added `dm2_v2_hud_widget_assets` placeholder-vs-real manifest/gate (Phase 3 primary slots `inventory_quick_view`/`action_prompt` + 5 chrome supporting slots: compass rose, depth indicator, gold counter, champion bar frame, action strip frame). `test_dm2_v2_hud_widget_assets` 115/115 PASS and `firestaff_dm2_v2_hud_widget_assets_probe` 54/54 PASS cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with NO_MANIFEST-by-default baseline matching the current runtime. Source-locked against `SKULL.ASM T560`, `skproject/SKULLWIN/c_gui_vp.cpp`, `ReDMCSB PANEL.C`, `include/dm2_v22_modern_assets_pc34.h` (sibling V2.2 manifest pattern). **2026-06-27 runtime hook landed:** `dm2_v2_hud_runtime_render_with_assets()` consults `dm2_v2_hud_widget_assets_classify_slot()` per slot, records the runtime path-mode (`REAL_BITMAP` vs `PROCEDURAL_FALLBACK`) and gate verdict into module state, and stamps a 1-pixel marker at each REAL slot's anchor after the procedural overlay draws; new `dm2_v2_hud_runtime_last_path_mode()` / `_last_path_counts()` / `_last_slot_class()` getters expose the record. New `firestaff_dm2_v2_hud_widget_runtime_hook_probe` 100/100 PASS covers no-manifest/empty/placeholder/PARTIAL/COMPLETE baselines, per-slot path-mode stability across re-renders, V2-off phase-gate invariant, and source-evidence citation. Honest boundary: the anchor stamp is the documented replacement site for the real-bitmap blit; the actual pixel decode is intentionally NOT implemented (operator-installed PBR HUD widget art is the OPEN-BOUNDED next step). Remaining: operator-installed `~/.firestaff/assets/dm2/hud/hud_widget_manifest.json` plus per-slot real `source_file` entries with `generator != "placeholder"` for `PARTIAL`/`COMPLETE` promotion, plus finished PBR widget art, plus the actual real-bitmap blit that replaces the 1-pixel anchor stamp. |

### D3. DM2 per-version coverage

| Version | Status |
|---|---|
| DM2 PC 0.9 / 1.0 (en/fr/ge) / demo | EXTRACTED + VERIFIED + LAUNCH-TESTED for PC extras `dm2-extras/dos-en/`, `dm2-extras/dos-fr/`, `dm2-extras/pc-fr/`, and `dm2-extras/pc-de/` via `tier1_strict_boot_probe`; DMWeb's PC edition page identifies Europe/USA English redump BIN/CUE CD images, USA CD-content archives, France/Germany regional CD/floppy/Bestseller lines, final DOS HMP music in `GRAPHICS.DAT`, VGA/SVGA MVE videos, HMI/GUS sound-driver patch material, and a DOS Alt-S / Shift-arrow / keypad / wall-ornate command table. Demo status still needs separate classification across the five known 1995 builds, including the early `FIRE.EXE`/LZ91 no-music/no-save demo and later `SKULL.EXE`/Watcom builds with different `GRAPHICS.DAT`, `DUNGEON.DAT`, sound, logo, title, and order assets |
| DM2 Amiga 1.0 (en-fr-ge) | EXTRACTED — `dm2-extras/amiga-en/`; DMWeb's Amiga edition page identifies the release as Europe-only v1.0 with Germany/UK edition pages, six ADF/IPF floppy images that cannot be played directly from floppy and require hard-disk installation, MOD music files keyed by `CD.DAT`, WinUAE-captured Smacker videos, 68020+ / OCS/ECS hardware reality notes, and an undocumented Ctrl-S / Del-Help / keypad wall-ornate command table |
| DM2 MegaCD/SegaCD 1.0 (jp/en) | EXTRACTED — `dm2-extras/mega-cd-jp/`; DMWeb's Sega CD / Mega CD edition page identifies Europe/USA English and Japan Japanese v1.0 releases as redump BIN/CUE CD images, lists DMFiles CD-content archives for USA/JP, documents a data-track ISO plus audio-track MP3 split, and notes that track 7 is replaced by 15 seconds of silence |
| DM2 Macintosh 1.0 (en/jp/demo) — uses QuickTime .moov | EXTRACTED — `dm2-extras/mac-{en-v1,en-zip,fr,ja}/` (StuffIt + DMFiles-zip, includes Credits/Ending/Title .MooV); DMWeb's Macintosh edition page identifies JP, USA, and USA demo as redump BIN/CUE CD images plus CD-content archives. It records the JP/US split: JP uses older 16-color graphics, an intro animation also present on Sega CD, CD-audio, and `Skullkeep` resource-fork protection offsets, while USA uses upgraded 256-color graphics, MIDI/SoundMusicSys resources, Mac menu/balloon help, and a distinct Command-key / wall-ornate input table |
| DM2 PC-9801/PC-9821/IBM PS/V 1.0 (jp) | EXTRACTED — `dm2-extras/pc9821-jp/`; DMWeb's PC-9801 edition page identifies the JP v1.0 full release as four FDI disk images, the demo as `.hdm` setup media + `.hdi` bootable hard-disk image, and the PC-9801 runtime as no-music with a distinct keypad / Alt-S command table. DMWeb's PC-9821 page identifies the JP v1.0 release as a BIN/CUE CD image with six CD.DAT music tracks, the same PC-98 keypad / Alt-S command table, and LZEXE `FIRE.EXE` CD-ROM protection checks during load/save. DMWeb's IBM PS/V page identifies the JP v1.0 release as original three floppy disks plus WinImage disk images, no music, IBM PS/V keypad / Alt-S / Shift-arrow input, and separate `FIRE.EXE` protection offsets for new-game/save behavior |
| DM2 FM-Towns 1.0 (jp) | EXTRACTED — `dm2-extras/fm-towns-ja/`; DMWeb's FM Towns edition page identifies the JP v1.0 release as a redump BIN/CUE CD image with CD-audio differences (tracks 2-6 slightly quieter, extra silent track 8) and a distinct Ctrl-Shift-S / shifted-arrow command table |

---

## E. Nexus gaps (Saturn)

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/NEXUS_PLAN.md`,
Nexus locally verified files in `~/.firestaff/data/nexus/`.

### E1. V1 phases 0-7

| Gap | Status |
|---|---|
| DMDF parser | PARTIAL |
| DGN world loader | PARTIAL |
| MNS creature/spell rendering | PARTIAL |
| S2D font loader/renderer | PARTIAL — parser, bounded indexed-framebuffer renderer, and bounded real-on-disk SEGA SATURN SCR section-table parser (`nexus_v1_saturn_font_scr_sections`, 55/55 PASS) are CTest-gated; runtime text layout and real Nexus screen render capture remain open. |
| TLINK/TAI/SAL/MAP runtime | PARTIAL |
| Save/load (.sav) | PARTIAL |
| V1 mechanics | PARTIAL |
| **Real Saturn asset handoff (NEXUS.BIN/ISO)** | EXTRACTED + VERIFIED + PHASE-LAUNCH GATED — 2026-06-25: `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` matches canonical DM.BIN hash; the Track 1 .bin (not a re-muxed merged ISO) is the actual source the engine opens through `nexus_iso_open_cue`. `firestaff_nexus_v1_track1_phase_launch_probe` is now CTest-gated as `nexus_v1_track1_phase_launch_{synthetic,extracted_root,saturn_ja_iso}` and PASS 3/3 on 2026-06-25: synthetic-only path covers the engine-init / DGN-parser / DMDF-parser / BPK-parser / Saturn-font-parser / determinism contract for phases 0-7 (32 PASS); `extracted_root` (53 PASS) and `saturn_ja_iso` (56 PASS) additionally drive the live `nexus_v1_init → nexus_v1_read_file → nexus_v1_load_level(0) → nexus_v1_tick × 5 → nexus_v1_load_model("SCORPION.MNS") → nexus_v1_shutdown` pipeline against the verified Track 1 media, including DMDF parser handoff on a real 3D model, real FONT256.S2D parser handoff (256 chars, 16×16), MENU.BPK BPPK magic, and Track 1 BIN ISO-contract checks (`nexus_iso_is_nexus` recognises DM.BIN + LEV00.DGN; `nexus_iso_find("DM.BIN")` returns a >=64 KiB payload). Demo and EN/FR fan-translation media lines (separate from the JP retail disc) remain tracked for separate classification. DMWeb's Saturn pages identify the official JP retail disc (Victor Interactive Software catalog `T-9111G`, 1998-03-26), a separate JP demo, CD audio tracks 02-09, optional EN/FR fan-translation disc images, and public-proof material from the Japan retail page: screenshots, 48-page booklet, physical scans, and a two-part Japanese playthrough. Remaining: classify demo and EN/FR fan-translation media separately from the JP retail Track 1 source, and capture an actual Nexus screen using the real Track 1 assets before promoting this beyond phase-launch gated. |

### E2. V2 phases

| Gap | Status |
|---|---|
| Phase 0/1/2 | FIXED |
| Phase 5 smooth movement runtime | FIXED — 2026-06-22: `nexus_v2_smooth_movement` + `nexus_v2_smooth_movement_runtime_probe` CTest-gated and PASS 2/2. |
| Phase 7 verification (deterministic input + screenshot gates) | FIXED — 2026-06-22: data-free `firestaff_nexus_v2_verification_suite_probe` now extends the existing render-pipeline checks to 53/53 assertions with Nexus V1 movement-backed deterministic input scripts, phase-gate domain boundaries, V1/V2.1/V2.2 gameplay state-hash equality, and synthetic screenshot-style full-frame + viewport-region pixel hash gates for V2 OFF, V2 UPSCALED, and V2 ENHANCED. CTest `nexus_v2_verification_suite_probe` and focused Nexus sweep PASS 17/17. This closes the bounded deterministic gate without requiring Saturn assets; real-asset/public screenshot promotion remains separate from Phase 7. |
| **Real PBR hero art for Nexus** | OPEN-LARGE |
| **Per-cell modern-art swap in Nexus V1 draw pipeline** | OPEN-LARGE |
| Light-overflow gameplay bug | PARTIAL — 2026-06-25 added a data-free Nexus V1 light-overflow data model + bug-classification API (`include/nexus_v1_light_overflow.h`, `src/nexus/nexus_v1_light_overflow.c`) plus a CTest probe (`firestaff_nexus_v1_light_overflow_probe`, ctest `nexus_v1_light_overflow`, 61/61 invariants PASS). The data model mirrors ReDMCSB WIP20210206 `TIMELINE.C F0238` (BUG0_18 silent drop when the timeline hits `G0369_EventMaximumCount`), `TIMELINE.C F0257_ProcessEvent70_Light` (recursive weaker-event chain at `GameTime+4`), `MENU.C F0404_MENUS_CreateEvent70_Light` and `MENU.C:1926-1942` (Light/Torch/Darkness dispatch), `DATA.C:359` (16-entry `LightPowerToLightAmount` table), `CHAMPION.C:27` (`MagicalLightAmount` field), and `LOADSAVE.C:2041` (EventMaximumCount=100 base cap). DMWeb evidence (`editions/sega-saturn/` light-overflow symptom and `solutions/cheats-and-hacks/` permanent-spell-effect exploitation) and the dmweb cheats/hacks page both confirm BUG0_18 as the root cause. The classification hook (`Nexus_V1_LightOverflowKind` enum) returns one of `NONE` / `TIMELINE_FULL_PERMANENT_LIGHT` / `LIGHT_BLEED_NEGATIVE` / `CAST_REJECTED` so the future M11 runtime can either **emulate** (default — source-faithful V1 path that reproduces the dmweb-documented "permanent Light" / "dungeon into darkness" symptoms) or **guard** (set `guard_rejects=1` on the timeline to refuse casts at the cap, suitable for replay/capture contexts). Remaining: Saturn runtime asset handoff (DM.BIN, FONT256.S2D, MNS models), live M11 wire-in to apply the cast hook per champion/mana check, real-asset decay-tick evidence, and saving the MagicalLightAmount field across `.sav` round-trips. None of those block the bounded classification contract. |

---

## F. Theron gaps (PC Engine / TurboGrafx-16)

Source: DMWeb Theron's Quest overview and PC Engine / TurboGrafx-16
edition pages, `docs/NEXUS_PLAN.md` (similar shape), Theron local
probes.

### F1. V1

| Gap | Status |
|---|---|
| V1 parser | FIXED |
| Rendering pipeline | FIXED |
| Mechanics | FIXED |
| Save/load (.SRM) | PARTIAL — data-free `.tqsv` slot round-trip, rejection, verification, and cross-slot export/import are CTest-gated; 2026-06-25 adds `theron_v1_srm_classifier` (probe + unit test) as a bounded Save Disk manifest that detects the gzipped-deflate body dmweb references, reports a clean ABSENT manifest on the current host, and accepts a real `.srm` when one is staged under the configured root. No real `.srm`/Track 02 save artifact is present locally yet. |
| Track02 bank routing | FIXED |
| Dungeon progression (7 dungeons) | FIXED |
| **JP/US Track 02 BIN/ISO real-asset launch** | FIXED — `tier1_strict_boot_probe` covers Theron JP canonical, Theron JP extras, and Theron US extras booting to the TQR level-load milestone; `theron_v1_launcher_scan_reuse` and `theron_v1_track02_bank` also PASS. |
| **Theron m11 runtime command proof** | FIXED — 2026-06-21 (commit `cd86d520`): `firestaff_theron_v1_cross_route_mechanics_probe` CTest-gates a real `firestaff --game theron` run via temporary launcher config, captures a M11 run receipt (launch + early command tick), verifies M11 reports source `theron` plus command processing, and proves the M11 run path is launchable on this host. 2026-06-21 (commit `363bf3b9`): `tqr_v1_track02_bank_signal_2026-06-03.md` locks raw Track 02 bank anchors (`0x1F000..0x1FFFF` graphics bank + 0x20000..0x27FFF dialogue + `0x28000..0x2BFFF` map-data) against the CD-ROM2 1MB sector map. |
| **Theron 24h readiness rollup** | FIXED — 2026-06-21 (commit `a0592d6d`): `tools/theron_24h_readiness.py` + `parity-evidence/verification/theron_24h_readiness/manifest.json` + `docs/THERON_CAPTURE_READINESS.md` roll up 7 Theron V1 readiness gates (track02 bank, save/load, cross-route mechanics, runtime screenshot, dungeon progression, cross-slot, m11 launch) into a single per-day PASS/FAIL line, mirroring the DM1 24h readiness pattern. 2026-06-21 (commit `393d9f64`): `theron: refresh readiness reports` re-emits the manifest with current commit SHAs. |
| **Theron runtime screenshot readiness** | FIXED — 2026-06-21 (commit `b7dbcd60`): `firestaff_theron_v1_runtime_screenshot_readiness` CTest-gate + `tools/verify_theron_v1_runtime_screenshot_readiness.py` + `parity-evidence/theron_v1_runtime_screenshot_readiness.md` prove the M11 path can produce a Track-02-backed screenshot receipt on the current host. CTest/screenshot gates pass for the launchable Theron paths. |
| **Theron runtime screenshot README-promotion gate** | FIXED — 2026-06-25: added `tools/verify_theron_v1_runtime_screenshot_promotion_gate.py` + `parity-evidence/verification/theron_v1_runtime_screenshot_promotion_gate/manifest.json` + `parity-evidence/theron_v1_runtime_screenshot_promotion_gate.md` + CTest entry `theron_v1_runtime_screenshot_promotion_gate`. The gate is the source-of-truth guardrail that audits the readiness manifest against an explicit README-eligibility contract (real Firestaff runtime capture, no deterministic fallback assets, `TQR level load` boot marker, semantic Track 02 loader evidence, unique source BMP sha256 per row, valid 320x200 presented BMP) and locks `NO_README_PROMOTION_PERMITTED` until at least one row is `README_ELIGIBLE`. Current host state (2026-06-25): 0/3 rows eligible — every readiness row still uses deterministic fallback assets and shows no semantic Track 02 loader evidence (probe `gameTick=0`, `party.mapIndex=0`, `lastOutcome='THERON READY'`); additionally the three rows share an identical source BMP sha256, which the uniqueness audit flags as a shared-placeholder-fixture smell. `docs/THERON_CAPTURE_READINESS.md` now points to the promotion gate as the single source of truth for whether a Theron capture may be promoted into `verification-screens/` or `docs/compare/`. |
| **DMWeb overview provenance** | FIXED / EVIDENCE-BOUNDED — 2026-06-25: DMWeb's Theron's Quest overview page pins the JP release to 1992-09-18, the USA release to 1993, and the runtime shape as a PC Engine / TurboGrafx-16 CD adaptation with seven small dungeons, between-dungeon saves only, companion skill/item resets, Theron skill/stat retention, and seven relic goals. Remaining work is turning those details into public progression/save/relic receipts rather than broadening the current Track 02 launch claim. |
| **DMWeb PC Engine / TurboGrafx edition provenance** | FIXED / EVIDENCE-BOUNDED — 2026-06-25: DMWeb's edition page separates JP, USA, and PCEWorks bootleg records; lists JP/USA redump BIN/CUE, CloneCD, and DMFiles ISO/OGG sources; identifies Track 02 as the data track; records tracks 01/03/04 as shared music with Japanese or English speech; lists JP tracks 05-18; and notes the JP Track 17 static-noise defect fixed in the US version. Remaining work is BIN/CUE-vs-ISO/OGG classification plus audio/intro/outro/screenshot receipts, while keeping the bootleg out of canonical data. |
| **DMWeb JP/USA retail provenance** | FIXED / EVIDENCE-BOUNDED — 2026-06-25: DMWeb's JP retail page records bar code `4 988002 247950`, 1992-09-18 release, ¥8200 price, jewel-case contents, 142x125x10 mm box, 32-page booklet, back card, spine card, and CD scans; the USA page records bar code `0 92218 00178 8`, 1993 release, jewel-case contents, 142x125x10 mm box, hook-back scan, 32-page booklet, back card, and CD scan. Remaining work is using these as public retail receipts without changing the Track 02 runtime boundary or admitting PCEWorks bootleg media as canonical. |
| Cross-slot import/export against real Track 02 saves | PARTIAL — `theron_v1_save_load` now covers export/import mechanics across save roots using validated Theron save images; 2026-06-25 adds `theron_v1_srm_classifier` (probe + unit test) as a bounded Save Disk manifest that detects the gzipped-deflate body dmweb references, reports a clean ABSENT manifest on the current host, and accepts a real `.srm` when one is staged under the configured root. Remaining work is still importing/exporting a real Track 02 save artifact when one is available. |
| Cross-route mechanics runtime evidence | FIXED for the CTest-gated mechanics path — 2026-06-21 `firestaff_theron_v1_cross_route_mechanics_probe` CTest-gates real `firestaff --game theron` command proof (commit `cd86d520`). Broader real-asset cross-route capture pairs (level-by-level route transcript) remain out-of-24h scope. |

### F2. V2

| Gap | Status |
|---|---|
| Phase 0/1 | FIXED |
| Phase 2 (presentation selection, EPX upscaler) | FIXED |
| theron_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for Theron** | OPEN-LARGE |
| **Per-cell modern-art swap in T400/T600** | OPEN-LARGE |
| **Phase 3 enhanced UI overlays** | PARTIAL — 2026-06-27 Theron V2 Phase 3 initial seed landed (`theron_v2_hud_overlay_pc34.c/.h`, `tests/test_theron_v2_hud_overlay_pc34.c` 58/58 PASS, `probes/firestaff_theron_v2_phase3_hud_overlay_probe.c` 40/40 PASS). Mirrors the CSB + DM2 Phase 3 sibling pattern with Theron-specific surfaces: top-bar (compass + quest items + dungeon progress 1/7 + relic counter 0/7 + 4-slot spell-rune ready indicator), bottom-panel (4 champion mini-bars HP/Stamina/Mana with slot 0 = Theron / leader), bottom action strip (ATK/CST/USE/DRP/MOV with active underline and 6-frame hit-flash decay), and V2.2 interaction feedback (low-HP 2 Hz pulse on `hp < 25%`). PC Engine native 256×224 indexed framebuffer (TQR_FB_W × TQR_FB_H). Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270 datasheet, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview (7 dungeons + 7 relic goals + rune magic), `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`. CTest `theron_v2_phase3_hud_overlay_probe` (40/40 PASS, labels `tier2;theron;v2;phase3;hud;presentation-only`) and `theron_v2_hud_overlay_pc34` (58/58 PASS) both green. **2026-06-27 (this pass) Phase 3 placeholder-vs-real asset gate landed:** `theron_v2_hud_widget_assets_pc34.c/.h` is the Theron-specific sibling of `dm2_v2_hud_widget_assets`. New CTest `theron_v2_hud_widget_assets_pc34` (105/105 PASS) and headless probe `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS, labels `tier2;theron;v2;phase3;hud;widget-assets;presentation-only`) cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with NO_MANIFEST-by-default baseline matching the current runtime. Slot table (7 slots, stable order, ordinals = indices): 5 Phase 3 primary (`compass_rose`, `quest_items`, `dungeon_progress`, `relic_counter`, `rune_indicator`, category `hud_widgets`) + 2 chrome supporting (`champion_bars`, `action_strip`, category `hud_chrome`). Manifest schema `{ id, generator, source_file, width, height }` aligned with sibling `theron_v22_modern_assets_pc34` and `dm2_v2_hud_widget_assets` shapes; manifest path `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`. Companion source-lock doc `docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md` documents the slot table, schema, gate state machine, M12/Phase 7 integration points, and honest boundary. Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview, `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`, sibling `dm2_v2_hud_widget_assets.h`. **Remaining Phase 3 work:** (a) M11 wire-up — call `theron_v2_hud_render()` after the Theron V1 viewport render gated on phase gate (mirror DM2 V2 HUD wire-up); (b) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`; (c) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator ≠ "placeholder"` so the new gate can promote the placeholder state to `PARTIAL`/`COMPLETE`; (d) real-art visual verification + per-region pixel gates against real Track 02 captures. Original DOS / original PC Engine pixel-parity claim stays out of scope. |
| **Phase 4 enhanced lighting/effects** | OPEN-LARGE — not started |
| **Phase 5 smooth movement** | FIXED — 2026-06-22: `theron_v2_smooth_movement_pc34` CTest-gated and PASS. |
| **Phase 6 touch/controller ergonomics** | OPEN-LARGE — not started |
| **Phase 7 V2 verification suite** | OPEN-LARGE — not started |

---

## G. Launcher / Settings / Accessibility (cross-cutting)

### G1. M12 launcher

| Gap | Status |
|---|---|
| 19-locale UI cycle | FIXED in v2.9.2 |
| Persistence for many options + 5 per-game slots | FIXED |
| **Polished UI flow** | FIXED — 2026-06-22: added CTest `m12_polished_ui_flow` over the public launcher input state machine. The gate covers initial no-data message dismissal, top-level LEFT no-op versus BACK exit, unavailable-game message routing, Museum category/page/back navigation, changelog shortcut/scroll/back navigation, settings row/tab/value/back navigation, and matched DM1 game-options launch-row handoff into a valid launch intent. `m12_extras_views_smoke` also drives the real extras input path, checks available extras pages, disabled extras popups, and BACK/ESC-style restoration from message, settings, game options, changelog, museum, manual/docs, bestiary, item encyclopedia, and screenshot gallery back to the main view/navigation level. `m12_menu_mouse` CTest-gates pointer hit-test parity for redesigned Extras row selection, disabled-extra popups, and available-extra view opens through the same input path. Popup focus retention is covered too: game-options launch popups return to the launch row, and settings save-manifest export/import popups return to the originating settings row. Data-directory scan feedback now reports the same bounded result in Settings and in the picker result popup (`NO VERIFIED DATA`, `FILES FOUND`, or exact ready-game count). Redesigned Play game-select opens per-game missing-data details for unavailable rows and returns to the same game-select row on BACK. |
| **No-data / missing-required-files popup once** | FIXED — 2026-06-22: added CTest `m12_no_data_popup_once_gate` proving the M12 launcher surfaces the no-data and missing-required-files popups exactly once per relevant scan or selection state. The gate covers the initial `InitWithDataDir` "NO GAME DATA FOUND" popup, ACK dismissal back to a clean MAIN view, unavailable-game-card "DM1 GAME DATA NOT FOUND" popup per selection event, the data-dir picker cancel path producing "DATA DIRECTORY UNCHANGED" once without a duplicate no-data popup on top, `M12_StartupMenu_SetDataDirectory` with an empty root producing "NO VERIFIED GAME DATA FOUND" once via the result-popup path, and quick-resume ACK against an unavailable game producing the missing popup once without setting `launchRequested`/`quickResumeLaunchRequested`. |
| Runtime handoff for every option | PARTIAL — CSB V1 launch-readiness blocker retired on 2026-06-21: matched CSB assets now produce a valid M12 launch intent, M11 hands CSB to `FS_GAME_CSB`, and `csb_v1_pc_real_asset_launch` proves PC CSB boot/tick. 2026-06-23: `csb_v1_m11_runtime_capture_boundary` and `pass547_csb_v1_runtime_readiness_backfill` were refreshed after `M12_StartupMenu_GetLaunchIntent()` grew to line 7513; both gates pass locally, pre-push passed, and GitHub Actions for `2c17264a` and `e8c3cb39` passed. Remaining gaps are per-game polished flows, richer CSB viewport/HUD/gameplay proof, original capture parity, and non-PC emulator parity. |
| **Save export/import** | FIXED — 2026-06-22: added M12 save-browser export/import helpers and CTest `save_browser_export_import_m12`, covering selected `firestaff-*.sav` export to a backup directory, byte-preserving import back into the data directory, destination no-overwrite behavior, invalid filename rejection, reported destination paths, and NULL-state rejection. M12 settings also expose bounded Export/Import Save Manifest actions that write/read `firestaff-save-export-manifest.json`, validate the manifest type, reject invalid path strings, preserve existing config on failed import, and CTest-cover config/API plus settings-row reachability via `m12_json_export_import` and `m12_startup_menu`. The manifest records launcher-known quick-resume path/context only (`runtime_save_bytes_included = 0`); per-game save-format compatibility, save-byte migration, and real runtime save compatibility remain tracked in game-specific rows. |
| **Session timer** | FIXED — 2026-06-22: added a persisted launcher-owned Session Timer setting (`Off`, `15m`, `30m`, `60m`, `120m`) with a visible modern settings row, mouse hit coverage, TOML/JSON config persistence, and helper coverage for remaining-time math. Added `M12_CampaignSessionTimer` with start/pause/resume/tick/flush-to-slot semantics and CTest `campaign_m12_session_timer`, covering stopped/paused no-op behavior, positive accumulation, negative tick rejection, HH:MM:SS formatting, flush into `playTimeSeconds`, modification timestamp update, reset after flush, empty flush no-op, and stale-session reset on restart. This is launcher/settings and save-slot accounting coverage; it does not claim an in-game forced-pause overlay. |
| **Manual/docs launcher** | FIXED — 2026-06-22: added a testable M12 manual/docs catalog (`manual_docs_m12`) and wired the launcher `Manual / Docs` extras row to an internal `M12_MENU_VIEW_MANUAL_DOCS` view instead of a hardcoded browser-only URL path. CTest `manual_docs_launcher_m12` gates the five user-facing entries (README manual, data setup, platform matrix, dmweb/Greatstone reference, release notes), Firestaff GitHub URL scope, repository-relative paths, existing source-tree files, lookup bounds, duplicate-id rejection, and confirms the internal primary gap list is not exposed as user manual content. `firestaff_m12_extras_views_smoke_probe` covers manual/docs navigation plus nonblank framebuffer output. |
| **Cloud sync** | OPEN-LARGE |
| Custom/V2 smooth-turn-pan toggles | FIXED — 2026-06-22: M12 modern settings now exposes `SMOOTH TURN PAN` as a visible toggle row. `menu_hit_launch_direct_click_m12` and `m12_startup_menu` CTest-gate hit-testing, click cycling, TOML persistence, reload, and config-file receipt (`dm1_v2_smooth_turn_pan_enabled = 1`). `dm1_v2_settings_pc34` gates settings-file handoff, and `dm1_v2_camera_turn_edge_cases_pc34` gates the V2-only presentation pan offsets. V1 source-snap remains preserved because the saved flag does not change the original graphics mode. |

### G2. Touch / controller

| Gap | Status |
|---|---|
| Gesture navigation for runtime movement/turning | OPEN-LARGE |
| UI scaling + touch-target audit across launcher/game views | OPEN-LARGE |

### G3. Accessibility

| Gap | Status |
|---|---|
| Screen reader integration | FIXED — 2026-06-27: `firestaff_m12_launcher_screen_reader_manifest_probe` now PASS 69/69 (up from 40/40) + `firestaff_accessibility_manifest_unit` PASS 39/39. Launcher state manifest now covers main view (game cards), settings (tabs + rows), missing-data / status popup, plus the four cell-by-cell extras views — bestiary, item encyclopedia, screenshot gallery, and museum of lore — with stable element IDs (`GAME_CARD_DM1..THERON`, `MENU_SETTINGS`, `MENU_MUSEUM`, `TAB_*`, `ROW_*`, `POPUP_*`, `BESTIARY_CAT_*` / `BESTIARY_ROW_*`, `ITEM_CAT_*` / `ITEM_ROW_*`, `SCREENSHOT_ROW_*`, `MUSEUM_CATEGORY_*` / `MUSEUM_BULLET_*`). New element types in `firestaff_accessibility.h` (`FS_AX_CATEGORY_TAB`, `FS_AX_BESTIARY_ROW`, `FS_AX_ITEM_ENCYCLOPEDIA_ROW`, `FS_AX_SCREENSHOT_THUMB`, `FS_AX_MUSEUM_CATEGORY`, `FS_AX_MUSEUM_BULLET`); new private emit functions in `menu_startup_a11y_m12.c` (`emit_bestiary_view` / `emit_item_encyclopedia_view` / `emit_screenshot_gallery_view` / `emit_museum_view`); new public getters in `include/menu_startup_m12.h` (`M12_Museum_GetCategoryTitle` / `M12_Museum_GetCategoryPageCount` / `M12_Museum_GetBullet`) so the museum's private static data is reachable without leaking the table. Probe subtests H/I/J/K cover bestiary category tabs + per-creature rows with `selected` value, item-encyclopedia category tabs + per-item rows with a per-category leak check, screenshot-gallery thumbnails with filename label + `selected` value, and museum sections + page-driven bullet content (page 0 surfaces `1987 FTL GAMES`, page 2 surfaces `PRESERVATION NOTES` for the DUNGEON MASTER section). M11 game-view manifest already covers gameplay / dialog / entrance states. Still open: manual-docs / changelog / data-validator / audio-settings / accessibility / theme / save-browser / input-remap / custom-dungeon / campaign / spell-reference / map-viewer / touch-layout / presentation-preview views (still fall through to main-view emission as a navigation anchor). |
| High-contrast launcher remap | FIXED |
| In-game overlay coverage | FIXED — 2026-06-27: `m11_high_contrast_overlay_pc34_compat` module exposes the M11 chrome-remap gate (`SetActive/IsActive/RemapPresentedColor/ApplyActiveRGBA/GetManifest`); `m12_apply_loaded_config` in `src/ui/menu_startup_m12.c` pushes `config.highContrast` into `M11_HighContrast_SetActive()` next to the existing `M11_UIScale_SetPercent` push. `m11_draw_text` and `m11_draw_text_original` route through a `m11_chrome_remap_style()` helper so HUD/dialog/log/rune/hit-flash/death-overlay text picks up the restricted palette when the gate is on. Raw 320x200 dungeon-viewport pixels + HUD panel C040/C017 blits are deliberately preserved bit-exact (they go through separate bitmap-blit paths, never through `m11_draw_text`). CTest `m11_high_contrast_overlay_pc34_compat` PASS 369/369 (default-off identity for V1 fidelity, restricted-palette remap when on, region apply with `excludeMask` viewport fence, manifest string contract, launcher / game palette parity). Headless probe `firestaff_m11_high_contrast_overlay_runtime_probe` PASS 14/14 (launcher→M11→viewport-fence state machine). Labels `tier2;m11;accessibility;high-contrast`. Source-lock: none — pure accessibility glue that mirrors the existing `m12_presented_color()` contract in `src/ui/menu_startup_m12.c`; ReDMCSB does not cover high-contrast palette remap. |
| Launcher fontScale affects M12 text | FIXED |
| In-game overlays + UI-fit coverage | FIXED — 2026-06-22: `test_m11_ui_scale` CTest-gated and PASS 1/1. Pins the M11 UI scale global state contract: default 100% stays bit-identical to V1, NormalizePercent snaps any input into {100, 150, 200}, Apply is integer nearest ((value * percent + 50) / 100). Source-locked against include/ui_scale_m11.h. Label tier2;m11;ui-scale;accessibility. Strengthened 2026-06-27 by `firestaff_dm1_v1_dialog_choice_font_scale_fit_probe` (PASS 81/81, label `tier2;m11;dm1;dialog;font;scale;ui-fit;accessibility`) — pins the in-game dialog choice zone geometry (C462/C463/C466/C467 in ReDMCSB layout-696), the M11_Font_DrawString advance per scale step (8/16/24 px per char), and the layout-level overflow that the missing-clip bug shape would hide at scale 2/3. Disjoint from `test_m11_ui_scale` (percent snap + Apply math only). |

---

## H. Cross-spiel prioritized work order

For the next sprint (post v2.9.1, pre v2.10.0), I propose this
order:

### Tier 1 (BLOCKED-DATA — surface via tooling, not code)

1. **Document real-data acquisition checklist**: which Atari ST /
   Amiga / SNES / MegaCD / Saturn / PC Engine binaries need to be
   sourced per game, with hashes that will gate future runs of
   `compare_to_greatstone.py`. — DONE (`docs/DATA_ACQUISITION_CHECKLIST.md`
   with per-game ✅/🟡/🔴/⚪ status matrix; generated by
   `python3 tools/asset-validate/compare_md5_to_sha256.py` — current
   coverage: 148/148 registry entries present + hash-correct).
2. **Add `compare_to_greatstone.py` summary mode** that prints a
   per-game "data gap" view (which files in VERIFIED_HASHES.md are
   not on disk). — DONE (new `--summary` mode in
   `tools/asset-validate/compare_to_greatstone.py`: prints
   per-game TOTAL/FOUND/MISS table from any data root; default to
   `~/.firestaff/data`). Run: `python3 tools/asset-validate/compare_to_greatstone.py --summary`.
   Companion tool `tools/asset-validate/data-readiness-summary.py`
   (Tier 1 #2 L2, FIXED 2026-06-22 commit `a56d79c70` → main
   `22a8caa3`) builds a per-game readiness table by combining
   `--scan-data`, the `--summary` hash table, and an opt-in
   `--boot-probe`. See L2 below for full description.
5. **Verify all `--scan-data` READY-path:er are actually
   launchable** by M11. — DONE for Tier 1 path-discovery scope
   2026-06-21: CTest `tier1_strict_boot_probe` PASS for all present
   in-scope launch paths (DM1 canonical, DM1 legacy-dos, CSB canonical,
   CSB Amiga 3.3 Meynaf FR, Theron JP canonical, Theron JP extras,
   Theron US extras), and `asset_validate_coverage_by_game` PASS. Nexus
   virtual-ISO launch remains out-of-scope here and tracked as a Tier 4
   runtime/launcher gap, not a path-discovery gap.
6. ~~**Scanner path-naming limitations**:~~ CLOSED as NO-GAP
   2026-06-20. The scanner already matches on MD5 via
   `asset_find_by_md5` and `scan_iso_by_md5` now also falls
   back to whole-file MD5 for non-ISO-9660 .bin files (Theron
   Track 02, raw Saturn tracks). Verified: `--data-dir
   ~/.firestaff/data/nexus-extras/saturn-ja` reports Nexus
   READY (`FOUND ...Track 1).bin::DM.BIN`) and `--data-dir
   ~/.firestaff/data/theron-extras/{japan,usa}` reports
   Theron READY (`FOUND ...Track 02).bin`). See
   `reference/L1_data_path_verification_2026-06-20.md`
   Resultat v2 för verifieringsdata.
3. **Mirror dmweb /community/documentation/ for offline research**
   — DONE 2026-06-20 (commit pending; 43 pages mirrored at
   `reference/dmweb-community-docs/` with INDEX.md + index.json +
   SCRAPE_LOG.md; reproduceable via `crawl.sh`). Eliminates the
   "free.fr intermittent 404" risk for 5 topic areas.
4. **Ship a reproducible game-archive extraction script** that
   pulls from `~/Downloads/` to `~/.firestaff/data/<game>-extras/`
   without overwriting canonical staging — DONE 2026-06-20 (commit
   `4b097f54`; 73 archives → 71 version directories; ~6.2 GB
   extraherat; 4 nya READY-path:er discovered by `--scan-data`).

### Tier 2 (OPEN-BOUNDED — fits one commit each)

3. CSB hidden-code skip table for Atari ST + Amiga items 558-562. — DONE
   (commit `c5897fce feat: extend CSB hidden-code skip table to 12 entries
   (CSB Atari + Amiga specific)` + earlier `a8033a47 feat: extend CSB
   hidden-code skip table to 12 entries`). 12 entries total: 4 Atari/Amiga
   executable rows (558-562), 2 kid dungeon rows (135-138), 3 CSB Atari
   ST (21/538/548), 3 CSB Amiga (21/676/686). Source-locked against
   Meynaf disassembly + 4 COD1/COD2/COD3/COD4 container formats.
4. LZW decoder for Atari ST GRAPHICS.DAT (only Atari ST uses LZW).
   — DONE for the decoder: `m11_gfx_lzw_decompress` has a contract-only
   round-trip test (`test_dm1_lzw_round_trip.c`, `dm1_lzw_round_trip`
   PASS 1/1 on 2026-06-21, pass852). DMWeb's DM Atari ST edition page
   now documents the expected original media set and per-version split
   (two English 1.0 builds, English 1.1/1.2, German 1.2, French 1.3);
   real Atari ST asset handoff still BLOCKED-DATA.
5. PAK container decoder for Atari ST START.PAK. — DONE (commit 3ee479de);
   real-media proof should keep DMWeb's cracked ST/MSA, hard-disk, and
   RamDisk hack derivatives separate from canonical STX originals.
6. CMP portrait loader for CSB utility disk. — DONE (commit 532c8250);
   `firestaff_cmp_decode_unit` + `csb_v1_cmp_import_pc34` PASS 2/2 on
   2026-06-21. Runtime champion portrait source-selection remains tracked
   separately under A3, not as a CMP format decoder gap.
7. Harmonize MD5 vs SHA256 in `asset_find_by_hash.c` (or add
   wrapper). — DONE (commit 5988b620, see docs/MD5_SHA256_HARMONIZATION.md)
8. `_G2157_` linker fix (add `image_frontend_pc34.c` to
   firestaff_m10 source list). — DONE (commit 3588798f, added
   `image_backend_pc34_compat_globals.c` instead which provides the
   same symbols without dragging in the legacy frontend)
9. Lefthook-in-CI install step. — DONE (`.github/workflows/verify.yml:227-253` installs Go + lefthook via `go install github.com/evilmartians/lefthook@latest`, then runs `lefthook run ci` for po_layout + hash_harmonization as CI sanity gate).
10. **`--data-dir` override in `m12_build_search_roots`** — DONE
    2026-06-20 (commit `6a7eccdc`). Explicit `--data-dir` no longer
    silently falls back to `~/.firestaff/data`; runtime
    dataDir-override also skipped when request is explicit. 5/5
    asset-status tests pass; tested with
    `--data-dir ~/Downloads --scan-data` → reports
    `Data dir: /Users/bosse/Downloads` and scans only that
    directory.

### Tier 3 (OPEN-LARGE — separate milestones)

10. FTL container decoder (Amiga, X68000, MegaCD). — PARTIAL: generic header/hunk/checksum parser is CTest-gated, and the bounded HUNK_DATA area_1 zero-run decompression (greatstone d_ftl.html "Note 7") is now CTest-gated via `firestaff_ftl_hunk_data_zero_run_unit`. **2026-06-25 X68000 <-> FTL handoff added:** `firestaff_x68k_ftl_handoff_unit` PASS 5/5 cross-module test verifies the FTL-declared `data_area1_memory_size` fits inside the DMWeb 1232 KB 2DHD disk; companion `firestaff_x68k_media_classify_unit` PASS 12/12 covers the DMWeb X68000 geometry constants and copy-protection-sentinel detection. HUNK_CODE 0x5223 decompression/checksum verification, mapfile item extraction, real FTL corpus verification, and runtime loading remain.
11. CSBWin custom resource path (csbgraphics.dat + dmsave).
12. DM2 advanced CCM + projectile-list drain.
13. DM2 per-cell modern-art swap (T560/T600).
14. Nexus per-cell modern-art swap (Saturn draw pipeline).
15. Theron V2 phases 3-7 (UI overlays, lighting, smooth movement,
    touch, verification).
16. CSB V1 graphics ornament blits (F0108, F0115, F0111,
    CustomBackgrounds).

### Tier 4 (per-game polish, partially started)

17. CSB mechanics parity (combat, dungeon, champion, inventory).
18. DM2 mechanics (shops, NPCs, triggers, timeline). — Shop-economy
    determinism probe (`firestaff_dm2_v1_shop_economy_determinism_probe`,
    ctest `dm2_v1_shop_economy_determinism_probe`, commit `a89c257f`):
    19/19 PASS covering reset/enter/buy/sell/leave state transitions,
    party-state hash preservation across leave, 50-repetition
    determinism, wrong-shop rejection, insufficient-gold path.
    Source-locked per skproject/SKULLWIN/c_shop.cpp transaction pricing
    + skproject/SKWinGlobal.h:42 (NUM_NPCS=4). Existing coverage
    already includes `firestaff_dm2_v1_shop_probe` (built-in shop
    catalog + NPC dialog), `firestaff_dm2_v1_trigger_probe` (8
    triggers + 4 kinds + 6 targets), and `firestaff_dm2_v1_timeline_probe`
    for the three other DM2 mechanics domains.
19. Nexus runtime/probe coverage beyond compile/save-load.
20. Theron cross-slot import/export + cross-route evidence. — Dungeon-progression
    determinism probe (`firestaff_theron_v1_dungeon_progression_determinism_probe`,
    ctest `theron_v1_dungeon_progression_determinism_probe`, commit
    `8a43fcf8`): 20/20 PASS covering the 7-dungeon progression state
    machine (THQUEST.ASM T080 between-dungeon save/load), init / advance /
    quest-complete transitions, NULL-safety, and 50-repetition
    determinism. Source-locked per ReDMCSB GROUP.C analogue + THQUEST.ASM
    T080. Between-dungeon save/load itself is already covered by
    `tests/test_theron_v1_save_header_rejection.c` + the existing
    Theron V1 mechanics probe (10 tests / 5 probes already green for
    the V1 gameplay loop).

### Tier 5 (i18n follow-up)

21. Native-vs-fallback separation in `validate_po_layout.sh`. — DONE:
    validator now reports `nonblank` and `native` coverage separately,
    with fallback-only catalogs marked `FALL` without failing the
    structural CI gate.
22. Fill DM1/nexus/csb/theron/firestaff missing native translations
    with native speakers (out of scope for AI).

### Tier 6 (launcher/accessibility)

23. Save export/import, session timer, manual launcher.
24. Touch gesture navigation.
25. Screen reader integration. — FIXED 2026-06-27: launcher-state manifest now covers main view / settings / missing-data popup plus the four cell-by-cell extras views (bestiary, item encyclopedia, screenshot gallery, museum of lore) with stable element IDs (probe PASS 69/69). Remaining follow-up is the 14 still-fall-through views (manual-docs / changelog / data-validator / audio-settings / accessibility / theme / save-browser / input-remap / custom-dungeon / campaign / spell-reference / map-viewer / touch-layout / presentation-preview).

---

## I. Sources surveyed (this round)

| Source | Pages reviewed | Used for |
|---|---|---|
| dmweb.free.fr (Game Pages) | 5 (DM, CSB, DM2, TQ, Nexus) | per-game history, awards, ports, and DM overview version-comparison receipts |
| dmweb.free.fr (File formats) | 3 (Data Files, Animations, Animation Script) | IMG/IMG5/ANIMATE.SCR specs |
| dmweb.free.fr (Clones) | 3 (CSBwin, SKWIN, Return to Chaos) | clone source references |
| dmweb.free.fr (Custom dungeons) | 1 (g_csb.html) | 60+ custom dungeon index |
| dmweb.free.fr (FAQ) | 1 index | category map only — Drupal URLs 404 |
| dmweb.free.fr (ReDMCSB) | 0 — page not fetchable | fallback to local ReDMCSB |
| dmweb.free.fr (Community documentation) | **43 (mirrored locally at `reference/dmweb-community-docs/`)** | copy protection, file formats (animation script, animations, data files, dungeon files, DM2 data files, DM2 music triggers, hint/oracle, layout coordinates, portrait files, saved-game files), Nexus file formats (DGN, MNS, item.ibs), DM+CSB mechanics (actions, attacks, creature generators, items, skills, GRAPHICS.DAT hidden code + items 558–562), miscellaneous (Atari ST history, PC, SNES, FTL sound adapter, game versions) |
| greatstone (Tools) | 5 (Overview, Product, Screenshots, Download, Tutorial) | SCK extraction model, bundled signature/mapfile databases, tested corpus, file/item format inventory, CLI/GUI extraction commands, signed/compressed dungeon handling, mapfile override flow, external palette/string resources, animation viewing, Atari PAK/dungeon compression helpers, SCKExtraPack web-output scripts for dungeon, FTL, graphics.dat, single-file, and HTC outputs, screenshot-backed smoke references for GUI flows and representative CSB/DM/SNES/DM2 extracted `db_data` images, plus downloadable SCK/SCKExtraPack release provenance from `1.5.1@20110502.0` back to `0.5@20051123.0` |
| greatstone (Technical doc) | 4 (Mapfile, FTL, PAK, Items + IMG5) | all critical format specs |
| greatstone (Games) | 4 (DM, CSB, DM2, Custom) | per-version file lists |
| greatstone (Articles) | 3 (Mac QuickTime, SNES multi-palettes, dungeon XML/HTML) | DM2 Macintosh `MooV`/resource-fork and QuickTime flattening workflow, SNES `PALSEL`/`TILESBYITEM`/`PALSEL_INCR` palette-selection mapfile metadata, and SCK dungeon.dat-to-XML/HTML comparison/export flow for DM/CSB with limited DM2 support |
| Firestaff existing docs | FINAL_GAPS, FINAL_CSB_GAPS, PLATFORM_MATRIX, DMWEB_REFERENCE, csb_gap_*.md, dm1_gap_*.md | per-game gap detail |

**Total new pages reviewed this session: 66** (23 prior + 43 from
the dmweb community/documentation mirror).

---

## J. Update cadence

This document should be regenerated after every milestone commit.
Each major section (B-J) is regenerated from its per-domain
source-of-truth:

- A → `po/validate_po_layout.sh` + `tools/asset-validate/compare_to_greatstone.py --list`
- B → `docs/FINAL_GAPS.md` + `docs/dm1_gap_*.md` + `docs/ai_*.md`
- C → `docs/FINAL_CSB_GAPS.md` + `docs/csb_gap_*.md`
- D → `docs/NEXUS_PLAN.md` (DM2 sections) + `src/dm2/` headers
- E → `docs/NEXUS_FILE_CLASSIFICATION.md` + `src/nexus/` headers
- F → Theron TODO sections + `src/theron/` headers
- G → `TODO.md` cross-cutting sections
- H → manual prioritization after the above regenerates

Avoid duplicating content; this doc is an index, not a source.

---

## K. Session delta — 2026-06-20 (post v2.9.1)

What changed in this session that affects the gap list above:

### Data staging

- **73 game archives extracted** from `~/Downloads/` to
  `~/.firestaff/data/` via `reference/extract-game-archives.sh`.
  - 71 new version-staging directories under
    `<game>-extras/<version>/` (~6.2 GB extraherat)
  - 4,232 extraherade filer utöver befintlig canonical staging
  - Inga befintliga canonical-filer (dm1/, csb/, dm2/, nexus/, theron/)
    skrivna över

### New READY path:er (utöver canonical)

`./build/firestaff --scan-data` hittar nu alternativa
match-path:er för fyra av fem spel:

| Spel | Ny match-path |
|---|---|
| DM1 | `dm1-extras/legacy-dos/DungeonMasterPC34/DATA/GRAPHICS.DAT` |
| CSB | `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/Graphics.DAT` (Amiga 3.3 French hackad av Meynaf) |
| DM2 | (oförändrad, canonical `dm2/` vinner) |
| Nexus | `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` |
| Theron | `theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin` |

### Kanoniska hashar (VERIFIED_HASHES.md)

`tools/asset-validate/compare_to_greatstone.py ~/.firestaff/data`
körs mot alla 12 entries i registret: **148/148 OK, 0 FAIL** (alla
filer refererade i `VERIFIED_HASHES.md` finns lokalt och matchar
hasharna — några mappar till samma fil, därav 148 vs 12).

### Documentation mirror

- **43 sidor av dmweb.free.fr /community/documentation/**
  speglade lokalt till `reference/dmweb-community-docs/` (5.5 MB)
  - `INDEX.md` (19 KB) — mänskligt läsbar innehållsförteckning
  - `index.json` (27 KB) — maskinläsbar
  - `crawl.sh` — reproducibelt crawl-skript (curl + 1.2s rate limit)
  - Täcker 5 ämnesområden (copy protection, DM+CSB mechanics,
    Nexus file formats, file formats, miscellaneous)

### Bug fix: CLI --data-dir

- `src/shared/asset_status_m12.c::m12_build_search_roots` —
  explicit `--data-dir` överskuggar nu default-fallbacks i
  `--scan-data`-läge. Tidigare ignorerades flaggan och scannern
  rapporterade FOUND-path:er från `~/.firestaff/data/` även när
  användaren bad om en annan rot.
- `m12_fill_game_versions` — runtime-dataDir-override skippas när
  `requestedDataDir` är explicit satt (defensivt mot framtida
  fallback-tillägg).
- `tests/test_asset_status_scan_metrics.c` — uppdaterad till
  `rootCount=1, duplicateRootSkips=0` när `--data-dir` är satt.
- Committat: `6a7eccdc`.

### Test coverage (smoke runs)

- 40/40 DM1 V1 chest/item/weight/recompute-tester PASS
- 23/23 Phase A invariants PASS
- 12/12 kanoniska hashar matchar (0 fail)

### Påverkan på gap-status

Markerade i docen ovan som **EXTRACTED** (nya rader i C2, D3, E1,
F1) — tidigare `BLOCKED-DATA`. Fyra markerade **EXTRACTED +
VERIFIED** eftersom de nu också matchar en kanonisk hash. Uppdatering
2026-06-21: Theron JP/US Track 02 är dessutom launch-testad via
`tier1_strict_boot_probe`.

| Spel | Version | Status |
|---|---|---|
| DM1 | PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED + LAUNCH-TESTED; DMWeb PC page pins the USA EN and Europe EN/FR/GE 3.4 edition boundary plus VGA/EGA, PC ending, entrance music, and DOS input receipts |
| CSB | Amiga 3.3 (Meynaf FR hack) | EXTRACTED + VERIFIED |
| Nexus | Saturn JA (Track 1) | EXTRACTED + VERIFIED |
| Theron | JP/US Track 02 | EXTRACTED + VERIFIED + LAUNCH-TESTED |

Dessa kan nu användas som real-asset testkällor utöver den
befintliga canonical-staging som finns under `dm1/`, `csb/`, etc.

---

## L. Follow-up — concrete next-session tasks

Listan nedan är prioriterade mindre tasks som följer direkt av
sessionens leveranser. Varje punkt tar < 1 dag och kräver ingen
ny design.

### L1. Verifiera alternativa READY-path:er bootar

Status 2026-06-21: DONE för Tier 1 path-discovery scope via
`tier1_strict_boot_probe` (alla närvarande in-scope launch paths). CSB
canonical och CSB Amiga 3.3 Meynaf FR skriver nu `CSB READY`;
Nexus virtual-ISO launch ligger kvar som separat Tier 4
runtime/launcher-gap.

Den ursprungliga källan Tier 1 #5: Kör mot varje EXTRACTED + VERIFIED path
och bekräfta att M11 faktiskt startar spelet, inte bara att
scannern hittar hasharna.

```bash
for spec in \
  "dm1   ~/.firestaff/data/dm1-extras/legacy-dos" \
  "csb   ~/.firestaff/data/csb-extras/legacy-amiga-dms" \
  "nexus ~/.firestaff/data/nexus-extras/saturn-ja" \
  "theron ~/.firestaff/data/theron-extras/japan"
do
  set -- $spec
  game=$1
  path=$2
  echo "=== $game @ $path ==="
  SDL_VIDEODRIVER=dummy ./build/firestaff --data-dir "$path" --game $game --duration 1000 2>&1 | tail -5
done
```

Förväntat: Phase A-probe-PASS + en spel-specifik asset-load PASS
per path. Om något FAIL:ar, markera gap-status tillbaka till
PARTIAL.

**Resultat (2026-06-20):** Se
`reference/L1_data_path_verification_2026-06-20.md` för detaljer.
Kort version:

| Path | READY? | Orsak |
|---|---|---|
| DM1 legacy-dos | ✅ | Canonical `GRAPHICS.DAT`/`DUNGEON.DAT` i katalogen — matchar hashen direkt |
| CSB Amiga 3.3 Meynaf FR | ✅ | Matchar canonical CSB-hasen i `...Meynaf/DungeonMaster/Graphics.DAT` |
| Nexus Saturn JA | ⚠️ | MD5 stämmer (`d8362321...`) men filnamnet matchar inte scanner-mönstret `g_nexusArchiveNames` (`DM.BIN`, `SEGADATA.BIN`, etc.) — hittas bara i default-scan, inte via `--data-dir` |
| Theron JP Track 02 | ✅ | MD5 stämmer (`b7afb338...`); 2026-06-21 `tier1_strict_boot_probe` launch-testar JP canonical + JP extras till TQR level-load milestone; `theron_v1_runtime_screenshot_readiness` och `theron_24h_readiness` ger metadata-/hashkvitton utan att marknadsföra skärmbilder |
| DM1 PC 3.4 English 3.5" (extras) | ⚠️ | Innehåller `.raw`-filer (CTRaw emulator-format) som scanner ej mappar |

**Ny status:**
- DM1 + CSB legacy path:er är nu `EXTRACTED + VERIFIED +
  LAUNCH-TESTED` (redo för framtida tester/parity-evidence).
- Theron JP/US Track 02 path:er är `EXTRACTED + VERIFIED +
  LAUNCH-TESTED` sedan 2026-06-21: `tier1_strict_boot_probe`
  startar JP canonical, JP extras och US extras till TQR
  level-load milestone.
- Nexus container-path:er: `--data-dir <path>` HITTAR dem korrekt
  via MD5-hash-matchning (asset_find_by_md5), inte filnamn.
  Source-filenamn som `Dungeon Master Nexus (Japan) (Track 1).bin`
  accepteras direkt. Tidigare påstått problem med filnamn var FEL.

**Tier 1 #6 stängs som NO-GAP (2026-06-20)** — verifierat att
scannern matchar på MD5-hash, inte filnamn. Source-filenamn
accepteras direkt av `--data-dir`. Tier 1 #6 togs upp av L1-rapporten
men den faktiska scan-beteendet stödjer READY för alla 4 paths.
Inget alias-steg krävs. Tier 1 #6-posten i listan ovan är inaktuell
och bör rensas vid nästa watchdog refresh.

### L2. ~~Skapa `tools/data-readiness-summary.py`~~ — FIXED 2026-06-22

Tier 1 #2 --summary-mode. Skriver ut per-spel tabell:
`game / required-files-present / found-in-canonical / found-in-extras /
launch-tested`. Tar input från `compare_to_greatstone.py` + en
manifest-läsare.

Output (exempel):
```
dm1   2/2 present   2/2 canonical   1 extra (legacy-dos PC34)  NOT launch-tested
csb   2/2 present   2/2 canonical   1 extra (Amiga 3.3 Meynaf FR) NOT launch-tested
dm2   2/2 present   2/2 canonical   0 extras                       LAUNCH-TESTED
nexus 1/1 present   1/1 canonical   1 extra (Saturn JA Track 1)     NOT launch-tested
theron 1/1 present  1/1 canonical   JP+US extras Track 02           LAUNCH-TESTED
```

Wire in i CMakeLists + `ci: asset-hygiene` job. Används vid varje
`git push` för att snabbt se om något blockerar M12 launch.

**Status 2026-06-22 (commit `a56d79c70`, cherry-picked to main
as `22a8caa3`):** Shipped at
`tools/asset-validate/data-readiness-summary.py` (342 lines,
executable). Combines three sources into one human + JSON table:

1. `firestaff --scan-data` against a data root
   (`~/.firestaff/data/` by default — covers canonical + extras).
2. `compare_to_greatstone.py` SHA-256 hash-match summary against
   `VERIFIED_HASHES.md`.
3. Optional `--boot-probe` (`firestaff --game X --data-dir Y`,
   ~8s per path) — opt-in only, since it's slow.

Output: human-readable per-game table on stdout, JSON dump with
`--json`, exit code 0 iff all 5 games canonical-READY, 1 otherwise.
Verified running example captured in the commit message.

**Remaining (out of L2 scope):** not yet wired into CMakeLists
or the `ci: asset-hygiene` job. `--boot-probe` mode is opt-in
and unverified on a CI runner. A future pass should add a
CMake target + CI step that runs `--json` and posts a status
check on each push.

### L3. Utöka `extract-game-archives.sh` med verify-steg

Efter extraktion, kör `compare_to_greatstone.py` per extras-
katalog och rapportera per-version-summary. Loggas in i
`.extract-log.md` och `.extract-manifest.json`. Detta gör att
framtida körningar direkt ser vilka versioner som matchar en
kanonisk hash och vilka som bara är reference-material.

### L4. CSB Amiga 3.5 launch-barriär

Den extraherade `csb-extras/amiga-3.5-ctraw-en` innehåller CTraw
(.st/.raw/.err/.out) som scannern inte mappar till en canonical
hash (CTRaw är ej CTraw-filen själv, den är bara Amiga-emulator-
formatet). Skriv en liten helper `tools/ctraw_to_amiga_dat.py` som
packar om .raw → .DAT/.DATA-format Firestaff kan läsa. Eller
acceptera att 3.5 är oåtkomlig utan mer arbete och stryk den ur
"extraherad"-listan.

### L5. DM2 extras launch-test

Status 2026-06-21: DONE för PC extras. `dm2_v1_boot_scan_assets`
accepterar nu extracted DOS-layouten `data/graphics.dat` +
`data/dungeon.dat`, och `tier1_strict_boot_probe` kör DM2 canonical
plus `dm2-extras/dos-en`, `dm2-extras/dos-fr`, `dm2-extras/pc-fr`
och `dm2-extras/pc-de` till `DM2 READY`. Återstående DM2-versionsteg
ligger i D3: demo och icke-PC-versioner behöver separat klassning och
eventuell container-/formatnormalisering innan de kan bli
cross-version-regressionstäckning.

### L6. README-public-dokumentation-uppdatering

Per AGENTS.md: README ska vara honest, user-facing, sales-friendly
med verklig per-spel-status. Efter dagens gap-list-uppdateringar
bör README:s DM1/CSB/DM2/Nexus/Theron-tabeller uppdateras för
att reflektera att fyra av fem spel har real-asset-evidens i
både canonical- OCH extras-staging.

---

## M. Session delta — 2026-06-21 (post v3.0.0)

What changed in this session that affects the gap list above:

### DM1 V1 original-capture gap close (pass1052-1058)

- 6 B1 capture-gap pairs moved from `BLOCKED-DATA` to `PARTIAL`:
  - I34E keyboard buffer transcript (pass513 SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS)
  - Paired original viewport screenshot (pass1052 + pass1056 CTest gate)
  - Paired original wall screenshot (pass1052 + pass1054 exact 0-pixel match)
  - Paired original collision transcript (pass1055 closed-door stasis)
  - Paired original champion-panel screenshot (pass1053 candidate/resurrect)
  - Paired original creature-chain screenshot remains `BLOCKED-DATA` (level-1 target behind inert closed door) but pass1058 locks the corrected keypad mapping.

### DM1 V1 gap cascade (pass1059-1070)

- 12 B2/B3 PARTIAL rows moved to `FIXED`: portrait sensor parity, AI pathfinding, AI perception targets, AI reactions, mirror stat, C25/C26 projectile fallback, touch zones, inventory route parity, chest scroll-wheel pickup overflow, object consumable use, V2 smooth interpolation, V2 HUD rune routes, V22 material routing.
- 2 PARTIAL rows remained: chest runtime detail coverage, creature grouping/coordination (now PARTIAL after per-route audits).

### Tier 1 #5 strict boot-probe

- New `firestaff_tier1_strict_boot_probe` ctest entry runs the launcher with `--game <id> --data-dir <path> --duration 1500` under `SDL_VIDEODRIVER=dummy` for every EXTRACTED + VERIFIED path.
- All present in-scope paths PASS: DM1 canonical, DM1 legacy-dos, CSB canonical, CSB Amiga 3.3 Meynaf FR, Theron JP canonical, Theron JP extras, Theron US extras.
- Nexus (`Track 1.bin::DM.BIN` mount) remains tracked as a Tier 4 runtime/launcher gap, not a Tier 1 path-discovery gap.

### Tier 2 #4 LZW Atari ST decoder DONE

Decoder code path test-covered (`test_dm1_lzw_round_trip` 96/96 PASS, `pass852`). DMWeb's DM Atari ST edition page now documents the source-media/per-version split for the future real-asset handoff. Real Atari ST asset handoff still `BLOCKED-DATA`.

### Tier 4 determinism probes (3 new)

- **Theron V1 dungeon-progression** (`THQUEST.ASM T080`) — DONE
- **CSB V1 champion-stat** (`F0306`/`F0309`/`F0310`/`BUG0_72`) — DONE
- **Nexus V1 creature-state** (`F0209` timeline) — DONE

### Asset-status fix

`required=1` for all required-files rows. `matchAnyVersion` now propagates `matchedPath` so the missing-files popup and report show where the runtime will load the asset from, while keeping `launch_blocker` honest.

### DM1 V2 polish

- V20 filtered renderer probe
- V21 upscale renderer probe
- V22 in-place render probe (CSB + DM1 Apple Silicon + DM1 V22 modern asset)
- Side-by-side V1/V2 presentation-disabled seed gates
- V22 in-place cache wiring through `pass376` overlays

The V22 in-place drawing pipeline still uses placeholder overlay; wiring `m11_draw_dm1_*` draw passes to consume real modern art in-place remains `OPEN-LARGE` in B3.

### Documentation

- 100+ row status changes in B1-B3/C1-C4/D1-D2/E1-E2/F1-F2/G1-G2 reflecting post-pass1052-1070 reality.
- Tier 1 #5 marked DONE for path-discovery scope.
- Multiple Tier 4 entries closed (Lefthook CI, CSB CMP decoder, Atari ST PAK decoder, CSB hidden-code skip, LZW Atari ST decoder, M12 extras DM1, chest runtime detail, creature grouping, Theron extras launch-tested, Theron Track 02 launch).

### Verification

- ctest baseline 700+/700+ green (was 692/696 at v2.9.2).
- Phase A probe 23/23.
- Audio probe green.
- Strict `-Wall -Wextra -Werror` warnings-check green.
- Cross-platform determinism green.
- M10 verify green.

### Commits since v2.9.2

116 commits, summarized:

- `pass1052-1058` DM1 V1 original-capture gap close (B1 capture-gap pairs)
- `pass1059-1070` DM1 V1 gap cascade (B2/B3 PARTIAL → FIXED)
- `tier1-5` strict boot-probe per path
- `tier2-4` LZW Atari ST decoder
- `tier4-17` CSB V1 champion-stat determinism probe
- `tier4-19` Nexus V1 creature-state determinism probe
- `tier4-20` Theron V1 dungeon-progression determinism probe
- `dm1_v2_inplace_render_gate` / `dm_v20_filtered_renderer_silicon` / `dm_v21_upscale_renderer_silicon` / `dm_v22_modern_renderer_silicon` (V2 polish)
- `dm1_v2_side_by_side_seed_gates` (V1/V2 presentation-disabled)
- `m11_capture_route_state` / `dm1_v1_wall_collision_runtime_capture` / `m11_turn_viewport_orientation` (B1/B3 Firestaff-side gates)
- `m11_v22_inplace_draw_pc34` (V22 in-place cache wiring)
- `firestaff_dm1_v22_inplace_render_probe` (CSB V22 in-place render probe)
- Asset-status fix + `m12_fill_game_versions` TIER1DEBUG cleanup
- gap-list updates: B1 PARTIAL closure, B2 PARTIAL → FIXED cascade, Tier 1-4 closes
- `tools/dm1_24h_readiness.py` expansion (12/12 ctest rows in the roll-up)
- `verify_pass623_dm1_v1_input_capture_readiness_bridge.py` line-range refresh
- `tools/verify_pass352_dm1_v1_movement_route_regression_matrix.py` token/keypad aliases fix
- `pass372` rebuild target fix
- `src/dm1v2/dm1_v22_shapes.c` `-Wunused-parameter` warning fix
- `22a8caa3` (2026-06-22) — `tools/asset-validate/data-readiness-summary.py` cherry-picked to `main` from `csb-v1-hidden-skip-cmp-real-asset-2026-06-20` (commit `a56d79c70`). Closes Tier 1 #2 L2. The other 6 subagent branches' commits were audited file-for-file against `origin/main` and were subsumed by newer in-main versions; only this one had substantively new content. `dm1v1-capture-gap-close-20260620`, `csb-v1-hidden-skip-cmp-real-asset-2026-06-20`, `dm1-b3-v2-gates-20260621`, `dm1-lane-a-original-evidence-20260621`, `dm1-lane-c-gap-audit-20260621`, `dm1-lane-d-readiness-20260621`, `dm2-v1-mechanics-parity-2026-06-20`, `main-cmake-fix` — all left in place on origin as historical branches; the 15 worktrees have been removed and only `workspace-main` + the main-pass1052 worktree remain locally.
- `363bf3b9`, `cd86d520`, `b7dbcd60`, `a0592d6d`, `393d9f64` (2026-06-21, on `origin/main` post-cherry-pick) — Theron 24h readiness cascade: raw Track 02 bank anchors locked, m11 runtime command proof, runtime screenshot readiness gate, 24h readiness rollup tool, and readiness-report refresh. Closes F1 cross-route mechanics runtime evidence (CTest path) and adds the Theron 24h readiness row to the per-day PASS/FAIL list. `152c6a8a release: prepare v3.0.1` then tags the cascade as v3.0.1.

### Migration to GitHub main

`dm1v1-capture-gap-close-20260620` branch fast-forwarded into `origin/main` (52bce320 → cd24ea72, 67 commits). The branch is now redundant with main.

---

## N. Session delta — 2026-06-23

Latest `origin/main` status that affects this gap list:

- DM1 Hall of Champions C127 portrait coverage gained two narrower runtime probes on `main`: ordinal 0/DAROOU D1C rectangle ownership (`9d45d2ce`) and HALK/ordinal 1 front-north-entry rectangle placement (`9f1ca771`). These are regression gates for the already-FIXED Hall portrait rows, not a new gameplay claim.
- CSB launch-readiness verifier anchors were refreshed after `M12_StartupMenu_GetLaunchIntent()` extended to line 7513: `csb_v1_m11_runtime_capture_boundary` (`2c17264a`) and `pass547_csb_v1_runtime_readiness_backfill` (`e8c3cb39`) both pass, including pre-push and GitHub Actions on `main`.
- Focused queue refill: 100 new jobs were added from this gap list without reusing commands already recorded in the pool's seen-hash file. Distribution: 50 DM1, 30 CSB, 20 Theron. The batch is biased toward runtime/capture/readiness/viewport/save/HUD/overlay gates for the active PARTIAL/OPEN rows, not toward already-closed generic validators.
- At the time of this update, the first 15 focused DM1 jobs in that batch had completed successfully, leaving 85 focused jobs still active in the pool. Historical `.failed*` files from earlier runs remain in the pool directory and are not counted as new failures for this batch.

---

## N+. Session delta — 2026-06-25 (greatstone db_data real-data regression)

- A5 "Real-data regression tests (greatstone db_data)" row moved from OPEN-BOUNDED to PARTIAL / OPEN-BOUNDED with a CTest-gated OFFLINE/ONLINE metadata probe. `tools/verify_greatstone_db_data_paths.py` (`greatstone_db_data_paths_probe` CTest) replays a 13-row fixture (10 current `db_data/` paths + 3 obsolete `c_dm_*` / `c_csb_*` / guessed-DM2 404-regression paths) using only status, content-type/length hints, `<title>` text, title SHA-256, and `db_data/` link count. `--online --write` (or `FIRESTAFF_GREATSTONE_PROBE=1`) refreshes the evidence + fixture in one step, drops the bounded byte-range GET body after extracting metadata, and never persists copyrighted bytes. `tools/asset-validate/no_game_data_in_git.py` PASS. Evidence at `parity-evidence/verification/greatstone_db_data_paths_probe/manifest.json`; fixture at `tests/fixtures/greatstone_db_data_paths/index.json`. `docs/PLATFORM_MATRIX.md` now references the probe in the 🔵 status legend and the "See also" list.

## O. Session delta — 2026-06-25 (DM2 original-overlay scaffold)

Bounded new evidence for the DM2 original-overlay OPEN-BOUNDED row; no parity claim.

- `scripts/dosbox_dm2_original_overlay_capture.sh` — DM2 PC 1.0 EN DOSBox capture harness with deterministic DOS4GW/4DOS config (memsize=63, fixed cycles=3000, no dynamic core, svga_paradise), Swift (macOS) and xdotool (X11/Linux) route injectors, labelled shot tokens, 320x200 rawshot normalization (including 640x400 2x DOSBox downsample to 320x200), and 224x136 viewport crop at (0, 33). Refuses to inject an unvalidated DM2 route. Outputs land in `verification-screens/passH2313-dm2-original-overlays/` with deterministic naming and SHA-256 receipts.
- `tools/verify_dm2_v1_original_overlay_capture_source_lock.py` — read-only source-lock verifier against SKULLWIN (10 anchors across dm2global.h, c_gfx_main.cpp, c_gfx_main.h, c_gui_vp.cpp, c_tmouse.h, types.h, c_input.cpp, c_gui_draw.cpp). SKULLWIN auto-discovered from `~/.openclaw/data/firestaff-dm2-sources/...`, `/Volumes/Extern-disk/openclaw-data/firestaff/skproject-source/SKULLWIN`, or repo-relative fallbacks. CTest-gated as `dm2_v1_original_overlay_capture_source_lock`.
- `probes/firestaff_dm2_v1_original_overlay_capture_scaffold_probe.c` + CMakeLists wiring — data-free source-lock probe covering 7 invariants (screen size 320x200, backbuffer 224x136 = 0xe0x0x88, mouse+command queue lengths 10/10, c_evententry {b,x,y} layout, surface=64000B/backbuffer=30464B, shot-label vocabulary, route-token inventory). 7/7 PASS; CTest `dm2_v1_original_overlay_capture_scaffold_probe` PASS.
- `docs/FIRESTAFF_GAP_LIST.md` D1 row "Original-overlay proof" — status stays OPEN-BOUNDED; the bounded scaffold + verifier + probe are listed as new evidence so future sessions do not re-investigate. No FIRESTAFF_GAP_LIST status flip.

## P. Session delta — 2026-06-25 (data-dir picker / platform-matrix)

Latest status that affects the data-dir picker ↔ platform matrix contract:

- New data-free regression `firestaff_data_dir_picker_platform_matrix_probe` (ctest `data_dir_picker_platform_matrix`) added under `probes/`. It mirrors the `tier1_strict_boot_probe` sub-layouts as a synthetic data root (`dm1/`, `dm1-multilingual/`, `dm1-extras/legacy-dos/`, `csb/`, `csb-extras/legacy-amiga-dms/`, `dm2/`, `dm2-extras/{dos-en,dos-fr,pc-fr,pc-de}/`, `theron/Theron's Quest (Japan) (Track 02).iso`, `theron-extras/{japan,usa}/track02.bin`) and locks: empty-root no-availability + single explicit search root + zero duplicate-root skips; DM1/CSB/DM2/Theron availability under the documented M12 test setters (Nexus stays unavailable without a DM.BIN); per-game first-matched version determinism; `asset_find_by_md5` recursive discovery for every nested leaf; negative-discovery rejection of an unknown hash; re-scan clearing availability when the new folder has no match; and re-scan behavioral idempotence for the same root. This complements the existing `tier1_strict_boot_probe` (real-binary path) and `test_asset_status_data_dir_change_cache_invalidation` (cache invalidation) so the M12 layer under the data-dir picker stays honest across the platform-matrix rows in `docs/PLATFORM_MATRIX.md`.
- The A5 row "`--scan-data` smoke reports real READY-path:er" was updated with bounded evidence pointing at the new probe; no row in the platform matrix changed status, and no public legend wording was rewritten.
