# Theron stage-2 System Card callback dispatch

This note follows the `$E009` and `$E00F` calls in the static JP/US stage-2
candidate into an authentic System Card 3.0 image. It establishes their BIOS
vector destinations and the callback control flow visible in the BIOS code;
it does not prove that the static Track 02 candidate supplied the runtime
bytes or that the calls completed in a game session.

## System Card source lock

The local `syscard3.pce` image has size 262,656 bytes, MD5
`ff1a674273fe3540ccef576376407d1d` and SHA-256
`df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1`.
Mednafen 1.32.1 `src/pce/huc.cpp::HuC_Load` (lines 281-287) skips the 512-byte
copier header for this image; logical `$E000` therefore maps to file offset
`0x200`. The optional real-media regression
`tests/test_theron_v1_syscard_stage2_dispatch_source_lock.py` verifies the
complete image identity, vector bytes and entry-code windows. It skips when
the user's local System Card is unavailable. The BIOS itself is not included
in this repository and this test adds no runtime BIOS dependency.

The vector at logical `$E009` (file offset `0x209`) is `JMP $EC05`. The vector
at `$E00F` (file offset `0x20f`) is `JMP $EBEC`. `da65` disassembly of those
hash-locked entries shows that `$E009` enters the shared core at `$EC05`; its
`$FF=1` path copies `$F8` to BIOS RAM `$2280`, shifts `$F8` left three times
into `$F9` and clears `$F8`. Thus the two caller tuples in the
[`stage-2 parameter-block note`](theron-jp-us-stage2-parameter-blocks-20261010.md)
produce `$F9=$88` for `$F8=$11` and `$F9=$10` for `$F8=$02`. These are observed
register transformations, not names or meanings for the fields.

## `$E00F` callback path

The `$EBEC` wrapper loads `$FA/$FB` into `$2282/$2283`, calls the same core at
`$EC05`, then tests its result. On zero, the wrapper sets the stack pointer
from `$X=$FF` and jumps indirectly through `$2282`; on nonzero it jumps to
`$E0F3`. It is therefore a callback/trampoline path rather than an ordinary
return on the zero-status branch.

The static stage-2 routine at `$4080` sets `$FA=0`, `$FB=$40` before calling
`$E00F`, making the callback address `$4000`. The authentic JP and US Track
02 images contain the same two bytes `64 00` there (`STZ $00`), immediately
before the byte-locked candidate beginning at `$4002`. The real-media
regression now verifies that prefix. This aligns the callback address with
the candidate's adjacent source bytes, but it is still static correspondence:
no captured BIOS callback or source-to-runtime receipt establishes that these
bytes were loaded or executed.

## Evidence boundary

The `$E009/$E00F` vector mapping, shared `$EC05` core, conditional callback
dispatch and authentic `$4000` prefix are now source-locked. The broader BIOS
operation, nonzero-result path at `$E0F3`, source transfer into runtime
memory, and any resulting game transition remain unproven. No BIOS or game
image bytes are copied into the repository.
