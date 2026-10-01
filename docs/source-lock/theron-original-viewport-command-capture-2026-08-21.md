# Theron's Original Viewport Command `$50`

## Authentic Capture

The capture uses the US edition's authentic Track 02 with MD5
`ceb02343868f80cec899e9b239aff2da`, System Card 3.0 with MD5
`ff1a674273fe3540ccef576376407d1d` and the Mednafen save state
`f17f377df210b4a3ae904a13fb85a7f0`. The instrumented Mednafen binary's MD5 is
`3731a8a78f91c5cc355546b27e7ba418`.

Button I is written as `$28b8=$01` from logical PC `$44e5` and physical PC
`$0d04e5`. The viewport click `$3c/$78` queues `$2905=$50` from
`$ccdb/$0dacdb`. The queue is cleared at write 308 from `$d3a0/$0db3a0`.
The trace continues to the explicit boundary of 65,536 ordered main-RAM
writes.

Verified MD5s:

- command trace: `a66d3a52cf2d246f80a915017a70a053`
- main-RAM consumer trace: `84634112aaa47e0ada4f86d453697aa6`
- 64 KiB code image: `036f62625740c7c887b0c588f2fa175b`
- RAM before: `a321518da370a456a36758b7c54a0cf1`
- RAM after `$2905=$00`: `2c4953862f6f6d52dbe2985ad0f9cf39`

A control run from the same save state without a button press writes only the
input buffer's empty `$f0/$00` polls. It creates no command or RAM images.

## Machine Gate

`theron_v1_original_command_capture_admit()` requires all identities above,
the exact Button-I edge, an unbroken sequence `0..65535`, the correct physical
main-RAM address for each write, the queued command, queue termination and
the correct code and RAM image sizes. The receipt re-hashes all artifacts.

The consumer trace is part of the same receipt. Between the original's first
read of `$2905=$50` at `$D34D` and the first subsequent read of `$2905=$00`,
there are **0** reads in `$2600–$27FF`. A missing, reordered or
malformed consumer trace is rejected.

## Semantic Boundary

`$50` here is only a verified original command type for a viewport click.
The capture does not identify the clicked Track 02 record's dungeon, level,
chain reference or T900 consumer. The receipt therefore always sets
`semantic_publication_allowed=0`. It must not be used to open a door, use an
item or bind starting equipment until that same transaction can be connected
to an exact source occurrence and an observed state change.

A separate research probe moved a copy of the authentic save state from
`(2,3)` to `(4,4)`, in front of the real US Track 02 door on map 0 at
`(5,4)` (`source_ref=0015`, index 21, raw record `fe ff 20 00`). Even there,
the standard click `$3c/$78` yielded zero source-area reads in the command
window. The position copy was synthetically adjusted solely to find the next
real consumer and is explicitly not admitted as game state or semantic
evidence.

## Bounded Forward Collision with the Real Door

A new research producer arms on the Button I edge and logs only reads in the
HuC6280 main-RAM window `$2000–$3fff` until the original clears the command
type. This prevents instruction fetches from filling the bounded buffer before
a late command. The same position copy and reproducible plan
`right@1:140,down@2:33,i@145:5` produced original command `$03`, followed by
`$2905=$00`, and a complete boundary after 55,078 ordered RAM reads.

The window contains 96 reads in `$2600–$27ff`. Observed addresses are primarily
in `$271b–$2724` and `$27af–$27c8`; reader PCs include
`$c1fd`, `$c2d8–$c450` and `$a01e–$a6e6`. The trace MD5 is
`6cdee36618c6cdbd43ad3f4fe39a6c47`.

Physical remapping of the code image now separates preparatory work from the
command dispatch itself. `$2905=$03` is first read at the original's `$d34d`
at sequence 50,862. After that point there are **0** reads in `$2600–$27ff`.
All 96 reads above therefore occur before the original dispatches the forward
command. The capture receipt consequently reports `command_consumer_source_reads` and
`command_consumer_post_dispatch_source_reads` separat.

