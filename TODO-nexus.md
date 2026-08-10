# Firestaff TODO - NEXUS

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

2026-08-10: Mednafen START-injection timing is now source-corrected to run
once at `SMPC_StartFrame` before Saturn `IODevice::UpdateInput` consumes the
port bytes. The resulting 1,200-frame authentic J-BIOS/English-disc witness
still has no semantic startup-to-menu transition: its raw bytes are
bit-identical to the prior no-menu witness. Keep MENU.BPK, FONT256, VDP1
owner, and production admission blocked; the next capture must prove a
different source-owned runtime state rather than infer it from the input hook.

2026-08-10: VDP2 NBG1 tilemap capture har korrigerad registerordning för
legacy big-endian `TVMD=0x0080` och native little-endian witness. Detta stänger
en decoder-lucka, men öppnar inte meny/HUD/viewport-produktion: source-map,
FONT256-ägare, textkodmappning och layer-komposition kräver fortsatt
byteexakt Saturn-bevis.

## SLEV/SAL/SDDRVS runtime corridor (2026-08-10)

2026-08-10: En ny samma-session-capture på extern disk
`run-codex-same-session-scsp-20260810/` innehåller 1 200 VDP1/VDP2-frame-
block samt separata main-/sound-SCSP-traces från samma Mednafen-process.
Rålayouten passerar med 1 033 observerade icke-idle VDP1-frame states. Den
source-bundna SCSP-joinen förblir korrekt blockerad: denna körning visar inte
en verifierad voice-registerföljd tillsammans med producent/event-semantik.
Detta är capture-infrastruktur och negativt runtime-bevis, inte tillstånd för
HUD, viewport eller ljuduppspelning.

`nexus_v1_scsp_runtime_join()` binder nu ihop hashbundna SLEV/SAL/MAP/SDDRVS-
identiteter med separata autentiserade main- och sound-CPU-traces samt den
source-bundna SDDRVS-disassemblyn. En partiell trace utan SCSP-voice-write i
samma observation förblir blockerad. Nästa capture behöver därför innehålla
producentkommando, sound-CPU-handler och voice-register-write i samma trace;
först därefter kan event-selector och SAL-codec analyseras. Ingen playback är
öppnad av denna bindning.

2026-08-10: En ny kallstart-witness på extern disk
`run-codex-j-coldstart-20260810/runtime-vdp12.raw` innehåller 1 200 frames från
J-BIOS 1.01 och den hashbundna English/Merged-discen. Mednafen rapporterar
`SGAREA=J` och rå-envelope-validatorn passerar. Den undersökta kedjan visar
fortsatt ingen byteexakt `MENU.BPK`, `FONT256.S2D`, `TITLE` eller DGN-ägare i
VDP1/VDP2; detta är negativt startup-bevis och får inte ersättas med en
syntetisk menyidentitet. Witnessen ligger kvar utanför repot på extern disk.

2026-08-10: Produktionsobjektet behåller nu även de autentiserade FONT256
Page-, Palette- och Attribute-orden (4096/256/242). Detta är fortfarande en
source-retention-grind: Saturns glyph-code→tile-mapping, VDP2 page/PND-
placering och textkonsument kräver fortsatt capture-witness.

Nästa capture-grind: bind en autentiserad Saturn VDP2-captures råa FONT256
Page-span till exakt samma `FONT256.S2D`-bytefönster. Page-span-joinen finns
nu tillsammans med CG- och Palette-joinen, men får inte markeras som
textkonsument förrän PND-fält, teckenkodsmappning, placering och layer-ägare
är bevisade av capture.

VDP1:s direct-colour-lane behöver fortsatt en riktig runtime-frame med DGN
encoding `28h` från samma retail-LEV och en verifierad command/VRAM-join.
Koden kan nu göra den exakta Structure2-ägarskapskontrollen och behåller
capture-only; det är inte ännu ett bevis på DGN-face-val, kamera, culling
eller full viewport-komposition.

Extern J/English `runtime-vdp12.raw` har verifierats genom direct-colour-
capture-dekodern för frames 0–9. Samma frames kunde inte bindas till
`LEV01.DGN` via mode-1-sekvensen; capture-materialet är därför inte ett
level-face-witness och får inte användas för att hävda retail viewport-paritet.

