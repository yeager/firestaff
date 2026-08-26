# Game-data formats

Firestaff reads original media supplied by the player. This reference describes
the file containers and record families that the current code recognises. It is
not a compatibility promise: a successfully decoded record may still be held
behind a runtime gate until its original consumer is known.

The scanner identifies an edition by content hash. A familiar filename is not
enough, and files from different editions must not be combined. Original media,
saves and firmware stay outside the repository.

## Format status

| Term | Meaning |
|---|---|
| **Read** | The container and the stated record family are decoded from verified original bytes. |
| **Bound** | Decoded data reaches a source-owned runtime consumer. |
| **Opaque** | The bytes and their boundaries are verified, but their semantics have not been inferred. |
| **Closed** | Firestaff deliberately refuses the route until the missing original evidence is available. |

## Common media handling

Loose files, ZIP archives and supported disc-image containers can be scanned
without relying on their paths. Archive members are read directly from the
supplied container into bounded process memory; Firestaff does **not** unpack
or persist game-data members in a Firestaff-owned directory. For the DAT-based
games, `GRAPHICS.DAT` and `DUNGEON.DAT` form an edition pair; a recognised
graphics file never authorises a dungeon from another release.

## Dungeon Master and Chaos Strikes Back

DM1 and CSB share much of the original dungeon lineage, but their graphics
containers differ by platform. A renderer must select the decoder belonging to
the verified release; it must not reinterpret another platform's bytes.

| Family | Container or record form | Current use |
|---|---|---|
| PC 3.4 graphics | `GRAPHICS.DAT` **IMG3** records. The decoded pixels are packed 4-bit values, high nibble first, with row padding. | Read and bound for the PC DM1 route. CSB has no original DOS release; PC-shaped CSB data remains a reference and compatibility boundary, not CSB's default platform. |
| Amiga graphics | Direct **IMG1** records. The expansion path is nibble RLE and is independent of IMG3. | Read and bound for DM1 and the native CSB Amiga routes. |
| Atari ST graphics | **DMCSB1** catalog with 563 records. Individual records use Atari LZW; graphics records then use big-endian IMG1 data. | Read and bound for DM1. CSB uses the same native-container family for its Atari startup, HUD and supported viewport material. |
| FM Towns graphics | `GRAPHICS.DAT` has an `0x8001` envelope but its records use the native **IMG2** stream. It is not a PC IMG3 file merely because the outer marker is similar. | Read and bound for the verified DM1 and CSB FM Towns paths. |
| Dungeon data | `DUNGEON.DAT` holds maps, square data and linked thing records. Byte order is selected from the verified platform data rather than guessed from a filename. | Read and bound where the corresponding game/runtime path is admitted. |
| Amiga program media | CSB uses platform program and presentation assets such as `TITL.DAT`, `APPB.FTL`, `KAOS.FTL`, `BJELoad_R` and decoded IMG1 graphics. | Native Amiga startup, entrance and supported HUD/viewport surfaces are bound per edition. |
| Atari CSB program media | `ANIMATE.SCR`, `ANIMATE.DAT`, `ANIMATE.FTL`, `FTLCODE`, `CHAOS.FTL` and `SWITCH.DAT` are separate Atari-era assets. | Native ANIMATE-to-runtime handoff is bound; file names alone are never a substitute for the matching hashes. |
| FM Towns CSB program media | `RUN386.EXE`, `SWITCHTW.EXP`, `CHTWE.EXP`/`CHTWJ.EXP`, `UTILE.EXP`/`UTILJ.EXP`, ANM streams and CD data are a version-specific package. | The verified title, Game and supported Utility routes use the selected language package. |

### CSB saves and character media

| File family | What is established | Current boundary |
|---|---|---|
| `MINI.DAT`, `MINIF.DAT`, `MINIG.DAT` | Original campaign/bootstrap media, not a generic user-save name. | Read where a source-supported platform route uses it. Firestaff never writes campaign MINI files. |
| Atari/Amiga `CSBGAME*.DAT` and `.BAK` | Original GAMEBLOCK-derived saves with platform-specific byte order and language slot names (`CSBGAME`, `CSBGAMEF`, `CSBGAMEG`). Backup recovery follows the original canonical-slot rule. | Provenance and native backup recovery are bound. A complete byte-for-byte Amiga user-save corpus is still required before expanding Amiga write claims. |
| FM Towns `CSBGAME.DAT` / `.BAK` | F31 save image: a C5 header, five checksummed save parts, active groups, champions/party, events, timeline, four portraits and a dungeon tail. `CDATA/MINI.DAT` and `CJDATA/MINI.DAT` are bootstrap images, not user saves. | Reader, canonical-slot backup recovery and bounded F0433/F7052 write-back are fail-closed. Writing requires an already admitted canonical slot, preserves unowned source bytes, rotates `.BAK` and is read back through F0435. The supplied English candidate is rejected for its impossible map pairing; the language-matched Japanese corpus passes its F0435 runtime gate. Independent byte comparison against an original F31 write capture remains open. |
| CSBWin `csbgame*.dat` | CSBWin's save layout is source-format evidence for GAMEBLOCK body variants, timer widths and tail records. | It is not original CSB game media, a DOS edition or a Firestaff runtime route. No CSBWin data path can make CSB playable on PC/DOS. |
| Utility portraits | Amiga Utility ADF, Atari Utility MSA and FM Towns C06 portrait files are separately identified media. FM Towns `.CMP` admission checks its original header fields before use. | The verified FM Towns C06 route is exposed through the dedicated start-menu entry and `--csb-utility-disk`; it boots the selected F31 package before C06. Supported import/editor slices are bound; a full native file-picker and save transaction are not claimed. |

