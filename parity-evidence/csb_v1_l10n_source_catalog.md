# CSB V1 source-owned localization catalog

Firestaff's CSB localization template is generated from original media by
`firestaff_csb_source_text_dump`. The tool uses `csb_v1_boot_scan_assets()` and
`csb_v1_boot_enter_game()`, retains archive members in memory, and emits the
C699 action names, M564 object names, and reviewed DB2 presentation text
admitted by the native runtime.
It does not extract game files or fall back to compiled/synthetic labels.

Verified corpus: `Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip`.
The selected runtime reported:

- GRAPHICS identity: `61fbfd56887c94adc26888a9491c6611`
- DUNGEON identity: `6695d2acebce49f95db1d8f3a5c733de`
- 221 unique player-facing C699/M564/DB2 msgids
- valid GNU gettext POT output

The combined retail FM Towns CD ZIP is also verified without extraction to
disk. Pass `fm-towns-en` or `fm-towns-ja` as the optional second argument.
The selected-variant handoff retains CDATA/CJDATA `GRAPHICS.DAT` and
`CHTWE.EXP`/`CHTWJ.EXP` in memory after their fixed media hashes pass. M564 is
decoded from F31 GRAPHICS.DAT item 694. F31 does not compile the C699 loader:
`FMTOWNS.H` aliases the consumer to executable-owned `DYNA_BUTTONS`, bound at:

- CHTWE.EXP: file offset `0x29f50`, 289 bytes, 221 msgids including header
- CHTWJ.EXP: file offset `0x2a0ec`, 336 bytes, 219 msgids including header

F31J M564 follows `OBJECT.C`'s MEDIA689 NUL-row/byte-1 rule, rather than
mistaking Shift-JIS high bytes for older high-bit terminators. Japanese source
bytes are converted from CP932 to UTF-8 only for POT presentation. Both
templates pass `msgfmt -c`. The Atari runtime binding is documented below
and is not substituted with another platform's catalog.

The former semantic CSB catalog was removed because it had no production
callsites and contained invented lore/UI text plus a nonexistent CSB PC/DOS
release. Exact matches already translated in the DM1 domain were copied into
CSB locale catalogs as translations only; runtime domains remain isolated.
# Atari ST S21E extraction boundary

The supplied `Chaos Strikes Back.stx` is now consumed without writing any
member to disk. Its authenticated GRAPHICS.DAT
(`ebf6a57af3f27782e358c0490bfd2f2e`) is the 563-item DMCSB1 layout. Item 556
decodes to the authentic 1848-byte high-bit-terminated M564 stream and the
runtime accepts all 199 object-name rows.

Atari S20E/S21E does not have C699 in GRAPHICS.DAT. ReDMCSB `MENU.C` declares
G0490 as a 300-byte global for these media. STARTUP2.C F0750 restores its
authentic initialized bytes from `C560_GRAPHIC_GLOBAL_VARIABLES`; the selected
GRAPHICS.DAT item 560 contains the unique 44-row table at offset `0x174`. The
same STX contains `START.PAK` (97,712 bytes), whose header records 72,723
decompressed words, 143,636 text bytes and 1,516 initialized-data bytes. F0913
emits `(72,723 * 2) - 28 = 145,418` bytes. The corrected generic
`FirestaffPak_Decode` accepts and reproduces that complete body in memory,
including the initialized-data segment. The action table remains source-owned
by C560 rather than guessed from executable DATA: the catalog dumper uniquely
admits the 44-row C560/G0490 subtable and fails closed instead of borrowing
Amiga or FM Towns action strings.

`csb_v1_boot_enter_game` uses the same native Atari IMG/LZW reader for M564
and C560. The live action menu and leader-hand object-name presentation now
consume the selected ST media tables rather than the PC record-699 decoder or
a compiled DM1 string array. The real-media CTest pins `BLOCK`, `FUSE`, and a
non-empty first object name after the boot-to-runtime handoff.

# FM Towns live runtime binding

The F31 boot profile retains the selected CD's `CDATA/CJDATA/GRAPHICS.DAT`
and `CHTWE.EXP`/`CHTWJ.EXP` members in memory. Boot now decodes M564 item 694
from that retained GRAPHICS.DAT and installs the English high-bit or Japanese
Shift-JIS table according to the admitted edition. It also validates all 44
rows of the executable-owned `DYNA_BUTTONS` pool at the hash-locked F31E/F31J
offset before installing G0490. No loose-file reopen, PC record-699 fallback,
or cross-language string reuse occurs. Both real-media M11 handoff tests pin
the tables after SWITCHTW enters CHTWE/CHTWJ and before the C004 Entrance.

# Amiga live runtime binding

The supplied ZIP resolves to the authenticated A31M pair (`GRAPHICS.DAT`
`61fbfd56887c94adc26888a9491c6611`) inside its ADF. Its M564 item 694 and
C699 item 699 are decoded directly from that retained big-endian DMCSB2
member. The real M12-to-M11 test pins `BLOCK`, `CHOP`, `FUSE`, and a
non-empty first object name after APPB selects English and KAOS enters C03.
It also pins the authentic multiline `THERE IS ONLY ONE LEVEL HERE` utility
instruction decoded from that live dungeon; this reviewed player-facing entry
is promoted to the canonical CSB POT. The scanner walks native DB2 records
through ReDMCSB F0507 but emits only this independently consumed instruction.
CEDT champion records share DB2 storage and append encoded sex, attributes,
skills and statistics after the displayed name; those payloads are explicitly
excluded rather than translated as prose. The output passes `msgfmt -c`
without staging an extracted data file. The real handoff additionally loads
`po/csb.sv.po` and proves that this source text reaches the Swedish CSB-domain
final-presentation lookup while the original remains the missing-locale
fallback.

ReDMCSB `MENU.C` gives English A31E/A35E different ownership: G0490 is
compiled into C03_GAME. Those variants therefore resolve `APPB.FTL` through
the already-admitted `ZIP::ADF::GRAPHICS.DAT` context, verify the exact A31E
or A35E APPB hash, and admit only its unique complete 44-row action table.
They do not read item 699 or borrow PC, Atari, FM Towns, or DM1 strings.
