# Theron generator/RNG state receipt, 21 August 2026

Four independently captured 8 KiB PCE main-RAM snapshots from the authentic
US Track 02 runtime contain the same bytes at logical `$28b9-$28bb`:

```text
1a 62 29
```

The complete snapshot identities are:

```text
8a635d6d31631a2ac60778525b5ae603  atomic forward RAM
576ffb601cf19c3bbffea20e9e781a80  map-0 teleporter local
5b22784fe943674b7534c5131504e259  forward authentic
3d11a64095832d0f01a9d59bf061080d  map-1 door command
```

The byte-level `$4667` implementation maps seed byte `$00` to initial state
`42 29 29`. Two calls produce return bytes `$84`, `$62` and state
`1a 62 29`; the next call from the captured state returns `$9d`. The
`theron_v1_rng_source` regression locks this exact continuation.

This receipt proves the RNG state transition and an authentic observed state.
It does not prove that Firestaff has consumed every intervening original RNG
call. Generator direction, randomized Formicia group count and generated HP
therefore remain unavailable until their call order is bound.

The seven reachable authentic corridor-generator events resolve to four exact
records:

| Dungeon | Target | Raw record | Type value | Generation | Toughness | Pause |
|---|---:|---|---:|---:|---:|---:|
| Drator | `0c38` | `feff060680082040` | 12 | 1 | 32 | 64 |
| Formicia | `0c57` | `feff0608000e1020` | 16 | 12 | 16 | 32 |
| Thieves | `0c70` | `feff060180081040` | 2 | 1 | 16 | 64 |
| Demon | `0c64` | `feff060b800810f0` | 22 | 1 | 16 | 240 |

Drator is targeted by four events; the other records are targeted once each.
Generation 1 is a fixed one-member plan. Formicia's generation 12 carries the
random-count flag and a bound of 4, so it needs one authenticated bounded RNG
return before its member count can be published. The plan decoder retains
these source values without generating a direction, HP value or creature.

## Exact entry/return call order

The instrumented Mednafen producer now records the RAM-loaded primitive only
at logical `$4667/$467e`, physical `$0d0667/$0d067e`. HuC6280 return addresses
are read from the real stack page `$2100-$21ff`. An authentic three-second
autoload capture produced 26 complete, contiguous entry/return pairs:

- RNG-state trace MD5: `324febc4e5f278f04cd4aa2172d93637`
- transition receipt MD5: `c02dcf0844612a7360ded92e4ffd47c8`
- Track 02 capture-medium MD5: `ceb02343868f80cec899e9b239aff2da`
- System Card MD5: `ff1a674273fe3540ccef576376407d1d`
- authentic autoload-state MD5: `82e151fa51aa3e7d578d0dfdb09eb55b`
- instrumented Mednafen MD5: `7634065f128f7dd55572ab4f4bb93bd5`

The exact return owners were `$4647` five times, `$464d` nine times, and
`$cca4` twelve times. Every return state is the next entry state. The first
observed pair is `dc/ad -> 32/63`, called from `$cca4`; the next three calls
return to `$4647`, `$464d`, `$464d` and yield `ae`, `b7`, `53`. This proves the
primitive's real interleaving. It still does not assign one of these values to
a particular generator record without an event-bound witness.

The production world can now resolve all seven authentic floor-party events
to one unique source-generator record without executing it. The resolver
revalidates the original eight-byte source actuator, its effect, delay, sound,
target coordinate and the generator identity. The real event locations are:

| Dungeon | Event refs and source squares | Generator |
|---|---|---|
| Drator | `0c81` `(2,3)`, `0c94` `(11,7)`, `0cac` `(12,2)`, `0c9e` `(12,3)` on level 2 | `0c38` `(4,3)` |
| Formicia | `0c6b` `(8,6)` on level 1 | `0c57` `(7,2)` |
| Thieves | `0c7f` `(18,16)` on level 2 | `0c70` `(17,3)` |
| Demon | `0c58` `(16,9)` on level 2 | `0c64` `(18,5)` |