VDP1-resolvern räknar nu också exakt de Structure3-rader som äger en matchad
Structure2-bild och skickar detta som capture-receipt. Det är en starkare
bytebunden ägarrelation, men inte Saturns runtime face-selection: en draw kan
fortfarande ha flera möjliga Structure3-ägare och transform/culling är öppna.

2026-08-10: Engine-API:t kan nu läsa en vald, hashbunden MENU.BPK-PRS3-yta
som exakta indexpixlar. Detta är source-pixelåtkomst, inte PALT-färgdekodning,
VDP1-upload eller menyplacering; Saturn-konsumentgrinden är fortsatt öppen.

2026-08-09: VDP1-VRAM/CMDLINK till atomisk capture-replay-adapter är nu
implementerad och CTest-verifierad. Den kräver fortfarande en explicit
source/CLUT-resolver per draw; komplett DGN-sceneägarskap, transform, culling
och produktionskonsument är fortsatt öppna.

2026-08-09: Den autentiserade `FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1`
raw-envelope kan nu läsas i C. VDP1-VRAM, VDP1-state/COPR och VDP2-span pekas
direkt in i capture-lanen, med semantic admission fortsatt spärrad. Extern
J-resetwitness och DGN frame 760 passerar parsern; detta bevisar inte
startup→meny, DGN-face-selection eller produktionsraster.

2026-08-09: `nexus_v1_font256_vdp2_capture_join()` kräver byteidentisk
character-generator- och 256-färgs palette-span mot samma hashattesterade
FONT256.S2D. Positiv/ändrad-palette-fixture passerar. Textkod→tile, page-PND,
SLEV/TABL-ägarskap och faktisk menyplacering är fortfarande obevisade; no-draw
kvarstår.

2026-08-09: SLEV/SCSP-parsern behåller nu råbyte-offsetar för första mailbox-,
SDDRVS-handler- och SCSP-röstregisterobservation samt en strikt intra-trace-
ordningsflagga. Main-SH-2-tracen behåller första producerade kommando-offset.
Extern fransk trace passerar; separata tracefiler saknar gemensam tidsbas, så
eventägare, SAL-codec, MAP-bindning och playback är fortsatt spärrade.

2026-08-09: C-capture-lanen korrigerad till producentens verkliga VDP2-ordning
`RawRegs → VRAM → CRAM` (tidigare felaktigt pekade C på CRAM först). Ny
registerreceipt verifierar byteordning och NBG1-state; extern engelskspråkig
frame 80 läses korrekt som `TVMD=0x8000`, `BGON=0x0003`, NBG1 character mode.
Menyägare, FONT256-textkod och faktisk presentation är fortfarande spärrade.

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
`floor -> Structure2` route. DMWeb's Structure2 payload grammar is now
decoded in the authenticated engine lane: 08h MSB-first 4bpp images resolve
their 16-word palette (including zero-offset same-ID reuse), and 28h images
retain exact big-endian Saturn 15-bit words. The canonical material source is
the hash-verified Track 1 `LEV00.DGN` through `LEV15.DGN` entries themselves,
not `MENU.BPK`, a `FLOORS/WALLS.BPK`, or a DMDF family candidate. This source
decode does not authorize VDP1 upload, selector/UV semantics, animation
timing, transform, culling, or viewport drawing; those remain capture-gated.
Model-face animated textures and animation timing/flag execution are still
open.

2026-08-10: Real LEV01 engine loading now decodes every Structure2 descriptor
into a source surface (indexed 08h or exact direct-color 28h) while retaining
`animated_floor_material_route_valid = 0`. The viewport therefore cannot use
the new surfaces without the existing Saturn VDP1/CLUT/transform admission.

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


## Dungeon Master Nexus

### Nexus V1

