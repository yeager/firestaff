# Nexus SH-2 startup decompilation

The retail Saturn startup dispatcher is recovered from an instrumented
Mednafen work-RAM capture, not from a guessed translation of menu labels.
The relevant capture was exported by Ghidra as SuperH big-endian SH-2 code.
Every export records the SHA-256 of its input capture.

`scripts/verify_nexus_sh2_decompilation.py` validates that a set of function
and disassembly exports share exactly one input hash, use the same language,
and contain only complete function exports.  It emits a JSON provenance
receipt and does not grant any drawing or startup admission by itself.

For the retained retail trace on TRV2, validate the dispatcher and its screen
callback together:

```sh
python3 scripts/verify_nexus_sh2_decompilation.py \
  /home/trv2/work/firestaff-nexus-decompile-20261005/event38-screen-route-output/functions.c \
  /home/trv2/work/firestaff-nexus-decompile-20261005/object-builder-followup-20261005/out/functions.c \
  --require 06026cde --require 06027020
```

That evidence establishes the following bounded facts:

* `0x06026cde` dispatches distinct modes, including mode values `2`, `4` and
  `6`, and installs a callback when the relevant runtime state requires it.
* `0x06027020` selects one of two continuation paths from a byte in the
  runtime object's 0x40-byte row.
* The dispatcher is stateful and callback-driven.  It is not evidence that
  the current host-side title, save, or champion presentation has the same
  pixels, timing or input semantics as the Saturn UI.

The next admissible implementation step is to capture the same session's
VDP1/VDP2/CRAM state and the displayed frame around this dispatcher.  Only
that join may unblock the existing real title or `MENU.BPK` render gates.

## BUP format signature check (2026-10-29)