This closes event-to-record identity but deliberately leaves each event queued:
no timer, RNG value, direction, HP or creature is published by the resolver.

The next capture schema (`rng-state-v2`) adds the physical caller and the
original `$B0/$B1/$B3-$B6/$B8/$BA/$BB` context. A real autoload smoke capture
produced 18 contiguous pairs (trace MD5
`aeafc83b80296b4a63c345c127644de1`). It proves `$4647` maps to physical
`$0d0647`, `$464d` to `$0d064d`, and `$cca4` to `$0d8ca4`; the capture script
accepted the complete state-continuity contract. This diagnostic capture did
not trigger one of the seven generator events and therefore does not yet open
spawn publication.

## Event-bound materialization receipt

`theron_v1_world_bind_track02_generator_execution_witness` is now the strict
join point for a future live capture. It re-resolves the queued event, matches
both exact eight-byte actuator records against the authenticated world ledger,
and exposes the fixed generator plan. Publication remains false unless the
same capture sequence also proves an original `$4644` or `$4667` return edge,
its physical caller's source bytes, the return boundary, and the generator
consumer contract. Thus the existing smoke captures are useful provenance but
cannot be mistaken for a generator execution. All seven real US events pass
the source join and remain closed; changing one generator byte is rejected.

## Full main-RAM and caller-byte capture

The Mednafen producer now writes an 8 KiB main-RAM image at every authentic
physical `$0d0667` entry, together with the logical and physical return owner
and 32 bank-mapped bytes around that owner. This is raw same-instant evidence;
the producer assigns no generator, creature or timer meaning to the bytes.

A three-second run from the research-only Drator positioning state produced
156 complete contexts plus the provenance header (context sidecar MD5
`ac6e59f035602993755d7d1dbd16fd12`). Its exact return-owner/code windows are:

| Return owner | Physical | Calls | Captured caller window |
|---|---:|---:|---|
| `$4639` | `$0d0639` | 21 | `00f00f85c4206746c5c49006e5c4b0fc65c460206746290160206746290360ad` |
| `$4647` | `$0d0647` | 68 | `b0fc65c460206746290160206746290360adb7280a69428db928adb728696449` |
| `$464d` | `$0d064d` | 37 | `6746290160206746290360adb7280a69428db928adb7286964494d8dba288dbb` |
| `$cc33` | `$0d8c33` | 3 | `e445d01e5a20674629021869018d77287aa54608a900283008c4469002d002a9` |
| `$cc55` | `$0d8c55` | 6 | `c446d01eda20674629021869008d7728faa54508a9032830e6e4459002d0e0a9` |
| `$cca4` | `$0d8ca4` | 18 | `20d6cbd016206746290285c468aa1a1865c429038d77287a7a8a60204446f00b` |
| `$da5a` | `$0dfa5a` | 3 | `03c902d00d20674629021865bb1a290385bba4bba6baa5b8208fcb85b8a5b9f0` |

The `$cc33`, `$cc55` and `$da5a` windows now prove their immediate original
post-RNG masks and stores byte-for-byte. The run did not place one of the four
raw Drator type-6 records directly in main RAM and scripted UP/RIGHT probes did
not change the RNG boundary sequence. Therefore this is a stronger caller
contract, but not yet the event-bound generator witness required by the
materialization receipt. The generic capture wrapper stopped later on its VDC
snapshot boundary; the independently line-delimited RNG sidecars and their
entry/return continuity remain the evidence described here.

## Original dungeon-bank selection remains the live boundary

The authentic savestate at party position `(2,3)` was verified directly from
Mednafen BaseRAM: `$2040/$2041 = 02/03` and `$203F = 00`. It is nevertheless
an Akutuba state, with `$20DA/$20DB = 01/00`; matching a Drator event coordinate
alone is therefore insufficient. A new research run from the byte-identical
post-Akutuba state (MD5 `f17f377df210b4a3ae904a13fb85a7f0`) executed the
original `DMS-SG.001` ordinal-1 path and six scripted original-controller
presses. The original SCSI request for LBA 4201, four sectors, completed, but
the final 8 KiB BaseRAM still contained `$20DA/$20DB = 01/00`. This is negative
evidence: the run proved Drator's campaign bit again but did not reach original
stage selection, so none of its RNG calls may be attributed to Drator.