- 🔧 2026-07-09 Nexus MENU.BPK/DGN/SLEV/SNDLEV follow-up: engine init exposes hash-resolved PRS3 decode and upload-plan receipts for `MENU.BPK`; DGN level load exposes renderer/runtime mesh-readiness and viewport render-plan receipts and hash-resolves renamed `LEV00.DGN`; SLEV runtime receipts block unsupported script dispatch without fallback rules; SNDLEV runtime receipts load real SAL/MAP bytes and block unsupported SFX decode/playback. 2026-07-10 update: Nexus now has one complete-support receipt requiring title, save, champion, dungeon/DGN host routes, Saturn timing/capture matrices, no fallback visuals, and material-validated DGN viewport rendering together. 2026-07-10 update: known Nexus DGN levels 00-15 plus SLEV00-15 and SNDLEV00-15 SAL/MAP now resolve hash-first before filename fallback, with renamed real local LEV01/SLEV00/SNDLEV00/MENU.BPK proof. 2026-07-10 update: real `MENU.BPK` PRS3 streams decode and upload as `ready-decoded`, and champion-start host routes now require the DGN commands to come from the material plan/viewport path before drawing. 2026-07-11 update: DGN Structure1B mesh refs are now budgeted alongside Structure1C collision refs and propagated into render-plan receipts; bounded 4-byte mesh descriptors are decoded and applied to DGN command quads; SLEV trigger dispatch now has a bounded receipt-gated rule-table parser while unknown real candidates still block fallback dispatch. 2026-07-11 update: SNDLEV MAP data now has a bounded event-to-sample route receipt, while SAL sample decode and real playback remain blocked. 2026-07-11 update: Structure1F descriptors now carry bounded footprint semantics through geometry, handoff, and render-plan receipts. 2026-07-11 update: real SLEV00-15 files are now profiled as SH-2 task-like streams with dispatch still blocked, including JSR, PC-relative load, immediate, branch, and literal pointer operand receipts. 2026-07-11 update: real SAL00-15 packages now emit bounded package metadata receipts, SNDLEV MAP record tables expose bounded SAL offset/size windows, first/last record windows expose checksum/nonzero/high-bit metadata, and blocked event-selected SFX calls now report the matching SAL window metadata without playback. 2026-07-11 update: SAL record windows now also expose payload-shape diagnostics (first/last nonzero relative offsets, distinct byte count, and byte-transition count) for first, last, and event-selected windows without enabling playback. 2026-07-11 update: SNDLEV MAP headers now expose checksum, nonzero byte count, distinct byte count, and transition count as bounded diagnostics before record parsing; MAP records also expose min/max/span event IDs plus unique/duplicate event counts and an explicit duplicate-event flag. Remaining work is broader real Saturn capture comparison beyond the material-route proof, decoding SLEV call targets/operands into safe dispatch rules, actual SAL payload/sample decode/playback, and confirming the Structure1F descriptor interpretation against a larger real DGN corpus.

  - 2026-07-15 update: an engine-owned route now admits one raw MAP selector only when the active level's SAL and MAP identities are hash-verified and the selector resolves uniquely to a bounded SAL window. The selector remains opaque: original Saturn event-dispatch, SAL payload decoding, SDDRVS driver ABI, and playback are still blocked pending source/capture proof.

  - 2026-07-15 update: the active engine now also admits the SLEV entry receipt only when the current level, hash-verified `SLEVxx.BIN`, VM source, and corpus-proven SH-2 header agree. It exposes bounded entry/literal facts only; original task-body dispatch, callback targets, and trigger semantics remain blocked.

  - 2026-07-15 update: the active verified SLEV route can now write an execution-capture target that pins the canonical SLEV identity, entry framing, and literal addresses and demands observed entry PC, task-body transfer, and callback-or-write evidence. It remains a producer request, not a task decoder or dispatcher.

  - 2026-07-15 update: the source-owned SLEV campaign probe can emit those no-dispatch targets for every canonical `SLEV00.BIN`--`SLEV15.BIN` from the local retail corpus. This supports offline capture planning without a Saturn BIOS, but it does not create a trace or prove task-body semantics. The remaining need is still one authentic Saturn SH-2 capture per promoted behavior.

  - 2026-07-15 update: the active hash-verified SNDLEV route can now emit a capture target for each uniquely bounded raw MAP selector across the retail corpus (106 targets across LEV00--15). Each target pins SAL, MAP, and `SDDRVS.TSK` identities plus the exact SAL window and asks for original selector-dispatch, SAL-read, and driver-output evidence. It cannot decode, map host events, or play the bank; those semantics still require authentic Saturn capture.

  - 2026-07-15 update: admitted SLEV trace evidence now reaches a separate active host-consumption receipt only after the current SLEV target is rebuilt and revalidated. Level/VM source drift is rejected without replacing prior host evidence. Consumption does not execute the observed opcode or callback/write location; semantic dispatch remains open.

  - 2026-07-15 update: host consumption now additionally requires raw Mednafen trace bytes to match the manifest's declared FNV-64 receipt. A manifest-only trace remains evidence-only and cannot reach the host route. This binds imported bytes but still does not prove opcode meanings or authorize dispatch.

  - 2026-07-15 update: raw-trace evidence now verifies that the bound capture contains the exact declared entry, task-body, and callback/write observations. This establishes occurrence only, not task-body grammar, callback ownership, or gameplay semantics; dispatch remains blocked.

  - 2026-07-15 update: the evidence receipt now also requires byte-order entry → task-body → callback/write within one raw trace, with each offset retained for audit. This is observation ordering, not execution semantics.

  - 2026-07-15 update: the same raw trace must now contain both corpus-proven PC-relative SLEV literal addresses. This verifies that both entry operands occur in capture, not what either literal owns or dispatches.

  - 2026-07-15 update: trace admission now additionally binds canonical SLEV name, task-header size, and both literal values to the active target. Any cross-level or partial-header manifest is rejected before raw evidence can be consumed.

  - 2026-07-15 host-route update: SLEV host intake now also requires the
    bound raw trace's ordered entry/task-body/callback observation and both
    literal observations. This validates capture occurrence only; the task
    body remains opaque and dispatch/callback execution stays blocked.
    Evidence retains the exact raw-trace FNV and byte count, so an older
    same-level observation cannot satisfy a changed active trace.

  - 2026-07-11 update: Nexus `runtime_screenshot_readiness` and `track1_real_screen_capture_readiness` now pass locally. The runtime gate avoids the old M12 screenshot-gallery startup timeout by using a boot-probe app receipt for Nexus launch metadata and the Nexus-owned Track 1 BMP probe for the real-data image receipt. The Track 1 probe is self-contained, no longer links `firestaff_m11`, writes deterministic 24-bit BMP receipts, and stamps a real `FONT256.S2D` glyph into the indexed framebuffer before BMP export. Remaining capture work is reviewed Saturn capture comparison and eventual public screenshot promotion, not the readiness plumbing.

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked. The M11 launcher handoff boundary (`nexus_v1_m11_launcher_handoff_boundary`) passes against local retail ISO. Real Saturn asset-path proof for the DGN material containers is now anchored by the boot profile's hash-first validation of `SN_FLOOR.MNS`/`SN_WALL.MNS`. Remaining work is the capture-blocked DGN material raster decode and broader packaged startup capture proof, not synthetic fallback rendering.

