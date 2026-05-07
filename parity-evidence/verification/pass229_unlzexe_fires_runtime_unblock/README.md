# Pass229 — DM1 PC34 FIRES post-LZEXE image evidence

Status: `PASS_DECOMPRESSED_FIRES_IMAGE_AVAILABLE`

UNLZEXE produced a decompressed FIRES image from the stock DM1 PC34 `FIRES` binary.

## Original packed FIRES

- path: `<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34/FIRES`
- size: `94779`
- sha256: `ebf84045c3edbce7690b826eadbea2e278fbb4c0a3cc19a470552586f37712eb`
- entry CS:IP: `1665:000e`
- relocation-table signature: `4c5a3931` (`4c5a3931` = LZ91)

## Decompressed FIRES.EXENEW

- local evidence path: `<firestaff-repo>-dm1v1-viewport-walls-source-lock-20260506-0237/parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW`
- size: `178224`
- sha256: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`
- entry CS:IP: `0000:0000`
- header bytes: `10240`
- relocations: `2518`

## Current meaning

This clears the "no decompressed FIRES image exists" part of the runtime blocker. It does **not** by itself prove source-symbol runtime addresses. The next blocker is symbol-map/bootstrap: derive runtime image layout and bind ReDMCSB source seams to this decompressed image, then drive a single command trace.

## Binary policy

`FIRES.EXENEW` is original-game derived binary evidence and is **not committed**. The manifest stores size, SHA256 and MZ layout only.
