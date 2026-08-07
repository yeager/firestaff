# Firestaff TODO - NEXUS

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Nexus Structure1G Material Follow-up (2026-07-11)

2026-07-12 update: original `SN_FLOOR.MNS` and `SN_WALL.MNS` now feed the
runtime through their bounded top-level `TEXT` sections (`22528 + 27236` and
`16384 + 27236` bytes respectively). Their 15 BGR555 descriptors decode into
the indexed material banks without quantization. The DGN viewport accepts
them only when both concrete `SN_FLOOR.MNS` and `SN_WALL.MNS` sources have
crossed their canonical Track 1 MD5 receipts; parseable, renamed, or mixed
bytes cannot promote a static material route. Remaining Nexus material work
is full descriptor/UV semantics and Saturn capture comparison. Do not
substitute these resources with `MENU.BPK`, guessed BPK names, flat colours,
or generated art.

Structure1G declarations, their canonical Structure2 descriptor IDs, and the
only canonical Structure1B animated-floor binding are now validated across
LEV00-LEV15. The 41 LEV08 cells bind animation ID 0 through a typed
`floor -> Structure2` route. Structure2 now has a bounded
`descriptor[20]... + FFFF + opaque payload` envelope parser, but the payload
record grammar, offset base, image/palette encoding, and its exact
image-to-viewport-material bridge remain unproven. The canonical material
source is now authenticated as the hash-verified Track 1 `LEV00.DGN` through
`LEV15.DGN` entries themselves, not `MENU.BPK`, a `FLOORS/WALLS.BPK`, or a
DMDF family candidate. The Nexus host must continue to block that visible
animated-material route without fallback until the retail LEV corpus proves a
payload record grammar and host surface evidence exists. Model-face animated
textures and animation timing/flag execution are also still open.

2026-07-12 update: the available hash-verified LEV00-LEV15 corpus now gives
one bounded descriptor-to-payload correlation. Across all 16 Structure2
envelopes, all 2,944 nonzero descriptor offset fields fall inside their own
post-`FFFF` opaque span, with zero out-of-span fields. Firestaff records that
numeric local-span pattern in a read-only receipt only. It does not prove an
offset base, record boundary, palette role, image codec, texture dimensions,
or a renderable material, so animated routes remain blocked.

2026-07-12 update: a separate real-corpus dataflow probe now follows the 51
Structure1G first-image references across LEV00-LEV15 into 45 local
Structure2 descriptors. Their 95 nonzero numeric offsets all stay inside the
same descriptor envelope's opaque post-`FFFF` span (zero outside). This proves
only that bounded reference-to-window relation, not an offset base, payload
record grammar, palette/image role, decoder, animation, or render route.
2026-07-12 update: those same 51 references now also prove the exact numeric
global-to-local handoff: every original Structure1G `first_image_index` is at
least `0x14c`, subtracts to its stored local Structure2 ID, and matches that
descriptor's `image_id` (zero mismatches). This remains an index relation,
not image/palette data, a payload grammar, decoder, animation, or rendering
claim.
2026-07-12 update: all 51 stored `first_image_index` fields now also match
their original Structure1G descriptor word and the first word at that
descriptor's validated sequence offset (zero mismatches). This proves only a
raw descriptor-to-sequence dataflow relation, not instruction timing, image
bytes, palette bytes, a payload grammar, decoder, animation, or rendering.
2026-07-12 update: the same 51 original sequences contain 154 non-control
image-index instructions; all subtract from the global `0x14c` base into a
present local Structure2 descriptor (zero mismatches). This establishes only
sequence-index-to-descriptor reachability, not instruction timing, payload
bytes, palette bytes, a decoder, animation stepping, or rendering.
2026-07-12 update: all 51 raw `FFFE` control instructions in the same corpus
carry negative, instruction-aligned targets to earlier words within their own
validated Structure1G sequences (zero out-of-sequence targets). This is only
bounded original control-flow evidence, not animation timing, stepping,
payload interpretation, decoder, or rendering.
2026-07-12 update: all 51 validated original Structure1G sequence windows
reach one `FFFF` terminator and contain no unclassified instruction words
before it (154 image-index instructions and 51 backward gotos). This proves a
bounded raw control envelope only, not timing, stepping, payload semantics,
decoder, or rendering.
2026-07-12 update: the 154 raw sequence image indexes now each reach their
local Structure2 descriptor fields; together their 282 nonzero numeric
targets stay within the descriptors' opaque post-`FFFF` spans (zero outside).
This is numeric sequence-to-window dataflow only, not an offset base, record
grammar, payload bytes, palette/image role, decoder, animation, or rendering.
2026-07-12 update: those 282 sequence-referenced numeric targets are all
word-aligned; measured per original level, 232 target positions are distinct
and 50 are reused. This is only opaque-span layout evidence, not record
boundaries, field meanings, payload grammar, palette/image semantics, decoder,
animation, or rendering.

