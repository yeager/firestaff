# Active Top 10 Native-Parity Work Items

Last reviewed: 2026-11-01

This is the current cross-game priority order. Each item is either actionable
with the supplied original media or explicitly marked **blocked by missing
evidence**. A fixture, a generated save, a guessed decoder, or an emulator
runtime dependency is not an acceptable substitute. Firestaff must continue
to read supplied game media in memory and run it natively.

| Priority | Work item | Scope | Acceptance evidence | State |
|---:|---|---|---|---|
| 1 | Complete the same-revision Nexus composed title/menu state | Nexus Saturn | A private same-session title capture now verifies Track 1 CD FIFO → Work RAM → instruction reads → VDP2 values, plus 162 contiguous VDP1/VDP2 hardware frames with nonzero VDP1 framebuffer/VRAM and VDP2 VRAM/CRAM/register domains. It still does not bind MAPD transformation to its display consumer and has no corresponding menu, HUD or dungeon capture. | **Partly proven; consumer/menu/HUD/dungeon capture missing** |
| 2 | Bind decoded Nexus PRS3 output to a real Saturn VDP1 consumer | Nexus Saturn | The Japanese retail Track 1 proves all 162 PRS3 streams decode in RAM, with aggregate decoded-byte FNV-1a-64 `b60830cf2f601003`. In same-run authentic Magic-window directional captures, nine final-frame 8bpp VDP1 command sources join to `MENU.BPK` entries 99–106 and 144; 31 nonzero indices across those surfaces match the captured CRAM and retail PALT words in every state. Down/Left/Right change the referenced entries while Up matches the control. Full dispatch root, complete screen composition and end-to-end Firestaff rendering remain unbound. | **Partly proven; consumer presentation remains blocked** |
| 3 | Bind Theron's generated row to the original position consumer | Theron US (Drator) | The authentic Drator event receipt follows generated-row construction, publication, first reads and unlink. A 2026-10-31 cold US replay recorded 139 `$C852` fetches; a rebuilt follow-up reached `left-event` and `reentered-event`, recording 132 fetches at physical `$0DC852`. The first five 512-byte windows exactly match authentic Track 02 raw offset `$A3AD2` (SHA-256 `c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351`). The same-session trace proves writes and matching reads of row offsets 6–8 at `$1F093B-$1F093D`; PC `$45C3` later copies all ten bytes from `$2935` to `$2400`. The filtered observer reached its 1,048,576-row cap after those reads; the cold route subsequently reached reentry. The BIOS trace is explicitly truncated; its streaming verifier correctly rejected the marker with about 18 MB RSS. In the 132-fetch capture, callback-state verification passed for 412,791 rows (87 reads, 412,704 writes). The wrapper completed but rejected the full capture package because the BIOS trace was capped. The 2026-11-01 strict source-window fixture matches constructed `$CBC6` and `$45C3` readers to four and three possible raw offsets in authentic US Track 02. A later authentic cold run captured live `$CBC6` windows that match retail candidates, but live `$45C3` windows fail the exact source verifier. Its separate complete 129-dispatch sidecar records `$C852` → `$C85E` `JSR $5039` → `$5039` under MPR2 `$68`; live target bytes `44 05 86` differ from the static target candidate. The wrapper rejected this package due to source mismatch and trace caps. Three identity-checked scans found no contiguous 8 KiB page in raw US Track 02 or Track 19 matching the 102 unique MPR2 `$68` instruction windows. A cross-capture comparison also found 0/102 matches in the authentic `$23AD` output from a separate US run. This still leaves other decode or runtime-write sources open. The C852 reference patch adds a shared event sequence; use the existing all-writer `$0D0000–$0D1FFF` hook in a fresh same-session capture before making a causal write-to-fetch join. Bind the live reader/target sources, trace the work buffer downstream, and keep row bytes opaque. The separate cold CD-to-RAM game-owned handoff and `$C1FC` movement/teleporter question remain open. See [ordered call-window report](../source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md), [runtime-page source scan](../source-lock/theron-disassembly/theron-us-mpr2-68-runtime-page-source-scan-20261101.md), [capture boundary](../source-lock/theron-us-drator-c852-cold-capture-20261031.md) and [reader source candidates](../source-lock/theron-disassembly/theron-us-drator-row-reader-source-map-20261101.md). | **Runtime source mapped; generated-row semantics and full movement/loader parity remain open** |
| 4 | Obtain DM1 original runtime overlay pairs | DM1 PC 3.4 | Hash-locked original and Firestaff captures for movement, HUD and viewport states; pixel/behaviour regression. | **Blocked by missing captures** |
| 5 | Bind CSB DSA-bearing original saves | CSB Atari, Amiga, FM Towns | Provenance/hash receipts plus native timer, transaction and DSA regressions per admitted edition. | **Blocked by missing saves** |
| 6 | Build CSB representative visual/audio comparisons | CSB Atari, Amiga, FM Towns | Original HUD, viewport, title, door and audio captures with native comparison gates. Native CLI and start-menu routes pass against supplied Atari STX, Amiga A31 and FM Towns retail archives; the FM Towns check also captures the real C28 + C002/C003 closed Entrance composition in Original, Modern and Custom and rejects the broad-red regression. Private, hash-bound emulator startup captures now exist for Atari ST (Hatari), Amiga (FS-UAE A1200/Kickstart 3.0; CPU-fault-free FTL title), and a first FM Towns CSB title frame (Tsugaru framebuffer, source archive/CUE/track/ROM hashes bound in its private receipt). The earlier A500 profile is explicitly rejected by the capture tool due to its emulated CPU exception. The Atari session also has a non-silent native WAV with no emulator audio warnings. No original HUD, viewport, door or same-state comparison capture is promoted. | **Blocked by missing gameplay captures** |
| 7 | Complete DM2 retail save ownership | DM2 DOS, Amiga, FM Towns, Mac | Authentic `SKSAVE`/record-pool/relocation corpus, fail-closed parser and save/resume regression for every admitted platform. The supplied DOS archive now binds all eight original SKSAVE primary/backup members to the native corpus regression; broader cross-platform save evidence remains incomplete. | Partly actionable; corpus limited |
| 8 | Extend DM2 retail gameplay receipts | DM2 DOS, Amiga, FM Towns, Mac | Real-media dialog/input ordering, AI/drop, audio and save/resume paths captured and natively regression-tested. CTest binds the supplied DOS, Amiga and Mac retail archives to native title, startup, New Game, Amiga LZX/resource, movie, audio and input receipts; the DOS `SONGLIST.DAT` table is read directly from its retail ZIP and the same original DOS dungeon now proves native pit, stair, door and active-creature runtime transitions in RAM. A private Tsugaru framebuffer receipt now binds the supplied FM Towns CD image, ROM set and an original title-sequence frame. Broader cross-platform gameplay remains open. | Partly actionable; corpus limited |
| 9 | Prove DM1 FM Towns audio and input | DM1 FM Towns | BIN/CUE in-memory track selection, CD-audio timing receipts and FM Towns-specific input-route tests. CTest derives actual CDDA ranges, TMenu poll/initialization/TBIOS entry bytes, RLE graphics-library corpus, and menu-font raster from ZIP→CUE→BIN→ISO9660 in RAM. It now also reaches `EDM.EXP` and verifies the complete public initial input matrix (forward, backward, both turns, both strafes, action) from separate real-disc sessions; low-level TownsOS event-queue capture remains open. | Partly actionable with supplied media |
| 10 | Prove DM1 Amiga planar presentation | DM1 Amiga | Original ADF-to-native planar image decode and representative pixel receipt; no PC-byte fallback. A real nested ADF record now decodes through the explicit big-endian Amiga IMG1 path into source palette indices; composed planar/palette presentation remains open. | Partly actionable with supplied media |