- 🔧 2026-07-14 update: DGN face/material admission now requires the exact
  launcher-reopened LEV bytes to match the authenticated canonical entry before
  raster input is accepted. Remaining work is real face/mesh/pixel decode and
  Saturn capture, not fallback rendering.

  - 2026-07-13 update: the selected retail DM.BIN V1 SH-2 route now has an importable instruction receipt for its R11 control test, bounded R12 post-increment byte read, R13/R0 byte store, and loop branch. It is not a live MENU.BPK binding or VDP1 capture. Remaining work is an original execution capture connecting one hash-verified BPK entry to those reads, its full output range, and a real VDP1 command/source range before PRS3 decoding or menu handoff can be considered.

- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates. 2026-07-22 update (Lane D, cycle 3): creature attack damage is now applied to the party leader (or first living party member) and total party death sets `game_over=1` / `game_over_reason=2 (all_dead)`. The empty-party `nexus_mechanics_party_alive()` bug is fixed (empty party is dead, not alive). The mechanics parity probe now covers the integrated tick with a synthetic scorpion-vs-party combat scenario. 2026-07-22 update (Lane D, cycle 4): champion death auto-leader promotion is implemented. `nexus_v1_champion_on_death_update_leader()` in `src/nexus/nexus_v1_champions.c` promotes the first living party member to leader when the current leader dies, matching ReDMCSB CHAMPION.C F0319 lines ~1662-1679. The mechanics tick calls it after creature-attack damage and stamina-collapse death. The mechanics parity probe now verifies non-leader death leaves leader unchanged, leader death promotes the next living member, and total party death returns no successor. 2026-07-22 update (Lane D, cycle 5): pit/chute square-event integration is implemented — stepping on a `NEXUS_SQUARE_CHUTE` now forces a level transition to `map_index + 1` via `pending_level_change`. Item usage/click-route wiring is implemented — `NEXUS_CMD_USE_ITEM` consumes the selected leader inventory slot (`use_item_slot`), applies consumables (health/mana/stamina potions, antidote, corn, water flask) and equips weapons/armor, then clears the slot and recalculates load. Source locks: DM1 MOVESENS.C F0267/F0268 (chute/pit), COMMAND.C item-use dispatch, CHAMPION.C F0309 equipment slots. The mechanics parity probe now covers both new behaviors (207/207 PASS). 2026-07-22 update (Lane D, cycle 6): mouse click-route dispatch for inventory/world objects is implemented — `nexus_click_route_dispatch()` translates inventory-slot, equipment-slot, world-square, door-square, and floor-item clicks into the same command queue used by keyboard input (`NEXUS_CMD_USE_ITEM`, turns, `NEXUS_CMD_FORWARD`, `NEXUS_CMD_INTERACT`). New `NEXUS_CMD_INTERACT` picks up floor items at the party's current square into the leader's inventory. Source locks: DM1 COMMAND.C mouse/click dispatch, CLIKMENU.C F0366 command queue, CHAMPION.C F0309 equipment slots, MOVESENS.C F0267/F0268 square interaction. The mechanics parity probe now covers click-route dispatch (218/218 PASS) and the dedicated `test_nexus_v1_click_route` regression test covers 31 checks. 2026-07-23 update (Lane D, cycle 7): pit/teleporter broader runtime coverage is implemented — `nexus_process_square_event` now reports the registered stair facing (`out_target_dir`) for stairs up/down; `nexus_mechanics_tick` processes `pending_teleport` before the step cooldown so teleporter warps are immediate, and cross-level teleporters set `pending_level_change` to the target level. New regression test `test_nexus_v1_pit_teleporter_runtime` covers chute step, chute max-level clamp, same-level/cross-level/unregistered teleporters, and stairs down/up targets (24/24 PASS). The mechanics parity probe adds Probe 12 for teleporter runtime (same-level, cross-level, unregistered) and now passes 226/226. Source locks: DM1 MOVESENS.C F0267/F0268 (teleporter/pit/stair sensors), DUNGEON.C square type dispatch, CLIKMENU.C:264-276 level-transition special cases. 2026-07-23 update (Lane D, cycle 8): stairs/exit/alarm broader runtime coverage is implemented — unregistered stairs now fall back to the adjacent level (down +1, up -1, clamped to [0,15]); registered stairs keep their exact target level/coordinates/facing; exit squares only end the game on the final level (level 15), with non-final exits treated as ordinary floor; alarm traps now alert only creatures on the current level and set a bounded 60-tick alarm timer that keeps alerted creatures chasing even when the party moves out of normal detection range. `Nexus_Creature` gains a `level` field, `Nexus_V1_CreatureManager` gains `alarm_timer`, and `nexus_v1_creature_spawn_on_level()` is added so probes/tests can place creatures on specific levels. `nexus_v1_creatures_tick()` now skips/attacks only creatures on the active level. `test_nexus_v1_pit_teleporter_runtime` expanded to 34 checks covering stairs down/up registered/unregistered and final/non-final exits. The mechanics parity probe adds Probe 14 for stairs/exit/alarm runtime and now passes 240/240. Source locks: DM1 MOVESENS.C F0267/F0268 (stairs/exit sensors), F0277 ALARM; CLIKMENU.C F0364_COMMAND_TakeStairs; ReDMCSB CHAMPION.C F0309 equipment slots. 2026-07-23 update (Lane D, cycle 9): water/fire square traversal mechanics are implemented — water squares (type 21) now block movement unless the party leader carries a Rope (item 65); fire squares (type 22) block movement unless the party leader carries a Rune of Fire (item 80). The passability gate lives in `nexus_mechanics_tick()` alongside the existing door key check; the square event layer now emits `NEXUS_EVENT_CROSS_WATER` and `NEXUS_EVENT_CROSS_FIRE`. New `NEXUS_MOVE_CROSS_WATER`, `NEXUS_MOVE_CROSS_FIRE`, `NEXUS_MOVE_BLOCKED_WATER`, `NEXUS_MOVE_BLOCKED_FIRE`, and `NEXUS_MOVE_BLOCKED_DOOR` result codes are defined in `nexus_v1_movement.h`. `test_nexus_v1_pit_teleporter_runtime` expanded to 44 checks covering water/fire blocked/crossed and square-event returns. The mechanics parity probe adds Probe 15 for water/fire square runtime and now passes 251/251. Source locks: DM1 MOVESENS.C F0267/F0268 water/fire square sensors; nexus_v1_inventory.c Rope (65), Rune of Fire (80). 2026-07-23 update (Lane D, cycle 10): real-DGN playability probe is implemented — new `firestaff_nexus_v1_mechanics_playability_probe` loads retail `LEV00.DGN` from `FIRESTAFF_NEXUS_DATA_DIR` (or `~/.firestaff/data/nexus`), verifies 64x64 Structure1B load, initializes a party on the actual starting floor square, exercises forward movement/turning on real geometry, verifies OOB/map-edge blocking, reports decoded floor/wall/door counts, and flood-fills reachable passable squares. The probe is skip-safe when the retail corpus is absent. Source locks: DMWeb DGN Structure1B format; ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C. CTest `firestaff_nexus_v1_mechanics_playability` passes 16/16 against the local Track 1 LEV00.DGN and exits 0 (skip) when data is missing. 2026-07-23 update (Lane D, cycle 11): expanded the real-DGN playability probe to all 16 retail levels (LEV00–LEV15). `firestaff_nexus_v1_mechanics_playability_probe` now loops over LEV00.DGN–LEV15.DGN, loads each through the existing Structure1B decoder, verifies 64×64 dimensions, counts floor/wall/door squares, checks OOB boundary blocking, real wall blocking, forward movement/turning on real floor, and flood-fills reachable passable squares; the probe reports 253/253 PASS against the local Track 1 corpus and remains skip-safe when data is absent. A companion CTest regression test `nexus_v1_dgn_multi_level_playability` (`tests/test_nexus_v1_dgn_multi_level_playability.c`) covers the same core checks across all 16 levels and returns 77 when no data is present. Remaining mechanics work: sound playback binding (still blocked on SAL decode), stairs/exit/alarm exact original timing/feedback, and real-data playability probes for additional square-event semantics once Structure1B wall/special-square decoding is source-locked against original Saturn evidence.

