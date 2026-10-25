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

Follow-up disassembly of the pinned retail `DM.BIN` established that all eight
hit rows enqueue event 44 with slot arguments 0 through 7, and the early main
event jump table routes event 44 to handler `0x06036e58`. The handler accesses
context and item metadata, replaces a context/slot two-word pair, and calls
update helpers. See [`docs/nexus_input.md`](../../../docs/nexus_input.md) and
[`scripts/analyze_nexus_mouse_event_crossrefs.py`](../../../scripts/analyze_nexus_mouse_event_crossrefs.py)
for the verified chain and address formulas.

The callback is now known to perform retail item/slot assignment or swap
behavior. Its pair has not yet been mapped to Firestaff's champion and
equipment fields, and a visible post-click result has not been verified.
Therefore this evidence alone still does not authorize production gameplay
clicks. The `yam\\mouse.c` string references have not been bound to this input
route.
