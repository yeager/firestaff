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

## Next source join

Capture the code page's actual population path in the same cold US run: source
read/LBA and bytes, any System Card copy or decode, destination physical span,
and subsequent instruction fetches. The source-window join must account for
the runtime bytes before assigning the static stage-two candidate or inferring
row semantics. The full Drator capture package remains rejected because live
`$45C3` windows fail retail-source verification and other traces hit their
caps; see the [ordered call-window report](theron-us-c852-runtime-call-capture-20261101.md).
