# Theron's Quest V1 — Track02 consumer disassembly

## Status

The first static consumer fragment is now checked in at
`docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`.
It is generated from the hash-locked US retail projection, not from a
synthetic fixture:

- source: `TQUS19.iso + TQUS02End.iso`;
- MD5: `ceb02343868f80cec899e9b239aff2da`;
- extracted file offset: `$1f0000`;
- CPU: HuC6280;
- disassembler: da65 V2.18.

## What the fragment proves

The `$2450–$24b0` region is real code. It performs a bounded byte/bitstream
step and uses `$2459`/`$246e` to load HuC6280 memory-mapping registers from the
table at `$3b7e–$3b85`. The forward path writes through `($30)`; the reverse
path computes an indexed source address through `$32/$33` and reads through
`($36)`. That is useful loader provenance, but it is not by itself a level,
object, tile, bitmap, or palette classification.

The live capture's later `$2600` consumer remains a separate RAM-loaded
stage. The static retail bank contains no byte-backed listing for that code,
so the disassembly deliberately does not manufacture one. The next capture
requirement is the RAM window after the authenticated CD transfer, including
the bytes around `$2400–$2800`, the executing PC, and the source LBA/span.

## Relationship to the existing handoff

The existing IPL/stage-2 listings remain the source for the `$4090` loader
transfer and `$3800/$4000` stage-two entry. This fragment extends the static
listing into the bank-switch/byte-consumption helper that surrounds the live
handoff. It does not promote the current opaque startup envelope or any
object/later-level record into runtime use.