## Dungeon Master II: Skullkeep

| Family | Container or record form | Current use |
|---|---|---|
| `GRAPHICS.DAT` | Typed **GDAT** record graph rather than a flat sprite sheet. Records carry palettes, interface media, maps, map chips, materials, creature data and placement data. | Read and bound only through typed source consumers. Unknown records are not promoted to images. |
| `DUNGEON.DAT` | PC G1 map/record data. Real-data evidence establishes that `GenericRecord::w0` is game data, not a next-record link; DB3 and DB4 have proven extension pools. | Read and bound for the covered world/runtime slices. Other record semantics stay gated. |
| GDAT audio | A sound entry has a two-byte format header followed by unsigned 8-bit mono PCM at 6,000 Hz. | Source PCM decoding and bounded voice allocation are implemented. |
| FM Towns data | Original CD `DATA/` content, Japanese program/presentation material and platform-specific animation streams. | Identified and used only by the corresponding Towns route; a verified PC English companion is an English text companion, never a replacement disc. |
| Amiga media | Installer media plus `CD.DAT` and `SK00.MOD` through `SK09.MOD` for original map music. | Read as original media where supported; these files do not replace the required edition pair. |
| `SKSAVE` | Original save media. | Optional resume input, never a substitute for a verified new-game data pair. |

More detail on GDAT and the G1 record graph is in
[DM2 GDAT internals](wiki/DM2-GDAT-Internals.md).

## DM Nexus

Nexus is a Sega Saturn game. Preserve the complete CUE/BIN image or an
equivalent verified extraction; individual files are useful evidence but are
not a replacement for the source disc.

| Family | Container or record form | Current use |
|---|---|---|
| Disc identity | `DM.BIN` and associated Saturn data track identify the package. | Required admission marker. |
| `LEV*.DGN` | DMDF/DGN level data with bounded Structure1, Structure2 and Structure3 families. Structure3 currently supplies typed topology and fixed-point vector evidence, not an inferred draw format. | Read; only source-proven world/material paths are bound. |
| `*.MNS` | Static material resources such as `SN_FLOOR.MNS` and `SN_WALL.MNS`, with bounded texture/palette material data. | Used by the admitted static-material route. |
| `MENU.BPK` and `FACE.BIN` | **PRS3** compressed data. The decoder consumes control bits least-significant-bit first, literals and bounded window copies; `FACE.BIN` also carries 64-entry BGR555 palettes. | Byte decoding is verified. General BPK presentation remains closed until VDP1 upload, palette lane, placement and command order are captured. |
| `SNDLEV*.SAL` / `SNDLEV*.MAP` | Sound-level data. A shared SAL prefix is verified, but no complete payload or driver grammar has been established. | Opaque and closed for speculative decoding. |

See [Nexus DGN and PRS3 internals](wiki/Nexus-DGN-and-PRS3-Internals.md) and
[Nexus SAL/MAP internals](wiki/Nexus-SAL-MAP-Internals.md).

## Theron's Quest

Theron's Quest does not expose a conventional runtime filesystem on its data
track. The original program addresses CD records relative to the track base.

| Family | Container or record form | Current use |
|---|---|---|
| Track 02 | Hash-verified JP/US raw **MODE1/2352** BIN with strict CUE layout. A physical sector is 2,352 bytes and its MODE1 user data begins at offset `0x10` with 2,048 bytes. | Required source route. ISO/2048 data is not silently treated as the source-locked raw track path. |
| IPL and stage records | The verified IPL, stage-two and later record windows have bounded record, sector and destination receipts. | Transport and selected data framing are bound; unproven payloads remain opaque. |
| Level descriptors and resources | Descriptor records identify bounded resource spans and variable-width encoded payloads. | Framing and source byte decoding are available only with the matching runtime evidence; tile, palette and later semantic claims stay gated. |
| Track 19 | Separate optional audio/metadata media. | Preserved independently from Track 02 admission. |
| SRM saves | A save candidate is a gzip member with bounded header parsing, CRC32 and ISIZE checks. Firestaff-native exports use a clearly separate `FSTQPTY1` body and no-replace publication. | Unknown original bodies remain opaque and are not restored by guesswork. |

See [Theron's Quest Track 02 internals](wiki/Therons-Quest-Track02-Internals.md)
for the sector and save boundaries.

## What Firestaff does not do

Firestaff does not create game data, fill missing formats with a different
platform's assets, re-label an opaque span as a known structure, or format an
original game disk. When a required decoder, ownership rule or original
runtime consumer is missing, the relevant route stays closed.
