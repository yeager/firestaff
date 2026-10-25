# Nexus ShuttleMouse routing test with Ymir PR #963

This test checked whether the Ymir SDL global-mouse routing fix delivered host
mouse motion to the authentic Nexus ShuttleMouse path. It did not establish a
retail pointer action.

## Setup

- Ymir source commit: `447508706ee161ba100e55adc10b958cfaa0caf7`
- Ymir Linux Debug binary SHA-256: `20ac5f1a6cdf28fef9c504d7bb170ce1b01805358cb6b493c1fa41d99e74c904`
- Retail disc: hash-verified Japanese Dungeon Master Nexus, product `T-9111G`
- JP IPL SHA-256: `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`
- Savestate SHA-256: `eb77a6f7e458e4aca84a67833c027c8bc28ee5060864231dd2569aef8a804737`
- Ymir was launched with `--debug --fast-forward` in a private profile whose
  `Dumps` path pointed to that profile's own dump directory.
- Port 1 was configured as `ShuttleMouse`. Ymir reported “Mouse bound to
  Shuttle Mouse on port 1” and entered mouse-capture mode.

## Result

After capture, XTEST absolute motion of 280×150 pixels and relative motion of
150×60 pixels left the visible retail selection and hand cursor unchanged. A
single click on an occupied lower-right HUD icon also left the screen
unchanged. The fresh post-action dump was written at 07:40 UTC and differs from
the baseline in unrelated runtime bytes, confirming that the dump refreshed.

The paired high-WRAM snapshots were analyzed with
`scripts/analyze_nexus_inventory_wram_pair.py` over context indices 0–15:

| Capture | SHA-256 | Queue count | Queue state | Item-pair changes |
| --- | --- | ---: | ---: | ---: |
| Before | `be8c8d547cf246756f5c1aa613bb06e13e2110e2d5f7a0344d82f8eaf4c0c931` | 0 | 3 | 0 |
| After | `7aa5b36e78efeee9ddf734f73667e24ae5c0333f3d5ff66b4c3ce44dc01865f4` | 0 | 3 | 0 |

All eight queue-storage records contained stale event 39 in both snapshots;
the live queue count was zero. Event 44 was absent, queue storage was
unchanged, and no item-pair changed in contexts 0–15. The cursor did not move
visibly, so this is a negative input-routing result; it does not show that the
retail event-44 handler rejected a correctly targeted click.

Raw memory dumps and the full test log are kept outside Git at
`/Volumes/Extern-disk/ymir-event44-pr963-a2-before.bin`,
`/Volumes/Extern-disk/ymir-event44-pr963-a2-after.bin`, and
`/Volumes/Extern-disk/ymir-event44-pr963-a2-*.png`. The screenshots committed
here are the source-state evidence; no BIOS, disc image, savestate, or WRAM dump
is included.

## Screenshots

| Capture | SHA-256 |
| --- | --- |
| `inventory-baseline.png` | `78345348b95547b7ee10d4b3f679e6b81cc58c73bcf21217dc0d609923de93c1` |
| `capture-mode.png` | `a084599a29f836af5a258517893ac6d15804be2d43ad1fc13fd1325def1b4611` |
| `absolute-motion.png` | `4271b4b098c8de7d3ac498c359ac9c208659c3c663d2f478d37006c751ad51ab` |
| `relative-motion.png` | `32d303d46cde285ee5776ae12057dd239197d2a8cb47cb25bc115aced527d5b8` |
| `after-click.png` | `6b6baf96a4c5ce0e81c44d89956f3600b213293af9f30c30d0e84fb7479c127b` |