2026-07-13 update: the Structure2 receipt now retains this same word-alignment
measurement for every nonzero descriptor target. A nonzero in-span odd target
is explicitly distinguishable from the observed aligned corpus shape, but it
does not reject parsing, establish a record size, or promote source material.

2026-07-13 update: descriptor targets now retain unique/reused numeric-address
counts as opaque layout provenance. This records local target aliasing without
calling an alias an image, palette, record, shared surface, or render route.

2026-07-13 update: the same receipt distinguishes an in-span target with a
full two-byte window from one that merely reaches the final opaque byte. This
is an exact byte-boundary observation only; it proves neither a payload word
grammar nor image/palette semantics, and cannot enable drawing.

2026-07-13 update: the parser also retains a strict zero/nonzero byte count
for the already bounded post-`FFFF` span. This makes truncation or payload
replacement observable at the envelope boundary, but assigns no byte a
record, palette, image, codec, or render meaning.

2026-07-13 update: the same bounded span now retains complete raw two-byte
pair and trailing-byte counts, plus all-zero versus nonzero pair counts. This
is only composition evidence for the existing byte range; it establishes no
word grammar, byte order, record boundary, palette/image role, codec, or
render route.

The Structure1F handoff and DGN render-plan receipt now separate the six
documented direct-coordinate records (items, floor decorations, floor
sensors) from Structure1A-bound alcove/wall records without assigning the
latter a guessed cell or draw command. The engine, launcher, viewport, and M11 command handoff now consume direct
retail MNS material for static Structure1B floor, ceiling, and wall commands.
Launcher runtime, route, and host-ownership receipts retain the canonical
paired MNS source receipt and report when that static route was consumed; the
alternate BPK route remains distinct and does not inherit MNS provenance.
If that MNS pair is unavailable, the host again requires the current level's
canonical Structure2 materialization receipt before any BPK or other
non-MNS material plan can draw.
Structure2 provenance remains a gate only for declared animated-image
commands; it must not hide static MNS-backed geometry. Opaque Structure2 bytes
remain non-drawable. Remaining work is the retail animated payload grammar,
animation timing, and Saturn comparison capture.

2026-07-12 update: a separate real-corpus probe now traces all 1,006 direct
Structure1F coordinate records (items, floor decorations, floor sensors)
across LEV00-LEV15 into their typed runtime entries with zero mismatches. This
does not assign object, sensor, trigger, draw, gameplay, or rendering
semantics, and leaves Structure1A-bound alcove/wall records unresolved.
2026-07-12 update: the same corpus probe now verifies every byte field that
the runtime copies for those 1,006 direct records (item location/ID/selected
attributes; decoration offsets, model/aspect, rotation, control, extent; and
sensor model/aspect, rotation, extent, control, destination) with zero
mismatches. Uncopied source bytes remain unclassified; no gameplay, trigger,
decoder, or rendering semantics are inferred.
2026-07-12 update: the same raw-field receipt now covers all 1,749 typed
Structure1F records across all six families with zero mismatches, including
the copied Structure1A index fields of alcove/wall families. Those indexes are
still only raw bindings: no cell, trigger, object, draw, gameplay, decoder, or
rendering semantics are inferred.

The launcher/package route itself is verified: M12 availability may open the
Nexus runtime but cannot claim package readiness, and M11 consumes one
canonical full-start receipt through champion/save/dungeon handoff. Save
selection/load confirmation, keyboard ACTION, and the champion-footer pointer
start obtain their action receipt from that host-owned package. The host copies
the ownership-built DGN plan instead of evaluating the runtime action route a
second time. This does not relax the remaining Structure2 payload or BPK
material blockers.

The retail corpus is now locally available and has been consumed only by the
read-only receipt above. Do not infer payload record boundaries, offset bases,
texture/palette encoding, or a material bridge from descriptor correlation
alone. The next admissible work is an independently evidenced payload-record
grammar or Saturn executable route.

2026-07-14 update: Structure3 now retains an entry-local face-row edge
incidence receipt across the hash-verified LEV00-LEV15 corpus. It counts only
consecutive bounded vertex-index pairs, their multiplicity, and whether paired
rows traverse the same or opposite raw index direction. This does not prove
winding, manifoldness, surface continuity, transforms, culling, UVs, texture
or palette decoding, VDP1 state, or a draw command. Original Saturn execution
evidence remains required before any mesh rendering route can be promoted.

