# Theron US Drator `$C852` cold capture boundary (2026-10-31)

> **Later follow-up (2026-11-01):** the cold US replay with the independent
> 129-dispatch sidecar records `$C852` → `$C85E` `JSR $5039` → `$5039` under
> MPR2 `$68`. The live target bytes differ from this static candidate; live
> `$45C3` windows fail retail-source verification, and bounded traces reach
> their caps. The package remains rejected. See the
> [ordered call-window report](theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md).

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
  sample limit before route reentry, after retaining the same-session row-byte
  reads documented below. The target-write trace contains 139 rows.
- Callback-state verification passed: 403,975 rows, comprising 87 reads and
  403,888 writes. The System Card I/O/RAM trace reached its configured
  8,388,608-row cap. Its verifier attempted to materialize all rows in a list,
  reached about 3.2 GB RSS, and was stopped before completing. The verifier has
  since been changed to process rows incrementally, with iterator-mode
  regression coverage. The full trace was processed at about 18 MB RSS and
  correctly rejected at its explicit
  truncation marker.
- The first follow-up `$C852` trace contains two fetches at physical `$0DC852`,
  at shared event sequences 10,644,900 and 10,648,164. Its initial 27-byte
  windows begin `a5ba85a9a5b885a8`; the later isolated 512-byte follow-up below
  resolves the runtime code-window source to Track 02 raw offset `$A3AD2`.

### 2026-10-31 isolated 512-byte source-map follow-up

A subsequent isolated cold run used a rebuilt Mednafen PCE Fast binary and a
fresh capture profile. It used the authentic US Track 02 and System Card with a
signature-gated Drator route, including the four-second System Card RUN hold.
The route reached Drator `start` at level `$02`, position `$02,$03`, then
`left-event` at `$03,$03`, and `reentered-event` at level `$03`, position
`$02,$03`.

The bounded `$C852` trace contains 132 opcode fetches at physical `$0DC852`
(trace SHA-256 `af0e870c3405700baf34b984bb26b9ef301d06f6ecb4f1b86d28058c7357b3d4`).
The first five carry identical 512-byte windows, SHA-256
`c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351`. That
hash exactly matches the candidate at raw offset `$A3AD2` in the hash-verified
US Track 02 BIN; the other six candidates have distinct window hashes. This
establishes the source bytes for this captured runtime code window. It does not
show that `$C852` consumes the generated row or establish any row-field
meaning. The first instruction sequence is `LDA $BA; STA $A9; LDA $B8; STA $A8;
LDX $A9; LDY $A8; JSR $5039`; the complete 512-byte linear listing is linked
above. These instruction bytes do not assign semantics to the zero-page values.

For this isolated run, the capture wrapper independently verified 65,536
System Card status rows and 412,791 callback-state rows (87 reads, 412,704
writes). The System Card I/O/RAM stream reached its configured 8,388,608-row
cap. Its streaming verifier processed the 2.7 GB trace with about 18 MB RSS
and rejected the explicit truncation marker. Thus the bounded source and route
receipts and these two sidecar validations are verified, while the full
wrapper package is incomplete.

The wrapper completed, but its System Card I/O/RAM verifier rejected the file
at its explicit row cap. Treat the source-byte match, route receipts and
passing sidecar checks as verified observations; this is not a fully accepted
capture package. The raw trace and emulator profile remain on TRV2 outside Git.

### Same-session generated-row byte reads

The filtered consumer trace did retain a source-to-read join before its later
sample limit:

- `$C852` was fetched at event 10,644,900. The row fields were written next at
  physical `$1F093B=$02` (event 10,644,901), `$1F093C=$03` (10,644,902), and
  `$1F093D=$00` (10,644,910).
- At events 10,644,912–10,644,914, the trace recorded reads from those same
  physical addresses and values: `$1F093D=$00`, `$1F093C=$03`, then
  `$1F093B=$02`, with reader PC `$CBC9` / physical `$0DABC9`. The recorded
  reader window is `b9352991bc8810f8`.
- At events 10,647,071–10,647,073, reader PC `$45C3` / physical `$0D05C3`
  read `$1F093B-$1F093D` again as `$02,$03,$00`. Its captured code begins
  `73352900240a00`, a HuC6280 `TII` instruction with source `$2935`,
  destination `$2400`, and length `$000A`. This establishes that the runtime
  copies the ten-byte row into a working buffer and includes offsets 6–8 in
  the transfer.
