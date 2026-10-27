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

Follow-up disassembly of the pinned retail `DM.BIN` verifies that all eight
hit rows enqueue event 44 with slot arguments 0 through 7, and that the early
main event jump table points the event-44 case at code `0x06036e58`. The checker
also verifies selected instruction words and address literals in nearby
handler/helper regions. It does not verify a control-flow chain from the event
case through those helpers or establish any memory writes, callbacks, or
item/slot assignment effect. The retail state transition, its mapping to
Firestaff's champion and equipment fields, and a visible post-click result
remain unverified. This evidence therefore does not authorize production
gameplay clicks. See [`docs/nexus/nexus_input.md`](../../../docs/nexus/nexus_input.md) and
[`scripts/analyze_nexus_mouse_event_crossrefs.py`](../../../scripts/analyze_nexus_mouse_event_crossrefs.py)
for the verified dispatch and the specific checked values. The
`yam\\mouse.c` string references have not been bound to this input route.
