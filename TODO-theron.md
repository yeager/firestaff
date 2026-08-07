# Firestaff TODO - THERON

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Theron Authentic CD Trace Follow-up (2026-07-12)

2026-07-13 live stage-two correction: the authentic US-CUE/System Card capture
reaches loaded RAM and disassembles `$40cd` as `JSR $e009` and `$40a4` as
`JSR $e00f`. `$4090` is instead `STA $fd` in the second table setup, not a
System Card call. Retain these as raw execution facts only; capture the exact
register/result/sector relationship for the two actual calls before assigning
CD_READ, record, destination, bitmap, palette, object, or level semantics.
The trace patch now emits one raw `stage2_system_card_call` and matching
`stage2_system_card_return` receipt at each exact call/return PC, including
the target, HuC6280 `A/X/Y/P`, and `$fc/$fd/$fe/$f8/$fa/$fb/$ff` registers. It
deliberately still does not label a call as a CD read or assign any data role
until a live post-call sector/result relationship is observed.
The authenticated US-CUE capture observes call/return register tuples
`40cd: A=01 X=03 Y=03 P=00 -> 40d0: A=00 X=01 Y=ff P=03` and
`40a4: A=01 X=03 Y=ff P=01 -> 40a7: A=78 X=00 Y=03 P=00`. The currently
sampled zero-page bytes are not assigned loader semantics by this receipt.
The same run records `MPR0=ff` and equal logical/physical `$fc=00` at every
sample, so the prior zero-page observation is now bank-accounted but still
does not establish a record, byte count, destination, or payload role.

The Mednafen trace harness now forces System Card 3.0 and the correct raw-CUE
PCE route. It records reproducible CPU progress but still lacks the original
target-PC snapshots required to select the IRQ2 hardware branch. Continue with
real trace evidence only; no inferred CD register value may enter runtime.
2026-07-13 sparse-instrumentation update: the debugger callback now returns
outside bounded System Card/loader PC windows, eliminating generic per-PC
transcripts and out-of-window memory inspection. A rebuilt 30-second authentic
US-CUE/System Card run shrank the debugger trace from 1,195 to 521 rows yet
reached the same `$c897/$c8c4` wait with three CDIRQ callbacks and no
non-System-Card PCECD read or raw sector. The trace overhead is therefore not
the cause of the missing handoff; do not infer one from the timing change.
The current macOS capture harness now focuses Mednafen and sends opt-in real
SDL `RUN`, `SELECT`, or `I` input. Fresh authentic US-CUE/System Card 3.0
captures prove host events and PCE port changes (`RUN=0008`, `SELECT=0004`,
`I=0001`) but no later controller transaction after those events: the observed
128 port transactions have already completed before the host-side event is
processed. None reaches a non-System-Card PCECD caller or raw sector. The next
job is therefore a frame-synchronised original input capture that lands before
the relevant System Card poll, or an independently observed later poll; do not
inject PCE state, manufacture a controller result, or infer a Track 02 read.
The verified `$e8ec` path is a fixed `$1804` latch/counter delay. After the
authenticated US System Card Run input, the first post-latch controller
exchange is now captured at `$e908..$ea3a`: `$81 -> $1801`, `$60 -> $1800`,
`$ff -> $1801`, then `$80` latched in `$1802` while `$1800` reads `$90`.
This is a raw CPU/register receipt only, not a `CD_READ` classification or
Track 02 binding. Continue from the observed `$e900/$e95a/$e97a` controller
branches and prove an explicit original read plus its record/destination
before assigning any payload, bitmap, palette, object, or level semantics.
The first post-exchange `$e97a -> $e981` dispatch is now a raw trace receipt
only; it still leaves `$224c,X` and its controller transition unclassified.
The next observed controller-only order is `$e981 -> $e985 (JSR $ea27) ->
$e988 (BRA $e95e) -> $e96a (AND #$b8)`. It separates this controller
transition from an actual data transaction but proves neither a `CD_READ` nor
record/register/payload semantics. Continue with a later explicit original
System Card data transaction whose record/register inputs and destination can
be independently observed.
The authenticated US raw-CUE trace also proves the immediate resume branch
after `$e96a`: `CMP #$98` at `$e96c`, `BEQ $e98a` at `$e96e`, then executed
`LDA $22a4` at `$e98a`. This is controller-status flow only, not a `CD_READ`,
record, destination, transfer, or payload classification. The next task is an
equivalent fresh US/JP capture through the new post-`$e98a` transfer hook.
That hook records only the first live branch/call/return source and target in
the immediate `$e98a..$ea3f` window; its verifier rejects static disassembly,
fixtures, and captures that do not first execute the original `$e98a` load.
The hook now gates directly on that live program-counter receipt rather than
on earlier diagnostic markers. A fresh hash-verified US-CUE/System-Card run
with this build reached the original `$c860..$c96f` wait loop but not `$e98a`
before timeout; it is a negative capture result, not a branch selection.
Neither capture can promote a controller transfer into a CD transaction. A
later independently observable data transaction is still required. The
post-`$e98a` Track 02 runtime consumer now accepts only one combined live
capture that also proves the `$4090 -> $4093` one-sector `$3800` transaction,
its observed `CL/DL/CH` record, and the `$e74c` IRQ2 controller-state merge.
No fresh combined JP/US capture has been staged, so it remains blocked; the
consumer must not derive a record from the controller transfer or a static
media receipt. The Mednafen patch now emits the consumer's two rows directly
from the observed `$4090/$4093/$e736/$e742/$e74c` instruction checkpoints and
their live zero-page/CD-register snapshots. It labels only a captured packed
record; unrecognized observations remain fail-closed. A fresh authenticated
combined JP/US capture is still required before this receipt can reach the
runtime handoff. Firestaff now parses only that exact combined receipt into
the existing Track 02/System Card runtime gate; missing, duplicated,
malformed, mismatched, or unrecognized rows cannot construct a live branch.
The boot/runtime intake now accepts an explicit trace-file path alongside the
authenticated Track 02 and System Card paths, but no authenticated combined
capture is staged. A missing, unreadable, oversized, or rejected trace keeps
the existing launch route blocked and does not synthesize a receipt.
2026-07-13 raw-media binding update: the combined receipt must now also match
its captured 32-byte `$3800` FNV span against the exact selected raw Track 02
sector before final startup-media binding. This proves only the observed
record-to-RAM transfer; it does not classify palette bytes, bitmap data,
objects, or a later dungeon record. A fresh authentic combined capture still
must pass this gate before any positive runtime route is accepted.
2026-07-14 Stage 3 sector-receipt update: the bound live CD_READ trace now
also joins the hash-verified Stage 3 MODE1/2048 receipt, preserving the
physical sector, raw offset, user-data offset, full user-data hash, and
observed `$3800` span checksum as one fail-closed handoff boundary. This
proves the executed loader record only; a Soul Room selection-to-dungeon
record relationship remains unobserved and must not be inferred.
2026-07-13 register-provenance update: a dynamic CD_READ receipt must now
carry the observed `CL/DL/CH` bytes and reconstruct its accepted Track 02
record exactly before raw-media binding. The existing startup bitmap receipt
remains an independent hash-verified raw-route proof; no record is labelled
as bitmap data until a real capture establishes that relationship.
2026-07-13 runtime bitmap-chain update: the opt-in raw Track 02 probe now
requires the hash-verified System Card, Track 02, and Mednafen loader trace
before it consumes a real startup bitmap route. It binds the trace span and
the complete raw bitmap-route receipt to the same media identity, while still
blocking palette output and record-to-bitmap semantics. A positive authentic
three-artifact run is still required; absent inputs skip without fallback.
2026-07-13 local-capture intake update: the same probe can now discover the
raw 2352-byte Track 02, System Card, and a variant-matched Mednafen trace by
hash beneath an explicit `THERON_CAPTURE_ROOT`. Known 2048-byte Track 02 ISO
images are counted only as diagnostics and are never promoted to raw media.
The locally inspected US/JP corpus contains two such ISO candidates but no
authenticated raw-Track02/System-Card/trace triple, so the positive bitmap
record route remains legitimately skipped and open.
2026-07-13 boot-handoff update: a complete authentic triple now reaches the
real Theron boot raw-media consumer and must retain the same Track 02 identity,
bitmap-route mask, atlas checksum, and no-fallback flag. It can acknowledge
the raw capture handoff, but remains unable to submit RGBA pixels until an
original palette-byte relation is captured.
2026-07-13 corpus-search update: capture-root discovery now makes a second,
trace-only bounded pass after it has hashed a raw Track 02 BIN, so trace
discovery is independent of filesystem order. The current local corpus still
contains only the two known 2048-byte ISO variants and no authentic triple;
the positive bitmap/palette receipt remains open.
The explicit intake can now bind only a variant-matched authenticated receipt
to a prepared boot profile. The production Soul Room forcefield transition
checks that receipt before it mutates startup flow or world state, so no
missing or unrecognised trace can open the dungeon route. A fresh combined
capture is still required to demonstrate the positive path.
2026-07-14 atomicity follow-up: the direct Soul Room runtime entry now applies
that same complete startup-media and stage-two `$4090 -> $3800` preflight
before it creates the party or changes flow/world state. The tracked startup
probe verifies the rejected known-profile path remains in the Soul Room;
positive progress still requires an authentic combined capture.
2026-07-14 Soul Room handoff update: a hash-verified raw Track 02 can now
positively bind its complete original bitmap receipt and the authenticated
stage-two `$4090 -> $3800` loader receipt to the candidate runtime world,
selecting only the original forcefield media bank before any level load.
This does not create tiles, objects, a party pose, or a dungeon-record claim;
the handoff remains atomic and exact level/object semantics stay separately
blocked on original loader/layout evidence.
The generic startup-flow probe no longer treats its own rectangles or
fabricated Track 02 bytes as title/runtime evidence. The title plan remains
empty until an authenticated Track 02 bitmap route exists, while a known
Track 02 identity with no raw bytes now blocks the level handoff explicitly.
Authentic media and a combined live capture remain necessary for either
positive route.
The subsequent observed `$e9d3 -> $e9dc -> $e9eb -> $e9f3 -> $ea15 ->
$ea1d -> $ea26` status loop is now independently verified from the same US
capture. It clears `$227b`, samples `$1801`, then completes two `$1800`
status waits before returning. This remains controller-only evidence: it does
not identify a CD read, a source record, a destination, a transfer, or payload
meaning. Continue only from a later explicit original data transaction.

