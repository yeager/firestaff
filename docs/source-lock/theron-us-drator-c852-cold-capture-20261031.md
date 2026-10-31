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

The first run directory on TRV2 was
`/tmp/firestaff-theron-c852-pr238-20261031/capture/drator-c852-cold-runhold4s-20261031-2125/`.
The follow-up run directory is
`/tmp/firestaff-theron-c852-pr238-20261031/capture/r4-cold-runhold4s-filtered-20261031/`.
It used a clean-source PCE Fast build with an 8,388,608-row System Card
I/O/RAM cap, a 524,288-row callback-state cap, a filtered `1F093B-1F093D`
consumer range and a runtime code-window `$C852` hook. It ran for about six
minutes and reached `start` and `left-event`, but not `reentered-event`.

## Observed results

- The initial broad-filter `$C852` hook recorded 139 opcode fetches. The first
  was logical PC `$C852`, physical PC `$0DC852`, shared event sequence
  1,590,068. The trace
  records fetched opcode, MPRs and registers.
- The initial main-RAM consumer trace sampled broad physical range `$1F0000` and
  stopped at its 65,536-row sample limit. It did not retain the target reads
  around the Drator event.
- The initial capture wrapper failed verification because the callback-state trace
  reached its old 524,288-row cap. The System Card status sidecar passed for
  65,536 rows. A later tool change raises the callback cap to 4,194,304, but
  this capture was not rerun with that binary.
- In the follow-up, the filtered consumer trace reached its 1,048,576-row
  sample limit before route reentry. It establishes no same-event generated-row
  read. The target-write trace contains 139 rows.
- Callback-state verification passed: 403,975 rows, comprising 87 reads and
  403,888 writes. The System Card I/O/RAM trace reached its configured
  8,388,608-row cap. Its verifier attempted to materialize all rows in a list,
  reached about 3.2 GB RSS, and was stopped before completing; wrapper
  verification did not finish. The verifier has since been changed to process
  rows incrementally, with iterator-mode regression coverage. The full trace was
  processed at about 18 MB RSS and correctly rejected at its explicit
  truncation marker.
- The follow-up `$C852` trace contains two fetches at physical `$0DC852`, at
  shared event sequences 10,644,900 and 10,648,164. Both captured 27-byte code
  windows beginning `a5ba85a9a5b885a8`. This runtime window differs from the
  static source window previously attributed to logical `$C852`; runtime bank
  and source identity remain unresolved.

## Conclusion and next capture

Together the runs prove that authentic cold US media reaches the Drator
generator route and that runtime fetches occur at logical `$C852`, physical
`$0DC852`. They do not prove that those fetches read the generated row from
the separate lifecycle receipt. The follow-up also exposes a runtime code
window that is not yet tied to the static source attribution, and its wrapper
verification did not finish because the I/O/RAM verifier's whole-file list
exhausted several gigabytes of memory. This is not native Firestaff gameplay
or renderer parity evidence. Keep row bytes semantically opaque.

The System Card I/O/RAM verifier now streams rows with bounded memory. Rerun
the retained capture through it and give consumer tracing an event-aware
window or a sufficiently large bounded sample limit. Resolve the runtime
`$0DC852` bank and code bytes
against the authentic ROM mapping before attributing the fetch to static
`$C852` source. Then repeat in a fresh isolated profile and accept a row
consumer join only when same-session event order, reader PC and physical bank
bind the fetch and generated-row reads. The native viewport still requires a
separate map/pose-to-VDC/VCE/VRAM-to-frame join.

See [capture readiness](../theron/THERON_CAPTURE_READINESS.md), the
[Theron TODO](../../TODO-theron.md), and the
[generated-row source lock](theron-generator-rng-state-2026-08-21.md).