2026-07-14 update: the paired Structure3 face/normal rows now also have an
overflow-bounded fixed-point arithmetic receipt. Across the verified retail
corpus it measures exact base-edge orthogonality and one non-collinear
cross-product/normal-dot sign per face, preserving the observed mixed result
instead of inventing a winding or normal-use convention. It remains no-draw:
only an original Saturn execution trace or frame capture may establish normal
use, transforms, texture/palette decoding, culling, VDP1 ordering, or a host
mesh command.

2026-07-13 update: the LEV00-LEV15 verification gate now checks all 1,678
Structure2 descriptors and the observed 2,944 nonzero targets as aligned,
two-byte-bounded addresses inside their own opaque spans. This strengthens the
descriptor envelope only; it does not identify a payload record boundary,
offset base, palette, image, decoder, animation, or draw route.

2026-07-13 update: valid Structure1G control bytes are now insufficient on
their own. The runtime requires every declared first image and every sequence
image instruction to bind to a present local Structure2 descriptor before
host handoff can claim mesh readiness. An unbound original-data reference
blocks the whole DGN route with no fallback. This proves descriptor identity
and reachability only; it does not interpret Structure2 payload bytes, decode
pixels or palettes, or execute animation timing. The remaining admissible work
is still a retail payload grammar plus a Saturn executable/capture route.

2026-07-13 update: Structure1G's descriptor identity handoff now also rejects
an otherwise present local Structure2 descriptor when any nonzero original
target is odd, escapes the post-`FFFF` span, or lacks a complete two-byte
window there. The LEV00-LEV15 corpus satisfies this bounded envelope gate for
all observed targets; zero target fields remain structurally admissible. This
is an integrity check for the already proven envelope only, not a word grammar,
offset base, palette/image interpretation, decoder, animation, or draw route.

2026-07-13 update: canonical-hash Structure2 source receipts now consume that
same descriptor-envelope integrity gate before binding a level to any host
route. A malformed local descriptor layout can therefore no longer become a
materialization receipt merely because its containing `LEVxx.DGN` hash is
known. This remains provenance only: no payload, image, palette, PRS3,
animation, or rendering semantics are added. The next admissible work remains
an independently evidenced retail payload grammar or Saturn executable route.

## Nexus SLEV/SAL Semantic Follow-up (2026-07-11)

Level loading binds `SLEVxx.BIN`, `SNDLEVxx.SAL`, and `SNDLEVxx.MAP` to their
hash-verified canonical Track 1 identities before handing their bytes to the
script and audio runtime receipts. Across all sixteen canonical SLEV files,
the first source-evidenced task grammar is a 36-byte big-endian SH-2 entry
spine (`2fe6`, `e2ii`, `d3dd`, fixed body, `d0dd`) with two bounded in-file
PC-relative 32-bit literals in the observed `0x0020xxxx` range. Firestaff
records those header/literal fields and task-shape counts only; it creates no
rules and dispatches no task bytes. No SLEV task-body opcode, MAP event ID,
SAL sample window, or CD playback route is semantically promoted. The old
host byte-N-to-event-N route and last-duplicate MAP selection have been
removed: a MAP record is retained only as an opaque bounded record/window,
and a named host SFX request cannot select it. Remaining work requires a
source-backed Saturn dispatcher/audio-driver path and proof of literal
ownership, task-body record grammar, event dispatch, sample encoding, and
host playback. Keep unknown or merely readable bytes no-op with no fallback.

2026-07-12 update: the bounded entry receipt now classifies `e2ii` as the
SH-2 `MOV #imm,R2` setup operand, `d3dd` as `MOV.L @(disp,PC),R3`, and the
terminal `d0dd` as `MOV.L @(disp,PC),R0`. Those classifications, their raw
immediate, and their in-file literal offsets/values are carried only through
the profile receipt and hold across the local 16-file corpus. They establish
instruction provenance, not literal ownership, address semantics, or a task
dispatch route. The same receipt now retains the fixed `d3dd`/`d0dd`
instruction offsets and their raw displacement bytes, and verifies each
in-file literal slot as the SH-2 PC-relative formula result across all sixteen
files. This remains parser evidence only, not a task-body grammar or route.

2026-07-14 update: the terminal `d0dd` is retained only as the second
PC-relative load in the fixed entry spine. The receipt now separately retains
the raw `0x6ef6` word immediately after the fixed `RTS` at byte 28 across all
sixteen hash-bound SLEV entries. This corrects the former adjacency claim;
neither word receives task, target, callback, or dispatch semantics.