- The consumer trace later reached its 1,048,576-row cap at event 10,647,443;
  that cap occurred after the observations above and before route reentry in
  that earlier run. The separate 512-byte follow-up later reached reentry.

These rows prove an authentic same-session write/read match for row offsets
6–8 and a later ten-byte source-to-work-buffer transfer. The separately
captured `$C852` code source is now mapped to `$A3AD2`, but these rows do not
prove that this routine reads the generated bytes. Keep byte meanings opaque;
the native creature type, HP, position-field semantics and complete gameplay
materialization remain unproven.

Raw-file hashes for the earlier filtered row-byte follow-up are recorded here
so those compact observations remain tied to the private TRV2 files. The later
512-byte source-map run ended before a complete wrapper manifest was accepted:

| File | SHA-256 |
|---|---|
| `trace.c852-fetch` | `3e3337a8c404811252077189e23f2d503fb3b104be15c3347f7e5461fc10b701` |
| `trace.pce-fast-main-ram-target-write` | `0ae940ef83cc3c41ce28816462d02e95749ea9703a84a59adb8c8e0d1c6bc4c4` |
| `trace.main-ram-consumer` | `aef897714ee8b11501276efd11ec9a26fdc30c84d78cfad6f43626627703995c` |

The run used US Track 02 SHA-256
`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`, System
Card 3.0 MD5 `ff1a674273fe3540ccef576376407d1d`, and authentic Akutuba-complete
BRAM SHA-256
`4a0ab947abeb05e8892a5892389def9c64f3122b35b5b2bfe91c7aa009785af5`. No raw
media, BRAM or full trace is checked into Git.

### Authentic Track 02 source candidates

The initial 27-byte runtime `$C852` window
`a5ba85a9a5b885a8a6a9a4a8203950a536c9fed004a537c9fff05f` was searched in
the hash-verified US Track 02 BIN. It occurs at seven raw offsets:
`$A3AD2`, `$ED2D2`, `$136AD2`, `$1802D2`, `$1C9AD2`, `$2132D2` and
`$25CAD2`. The first 128 bytes at all seven locations are identical (SHA-256
`7050572233a57965ec90006746121e56ad1d6b8ed60d545c1eaade14c6702109`), so
the initial runtime window could not distinguish them. The rebuilt 512-byte
runtime capture now resolves this: its first five fetches have SHA-256
`c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351`, exactly
matching the 512-byte candidate at raw offset `$A3AD2`. Later fetch windows vary
as execution advances, but the repeated first-five window provides an exact
source-byte join. The runtime fetch at physical `$0DC852` therefore maps to the
Track 02 code copy beginning `$A3AD2` in the authenticated US BIN.

The verified trace contains 132 `$C852` hits. The wrapper completed,
reported the cap and did not accept the capture package. The cold route reached `start` (level `$02`, position `$02,$03`), `left-event` (position `$03,$03`),
and `reentered-event` (level `$03`, position `$02,$03`). The source-byte join
is conclusive for the captured code window, but it does not establish that
`$C852` reads the generated row, identify the meaning of the bytes, or prove
native gameplay parity. Retain the 4,096-hit cap and use a filtered Drator
session for further tracing.

## Conclusion and next capture

Together the runs prove that authentic cold US media reaches the Drator
generator route, map the captured `$C852` code window to Track 02 raw offset
`$A3AD2`, show same-session row-byte writes and reads, and establish a later
copy into a work buffer. They do not prove that the `$C852` routine consumes
those bytes. Wrapper verification remains incomplete because the I/O/RAM trace
is explicitly truncated at its row cap. This is not native Firestaff gameplay
or renderer parity evidence. Keep row bytes semantically opaque.

The System Card I/O/RAM verifier now streams rows with bounded memory; it
processed the complete retained file and correctly rejected its truncation
marker. For another capture, use the existing source-locked PC-specific BIOS
decision/status traces instead of the exhaustive I/O/RAM observer, or add a
strictly verified handoff scope so repeated polling cannot consume the row
budget. Trace the `$CBC9`/`$45C3` work-buffer consumer downstream and determine
whether the mapped `$C852` routine participates in that row lifecycle. The
native viewport still requires a separate map/pose-to-VDC/VCE/VRAM-to-frame join.

See [capture readiness](../theron/THERON_CAPTURE_READINESS.md), the
[Theron TODO](../../TODO-theron.md), and the
[generated-row source lock](theron-generator-rng-state-2026-08-21.md).
