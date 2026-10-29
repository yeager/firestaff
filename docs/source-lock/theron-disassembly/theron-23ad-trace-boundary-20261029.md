# Theron `$23AD` Trace Boundary Correction — 2026-10-29

## Static source boundary

The hash-locked US/JP bank-$1f listing at
[`theron-us-bank1f-consumer.asm`](theron-us-bank1f-consumer.asm) shows the
caller at `$2386-$23A3`. Its `BSR` at `$2390` enters `$23AD` and returns to
`$2392`. The `$23AD` routine's terminal `RTS` is `$2429`. The following
`$242A-$252A` bytes are the variable-bit reader, bank-mapping, and output
helpers; `$252A` is not the return boundary for the `$2390` call.

## Instrumentation correction

`scripts/apply_mednafen_theron_huc6280_23ad_trace.py` first watched for an
`RTS` at `$252A`; the actual terminal RTS for this call is `$2429`. A later
probe matched computed physical addresses `$1F2390` and `$1F2429`, but that
quietly assumed the static file projection was mapped at MPR `$F9`. The
source-lock establishes retail bytes at ISO offset `$1F0000 + address`; it
does not establish their live HuC6280 mapping. The 600-second run using that
assumption emitted no trace and is inconclusive.

The current probe searches opcode fetches for the caller's five-opcode prefix
at instruction offsets `0, 2, 5, 7, 10`. Only then it hashes the full authentic
30-byte caller and requires FNV-1a `$699E8DA1`, independent of logical PC and
MPR. It records the observed logical PC and all eight MPRs, checks that the
relative BSR targets the caller-relative entry, then matches the 382-byte
routine by FNV-1a `$3056F96C`. It starts instruction tracing only
when execution reaches that entry with the expected return address on the
stack. HuC6280 BSR pushes the operand address (`return PC - 1`), so the probe
checks the pushed `$2391`-relative stack word and reports the actual resume PC
separately. Completion is recognized at entry-relative offset `$7C` (the
byte-backed `$2429 RTS`) with that dynamic stack word still present.
The probe's first successful authentic invocation is recorded below. The
capture establishes the caller and routine byte signatures and a complete
runtime return boundary. It does not yet establish the decoder's source/output
buffer correspondence or the disc-to-RAM cause of the input bytes.

The capture verifier rejects literal `\\n` sequences so the generated trace
remains machine-readable. The PCE Fast compilation check below exercises the
probe's generated C code.

## Evidence boundary

Four 2026-10-29 US cold-start captures used the original CUE, System Card, and
authentic Ak-Tu-Ba Backup RAM with the PCE Fast research route. They reached
the signature-gated menu and generator route; two also logged a left event and
a re-entry event marked level `03`. None emitted a complete `$23AD` trace.
The first run used the old logical-PC/`$252A` boundary, the next two used
unproven logical or physical address assumptions, and the fourth used an
incorrect `$AD` candidate opcode and was stopped. Their missing traces do not
show that the caller failed to execute. Run a new cold capture with the
corrected `$A5` prefix probe before making that determination. These
research-route logs do not establish decoder behavior, level semantics, or
native Theron parity.

## Dynamic caller receipt and bound correction

A later cold USA capture used the authenticated 2 KiB Ak-Tu-Ba Backup RAM,
the original CUE, and System Card 3.0. The controlled Drator route reached
the generator start at level `$02`, left the starting tile, and re-entered at
level `$03`. The byte-signature probe found the caller at logical `$3B86`
with MPR slot 1 set to `$F8` (physical `$1F1B86`), and verified a BSR to
logical `$3BAD` (physical `$1F1BAD`) with stack operand `$3B91` and return PC
`$3B92`. The fetched 382-byte routine matched FNV-1a `$3056F96C`.

The first correctly configured capture did not complete the invocation: its
20,000-event bound ended in the routine's helper path before the caller
return. A second run raised that bound to 250,000 and again matched the same
caller and routine. At the cap, the decoder's remaining input count had
decreased from `$0A36` to `$076A` (716 bytes consumed, 1,898 remaining).

A subsequent authentic run raised the bounded event ceiling to 2,000,000 and
captured the full invocation at
`/dev/shm/firestaff-theron-fnv-probe-20261029/captures/us-authentic-bram-byte-signature-v5.trace.23ad`.
The trace had 921,436 lines and SHA-256
`3668f340372938c2a366237d11dd63158eb8c312e8372d0716f684eb968465bb`. It
contains one caller signature, one BSR, one terminal `RTS`, and no event-limit
truncation. The caller matched FNV-1a `$699E8DA1` at logical `$3B86`, MPR1
`$F8` (physical `$1F1B86`); the routine matched `$3056F96C` at `$3BAD`
(physical `$1F1BAD`). Its BSR operand was `$3B91`, resume PC `$3B92`, and
terminal `RTS` `$3C29` (routine offset `$7C`). The run used the original US
disc, System Card 3.0, and authentic Ak-Tu-Ba Backup RAM.