The live-capture transition receipt now records the raw final values as
`dungeon_bank_20da` and `dungeon_bank_20db`. These fields are provenance, not
an inferred dungeon identity. A future event-bound capture must show the
source-locked Drator bank pair and an authentic level-2 load before the
generator materialization gate can open.

## Authentic Drator cold-start boundary

The original-menu route now closes that dungeon-selection prerequisite. A
cold start with the real `DMS-SG.001` campaign mask `01` selected the first
and only unlocked scenario through the original menu code, loaded Drator's
Track 02 sector chain, and reached level 2 at `(2,3)`, facing east. This is
documented separately in
`theron-authentic-drator-menu-route-2026-08-21.md`.

A same-session 160000-frame capture then remained stationary on `(2,3)`.
It produced no RNG-state, RNG-code, RNG-consumer, or generator-context
sidecar. This is a useful negative boundary: loading on event `0c81` does not
itself execute the corridor generator. The party must leave and re-enter the
event square through the original movement path before any RNG return may be
attributed to Drator's `0c38` generator record.

## Cold-start panel-dispatch boundary

The known original forward-panel sequence was replayed after the authentic
Drator cold start. Its timing was tied to Mednafen's public input-frame clock,
so the route is independent of how many emulated controller ports are updated.
The exact directional holds move the original raw cursor from `$78/$78` to
`$30/$8f`. A RAM snapshot at the same pre-click boundary in the authentic
autoload state reaches the identical `$30/$8f` raw tuple. The snapshots are:

```text
c9c6881d8c42c3f2539458999943ee9  cold-start pre-click RAM
2ead7050a890723ab84a0809bb329fe2  authentic-autoload pre-click RAM
```

Button I also reaches the same original scratch/input producer observed in the
successful capture: the CPU writes `$01` to logical `$28b8` at `$44e5`. This
write alone is not a command-queue receipt. In particular, adjacent `$28b7`
is a rolling work counter that repeatedly cycles `$00..$10` at `$434f`; it is
not a queue pointer. The cold-start session nevertheless does not enter
the pointer conversion at `$d52a`, does not produce `$20cd=$8a`, and does not
queue command `$03`. Thus neither cursor targeting nor controller delivery is
the remaining failure. The unresolved boundary is the original consumer of
the `$28b7/$28b8` input queue after a cold start.

An earlier comparison against `trace.command-before.ram` was rejected: that
snapshot is taken after pointer conversion and already contains transformed
`$2e3b/$2e7b` values. It is not an idle-panel reference. Likewise, `$293f`
must not be treated as a mode flag merely because its value differs; the
authentic trace shows it being written during a block-copy operation at
logical `$b723`.

These panel runs by themselves do not attribute a generator or RNG value.  A
later controller-only raster pass did, however, find the original movement
hitboxes and closed the missing transition described below.

## Authentic Drator leave/re-enter witness

Starting from the same real cold-start session, original controller input moved
the party backwards from level 2 `(2,3)` to `(1,3)` while it remained facing
east.  A second controller-only pass then returned it to `(2,3)`.  The game
logged both committed positions in one process:

```text
start           level=02 x=02 y=03 direction=01 input_frame=16918
left-event      level=02 x=01 y=03 direction=01 input_frame=19146
reentered-event level=02 x=02 y=03 direction=01
```

The route uses no RAM mutation.  Its only generated input is a research raster
whose purpose is to discover the real panel geometry; the campaign, dungeon,
map, event records, movement result and RNG execution all come from the
authenticated US disc and BRAM.

The first capture armed RNG tracing after the committed `(1,3)` observation,
not after the later `(2,3)` re-entry.  It contains 811 contiguous entry/return
pairs, numbered 0 through 810, and 811 full 8 KiB entry snapshots.  Sequence 0
is `$4667 → $cc55`, returns `$8f`, and still sees `(1,3)`.