This is a positive main-RAM time window, but not in itself causal consumer
evidence. The HuC6280 core can interrupt an ongoing block transfer without the
PC reported by memory instrumentation leaving the block instruction's
address. Reads from interrupt routines can therefore receive a misleading PC
label inside the movement routine.

Targeted research mutations confirmed the boundary. `$271b/$c450`,
`$271e/$c3f1`, `$272b/$c1fd` and `$27af–$27c8` changed loop, rendering or
temporary working state, but did not bind the real door record. `$2098` was
given value `$81`, whose high bits structurally resemble the map format's door
type, but neither a point mutation on the read, a persistent mutation at the
Button I edge nor a mutation of the write `$81→$01` changed movement or final
position. The field is therefore not proven as a movement consumer.

Door semantics remain fail-closed until a trace with the actual execution PC
or a write/branch mutation can bind an exactly transformed runtime byte to the
real record `fe ff 20 00` and an observed state branch.

## Data-Read Trace and Verified Open-Cell Control

An isolated research build now distinguishes HuC6280 data reads from
instruction and operand fetches and arms only when `$2905=$03` is actually
read at `$d34d`. The door run ended after 5,016 data reads
(`f8a8b69d6ec82118a45b11b00b21e2df`). A control from the same authentic
save state changed only the two serialized X coordinates from `2` to `1`;
the party then stood at `(1,3)` and the original moved it to the genuinely
open cell `(2,3)`. The control trace ended after 6,830 data reads
(`21cc2557bc97a16cc04c2c9e975fe018`).

The first control-flow difference is in the original's boundary check. For
the door position, target X `5` is calculated and compared against the level's
exclusive X boundary `5` at `$4fbb–$4fbd`; the routine returns blocked before
the corresponding Y/cell check. The open control's target X `2` passes and
continues through
`$4fc0`. The banked read at physical address `$0e8af7` returned `$10` in the
door case, while the control's coordinate-dependent address `$0e8ae1` returned
`$20`. A research mutation `$0e8af7:10→20` later changed working state but
neither the boundary branch nor the final position.

This proves that the observed forward collision is a map boundary, not an
open-door consumer. It must therefore not be used to infer T900, door-button,
key or actuator semantics. The next positive door capture must use the
original interaction path and show a source-bound state branch; the production
path for real doors remains closed in the meantime.

## Original Right-Column Command `$74`

The authentic VDC/VCE image from the door position shows that `$3c/$78` does
not hit the front face. With the same real disc, BIOS and save state, a
reproducible PCE input plan moved the cursor to `$79/$62`. The original's own
hover routine then wrote `$2911=$74` at `$d57e`, and Button I queued
`$2905=$74` from `$ccdb`. The command was cleared from `$d3a0` after 69,208
ordered main-RAM writes. The complete data-only window contains 72,693 reads
and an explicit boundary record; the trace MD5 is
`c9078f2d894ce3025ebe094714a7aeea`.

A horizontal hover sweep at logical Y `$78` shows the relevant boundary: the
original writes `$50` for X `$00–$6f`, then snaps the cursor to `$79/$78` and
writes `$74`. A later vertical movement gives `$79/$62` with the same command.
Thus, `$74` belongs to the right column and is not a front-cell or door zone.

The same right-column point and command were run from the verified open
control position. The first 4,381 data reads followed the same control path,
and both runs read the same 60 bytes in `$2600–$27ff`. Neither the final
position nor any source-bound door record changed. `$74` is therefore a proven
original code for the right column, but not a door consumer or basis for view
geometry.

Firestaff's older V1 click matrix published nine host-created 320×240 regions
as though they were Theron's V1 geometry. The original image is 320×200, and
two proven points are insufficient to infer rectangle boundaries. The nine V1
regions have therefore been removed. V1 queries now return no zones until the
original's complete rectangle table or equivalent complete boundary trace is
recovered. The explicitly modern V2 overlay remains as presentation geometry
and is not used as original data.