A same-run Ymir read trace records five reads at SH-2 PC `0x06002a12`, all at
bus address `0x180001`, and 80 reads at `0x06002a2a`, at
`0x180001, 0x180003, ... 0x18001f` five times. These instruction addresses
were disassembled with the architecture-neutral Ghidra scripts
`DecompAddrs.java`, `DisasmRange.java`, and `FindRefs.java` from
[`yeager/pcrecomp`](https://github.com/yeager/pcrecomp) commit
`a85ea970f9e0640ab6546d2ca7a01fccb132973d`. The retained `RuntimeRAM`
Ghidra project used an earlier full RAM snapshot, but the code bytes at
`0x06002780..0x06002a20` match the same-run trace's `wram-hi.bin` exactly.

Ghidra shows that `FUN_0600233e` writes the ASCII text `BackUpRam Format`.
`FUN_060029ea` calls that helper and compares 16 bytes, stepping two bus bytes
at a time; `FUN_06002672` calls the routine at `0x060027a8` and `0x06002878`.
The matching trace PCs therefore identify these early BUP reads as format
signature probing. The full 10,849-row trace contains other instruction sites
and is not wholly explained by this one routine. Since the trace records
addresses and PCs but not values, this does not show the save-directory
contents, decode any BKR payload fields, or map a Continue save into runtime
state. The exact exports and trace join are in the operator-managed
`coldboot-readtrace-boot12000/pcrecomp-ghidra-analysis-20261029/` evidence
folder.

## BUP directory scan routines (2026-10-29)

The same `pcrecomp` Ghidra scripts were run against a temporary copy of the
retained `RuntimeRAM` project for the other high-frequency read PCs. The
decompiled code windows were checked byte-for-byte against the same-run Ymir
`wram-hi.bin`: `0x06003480..0x06003580` has SHA-256
`f1775352f1b30911173a0e56f94b8d855b06fcafbf756980aecff8fc5afe64ab`, and
`0x06003d30..0x06003de0` has SHA-256
`a67c5c6fe08b515b1dfaa098deee7dce089cfe3f5f159fce430637b246c93aae`.

The trace's 2,040 reads at `0x060034b0` are in `FUN_06003434`. That routine
walks fixed-stride records, checks an allocation bitmap, and compares the
record's 12-byte name field against a caller-provided string. The 2,550 reads
at `0x06003d68` are in `FUN_06003d30`, which counts set allocation-bitmap
entries. These are BUP directory/space metadata operations; they do not read
the Nexus save payload. The `0x0600289c`, `0x060028f2`, and `0x06002900`
readers also lie in `FUN_06002672`, whose observed code iterates configured
records and applies an allocation bitmap, but their exact role in the startup
flow is not established here.

This narrows the interpretation of the major trace sites: the confirmed
signature reads probe the BUP format, and the decompiled high-frequency
readers process directory metadata. No payload read or payload-to-runtime
mapping has been identified. The full follow-up Ghidra output is retained as
`decompilation-readers-followup.log` in the same external evidence folder.

## Slot-detail callback candidate (2026-10-29)

A hash-locked static pass over the authentic Japanese `DM.BIN` verifies a
companion callback pointer at `0x06025a68` to `0x06027132`. The setup code
stores it in an object field at `+0x6c`; a separate callback occupies `+0x70`.
The 342-byte callback body is SHA-256 pinned. Its selected-slot pointer cell
resolves to `0x0607952c`, it references helper `0x060232b4`, and its literals
include `" save comment %s\n"`, `" EASY"`, and `" NORMAL"`. These facts make
`FUN_06027132` a plausible slot-detail formatter.

These static anchors alone do not show that the callback runs on the retail
Load panel, that `%s` reads the BKR directory comment, or how it constructs the
displayed row. `scripts/verify_nexus_bkr_slot_detail_static.py` checks the
pointer, instruction, literal, and callback-byte anchors against the
hash-verified Japanese `DM.BIN` without copying game data into the repository.
A replayed authentic Load-panel state and forced Ghidra function analysis now
close most of that gap. In Ymir's
`--debug` trace, `FUN_06027132` runs at VDP1 frames 6 and 31 after Down/Up
selection inputs. At PC `0x060271f8` it writes the ten bytes `B01  0:06N` to
stack address `0x060ffedc` on both updates. Those bytes match the directory
comment at offset `0x90` in the authentic two-champion card, and the visible
Load row reads `B01  0:06 NORMAL`. Ghidra decompilation of this same retail
callback shows it selects a 12-byte slot record, copies the comment at
`record + 0x10 + 12 * slot`, removes a final `E` or `N`, and appends the
corresponding `" EASY"` or `" NORMAL"` label. It then passes the assembled
string through the glyph converter and the callback's text draw routine. The
runtime bytes and control flow therefore bind the displayed difficulty label
to the authentic BKR comment suffix. The traced card comment says `0:06`, and
enlarged inspection of the authentic screenshot confirms the visible row also
says `0:06`.
The static branch also maps suffix `E` to ` EASY`, but the retained authentic
runtime capture exercises only `N`; an authentic EASY card has not yet been
traced.

A same-run read trace records consumers of that exact stack buffer. The call
target `0x0604265c` is a NUL-terminated string append routine: it advances to
the end of the destination string and copies the source including its NUL.
The call target `0x06039ab8` maps ASCII character classes through glyph-index
bases and writes the resulting one- or two-byte glyph identifiers to its
destination. In this callback, that output is passed to the text draw routine.
The string append helper at `0x0604265c` adds the difficulty label after the
comment prefix.

The external evidence at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_load_slot_detail_trace_20261029/`
contains the complete selection write/read traces, Ymir logs, frame capture,
and saved replay state. The broad cold-boot write trace in that folder used a
frame window that did not include the actual Load-panel frame and must not be
used for frame-specific conclusions.

Reproduce with:

```sh
python3 scripts/verify_nexus_bkr_slot_detail_static.py \
  --dm-bin .firestaff/data/nexus/DM.BIN
```

## Continue payload transfer (2026-10-29)

A separate Ymir cold boot used the authentic two-champion BKR from
`.firestaff/data/nexus` in internal backup RAM, selected its real save row,
and accepted it. Ymir's SH-2 read trace covered `[0x180000,0x200000)` for
17,000 frames: 44,650 rows, no dropped master/slave events. At PC
`0x06003680`, the trace has exactly 20,480 byte reads, with unique monotonic
bus addresses from `0x18072d` through `0x18b1d3`. Walking the authentic BKR
directory continuation chain with Firestaff's read-only BKR parser yields the
20,480-byte `DMNEXUS__01` payload. Mapping each payload stream byte to its
Saturn bus address (`0x180001 + 2 * raw_payload_offset`) reproduces the full
observed address sequence exactly. The payload SHA-256 is
`9c969745745acc7f843148abf78f114ab56e82b39b7f01251b6407a0d9fdf4eb`.

The caller chain in the retained Ghidra project joins the read to the save
consumer: `FUN_060220c0` passes the name `DMNEXUS__01` and destination
`0x00204000` to `FUN_0600335a`; that routine validates the BUP, looks up the
named entry through `FUN_06003434`, then calls `FUN_0600357c` to transfer its
payload bytes. `FUN_060220c0` subsequently calls `FUN_06022668`, which checks
the payload checksum using `FUN_0602268a`. Thus the retail Continue path's
authentic payload transfer and checksum gate are now source-bound.

The captured final WRAM-L buffer at address `0x00204000` matches 20,476 of
the 20,480 payload bytes. A second same-route capture with SH-2 writes enabled
shows the copy routine at `0x06003682` writes the original bytes `00 00 00 cc`
at payload offsets `0x58..0x5b` on frame 12,922; PC `0x0602ec94` overwrites
those four bytes with `05 ef bf 3c` on frame 12,931. The latter instruction
stores a stack-derived value at `r2+8`; in this invocation `r2` is
`0x00204050`. Same-run disassembly of the containing function shows it first
loads 32-bit fields at offsets `+4` and `+8` from that structure and adds the
structure base to each before using them as source addresses. All four
authentic BKR payloads have `0x10` and `0xcc` at payload offsets `0x54` and
`0x58`; with the structure base at payload offset `0x50`, these resolve to
substructures at payload offsets `0x60` and `0x11c`. Thus those two fields are
source-relative offsets for this routine. The function later overwrites the
second offset with a stack-derived value after using the pointed-to source
region. This is evidence for an in-place runtime pointer/fixup transformation,
but does not identify the substructure schemas, the transformed field's later
consumer, or the complete loaded-state mapping. It does not establish a
Firestaff engine-state mapping. The caller analysis used `DecompAddrs.java`,
`DisasmRange.java`, and
`FindRefs.java` from [`yeager/pcrecomp`](https://github.com/yeager/pcrecomp)
commit `a85ea970f9e0640ab6546d2ca7a01fccb132973d`; no retail payload or disc
image is included in the repository. Full trace, WRAM captures, run log,
decompilation output, hashes, and reproduction details are in the external
evidence folder's `coldboot-continue-readtrace-20261029/` directory.

A follow-up pcrecomp/Ghidra pass decompiled `FUN_0602eb4c` and its callers.
`FindRefs.java` confirms that `FUN_0602250e` calls it through the computed
call at `0x06022570`; `FUN_0602250e` is itself reached from `FUN_060110ac` at
`0x0601113a`. The decompilation shows `FUN_0602eb4c` forming both source
pointers from the structure base and the 32-bit offsets at `+4` and `+8`, then
storing a stack-local address relative to that base back to `+8`. The same
function-code ranges from the earlier Ghidra project match this Continue run's
`wram-hi.bin` byte for byte: `FUN_0600357c` (`0x0600357c..0x060036dc`, SHA-256
`30d7c8dd756305c10d79e0fc71449bacb14f1918093b0c571513ac8a052aff88`),
`FUN_0602250e` (`0x0602250e..0x06022668`, SHA-256
`871fa46b70f316dc1e5bf0f01c2b63f4e16911141694b56e7dd22088a08b0f28`), and
`FUN_0602eb4c` (`0x0602eb4c..0x0602ef52`, SHA-256
`1365fcbbec44210570f9f57b3347b7fdfc1d2fe154cfefed78f4604addf51b86`).
This confirms the observed access and fixup instructions against the captured
retail code. It still does not name the substructures or establish the later
consumer of the stack-relative field. The follow-up decompilation and caller
reference outputs are in
`coldboot-continue-readtrace-20261029/pcrecomp-ghidra-payload-followup-20261029/`
within the operator-managed external evidence bundle.

The same-run WorkRAM snapshots verify a broader mechanical projection of the
loader. `scripts/analyze_nexus_bkr_runtime_projection.py` restores the four
bytes changed by the observed pointer fixup solely to hash the captured
20,480-byte source against the authentic `DMNEXUS__01` payload. The hash
matches. The decompiled constants give a source-record stride of `0xe0` and a
runtime-record stride of `0x13c`; the captured world context reports the same
record count as the payload. For each record, the analyzer checks 83 explicit
operations from `FUN_0602eb4c`: byte spans, byte-to-halfword conversions,
the 14-entry byte table including its `0xff` sentinel conversion, ten paired
table rows, signed-halfword extensions, and unsigned-halfword copies. This
produces 166, 249, and 332 projection checks for the authentic two-, three-,
and four-champion captures. Source offsets and destination offsets are kept
explicit in the analyzer so each check can be reviewed against the
decompilation and same-run WorkRAM dumps. These offsets remain mechanically
verified fields; they do not supply semantic names.

A further `--debug` Ymir cold boot on TRV2 traced reads and writes to the
payload buffer `[0x00204000,0x00209000)` for the authentic two-champion card.
The complete read trace contains 1,777 rows and the write trace 40,964 rows;
both report zero dropped events. The capture reached VDP1 frame 14,121. The
same-run analyzer matches 160 of 166 projection checks and identifies six
final differences at champion offsets `+0x10`, `+0x18`, and `+0x1a`; the
retained leader-index-3 retail write trace accounts for all six, leaving zero
unresolved differences in this bounded set. The observed access PCs include
payload checksum and importer routines, but do not provide semantic labels
for other save fields or prove Firestaff applies the Saturn state. The BKR
hash was unchanged before and after the run. Selected traces, memory dumps,
the analyzer output, and a SHA-256 manifest are retained outside Git in
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_authentic_bkr_continue_20261028/wram-consumer-trace-20261029/`.

One context field has a semantic cross-check beyond the champion table. The
decompilation assigns payload byte `+0x61` (the signed byte at source context
`+1`) to the runtime context's signed 32-bit field at `+0x0c`. The roster count
is captured at runtime context offset `0x80090`, placing that field at
`0x0608009c`. All four authentic captures match this projection exactly:
source/runtime indices are `0/0`, `0/0`, `0/0`, and `3/3` for roster sizes
2, 3, 4, and 4. The capture manifest identifies the last sample as
leader-index-3. The BKR probe now validates this party leader index as a roster
member. No other context or champion field semantics are inferred from this
match.

Firestaff's Saturn BKR parser has a bounded `DMNEXUS__01` payload probe. It
checks the two source-relative offsets (`0x10` and `0xcc`), the retail champion
count at payload `+0x62`, the party leader index at `+0x61`, the champion table
at `0x11c`, its `0xe0` source stride, and the captured count-dependent payload
span. The real-data test applies this probe to all four hash-pinned
retail-written images. It returns the record bounds and leader index, and
extracts three signed champion pairs. Source `+0x38/+0x3a` projects to runtime
`+0/+4`; `FUN_060361a8` bounds an increment by the gap between those values.
They are exposed as recovery current and maximum, with domain semantics left
unassigned. All present records in the four authentic saves contain `47/47`.
The leader-index-3 Ymir trace records the load writes at frame 12931 but no
later writes to this pair, so the updater behavior is established by static
decompilation rather than a captured runtime update. The probe also extracts
the signed stamina pair at source `+0x3c/+0x3e`, which the loader projects to
runtime `+0x08/+0x0c`. The retail updater `FUN_060360fc` reads
runtime `+0x08`, subtracts a caller-provided amount, routes values below one
through damage handling, and clamps against runtime `+0x0c`; this binds those
cells as current and maximum stamina. Every record in the two-, three-, and
ordinary four-champion saves contains `66/67`; the leader-index-3 save contains
`67/67`. The probe also extracts source `+0x40/+0x42`, mechanically projected
to runtime `+0x10/+0x14`. `FUN_060361a8` adjusts the first value up while below
the second and decrements it if it is above the second. All four authentic
saves contain `18/20`; the Ymir captures contain `19/20` after the retail
update. This supports only the behavioral labels bounded value and limit; it
does not identify the game's domain meaning. Other champion fields remain
opaque, and the probe does not load a champion into the Firestaff engine.

Several mechanically copied fields change after the initial loader writes.
The source `+0x40` to runtime `+0x10` field is 18 in the save and 19 in all
three final roster snapshots. In the leader-index-3 run, the trace covers this
field for three champions and records the load write at frame 12931 followed
by a write of 19 at frame 13951, PC `0x06036272`, inside `FUN_060361a8`. The
trace's address range ends before the fourth champion's `+0x10` field.

The same retail updater also changes source `+0x44` / runtime `+0x18` and
source `+0x46` / runtime `+0x1a`. In the leader-index-3 run, the loader writes
the source values `0x0674` and `0x06cf` to all four records at frame 12931.
The captured updater sequence reaches `0x066e` and `0x06cc` for the first
three records. These rows join to `FUN_060361a8` through PCs `0x060363ec` and
`0x06036462`. An expanded Ymir trace now covers the fourth record as well and
observes the same `0x0674 -> 0x0672 -> 0x0670 -> 0x066e` and
`0x06cf -> 0x06ce -> 0x06cd -> 0x06cc` sequences at runtime offsets `+0x18`
and `+0x1a`. The writes join to `FUN_060361a8`; they do not give these fields
domain names.

The leader-index-3 save also has four differences at source `+0x3c` / runtime
`+0x08`: 67 in the save and 66 in final WRAM. The all-record write trace
captures loader value 67 at frame 12931, updater writes 68 and 67 at frame
13055, then 64 and 66 at frame 13951 for all four champions. Each post-load
write joins to `FUN_060360fc`; decompilation shows it subtracting a
caller-provided value from the runtime `+8` longword and clamping it. These
traces originally explained seven of the eight stamina and bounded-value
differences; the expanded window also captures the fourth record's `+0x10`
update. They do not assign semantic names to the fields or caller values.

For the leader-index-3 capture, the expanded write trace covers all four
records and explains all 16 final-value differences across offsets `+0x08`,
`+0x10`, `+0x18`, and `+0x1a`; the analyzer reports zero unresolved
differences in this bounded set. Other roster captures verify the same 83
mechanical projections per record, but do not carry write traces for their
post-load changes. These checks still assign no domain names to `+0x18/+0x1a`,
do not decode any remaining record bytes, and do not prove that Firestaff maps
or renders the resulting state.

A read-only Ghidra 12.2 scan found another access to runtime champion row
offset `+0x1a` in `FUN_060373d0`. This routine derives its row pointer from
`PTR_DAT_06037438` and `DAT_0603741c`; both resolve to the same row base
(`0x0607fba0`) and `0x013c` stride used by `FUN_060361a8`. On one event path,
it advances the row's `+0x1a` halfword by a constant, clamps it to another
constant, and stores it while decrementing an event nibble. This confirms a
second retail event routine consumes and updates the same field, but does not
identify its game-domain meaning. `FUN_060366b8` is the direct caller of
`FUN_060361a8` and invokes it for active non-leader champions. The consumer
set and source meanings of both fields remain open. Another routine,
`FUN_060295b8`, selects the same indexed row, reads both `+0x18` and `+0x1a`,
adds `0x0400` to each, and passes them separately to a resource helper with
different two-byte descriptors. Its caller `FUN_060296d8` invokes it in one
of its mode paths. This links the fields to retail presentation work, but the
helper's coordinate system and the fields' domain meanings are still unknown.
The disassembly scan and complete decompilation are retained in the operator-
managed external evidence bundle's `field-consumer-scan-20261029/` directory.

Re-run against retained authentic captures with:

```sh
python3 scripts/analyze_nexus_bkr_runtime_projection.py \
  /path/to/coldboot-continue-readtrace-20261029/wram-hi.bin \
  /path/to/coldboot-continue-readtrace-20261029/wram-lo.bin
```

For the leader-3 update sequence, pass the all-record trace and its bundle:

```sh
python3 scripts/analyze_nexus_bkr_runtime_projection.py \
  /path/to/leader3-expanded-write-trace-20261029/wram-hi.bin \
  /path/to/leader3-expanded-write-trace-20261029/wram-lo.bin \
  --leader3-write-trace /path/to/leader3-expanded-write-trace-20261029/sh2-memory-write-trace.csv \
  --leader3-write-trace-bundle /path/to/leader3-expanded-write-trace-20261029/sh2-memory-write-trace-bundle.txt
```

The original bounded trace covers 8,619 writes with no dropped rows. A
follow-up cold boot uses the authentic four-champion leader-index-3 card and
expands the range to `[0x0607fba0,0x06080000)`. Its complete trace contains
26,053 rows with no dropped writes and records all four destination records.
After the loader starts at frame 12931, the analyzer verifies the
clear/load/update sequence at all four `+0x08` fields, all four `+0x10` fields,
and each `+0x18/+0x1a` field. It compares final WRAM to the SHA-256-pinned
authentic payload and explains all 16 observed changes; game-domain meanings
for `+0x18/+0x1a` remain unassigned. Captures, logs, trace, and SHA-256
manifest for this run are in the external evidence folder
`leader3-expanded-write-trace-20261029/`; the previous matrix artifacts remain
in `nexus_bkr_projection_matrix_20261029/`. The separate pointer fixup's
consumer and the rest of the payload schema remain open.

## Rejected startup candidate

The retained Mednafen capture from the `nexus-save-input-sweep-20261008`
session is authentic but is not a title/menu render candidate:

* SHA-256: `d5d9e190e7b3ee2812206b0c7da3277690490fb45c9498f0297d2e4f6d34d36e`
* 300 raw VDP1/VDP2 frames, with active VDP1 state throughout.
* The raw NBG1 tilemap decoder accepts sampled frames as capture-only data.
* VDP1 source joins resolve to `LEV00.DGN`, but the sequence join, camera
  transform, display origin and VDP2 composition remain unproven.

It must therefore remain excluded from title and menu admission.  It is useful
as a real-data regression input for the capture-only decoders, not as a
substitute for the missing same-session title/menu capture.
