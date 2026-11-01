# Theron US `$C852` runtime call-window capture (2026-11-01)

## Scope and media identity

This report records a reference-emulator disassembly observation from a cold
US Drator route. It is not a Firestaff runtime trace or a complete accepted
capture package. The mounted CUE was assembled outside the repository from
the authentic US archive. The US raw Track 02 BIN has MD5
`f23601102138f87c33025877767ebf76`; the archive passed `unrar t` with MD5
`ac34e0f1482416e9728255dcb25d8234`. Its 17 OGG members are archived CD-DA
recordings decoded by Mednafen's Vorbis support; audio bit-exactness is not
claimed. No media, trace, state, or emulator binary is checked into Git.

## Ordered runtime observation

The instrumented Mednafen PCE Fast run recorded a complete 129-dispatch
`c852-runtime-window` sidecar (SHA-256
`971adf4d31d5cea2d4a1edeb2d72a77f4c0fa6af3a573eca0d2784fd0725fef7`).
Its first five 512-byte `$C852` fetch windows have SHA-256
`c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351`,
matching the authenticated raw Track 02 bytes at offset `$A3AD2`. At step 6,
logical `$C85E` (physical `$0DC85E`) dispatches `20 39 50`, a `JSR $5039`.
The immediately following step 7 dispatches at logical `$5039`, physical
`$0D1039`, with bytes `44 05 86`; MPR2 is `$68` for both records. The sidecar
ends with `rows=129 status=complete`.

This proves that this captured `$C852` path executed the call and then
dispatched at its target address. It does not establish the target's source
owner or semantics. The previously identified static US stage-two candidate
at raw offset `$2BFDC9` begins `31 4f 20 79 38 64 5b 60`, which differs from
the live bytes at `$5039`. Therefore the static target candidate cannot be
assigned to this runtime mapping. See the earlier
[static candidate report](theron-us-stage2-c852-call-target-5039-20261101.md).

The sidecar's eight-byte records are instruction samples; later operands in
each record are not a complete fetched-code window. Only the separate first
five `$C852` 512-byte fetch receipts are source-matched. Do not infer routine
semantics from the 129 dispatches.

## Capture rejection and remaining boundaries

The cold route diagnostics recorded Drator `start` at level `$02`, position
`$02,$03`, a `left-event`, and reentry at level `$03`, position `$02,$03`.
However, the strict wrapper rejected the capture: live `$45C3` instruction
windows did not match the authenticated Track 02 BIN, so the reader-source
verifier produced no accepted report. The System Card status trace hit its
65,536-row cap, and the main-RAM consumer observer hit its 1,048,576-row cap.
Other sidecars also have their own bounds. A route receipt and complete
instruction-window sidecar do not override those failures.

The `$C852` code source match and ordered `$C85E` call/`$5039` dispatch are
useful bounded disassembly evidence. The live `$45C3` source occurrence,
runtime target source mapping, row semantics, complete untruncated cold
handoff, source-to-render join, and native gameplay parity remain open. The
reference-emulator result makes no claim about Firestaff behavior.

The private trace base is named `us-drator-raw-cue-20261101`; its
`c852-fetch` sidecar SHA-256 is
`5d775f0297989365e8078cf4680f73a74178191943738afed44cd45a22948cd8`. Trace
files and local absolute paths are intentionally excluded from this tracked
report.