- 🔧 DMDF embedded BITMAP/palette/string runtime handoff remains open after the parser-level bounds gates. The real MNS `TEXT` descriptor and BGR555 material-bank route is now regression-covered: all 30 retail models retain matching descriptor/pixel receipts and all 815 source textures decode. The seven creature banks whose source colour cardinality exceeds the indexed 256-entry host bank now retain exact BGR555 words in a source-only direct-colour lane; they are not quantized, substituted, or admitted to the indexed viewport. VDP1 command/CLUT ownership, direct-colour display semantics, texture upload and runtime render binding remain capture-gated.

- 🔧 2026-06-28 Nexus V1 save multi-slot round-trip follow-up: new `test_nexus_v1_save_multislot_roundtrip_pc34_compat` (CTest `nexus_v1_save_multislot_roundtrip_pc34_compat`) drives 4 distinct slots (0..3) with distinct per-slot world + champion state through `nexus_v1_save_full` / `nexus_v1_load_full` and verifies party_level/x/y/dir + world_tick + per-object (type, state, x, y, level, quantity, linked_id, flags) + per-event (type, level, x, y, arg0, arg1, fired, repeat) + per-active-timer (id, kind, level, remaining_ticks, interval_ticks, flags) + transition (pending, target, spawn_x, spawn_y) + per-champion stat blobs (name, primary_class, hp, max_hp, stamina, max_stamina, mana, max_mana, str, dex, wis, vit, anti_magic, anti_fire, fighter/ninja/priest/wizard level, food, water, alive, portrait_index, wounds, attributes, inventory[30]) + party[] indices round-trip per slot, plus manager slot cache + scan() + isolation + deletion + CRC tamper rejection (one-byte flip in the data section → `NEXUS_SAVE_ERR_CRC`) + foreign-magic rejection (`NEXUS_SAVE_ERR_UNKNOWN_VARIANT` + non-empty diagnostic). Source-lock: `src/nexus/nexus_v1_save_load.c` (NEXUS_SAVE_MAGIC='FNXS', CRC-32 over champion+world data sections) + `src/nexus/nexus_v1_world.c` (party + objects + events + active timers + transition + world_tick + state_hash) + `src/nexus/nexus_v1_champions.c` (CHPN magic, 270-byte champion blob) + ReDMCSB LOADSAVE.C F0433/F0434 lineage. Same family, disjoint scope: existing slot-0/party-x test still covers the single-field gate; this new test extends coverage to 4 slots + 30+ per-slot world/champion fields + cache/scan/isolation/deletion + CRC + unknown variant. Companion source-side fixes (also shipped this pass): (a) `nexus_v1_champion_pool_serialize_size` now matches the actual `wr32`-based 24-byte header (was claiming 22 with a `version(2)` that the serialize code does not write); (b) `champion_blob_size` now counts 25 int fields per champion (was 23, which under-counted by 8 bytes/champion and silently overflowed the 24-champion pool blob in older code paths); (c) `nexus_v1_world_serialize_size` now omits the bogus 4-byte object-count prefix (the actual serialize path reads the count once from the header); (d) `nexus_v1_load_full` and `nexus_v1_load_full_from_path` now allocate buffers via the new `nexus_v1_save_max_champion_pool_size` / `nexus_v1_save_max_world_size` helpers instead of asking the destination's serialize_size (which underestimates because the destination has not been loaded yet — the prior code only worked when the saved world happened to have no objects/events/timers). Remaining save-slot work: original Saturn 8 KB memory card format reverse-engineering (Firestaff-native only today), real-asset save compatibility artifacts, and broader per-game (DM1/CSB/DM2/Theron) save interoperability.

