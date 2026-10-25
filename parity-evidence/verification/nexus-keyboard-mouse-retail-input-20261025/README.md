# Nexus Keyboard and Mouse Retail Input Capture

This evidence set preserves a Japanese retail Nexus gameplay screen, a Ymir ShuttleMouse port configuration, and a click-attempt screenshot. The investigation also examined two 1 MiB SH-2 WRAM snapshots; their hashes and relevant decoded values are recorded below, while the raw memory dumps are omitted from Git. The game data is authentic; no synthetic game state was used.

The DM.BIN used by the investigation is the hash-pinned Japanese retail image (SHA-256 `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`). The screenshot `nexus-port1-mouse-selected.png` shows ShuttleMouse on port 1. The gameplay screenshot shows the four-champion HUD.

## Findings and limits

The queue record area at WRAM offset `0x0797b0` contains eight 12-byte records. In the pre-attempt snapshot, the event fields include `0x03` and `0x0c`; in the post-input snapshot, multiple records contain `0x27`. The retail event-name table resolves `0x27` (39) to `EV_RING_SELECT`. The queue count at `0x079814` remains zero in both snapshots, and the queue-state word at `0x079810` remains `3`. The ten inventory item-pair regions at `0x07fcb0 + context*0x13c` show no changes attributable to an item transfer.

These snapshots are not a controlled click/no-click pair: elapsed emulation time and other inputs were not matched. The event 39 values may be stale queue records or may reflect earlier input; they do not prove that the mouse click selected a ring. The screenshots also do not show a confirmed game action. Do not bind attack, inventory, spell, or item-use clicks from this evidence.

The next useful capture is a same-session, same-duration no-input control and one click at a known authentic hit region, with event-queue and champion/item state captured immediately before and after.

## Examined memory snapshots

- `ram-before-click.bin` — SHA-256 `26fe4c8aa0fd4f3c0eef809fa484a73f221a80db00700437285f2e4c2b40982e`
- `ram-after-port1-input.bin` — SHA-256 `941f035e8b30bd34d3e6abcbad35e82702fb14cd7c2418556d67ca4d53b6a7e0`

The examined queue bytes at `0x0797b0` begin `00 00 00 03` before and `00 00 00 27` after. At `0x0797ec`, the event field changes from `0x0c` to `0x27`. Multiple ring entries contain `0x27` after the attempt. These raw dumps are excluded because they contain the complete Saturn WRAM image; the hashes and bounded event-field summary retain the relevant evidence without committing the full memory payload.

## Files

- `nexus-after-start.png` — `294815` bytes, SHA-256 `f3e660b56ccc9bbdfd5f691c8cd86c6259fbb046876650c6835d9a87589dc3b1`
- `nexus-port1-mouse-selected.png` — `312005` bytes, SHA-256 `9f01e72a8fb5daf5fe2de811323b1c8690154e406de5529ba88bae03a8f44f27`
- `nexus-after-target-click.png` — `304365` bytes, SHA-256 `25198b85e3f8e842b69e0b4333e99c9ce66f138ed63d6c1975a2be117d1c71c2`
- `ram-before-click.bin` — `1048576` bytes, SHA-256 `26fe4c8aa0fd4f3c0eef809fa484a73f221a80db00700437285f2e4c2b40982e`
- `ram-after-port1-input.bin` — `1048576` bytes, SHA-256 `941f035e8b30bd34d3e6abcbad35e82702fb14cd7c2418556d67ca4d53b6a7e0`
