# Theron US Drator `$C852` cold capture boundary (2026-10-31)

## Purpose and provenance

This reference-only capture tested whether authentic cold media can reach the
Drator generated-row route while recording instruction fetches at logical
`$C852`. It used the hash-verified US Track 02, System Card 3.0 and the
authentic Akutuba-complete BRAM listed in the private TRV2 capture receipt. A
four-second RUN hold was used at the System Card prompt. The route controller
was source-signature gated; it supplied input to reach original game states
and did not synthesize a save, game record or trace evidence. Emulator,
firmware, game data and traces remain outside Git.

The run directory on TRV2 was
`/tmp/firestaff-theron-c852-pr238-20261031/capture/drator-c852-cold-runhold4s-20261031-2125/`.
The trace binary was built before the System Card callback cap was raised from
524,288 to 4,194,304 rows. The run lasted ten minutes and reached the route
diagnostics `start`, `left-event`, and `reentered-event`.

## Observed results

- The `$C852` hook recorded 139 opcode fetches. The first was logical PC
  `$C852`, physical PC `$0DC852`, shared event sequence 1,590,068. The trace
  records fetched opcode, MPRs and registers.
- The main-RAM consumer trace sampled broad physical range `$1F0000` and
  stopped at its 65,536-row sample limit. It did not retain the target reads
  around the Drator event.
- The capture wrapper failed verification because the callback-state trace
  reached its old 524,288-row cap. The System Card status sidecar passed for
  65,536 rows. A later tool change raises the callback cap to 4,194,304, but
  this capture was not rerun with that binary.

## Conclusion and next capture

This run proves that the authentic cold US route reaches Drator and that the
hook records `$C852` at the expected physical code address. It does not prove
that `$C852` reads the generated row observed by the separate lifecycle
receipt, and it is not native Firestaff gameplay or renderer parity evidence.
Keep row bytes semantically opaque.

Rebuild the instrumented PCE Fast reference binary with the raised callback
cap. In a fresh isolated profile, repeat the authentic route with
`THERON_CAPTURE_PCE_FAST_MAIN_RAM_CONSUMER_RANGE=1f093b-1f093d` and a bounded
consumer sample limit large enough to include the Drator event. Optionally
record target writes with
`THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE=1f0935-1f093e`. Accept a
join only when same-session event order, reader PC and physical bank map the
`$C852` fetch to the generated-row reads. The native viewport still requires a
separate map/pose-to-VDC/VCE/VRAM-to-frame join.

See [capture readiness](../theron/THERON_CAPTURE_READINESS.md), the
[Theron TODO](../../TODO-theron.md), and the
[generated-row source lock](theron-generator-rng-state-2026-08-21.md).