The negative capture's immutable artifact identities are:

```text
404564360221b9f10e78fc554aac6c2c  mednafen.log
5cddf7f48eee8ca19e8da88e21a4dd21  IRQ trace
a6630524618b0000045cb039c7060116  RNG entry/return state
87a4e58b9b44588b97bf055859819cc9  RNG generator contexts
598679c026c33f59b7e46d0ee00f3e10  RNG consumer trace
5dbea13a3d4a709332ba485be2fb7ae5  RNG code windows
```

The follow-up run opened the identical trace gate only after the game committed
the return to `(2,3)`.  Its first boundary, new sequence 0, is byte-for-byte
identical to the first run's sequence 1 at the complete RNG-state line after
normalising the sequence number.  Both are `$4667 → $4639`, enter with state
`98 8f 29`, and return `$f8`.  The caller window and all named RAM context
fields are likewise identical.  Therefore exactly one RNG edge separates the
two gates: the first run's sequence 0 at `$cc55`.  It belongs to the original
movement/event transaction, not to later frame AI.

At that edge the state changes `2a 54 29 → 98 8f 29`.  The captured `$cc55`
window is
`c446d01eda20674629021869008d7728faa54508a9032830e6e4459002d0e0a9`;
it contains `JSR $4667`, `AND #$02`, `CLC`, `ADC #$00`, and `STA $2877`.
The entered source event is `0c81`, raw `330c0300c4100019`, and its unique
resolved target is generator `0c38`, raw `feff060680082040`.

The post-entry capture contains 660 complete pairs plus its header.  Its
artifact identities are:

```text
26adb1ca01b2f98cb1099c9a6d61a0c3  IRQ trace
63ccac8ce49f1da9a371647dba68ef9b  RNG code windows
28734f3a7253eb2aae950de388ebadf6  RNG consumer trace
ac37e03a137a21e7f41b48c65d60b9ab  RNG generator contexts
045a7a41f938863a5771a7f3fbe76989  RNG entry/return state
```

The materialization receipt now checks the exact logical and physical entry,
`$cc55` return owner, caller bytes, pre/post state, `$8f` result and the
`$4639` successor continuity.  A changed caller byte or successor state is
rejected.  This opens only the captured Drator `0c81` fixed-one-member plan;
other events and unobserved direction/HP/timer consumers remain closed.

## Same-transaction runtime group write

A follow-up run used the same authenticated US CUE, System Card, BRAM and
controller-only Drator route. A temporary write probe remained dormant until
the already captured `$cc5d` store wrote `$02` to `$2877` while the RNG state
was exactly `98 8f 29`. It then recorded the original code's writes without
changing RAM.

The event transaction constructs the ten-byte working row
`a500000120ee11030800` at `$2935..$293e`. The original `$cbce` copy loop
(physical `$0dabce`) writes those same bytes, in reverse write order, to
runtime slot 0 at `$60ff..$6108`; `$cbaf/$cbbe` then publishes slot 0 through
the `$6000` index table. The relevant bounded write sequences are 3733–3749
in `work/theron-drator-generator-materialization-v2/trace.stderr`.

A second controller-only replay added a read probe which armed from the same
exact `$cc5d`/`98 8f 29` signature.  After publication, the first original
consumers read `$60ff..$6101 = a5 00 00` at logical PCs
`$c9f3/$c9fb/$ca02` (physical `$0da9f3/$0da9fb/$0daa02`).  The original
`$ca78` write (physical `$0daa78`) then unlinks index `$600d`.  Firestaff now
requires this read-and-unlink lifecycle as well as the ten writes.  This also
shows why the slot cannot be treated as a permanent host creature record:
the original runtime consumes and unlinks it during the same event flow.

This proves the byte-exact generated runtime row, its allocation and its first
consumer lifecycle in the same transaction as the event-bound RNG call. It
does not by itself name
runtime byte `$20`, `$ee`, `$11`, `$03` or `$08` as creature type, HP,
direction, position or timer. Production therefore verifies and retains the
raw row but does not yet translate those bytes into native creature fields.

