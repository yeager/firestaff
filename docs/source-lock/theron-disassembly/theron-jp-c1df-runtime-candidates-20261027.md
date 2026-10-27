# Theron JP `$C1DF` runtime code candidates

## Authentic evidence

The source is Japanese Rev. 1 Track 02, SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`.
An isolated Mednafen PCE Fast capture loaded an emulator-created in-dungeon
save state. The state load bypasses CD reads, so the trace has no source-LBA
receipt. Its trace and the save state remain on TRV2; neither is stored here.
The instrumented Mednafen binary SHA-256 is
`8dc5faf9b89dfc917e751ec4403072c2a8bd6a27e4d4ba880f27bab3dadd7097`; the
mouse trace SHA-256 is
`3c42994bfffb2c8d3a0335a9befe146d62561a0b9a0cd8f657f4e7497c8e6126`.

The corrected instruction hook recorded three calls to `$C1DF`, at `$B2B1`,
`$B37A`, and `$B437`. In the first bounded window, runtime PC `$C1DF` mapped to
physical `$0D21DF` with MPR5 `$69`; the logged opcode bytes were `20 81 59`
(`JSR $5981`), followed at `$C1E2` by `20 9E C3` (`JSR $C39E`). Across the
capture, 155 instruction records at 77 unique PCs in `$C1DF-$C2FF` matched all
three logged bytes (opcode and two following bytes) against each of six
identical 256-byte Track 02 windows:

```text
0x09723f  0x0e0a3f  0x12a23f
0x173a3f  0x1bd23f  0x206a3f
```

Each window has SHA-256
`263af8e1a7108155c01d5487b69dfc23f9eec7a7299ccd06b9d99bca2036c186`; a raw
Track 02 scan found exactly these six copies. The host regression
`test_theron_v1_huc6280_disassembly` checks the 256-byte window at each offset,
its FNV-1a-64 value, and the exact match count.

## Disassembly boundary

The candidate begins:

```text
$C1DF  JSR $5981
$C1E2  JSR $C39E
$C1E5  BEQ $C1F7
```

Further bytes include conditional branches, a `TII` from `$2090` to `$2897`,
and code that reads through zero-page pointer `$01` and updates `$27xx` and
`$A0-$A3`. This records visible instructions only; their high-level purpose
and relation to pointer input are not established.

The repeated save-state capture still had no CD IRQs, no authenticated
CD-to-RAM receipt, no command RAM writes, and `transition=missing`. Matching
runtime instructions to repeated retail bytes narrows the code candidate but
does not identify which copy was loaded. It also does not bind a mouse event,
rectangle record, dispatched game command, or resulting party/world state.
Keep native world clicks open until those same-session joins are obtained.
