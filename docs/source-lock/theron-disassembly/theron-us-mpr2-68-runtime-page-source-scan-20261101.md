# Theron US runtime MPR2 `$68` page source scan (2026-11-01)

## Evidence inputs

This analysis uses the authentic US cold Drator runtime-window trace and the
regional raw media images. The runtime sidecar SHA-256 is
`971adf4d31d5cea2d4a1edeb2d72a77f4c0fa6af3a573eca0d2784fd0725fef7`.
Authenticated US Track 02 has MD5
`f23601102138f87c33025877767ebf76`; authenticated raw US Track 19 has MD5
`fd45d13690a214b17b48a6b7c05b93b4`. Both identities were checked by the
analysis tool before searching. No game media or trace is stored in Git.

The new
[`analyze_theron_runtime_mpr_page_source.py`](../../../scripts/analyze_theron_runtime_mpr_page_source.py)
tool validates the 129-row completion receipt, each logical-to-physical PC
calculation, and the ordered `$C85E` `JSR $5039` followed by the `$5039`
dispatch under MPR2 `$68`. It collects the observed three-byte opcode window
at each logical PC in that 8 KiB MPR page and searches each possible page base
in the identity-checked source image.

## Result

The trace contains 102 unique logical PCs in the MPR2 `$68` window. At logical
`$5039`, the live three-byte sample is `44 05 86`. The static US Track 02
candidate at raw offset `$2BFDC9` begins `31 4F 20`. No contiguous 8 KiB page
base in either raw US Track 02 or raw US Track 19 matches all 102 observed
three-byte windows.

The tool was run three consecutive times on the same authentic trace and
identity-checked media; each run reported the same two zero-candidate results.
This is a bounded negative source search. It does not prove the runtime page
is absent from the game, nor rule out decompression, relocation, mixed source
segments, runtime writes, or a source elsewhere on the disc. It assigns no
semantics to `$5039` or its caller.

## Cross-capture comparison with the authentic `$23AD` output

A separate authentic US capture recorded the complete 5,064-byte `$23AD`
decoder output at physical `$0D0000–$0D13C7`, with SHA-256
`9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`. Its
source trace SHA-256 is
`a4bdc26eb82d6982490a717f873ee92fa3510818df941c67313c39592087e08d`; see the
[decoder trace boundary](theron-23ad-trace-boundary-20261029.md). The 102
unique MPR2 `$68` instruction windows from the later cold US capture all fall
inside that physical range when mapped by page offset, but none matches the
corresponding decoder-output bytes (0/102), including the `$5039` sample at
page offset `$1039`.

These are separate authentic capture sessions. This cross-capture comparison
shows that the recorded `$23AD` output does not explain the later live code
bytes; it does not prove that `$23AD` never writes a code page in another
session or identify the later page's owner.

## Next source join

The reference wrapper already has an all-writer main-RAM hook at `WrMem()`.
For the next cold US session, build the instrumented PCE Fast reference core
with the `$23AD` hook enabled, then set these wrapper variables:

```sh
THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE=0d0000-0d1fff
THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_SAMPLE_LIMIT=1048576
THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=1
THERON_CAPTURE_HUC6280_23AD_TRACE_OUTPUT=<capture-path>
```

The wrapper emits `${trace}.pce-fast-main-ram-target-write`; the `$23AD`
output destination is supplied separately. The all-writer rows include the
shared `event_sequence`, physical and logical addresses, value, writer PC and
instruction bytes, and MPRs. The 129-dispatch C852 runtime-window sidecar is
also captured by the wrapper. Check for
`main_ram_target_write_truncated=sample_limit` before treating the write stream
as complete; the configured cap is the hook's maximum, not a completeness
receipt.

The C852 runtime-window patch now emits a shared `event_sequence` under schema
v3 and includes a wrapper-generated capture ID. The all-writer trace records
the same ID, configured address range and sample limit; the wrapper receipt
binds both sidecars by SHA-256 and records row and truncation counts. The counter
advances once per emitted dispatch, before instruction execution, so writes
caused by that instruction receive later sequence values. The complete Mednafen
patch chain applied to an isolated source copy in three consecutive loops; this
is patch-application evidence only, not a compiled-binary or runtime-capture
result. A fresh cold US session must still join all-PC writes, `$23AD` output,
CD source reads and C852 instruction checkpoints by sequence, source read/LBA,
destination span and MPR mapping. The new
[`verify_theron_runtime_page_write_join.py`](../../../scripts/verify_theron_runtime_page_write_join.py)
requires matching IDs and hashes, the full page range, and an uncapped writer
trace before comparing each fetched byte with its latest preceding write. Nine
fixture cases cover success and fail-closed conditions; no authentic capture
has exercised the verifier. A match does not bind that writer to CD source bytes or
establish what the code does. The older code-RAM trace still filters
writers to `$1F1C00–$1F1DFF`, so it cannot stand in for the all-PC target trace.
Reject any target-write sidecar with its explicit truncation marker. Keep row
bytes opaque until the runtime page source is accounted for. The updated analyzer was rerun against the original v1 authentic trace and raw US Track 02: it reproduced the 129-dispatch/102-PC receipt and no-page-match result, reporting the shared sequence as absent for that historical capture. The full Drator package remains rejected because live `$45C3`
windows fail retail-source verification and other traces hit their caps; see
the [ordered call-window report](theron-us-c852-runtime-call-capture-20261101.md).
