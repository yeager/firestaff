# JP cold map-byte writer capture (2026-11-01)

## Capture identity

This reference-only attempt started the authentic Japanese Rev. 1 CUE with no
autoloaded state. Track 02 has MD5
`b7afb338ad31be1025b53f9aff12d73a`; System Card 3.0 has MD5
`ff1a674273fe3540ccef576376407d1d`. The isolated Mednafen 1.32.1 PCE Fast
binary has SHA-256
`688f12959c49aa51d36ced85f4bd8223e87dfce070e1a20bf36f23c822b95c67` and
includes `$0E8B14` in its CD-RAM target-write allowlist.

The cold replay scheduled 15 RUN/Button I events, including 4-second holds
for RUN and the two menu selections, then twelve 1-second Button I presses.
All 15 scheduled event records were emitted. The capture wrapper rejected the
package because it did not observe the System Card `$E11D` BIOS-to-game launch
branch (`transition=missing`). The corrected scripted-input verifier also
blocks this trace: at least one event interval has no controller read exposing
its scripted mask. It processed all 15 event records and 883,598 input-read
rows. Do not treat the event records alone as proof of guest consumption.

## Verifier correction

The capture exposed a verifier bug: PCE Fast `read_index` is the serial shift
position within a controller poll (values 0–4), while `scripted_port` identifies
the gamepad slot. At frame 480, the trace records RUN on slot 0/index 0
(`$E4C5`, active-low value `$37`); later serial indexes in the same poll
correctly return released values. The verifier had rejected those later indexes
as a different controller slot. It now validates `scripted_port` and physical
input separately and uses the selected serial index only when accepting a
controller-read witness. On this authentic trace that false rejection is gone,
but the actual consumption gate still blocks at least one scheduled event.
This does not establish a game-owned poll or action.

## Bounded observations and limits

- The bounded System Card status, callback-state, I/O/RAM, and decision
  validators passed with 65,536, 382, 33, and 88 rows respectively. The wrapper
  still rejected the capture because the `$E11D` branch was absent.
- The CD-RAM target-write trace recorded 590 rows under its 65,536-row cap.
  It contains no write to `$0E8B14`; because the route did not establish the
  game handoff, this says nothing about the byte's gameplay loader provenance.
- The exact-LBA 3674 CD data-port trace contains zero rows. No authenticated
  Track 02 read or CD-to-RAM receipt was established.
- The movement-context trace contains 18 `$50D2`/`$50D6` map-index instruction
  contexts and 1,887 map-array instruction-context rows, but zero
  `map_array_read` rows. It also records two `$2905` command writes and two
  `$2040/$2041` state writes; their meaning and relation to the scripted events
  are not established by this rejected run.
- The main-RAM consumer trace reached its 4,096-row sample limit and is
  truncated. The retained rows cannot be used as an exhaustive consumer census.

The input trace SHA-256 is
`ca601f434011561bc7037cb59e53659732c110fa9d30630a52d763b5fbb0f17d`; the
CD-RAM target-write trace SHA-256 is
`5723c875718019e28534157641814af7ab763fea9f7efcb93da56c70bfa1a90e`; the
movement-context trace SHA-256 is
`71087739e6295e80f71340f323b6343f1d00fe512d07b36281d7c538c816899a`. Raw
traces and media remain on TRV2 and are not committed.

Next, resolve the remaining input-poll gap and obtain an uncapped accepted
cold handoff. Then repeat the exact-address writer trace through the JP loader
and join any write to the subsequent `$0E8B14` read, active-map consumer, and
video state. See the [writer-trace audit](theron-jp-map-byte-writer-trace-audit-20261101.md),
[capture readiness](../../theron/THERON_CAPTURE_READINESS.md), and
[Theron TODO](../../../TODO-theron.md).
