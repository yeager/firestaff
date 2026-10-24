# Nexus retail mouse callback trace

## Inputs

- Retail data: `.firestaff/data/nexus/DM.BIN`, read from the configured local data directory.
- SHA-256: `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.
- Emulator: Ymir 0.4.1 on TRV2 was used for retail startup and input-path investigation. No gameplay mouse action was admitted from the incomplete emulator attempt.

## Finding

The SH-2 code at `DM.BIN:0x16588` loads the authentic hit-region subtable at runtime address `0x06048040`, companion data at `0x0604850c`, and callback code at `0x060264a8`, then calls setup routine `0x06034ad0`. The setup stores the supplied callback pointer in a global slot. A dispatcher checks an active flag and invokes the stored callback indirectly.

The report is reproducible with:

```sh
python3 scripts/analyze_nexus_mouse_event_crossrefs.py
```

## Limits

This trace establishes retail hit-region registration and callback transport. It does not identify the callback's complete state transitions, bind individual hit rectangles to actions, or prove Firestaff mouse parity. The `yam\\mouse.c` string references have not been bound to this input route. Do not enable Nexus gameplay clicks from these findings alone.
