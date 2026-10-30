# Nexus BKR Continue initialization state

## Capture and inputs

An authentic Japanese retail Nexus disc and the retail-written two-champion BKR save were run through Ymir `0.4.2-dev` with `--debug`. The isolated profile used the verified BKR image unchanged; its SHA-256 was `3d60856119df55ffa4937d13b5dc7487157c7c6b131a999a2e61683316679634` before and after the run. The retail cue and IPL hashes and the loader source-read evidence are recorded in the sibling external evidence package `nexus_bkr_continue_source_read_20261031`. No synthetic game or save data was used.

The capture ran 17,500 scheduled frames and recorded SH-2 writes in `[0x0606455c,0x06064580)` for VDP1 frames `[12000,15000)`. It exited successfully and dumped final WRAM-HI. The complete write trace contains 5,244 rows through frame 14636, with zero dropped master/slave rows.

## Runtime chain

The loader had already placed the authentic tuple `00 01 40 00 00 14 00 1a` at `0x06064544`: level 1, heading `0x4000`, X 20, Y 26. The consumer read trace and SH-2 prologue establish that `FUN_06020fee` receives this address as its tuple base. Its indirect callback points to `FUN_0601f584`; the callback's captured write rows retain both `FUN_0601f584` and `FUN_06020fee` in the call context.

At frame 12931, the dynamic write trace records:

| Instruction | Destination | Value | Meaning from retail code |
|---|---:|---:|---|
| `0x0601f5a0` | `0x06064568` | `0x060746c0` | Newly allocated actor/body object |
| `0x0601f5a6` | `0x0606455c` | `0x060792b0` | Actor record base |
| `0x0601f5a8` | `0x06064560` | `0x060792b0` | Same actor record base |
| `0x0601f5b4` | `0x06064564` | `0x060746e8` | Body child at allocated object `+0x28` |
| `0x0601f5d6` | `0x0606456c` | `0xffffffff` | Retail initializer field |

The final memory dump shows body cell `0x060746e8 = 0x0014001a`, preserving the authentic X/Y pair `(20,26)`. The actor record at `0x060792b0` has heading `0x4000` at offset `+0x0e`, matching the BKR tuple. The actor/body pointers, tuple reads, callback disassembly and final state together establish that retail Continue initialization creates gameplay state from this saved position and heading.

This is initialization evidence for one authentic Continue path. It does not establish complete Firestaff parity or keyboard/mouse behavior. A separate external evidence package, `nexus_bkr_continue_heading_move_20261030`, records the subsequent Up-to-grid-cell movement result.

## Reproduce the checks

The machine-generated trace and full final WRAM-HI dump are stored on the external evidence disk at `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_bkr_continue_initializer_state_20261031/`. Validate them with:

```sh
python3 scripts/verify_nexus_bkr_initializer_state_trace.py \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus_bkr_continue_initializer_state_20261031 \
  /Volumes/Extern-disk/firestaff-evidence/nexus/nexus_bkr_continue_consumer_read_20261031
```