## Source-bound position fields

The authenticated US Track 02 BIN (MD5
`f23601102138f87c33025877767ebf76`) contains the original consumer at raw
offset `$a1612`, mapped to logical `$c852` in its observed bank. Its exact
27-byte span is:

```text
ad3b2985b5ad3c2985b6ad3d2985b4a5b4186902290385bb20f851
```

The routine copies working-row offsets 6 and 7 from `$293b/$293c` to `$b5/$b6`
and ends by calling `JSR $51f8`. It copies offset 8 from `$293d` to `$b4`,
adds two, masks with `$03` and stores the result in `$bb`. The bytes therefore
prove those copies and the low-two-bit transform, but this span alone does not
prove that `$b5/$b6` are map-local X/Y or that `$bb` is direction. In the
captured row the retained values are byte 6 = 17, byte 7 = 3, byte 8 = 8 and
the transformed low-two-bit value = 0.

The materialization receipt requires the exact logical address, raw offset and
source bytes before exposing those four retained values. Its field names are
provisional and do not authorize host-coordinate placement. A changed source
byte or file offset closes the gate. This is a static source join against the
same authenticated retail BIN. The research probe in
`work/theron-drator-generated-working-consumers-v7/` did not observe `$c852`
consume this exact generated row during the captured session. Accordingly the
receipt calls the fields source-verified, not same-session-consumed. The row's
type, HP and timer ownership remain unproven and are not materialized.

## Post-event bank-dispatch trace

Two further controller-only replays used the same authenticated medium, BRAM
and Drator route. The first dumped the active target behind the immediate
`JSR $5d58` after the generated row had become
`a500000120ee11030800`. At that instant MPR2 was `$68`, so `$5d58` mapped to
physical `$0d1d58`. The six-byte wrapper is `20e345f5aa60`: it calls the common
bank dispatcher at `$45e3` with inline target `$aaf5` and then returns.

The second replay captured `$aaf5` itself. MPR5 was `$70`, giving physical
`$0e0af5`. The authenticated code updates four-byte ranges at `$287f..$2882`
from `$2897..$289a`, followed by a two-byte pointer pair at `$289b/$289c`.
It is a spatial range/index update and supplies no source basis for naming a
creature type, HP or timer. Its trace is
`work/theron-drator-generator-aaf5-v9/trace.stderr` (MD5
`60f11db9a20d3eec8ecddb1eeb028d24`).

A bounded 4,096-instruction trace starting at the exact event store `$cc5d`
is retained in
`work/theron-drator-generator-instructions-v10/instructions.log`. It proves
that the original performs several thousand instructions of movement,
lookup and bank-dispatch work before the later row-publication boundary; the
limit expires before publication. It also observes logical `$c852` at
physical `$0dc852` early in the transaction. That is a different bank from
the source occurrence at raw `$a1612` used for the static position-field
receipt. Logical address equality alone is therefore explicitly insufficient
for a same-session consumer claim. The next probe must arm nearer the later
`$47b7` object-key write and retain physical PC/MPR identity.

The focused follow-up arms at the generated object-key write `$47b7`
(physical `$0d07b7`) instead. Its 4,096-instruction window contains the whole
first-row lifecycle: build entry `$c799` at sequence 2, ordering `$ca7e` at
431, final toughness write `$caa3` at 467, copy loop `$cbc9` at 592,
publication `$cbbe` at 795, first row read `$c9f3` at 936 and unlink `$ca78`
at 966. The instruction log is
`work/theron-drator-generator-focused-instructions-v11/instructions.log`
(MD5 `96ff3916a93d21e3f91acf9ff10ae16b`). The production receipt now requires
all eight logical PCs, physical PCs and sequence ordinals in exactly this
order. Mutating one ordinal closes the gate. This binds allocation, build,
copy, publication, first read and unlink to one bounded authentic execution;
it still does not identify a host creature type, local-map coordinate or HP.
