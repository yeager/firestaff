# Theron US cold Select and Button I capture boundary — 2026-10-31

## Result

A bounded, cold standard-PCE Mednafen run used the authentic US Track 02 CUE
and verified System Card. No save state or configured profile SRAM was loaded.
The run recorded one dispatch to logical `$E009` from logical PC `$2CC7`
(physical PC `$1F0CC7`) in the instrumented trace. The dispatch receipt is
real, but this run does not establish the dispatched routine's meaning, a
successful game-owned input path, or any gameplay action.

The capture validator returned `transition=missing`. It also rejected the
scripted-input receipt because no controller-port read exposed the requested
mask before the next scripted frame. The plan contained three events:
`run@480:300`, `select@600:1`, and `i@1200:300`. The input trace reached its
1,048,576-row cap. The validator rejection is recorded as a missing
controller-port read before the next scripted frame. The transition receipt
records zero source-bound and zero
source-unbound CD data-port reads, so the E009 dispatch is not joined to a
runtime Track 02 source read or CD-to-RAM transfer. It must not be labeled a
source-authenticated game handoff.

## Run identity

- Module: standard `pce`.
- Track 02: authentic US MODE1/2352; MD5
  `f23601102138f87c33025877767ebf76`.
- System Card: MD5 `ff1a674273fe3540ccef576376407d1d`.
- Autoload state: none; configured profile SRAM files: zero.
- Instrumented Mednafen binary MD5: `0292c01b63d20ab6ae9263830df0206e`.
- Trace prefix on TRV2: `/dev/shm/firestaff-theron-instruction-start-pc-20261031/us-cold-run-select-i-hold-20261031.trace`.
- Transition receipt SHA-256: `3fe7aca3abc964f0a9657ea8bb21ec905606f5fd9c9af33de1d8d0242b1695cf`.
- Main-RAM consumer trace SHA-256:
  `3f9ec961e00f65140bab0f633bb74cff4df67d5a0004b307ee2506c45afac53f`.
- Capture trace SHA-256: `2385d59dfca9f6a2fac885744a674d6f4845a708b30ce4d58b3c7642cefd1f62`.

The trace files were held in TRV2 `/dev/shm`; raw traces and RAM images are
not repository assets. The small receipt and hashes above preserve the result
without committing game media, saves, or generated capture data.

## Follow-up

Repair or retune the scripted-input delivery validator/capture schedule, then
repeat a cold authentic-media run. Require controller reads for each scripted
event and a same-run source-origin join before interpreting the E009 dispatch
as game-owned behavior. Continue to keep emulator instrumentation diagnostic;
native Firestaff parity requires its own real-data behavior and presentation
evidence.