- 🔧 2026-07-17 FONT256 first-section witness: the canonical SHA-256-attested
  Treat that observed ramp as opaque and capture-required, never as a glyph
  table or pixel layout.

  - 2026-07-19 update: all four populated SCR sections (table indices
    unchanged: an original Saturn trace or independently reviewed format
    material before any subrecord grammar, palette, glyph, or draw route.
    pixel meaning; draw routes remain blocked. CTest
    section, the preamble, and the section table. Remaining FONT256 work is

  - 2026-07-20 update: the subrecord question is now answered read-only.
    independently reviewed format material before any glyph layout,
    palette, encoding, or draw route is assigned to these structures.
    Remaining FONT256 work is unchanged: an original Saturn trace or

  - 2026-07-20 update (round 15): the ordinal-1 section (table index 2,
    independently reviewed format material before any subrecord grammar,
    glyph layout, palette, encoding, or draw route.
    `nexus_v1_font256_s2d_subrecord_grammar` (+ `_real`) PASS. Remaining

- 🔧 2026-07-17 WARNING.BIN source-only follow-up: the canonical, directly
  header values only; neither the prefix nor body is assigned CLUT, pixel,
  colour, stride, or draw semantics without further original evidence.
  semantics; those remain separate original-Saturn evidence requirements.
  and the two trailing bytes before the next descriptor. Width/height remain

  - 2026-07-17 update: Sega Saturn/32X Graphic References ST-124-R1 section
    6 now supplies the missing PP contract: a 256-word BGR555 CLUT follows the
    six-byte PP header and a one-byte palette code follows for each image
    pixel. The canonical resource-0 executor consequently accepts only the
    admitted 240x96 body with stride 240, copies its exact index bytes and
    original BGR555 words to caller-owned buffers, and invokes an explicit
    presentation callback. It has no default presentation, host-RGBA
    conversion, CLUT substitution, trailing-byte interpretation, or fallback.
    Remaining evidence is an original Saturn display/VDP route if this asset
    is to be connected to a live screen rather than an externally supplied
    source-faithful presenter.

  - 2026-07-17 update: resource 0 now reaches the real 320x200 M11 indexed
    presentation surface. Each warning frame reopens the direct canonical
    source, checks the engine's exact asset identity, then revalidates the
    full PP receipt before it writes the top-left 240x96 index plane. The
    host palette receives only the 256 BGR555 words in ST-124 order
    `B4..B0/G4..G0/R4..R0`, expanded by exact bit replication to M11's RGB6
    palette API. A changed source/body, noncanonical asset, wrong host size,
    or any failed receipt leaves the already-cleared M11 frame unpresented;
    title, generic UI surface, and solid-image substitutes are not used.
    This does not prove a Saturn VDP display command, interlace, colour-DAC,
    gamma, timing, or placement contract beyond the documented PP resource-0
    bytes and the explicit M11 host surface.

  - 2026-07-19 update: all four canonical DGT2/PP resources now carry the
    same admission -> execution -> M11 presentation chain, not only resource

    0. New `nexus_v1_warning_dgt2_resource_corpus` module
    warning flow shows, in which order, remains original-Saturn evidence
    work).

- 🔧 2026-07-20 TITLE.BIN RES* directory corpus follow-up: new
  original-Saturn evidence work); 0DMSTRT.BIN shows no RES* framing and
  stays excluded from this block pending original evidence.
  [0x2e8, 0x1b658) that covers the source tail with zero gap; a bounded
  the original title/startup flow uses, in which order, remains

