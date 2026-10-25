# Nexus Keyboard and Mouse Retail Input Capture

This evidence set preserves a Japanese retail Nexus gameplay screen, a Ymir ShuttleMouse port configuration, and a click-attempt screenshot. The investigation also examined two 1 MiB SH-2 WRAM snapshots; their hashes and relevant decoded values are recorded below, while the raw memory dumps are omitted from Git. The game data is authentic; no synthetic game state was used.

The DM.BIN used by the investigation is the hash-pinned Japanese retail image (SHA-256 `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`). The screenshot `nexus-port1-mouse-selected.png` shows ShuttleMouse on port 1. The gameplay screenshot shows the four-champion HUD.

## Findings and limits

The queue record area at WRAM offset `0x0797b0` contains eight 12-byte records. In the pre-attempt snapshot, the event fields include `0x03` and `0x0c`; in the post-input snapshot, multiple records contain `0x27`. The retail event-name table resolves `0x27` (39) to `EV_RING_SELECT`. The queue count at `0x079814` remains zero in both snapshots, and the queue-state word at `0x079810` remains `3`. The ten inventory item-pair regions at `0x07fcb0 + context*0x13c` show no changes attributable to an item transfer.

These snapshots are not a controlled click/no-click pair: elapsed emulation time and other inputs were not matched. The event 39 values may be stale queue records or may reflect earlier input; they do not prove that the mouse click selected a ring. The screenshots also do not show a confirmed game action. Do not bind attack, inventory, spell, or item-use clicks from this evidence.

An isolated follow-up used a clone of the Ymir profile and the authentic slot-1 save from 2026-10-25 01:38:13. It ran a two-second no-click control and a two-second click attempt after loading that same slot, then paused and dumped WRAM. ShuttleMouse was selected on port 1 in the cloned profile. The click used host display position `(974, 435)`, chosen from an assumed viewport transform for retail hit rectangle 0. The transform was not independently calibrated, so the click attempt does not prove that the Saturn pointer landed inside the authentic hit rectangle.

In both dumps, the eight event-queue records at `0x0797b0` are byte-identical: they retain `0x27` (`EV_RING_SELECT`) in the event fields, the queue state at `0x079810` remains `3`, and the queue count at `0x079814` remains zero. The ten champion-context item-pair regions are also byte-identical between the control and click attempt. The full 1 MiB WRAM images differ in 844 bytes elsewhere; these changes are not assigned to the click. The screenshots show no confirmed game-state change.

This result narrows the next step: calibrate the emulator's captured mouse coordinates against the Saturn display and confirm that a click is delivered to a known retail hit region before interpreting any event or item-state deltas. Keep attack, inventory, spell, and item-use clicks disabled until that is proven.

## Examined memory snapshots

- `ram-before-click.bin` — SHA-256 `26fe4c8aa0fd4f3c0eef809fa484a73f221a80db00700437285f2e4c2b40982e`
- `ram-after-port1-input.bin` — SHA-256 `941f035e8b30bd34d3e6abcbad35e82702fb14cd7c2418556d67ca4d53b6a7e0`

The examined queue bytes at `0x0797b0` begin `00 00 00 03` before and `00 00 00 27` after. At `0x0797ec`, the event field changes from `0x0c` to `0x27`. Multiple ring entries contain `0x27` after the attempt. These raw dumps are excluded because they contain the complete Saturn WRAM image; the hashes and bounded event-field summary retain the relevant evidence without committing the full memory payload.

The controlled follow-up WRAM snapshot hashes are `8c31af5cb9cb5a500013c7ffc88987c8d9746053c1ae1aeae248e60cd295ad8c` (no-click control) and `63f18ffa887f9731cd8664400a2de118da48112e654a2e8c6e510784537e8db3` (click attempt). These 1 MiB dumps are also omitted from Git; the event-queue and item-pair comparison above records the relevant result.

## Files

- `nexus-after-start.png` — `294815` bytes, SHA-256 `f3e660b56ccc9bbdfd5f691c8cd86c6259fbb046876650c6835d9a87589dc3b1`
- `nexus-port1-mouse-selected.png` — `312005` bytes, SHA-256 `9f01e72a8fb5daf5fe2de811323b1c8690154e406de5529ba88bae03a8f44f27`
- `nexus-after-target-click.png` — `304365` bytes, SHA-256 `25198b85e3f8e842b69e0b4333e99c9ce66f138ed63d6c1975a2be117d1c71c2`
- `retail-no-click-control.png` — `293157` bytes, SHA-256 `9af20f3ed46a48287b589c984fbfef30d8870f9933ef7f808c2d2d72ed8d2360`
- `retail-click-attempt.png` — `305505` bytes, SHA-256 `5d776aae663849bf5bd35f63d5f816b02626c28e82bb9743e1440b4dab50011b`
- `ram-before-click.bin` — `1048576` bytes, SHA-256 `26fe4c8aa0fd4f3c0eef809fa484a73f221a80db00700437285f2e4c2b40982e`
- `ram-after-port1-input.bin` — `1048576` bytes, SHA-256 `941f035e8b30bd34d3e6abcbad35e82702fb14cd7c2418556d67ca4d53b6a7e0`
