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

The same-run WorkRAM snapshots also verify a bounded part of the loader's
runtime projection. `scripts/analyze_nexus_bkr_runtime_projection.py` restores
the four bytes changed by the observed pointer fixup solely to hash the
captured 20,480-byte source against the authentic `DMNEXUS__01` payload. The
hash matches. The decompiled constants give a source-record stride of `0xe0`
and a runtime-record stride of `0x13c`; the captured world context reports two
records. Across both records, the analyzer verifies six byte-copy spans and
five signed-halfword-to-longword conversions per record between payload
offset `0x11c` and the runtime array at `0x0607fba0`. The field offsets are
checked directly against `FUN_0602eb4c` and the same-run WorkRAM dumps.

The same analyzer also verifies 33 and 44 projections for authentic
three- and four-champion saves. A four-champion save with leader index 3 has
four runtime values that differ from the corresponding loaded halfword:
source `+0x3c` is 67, while the final runtime `+0x08` value is 66. A complete
Ymir SH-2 write trace covering all four destination fields captures the same
sequence for each record: loader write 67 at frame 12931, updater writes 68
and 67 at frame 13055, then 64 and 66 at frame 13951. Every post-load write
joins to `FUN_060360fc`; the retained pcrecomp/Ghidra decompilation shows that
routine subtracting a caller-provided value from the runtime `+8` longword and
clamping it. Thus all four differences are explained by later retail writes,
not by a failed BKR transfer. This does not establish the semantic name of
that field or of the caller's values.

These checks establish only the listed mechanical projections and the traced
post-load updates. They assign no field names, do not decode the rest of each
record, and do not prove that Firestaff maps or renders the resulting state.
Re-run against retained authentic captures with:

```sh
python3 scripts/analyze_nexus_bkr_runtime_projection.py \
  /path/to/coldboot-continue-readtrace-20261029/wram-hi.bin \
  /path/to/nexus_bkr_projection_matrix_20261029/four-wram-lo.bin
```

For the leader-3 update sequence, pass the all-record trace and its bundle:

```sh
python3 scripts/analyze_nexus_bkr_runtime_projection.py \
  /path/to/nexus_bkr_projection_matrix_20261029/leader-3-all-record-wram-hi.bin \
  /path/to/nexus_bkr_projection_matrix_20261029/leader-3-all-record-wram-lo.bin \
  --leader3-write-trace /path/to/nexus_bkr_projection_matrix_20261029/leader-3-all-record-write-trace.csv \
  --leader3-write-trace-bundle /path/to/nexus_bkr_projection_matrix_20261029/leader-3-all-record-write-trace-bundle.txt
```

The trace covers 8,619 writes with no dropped rows and has 24 writes to the
four target fields. Captures, logs, trace, decompilation output, and SHA-256
manifest are in the external evidence folder
`nexus_bkr_projection_matrix_20261029/`. The separate pointer fixup's
consumer and the payload schema remain open.

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
