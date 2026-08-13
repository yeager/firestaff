# Theron's Quest V1 — Track02 consumer disassembly

## Status

The complete static consumer/decompressor chain is now checked in at
`docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm`.
It is generated from the hash-locked US retail projection, not from a
synthetic fixture:

- source: `TQUS19.iso + TQUS02End.iso`;
- MD5: `ceb02343868f80cec899e9b239aff2da`;
- extracted file offset: `$1f0000`;
- CPU: HuC6280;
- disassembler: da65 V2.18.

The same 134-byte fragment at `$1f0000+$243e` is byte-identical in the
hash-locked JP retail image `TQJP19.iso` (MD5
`f9f069a5e489b91207f3156059b756f1`, size `0x600000`). This is a regional
static-bank receipt only; it does not turn the absent post-CD `$2600` RAM
consumer into a static disassembly.

The listing includes the caller/output-size tail at `$2386–$23a3`, the
entry/framing routine at `$23ad–$2429`, and the contiguous 382-byte routine at bank address
`$23AD–$252A` (FNV-1a `3056f96c`). The verified listing contains the real
variable-bit reader at `$242A–$2458`, HuC6280 bank-window switches at
`$2459–$2482`, literal output at `$2483–$2495`, and the back-reference path at
`$2496–$252A`. US and JP bytes are identical for this range.

The receipt also covers the 30-byte caller tail at `$2386–$23A3` (FNV-1a
`699e8da1`). It saves `$30/$31` to `$3b7c/$3b7d`, calls `$23ad`, then replaces
those words with the produced-byte count. This is an authenticated output-size
contract for the helper, but it still does not identify the caller's input
block, destination bank, or level/object meaning.

## 2026-08-09 byte-level lift

The previously truncated source-lock listing now includes the complete
back-reference tail through `$252A`, including the two pointer reads, unsigned
length subtraction, low-byte copy, high-byte `TII` path, and output-pointer
advance. `theron_v1_huc6280_decode_resource()` is a bounded C lift of that
byte-level contract. It accepts the authentic six-byte resource frame, a
caller-owned flattened destination window, and a caller-owned pointer table;
it reports literal/back-reference counts and rejects truncated input, missing
pointer entries, destination overflow, and unsupported address wrapping.

This is deliberately a decompression boundary, not a semantic promotion. The
stage-2 MPR values and the post-CD `$2600` consumer still have to be captured
in the same runtime before the decoded bytes can be called a level record,
bitmap, tile atlas, object table, or palette.

## What the fragment proves

The `$2386–$23A3` caller measures the output pointer delta in
`$3b7c/$3b7d`; `$23AD–$2429` initializes the resource header, advances past
the six-byte frame, writes the destination pointer table, and widens the
variable-bit width when token `$0100` is encountered. The `$2450–$24b0`
region is real code. It performs a bounded byte/bitstream
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

## 2026-08-06 verification boundary

The checked-in receipt was rerun against both owned real regional images. The
US and JP bank-$1f bytes pass the fragment, decompressor, caller and stage-2
resource-handler hashes. The Track 02 text corpus was also rerun from the real
US BIN: it decodes into diagnostic strings, but unresolved brace/control-code
values remain present. Accordingly the world-text loader continues to reject
that block, and this disassembly still authorizes no text, menu, dungeon,
object, tile, palette or viewport semantics. A future promotion requires the
executing HuC6280 text consumer plus its source-LBA/payload join.

## 2026-08-12 — complete US dungeon-text ledger is now release-build tested

The source-admission regression now reads the operator-supplied, hash-locked
US `TQUS02.bin` directly and checks every one of the seven dungeon text
windows. It admits only their real word counts
`013c,d0,e0,e8,e0,d9,e8`, observes decoded terminator counts
`17,11,11,15,13,13,13`, and retains all three five-bit glyph values from every
source word. Every block contains unresolved control glyphs; the regression
therefore also proves that `theron_v1_world_load_dungeon_text()` publishes
zero world/UI strings for all seven, including in builds where `NDEBUG` would
otherwise erase C `assert()` checks.

The source-layout receipt agrees with DMBuilder's
`DMBUILDER6/src/loaddungeon.c` at upstream commit
`7f080387adc13a5b9ea1383c0604f97c8f78c2b2` (file SHA-256
`36fc0467c5746b44b93a70bc2be20df98b471ab3e4e38382b715ef5df31f54b2`):
its `iTQMemOffset[][5]` table and `iTQTextDataSize[]` table specify the same
seven source windows, then hand the raw words to `loadTexts()`. This is an
offset/ownership cross-check, not evidence for the original PC Engine text
renderer. The upstream author also records that text encoding/decoding alone
did not make in-game scroll text display correctly. Firestaff consequently
keeps the control-code and renderer boundary fail-closed; it does not promote
these diagnostic strings into a menu, scroll, plaque or HUD surface.