## Authentic source and output comparison

A subsequent capture filtered the complete invocation's actual memory reads
and writes. The 2,614 sequential reads at PC `$3C3E`, MPR slot 7=`$7C`, came
from physical `$0F8006..$0F8A3B`. The bytes match the original US Track 02
MODE1/2352 user-data stream at normalized ISO offset 3,825,670 (LBA 1871,
offset 6); the authentic six-byte frame header immediately before it is
`0A 82 3B 0A C8 13`. The source payload SHA-256 is
`5f6f6aa4a9e175c3016cac22e6967350fb72cb3ea2a091f5ca1fa8867d5e333b`.

The runtime wrote each physical byte from `$0D0000` through `$0D13C7` exactly
once, for 5,064 output bytes (SHA-256
`9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`). This
differential exposed a decoder bug: `$2496` subtracts one from each token
byte before doubling to a byte offset. Since the native pointer table is
represented as 16-bit entries, its array index is `token - $0101`. With that
correction, the native decoder produced 5,064 matching bytes: 2,125 tokens,
1,840 literals, 285 backreferences, 2 width markers, and final width 11. A
small codec-boundary test covers token `$0105`; it is not a game asset.

This establishes exact decoder input/output parity for this one authentic US
resource. The raw trace and media-derived bytes remain on TRV2 and were not
copied into the repository. The enclosing cold-run verifier still reports
`transition=missing` and zero authenticated CD-to-game-RAM receipts; full-level
completion, causal loader provenance, gameplay handoff, and other editions
remain open.

## Bounded post-return timeline instrumentation

The opt-in `$23AD` probe now keeps a bounded window after the matched `$2429`
return, capped at 200,000 post-return events. This window can include the
caller and downstream RAM consumer activity. Its records share the PCE Fast
CD/VWR/VCE event counter and can be merged into one chronology. The merger
labels the result as temporal ordering only; the shared sequence does not
establish causal data flow. A `post_return_truncated=event_limit` footer is
reported as possible truncation in the merged metadata.

The merger test passed three loops on TRV2. The complete instrumented Mednafen
1.32.1 PCE Fast build, with both the VWR and `$23AD` hooks enabled, completed
there with `-j1`. At that point this verified patch application and
compilation only; a later authentic capture is recorded below. The earlier
evidence still records the gameplay handoff and loader provenance gaps.

## Authentic post-return capture

A subsequent cold US run used the original USA CUE, System Card 3.0, and
authentic Ak-Tu-Ba Backup RAM. The scripted Drator research route reached its
generator start on level `$02`, logged a left event, and logged re-entry at
level `$03`. On that same run, the signature-gated `$23AD` probe recorded the
matched BSR and return and continued into its bounded post-return window. The
capture trace remains on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-23ad-post-return-20261029-r1/live.trace.23ad`;
its SHA-256 is
`a4bdc26eb82d6982490a717f873ee92fa3510818df941c67313c39592087e08d`. It
contains 1,121,429 sequenced events, one terminal decoder `RTS`, and one
post-return-window start. The window reached its 200,000-event cap. The
observed post-return PCs include `$F406`, `$F423`, and `$F425`; these are
execution observations only and do not identify their data as decoder output
or prove a gameplay consumer.

In this capture, the decoder wrote each physical address from `$0D0000`
through `$0D13C7` exactly once. The 5,064 ordered bytes hash to
`9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`, matching
the output hash from the earlier source/output differential. After the matched
return, the first observed write to the immediately following physical address
`$0D13C8` came from PC `$F425`. This adjacency is a useful candidate for the
next runtime handoff to investigate, but does not prove the write continues
the decoded stream or that the destination is consumed as a level asset.

The joined source-graphics chronology was generated on TRV2 and validated by
the merger. It contains 1,738,051 events from CD data reads, CD RAM-target
writes, VWR commits, VCE writes, and `$23AD`. Its metadata reports
`possible_truncation_at_event_limit` because VWR, VCE, and `$23AD` streams
reached their configured bounds. Its SHA-256 is
`ef8545f27b76c23936e19391eb1ec0c12b9cfbda0ae86708a760354a2c7264a4`.
The ordering is temporal, not causal. Raw traces and the joined timeline remain
on TRV2; no game media or BIOS was added to the repository.

This capture exposed a merger parser defect: valid event kinds such as
`page1_read` contain digits and were rejected by the previous name pattern.
The parser now accepts digits in event kinds, and the regression fixture covers
`page1_read`. The fixture passes locally, and the corrected merger processed
the full authentic chronology on TRV2. This verifies capture and join tooling,
not CD-to-RAM causal provenance, gameplay handoff, or full Theron parity.