## Recently closed before this queue

The current queue starts after these verified fixes, so they must not be
re-added as open work:

- Nexus `MENU.BPK` PRS3 byte decoding is bounded and regression-locked
  directly from the supplied Japanese Track 1 in RAM. Its 162 real surfaces
  are deliberately held at the no-render boundary until Saturn presentation
  ownership is captured.
- DM1 canonical and nested DOS ZIP startup routes are exercised through CLI,
  start menu and first movement.
- Cracked-only DM1 Atari archive selection fails closed instead of selecting a
  nearby sibling.
- CSB accepts supplied preservation ZIPs in memory; protected STX logical
  sector order is preserved even when payload offsets are skewed.
- CSB's supplied French preservation ZIP reaches title, runtime, first
  movement and start-menu launch through the native STX reader.
- The supplied DM2 Macintosh First Chapter demo fails closed and cannot be
  mistaken for retail media.
- Game-data archives are no longer materialized on disk by Firestaff.
- Completion evidence records only the verified native-media scope above.

## Selection rule for the next queue

When an item closes, replace it with the highest-impact item that has real
media or capture evidence available. If the next item is evidence-blocked,
keep that state visible rather than fabricating a substitute. Per-game detail
and deferred data requests remain in `TODO-dm1.md`, `TODO-csb.md`,
`TODO-dm2.md`, `TODO-nexus.md`, and `TODO-theron.md`.