2026-07-14 later-loader capture gate: the authenticated US raw Track 02 is
available locally, but the inspected historical Mednafen traces contain no
later `$e009` dispatch/return envelope. The trace patch records the first
post-stage-two HuC6280 `JSR $e009` as raw caller/return PC plus `CL/DL/CH`
record bytes. A standalone trace verifier and the opt-in paired-corpus probe
independently exercise the envelope parser and accept only a paired
hash-verified JP/US corpus and trace pair whose trace also retains its observed
`$4090/$4093` JP/US variant receipt, whose later records are in raw-sector
bounds, reconstruct from those bytes, resolve to the same existing stage-three
selector ordinal, and retain the same caller/return transition anchor.
Duplicate, mismatched, or standalone later rows reject. It assigns no CD
payload, object, graphics, palette, level, or gameplay-transition semantics. A
fresh matched JP/US capture remains required before any positive record
correlation is claimed.
2026-07-14 later-loader media join: the production raw-loader receipt now
requires that later `$e009` dispatch/return envelope after the same capture's
`$4090 -> $3800` Stage 3 span has already bound to hash-verified raw Track 02.
It retains only later record range, caller/return PCs, raw/user-data
coordinates, and a user-data hash. This is a positive executed
loader-to-media fact, not a dungeon/map/object/palette/bitmap meaning; a
positive dungeon route still needs an authentic executed range whose format
and route relationship are independently observed.
2026-07-14 selector-coordinate update: the production later-loader receipt
now additionally requires its captured record to resolve through the same
hash-verified Stage 3 descriptor selector table used by the paired-corpus
gate, and retains that selector plus ordinal. Synthetic or raw-sector-only
records cannot publish this receipt. This remains a coordinate constraint,
not a descriptor-format, dungeon, object, palette, bitmap, or transition
claim; a fresh matched JP/US Mednafen capture is still required.
2026-07-17 later-sector corpus launch handoff: an optional READY direct
CUE/coalesced-trace corpus receipt now crosses the real M12 launch intent into
M11 and is rechecked before the Soul Room/dungeon live-route admission against
the current direct media, trace identity, loader-replay tail, layout epoch,
and scan epoch. No local corpus remains an intentional unbound/SKIP state;
mixed, stale, cross-media, reordered, or trace-drifted evidence clears the
opaque no-draw readiness. Remaining work is an authentic matching later-route
capture corpus, not record-payload, object, level, bitmap, palette, or pixel
interpretation.
2026-07-17 first-dungeon world/level-object boundary: M11 now retains the
same original-observed Stage-3 descriptor coordinates (record, ordinal,
selector, caller/return PCs and bounded hashes) at the epoch-2 dungeon
handoff, but only while the direct media, coalesced trace, trace bundle,
loader replay tail, layout epoch, and scan epoch still agree. With no corpus
this admission is absent/SKIP; any drift clears it. It neither constructs a
world from record bytes nor names level/object fields or enables draw. The
next blocker is an authentic capture that proves a record payload grammar and
its world consumer, not speculative decoding.
2026-07-17 level/object descriptor capture intake: a ready direct
CUE/BIN/coalesced-trace corpus can now be re-attested as an opaque descriptor
receipt against the current direct layout, loader-replay tail, launch-trace
identity, layout epoch, and media-scan epoch, then enter the existing M12/M11
opaque corpus route without source changes. Missing local corpus remains
UNAVAILABLE/SKIP and malformed, stale, reordered, cross-media, or
cross-trace evidence rejects. The receipt deliberately publishes no record
payload grammar, level/object field, bitmap, palette, pixel, or draw meaning.
The remaining blocker is a real matching corpus and independently observed
payload grammar/world consumer.
2026-07-17 descriptor bitmap/palette capture intake: the dungeon-handoff
capture row can now be joined to that same descriptor-selected record only
when one complete external artifact agrees with the direct CUE/BIN layout,
coalesced trace, replay tail, launch trace, plan identity, layout epoch, and
scan epoch. M11 retains just the palette/bitmap output identities as an
explicit no-draw presentation witness; missing capture is UNAVAILABLE/SKIP
and any artifact or epoch drift clears it. No palette entries, bitmap bytes,
image format, record payload, world/object semantics, renderer, menu graphic,
or fallback exists here. The remaining blocker is an original capture proving
the byte-level palette/bitmap relation and an authorized consumer grammar.
2026-07-17 operator dungeon-handoff capture plan: the local Mednafen plan now
requires explicit executable, original CUE/System Card, operator-owned output
root, direct raw Track 02 MD5, layout epoch, replay final record/sector, and
capture-plan FNV before it writes a non-overwriting local request. Its only
opt-in execution path delegates to the existing bounded Mednafen trace
capture; descriptor manifest and palette/bitmap artifact paths remain empty
until the observed trace is converted and passes the existing strict intake.
No BIN, trace, payload, descriptor row, palette value, bitmap byte, or visual
output is checked in or synthesized. Remaining work is a real operator run
and the existing trace/descriptor/artifact validation chain over its outputs.
2026-07-17 capture-plan resume admission: M12 now retains a valid local
dungeon-handoff plan as CAPTURE_REQUIRED until an observed opaque artifact has
passed the descriptor bitmap/palette intake, and M11 copies that receipt at
real Theron launch. RESUME_READY additionally requires the live no-draw
presentation identities, direct CUE/BIN media, System Card requirement,
replay tail, plan FNV, layout epoch, and scan epoch to agree. Any mismatch
clears it; no plan can create a trace, descriptor, palette, bitmap, decoder,
or draw route. The remaining blocker is still an operator-produced observed
artifact accepted by the existing strict chain.
2026-07-17 dungeon-handoff source-trace binding: the strict local capture-plan
grammar now requires a lowercase source-trace MD5 and carries it in the
admission receipt. CAPTURE_REQUIRED can become RESUME_READY only when the
already admitted opaque descriptor/palette/bitmap artifact reports the exact
same coalesced trace MD5, in addition to the direct media, replay, plan, and
epoch checks. The local verifier hashes the supplied trace and compares it to
both plan and artifact. Missing, malformed, or divergent trace identity stays
rejected or capture-required; no payload, level/object, palette, bitmap, or
render semantics are introduced. The remaining blocker is an operator's real
observed trace and matching artifact chain.
2026-07-17 dungeon-handoff artifact-plan provenance: the capture-artifact
importer now requires all three opaque envelope rows to share one known direct
Track02 variant and MD5, then stamps the complete capture-plan identity into
its receipt. Descriptor bitmap/palette admission requires that exact identity
alongside the existing original media and coalesced trace checks. This keeps a
mixed, stale, or plan-drifted artifact out of the positive handoff path; no
positive local capture remains CAPTURE_REQUIRED/no-draw. No payload semantics,
bitmap/palette decoding, or rendering has been added. The remaining blocker
is still one complete original capture chain.
2026-07-17 dungeon-handoff operator capture schema: the external artifact
envelope now has a mandatory `capture_target_plan_fnv1a` row. The observed
Mednafen trace stamper and direct-media campaign emitter write it only from a
complete opaque capture plan; the importer and local handoff verifier require
it to match the current plan together with the direct Track02 and trace MD5s.
Wrong, missing, or stale plan evidence rejects before a receipt is published.
This is metadata-only: without a positive original CUE/BIN, trace, descriptor,
and artifact corpus the route remains CAPTURE_REQUIRED/no-draw, and no payload
or graphics semantics are inferred.
2026-07-17 dungeon-handoff operator launch receipt: a dedicated Mednafen
handoff validator now binds the requested capture-plan FNV to one direct
MODE1/2352 CUE/BIN intake before converting an explicitly MD5-bound observed
loader trace. It requires the complete opaque plan's Track02, CD-read,
loader-output, palette/bitmap identity, and destination record facts to agree
with the original media, then retains only capture-required/no-draw receipt
metadata. Bad plan identity, media, trace, or evidence clears the runtime
admission. The remaining blocker is a real local CUE/BIN plus observed loader
trace and later artifact corpus; no substitute media, decoded payload, or
render route exists.
2026-07-17 dungeon-handoff artifact-corpus import: one new strict operator
corpus admission accepts exactly one direct bundle only after the current
MD5-bound MODE1/2352 handoff receipt, source trace, and capture-plan FNV all
revalidate. No candidates returns SKIP, while multiple, virtual, stale,
cross-media, or altered candidates reject before the existing opaque importer
runs. READY is still capture-required/no-draw metadata and has no payload or
graphics semantics. The remaining blocker is a positive original corpus and
then an explicit M12/M11 receipt consumer.
2026-07-17 artifact-corpus host gate: M12 now carries only a current opaque
no-draw artifact-corpus receipt through launch intent, and M11 requires its
media, source-trace, and capture-plan identities before retaining
RESUME_READY. Drift leaves the route capture-required/no-draw.
2026-07-17 artifact-corpus intent identity: M12 now also rejects a stale or
mixed corpus receipt before launch intent reaches M11, requiring the current
single direct candidate, Track02 MD5, source trace MD5, no-draw policy, and
capture-plan identity. M11 remains the second runtime check.
2026-07-17 artifact-corpus rescan drift: a changed, rejected, or plan-drifted
direct-media rescan now clears the artifact corpus and dungeon capture plan
with the existing trace campaign state, preventing old no-draw evidence from
crossing a new media epoch.
2026-07-17 M11 active-session artifact-corpus epoch gate: M11 stamps the
opaque corpus receipt with the direct campaign-media scan epoch at launch and
refuses dungeon RESUME_READY whenever that bound epoch differs from the active
session epoch. The route is cleared fail-closed and stays no-draw until a
current capture-required or resume receipt is supplied; no payload, bitmap,
palette, level, or object field is read. The remaining blocker is still a
positive original artifact corpus.
2026-07-17 artifact-corpus envelope continuity: M12 now rechecks every
retained opaque start/Soul Room/dungeon envelope identity against the current
capture plan before a launch intent is valid, and M11 repeats the dungeon
palette/bitmap/destination identities before retaining RESUME_READY. Internal
envelope drift clears the route fail-closed. The receipt remains metadata-only
and no-draw; the remaining blocker is still a positive original artifact
corpus, never inferred payload or graphics semantics.
2026-07-17 dungeon loader-output continuity: the source-owned dungeon
descriptor/palette/bitmap receipt now carries the existing capture-plan loader
output identity. M11 retains it only as an opaque active-session witness and
requires the imported artifact's matching row before RESUME_READY. Loader
identity drift clears the route fail-closed/no-draw; no loader bytes, record
grammar, bitmap layout, palette values, or decoder semantics are exposed.
2026-07-17 dungeon CD-read scope continuity: the same receipt now retains the
source-owned capture-plan CD-read record separately from its descriptor record.
M11 requires the imported artifact's dungeon CD-read row to match this active
session witness before RESUME_READY. A changed record scope clears the route
fail-closed/no-draw; it remains a source coordinate only and establishes no
record format, level/object route, bitmap layout, palette, decoder, or render
meaning.
2026-07-17 dungeon loader-span continuity: artifact import now retains the
already verified loader raw offset and byte count for each capture-plan row.
The dungeon receipt carries only its offset/length coordinates into M11, which
requires the current artifact span before RESUME_READY. Offset or length drift
clears fail-closed/no-draw; no loader bytes, payload grammar, bitmap layout,
palette, decoder, or rendering semantics are retained or inferred.
2026-07-17 dungeon destination-span continuity: artifact import now also
retains the source-owned destination offset and byte count already verified
against each capture-plan row. M11 requires the dungeon coordinates to match
its active no-draw receipt before RESUME_READY. Offset or length drift clears
fail-closed; neither destination bytes nor level/object grammar, bitmap
layout, palette, decoder, or render semantics are read or inferred.
2026-07-17 dungeon route-selector continuity: the strict artifact grammar's
existing campaign-route selector is now required to name DUNGEON_HANDOFF
through corpus validation, M12 launch intent, and M11 resume admission. A
start or Soul Room selector cannot cross the positive dungeon handoff even
when all other identities match. This is route provenance only: no record
payload, level/object grammar, decoder, palette, bitmap, or draw meaning is
assigned.
2026-07-17 dungeon descriptor-selector continuity: the artifact envelope now
requires a nonzero observed Stage-3 descriptor selector. Descriptor intake
matches it to the existing source-backed sector-record receipt, M12 rejects
intent drift, and M11 requires the same active-session selector before
RESUME_READY. It remains an opaque loader record selector only; no level data,
object layout, bitmap/palette format, decoder, or rendering meaning is added.
2026-07-17 dungeon descriptor-ordinal continuity: the artifact envelope now
also carries the exact observed Stage-3 descriptor ordinal, including zero
when that is the authentic first-row coordinate. Intake matches it to the
source-backed sector-record receipt, M12 rejects intent drift, and M11
requires the active-session ordinal before RESUME_READY. It is record-location
provenance only, with no level data, object format, decoder, palette, bitmap,
or rendering semantics.
2026-07-17 dungeon descriptor-source-hash continuity: the artifact envelope
now requires the existing nonzero source hash for the observed Stage-3
descriptor witness. Intake matches it to the source-backed sector-record
receipt, M12 rejects intent drift, and M11 requires the active-session hash
before RESUME_READY. It is opaque descriptor provenance only; no source bytes,
level/object fields, decoder, palette, bitmap, or rendering semantics are
retained or inferred.
2026-07-17 positive handoff corpus ingress: M12 now has one operator-only
import boundary that invokes the strict direct CUE/BIN plus observed-trace
artifact-corpus importer before it can bind any handoff receipt. It requires
the active direct campaign, current scan epoch, launch-trace MD5 and complete
Stage-3 envelope identity to match, then sends only the ready opaque receipt
through the existing launch intent to M11's dungeon RESUME_READY gate. Missing
local media, missing corpus, virtual/altered bundle, or media/trace/plan drift
atomically clears the prior receipt and remains capture-required/no-draw. The
remaining blocker is one operator-supplied original MODE1/2352 CUE/BIN with
its complete observed Mednafen trace and artifact bundle; no test fixture can
substitute for that corpus, and no payload, level/object, bitmap, palette,
decoder, rendering, or fallback semantics may be added meanwhile.
2026-07-17 live dungeon loader-capture handoff: the selected Stage-3
descriptor corpus now retains its observed later `$e009` destination, bounded
span length, and span checksum. M12's real corpus import requires the
artifact's dungeon CD_READ and loader-output row to match those exact
original-trace facts, and M11 repeats that check at launch before publishing
the opaque live dungeon-handoff witness. A changed capture checksum clears the
witness and blocks RESUME_READY. This is still only source provenance: it does
not inspect the transferred bytes as a level, object, bitmap, palette, or
pixel. The next blocker is an original positive corpus for this direct path
and then a separately observed runtime consumer that establishes what, if
anything, the bounded output means; no decoder or fallback may bridge it.
2026-07-17 strict loader-output record admission: the manifest-bound original
`$e009` capture can now admit only the already disassembled `0x0b52` output:
the record-local `0x114` level-envelope span and the directly adjacent opaque
continuation retain exact offset, length, and checksum. The bitmap boundary is
explicitly unproven and remains absent. M11 invokes this admission only while
the original boot capture, imported artifact corpus, selected Stage-3 record,
and current trace all agree. The spans grant no level/object semantics, pixels,
rendering, or fallback. The blocker is an original `0x0b52` descriptor-selected
corpus through this path plus an observed consumer for any further bitmap or
object interpretation.
2026-07-17 typed `0x0b52` envelope header admission: the same original
loader-output receipt now verifies its disassembler-owned big-endian width,
height, seed, header-level-index, and opaque extension directly against the
manifest-bound envelope bytes. Those facts retain the existing exact
record-local spans and checksums but do not select a dungeon/route or grant
header-level-index semantics. Bitmap continuation remains explicitly unproven
and opaque. Remaining work is a second independently observed original capture
of this route and a game-owned consumer that can establish any identifier or
continuation meaning; no decoder, renderer, or fallback is permitted.
2026-07-17 local artifact-envelope verifier: the operator can now check a
direct regular plan, Mednafen trace, and capture artifact before handing it to
the existing importer. It requires the local plan's Track 02 MD5 and artifact
path, the actual trace MD5 in the envelope, dungeon route 2, and all three
descriptor/loader, palette, bitmap, and destination identity rows. It neither
reads payload windows nor promotes the file itself to RESUME_READY: only the
existing M12/M11 imported opaque receipt may do that. The next blocker remains
an authentic observed artifact and its strict importer admission.
2026-07-17 importer envelope ingress: the Track 02 artifact importer now
requires every start/Soul Room/dungeon envelope row to carry nonzero exact
loader checksum, palette identity, bitmap identity, destination record and
destination identity before it can publish a candidate receipt. It marks that
fact as `opaque_envelope_verified`; campaign verification and the descriptor
bitmap/palette intake require the mark before any M12/M11 resume path can be
considered. This remains identity-only/no-draw and does not inspect payloads
or infer a bitmap/palette format.
2026-07-17 Mednafen envelope stamp: a converted, MD5-bound observed trace can
now stamp the importer-compatible three-route handoff envelope from an already
source-locked capture plan. It writes only route identities and trace MD5,
requires nonzero descriptor/palette/bitmap/destination identities, and refuses
existing output. No payload, palette value, bitmap byte, decoder, or draw data
is emitted; the resulting file still needs existing importer/M12/M11 checks.
2026-07-17 later-route candidate intake: a closed metadata-only observed trace
row can now retain a later loader PC, record, raw sector, and destination
identity only when the direct campaign replay tail and layout epoch agree.
It publishes `CAPTURE_REQUIRED` with no route ID, level/object meaning,
payload access, decoder, or draw permission. Unknown, reordered, mismatched,
or absent rows reject/unavailable; an operator must still name and verify any
future route before the three-route startup table can change.
2026-07-17 later-route candidate manifest: a CAPTURE_REQUIRED candidate can
now be exported as one non-overwriting operator manifest containing only
Track02/trace MD5s, layout epoch, loader PC, equal record/raw sector, and
destination identity. M12 may retain the same opaque candidate only as
capture-required; it cannot create route-ready, name a route, expose payload,
or alter the fixed startup route table. Remaining work is operator naming and
independent route proof from a real observed capture.
2026-07-17 later-route candidate manifest import: operator manifests can now
be rehashed/admitted only when their closed metadata grammar matches the
current direct Track02 media MD5, trace MD5, layout epoch, and active replay
record/raw-sector. The importer republishes only opaque CAPTURE_REQUIRED
metadata and rejects missing, altered, cross-trace, cross-media, or
cross-epoch input. It still has no route ID, payload access, decoder, draw,
or route-ready state.
2026-07-17 later-route candidate campaign index: imported opaque candidates
can now be deduplicated by exact record/raw-sector metadata within one layout
epoch. Same-record metadata collisions reject; only CAPTURE_REQUIRED entries
are retained and M12 can bind the resulting index as capture-required only.
No route IDs, payloads, inference, or route-ready state are present.
2026-07-17 later-route candidate stale pruning: index currentness now requires
every retained opaque candidate to match the active layout epoch. Epoch drift
returns an empty index, preventing M12 from retaining stale capture-required
metadata. This remains metadata-only and cannot infer routes or payloads.
2026-07-17 later-route operator attestation import: a direct regular external
label may match only current record/destination/epoch candidate metadata and
returns CAPTURE_REQUIRED only. It is not a route ID and cannot promote runtime.
2026-07-17 later-route batch attestation import: a closed batch file now maps
only record, destination identity, and epoch onto current opaque candidates.
It restores metadata from the authoritative index, deduplicates deterministically,
and rejects collisions, missing candidates, and epoch/replay drift. Output is
CAPTURE_REQUIRED only with no route ID, payload, or promotion.
2026-07-17 later-route M12 launch binding: candidate campaign indexes now need
the current direct-media launch receipt and a valid current loader trace epoch
before M12 retains or carries them in launch intent. A media rescan clears the
index; trace epoch or entry drift rejects the intent. This still exposes only
opaque CAPTURE_REQUIRED metadata, with no route ID, payload, decoder, drawing,
or runtime promotion. Remaining work requires an independently observed trace
that can authenticate a named later route without changing the fixed route table.
2026-07-17 later-route receipt provenance: each newly admitted candidate now
retains the exact direct Track02 MD5 and observed loader-trace MD5. Candidate
manifest export must agree with those receipt fields, import restores them,
and campaign-index construction rejects mixed media or trace identities. This
remains opaque CAPTURE_REQUIRED metadata with no route ID, payload, decoder,
rendering, or promotion.
2026-07-17 SRM operator attestation portability: the direct-file gate now uses
Win32 file attributes to reject directories and reparse points, `_fullpath`
for canonical paths, and separator-neutral manifest splitting on Windows;
POSIX retains lstat symlink rejection. Both remain strict regular-file-only,
fail-closed paths with no save-content interpretation.
2026-07-14 complete-sector witness update: a selector-resolved later `$e009`
record now needs one provenance-marked Mednafen SCSI sidecar row whose complete
2352-byte sector and exact leading 32-byte span both match the hash-verified
Track 02 BIN. This establishes only an independent physical CD/media match,
not `$e009` causality, capture-session identity, a payload format, or
dungeon/object/palette/bitmap semantics. The next admissible handoff remains
a fresh matching JP/US original-capture pair with an independently observed
loader-to-sector ordering boundary.
2026-07-14 ordered-capture follow-up: the next JP/US captures must provide one
coalesced, provenance-marked Mednafen transcript whose exact authenticated
`$4090/$4093` row precedes one later `$e009` dispatch, one complete 2352-byte
raw-sector fingerprint, and the matching `$e009` return in that order. The
verifier records only observation order and rejects separate, reordered,
duplicated, unmarked, malformed, or cross-variant rows. It does not infer a
destination, payload format, dungeon, map, object, graphics, or palette
meaning. Existing split CPU/CD sidecars remain insufficient for this boundary.
2026-07-14 production coalesced-handoff follow-up: the Soul Room runtime
transition now rejects the earlier Stage 3/IRQ2 receipt by itself. It admits
only the manifest-bound coalesced `$e009` transcript after its later record,
32-byte local-RAM span, caller/return edge, and initial-level envelope all
rehash against the selected original Track 02. No synthetic transition,
graphics, map, object, palette, or format semantics were added. A fresh
authentic coalesced JP/US capture remains required for the positive route.
2026-07-14 manifest-bound coalesced follow-up: a future positive JP/US pair
must also supply one V2 capture manifest per transcript. Each manifest is
rehash-checked against its exact raw Track 02, System Card 3.0, and coalesced
Mednafen trace paths before the ordered sector receipt is accepted. This is
artifact provenance only: it still does not identify a payload format,
dungeon, map, object, graphics, palette, bitmap, or transition.
2026-07-15 post-`$3800` order gate: a future positive transcript must now
record the original Stage 3 `BRK $ff` IRQ2 return from `$3800` to `$3802`
before its later `$e009` dispatch. This proves ordering through the original
loader entry only. It does not classify the later sector, promote a grid,
or establish level, object, bitmap, palette, or transition semantics.

## Theron CUE IPL/Stage-Two Follow-up (2026-07-12)

The documented converted CUE layout now resolves only its explicit
`TQJP02.iso -> TQJP02End.iso` and `TQUS02.iso -> TQUS02End.iso` split-data
aliases, including MODE1/2048. The resolved member still crosses the normal
known-MD5 gate before any loader reads it; arbitrary missing CUE members do
not resolve. This provides real-media startup access, not original CDDA
provenance: the supplied `.wav` references remain absent and OGG companions
must not be presented as raw CDDA sectors.

The strict CUE route now carries an authenticated Track 02 IPL/stage-two
loader receipt from M12 into M11, with no filename fallback or cache payload.
The canonical `CD_EXEC` handoff is locked: record `0x0003e7` loads and enters
local RAM at `$4000`. Mednafen HuC6280/CD traces bind the first stage-two
`CD_READ` to record `0x0004df` (JP) or `0x0004e0` (US), one sector to `$3800`.
The original stage-two disassembly now proves this is a stage-three executable
handoff: after the completed read it transfers control with `jmp $3800`.
The stage-three entry is now proven as HuC6280 `BRK $ff`: it dispatches through
IRQ2 at `$fff6/$fff7` and resumes at `$3802`, so the descriptor table is not
linear CPU code. Remaining work is later handler execution, Track 02 reads,
and their data semantics.
The observed post-return chain now reaches
`$cbef -> $cb2f -> $e109 -> $c860 -> $fe92 -> $c86e -> $c897`, with no
proven `CD_READ` in the captured window. `$cb2f` is a verified `RTS`; `$e109` is a
verified `JSR $c860`; `$c860` initializes `$f8/$f9`, calls `$c950`, then
executes `JSR $fe92`. `$fe92` reads the System Card handshake registers and
returns, with `$1800..$1804` still `00 00 00 02 00`; it is not treated as a
data request. On the observed carry-clear return, `$c86e` reaches two writes
to `$18c0`; the live latch reads as `$d0` before the second write, then the
routine transfers through `$c950` to `$c897`. These are control-flow and
register receipts only, not a record binding. Capture a later explicit System
Card data transaction and bind its live registers to its Track 02 record
before promoting any payload.
The authenticated System Card 3.0 container has a 0x200-byte header; its
genuine IRQ2 vector is `$e736`. Bit 0 of `$f5` selects an indirect `$2200`
transfer; the clear path saves A/X/Y, calls `$e742`, then returns with `RTI`.
The post-`CD_READ` `$f5` value is not yet traced, so neither branch nor a
later record request is bound.
On that clear path, the handler now has verified original state flow:
`$1802 & $1803 | $f2` is stored in `$f2`, then bit 2 branches to `$e7b3`
when clear. The live hardware result remains unobserved, so this does not
select a branch or expose a later read.
The live trace gate now rejects all incomplete or media-only inputs. It will
accept only a complete Mednafen capture that preserves `$f5` from the `$4093`
CD_READ return through `$e736`, records `$1802/$1803/$f2` at `$e742/$e74c`,
and proves the resulting branch. No such original capture is staged yet.
The external capture manifest is now parsed as a closed five-field binding:
canonical lower-case MD5 values, control-free paths, exact field order, and no
unbound trailing data. This strengthens provenance but is deliberately not a
runtime handoff; only the complete dynamic Mednafen CD transaction can open
that route.
The runtime intake also rehashes the exact Track 02 and System Card files it
reads before parsing capture text, closing the caller-label/time-of-check gap.
It still needs a real complete dynamic transaction; the existing wait capture
does not qualify.
The completed stage-three record also now has physical MODE1/2352 provenance:
JP record `$4df` carries BCD MSF `01:03:38`, US `$4e0` carries `00:58:57`,
and both have the sync/mode-1/user-data envelope at byte 16. This identifies
transport only, not a payload format or semantic record role.
The runtime-facing IRQ2 receipt now additionally rereads those authenticated
MODE1 user-data bytes and requires the actual `$3800` prefix `00 ff` before
publishing the stage-three `BRK $ff` dispatch. Startup now consumes that gate
before it publishes the raw JP/US dynamic-manifest handoff; a changed selector
fails closed and cannot reach M11. Direct Track 02 runtime-level entry now
consumes the same gate before it examines bank anchors, so it cannot bypass
startup receipt construction. For raw JP/US media it also requires the entire
original IPL chain: `CD_EXEC` record `$3e7`, 17 sectors to/entry at `$4000`,
then the traced `$4090` one-sector local `CD_READ` to `$3800` with its full
live-record-register mask. The startup receipt now requires this same chain
before it publishes the dynamic-manifest handoff, so M12/M11 and direct
runtime entry share one authentic media boundary. This binds the loader's
executable transfer to physical Track 02 bytes, but still identifies no
descriptor, palette, object, level, or later CD request.
The runtime handoff now also carries the stage-two work-RAM precondition from
the original `TII`: `$2700..$37ff` is cleared before entry at `$3800`. This
is strict loader-state metadata, not a manifest layout or a bank/level role.
The strict CUE/M11 boot validator now consumes both the work-RAM transfer
contract and the physical `$3800` `BRK $ff` gate after rereading the original
payload. A CUE receipt can therefore no longer reach runtime on manifest facts
alone. Later IRQ2 branch selection and every bank/level meaning remain
separately unbound.
All three consumers now share one Theron-owned original-media handoff rather
than duplicating loader checks: IPL `CD_EXEC`, traced `$4090` read, cleared
work RAM, and physical `$3800` `BRK $ff` are validated together. This reduces
gate drift only; it does not create a new level, bank, palette, or manifest
binding.
That shared handoff now also consumes the physical MODE1 header: JP stage
three is BCD `01:03:38`, US is `00:58:57`, with the proven user-data window.
This is transport provenance only, not a sector-to-level or payload-role map.
It also requires the earlier IPL `CD_READ` at `$40cd` into local RAM `$3000`.
The original table at `$40dc` now proves record `$03e3`, two sectors, and both
physical MODE1 sectors are validated in JP/US media. Its payload role remains
unknown, so it contributes a verified local-RAM loader boundary, never a bank
or level route.
The actual 4096-byte logical preload payload is now retained as strict
provenance: both variants agree on its FNV receipt, first nonzero byte 243,
and 2911 nonzero bytes. This binds bytes and size only, not code, compression,
bank, object, palette, or level semantics.
The original post-read sequence is also locked: `JSR $e009; CMP #$00; BNE
$40a9; RTS`. Thus `$03e3` returns to the IPL retry loop rather than transferring
control to `$3000`; runtime must keep it as unclassified staging data pending
a later original consumer.
The next IPL transfer is now byte-bound as well: table `$40d5` is verified as
`00 e7 03 11`, the original `CD_EXEC` record `$03e7` / 17-sector handoff to
`$4000`. This proves the transfer table only, not stage-two game-data roles.
The following stage-two bytes are now bound too: the exact `$4080` setup makes
the one-sector local `$4090` read to `$3800`; its success path preserves the
live record bytes, clears `$2700..$37ff`, and jumps to `$3800`. This proves
loader control flow, not a descriptor, palette, object, or level role.
The first loaded payload is now structurally verified as a
218-unit manifest envelope, but its entries remain unclassified; do not treat
it as a graphics, palette, object, or dungeon-record binding. The hash-gated
startup handoff now publishes this exact record and envelope receipt for raw
JP/US media, while keeping all entries semantically blocked. M11's strict CUE
receipt validation rereads and rechecks this sector before launch.
The stage-three table now has a read-only original-media receipt: all 218
six-byte big-endian descriptors are retained as opaque word triples. The
verified JP/US tables share 211 descriptors and differ in 7; that comparison
does not establish any descriptor field's meaning or permit route promotion.
Descriptor zero's opaque selector now has a bounded coordinate correlation:
adding the JP base `0x4d5` or US base `0x4d6` to its shared value `0x000a`
resolves exactly to the already-proven stage-three sector. This does not prove
that other selectors are CD commands or bind any later record to an object or
level; later loader execution evidence is still required.

## Theron Track 02 Semantic Binding Follow-up (2026-07-11)

Verified Track 02 startup receipts now fail closed when an object-table anchor
is `NOT_BOUND`: launcher/title, stage select, Soul Room, forcefield, Continue,
and pointer/keyboard startup actions cannot use synthetic routes, while a
separately bound startup-level runtime receipt remains intact. Remaining work
is original-media evidence that binds the object table and later level records;
do not relax this gate or promote compact-row candidates meanwhile.

## Theron Original SRM Body Correlation Follow-up (2026-07-11)

Startup now exposes only fully gzip-trailer-authenticated unknown Save Disk
containers as opaque transfer candidates. They remain unavailable to Continue,
and failed SRM Continue leaves the world unchanged. The outstanding work is
still source-backed original body-layout correlation before any original SRM
can restore progression, party, or runtime state. Firestaff-native SRM export
also now publishes atomically without replacement, so it cannot overwrite a
staged original Save Disk artifact while the corpus remains unbound. The
direct SRM runtime handoff now requires all four hash-verified Track 02 media
surfaces and a selected real-media level bank before committing a restored
world; identity-only media rejects without mutation. Its structured receipt
now exposes the consumed media route mask, checksum, and selected level bank;
the remaining SRM blocker is still only original body-layout correlation.

## Theron's Quest

### Theron V1

- 🔧 2026-07-15 Track 02 post-Stage-2 `$e00f` service boundary: the same
  authentic 45-second boot receipt now covers direct non-System-Card calls to
  both System Card loader entries. Across two Stage-2 returns and 52 observed
  post-stage physical code pages, the only `$e00f` call is the already-known
  Stage-2 `$40a4 -> $e00f` setup, with `ff0000`/`ffff`/`ff` sentinel fields;
  the only `$e009` call remains `$3840` with the same invalid fields. No later
  direct game loader call to either entry and no game-owned `$1801` writer is
  observed. Indirect, block-transfer, or unobserved-route calls remain
  unclassified, so this is a boot-path boundary, not a universal absence
  claim. The next route still requires a non-sentinel caller correlated with
  a raw-sector receipt and verified return destination.