- 🔧 2026-07-20 TITLE.BIN TITL PP payload admission follow-up: new
  original title flow draws, where, and in which order, remains
  original-Saturn evidence work).

- 🔧 2026-07-20 TITLE.BIN DGT2 payload admission follow-up: new
  draws, where, and in which order, remains original-Saturn evidence
  work.

- 🔧 2026-07-20 TITLE.BIN MAPD TIBG admission follow-up: new
  assignment; how the original title flow uses this payload remains
  original-Saturn evidence work.

- 🔧 2026-07-20 TITLE.BIN CNFD payload admission follow-up: new
  the original title flow uses these payloads remains original-Saturn
  evidence work.

- 🔧 2026-07-20 0DMSTRT.BIN structure admission follow-up: the file
  execution route; how the original boot flow loads and uses this
  image remains original-Saturn evidence work.
  and rejection across NULL arguments, size/identity drift, gap

### Nexus V2.0 / V2.1 / V2.2
  - 2026-08-09 VDP1/DGN materialresolver: `nexus_v1_vdp1_dgn_material_resolver()`
    konsumerar en hashattesterad LEV*.DGN-Structure2 och kräver en unik
    byteidentisk mode-1-bild samt en unik återanvändbar 16-ords CLUT-join mot
    samma VDP1-VRAM-frame. CMDCOLR:s Saturn-ordadress konverteras till korrekt
    byteoffset (`<<3`). Positiv fixture och avvisning utan source-attest
    passerar. Resolvern tilldelar inte face, mesh-transform, culling eller
    produktionsägarskap; verklig full replay återstår. En frame-760-audit visar
    dessutom en första direct-color-draw utan byteexakt retailägare.

  - 2026-08-09 VDP2 raw-layout och NBG1-handoff: C använder nu capture-formatets
    verkliga VDP2-layout (CRAM 0x2000, VRAM 0x80000, registerfönster 0x200)
    och avkodar native-little-endian TVMD=0x8000 korrekt. Den nya
    `nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap()` kräver explicita
    källbytes- och VRAM-offset-attester och passerar med en bounded tilemap-
    fixture samt extern frame 760 som råtransport. En separat frame-80-audit
    visar NBG1 character mode och en oförändrad tvåords-PND-span runt 0x5c000,
    men ingen exakt FONT256 Page/CG/Palette- eller MENU.BPK/PRS3-bindning.
    Menyägare, FONT256-bindning,
    karta och autentiserad startup→meny-identitet återstår.
