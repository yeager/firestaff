# Theron authentic pointer interval-table candidates

## Authentic media inspection

A read-only inspection used the authentic US Track 02 BIN `TQUS02.bin`,
8,104,992 bytes, MD5 `f23601102138f87c33025877767ebf76`. The file remains local
and is not included in the repository.

The existing US source lock identifies seven static `$D515` callee candidates
at raw BIN offsets `0x9da93`, `0xe7293`, `0x130a93`, `0x17a293`, `0x1c3a93`,
`0x20d293`, and `0x256a93`. At offset `+0x92` from each candidate, all seven
contain the same 26-byte interval-table candidate. Its SHA-256 is
`8f9eea35bef73f48d39f09255524608a39188524935170168a44659b1b74b800`. The raw
bytes are intentionally omitted.

## Disassembly boundary

The existing `$D515..$D5A6` disassembly in
[`theron-jp-runtime-input-poll-bank-alias-20261009.md`](theron-disassembly/theron-jp-runtime-input-poll-bank-alias-20261009.md) shows the caller loading a descriptor pointer from runtime `$77DC/$77DD`, then calling `$D588` with `X=$0C`. `$D588` compares `$2E7A` with paired values at `$D5A8,X`, decreasing `X` by two, and returns the paired byte from `$D5A7,X`. The recovered static bytes confirm that the loop consumes threshold/result pairs.

This establishes a coordinate-dependent classification routine in the candidate code. It does not establish that a particular candidate is the loaded runtime copy, that `$2E7A` is a desktop pointer coordinate, or that a returned value is a gameplay command or screen hit region. The descriptor pointer and its source lineage are unresolved, as are the coordinate ranges and resulting state effects.

## Required runtime join

Before implementing native world-pointer routing, recover an authenticated runtime receipt that binds, in one session:

1. the live code window and the `$D515` caller path;
2. the runtime `$77DC/$77DD` pointer value and the exact descriptor bytes it addresses;
3. the cursor coordinate consumed by `$D588` and the selected threshold/result pair;
4. the resulting hover or click command and its source handler; and
5. the resulting state change, or an explicit source-backed no-op.

Until that join exists, retain the native host control panel and fail closed on
unmapped world clicks. Do not infer rectangle edges from six movement command
points or borrow DM1's mouse table.