- 🔧 2026-07-15 Track 02 post-Stage-2 game-call boundary: an authentic
  45-second US CUE + System Card 3.0 capture accepts two real host RUN
  transitions, reaches two Stage-2 returns, and observes 61 physical code
  pages afterwards. It contains exactly one direct non-System-Card
  `$3840 -> $e009` call, but its record (`ff0000`), destination (`ffff`), and
  mode (`ff`) are all sentinel values; it is not followed by a game-owned
  `$1801` writer (only System Card `$e90d/$e92d/$e981` are observed). The
  candidate therefore remains rejected and cannot be treated as a later
  record or dungeon handoff. Next evidence must be a non-sentinel game call
  correlated with a subsequent raw-sector/SCSI receipt and a verified return
  destination.

- 🔧 2026-07-15 Track 02 live SCSI caller/destination boundary: a fresh
  authentic US CUE + System Card 3.0 capture records every `$1801` SCSI CDB
  byte with its HuC6280 caller, alongside each decoded READ(6) packet and raw
  sector binding. All 48 observed READ(6) packets, including later reads
  through generation 48 / LBA 4265, were issued by System Card `$e981`
  (command bytes) after `$e90d` selection; FIFO bytes were copied only by
  `$ea50` into System Card RAM `$1f:2256+`. No non-System-Card CD caller,
  dynamic `$e009`, or game-owned destination was observed, so none of those
  later records may enter the dungeon handoff. Next admissible evidence is a
  real game-code caller and destination after the System Card returns, tied
  to a hash-verified Track 02 sector and an original level/object consumer.

- 🔧 2026-07-14 Track 02 initial-level payload handoff: the one complete,
  trace-witnessed 2048-byte `$e009` payload is now copied atomically from the
  rehashed original MODE1 user-data sector into the runtime boot receipt.
  Record `0x0b52`, source coordinate `0x114`, destination `$3800`, byte
  count, and FNV-1a checksum must all agree; any change rejects the Soul Room
  route and cannot select a generated fallback. The payload remains opaque:
  its dungeon/object/tile/bitmap/palette grammar and a positive level
  transition still need original execution evidence.

