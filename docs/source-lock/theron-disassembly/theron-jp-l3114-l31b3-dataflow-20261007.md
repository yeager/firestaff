# Theron's Quest JP `$31B3` helper-chain dataflow

## Evidence boundary

This is a static dataflow reading of the authentic JP Rev. 1 Track 02
instruction windows already admitted by
[`theron-jp-stage2-l3114-huc6280.asm`](theron-jp-stage2-l3114-huc6280.asm).
That listing records the Track 02 BIN MD5, stage-two user offsets, per-window
SHA-256 values, and the `$31B3` call chain. The source-lock receipt and
mutation checks are in `tests/test_theron_v1_stage2_disassembly_chain.c` and
`src/theron/theron_v1_track02.c` (`theron_v1_track02_verify_stage2_jp_l3114_flow`).
No new media bytes were available in this checkout, so this note does not
claim a fresh original-media run.

The statements below describe register, memory, and control-flow effects
visible in the bound bytes. They do not name a game subsystem, resource type,
or gameplay action, and do not prove that the JP game executes this path.

## Bounded dataflow

- `$31B3` calls `$5C77`, `$5D0E`, `$5D32`, then `$5CA7`, in that order.
- `$5C77` copies four consecutive bytes from `$5CA2..$5CA5` to
  `$4F8B..$4F8E`, then clears `$4FD1`.
- `$5CA7` selects `$F0` in `$5CA6`; its alternate entry `$5CAE` selects
  `$EF`. Both entries converge on `$53E8`. The direct `$31B3` caller targets
  `$5CA7`; this does not show that the alternate entry is reached.
- `$53E8` appends the current two-byte value `$4FD5:$4FD6` to the byte array
  at `$4FB9 + 2 * $4FB8`, then increments `$4FB8`. It calculates
  a 16-bit value in `$0E:$0F` by adding `$4F8D` repeatedly, shifting the
  16-bit sum left once, then adding `$4FD5:$4FD6`. Values below `$DFF0` branch
  to the separately bounded `$543E` continuation; values at or above `$DFF0`
  clear the two bytes just appended. This clearing path does not undo the
  `$4FB8` increment.
- `$543E` calls `$5237`, which replaces `$0E:$0F` with the 16-bit sum
  `$4F8B + ($4F8C >> 2)`. It writes `$4F8B..$4F8E`, then `$0E:$0F`, through
  `($04),Y` at six consecutive offsets. It advances `$04/$05` by six and
  repeats the `$547D` helper with X initialized from `$4F8D`; the outer Y
  counter is initialized from `$4F8E` and decremented after each call. On
  completion it stores the advanced pointer back to `$4FD5:$4FD6` and clears
  carry.
- `$547D` calls `$5498`, which writes `$0E/$0F` to HuC6280 I/O addresses
  `$0002/$0003` after selecting VDC register `$01`. It then reads those I/O
  addresses and writes the returned pair through `($04),Y`; X counts pairs.
  The final pair is followed by a pointer advance through `$550C`.
- Returning to `$5CA7/$5CAE`, the code reads the last two-byte array entry,
  adds six, and places the result in the zero-page pointer `$04/$05`. It then
  calls `$5CEB` repeatedly, preserving X/Y around each call and decrementing
  the saved Y value between calls. `$5CEB` ORs `$5CA6` into a run of bytes
  beginning at `($04)+1`; the run length comes from X. It then calls `$550C`,
  which advances `$04/$05` by twice the A value and calls `$5251`.
- After that loop, `$5CA7/$5CAE` saves `$4FD4`, sets it to one, calls `$54B3`,
  restores `$4FD4`, and returns.
- `$54B3` decrements `$4FB8`, indexes the corresponding two-byte entry at
  `$4FB9`, and returns if that entry is zero. Otherwise it copies the entry to
  `$4FD5:$4FD6`, reads the word at offsets `+4/+5` into `$0E:$0F`, and reads
  the bytes at offsets `+2/+3` into X/Y. `$54A7` advances `$04/$05` by six.
  The loop preserves X/Y around `$4F7A` and `$54FC`; the latter writes `$0E/$0F`
  to I/O addresses `$0002/$0003` with VDC register `$00` selected, then copies
  X pairs from `($04),Y` to those I/O addresses through `$53D8`.
- `$5D0E` decrements `$4FD1` via `$5C88`; on the zero branch it toggles
  `$4FD2`, calls either `$5CA7` or `$5CAE`, and returns A=0. On the other
  branch it calls `$5D21`, calls `$E063`, then polls `$222D` until nonzero and
  stores that byte in `$08`.
- `$5D32` preserves A/X/Y, repeatedly calls `$E063` while `$2228` is nonzero,
  then restores A/X/Y and returns.

The counted loops use 8-bit decrement-and-branch sequences. A counter that
starts at zero wraps and runs 256 iterations; the source bytes do not provide
an independent bound on these runtime counter values.

## Remaining unknowns

The bytes do not establish what `$4FB9` entries point to, why `$F0/$EF` are
ORed into the pointed-to bytes, what `$E063` does, what the `$222D/$2228`
device state represents, or which conditions select the branch paths. A
runtime source-copy/load receipt and a known transition are still required to
connect these static routines to an actual JP gameplay event.
