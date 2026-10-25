# Theron authentic pointer interval-table candidates

## Authentic media inspection

Read-only inspections used the authentic US Track 02 BIN `TQUS02.bin`
(8,104,992 bytes, MD5 `f23601102138f87c33025877767ebf76`) and JP Track 02 BIN
`TQJP02.bin` (8,102,640 bytes, MD5 `b7afb338ad31be1025b53f9aff12d73a`). Both
files remain local and are not included in the repository.

The existing source lock identifies seven static `$D515` callee candidates
in each region. The US raw BIN offsets are `0x9da93`, `0xe7293`, `0x130a93`,
`0x17a293`, `0x1c3a93`, `0x20d293`, and `0x256a93`; the JP offsets are
`0x9d155`, `0xe6955`, `0x130155`, `0x179955`, `0x1c3155`, `0x20c955`, and
`0x256155`. At offset `+0x92` from each candidate, all fourteen windows
contain the same 26-byte interval-table candidate. Its SHA-256 is
`8f9eea35bef73f48d39f09255524608a39188524935170168a44659b1b74b800`. The raw
bytes are intentionally omitted.

## Disassembly boundary

The existing `$D515..$D5A6` disassembly in
[`theron-jp-runtime-input-poll-bank-alias-20261009.md`](theron-disassembly/theron-jp-runtime-input-poll-bank-alias-20261009.md) shows the caller loading a descriptor pointer from runtime `$77DC/$77DD`, then calling `$D588` with `X=$0C`. `$D588` compares `$2E7A` with paired values at `$D5A8,X`, decreasing `X` by two, and returns the paired byte from `$D5A7,X`. The recovered static bytes confirm that the loop consumes threshold/result pairs.

This establishes a coordinate-dependent classification routine in the candidate code. It does not establish that a particular candidate is the loaded runtime copy, that `$2E7A` is a desktop pointer coordinate, or that a returned value is a gameplay command or screen hit region. The descriptor pointer and its source lineage are unresolved, as are the coordinate ranges and resulting state effects.

## Saved-state pointer-source mismatch

A read-only decode of the preserved post-Akutuba Mednafen state (MD5
`f17f377df210b4a3ae904a13fb85a7f0`) used the state layout from Mednafen
`state.cpp::MDFNSS_LoadSM`/`MakeSectionMap`, `pce/huc.cpp::HuC_StateAction`,
and `pce/huc6280.cpp::StateAction`. The CPU MPR values route logical `$77DC`
through MPR3 `$74` and logical `$C060` through MPR6 `$6D` into SysCardRAM.
Only hashes and match counts are retained here; no save-state or game bytes
are included.

- The 64 bytes at logical `$77DC` have SHA-256
  `0a10d28997dcc359c4c3b08b7e6f92b4c71d53587679dd9dcd65bb737c62e757`. They
  match JP Track 02 once at raw offset `0xB2038`, and have no full 64-byte
  match in US Track 02. Their first 32 bytes match US once at `0xB280C`.
- The 64 bytes at logical `$C060` have SHA-256
  `2659419c43b6d9c7cb3506a2bc06116d4b845b77df79d44b1c1e0b56c4904cf3`. They
  match US Track 02 seven times, at offsets `0xA0CF0`, `0xEA4F0`, `0x133CF0`,
  `0x17D4F0`, `0x1C6CF0`, `0x2104F0`, and `0x259CF0`; they do not match JP.
- `$C060+5` is zero in the saved state. The `$D544` zero check therefore takes
  the `$D587` return before the descriptor-row loop. This snapshot does not
  contain a nonempty descriptor lookup for hit testing.

This is not a clean same-edition source join. Existing input-sweep evidence
also does not bind the saved state's `$77DC`/`$C060` bytes to a live `$D515`
lookup. The state therefore does not establish that this pointer table or
descriptor was loaded from one claimed edition or consumed by the live game.
The next capture must use the instrumented PCE reference emulator with the
authentic original media; Firestaff itself continues to run natively. Admit
the exact source edition, record live MPR and mapped `$77DC`
and target bytes, then bind a nonzero descriptor through `$D515` and `$D588`
to `$2911` hover and `$CCDB` Button-I command output. Only after that can the
pointer data drive native hit testing.

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