- 🔧 2026-07-15 Track 02 level/object boundary: the authenticated original
  `$e009` sector at record `0x0b52` is now split only at the source-proven
  boundary: level envelope `[0x114,0x480)` and the remaining opaque bytes
  `[0x480,0x800)`. The latter is copied from the same rehashed 2048-byte
  loader witness, carries its own FNV-1a receipt, and is rechecked against the
  raw MODE1 sector before the Hall of Records route enters runtime. It is not
  an object table, is not parsed, and cannot create objects, graphics, or a
  fallback. Still required: an original game-owned consumer that establishes
  the record grammar and connects it to a later level/object transition.

  A complete 2048-byte ISO can now enter this byte-level boundary only when
  every sector from Track 02 INDEX 01 is identical to the selected canonical
  raw BIN user-data lane. That maps record `0x0b52` to ISO envelope
  `[0x5a9114,0x5a9480)` and opaque continuation `[0x5a9480,0x5a9800)` for the
  current corpus. The local short ISO is not that complete projection and
  remains blocked. This proves media identity and record coordinates only;
  it still does not establish object grammar or a game-owned object consumer.

  2026-07-15 source-map correction: the local original US Stage 2 HuC6280
  disassembly shows the completed `$e009` read being copied to `$3800` and
  immediately transferred with `jmp $3800`. Thus record `0x0b52` is an
  executed loader sector, not a proven initial level/object record. Firestaff
  now blocks its former level-shaped subrange from runtime promotion. The
  legacy direct semantic/grid APIs now fail closed too, so no caller can
  bypass that runtime gate with Firestaff's host level loader. Required next
  evidence is a game-owned post-`$3800` consumer that reads a separately
  hash-bound level/object record and proves its grammar.
  - Update: a provenance-marked loader trace can now establish one narrower
    stage handoff when an authenticated `$3c80` continuation `TII` is followed
    by a main-RAM `JSR` to exactly the copied destination. This proves original
    byte-to-code transfer only. Remaining: capture that sequence on real media
    and then prove the called code's record reads and grammar before publishing
    a level or object table.
  - Update: descriptor 0 of the authenticated Stage-3 manifest now has an
    explicit runtime boundary receipt. Its original three raw words, derived
    record coordinate, raw MODE1 sector, user-data offset, byte count, and
    FNV-1a hash must agree with the same `$3800` loader sector before startup
    proceeds. The descriptor remains opaque: it proves neither a descriptor
    grammar nor level, object, tile, palette, bitmap, or command semantics.
    Next evidence remains a real post-`$3800` loader trace that connects a
    non-self descriptor-selected record to its executing consumer.
  - Update: a later captured `$e009` record can now enter the handoff only
    with its complete originating Stage-3 descriptor row: raw `word0`,
    `word1`, selector `word2`, descriptor ordinal, and the selected MODE1
    user-data hash are re-derived from the canonical Track 02 bytes. A changed
    row or sector withdraws admission. This is still record identity only;
    no row field is assigned a level, object, tile, palette, bitmap, or loader
    command meaning. Next evidence must show how a post-`$3800` consumer uses
    one non-self row before any record grammar can be considered.
  - Update: descriptor-to-record aliasing is now part of the authenticated
    handoff. The selected selector's complete occurrence count, first/last
    ordinals, and hash over every matching raw row are re-derived from the
    original Stage-3 table. This distinguishes one selected row from a changed
    or ambiguous table relation without assigning aliases, row ordering, or
    any word a gameplay meaning. Remaining: a real post-`$3800` execution
    trace proving how a non-self descriptor record is consumed.
  - Update: the handoff now retains the exact six-byte big-endian descriptor
    source span inside the authenticated loaded Stage-3 sector. Its physical
    raw offset and hash must re-decode to the retained three descriptor words
    before the selected later sector may enter runtime. This proves one
    original loader-table-to-record boundary only. It does not assign field
    formats, graphics, palette, object, level, or command semantics. Remaining
    work still requires a real game-code consumer for a non-self record.
  - Update 2026-07-20: the static descriptor evidence now covers the complete
    stage-three loader record table, not just one selected row. A corpus
    correlation resolves every non-zero selector in the authenticated
    218-entry manifest against the derived base and re-verifies each resolved
    MODE1 sector against the hash-gated Track 02 bytes (sync, mode, and a
    chained user-data identity in descriptor-table order, aliases included);
    any out-of-bounds selector, malformed envelope, or changed byte fails
    closed. On the hash-verified US media all 216 non-zero selectors (2 zero)
    resolve in-bounds to 162 distinct well-formed MODE1 records spanning
    records 0x4d7..0x5d3 with chained identity hashes reproduced exactly.
    This proves the full physical media span of the loader record table only;
    no descriptor word, resolved record, or span boundary is assigned a
    level, object, tile, palette, bitmap, or command meaning. Remaining: JP
    corpus numbers await staged JP media, and a real post-`$3800` consumer
    trace is still required before any record grammar can be considered.
  - Update 2026-07-20: the corpus now also proves the exact referenced-record
    set as a span topology (d6e28c629). From a proven corpus and the same
    authenticated manifest, a fail-closed binder derives the precise distinct
    record set the stage-three table references — packed one bit per span
    slot with aggregate counts and a per-slot flag hash — and a membership
    query answers whether any record belongs to that authenticated set. On
    the hash-verified US media: 162 referenced records across 253 span slots
    (records 0x4d7..0x5d3, 91 unreferenced slots), slot-flag hash reproduced
    exactly; membership includes the stage-three self record 0x4e0 and
    rejects interior gaps, below/above-span records, and the later traced
    0x0b52 record. This is a record-membership boundary only: an
    unreferenced slot is not proven absent from any other loader path, and
    neither membership nor absence assigns a level, object, tile, palette,
    bitmap, or command meaning. Remaining: JP span numbers await staged JP
    media; wiring the membership gate into later-route candidate admission
    still requires the capture-side record evidence those intakes gate on.
  - Update 2026-07-20: the boot chain's complete static record footprint is
    now proven as one topology (6a4f0d079). Every statically named record of
    the authenticated Track 02 boot chain is joined into a single
    fail-closed receipt across both coordinate frames: the IPL-family spans
    (IPL executable, preload, stage-two executable) are anchored through the
    data track's INDEX 01 raw sector while the stage-three manifest record
    and the descriptor corpus resolve file-relative; each loader-named
    record's MODE1 envelope is re-verified against the hash-gated media, and
    the spans join the proven descriptor referenced set into one membership
    bitmap with a contains query. On the hash-verified US media: IPL
    executable 1156..1159 (4 sectors), preload 1220..1221, stage-two
    executable 1224..1240 (17), stage-three record 1248, joined with the
    162-record corpus into 183 distinct named records across a 336-slot span
    (1156..1491), slot-flag hash reproduced exactly. The receipt also proves
    the coordinate overlap facts: the stage-three table references the
    stage-two executable's final two sectors (1239/1240) and its own
    manifest record (1248). This is record-span topology only — no record is
    assigned a level, object, tile, palette, bitmap, code, or command
    meaning, and an unnamed record is not proven unreachable through any
    other path. Remaining: JP topology numbers await staged JP media
    (doc-attested JP anchors are probe-covered); consumer-side wiring still
    requires capture evidence.
  - Update: the Mednafen main-RAM loader capture now records RTS instructions.
    A continuation execution receipt requires exactly one RTS inside the exact
    destination span of the source-bound `$3c80` TII, after the matching JSR.
    This proves that execution reached a copied original termination
    instruction, not a return target, level/object grammar, or visual route.
  - Update: the same source-owned capture now records the first main-RAM
    instruction observed after that RTS. Admission requires that row to name
    the exact captured RTS and to land at the original JSR return address.
    The receipt carries the observed opcode and physical PC while retaining
    the TII's verified Track 02 source transfer. This proves a bounded
    copied-code return target only, not a descriptor grammar, level/object
    consumer, tile, palette, bitmap, or visual route. Remaining: authentic
    capture of this sequence followed by a proven non-self descriptor record
    consumer.
  - Update: when that observed return instruction is itself a JSR, admission
    now binds its immediate target from the adjacent original loader trace
    row, retaining the same source-bound Track 02 TII receipt. Any missing,
    reordered, or changed call-site row rejects. This proves only the next
    invoked routine's control-flow address; it does not identify the routine,
    its data tables, or any level/object/graphics semantics. Remaining:
    authentic capture tying that routine to one non-self descriptor-selected
    CD record or verified loader data-table span.
  - Update: that post-return routine call can now remain admitted only after
    one later main-RAM RTS and its linked post-RTS row return to the exact
    caller address. Nested returns are ignored unless their observed target
    is that caller. This extends the authenticated TII -> copied continuation
    -> routine -> caller control-flow chain, but leaves the routine and every
    byte it may access opaque. Remaining: an authentic loader trace that
    binds one subsequent CD read or table span to this verified routine path.
  - Update: after that caller resumption, admission now retains the first
    subsequent main-RAM JSR observed in the same original trace. Its physical
    call site and immediate target are tied through the complete prior Track
    02 TII/copy/call/return chain. The target is still opaque; no routine ABI,
    descriptor, CD read, table, level, object, or graphics meaning is claimed.
    Remaining: capture a verified CD-read or descriptor-table access from one
    of these bounded routine paths.
  - Update: the capture producer now records a main-RAM call-entry row only
    when the next-caller JSR target is actually executed at that target. The
    receipt requires exact caller PC, physical PC, target, entry PC, physical
    PC, and opcode, while retaining the prior authenticated Track 02 chain.
    A non-main-RAM or unobserved target stays unadmitted. This proves target
    execution only, never a routine ABI, data-table layout, CD read, level,
    object, palette, bitmap, or rendering semantics. Remaining: bind an
    authenticated loader data read from an observed entry path.
  - Update: the same producer now retains the next observed main-RAM
    instruction after an admitted entry, tied to the entry logical/physical
    PC and preserving its raw opcode. A missing or non-main-RAM continuation
    remains unadmitted. This is still only execution ordering; it does not
    assign opcode, loader, table, CD-read, level, object, palette, bitmap, or
    rendering semantics. Remaining: an authentic loader data-read receipt
    originating from one of these observed routine paths.
  - Update: when that successor is an observed `TII`, admission now requires
    its source interval to lie wholly within the earlier hash-bound Track 02
    continuation copy. The new receipt retains both RAM intervals and the
    original-source coordinate/checksum. This proves original bytes were
    re-copied by the observed caller path, not what those bytes represent.
    Remaining: capture an original CD read or descriptor-table access beyond
    this already known continuation interval.
  - Update: the first observed JSR after that admitted TII must now target its
    copied destination before it enters the receipt chain. This proves the
    exact original Track 02 interval reached an observed code destination,
    while keeping the routine and bytes opaque. Remaining: bind an observed
    CD read or a separately proven descriptor-table source beyond this copied
    continuation route.
  - Update: the capture producer now records a main-RAM call-entry row only
    when the next-caller JSR target is actually executed at that target. The
    receipt requires exact caller PC, physical PC, target, entry PC, physical
    PC, and opcode, while retaining the prior authenticated Track 02 chain.
    A non-main-RAM or unobserved target stays unadmitted. This proves target
    execution only, never a routine ABI, data-table layout, CD read, level,
    object, palette, bitmap, or rendering semantics. Remaining: bind an
    authenticated loader data read from an observed entry path.
  - Update 2026-07-19: the post-envelope execution binder now honours its
    documented "exactly one RTS inside the exact destination span" contract
    on long captures — RTS rows outside the copied continuation span and
    post-RTS rows of other routines are ignored instead of failing the
    chain, and the TII transfer receipt now snapshots the accepted row so
    later parsed-but-source-filtered block-transfer rows can no longer
    overwrite the reported source/destination/byte count. Both defects were
    latent and only surfaced on captures containing a second routine.
    Remaining: authentic capture exercising these windows on original media.
  - Update 2026-07-19: the chain now extends past the branch-target JSR CD
    receipt with consumer-read admission — the observed
    `pce_cd_fifo_origin_main_ram_consumer` row must carry the exact FIFO
    byte (generation/LBA/offset/value), the exact fifo_sequence and
    main-RAM destination of the joined receipt row, and a main-RAM reader
    PC; a joined byte consumed by a different transfer or by a System Card
    reader fails closed — and with control-transfer admission, which binds
    the first observed main-RAM JSR after that consumer read (opaque
    target). This is still byte- and control-flow provenance only: no
    level, object, palette, bitmap, or rendering semantics are proven.
    Remaining: an authentic capture of the loader's consumer reads and
    control decisions on original media; every chain remains fail-closed
    without semantics.
  - Update 2026-07-19: the chain now binds the control routine's observed
    execution window after that control transfer — the producer's
    call-entry row must be adjacent to the control JSR and prove the
    control target was actually fetched in main RAM, the next main-RAM
    instruction row must be adjacent to that entry, and exactly one
    main-RAM RTS whose linked post-RTS row resumes at the exact control
    call return address closes the bounded window (zero or two qualifying
    resumes fail closed; other routines' RTS/post-RTS rows remain opaque).
    This is still bounded execution provenance only: no routine ABI,
    record, level, object, palette, bitmap, or rendering semantics are
    proven. Remaining: an authentic capture of this control window on
    original media, and evidence of what the resumed loader path reads
    next.
  - Update 2026-07-19: the chain now covers what the resumed loader path
    reads after that bounded return — after the exact post-RTS resume row,
    an observed FIFO receipt row for the byte adjacent to the first bound
    consumer byte (same generation/LBA, source_offset + 1) must re-verify
    against the hash-verified Track 02 media, and the following consumer
    row must be the second observed consumer (sequence=1) joined to that
    receipt's fifo_sequence and main-RAM destination with a main-RAM
    reader; out-of-order, different-byte, different-transfer, or System
    Card reader observations fail closed. The first main-RAM JSR after
    that resumed read is bound as an opaque control transfer, and an
    adjacent call-entry row must prove its target was actually fetched in
    main RAM. This is still byte- and control-flow provenance only: no
    record, routine ABI, level, object, palette, bitmap, or rendering
    semantics are proven. Remaining: an authentic capture of the resumed
    read/control sequence on original media, and evidence of how far the
    loader's per-byte consume/dispatch loop extends.
  - Update 2026-07-20: the chain now closes the resumed control routine's
    own bounded window — an adjacent next-instruction row after the
    resumed entry, then exactly one main-RAM RTS whose linked post-RTS
    row resumes at the exact resumed control call return address
    (zero or two qualifying resumes fail closed) — and observes the
    loader path's second resumption: after the exact second resume row,
    a FIFO receipt row for the byte two positions after the first bound
    consumer byte (source_offset + 2) must re-verify against the
    hash-verified Track 02 media, and the third observed consumer
    (sequence=2) must join that receipt's fifo_sequence and main-RAM
    destination with a main-RAM reader. This is the first loop-iteration
    evidence for the loader's per-byte consume/dispatch pattern, still
    byte- and control-flow provenance only. The synthetic capture buffer
    was enlarged after the chain outgrew 4096 bytes. Remaining: an
    authentic capture of the repeated consume/dispatch loop on original
    media, and evidence of where the loop terminates or dispatches into
    a record consumer.
  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop instead of binding one iteration at a time — a
    single continuation binder requires a fixed count of further
    iterations after the twice-resumed consumer read, each with the full
    window: an opaque main-RAM control transfer, its adjacent call-entry
    and next-instruction rows, exactly one bounded RTS whose post-RTS row
    resumes at the exact call return address, and the next
    source-adjacent FIFO byte's consumer read joined to its
    media-re-verified receipt with the expected loop sequence,
    fifo_sequence, and main-RAM destination. The consumer reader PC must
    equal the resumed return address, so the loop back-edge is explicit:
    the resumed loader path itself performs the next read. A missing
    iteration, out-of-order observation, off-target or duplicated
    resume, media-mismatched receipt byte, different-byte consumer,
    out-of-order sequence, different transfer or destination,
    non-main-RAM reader, or a reader that is not the resumed path fails
    closed. The synthetic harness grew a loud truncation guard for the
    capture construction. This is still byte- and control-flow
    provenance only. Remaining: an authentic capture of the repeated
    consume/dispatch loop on original media, and evidence of where the
    loop terminates or dispatches into a record consumer.

- 🔧 2026-07-11 Theron paired-CUE real-media follow-up: the hash scanner now
  accepts a CUE only when its one readable Track 01 AUDIO plus Track 02
  MODE1/2352 declaration canonically resolves to the independently
  hash-verified Track 02 payload. M12 passes that original CUE path to the
  launch profile, while an absent, malformed, renamed, or mismatched pair
  stays Track-02-only. No media is copied or synthesized. The bounded Track 01
  consumer now accepts only the CUE-declared WAV stem's local OGG counterpart
  and decodes it through optional Vorbis support; platforms without that
  decoder remain silent. Remaining work is user-staged JP/US title
  playback/capture evidence, not broader filename pairing or invented audio.

- ✅ 2026-07-27 Theron CDDA host-consumer correction: M11 had the title-phase
  CDDA lifecycle but reset the handoff without rebuilding it. It now consumes
  only the scanner-admitted CUE path together with the verified Track 02 MD5;
  raw BIN/ISO paths remain silent. The local authentic USA CUE passes both
  the availability and handoff tests through `FIRESTAFF_THERON_CUE`.

- 2026-07-27 Theron raw-CUE runtime launch regression: the current M11 path
  reaches the real startup route from the authentic USA MODE1/2352 CUE/BIN
  set (`f23601102138f87c33025877767ebf76`) and no longer relies on a direct
  Track-02-only probe. The focused runtime CTest advances title, stage, and
  Soul Room inputs under the dummy SDL driver, then requires
  `phase=theron-startup-2` and the original US asset identity. This proves
  startup admission and flow only; it does not promote unbound Track 02
  graphics, later dungeon records, or save semantics.

- 🔧 Track 02 graphics-format follow-up: the real hash-verified JP/US raw-BIN
  catalog found 1,522 strict HuC6260-shaped windows and 78 strict LE16
  stride-shaped windows across 2,022 exact matching nonzero MODE1 sectors.
  Its bounded detail list retained 64 records and overflowed 1,536; these are
  overlapping syntax matches, not independently proven palettes/tables. The
  catalog authorizes no decoder or runtime route. Exact media receipt:
  `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
  Next evidence must trace one catalogued user-data offset through HuC6280 CD
  loader code to a VCE palette write or VDC VRAM destination, including the
  loaded byte count; only that can bind a candidate to graphics, a palette,
  or a compression routine.
- 🔧 2026-08-06 JP Stage-2 disassembly follow-up: the authentic JP Track 02
  BIN is now materialised as `~/.firestaff/data/theron/TQJP02.bin` and its
  IPL loader plus dynamic `$3800` payload receipt pass against record `0x4df`.
  The later static Stage-2 byte windows remain US-only because the JP image
  has region-specific bytes; do not widen those verifier gates until a JP
  disassembly identifies equivalent instruction/data spans and their callers.
- 🔧 2026-07-11 IPL-loader provenance update: original CUE sheets prove Track
  01 is CD-DA narration, while Track 02 is the MODE1 code track. The
  hash-gated JP/US Track 02 IPL information block at logical sector 1 selects
  record `0x0003a3`, load/entry `$4000`, and a 3-sector JP or 4-sector US
  executable. Both actual executables contain `JSR $e009` (System Card
  `CD_READ`) at CPU `$40cd`; the immediately verified setup selects local RAM
  `$3000` (`DH=$01`), not VRAM (`DH=$fe/$ff`). This is the first genuine
  loader/media linkage, but it does not bind the selected record, count,
  decompressor, palette, or graphics candidate. The next admissible step is
  bounded dataflow from this loader's record table through one complete read
  setup to a verified VDC/VCE destination; generated rendering remains
  fail-closed meanwhile.
  - Update 2026-07-20: the static IPL + stage-two loader read windows are
    now fully byte-bound (008caff66). Four remaining unbound windows were
    disassembled from the hash-gated US media and bound as fail-closed
    static patterns in `theron_v1_track02_find_ipl_loader`: the stage-one
    CD_EXEC retry branch at user offset 0xa7 (BRA back to the $4080
    table-reader loop head), the stage-one CD_READ preload-table load at
    0xa9 (24 bytes: CLX plus four LDA $40dc,X / STA pairs into the same
    CL=$fc/DL=$fe/CH=$fd/AL=$f8 zero-page argument map the CD_EXEC setup
    uses, joining the preload table bytes to their reader code), and the
    two static stage-two invocations of the $40ae register-seed
    subroutine (JSR at 0x29 on the init path; BSR +0x2e at 0x7e on the
    retry path, its displacement landing exactly on the seed body). With
    these, the stage-one read window is contiguously bound across
    [0xa9..0xd4] and the stage-two window across [0x7e..0xb4] plus 0x29;
    three receipt fields (cd_exec_retry_branch_proven,
    cd_read_table_load_proven, stage2_seed_call_sites_proven) record the
    completeness, and the probe exercises every window against the real
    hash-verified media plus four byte-mutation rejections. JP patterns
    are document-attested identical and covered by the synthetic JP
    fixture. Semantics boundary unchanged: this binds instruction bytes
    only — no System Card base arithmetic (the DL=0x02 trace values stay
    trace-only), no record semantics, no graphics role. Remaining: the
    System Card base arithmetic is still trace-only, and any VDC/VCE
    destination binding requires runtime evidence; the post-$3800
    consumer chain remains capture-blocked.
  - Update 2026-07-20: the executed stage-two entry path is now
    contiguously bound end-to-end (1c15b6c16). The two remaining
    unbound windows in the proven stage-two sector (raw sector 1224 US =
    index01 225 + record 0x3e7) were disassembled from the hash-gated US
    media, matched exactly against the source-locked disassembly
    (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:107-181`),
    and bound as fail-closed static patterns via the new
    `theron_v1_track02_verify_stage2_entry_path` verifier: the entry
    prologue at user offset 0x00 (41 bytes: SEI/stack/MPR paging around
    the L8000 call, the L40B7 call, and the System Card entry calls up
    to the seed JSR) and the main path at user offset 0x2c (82 bytes:
    post-seed init, interrupt mask, L4B2D/L4B73 calls, TII clear/copy up
    to the retry-head BSR). Together with the already-bound 0x29 (JSR
    $40ae, 3 bytes), 0x7e (BSR, 2 bytes) and [0x80..0xb5), the entire
    executed entry path [0x00..0xb5) = 181 bytes is now contiguously
    bound, and the contiguity is compile-time asserted in the verifier.
    Scope boundary: the source-lock document attests JP/US byte identity
    only for the $4090 window, so this verifier is US-only — the JP
    variant is documentedly rejected
    (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until JP media
    is staged; the JP fixture keeps its variant-neutral window proofs.
    The probe exercises both windows against the real hash-verified US
    media plus two byte-mutation rejections (0x00 -> 0x00 restored to
    0x78; 0x2c -> 0x00 restored to 0x20) and the JP-scope rejection.
    Semantics boundary unchanged: this binds instruction bytes only — no
    System Card base arithmetic, no record semantics, no graphics role.
    Remaining: JP verification of the same stream awaits staged JP
    media; the L40B7+/call-graph continuations (e.g. L4814 in sector 2,
    L8000 in image sector 8) are future windows; the post-$3800
    consumer chain remains capture-blocked.
  - Update 2026-07-20: the first-tier call-graph continuations of the
    executed entry path are now byte-bound (337f0474e). Four callee
    bodies invoked from the contiguously bound stream [0x00..0xb5) were
    disassembled from the hash-gated US media, matched exactly against
    the source-locked disassembly
    (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:189-219,
    1207-1232, 1622-1632, 1661-1680`), and bound as fail-closed static
    patterns via the new `theron_v1_track02_verify_stage2_call_graph`
    verifier: the L40B7 command-dispatch loop at user offset 0xb7 (58
    bytes, called at 0x1e; its own L4814 call at 0xb9 sits inside its
    body), the L4B2D count-down delay at 0xb2d (15 bytes, called at
    0x52), the L4B73 st0/st1/st2 port clear at 0xb73 (35 bytes, called
    at 0x55), and the L4814 zero-page pointer setup at 0x814 (46 bytes,
    called from the dispatcher; its two da65 decode-artifact bytes at
    0x81f-0x820 are bound to the authenticated media bytes). The
    verifier asserts that every call site sits inside an already-bound
    window, and 154 continuation bytes are now bound. Scope boundary:
    the source-lock document attests JP/US byte identity only for the
    $4090 window, so this verifier is US-only — the JP variant rejects
    (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until staged
    JP media can verify the same streams; the JP fixture keeps its
    variant-neutral window proofs. The probe exercises all four windows
    against the real hash-verified US media plus four byte-mutation
    rejections and the JP-scope rejection. Semantics boundary
    unchanged: instruction bytes only — no System Card base arithmetic,
    no record semantics, no command meanings for the L410D dispatch
    table, no graphics role for the st0/st1/st2 writes. Remaining:
    JP verification of the same streams awaits staged JP media; the
    next-tier call-graph windows (L410D dispatch table [0x10d..0x11d),
    L4AF7, L4F5E, L383E, L8000 in image sector 8 with its da65 decode
    artifacts, L45A6, L4696) are future windows; the post-$3800
    consumer chain remains capture-blocked.
  - Update 2026-07-20: the L40B7 dispatch machine is now closed
    contiguously (c66b83798). The five remaining windows of the
    dispatch machine were disassembled from the hash-gated US media,
    matched exactly against the source-locked disassembly
    (`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm:183-235,
    1590-1594, 2334-2337`), and bound as fail-closed static patterns
    via the new `theron_v1_track02_verify_stage2_dispatch_machine`
    verifier: the register-seed tail at user offset 0xb5 (2 bytes,
    closing the gap between the executed entry path [0x00..0xb5) and
    the dispatcher body), the seven dispatch stubs at 0xf1 (28 bytes:
    shared LDA #imm / BRA L40E4 return tails selecting the
    stream-advance count 1..5, 7, 9), the ten-entry jump table at
    0x10d (20 bytes: little-endian handler addresses $41C5..$4253,
    strictly increasing, each verified to point inside the loaded
    image), the L4AF7 MPR-page body at 0xaf7 (9 bytes), and the L4F5E
    selector body at 0xf5e (8 bytes) — the dispatcher loop head's two
    direct callees (call sites 0xcc and 0xc1 inside the bound
    dispatcher window). The verifier range-checks every jump-table
    entry, asserts the [0x00..0x121) contiguity chain (entry path +
    seed tail + dispatcher + stubs + table) and the in-window call
    sites (0xc1, 0xcc, 0xeb), and 67 closure bytes are now bound.
    Scope boundary: US-only, as before — the JP variant rejects
    (`THERON_TRACK02_SIGNAL_NOT_FOUND`, invalid receipt) until staged
    JP media can verify the same streams. The probe exercises all five
    windows against the real hash-verified US media plus five
    byte-mutation rejections and the JP-scope rejection. Semantics
    boundary unchanged: instruction and table bytes only — no handler
    semantics for the ten jump-table targets, no System Card base
    arithmetic, no record semantics, no graphics role. New finding:
    L383E is NOT in the stage-two image — $383E lies below the $4000
    load address inside the dynamically loaded $3800 payload (record
    0x4e0 US), so its binding belongs to the dynamic-payload lane, not
    the stage-two image lane. Remaining: JP verification awaits staged
    JP media; the next-tier windows (the ten jump-table handler bodies
    at $41C5..$4253, L8000 in image sector 8 with its da65 decode
    artifacts — L45A6 [0x5a6..0x5ca) is cleanly decodable but its only
    caller L8000 is unbound, so they bind together, L4696 with its
    head-byte decode artifact, L3114, L383E in the dynamic payload)
    are future windows; the post-$3800 consumer chain remains
    capture-blocked.
  - Update 2026-07-20: the L8000/L45A6 callee pair is now byte-bound
    (job/w5, round 14 — see DONE.md same-date entry). The L8000 body
    [0x4000..0x40bc) at the head of image sector 8 (the entry path's
    first call, site 0x11 inside the bound prologue) and the L45A6
    body [0x5a6..0x5ca) (its only call site is the JSR at L8000+0x1c)
    bind together via the new
    `theron_v1_track02_verify_stage2_l8000_pair` verifier — 224 pair
    bytes, with the three flagged da65 decode-artifact spans (the
    $2211 STZ at +0x0b, the ADC $00 at +0x2a, the STA $47CE/LDA #$00
    at +0x4a..+0x4e) bound to the authenticated media bytes; the
    source-locked disassembly's zero-page-as-absolute renderings
    (L0000/L004C) are superseded by the media bytes. US-only, as
    before — the JP variant rejects until staged JP media can verify
    the same streams. Remaining: JP verification awaits staged JP
    media; the next-tier windows (the ten jump-table handler bodies
    at $41C5..$4253, L4696 with its head-byte decode artifact, L3114,
    L383E in the dynamic payload — the dynamic-payload lane) are
    future windows; the post-$3800 consumer chain remains
    capture-blocked.
  - Update 2026-07-20: the ten jump-table handler bodies are now
    byte-bound (job/w5, round 15 — see DONE.md same-date entry). The
    L410D table targets $41C5..$4253 form one contiguous span
    [0x1c5..0x254) (143 bytes) bound via the new
    `theron_v1_track02_verify_stage2_jump_table_handlers` verifier,
    matched instruction by instruction against the source-locked
    disassembly (theron-us-stage2-huc6280.asm:344-430). Three da65
    decode-artifact spans of the same class as round 14 (the BSR L41F8
    at 0x1d8 split into .byte/.byte, the LDA $2780,x at 0x1fc split
    into .byte/bra, the ADC $3008/STA $3009 at 0x225-0x22a split into
    .byte/php/bmi/ora) plus the 0x245 zero-page STA $20 rendered as the
    absolute label L0020 are bound to the authenticated media bytes.
    The entry chain is asserted: strictly increasing targets, first
    target at the span head, the last target's single RTS closing the
    span, and the JMP (L410D,x) table-read site (0xe1) inside the bound
    dispatcher window. US-only, as before — the JP variant rejects
    until staged JP media can verify the same streams. Remaining: JP
    verification awaits staged JP media; the next-tier windows (L4696
    with its head-byte decode artifact, L3114, L383E in the dynamic
    payload — the dynamic-payload lane) are future windows; the
    post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-20: the L4696/L3114 far-callee pair is now
    byte-bound (job/w5, round 16 — see DONE.md same-date entry) via
    the new `theron_v1_track02_verify_stage2_l4696_l3114` verifier —
    163 bytes across two windows. L4696's flagged head-byte decode
    artifact is resolved: da65's linear $4000-based map labels image
    offset 0x696 as L4696, whose head byte $33 is no HuC6280 opcode;
    the authenticated body [0x4696..0x46db) (69 bytes, image bank 2
    alongside its callers) matches da65's own L8696 decode
    (theron-us-stage2-huc6280.asm:10212-10248) instruction by
    instruction — the 16-bit shift-add multiply — with the L0011
    zero-page-as-absolute renderings superseded by the media bytes.
    L3114 [0x1114..0x1172) (94 bytes), declared `L3114 := $3114`
    absolute by da65 without a body decode (CPU $3114 lies below the
    linear map; the CPU $3xxx window shows image bank 0 at offset
    CPU-$2000, and unlike L383E its bytes are not clobbered by the
    $3800 dynamic-payload read), decodes cleanly from the media; its
    trailing RTS at 0x1171 sits immediately before the da65-declared
    L3172 entry, confirming the span. Call-site invariant: the L4696
    JSR (L8000+0x6a) inside the bound L8000 window, the L3114 JSR
    (L4F5E+4) inside the bound selector window. US-only, as before —
    the JP variant rejects until staged JP media can verify the same
    streams. Remaining: JP verification awaits staged JP media; the
    next-tier windows (L383E in the dynamic payload — the
    dynamic-payload lane; the unbound $45xx-tier callers of L4696;
    L3114's BSR/JSR callees L3172/$117D/$4F66/$526D/$55E0/$5213) are
    future windows; the post-$3800 consumer chain remains
    capture-blocked.
  - Update 2026-07-21: L3114's six BSR/JSR callees and the two
    $45xx-tier L4696 call sites are now byte-bound (job/w5, round 17 —
    see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_l3114_callees` verifier — 89 bytes
    across eight windows. All six callees bind in the stage-two image
    lane: L3172 [0x1172..0x117d) and the $117D far-helper trampoline
    [0x117d..0x118a) sit directly after the bound L3114 body in the
    low-image region (below $3800, never clobbered by the
    dynamic-payload CD_READ); L4F66 [0x0f66..0x0f7a) (the shared delay
    loop, directly after the bound selector window), L5213
    [0x1213..0x121f), L526D [0x126d..0x1280), and L55E0
    [0x15e0..0x15e8) are listed inline by da65 under its linear map
    (CPU = image + $4000, image banks 0/1), matching the media
    instruction by instruction (asm:2339, 2754, 2805, 3293). The six
    L3114 call-site invariants are compile-time-asserted at offsets
    +0x01/+0x08/+0x0e/+0x32/+0x4d/+0x56 inside the bound L3114 body,
    and the two $45xx-tier 3-byte JSR $4696 windows bind at image
    offsets 0x45ba/0x45cb (da65 asm:10107/10115) with the target
    compile-time-asserted to be the bound L4696 body. US-only, as
    before — the JP variant rejects until staged JP media can verify
    the same streams. Remaining: JP verification awaits staged JP
    media; the callee-of-callee windows ($117D's $5BF5/$5C8C/$5CB0/
    $5C25, L526D's L51F9, L55E0's L55F6/L55E8), the enclosing $45xx
    routine, and L383E in the dynamic payload are future windows; the
    post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-21: the seven tier-2 callees (the callees of the
    bound $117D trampoline, L526D, and L55E0) are now byte-bound
    (job/w5, round 18 — see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_l3114_tier2_callees` verifier —
    162 bytes across seven windows. All seven bind in the stage-two
    image lane (every image offset lies below $3800; da65 lists each
    body inline under its linear map, CPU = image + $4000): L51F9
    [0x11f9..0x1213) (26 bytes — da65's L5200 mid-instruction label
    artifact splits the `ror $0E` at 0x11ff-0x1200 into `.byte $66`
    plus garbage `asl $664A`/`asl $0F85` renderings, so the media
    bytes are authoritative; the body ends exactly at the bound L5213
    entry), L55E8 [0x15e8..0x15ef) (7 bytes, directly after the bound
    L55E0 body), L55F6 [0x15f6..0x1600) (10 bytes), L5BF5
    [0x1bf5..0x1c06) (17 bytes), L5C25 [0x1c25..0x1c69) (68 bytes),
    L5C8C [0x1c8c..0x1c9f) (19 bytes), and L5CB0 [0x1cb0..0x1cbf)
    (15 bytes) — each matching da65 instruction by instruction
    (asm:2740, 3299, 3310, 4174, 4205, 4261, 4279), with the relative
    branches (L5C25's BRA/BSR/BNE, L5C8C's BEQ L5C8C self-loop,
    L5CB0's BNE poll loop, L5BF5's BNE copy loop) verified against the
    media encodings. Call-site invariants compile-time-asserted: the
    four JSR opcodes at +0x00/+0x03/+0x06/+0x09 inside the bound
    $117D trampoline, the JSR L51F9 at +0x00 of the bound L526D body,
    and the two BSR opcodes at +0x00/+0x02 of the bound L55E0 body.
    US-only, as before — the JP variant rejects until staged JP media
    can verify the same streams. Remaining: JP verification awaits
    staged JP media; the tier-3 windows (L5C06/L5C9F/L536E/L5439/
    L5C69/L54A0/L5600, the LE063 far-call targets, L5C25's L5C2C
    alternate entry, the L5C20 table), the enclosing $45xx routine,
    and L383E in the dynamic payload are future windows; the
    post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-21: the seven tier-3 windows plus the L5C20 table
    and L5C25's L5C2C alternate entry are now byte-bound (job/w5,
    round 19 — see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_l3114_tier3_callees` verifier —
    223 bytes across eight windows. All bind in the stage-two image
    lane (da65 inline under its linear map, CPU = image + $4000, all
    offsets below $3800): L5C06 [0x1c06..0x1c20) (26 bytes, the
    L4FD1/L4FD2 toggle with its JSR L5C25 / JSR L5C2C pair), the
    L5C20 table [0x1c20..0x1c25) (5 zero bytes as loaded), L5C69
    [0x1c69..0x1c8c) (35 bytes, self-modifying — STA L5C7E rewrites
    the ORA/AND opcode at 0x1c7e; the media bytes are the as-loaded
    image, BNE d0 f4 resolves to L5C7B), L5C9F [0x1c9f..0x1cb0) (17
    bytes, ends exactly at the bound L5CB0 entry), L536E
    [0x136e..0x13c4) (86 bytes, the $0E:$0F multiply-accumulate and
    $DFF0 bounds compare), L5439 [0x1439..0x1455) (28 bytes, the
    null-pair early-out), L54A0 [0x14a0..0x14af) (15 bytes) and L5600
    [0x1600..0x160b) (11 bytes) — the media confirms da65's `a:$02`/
    `a:$03` absolute-store renderings ($8D $02 $00), both bodies
    ending exactly at the next da65 labels L54AF/L560B. The L5C2C
    alternate entry sits at +0x07 inside the already bound L5C25
    window (offset compile-time-asserted, no new bytes). Call-site
    invariants compile-time-asserted: two JSR opcodes inside the
    bound L5C8C body (+0x00/+0x05), the JSR L536E / BSR L5C69 /
    JSR L5439 inside the bound L5C25 body (+0x0c/+0x2c/+0x3c), the
    JSR L54A0 / BSR L5600 inside the bound L55F6 body (+0x02/+0x05),
    and the LDA $5C20,x data site inside the bound L5BF5 body (+0x03)
    targeting the bound L5C20 table. US-only, as before — the JP
    variant rejects until staged JP media can verify the same
    streams. Remaining: JP verification awaits staged JP media; the
    tier-4 windows (L4F7A, L542D, L5482, L5492, L535E, L5455, L53C4,
    L54AF, L560B, the LE063 far-call targets), the enclosing $45xx
    routine, and L383E in the dynamic payload are future windows; the
    post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-21: the nine tier-4 windows are now byte-bound
    (job/w5, round 20 — see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_l3114_tier4_callees` verifier —
    279 bytes across nine windows. All bind in the stage-two image
    lane (da65 inline under its linear map, CPU = image + $4000, all
    offsets below $3800): L4F7A [0x0f7a..0x0f89) (15 bytes, the
    L4FD4-indexed X/Y delay nest, starting where the bound L4F66
    ends), L535E [0x135e..0x136e) (16 bytes, the ($04),y pair loader —
    media confirms da65's `a:$02`/`a:$03` absolute stores; ends at the
    bound L536E), L53C4 [0x13c4..0x1403) (63 bytes, the L51F9-driven
    record writer with its L5403 row loop), L542D [0x142d..0x1439)
    (12 bytes, the $04:$05 +6 fix-up, trailing RTS at 0x1438 = da65's
    L5438), L5455 [0x1455..0x1482) (45 bytes), L5482 [0x1482..0x1492)
    (16 bytes), L5492 [0x1492..0x14a0) (14 bytes), L54AF
    [0x14af..0x14c5) (22 bytes, the L4F9F-indexed L4FA0,x pair
    store), and L560B [0x160b..0x1657) (76 bytes, the 9-row $5667
    copy setup — da65's L563D mid-instruction label artifact splits
    the BCC operand at 0x163d into `.byte $90` plus garbage `st0
    #$EE`/`cld`/`.byte $4F` renderings, so the media bytes are
    authoritative: BCC L5641 / INC $4FD8; the L0000 zero-page-as-
    absolute renderings likewise superseded). The contiguous chain is
    compile-time-asserted: L4F66->L4F7A, L535E->L536E, L542D->L5439->
    L5455->L5482->L5492->L54A0->L54AF, L5600->L560B. Call-site
    invariants compile-time-asserted: the JSR L4F7A at +0x09 inside
    the bound L5C9F body and the JSR L5492 at +0x1f inside the bound
    L5C69 body. US-only, as before — the JP variant rejects until
    staged JP media can verify the same streams. Remaining: JP
    verification awaits staged JP media; the tier-5 windows (L5403,
    L541E, L52A2, L52C8, L5657, L54C5, the LE063 far-call targets,
    the L5657-tail data), the enclosing $45xx routine, and L383E in
    the dynamic payload are future windows; the post-$3800 consumer
    chain remains capture-blocked.
  - Update 2026-07-21: the enclosing $45xx routine is now byte-bound
    (job/w5, round 20 — see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_enclosing_45xx` verifier — 185
    bytes, one window [0x45b1..0x466a) holding the two round-17
    JSR $4696 sites at +0x09/+0x1a (offsets compile-time-asserted).
    da65 lists the body inline (asm:10102-10191, no entry label; its
    L8610 loop head sits at +0x5f); the L0011/L0000 zero-page-as-
    absolute and L466A data-label artifact classes are superseded by
    the media bytes; the entry CPU address is not pinned (no bound
    caller), so the receipt carries 0. Remaining: JP verification
    awaits staged JP media; the routine's six callees (L4552, L4932,
    L458E, L424B, L466B, L43D6), the tier-5 windows (L5403, L541E,
    L52A2, L52C8, L5657, L54C5, the LE063 far-call targets, the
    L5657-tail data), and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-21: the six $45xx-routine callees are now
    byte-bound (job/w5, round 21 — see DONE.md same-date entry) via
    the new `theron_v1_track02_verify_stage2_enclosing_45xx_callees`
    verifier — 325 bytes across six windows, all in the stage-two
    image lane (image bank 2, alongside their bound caller): L424B
    [0x424b..0x42bf) (116 bytes, with its BSR-local subroutine at
    +0x5b — da65's L82A6; da65's `.byte $85`/`bpl $821C` head
    mis-split and its L0011/L0000 zero-page-as-absolute renderings
    superseded by the media bytes), L43D6 [0x43d6..0x4417) (65
    bytes), L4552 [0x4552..0x458e) (60 bytes, ends at L458E), L458E
    [0x458e..0x45a6) (24 bytes), L466B [0x466b..0x4696) (43 bytes,
    self-modifying TIA setup — media bytes are the as-loaded image;
    ends at the bound L4696 body), and L4932 [0x4932..0x4943) (17
    bytes). CPU entry addresses are pinned by the JSR operands inside
    the bound $45xx body (the round-16 L4696 class); the $45xx
    call-site offsets (+0x25/+0x35/+0x42/+0x6d/+0x87/+0xb1), the
    internal JSR L43D6 at L424B+0x02, and the L4552->L458E /
    L466B->L4696 adjacencies are compile-time-asserted. US-only, as
    before — the JP variant rejects until staged JP media can verify
    the same streams. Remaining: JP verification awaits staged JP
    media; L424B's callees (L43A1, L42BF), the unbound gap streams
    ([0x45a6..0x45b1) TII routine, the $4417 stream, the TMA/PHA
    stream at $4943), the tier-5 windows (L5403, L541E, L52A2, L52C8,
    L5657, L54C5, the LE063 far-call targets, the L5657-tail data),
    and L383E in the dynamic payload are future windows; the
    post-$3800 consumer chain remains capture-blocked.
  - Update 2026-07-21: the seven tier-5 windows are now byte-bound
    (job/w5, round 22 — see DONE.md same-date entry) via the new
    `theron_v1_track02_verify_stage2_l3114_tier5_callees` verifier —
    208 bytes across seven windows, all in the stage-two image lane
    (da65 inline under its linear map, CPU = image + $4000, all
    offsets below $1800): L5403 [0x1403..0x141e) (27 bytes, the
    `a:$02`/`a:$03` absolute-load pair copy with BSR L541E and BSR
    L5492), L541E [0x141e..0x142d) (15 bytes, the ST0 #$01/#$02 VDC
    address setup — ends at the bound L542D), L52A2 [0x12a2..0x12c8)
    (38 bytes), L52C8 [0x12c8..0x12da) (18 bytes, da65's L0000
    zero-page-as-absolute superseded), L5657 [0x1657..0x1667) (16
    bytes, the 8-byte SXY copy loop), L54C5 [0x14c5..0x14db) (22
    bytes, called only from the unbound L54DB stream), and the L5667
    data table [0x1667..0x16af) (72 bytes, 9 rows x 8 read through
    the bound L560B $00:$01 setup — da65 garbage-decodes it as code,
    media authoritative; the L5C20-table class). Call-site invariants
    compile-time-asserted: the BSR L5403 at +0x2c inside the bound
    L53C4 body; the BSR L5657 at +0x1f, JSR L52A2 at +0x21, JSR
    L52C8 at +0x25, and the L5667 data site at +0x0e inside the bound
    L560B body; the L5403->L541E->L542D, L52A2->L52C8, and
    L5657->L5667 adjacencies. US-only, as before — the JP variant
    rejects until staged JP media can verify the same streams.
    Remaining: JP verification awaits staged JP media; L52C8's L52FD
    callee, L52DA, L54DB, L424B's callees (L43A1, L42BF), the unbound
    gap streams ([0x45a6..0x45b1) TII routine, the $4417 stream, the
    TMA/PHA stream at $4943), the LE063 far-call targets, and L383E
    in the dynamic payload are future windows; the post-$3800
    consumer chain remains capture-blocked.
  - Update 2026-07-21: L424B's callees and the $45A6 TII gap stream
    are now byte-bound (job/w5, round 23 — see DONE.md same-date
    entry) via the new
    `theron_v1_track02_verify_stage2_45xx_tier2_callees` verifier —
    92 bytes across three windows, all in the stage-two image lane
    (image bank 2, da65 inline $83A1/$82BF/$85A6 renderings): L43A1
    [0x43a1..0x43d6) (53 bytes, the $14:$15 shift-add with its
    L47CB/L47CC and $58 adds — ends exactly at the bound L43D6),
    L42BF [0x42bf..0x42db) (28 bytes, the $56 $10-counter with its
    L47C4 save/JSR L43D6/restore — the internal JSR L43D6 at +0x14),
    and the $45A6 TII gap stream [0x45a6..0x45b1) (11 bytes, STZ
    L47B8 / TII $47B8,$47B9,$00A7 / RTS — called only from the
    unbound $401C stream at image 0x401c, so its entry CPU address
    carries 0; ends exactly at the bound $45xx routine). CPU entry
    addresses for L43A1/L42BF are pinned by the JSR operands inside
    the bound L424B body (+0x19/+0x45 and +0x2d/+0x55, the round-16
    L4696 class); the L43A1->L43D6 and $45A6->$45xx-routine
    adjacencies are compile-time-asserted. US-only, as before — the
    JP variant rejects until staged JP media can verify the same
    streams. Remaining: JP verification awaits staged JP media; the
    $3B75 stream (after L42BF), the $4417 stream (called from the
    unbound $4228 stream), the $4943 stream (no JSR caller —
    jump-table/handler entry), L52FD, L52DA, L54DB, the LE063
    far-call targets, and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
- 🔧 2026-07-13 dynamic Track 02 RAM receipt: the instrumented original
  Mednafen route now requires a 32-byte FNV-1a receipt from System Card
  destination `$3800` immediately after the authenticated dynamic `CD_READ`
  returns. This proves record-to-RAM transfer but does not identify a Track 02
  source byte, decompressor, palette, VCE word, VDC transfer, level, or object
  family. Next evidence must tie that exact destination span to a hash-verified
  source sector and follow its bytes through one original VCE/VDC operation.
- 2026-07-16 update: the Track02 loader-intake chain now has a
  post-predecode-to-dungeon-level gate that preserves object/dungeon
  read-window topology only when it can also consume the source-locked initial
  level handoff for the same JP/US Track02 media. Missing raw media produces
  an explicit no-fallback blocker, and the positive branch remains conditional
  on `FIRESTAFF_THERON_TRACK02_RAW`. Remaining work is still real original
  loader/CD-read evidence that assigns a verified object-table or
  dungeon-record grammar before runtime/render admission.
- 2026-07-16 update: a grammar-admission barrier now consumes that
  dungeon-level topology receipt and preserves the original CD-read record,
  byte-window, hash, and topology evidence while explicitly requiring a future
  original object-table/dungeon-record grammar witness. It admits no grammar,
  decoder, runtime, rendering, fallback visual, or synthetic byte path.
  Remaining work is a real HuC6280/System Card trace that follows one of these
  exact windows into the original object or dungeon parser.
- 2026-07-16 update: the grammar boundary now also binds back to the
  read-table/layout-binding receipt, so a positive real-media path must
  preserve the exact original CD-read records, MODE1 user-data offsets,
  destinations, byte windows, copied-byte hashes, and topology hash before it
  can reach the grammar-witness-required blocker. Remaining work is still the
  original parser trace itself; this gate deliberately admits no object-table
  fields, dungeon-record grammar, runtime handoff, rendering, fallback visuals,
  or synthetic bytes.
- 2026-07-16 update: a parser-witness gate now admits object-table and
  dungeon-record grammar provenance only when supplied original trace facts
  prove that the original loader/parser consumed those exact preserved
  CD-read windows. Even that positive receipt keeps object fields, dungeon
  record fields, decoder semantics, runtime handoff, rendering, fallback
  visuals, and synthetic bytes blocked. Remaining work is to source such
  witness facts from a real HuC6280/System Card trace instead of a caller
  supplied receipt.
- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds. **2026-07-23 update (Lane E, cycle 11):** new `firestaff_theron_v1_mechanics_playability_probe` loads the authentic JP/US Track 02 Hall-of-Records level-0 grid and verifies movement, turning, wall blocking, and floor movement on the real 32×27 loader-accepted grid (36/36 PASS on staged TQUS02.bin + TQJP02.bin). **2026-08-06 update:** the real-data thing-data regression now discovers the supplied standard `~/.firestaff/data/theron/TQUS02.bin` path (or `FIRESTAFF_THERON_TRACK02_RAW`) before the legacy fixture path and verifies all seven dungeon object/text regions: AKUTUBA 228 ground refs/1021 items, DRATOR 249/969, FORMICIA 224/871, SARMON 226/1132, SHADODAN 264/980, THIEVES 255/988, DEMON 190/881. The loader also rejects non-sector-aligned raw input. Remaining work is broader real-asset gameplay traces for doors, pits, teleporters, altar, combat, drops, and sounds once those object semantics are source-locked.
- 2026-08-06 update: the real-data map, ground-reference and door/teleporter regressions now discover `FIRESTAFF_THERON_TRACK02_RAW` or standard `~/.firestaff/data/theron/TQUS02.bin` before the legacy fixture path. Against the supplied US BIN they verify all seven map groups, 4, 8, 5, 6, 3, 4 and 4 maps respectively; all seven ground-reference chains; and all seven door/teleporter tables. JP-specific map offsets remain a separate source-format gap and are not inferred from the US table.
- 2026-08-06 update: the same real-media discovery now covers text codon, actuator and creature regressions; all seven US dungeon regions pass. Text output remains an authenticated decode candidate, not parity-approved presentation text. The full runtime dungeon-loader test correctly remains legacy-only because real ground chains still contain unbound item categories; its fail-closed `-1` result must not be replaced by synthetic object kinds.
- 2026-08-06 update: Track 02 raw-media intake now parses `FILE`, `TRACK`, and
  related CUE directives case-insensitively, matching the CUE format instead of
  depending on one editor's capitalization. A real-data regression builds a
  temporary CUE around the supplied `TQUS02.bin`, verifies the US pregap/index
  at raw sector 225, the authenticated BIN MD5, and trace preparation. The
  remaining intake gap is broader real CUE/BIN/ISO corpus coverage, not a
  generated fixture.
- 🔧 2026-08-06 Theron drop-placeholder removal: the old category-to-item
  resolver accepted synthetic item IDs and a host seed, then presented a
  guessed weapon, armour, consumable, scroll, or key as a real drop. The
  category table remains a verified item-name/category receipt, but no drop
  can be admitted until the original T900 consumer and selection record are
  decoded from Track 02. `theron_v1_drop_loot()` already fails closed at that
  boundary; the obsolete resolver and its positive fixture assertions are
  removed. Next evidence is a real T900 drop record plus its consumer.
- 🔧 2026-08-05 Theron production combat boundary: removed the inferred
  `theron_v1_compat.c` implementation from the `firestaff_theron` library.
  Production now uses the existing fail-closed adapter, so creature speed,
  AI, attack/defense formulas, spell combat, drops and sound IDs cannot be
  published from guessed records. Compatibility mechanics remain explicit in
  fixture/probe targets. The next replacement is still the authenticated
  Track 02 T500/T600/T900 consumer, not a new host-side table.
- 🔧 2026-08-05 Theron static consumer receipt: the authenticated US Track 19
  image now has a byte/MD5-locked regression for bank `$1f` `$243e–$24c3`.
  It proves the existing HuC6280 bitstream/register-map fragment against the
  real `TQUS19.iso` and explicitly records that the `$2600` consumer is absent
  from static ROM. The next step remains a real post-CD RAM capture with PC
  and source-LBA provenance; no RAM bytes or level/object semantics are
  inferred from this receipt.
- 🔧 Startup presentation hardening: stage/Soul Room render rows, enriched startup layout labels, and Track 02 descriptor-role receipt summaries are now test-visible; remaining work is real Track 02 startup art/audio decoding and pixel evidence instead of fallback text presentation.
- ✅ 2026-07-22: Theron boot now owns the runtime input/idle facade (`theron_v1_boot_runtime_handle_m12_input` / `theron_v1_boot_runtime_handle_idle_tick`); M11 Track 02 runtime path no longer calls `theron_v1_boot_runtime_tick_world`, `theron_v1_boot_runtime_turn_party`, or `theron_v1_boot_runtime_move_party` directly. Regression test `test_theron_v1_boot_runtime_input` passes 12/12; related probes (`theron_v1_rendering`, `theron_v1_startup_flow_probe`, `theron_v1_m11_direct_launch`, `m11_phase_a`) pass.
- ✅ 2026-07-23: Theron boot now owns the startup host-receipt apply facade (`theron_v1_boot_apply_startup_host_receipt`); M11 no longer maps `Theron_StartupHostReceipt` fields to `m11_set_status`, `m11_set_inspect_readout`, `m11_log_event`, or input-result actions directly. M11 provides a small callback table (`set_status` / `set_inspect` / `log_event`) and the facade owns the decision order and semantics. This closes the chapter-inspect wiring and startup host-receipt apply items from the 2026-07-22 remaining work. Regression test `test_theron_v1_boot_host_receipt` passes 14/14; related probes (`theron_v1_rendering`, `theron_v1_startup_flow_probe`, `theron_v1_m11_direct_launch`, `theron_v1_m11_launcher_handoff_boundary`, `m11_phase_a`) pass.
- ✅ 2026-07-23: Theron boot now owns the startup action/state-receipt apply facade (`theron_v1_boot_apply_startup_action_host_receipt`); M11 no longer applies `Theron_StartupStateReceipt` field updates or drives the Track 01 CDDA lifecycle update directly after a startup action. M11 provides a callback table that adds `apply_state_receipt` and `update_track01_cdda_lifecycle` hooks to the existing status/inspect/log hooks; the facade owns the apply order (state receipt first, then host receipt, then CDDA lifecycle). This closes the remaining M11 decoupling gap from the 2026-07-23 host-receipt work. Regression test `test_theron_v1_boot_host_receipt` expanded to 20/20; related probes (`theron_v1_rendering`, `theron_v1_startup_flow_probe`, `theron_v1_m11_direct_launch`, `theron_v1_m11_launcher_handoff_boundary`, `m11_phase_a`) pass.
  - 2026-07-08 update: Theron boot now owns the runtime dungeon/UI/V2-HUD/present render frame facade. M11 no longer calls `theron_vp_render_dungeon`, `theron_vp_render_ui`, V2 HUD render, or `theron_vp_present` directly in the Track 02 runtime path.
  - 2026-07-08 update: Theron boot now owns runtime ownership release for profile/world/viewport/assets, and M11 shutdown no longer frees those Track 02 objects directly.
- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, shop price-table regressions, and data-free cross-slot export/import are green. Remaining work is a real Track 02 save artifact import/export pass when such a save is available.
### Theron V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`theron_v2_texture_upscale_pc34.c` provides `theron_v2_epx_upscale` indexed→RGBA via PCE palette). The Theron V2.2 manifest parser remains available for fixture inspection, but production now requires `source_provenance="authenticated_track02"`; the existing procedural/gpt-image-2 pack is explicitly rejected as real data. Remaining: obtain source-owned Track 02 bitmap/material records and bind them before enabling V2.2 art.
- 🔧 **2026-06-27 Theron V2 Phase 3 initial seed landed (presentation-only, data-free):** `theron_v2_hud_overlay_pc34.c/.h` is the Theron-specific sibling of `csb_v2_hud_overlay_pc34.c` + `dm2_v2_hud_overlay.c`. New CTest `theron_v2_phase3_hud_overlay_probe` (40/40 PASS, labels `tier2;theron;v2;phase3;hud;presentation-only`) covers the phase-gate + presentation-mode selector contract (V1_FAITHFUL → no HUD overlay, V20_FILTERED / V21_UPSCALED / V22_MODERN → HUD active), all 6 setters (direction, quest items, dungeon progress 1/7, relic counter 0/7, spell-rune ready indicator, 4-champion bars), render into a 256×224 indexed framebuffer, V1 chrome preservation when V2 inactive, source evidence citations (THQUEST.ASM T520/T560/T600/T700/T800/T900 + HuC6260/HuC6270 + dmweb Theron 7 dungeons + 7 relic goals + sibling csb/dm2 modules), and null safety. Companion smoke test `theron_v2_hud_overlay_pc34` (58/58 PASS, CTest `theron_v2_hud_overlay_pc34`) covers init/reset, hit-flash decay, low-HP pulse trigger, top-bar / stats-bar / action-strip visibility toggles, and per-region pixel-write assertions (compass / quest / dungeon / relic / champion bars / action strip all paint when active, and `visible=0` or `opacity=0` writes zero pixels). HUD surface: top-bar (compass + quest items + dungeon progress 1/7 + relic counter 0/7 + spell-rune ready indicator), bottom-panel (4 champion mini-bars HP/Stamina/Mana with Theron-as-leader at slot 0), and bottom action strip (ATK/CST/USE/DRP/MOV with active underline and hit-flash). Theron-specific surfaces (PC Engine 256×224 indexed fb, HuC6260 VDC layout, 7 dungeons + 7 relic goals, rune magic ready indicator) are mirrored from `dm2_v2_hud_overlay.c` + `csb_v2_hud_overlay_pc34.c`. **2026-06-27 Phase 3 placeholder-vs-real asset gate landed:** `theron_v2_hud_widget_assets_pc34.c/.h` is the Theron-specific sibling of `dm2_v2_hud_widget_assets` (the original Phase 3 gate pattern). New CTest `theron_v2_hud_widget_assets_pc34` (105/105 PASS) and headless probe `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS, labels `tier2;theron;v2;phase3;hud;widget-assets;presentation-only`) cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with the NO_MANIFEST-by-default baseline matching the current runtime. Slot table (7 slots, stable order, ordinals = indices): 5 Phase 3 primary (`compass_rose`, `quest_items`, `dungeon_progress`, `relic_counter`, `rune_indicator`, category `hud_widgets`) + 2 chrome supporting (`champion_bars`, `action_strip`, category `hud_chrome`). Manifest schema `{ id, generator, source_file, width, height }` aligned with sibling `theron_v22_modern_assets_pc34` and `dm2_v2_hud_widget_assets` shapes; manifest path `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`. Companion source-lock doc `docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md` documents the slot table, schema, gate state machine, M12/Phase 7 integration points, and honest boundary. Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview, `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`, sibling `dm2_v2_hud_widget_assets.h`. **2026-06-28 runtime handoff landed:** M11 now calls `theron_v2_hud_render()` in the live Theron Track 02 render path after `theron_vp_render_ui()` and before `theron_vp_present()`, gated by non-V1 presentation mode. **2026-06-29 overlay seed gate landed:** `theron_v2_hud_seed_from_v1_world()` now owns the V1-world snapshot mapping and returns explicit `V1_SKIPPED` / `V2_READY` states; `firestaff_theron_v2_overlay_seed_gate_probe` covers V1 hidden/no-paint behavior, V2 field mapping, byte-identical synthetic V1 world state before/after seeding and rendering, deterministic framebuffer output, gate-name stability, and NULL safety. **Remaining Phase 3 work:** (a) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`, (b) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator ≠ "placeholder"` so the gate can promote to `PARTIAL`/`COMPLETE`, and (c) real-art visual verification + per-region pixel gates against real Track 02 captures.
- ❌ Phase 4 - Enhanced lighting/effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- 🔧 Phase 6 - Touch/controller ergonomics: **2026-06-29 initial Theron-specific input seed landed (presentation-only, data-free):** `theron_v2_touch_controller_affordance.c/.h` maps Theron V2 touch swipes, edge-strafe, D-pad, left-stick, and right-stick affordances onto the shared DM1-family C001-C006 command ids while rejecting every affordance when V2 presentation is off; `theron_v2_touch_runtime.c/.h` translates accepted affordances into `Dm1V1QueuedCommandPc34Compat` entries and adds a Theron 256x224 HUD-chrome exclusion gate for touch starts on the V2 top bar, champion mini-bars, and action strip while controller inputs bypass the framebuffer coordinate gate. New CTest `theron_v2_touch_controller_affordance` (267/267 PASS) and probe `theron_v2_touch_runtime_probe` (138/138 PASS) are data-free and source-locked against THQUEST.ASM T520/T560/T600 plus ReDMCSB DEFS.H:238-243, COMMAND.C:2045-2155, CLIKMENU.C:142/180, and GAMELOOP.C:164-219. Shared M11 SDL gamepad routing now exists; remaining Theron-specific work is a real touch-layout target-size audit across launcher/game views and real Track 02 runtime proof.
- ❌ Phase 7 - V2 verification suite.

## Theron Track 02 remaining evidence

- [ ] THERON-V1-TRACK02-LIVE-LOADER-CONSUMER: the latest replay against the
  authenticated US Track 02 ISO now gives a real HuC6280 loader witness
  (`$2286` `TIA` followed by 13 block transfers, 24 RTS and 24 post-RTS rows)
  plus 4,096 static-bank consumer reads and an executed `$2c54–$2c69`
  code-window receipt. The parser now accepts this richer real trace. It still
  has no `$2600` dynamic consumer bytes, no VDC VRAM/VCE snapshot, and no
  source-owned level/object field decisions, so visual runtime drawing and
  source-consumer correlation remain blocked. The interactive forcefield route
  now admits a source-only map/thing handoff from authenticated raw BIN data;
  it does not promote VDC/VCE pixels, host item semantics, or guessed field
  meanings. Next evidence is a capture that reaches the game-owned post-CD
  consumer and closes the VDC snapshot on clean exit.

- 2026-08-06 update: the Track 02 thing-category enum is now source-bound to
  the retail order used by DMBUILDER6 (`4=monster`, `5=weapon`, `6=clothing`,
  `7=scroll`, `8=potion`, `9=chest`, `10=misc`, `14=missile`, `15=cloud`).
  A real US Track 02 regression now checks all seven dungeon object-count
  tables and requires nonzero copied payload for every populated category.
  This is raw record provenance only; runtime item/monster publication and
  combat/render semantics remain closed until their consumers are bound.
- 2026-08-06 update: categories 4–10 now have a portable little-endian raw
  record decoder. It binds the two-byte next-reference prefix and the
  DMBUILDER field layouts for monsters, weapons, clothing, scrolls, potions,
  chests, and misc across every populated record in the real US corpus.
  Categories 14/15 now use the same source decoder for their six-/two-byte
  payloads; no item is published into the runtime object model yet.
- 2026-08-06 update: the full Track 02 dungeon loader now consumes those
  source-bound records and follows their authentic next-reference chains on
  all seven US dungeons. It reports decoded/unbound records separately and
  leaves `Theron_V1_Object` untouched for categories whose host owner is not
  proven. The remaining handoff is the original object-kind/item-index
  consumer, not raw media intake or chain traversal.
- 2026-08-06 update: each real Track 02 map header now survives the world
  handoff as an exact verified receipt (`x/y` offsets, opaque bytes, XP and
  door bytes, map id and creature count). These fields remain semantic
  read-only evidence; seed, spawn direction and object-kind publication stay
  closed pending the original consumers.
- 2026-08-06 update: the same world handoff now retains each real map's
  `creature_gfx_bank` and cumulative column thing-count from the Track 02 map
  directory. They remain raw level-record evidence; no creature graphics or
  object semantics are inferred from either field.
- 2026-08-06 update: per-dungeon Track 02 world state now also retains the real
  twelve-entry thing-descriptor-size table and aggregate column count from the
  map directory. This is source layout provenance only; object-kind mapping,
  graphics ownership and runtime publication remain closed.
- 2026-08-06 update: every real category 4–10, 14 and 15 occurrence now carries
  both its exact raw bytes and the decoded source record (including missile and
  cloud payload fields) through the full-dungeon handoff. Host object-kind,
  inventory and projectile/cloud ownership remain deliberately unbound; no
  synthetic object is created.
- 2026-08-06 update: the real-data thing-record regression now covers both
  authenticated `TQUS02.bin` and `TQJP02.bin`. All seven Japanese dungeon
  blocks use their source-bound map/item offsets, retain 871–1 132 records per
  dungeon and decode every populated category without publishing a host
  object. Japanese text remains at zero until its codon consumer is proven;
  no translated or synthetic text is inserted.

## Theron Track 19 remaining evidence

- 2026-08-06 update: the authenticated 32x27 Track 19 startup-level record now
  survives the file-inventory handoff with its six raw header words, payload
  size/nonzero count and payload FNV-1a. This remains a source receipt only;
  tile, object and later-level semantics still require the original consumer.

- 2026-08-06 update: the real US and JP Track 19 startup envelope now has a
  bounded structural reader: big-endian 32×27 dimensions, six retained raw
  header words, and an 864-byte borrowed payload span are checked against the
  authenticated envelope hash. The payload remains opaque; tile/object
  ownership and later-level consumer semantics still require disassembly.

- 2026-07-15: Runtime level-bank selection now retains the authenticated
  startup bitmap's Track 02 MD5 and raw/user-data sector envelope. Remaining:
  obtain original loader/CD-read evidence that binds a post-startup bitmap or
  object-table record to a concrete runtime consumer. Do not infer palette,
  layout, object fields, or draw behavior from the retained startup envelope.
  - Update: the observed `$0b52` CD-read now reaches runtime as an opaque,
    hash-bound record receipt. It is explicitly not a level/object decoder.
    Remaining: capture a game-owned post-`$3800` consumer that identifies a
    record boundary and semantic role before any level or object publication.
  - Update: the authenticated initial-envelope boundary and its directly
    adjacent opaque continuation now require matching loader receipts and raw
    Track 02 checksums before runtime admission. This proves byte ownership,
    not a level grid or object table. Remaining: a captured game consumer
    must establish either grammar before promotion.
  - Update: runtime now retains the exact verified envelope and directly
    adjacent continuation bytes after rehashing both spans and enforcing their
    documented adjacency within the original `$0b52` record. The continuation
    remains explicitly opaque, not an object table. Remaining: capture a
    game-owned post-`$3800` consumer that identifies its level/object grammar
    before any semantic publication or drawing.
  - Update: capture intake can now bind one authenticated CD-to-game-RAM byte
    specifically to the directly adjacent continuation and reject bytes before
    or outside it. This proves an observed source-consumer overlap, not an
    object table. Remaining: obtain a real post-`$3800` transcript with a
    complete consumer sequence and independently identify record grammar.
  - Update: the same intake can now require an ordered 12-byte continuation
    prefix from one CD dispatch. It binds the exact source bytes and rejects a
    split capture chain, while retaining no object-table semantics. Remaining:
    capture the original consumer's field accesses and control decisions to
    establish a grammar separately from this byte boundary.
  - Update: the instrumented original Mednafen `TII` producer can now bind a
    post-`$3800` block transfer whose source starts exactly at `$3c80`, the
    continuation start in the verified loader sector. The destination and its
    content remain opaque. Remaining: capture the real `TII` row and the
    subsequent game-owned reads/control flow that establish record grammar.
  - Update: the live Mednafen capture script now preserves the separate
    main-RAM-loader trace and reports both all `TII` operations and `$3c80`
    candidates in its transition receipt. No candidate is generated when the
    original run does not execute one. Remaining: stage real media to acquire
    an admissible TII row and its later consumer evidence.
  - Update: the TII admission route now imports only an explicit bounded
    main-RAM-loader sidecar file, so a later real capture can feed the exact
    `$3c80` transfer receipt without reformatting trace text. Missing, empty,
    oversized, or unmarked files reject. Remaining: authentic transfer and
    follow-on consumer capture, not a synthetic sidecar.
  - Update: the Track 02-derived TII destination now requires an observed
    main-RAM `JSR` entry at exactly that target. This extends only the
    original-byte-to-execution chain; it does not identify a routine, data
    table, dungeon, object, palette, bitmap, or rendering role. Remaining:
    capture a real data read or subsequent call from the entered destination.
  - Update: destination-entry admission now also checks that the executed
    opcode equals the first byte copied by the authenticated TII from Track
    02-derived source address `$3c88`. This is byte provenance only, not a
    decoder, routine name, or level/object grammar. Remaining: a real trace
    must observe the entered routine's next data read or call.
  - Update: the immediate entry-successor now also has to remain inside that
    same copied TII span and match its mapped original Track 02 byte. This
    proves two bounded execution bytes, not instruction length, routine role,
    record format, or dungeon semantics. Remaining: observe a real CD-read,
    data access, or call from this copied routine.
  - Update: the Mednafen producer now records the next actual main-RAM step
    after that successor; admission keeps it only when it stays within the
    same TII destination and matches original byte `$3c8a`. This is execution
    provenance, not a control-flow interpretation or record-selection proof.
    Remaining: capture a source-observed call, CD read, or data access from
    the copied routine.
  - Update: the original US Track 02 bytes at the copied entry identify a
    HuC6280 `BRA` opcode with a source-bound displacement. The producer now
    emits its computed target, and admission requires exact opcode,
    displacement, and target agreement. This proves one real control transfer
    only; the branch target has no asserted loader, record, level, object, or
    graphics meaning. Remaining: capture the first source-observed data read
    or CD state after that target.
  - Update: the trace now admits the BRA only when Mednafen subsequently
    fetches its exact computed target in main RAM. The fetched opcode remains
    opaque control evidence, with no asserted loader, record, level, object,
    palette, bitmap, or rendering meaning. Remaining: capture a source-bound
    data read, call, or CD state after this executed target.
  - Update: the producer now binds the first observed `JSR` after that
    executed target to the same main-RAM control chain. Its destination remains
    opaque, not a loader name or a record, level, object, palette, bitmap, or
    rendering claim. Remaining: observe a CD state or source-data transfer
    from this control path before assigning record-selection semantics.
  - Update: a post-BRA JSR receipt now requires an observed CD data-register
    write at the JSR target, followed by one matching READ(6)/FIFO byte whose
    LBA maps to a hash-verified Track 02 record. This is control-to-media
    provenance only; it does not name the record or promote level, object,
    palette, bitmap, or rendering semantics. Remaining: authenticate a real
    capture satisfying this strict join and then observe its loader consumer.
  - Update: the control-to-media byte join now resolves the FIFO source byte
    through the verified Track 02 media layout. Raw BIN receipts must point
    inside MODE1 user data, while MODE1/2048 ISO receipts use direct user
    offsets; unknown MD5s and sector header/tail bytes reject. Remaining:
    capture the loader's later consumer reads/control decisions before any
    dungeon, object, bitmap, palette, or audio semantics can be assigned.
  - Update 2026-07-19: the chain now continues past the control-to-media
    byte join with consumer-read admission — an observed
    `pce_cd_fifo_origin_main_ram_consumer` row must join the exact FIFO
    byte (generation/LBA/offset/value), the fifo_sequence, and the main-RAM
    destination of the receipt row, with a main-RAM reader PC; a joined
    byte consumed by a different transfer or a System Card reader fails
    closed — and with control-transfer admission binding the first observed
    main-RAM JSR after that consumer read (opaque target). Two latent
    trace-window defects were also repaired: the TII transfer receipt now
    snapshots the accepted row so later parsed-but-source-filtered
    block-transfer rows cannot overwrite the reported fields, and the
    execution binder ignores RTS rows outside the exact destination span
    and other routines' post-RTS rows per its documented
    exactly-one-RTS-inside-the-span contract. Admission remains byte- and
    control-flow provenance only, with no level, object, palette, bitmap,
    or rendering semantics. Remaining: an authentic capture of the loader's
    consumer reads and control decisions on original media; every chain
    stays fail-closed without semantics.
  - Update 2026-07-19: the chain now also binds the control routine's
    observed execution window — an adjacent call-entry row proving the
    control target was actually fetched in main RAM, an adjacent
    next-instruction row, and exactly one main-RAM RTS whose linked
    post-RTS row resumes at the exact control call return address
    (control_pc + 3). Zero or two qualifying resumes fail closed, and
    other routines' RTS/post-RTS rows remain opaque. This is bounded
    execution provenance only; no routine ABI, record, level, object,
    palette, bitmap, or rendering semantics are proven. Remaining: an
    authentic capture of this control window on original media, plus
    evidence of what the resumed loader path reads next.
  - Update 2026-07-19: the chain now covers the resumed loader path after
    that bounded return — the consumer read of the FIFO byte adjacent to
    the first bound consumer byte must be preceded (after the exact
    post-RTS resume row) by its own FIFO receipt row re-verified against
    the hash-verified Track 02 media, must be the second observed consumer
    (sequence=1) joined to that receipt's fifo_sequence and main-RAM
    destination with a main-RAM reader, and is followed by an opaque
    resumed control transfer whose target must be proven actually fetched
    via an adjacent call-entry row. Out-of-order, different-byte,
    different-transfer, or System Card reader observations fail closed.
    This is byte- and control-flow provenance only; no record, routine
    ABI, level, object, palette, bitmap, or rendering semantics are
    proven. Remaining: an authentic capture of the resumed read/control
    sequence on original media, and evidence of how far the loader's
    per-byte consume/dispatch loop extends.
  - Update 2026-07-20: the chain now closes the resumed control routine's
    own bounded window (adjacent next-instruction row, then exactly one
    main-RAM RTS resuming at the exact resumed control call return
    address, with zero or two qualifying resumes failing closed) and
    observes the loader path's second resumption — after the exact second
    resume row, a FIFO receipt row for the byte two positions after the
    first bound consumer byte must re-verify against the hash-verified
    Track 02 media, and the third observed consumer (sequence=2) must
    join that receipt's fifo_sequence and main-RAM destination with a
    main-RAM reader. This is the first loop-iteration evidence for the
    loader's per-byte consume/dispatch pattern, still byte- and
    control-flow provenance only; no record, routine ABI, level, object,
    palette, bitmap, or rendering semantics are proven. Remaining: an
    authentic capture of the repeated consume/dispatch loop on original
    media, and evidence of where the loop terminates or dispatches into
    a record consumer.
  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop — one continuation binder requires a fixed
    count of further iterations after the twice-resumed consumer read,
    each with the full window (opaque control transfer, adjacent
    call-entry and next-instruction rows, exactly one bounded RTS
    resuming at the call return address) plus the next source-adjacent
    FIFO byte's consumer read joined to its media-re-verified receipt
    with the expected loop sequence, fifo_sequence, and main-RAM
    destination. The consumer reader PC must equal the resumed return
    address, making the loop back-edge explicit: the resumed loader path
    itself performs the next read. Missing iterations, out-of-order
    observations, off-target or duplicated resumes, media-mismatched
    receipt bytes, different-byte consumers, out-of-order sequences,
    different transfers or destinations, non-main-RAM readers, or a
    reader that is not the resumed path fail closed. This is still byte-
    and control-flow provenance only; no record, routine ABI, level,
    object, palette, bitmap, or rendering semantics are proven.
    Remaining: an authentic capture of the repeated consume/dispatch
    loop on original media, and evidence of where the loop terminates or
    dispatches into a record consumer.
  - Update: the render-asset admission receipt can now feed a dungeon-facing
    real-data handoff receipt only when the same admitted US raw Track 02
    session carries matching route hashes, payload/envelope/consumer checksums,
    decoded level/object-table/bitmap/palette hashes, source-byte binding,
    object-table layout proof, and bitmap/palette decode proof. The handoff
    explicitly keeps dungeon drawing and fallback visuals closed and rejects
    synthetic dungeon state, synthetic object layout, synthetic bitmap/palette
    decode, hash drift, and fallback observation. Remaining: the positive
    original capture/decoder producer that supplies these real proofs from
    Track 02 without sidecar or generated visual data.
  - Update: the multilevel Track 02 runtime path can now retain a same-capture
    bitmap/palette source-window receipt after a real level transition. The
    receipt binds the selected record, source/target levels, palette raw and
    MODE1 user-data offsets, palette payload/decode checksums, bitmap atlas
    route facts, and a combined source hash while explicitly requiring
    palette decode, bitmap decode, pixel output, M11 render admission, dungeon
    draw, and fallback visuals to remain closed. Remaining: acquire a positive
    original loader/decoder trace that proves palette words and bitmap pixels
    before connecting this source receipt to render-asset or M11 admission.
  - Update: a positive decode-vector receipt now consumes that source-window
    receipt plus the real US Track 02 bytes and verifies the HuC6260 palette
    words, the indexed bitmap atlas, route/tile/nonzero-pixel counts, first
    pixel row hash, and source checksum agreement. This proves a real
    palette/indexed-pixel vector on the multilevel route, but it deliberately
    still blocks M11 runtime consumption, M11 rendering, dungeon draw, and
    fallback visuals. Remaining: capture the original nonstartup dungeon
    graphics consumer that binds these decoded vectors, or another real
    Track 02 bitmap/palette window, to the active dungeon level before any
    render-asset admission or host-surface upload.
  - Update: the positive decode vector can now feed a production M11
    Soul Room runtime-consumption receipt. The receipt selects Track 02 level
    0 through the live `Theron_RuntimeLevelMedia` Soul Room surface, verifies
    exact indexed-atlas route checksum/nonzero pixels/offsets against the
    decode vector, verifies 1:1 host placement and clipping, and permits M11
    host presentation only for that source-owned Soul Room surface. Generic
    dungeon draw, fallback visuals, scale changes, checksum drift, later-level
    graphics, and non-Soul Room routes remain blocked. Remaining: prove the
    original nonstartup dungeon graphics consumer and per-level render layout
    before promoting broader dungeon rendering or host uploads.

- Nexus Saturn memory-card intake remains opaque: the verified boundary accepts
  only an authenticated, hash-bound 8 KiB image with 16 x 512-byte blocks on
  an active title/champion route. Remaining work is an original-card corpus
  and capture proving the proprietary header, slot layout, checksums, and any
  state semantics; FNXS/native-save fallback remains prohibited.

- Nexus Mednafen capture remains operator-only: a dry-run manifest now binds
  BIOS, disc, MENU.BPK, DM.BIN, LEV00.DGN and one PRS3 replay identity. A
  local retail launch/capture must still provide an independently validated
  trace; no launcher path infers decoder, palette, or graphics semantics.
  The PRS3 original-execution importer now also computes SHA-256 directly over
  the supplied Mednafen export bytes and requires exact equality with the
  external attestation, in addition to the existing FNV and V10 input/output/
  VDP1 contract. A formatted but byte-mismatched hash fails closed. This binds
  evidence provenance only; PRS3 grammar, decoder, pixels, palette, and draw
  remain blocked pending positive retail captures.
  A second PRS3 capture admission now binds opaque full output-range bytes and
  a separate opaque VDP1 capture to that authenticated execution receipt by
  exact byte count, FNV, and SHA-256. Both byte streams remain uninterpreted;
  any hash drift rejects and no decoder or graphics path is enabled.
  The same receipt now requires an externally attested command-order pair to
  repeat the original trace's final output-write sequence and later VDP1
  command sequence. Equal, reversed, missing, or drifted values fail closed;
  this is ordering provenance only and does not interpret a VDP1 command.
  The capture attestation now additionally repeats the exact MENU.BPK FNV,
  DM.BIN FNV, and entry index from the original-execution receipt; mixed stream
  identity rejects before any output or VDP1 evidence can be retained.
  Source inventory confirms real MENU.BPK BPPK/BMPD/PRS3 prefix bytes and the
  V10 execution-to-VDP1 handoff receipt, but no authenticated DM.BIN SH-2
  command-writer byte range, instruction-offset map, command-state export
  schema, or command-buffer source FNV is available here. Command state must
  therefore remain capture-required: do not infer a source-to-command parser,
  VDP1 word layout, decoder, palette, pixel, or render admission from the
  current files.
  - 2026-07-17 M11 presentation audit: the full-output admission is still an
    opaque evidence receipt. It authenticates one complete output byte range,
    its SHA-256/FNV, and later VDP1 command order, but deliberately publishes
    no indexed-pixel declaration, width, height, stride, CLUT/palette span,
    BGR/RGB ordering, transparency rule, or host placement. Both
    `graphics_permitted` and `decoder_promoted` remain zero. Do not connect
    this output to M11's indexed/palette surface, reuse WARNING.BIN's PP
    contract, or synthesize a title/menu image. A future original trace must
    attest all of those output-format facts before a byte-exact M11 consumer
    can be added.

- Nexus Structure1F multi-level capture remains no-draw: LEV00--LEV15 now
  require one exact package identity and per-level DGN, descriptor, mesh, and
  face candidate identities before a Saturn trace target can be emitted.
  The local English retail ISO has now been materialized into the configured
  Nexus data root and all `LEV00.DGN`--`LEV15.DGN` files parse positively.
  This proves the parser-backed source envelopes only; Structure1F/2/3
  material, capture and draw semantics remain no-draw. The M11 direct-LEV
  no-draw
  lifecycle now rehashes its selected ordinary source file and rejects level,
  epoch, MD5, size, FNV, buffer, source-file, and geometry drift. It also
  requires parser-observed DGN container-header and bounded counted
  Structure1F descriptor-table offsets, lengths, counts, and FNVs on the
  engine-owned M11 receipt; malformed tables or stale spans fail closed. Its
  selected Structure1F row is now separately rebuilt from the active parser,
  required to remain inside that table, and joined only to the already bounded
  Structure3 face/Structure2 envelope reference. That join remains opaque:
  mesh, texture, palette, pixel, decoder, and drawing semantics are still
  blocked. For selected static rows M11 also rebuilds the exact Structure3
  face and 20-byte Structure2 descriptor plus bounded opaque candidate spans;
  all FNV/offset/length drift rejects, but the candidates are not texture or
  palette data until a real Saturn decoder corpus proves them. It still awaits
  that same local corpus for a retail-positive route. An operator-only M11
  topology-descriptor intake now also rederives the documented Structure3b
  face row, its bounded entry-local vertex table, the exact referenced vertex
  row sequence, and the paired normal row from the active LEV. Their offsets,
  lengths, FNVs, index count, package, card/package route epoch, and selected
  Structure1F row must all agree at consumption. This is capture-required
  topology framing only: it proves no surface, winding, transform, material,
  texture, palette, VDP1, or draw behavior. Retail-positive confirmation
  remains unavailable without an authenticated Saturn material/VDP1 capture.
  An operator-only M11
  capture-replay target now rehashes and freezes the selected direct source,
  card/package/epoch, face/descriptor and candidate identities before it can
  accept an opaque original-Saturn trace/VDP1-lane observation. That receipt
  deliberately provides no byte decoder, VDP1 command grammar, palette
  meaning, texture output, or draw permission.
  A separate external VDP1 capture-envelope import now requires its fixed
  magic/version/header, exact DGN/card/package/epoch/face/descriptor/candidate
  identities, bounded opaque payload interval and payload FNV before it can
  retain no-draw evidence. No matching retail capture envelope is present in
  this workspace, so this does not prove VDP1 command semantics, pixel or
  palette format, texture output, or rendering.
  An operator-only local Mednafen plan can now bind a region-selected,
  SHA-256-verified external BIOS and original disc to that exact capture
  route, but it intentionally refuses `--launch` unless a capture-capable
  Mednafen produces the V1 artifact itself. No local BIOS, disc, or capture
  file is retained in the repository.
  M12/M11 now keeps that plan as an explicit `capture-required` no-draw route
  and may resume only after the V1 importer revalidates the same BIOS/disc
  identities and live card/package/epoch/DGN/descriptor route. The resumed
  receipt is still evidence-only; a real decoder, VDP1 format proof, and
  rendering remain open.
  The separate external `NXS3TOP1` topology-capture import is likewise
  capture-required: it rebuilds the active direct-LEV Structure1F/Structure3
  vertex/normal target, then requires its fixed header, exact card/package/
  epoch/level/source identities, all descriptor spans/FNVs, and one bounded
  opaque payload FNV. It retains no payload and proves no topology semantics,
  mesh, transform, texture, palette, VDP1 behavior, or draw route. No retail
  topology capture has been admitted. The broad synthetic
  `nexus_v1_dgn_geometry_readiness` Structure3 fixture remains known-red and
  is not admission evidence; the focused direct-corpus test is the only
  green contract coverage until authentic LEV corpus data is available.
  The local operator-only Mednafen `NXS3TOP1` plan now binds executable,
  region-selected SHA-256 BIOS, original-disc SHA-256, and every active
  LEV/Structure1F/Structure3/card/package/epoch topology fingerprint before
  it can describe the external producer command. It refuses launch without
  explicit `--operator-only --launch`, never copies BIOS/disc/capture bytes,
  and M12/M11 remains capture-required until the existing importer accepts
  the emitted artifact. Retail capture production and semantic evidence are
  still missing.
  A local read-only `NXS3TOP1` artifact verifier and M12/M11 preflight bridge
  now independently compare the plan's BIOS/disc hashes and the artifact's
  LEV, face, vertex, normal, bounded payload, and route metadata before the
  opaque importer is called. This is only a duplicate integrity boundary: it
  retains no payload and does not establish a retail artifact, topology
  semantics, mesh, pixel, palette, VDP1, decoder, or draw behavior.
  The matching `NXSVDP1C` Structure2/Structure3 material route now has the
  same local preflight: BIOS/disc, DGN/route, face, descriptor, image/palette
  candidate fingerprints, payload bounds/FNV, and nonempty opaque trace and
  command witnesses must agree before M12/M11 calls the existing importer.
  This is integrity evidence only; no capture payload, PRS3 mode, VDP1 command
  grammar, texture, palette, pixel, decoder, or draw semantics is retained or
  inferred. No retail material capture is admitted.
  The atomic Structure1F/1A-owner, Structure3-face, and Structure2-material
  capture path now has a separate fixed `NXS1OMC1` envelope preflight. It
  requires the selected source FNV/size and level, owner and Structure1F/1A
  coordinates, face row, descriptor and candidate FNVs, an exact bounded
  opaque payload FNV, and a nonempty raw-trace witness before it can be an
  opaque admission receipt. It does not establish the owner-to-entry map,
  topology, source-read relation, VDP1 state, texels, palette, decoder, or
  draw; a retail capture still must prove each such relation.
  M12/M11 now exposes the same selected owner/material target as an
  operator-only capture-required route. Resume rebuilds the active target and
  compares level/source, Structure1F/1A owner, Structure3 face, and Structure2
  descriptor/candidate identities before accepting `NXS1OMC1`; absent active
  corpus, target drift, or any semantic-promotion flag leaves it no-draw.
  `NXS1OMC1` now also has a strict import bridge: before the existing atomic
  Structure1F/1A/Structure3/Structure2 trace admission runs, it rechecks the
  live target, fixed artifact, raw-trace size/FNV witness, manifest, and an
  external original-Saturn attestation. This preserves opaque evidence only.
  A two-witness adjudication can now retain capture coverage only when two
  separately admitted opaque receipts agree on the exact source, descriptor,
  and face fingerprints while both their artifact and raw-trace FNVs differ.
  Duplicate witnesses and any target drift reject. This remains integrity
  coverage only: it proves no owner mapping, topology, VDP1 state, texture,
  palette, decoder, or draw behavior.
  A bounded campaign intake now aggregates two to sixteen already imported
  `NXS1OMC1` witnesses across distinct dungeon levels. Every witness must
  rebind its exact Structure1F/1A owner, Structure3 face, Structure2
  descriptor, source FNV, opaque capture FNV, and raw-trace witness to an
  admitted engine trace; duplicate level, source, capture, or trace identity
  rejects. It stores coverage metadata only, not payload bytes or semantics.
  M12 now has an operator-only export/import route for that campaign. The
  exported capture-required receipt binds the locally supplied BIOS region and
  BIOS/disc SHA-256 identities plus the exact witness count; import rebuilds
  those values and accepts only the existing fully imported opaque campaign.
  A missing, stale, partial, or count-drifted campaign remains capture-required
  and no-draw. The launcher neither emits nor retains a capture payload.
  The route now additionally requires a fixed `NXS1OMC2` campaign-index
  artifact before resume. Its versioned header, bounded 56-byte rows, row-table
  FNV, and every level/source/descriptor/face/capture/raw-trace identity must
  repeat the admitted campaign in order. It stores identities only and rejects
  truncated, malformed, hash-drifted, or cross-route rows without retaining
  raw trace or capture payload bytes.
  A selected campaign witness now has a second strict `NXS1OMC1` artifact
  preflight before consumption: it revalidates the fixed capture envelope,
  its opaque payload hash, the supplied raw-trace size/FNV, the selected DGN
  level/Structure3 face/Structure2 descriptor row, and the already admitted
  engine trace against the `NXS1OMC2` campaign row. Capture or trace drift
  fails closed and no bytes are retained. This is provenance only, not a
  payload decoder or a rendering promotion.
  M12 now exposes a separate capture-required route for one selected campaign
  witness. Its witness index must be within the hash-bound campaign export;
  resume first reconstructs `NXS1OMC2` and then applies the `NXS1OMC1`
  preflight to that exact Structure1F/Structure3/Structure2 row. Invalid,
  stale, or out-of-range route selection remains no-draw and cannot promote a
  decoder or renderer.
  M12 can now select multiple distinct campaign witnesses at once, bounded by
  the original exported witness count. Duplicate or out-of-range indices are
  rejected before capture import; the selection retains only row identities and
  remains capture-required/no-draw until every selected original artifact is
  independently preflighted. It establishes no cross-level geometry, texture,
  palette, VDP1, decoder, or draw semantic relation.
  The active M11 dungeon route now consumes that selection only as a live
  capture-required start receipt: it re-admits campaign identity, matches the
  current dungeon level to a selected witness, and exposes source/face/
  descriptor identity only for that route. An unselected or drifted live level
  receives no capture route and remains no-draw.
  Before that live receipt is exposed, it now rebinds and carries the selected
  source-backed Structure1F/Structure3/Structure2 capture target plan. Its
  level, source FNV, face-row FNV, and descriptor FNV must repeat the campaign
  row exactly; target-plan drift fail-closes the start route. The copied plan is
  capture provenance only, never geometry, texture, pixel, decoder, or draw
  authorization.
  No authentic direct-LEV corpus or independently reviewed artifact is present
  here, so owner mapping, face/mesh semantics, VDP1, texture, palette, decoder,
  and draw remain blocked.
  The MENU.BPK PRS3 material route now also accepts an external `NXSPRS3M`
  envelope only after its card/package/epoch, selected entry, bounded
  compressed-body span/FNV, and declared-output count exactly match the
  existing M11 no-draw presentation receipt. Its capture payload and trace
  witness remain opaque; this provides neither a PRS3 decoder nor any pixel,
  palette, Structure2 material, or render claim. No retail PRS3 artifact is
  admitted.
  An operator-only local Mednafen `NXSPRS3M` launch plan now hashbinds BIOS,
  disc, MENU.BPK package/entry/body and declared output to the same M12/M11
  capture-required route. It cannot launch without explicit operator opt-in,
  retains no media or payload, and resume remains closed until the local
  artifact preflight succeeds. Retail capture and all codec/render semantics
  remain missing.
  The M12/M11 champion-to-dungeon handoff additionally requires the matching
  direct-card, MENU.BPK package, selected level, and launcher epoch before it
  can retain that no-draw receipt; its positive branch has the same pending
  corpus evidence.
  Remaining work is direct,
  hash-first corpus discovery plus an operator-supplied hash manifest and
  original Saturn trace observations; mesh/face geometry and all
  pixel/palette semantics stay uninterpreted.

- The direct SLEV/SAL/MAP/SDDRVS discovery route now has the materialized
  English retail auxiliary corpus with positive hash/identity and bounded
  parser receipts. Retail-positive script/audio trace evidence, dispatch,
  decoding, and playback remain blocked. The direct SDDRVS dungeon
  admission also revalidates its direct file at consumption, but it still
  awaits authentic package/level/trace evidence before any script claim. The
  matching direct SAL/SLEV/MAP dungeon route now has the same identity-only
  rehash-on-consume guard; it does not establish a codec, event meaning,
  playback, or script semantics. The verified SAL `dsp01.EX` container
  preamble and bounded opaque payload interval are now retained only as
  provenance; descriptor/sample grammar and codec evidence remain open.
  Direct SNDLEV MAP provenance now also retains only its 24-byte header,
  bounded 8-byte rows, and terminator. M11 can bind one rehashed row to the
  active level/package/card/epoch, but selector/event semantics, codec proof,
  and playback remain unproven and blocked.

- Nexus SLEV task-body capture remains no-dispatch: every SLEV00--15 target
  requires matching admitted header/literal, raw-trace, and source-order
  receipts plus opaque external opcode and callback-owner labels. Remaining
  work is reviewed original-Saturn task-body grammar and callback ABI proof;
  no task opcode executes and no fallback script is admitted.
  The selected target can now enter M11 startup only through the matching
  direct SLEV/SAL/card/package/epoch receipt and exact SLEV FNV. That is an
  opaque source-order/trace admission only; authentic retail task-body and
  callback evidence is still required before any dispatch claim.

- Nexus SNDLEV/SAL capture planning remains playback-blocked: each unique
  level/selector route must preserve its canonical SAL/MAP/SDDRVS identities,
  bounded window, raw-trace identity, and selector-read-output ordering.
  Remaining work is original driver ABI and payload-format proof; no MAP event
  meaning, SAL payload semantics, decode, or fallback playback is admitted.
  M11 can now retain a selected task-to-SAL startup receipt only when one
  opaque original-trace binding repeats the exact task-trace, SAL descriptor,
  MAP table, and SDDRVS FNVs under the active card/package/epoch route. This
  remains a no-op boundary: command ownership, selector/event meaning, codec,
  and playback evidence are still absent.
  A fixed V1 external SLEV/SAL capture envelope can now be imported only after
  its header, bounded payload and payload FNV match that same active route.
  The payload remains opaque and non-retained; a real command grammar and
  driver/codec proof are still required before it can change no-op behavior.
  M12/M11 now exposes that route as capture-required until an operator imports
  an exact `NXSLSC01` receipt. The local Mednafen plan binds BIOS region/SHA-256,
  disc SHA-256, and every route FNV without copying media or producing a trace.
  A local artifact preflight now independently requires the fixed header,
  task/SAL/MAP/SDDRVS/card/package/epoch identities, and exact end-bounded
  opaque payload FNV before M12/M11 can resume import. A stale or mutated
  artifact rejects; an accepted receipt still cannot dispatch commands, decode
  audio, play sound, or draw. A real retail `NXSLSC01` capture and original
  command/driver semantics remain required.

- Nexus cross-domain Saturn capture import remains evidence-only: one complete
  Mednafen campaign must bind the PRS3 placement, Structure1F target, SLEV
  task location, and SAL window to one raw trace identity. Remaining work is
  independent review of each original trace; decoder, geometry, script, and
  playback semantics remain closed.

- Nexus PRS3 original-execution intake remains evidence-only: one independently
  authenticated V10 export must bind one MENU.BPK stream's complete SH-2 input
  reads, output fingerprint/range, and later VDP1 source command. Remaining
  work is reviewed opcode, pixel, and palette semantics; no decoder or graphics
  route is admitted.
  - 2026-07-22 capture-admission update: the final byte-admission stage now
    rehashes the supplied full MENU.BPK and DM.BIN bytes, derives the exact
    bounded MENU.BPK stream by the V10 offset/length, and requires FNV-1a plus
    SHA-256 agreement for those three source lanes before it accepts opaque
    output and VDP1 capture bytes. It also repeats the trace's strict final
    output-write -> VDP1-command ordering. This is not a PRS3 decoder, VDP1
    command parser, palette interpretation, pixel path, or draw permission.
    The remaining blocker is still an independently authenticated retail
    Mednafen/Saturn V10 export and its four real byte artifacts.

- Nexus PRS3 multi-capture review remains non-promoting: representative,
  independently authenticated MENU.BPK modes must agree on opaque bit-order
  and termination observations before a decoder candidate may be reviewed.
  Decoder, palette/pixel meaning, rendering, and fallback visuals remain off.

- Nexus Structure3 face/texturing capture remains capture-only: DGN face and
  Structure1F/2 provenance must agree with opaque material candidates and VDP1
  evidence. Pixel and mesh semantics remain unproven and no draw route opens.

- Nexus multi-level DGN capture remains opaque: LEV00--15 needs matched
  Structure1F, Structure2 placement, Structure3 face targets and ordered
  command/frame receipts. No decoder, mesh inference, or rendering is admitted.

- Nexus active dungeon route may report only capture-ready coverage when its
  loaded DGN identity matches the full multi-level adjudication receipt. Level,
  package, PRS3 trace FNV, or trace-size drift clears it. Decoder,
  mesh/texturing, and rendering remain unavailable.

- Nexus multi-level capture jobs remain operator-only planning data. A future
  Mednafen invocation must independently re-hash every staged retail asset and
  preserve the emitted job order; this planner never launches, captures, or
  interprets a trace.

- Nexus campaign asset intake is read-only and hash-first for explicitly staged
  loose files, ZIP members, and ISO/BIN/CUE members. Virtual container entries
  are never extracted or copied; unsupported containers remain blocked.

- Nexus Saturn memory-card startup intake remains opaque: authenticated 8 KiB
  card identity and selected route epoch may gate champion startup only. Save
  layout, FNXS fallback, and native-save semantics remain blocked.

- Nexus M12 card-startup selection consumes only exact opaque card/epoch
  readiness; native FNXS resume remains a separate route.

- Nexus Saturn-card discovery currently admits only one direct 8 KiB file;
  virtual ZIP/ISO/BIN/CUE identities are diagnostic-only and contents stay
  opaque; container launch remains blocked.

- Nexus champion startup accepts only an atomically bound direct card, package
  identity and current M11 route epoch; when the M11 PRS3 presentation receipt
  is present, it must share that exact package and epoch. Card bytes remain
  opaque and PRS3 remains no-draw.

- Nexus Structure1F records now retain parser-observed raw spans only; face,
  mesh, palette and texture semantics remain unproven and no-draw.

- Nexus Structure2 descriptor spans are source provenance only; codec, pixel
  and palette meaning remain blocked pending original evidence.

- Nexus Structure3 face spans are raw package provenance only; PRS3, palette,
  pixel and texture semantics remain blocked. The direct-source admission now
  also retains one hash-bound 40-byte entry header, its raw tag/count fields,
  and the three count-bounded 12-byte intervals only when the already admitted
  Structure3 target and ordinary source file still agree. This is framing, not
  a geometry, normal, material, texture, transform, or draw claim. The local
  retail LEV corpus is still absent, so positive corpus confirmation remains
  pending.

- Nexus Structure3 image/palette references are bounded source intervals only;
  codec and decoded surface admission remain blocked.

- Nexus MENU.BPK startup provenance now binds a selected PRS3 entry's bounded
  payload offset/length/FNV and header facts through an epoch- and
  package-bound M11 no-draw host receipt. Any engine-owned verified row may be
  selected, but its recognized mode byte, bounded opaque compressed body,
  declared output size, and body FNV must exactly match; unknown modes and
  declaration/span/FNV drift reject, including across launcher/card epoch
  transitions. PRS3 pixels, opcode grammar, and decoder promotion remain
  unavailable pending independent original-Saturn codec evidence.

- The legacy `nexus_v1_bpk_surface_class` synthetic fixture still asserts a
  synthetic PRS3 literal decoder and decoded material import. Its stored
  payload receipt now keeps the fallback-provenance bit closed, but it is
  incompatible with the current retail fail-closed PRS3 route and is not
  evidence for a Saturn codec; replace it with authenticated capture-backed
  expectations before treating it as a promotion test.

- 2026-07-17 DM1 original-save C-event package completed: F0435 now retains
  C2 `ActionIndex` and `PoisonEventCount`; F0802/F0796 preserve their bounded
  PC34 bytes. C25 and C29 exports require authenticated F0435 provenance,
  while C3/C4 snapshot drift, malformed poison width, synthetic C25/C29, and
  invalid source squares reject. The targeted original-save handoff suite is
  green; remaining work is external original-save corpus evidence.

- 2026-07-17 DM1 C2 PARTY_INFO follow-up completed: source byte 86
  `Event71Count_Invisibility` now materializes into both M10 invisibility
  owners and F0802 writes it back only as a bounded PC34 byte. The focused
  C71 path and full original-save handoff suite are green.

- 2026-07-17 DM2 DB14: the normal `QUERY_PICST_IT` `0x40`/neutral-mode branch
  now copies only authenticated native-size indexed IMG3 pixels under matching
  RAW4 clip and palette receipts. Flip, crop, nonzero offset, scaling, and
  every other blitmode remain fail-closed. Remaining: source-proven non-normal
  transform branches and live frame ordering.

- 2026-07-17 DM2 HUD SUMMARY_IMAGE: `c_gui_draw.cpp:926-942` now has a
  no-draw M11 receipt for exact `(1,vb_144,field)` HUD commands. It requires
  the source plan's decoded GDAT pixels, local palette, and RAW4 destination
  identity; tuple mismatch, absent palette, and stale destination reject.
  Remaining: source-proven HUD transform admission before any new draw path.

- 2026-07-17 DM2 HUD PICST transform: the exact `c_gui_draw.cpp:926-942`
  branch admits only source values `0..0x28`, retaining X scale `0x1f` for
  `0..0x0f` or `0x2f` otherwise and Y scale `0x35`. Out-of-range values,
  missing SUMMARY_IMAGE material, or stale destination reject; it remains
  source-gated for draw only where the resolved destination is the complete,
  exact scaled rect. Partial/unknown `QUERY_BLIT_RECT` clipping, flips, and
  every other HUD transform remain no-draw.

- 2026-07-17 DM2 pit viewport admission: `c_gui_vp.cpp:234-292`
  `DM2_DRAW_PIT_TILE` now has a bounded source receipt for cells 1..15. It
  binds `table1d6c70/90/a0/b0` selection, the live cell's `+8` state word,
  `DRAW_DUNGEON_GRAPHIC` light parameter, exact `(GRAPHICSSET,field)`
  SUMMARY_IMAGE, GFX256 raw material, decoded U4 bytes, and local palette.
  It remains `no_draw`: cell 0's `SET_GRAPHICS_FLIP_FROM_POSITION` and the
  selected `QUERY_BLIT_RECT` placement/clip chain are not yet proven.
  - 2026-07-17 composition update: cells 1..15 now bind their accepted
    SUMMARY_IMAGE/GFX256 material identity into the current DM2 viewport
    composition session/data epoch and parent ordering receipt. The receipt
    explicitly records that PIT_TILE's own draw slot is unresolved, so it
    cannot consume pixels. The sole remaining promotion precondition is the
    source's per-cell `QUERY_BLIT_RECT` destination/clip transaction.
  - 2026-07-17 RAW4 placement update: `table1d6c70[cell]` now binds through
    `DRAW_DUNGEON_GRAPHIC`/`QUERY_PICST_IT` to the exact
    INTERFACE_GENERAL/0/RAW4 root row, with destination, full material extent
    and table/row hashes retained in the PIT composition receipt. Chained
    rectangles, crop and clip grammar remain rejected. It stays no-draw until
    a PIT-owned ordered composition slot and authenticated buffer handoff are
    proven together.
  - 2026-07-17 buffer/slot update: PIT_TILE now retains its own authenticated
    decoded U4 buffer handoff and binds it to the generic DM2 viewport
    before/after surface snapshot and composition identity. Pointer, extent,
    stride, palette, material and surface-generation drift reject with no
    write. The slot deliberately remains no-draw: source proof is still
    missing for PIT_TILE's normal-branch `DRAW_PICST` row ordering.
  - 2026-07-17 normal-row update: cell 1's `blitmode=0` branch is now bound
    to `DRAW_PICST`'s top-to-bottom/left-to-right U4 row order and exact RAW4
    placement identity. It remains no-draw because `DRAW_DUNGEON_GRAPHIC`
    applies `DM2_query_B073` before that row loop; PIT still lacks its own
    authenticated transformed palette transaction.
  - 2026-07-17 B073 update: cell 1 now binds `DM2_query_B073`'s RAW7
    count/left/right/lookup palette program to its material, RAW4 placement
    and normal-row receipt. RAW7, placement or palette drift rejects. The
    transformed palette remains no-draw until alpha ownership and the final
    ordered handoff consumer are jointly admitted.
  - 2026-07-17 cell-1 consume update: only cell 1's normal (`blitmode=0`)
    path now consumes the authenticated U4 handoff through B073's transformed
    palette and low-nibble alpha into the current ordered owner surface. All
    other PIT cells, mirrors, crops and chained clips remain fail-closed.
  - 2026-07-17 cell-3 consume update: cell 3's independent normal
    (`blitmode=0`) route now admits only its exact GRAPHICSSET field `0x6e`,
    RAW4 rect `0x35b`, B073 transaction and ordered U4 handoff. Cell 2 and all
    other mirrored or unproven normal forms remain fail-closed.
  - 2026-07-17 cell-4 consume update: cell 4's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x6f`, RAW4 rect
    `0x35a`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Cell 2 and every other mirrored or unproven normal form remain
    fail-closed.
  - 2026-07-17 cell-6 consume update: cell 6's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x71`, RAW4 rect
    `0x358`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.
  - 2026-07-17 cell-7 consume update: cell 7's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x72`, RAW4 rect
    `0x357`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.
  - 2026-07-17 cell-11 consume update: cell 11's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x76`, RAW4 rect
    `0x355`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.
  - 2026-07-17 cell-12 consume update: cell 12's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x77`, RAW4 rect
    `0x354`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.
  - 2026-07-17 cell-14 consume update: cell 14's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x79`, RAW4 rect
    `0x352`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.
  - 2026-07-17 cell-2 HFLIP consume update: cell 2 admits only GRAPHICSSET
    field `0x6c`, RAW4 rect `0x35f`, B073/RAW7, and its own source-locked
    reverse-X U4 row walk. Crop, chained clips, vertical flip and all other
    mirror cells remain fail-closed.
  - 2026-07-17 cell-5 HFLIP consume update: cell 5 admits only GRAPHICSSET
    field `0x6f`, RAW4 rect `0x35c`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.
  - 2026-07-17 cell-8 HFLIP consume update: cell 8 admits only GRAPHICSSET
    field `0x72`, RAW4 rect `0x359`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.
  - 2026-07-17 cell-13 HFLIP consume update: cell 13 admits only GRAPHICSSET
    field `0x77`, RAW4 rect `0x356`, B073/RAW7 and its source-locked reverse-X
    U4 row walk. All other mirrored forms remain fail-closed.
  - 2026-07-17 cell-15 HFLIP consume update: cell 15 admits only GRAPHICSSET field `0x79`, RAW4 rect `0x353`, B073/RAW7 and its source-locked reverse-X U4 row walk.
  - 2026-07-17 crop/chained-clip update: `QUERY_BLIT_RECT` source-coordinate mutation remains no-draw behind a source-locked PIT provenance receipt; root RAW4 does not prove crop or chaining.

- 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT material admission:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now bind the
  successful `QUERY_GDAT_ENTRY_IF_LOADABLE` branch only: exact state-table
  lane, GRAPHICSSET SUMMARY_IMAGE/GFX256 raw bytes, decoded U4 indices, local
  palette, root RAW4 placement and the live DM2 composition/surface snapshot.
  It remains no-draw. The `QUERY_TEMP_PICST` fallback and the downstream
  B073/`QUERY_PICST_IT`/`DRAW_PICST` transform must be proven separately.
  - 2026-07-17 fallback update: the exact non-loadable `table1d6f7c` path at
    `c_gui_vp.cpp:514-527` now admits its own SUMMARY_IMAGE/GFX256 U4 and
    RAW4/M11 receipt plus `QUERY_TEMP_PICST(1,0x40,0x40,0,0,0,rect,-1,light,
    -1,8,graphicsset,field)` provenance. It remains no-draw because
    `query_32cb_0804` selects a live B073/field-7 palette transaction from
    `c_querydb.cpp:2415-2465`, which is not yet authenticated.

- 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary material admission:
  `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287` bind only cells
  1..8 with a defined `table1d6fdc/table1d6fee` state lane to authentic
  GRAPHICSSET SUMMARY_IMAGE/GFX256 U4 bytes, local palette, root RAW4 and M11
  owner surface. B073/`DRAW_PICST` remains no-draw pending a live palette and
  transform receipt.
  - 2026-07-17 transform provenance update: `SKULLWIN/c_image.cpp:450-475`
    now binds the side-stairs `DRAW_DUNGEON_GRAPHIC` delegation to blit mode 0,
    default normal scale and zero source offset; its source rects explicitly
    exclude the `0x2bc/0x2bd` offset special case. Material, RAW4 and M11
    identities must agree. The live `DM2_query_B073(image.palette,
    ddat.v1e12d2, alpha, -1, ...)` transaction remains unauthenticated, so
    the complete branch is intentionally no-draw.

- 2026-07-17 DM2 `TRIM_BLIT_RECT` wall-clip admission: `SKULLWIN/c_gui_vp.cpp:570-573,611-658` now has a DM2-owned no-draw receipt that binds the selected D1/D2 GRAPHICSSET trim word, recomputed authenticated wall material identity and current viewport-surface snapshot to the exact `(x,y,right,bottom)` clip margins. Missing trim rows, material/palette drift and invalid surface bounds reject. Applying this clip to a wall blit remains closed pending a source-owned live caller hand-off.
  - 2026-07-17 live `DRAW_WALL` update: the receipt now binds one existing
    `QUERY_TEMP_PICST` wall command to the same recomputed material hash,
    M11 wall-composition identity and atomically identical owner snapshots.
    Only the source's `0x40` normal scale, RAW4 `0x2be + cell`, movement
    offset and source flip are recorded. This gate remains no-draw; it does
    not introduce a second wall renderer.

- 2026-07-17 DM2 `DRAW_WALL_TILE` admission: `SKULLWIN/c_gui_vp.cpp:6703-6741`
  and `dm2data.cpp:266-273,602-605` now bind every `table1d7012` cell branch
  to the existing authenticated wall/M11 identity. The receipt records the
  exact 0/1/2 delegated-call count and `table1d6afe` orientation; it remains
  no-draw because `DM2_guivp_32cb_15b8` has separate unbound GDAT transforms.
  - 2026-07-17 `32cb_15b8` input update: the first simple `QUERY_TEMP_PICST`
    call at `c_gui_vp.cpp:6618-6628` now has a source-owned no-draw input
    receipt for category 9 selector/image field, exact `0x40` scales, flip,
    query parameters and RG71l alpha. Record layout and destination remain
    explicitly unavailable.
  - 2026-07-17 loadable `0x0f` update: the distinct `c_gui_vp.cpp:6651-6692`
    category-9 `QUERY_GDAT_ENTRY_IF_LOADABLE` branch now binds its successful
    `0x0f` selector, normal scales, transform inputs and RG71l alpha as a
    no-draw receipt. Its destination is still not inferred.
  - 2026-07-17 category-8 overlay update: `c_gui_vp.cpp:6322-6329` now has
    its own no-draw QUERY_TEMP_PICST input receipt for selector/image field,
    normal scales, flip, transform parameters and RG71l alpha.
  - 2026-07-17 branch-set update: the three authenticated category-8/9
    `32cb_15b8` input receipts now combine only when their independent
    identities and the loadable `0x0f` field agree; the aggregate stays
    no-draw and has no placement contract.
  - 2026-07-17 DRAW_TEMP_PICST admission update: the aggregate now has a
    no-draw consumption gate that rechecks every branch-set identity,
    category, `0x0f` field and normal-scale transform before admitting the
    source call. It carries no destination or pixel information.

- 2026-07-17 DM2 `query_B073` input admission: `c_querydb.cpp:2506-2545`
  now requires authentic palette, live light, alpha/mask, colors/cache,
  RAW7, lookup and traversal identities in one no-draw receipt. No palette
  buffer or pixel result is produced.
  - 2026-07-17 B073/DRAW_TEMP_PICST surface update: authenticated B073 and
    DRAW_TEMP_PICST receipts now bind to an owned viewport-surface snapshot
    in one no-draw palette/surface receipt. No buffer is borrowed or written.
  - 2026-07-17 original palette update: the next consumer may borrow only
    original 16/256-byte palette storage when its bytes hash matches the
    caller's authenticated identity and the B073/surface receipt remains
    current. No transformed palette or pixel buffer is created.
  - 2026-07-17 M11 palette-consumer update: borrowed original palette bytes
    now bind to a current owner-surface generation in a no-draw M11 receipt,
    with no transform, destination or pixel material.
  - 2026-07-17 original material update: a later consumer may borrow only
    original decoded GDAT storage with proven dimensions, stride, byte count
    and byte hash paired to the current M11 palette consumer. No decoder or
    render path is admitted.
  - 2026-07-17 M11 material/palette pair update: original material and
    original palette now admit only as a matching no-draw pair with current
    owner generation and verified dimensions/stride. No render contract.
  - 2026-07-17 live materialization update: the validated pair now has a
    no-draw M11 handoff guarded by the same live owner generation. It carries
    only borrowed bytes/layout, never a blit or destination.
  - 2026-07-17 DRAW_PICST trace update: source handoff now reaches an exact
    `QUERY_PICST_IT`/`DRAW_PICST` trace receipt, but missing source and
    destination rectangles remain an explicit no-draw blocker.
  - 2026-07-17 DRAW_PICST rect update: `query1 == -1` now admits only the
    exact direct `srcx/srcy + imgdesc.x/y` source rectangle branch from
    `c_image.cpp:240-296`; all QUERY_BLIT_RECT, flip and destination paths
    remain no-draw.
  - 2026-07-17 QUERY_BLIT_RECT trace update: `c_xrect.cpp:217-280` now
    admits only an authenticated unsigned root rectangle node with
    `query2 == -1`, `mode1 <= 8`, `mode2 == 0`, a present bitmap and its
    captured source-rectangle identity. Signed, overridden, mode-9 and
    chained nodes remain no-draw until their clip/destination semantics are
    separately evidenced.
  - 2026-07-17 QUERY_BLIT_RECT signed-root update: `c_xrect.cpp:228-276`
    now records the exact signed-node `datax/datay + input-x/input-y`
    transform for an authenticated unchained root. `crdecode`, final clip,
    destination, overrides and every chained node remain no-draw.
  - 2026-07-17 QUERY_BLIT_RECT mode-1 update: the signed-root receipt now
    reaches the exact `crdecode(1, ...)` origin assignment in
    `c_xrect.cpp:162-211,426-436`, guarded by current authenticated material
    dimensions and surface generation. All other modes, clipping and final
    destination bounds remain no-draw.
  - 2026-07-17 QUERY_BLIT_RECT default-clip update: `c_xrect.cpp:239,438-470`
    now admits the untouched `rc=[-10000,10000)` range only for a current
    mode-1 receipt whose full material rectangle lies inside it. Global clip
    override, chained terminal nodes and all surface-specific destinations
    remain no-draw.
  - 2026-07-17 QUERY_BLIT_RECT global-clip update: the explicit
    `c_gui_vp.cpp:570-573` `TRIM_BLIT_RECT` transaction now provides the only
    admitted `dm2rect1` override input for `c_xrect.cpp:438-439`, with active
    flag, trim-call, material and surface identities. Intersecting that clip
    with the destination rect and every final blit remains no-draw.
  - 2026-07-17 QUERY_BLIT_RECT global-intersection update:
    `c_xrect.cpp:446-470` now admits the exact `dx/dy` source-offset and
    clipped destination-rectangle calculation for the authenticated mode-1
    global-clip path. Missing overlap or any clip/material/surface identity
    drift rejects; no blit is admitted.
  - 2026-07-17 DRAW_PICST surface-address update: `c_image.cpp:293-335` and
    `c_gfx_blit.cpp:604-656` now admit only the native 8-bit `gfxsys.dm2screen`
    row-address path with packed original source stride, exact source/dest
    offsets, no palette translation and no alpha mask. The receipt borrows
    addresses only; all pixel writes and other surface formats remain no-draw.
  - 2026-07-17 DRAW_PICST row-traversal update: the original material bytecount
    now remains attached through the M11 handoff. `c_gfx_blit.cpp:604-656`
    default `BLITMODE0` admits only forward rows with authenticated first/last
    row offsets and exclusive source/destination bounds. Other modes and every
    pixel operation remain no-draw.
  - 2026-07-17 DRAW_PICST mask/palette update: `c_image.h:45-70` and
    `c_gfx_blit.cpp:655-760` now admit only the masked translated `BLITMODE0`
    input transaction with 256 authenticated palette bytes, exact alpha index,
    original material bytecount and forward row bounds. Palette translation
    and every pixel write remain no-draw.
  - 2026-07-17 DRAW_PICST palette-index update: `c_gfx_blit.cpp:39-42,675-682`
    now has a source-locked trace that records the exact ordering: compare raw
    8-bit source index to alpha first, then use that same index in PAL256.
    It does not dereference source/palette bytes or write pixels.
  - 2026-07-17 DRAW_PICST palette-write update: source proof now fixes each
    `t_palette` entry to one `c_pixel256` byte and PAL256 to 256 bytes. The
    masked destination write order is carried as no-draw row metadata with
    current surface identity; no conditional pixel write is executed.
  - 2026-07-17 DRAW_PICST masked-consume update: the exact source++ / masked
    write / destination++ order from `c_gfx_blit.cpp:675-682` is now a strict
    no-draw receipt. It rejects mode, alpha, row and identity drift; actual
    pixel consumption remains intentionally closed.
  - 2026-07-17 DRAW_PICST native execution update: the fully authenticated
    8-bit BLITMODE0/PAL256/mask branch now has its first source-backed pixel
    consumer. It revalidates all receipts and owner generation before the
    exact forward masked writes; every mismatch is no-write.
  - 2026-07-17 DRAW_PICST M11 update: the native executor now enters only
    through a DM2-owned M11 consumer that requires the exact live material
    handoff buffer/palette and owner generation. No legacy renderer or
    fallback path can reach this consumer.
  - 2026-07-17 DRAW_WALL admission update: authentic GDAT wall commands now
    enter a strict DRAW_PICST admission with their raw/decoded/palette/geometry
    receipts, but remain no-draw because the source route owns PAL16 rather
    than the proven native PAL256 executor contract.
  - 2026-07-17 DRAW_WALL B073 update: PAL16 now binds to a strict PAL256 cache
    output receipt only with complete RAW7, lookup, traversal and allocation
    identities from `c_querydb.cpp:2506-2668`; no expansion or write occurs.
  - 2026-07-17 DRAW_WALL B073 raw intake update: the wall route now carries
    the actual borrowed RAW7 program, v1e020c group and v1e0210 lookup bytes
    with sizes, identities and cache allocation. Interpretation remains closed.
  - 2026-07-17 DRAW_WALL B073 contiguous RAW7 loader update: only the
    original `INTERFACE_GENERAL/0/RAW7/2` record admitted by
    `dm2_v1_asset_load_typed_sized()` may bind its contiguous bytes, exact
    length and FNV identity to the wall PAL16/B073 cache allocation.
  - 2026-07-17 DRAW_WALL B073 interpreter update: `c_gdatfile.cpp:1919-2003`
    and `c_querydb.cpp:2506-2668` now source-bind RAW7's descriptor, interval,
    output and lookup regions to a supplied owned PAL256 cache. The resulting
    cache is attached to the wall `DRAW_PICST` output receipt, but remains
    no-draw until the authentic U4-to-PAL256 blit consumer is proven.
  - 2026-07-17 DRAW_WALL native M11 update: the proven normal, unflipped,
    unmoved 0x40 U4-to-8 branch now consumes the authenticated B073 cache
    using `c_gfx_blit.cpp:495-548` source order. Scaling, flip, movement,
    clip, cache, surface and composition drift remain fail-closed.
  - 2026-07-17 DRAW_WALL HFLIP M11 update: the separately proven BLITMODE1
    branch now follows `blitline_48_mi/mima` reverse-X destination order.
    Vertical/chained flips, movement and scale changes remain fail-closed.
  - 2026-07-17 DRAW_DOOR panel M11 update: the stationary, closed, unflipped
    and unscaled DOORS panel now consumes its exact IMG3 U4 bytes, local PAL16,
    colour key, RAW4 rectangle and composition-owned surface. Opening,
    movement and light-remap branches remain fail-closed.
  - 2026-07-17 DRAW_DOOR split M11 update: only the source-proven horizontal
    opening states 1..3 now consume the paired halves in the `DRAW_DOOR`
    table order: right half (`base + state + 6`), then left half
    (`base + state + 3`). Both halves require the same authenticated DOORS
    material receipt and distinct RAW4 geometry rows, plus current composition
    and owner surface identities. Vertical opening, movement, flip and every
    incomplete table/material chain remain fail-closed.
  - 2026-07-17 DRAW_DOOR vertical M11 update: the source-proven vertical
    intermediate states 1..3 now retain the whole original DOORS image and
    select exactly `tlbRectnoDoorPosition[cell] + state` before one forward
    palette-mapped consume. The raw material, RAW4 table row, composition and
    live surface must all still match; horizontal split, movement and flip
    remain separate fail-closed routes.
  - 2026-07-17 DRAW_DOOR_FRAMES right-jamb M11 update: the stationary
    `QUERY_TEMP_PICST(1, 0x40, 0x40, ..., rect, 3)` route now admits the
    authenticated GRAPHICSSET U4/PAL16 side-frame with reverse-X writes. Its
    scene-owned colour key and current scene hash are required alongside RAW4
    geometry, composition and surface identity. Left jamb, panel flips,
    frame motion and every other transform remain fail-closed.
  - 2026-07-17 DRAW_DOOR_FRAMES left-jamb M11 update: the matching stationary
    `QUERY_TEMP_PICST(0, 0x40, 0x40, ..., rect, 4)` branch now consumes its
    authenticated GRAPHICSSET U4/PAL16 material in forward-X order. Receipt
    identity locks the jamb kind, RAW4 row, scene colour key/hash, composition
    and live surface, so it cannot be used as the mirrored right route.
    Frame motion, scaling and panel flips remain fail-closed.
  - 2026-07-17 DRAW_DOOR_FRAMES movement M11 update: only the source's
    `v1e12d0` branch may select `table1d6b2c[cell]` and its swapped
    `table1d6ee1` jamb column, while preserving the original cell's RAW4
    rectangle and normal jamb direction. The movement owner bit, selected
    field, scene, composition and surface must all match; panel motion,
    scaling and every unrelated transform remain fail-closed.

- 2026-07-17 DM2 pit-roof viewport admission: `c_gui_vp.cpp:118-206` now
  source-gates cells 1..8 on the exact roof flag, `LOCATE_OTHER_LEVEL`
  success, remote tile type 2, and remote bit 0x08 before applying
  `table1d6c4c/5e/67`. The resulting GRAPHICSSET SUMMARY_IMAGE, GFX256 raw
  receipt, decoded U4 bytes and local palette remain `no_draw`; cell 0's
  position flip, the actual remote-map address walk, and `QUERY_BLIT_RECT`
  placement/clip still require separate evidence.
  - 2026-07-17 prerequisite update: the admitted PIT_ROOF receipt now also
    binds `DRAW_DUNGEON_GRAPHIC`'s `DM2_query_B073` c_light transaction and
    the authentic INTERFACE_GENERAL/0/RAW4 row for rects `0x360..0x368`.
    Only the exact root `mode1=1/mode2=0` `QUERY_BLIT_RECT` form is admitted;
    changed c_light identity, palette, RAW4 row/table, clip chain, cell 0,
    and every richer rectangle branch reject. It remains no-draw until the
    full B073 palette expansion and a pixel consumer are separately proven.
  - 2026-07-17 alpha/blend update: `SKULLWIN/c_image.cpp:450-475` and
    `c_gfx_blit.cpp:370-549` now bind the exact U4 alpha transaction to the
    B073 and RAW4 identities. The source alpha mask is retained in full and
    its low nibble is the only admitted transparent source index; only the
    proven normal and horizontal-mirror modes enter the no-draw receipt.
    Mask drift, vertical/combined modes, palette drift, and destination
    identity drift reject. B073's transformed palette and final destination
    composition still need independent source proof before any blit.
  - 2026-07-17 B073 table update: `SKULLWIN/c_gdatfile.cpp:1919-2003` now
    binds the exact `INTERFACE_GENERAL/0/dt07/2` RAW7 program that initializes
    `v1e020c` and `v1e0210` for `DM2_query_B073`. The count/length layout,
    both packed table regions, trailing color lookup region, raw hash, and
    B073/material identities are retained as no-draw evidence. Missing dt07/2,
    malformed lengths, and any valid raw-data drift reject.
  - 2026-07-17 B073 traversal update: `SKULLWIN/c_querydb.cpp:2506-2668`
    now admits only the cache-free per-color traversal for the authenticated
    U4 palette. Every palette byte must have an in-range two-byte RAW7 lookup,
    group, subindex, interval and alternate alpha neighbour; index drift and
    alpha-branch ownership drift reject. The transformed palette is still a
    no-draw receipt pending the exact QUERY_PICST_IT destination composition.
  - 2026-07-17 destination update: `SKULLWIN/c_image.cpp:98-410` now binds
    the normal-scale (`0x40/0x40`), zero-crop PIT_ROOF `QUERY_PICST_IT` path
    to its root RAW4 `QUERY_BLIT_RECT`, B073 palette traversal, alpha mask and
    source-proven horizontal flip. Clip receipt drift and every scale/crop or
    unsupported flip reject. It remains no-draw: the destination bitmap's
    live ownership, dimensions/resolution and final viewport clip are inputs
    to `DRAW_PICST` that are not yet retained by this DM2 receipt chain.
  - 2026-07-17 surface-owner update: DM2 viewport ownership now publishes an
    atomic framebuffer snapshot with pointer, dimensions, stride, resolution
    and monotonically advanced generation. PIT_ROOF binds only the exact
    current generation and remains no-draw on stale or rebound surfaces.
  - 2026-07-17 composition-slot update: PIT_ROOF additionally requires the
    DM2 composition slot's before/after owner surface pointer and generation,
    session identity, data epoch and ordered-member identity. Every mismatch
    remains no-draw; native blit still lacks a source-owned M11 consume hook.
  - 2026-07-17 material-buffer handoff update: PIT_ROOF now retains a borrowed
    identity receipt for the already authenticated decoded U4 buffer. Its
    pointer, width, height, stride, pixel count, palette hash and material
    identity must equal the composition candidate; every buffer or receipt
    drift remains no-draw.
  - 2026-07-17 ordered-consume update: the source-owned PIT_ROOF hook now
    executes only `DRAW_PICST`'s authenticated normal-scale U4-to-8bpp masked
    rows, including the proven horizontal mirror. It consumes the borrowed
    handoff buffer directly after the composition-order and before/after
    surface checks; there is no reload or re-decode. Every crop, scale,
    vertical/combined flip, changed source index, composition/surface drift,
    or incomplete receipt remains no-write.
