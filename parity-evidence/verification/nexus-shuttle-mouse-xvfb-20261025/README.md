# Nexus ShuttleMouse input under Xvfb (2026-10-25)

## Setup

This was a read-only input probe of the hash-verified Japanese retail Nexus
disc in Ymir 0.4.1 with `--debug`. It used the authentic slot-1 save state
(`eb77a6f7e458e4aca84a67833c027c8bc28ee5060864231dd2569aef8a804737`) in a
private copy of the Ymir profile. The profile selected a ControlPad on port 1
and a ShuttleMouse on port 2. Its ShuttleMouse profile bound the arrow keys to
relative up/right movement. The test ran on a private Xvfb display.

The cloned profile directed full-memory dumps and screenshots to private
temporary directories. No source profile, original save state, or retail game
data was modified. No memory dump is included in this evidence set.

## Probe

After reloading the same slot, the probe recorded a baseline, sent the
ShuttleMouse port's `Up` and `Right` key bindings for 0.4 seconds each, and
captured the same screen again. A separate XTEST core-pointer move was also
attempted. Ymir displayed its mouse-capture overlay, but the captures show no
visible movement of a clearly identifiable retail hand pointer or confirmed
gameplay change. They do not prove whether the configured relative input
reached the Saturn mouse port.

| Capture | SHA-256 |
| --- | --- |
| `before.png` | `d782124b72039443aeb4b412599fcaed6101d6247cc2a75eee6951368e483af9` |
| `after-shuttle-relative-arrows.png` | `0bbd379e02f515a31f5397161ad5b8be664ced3b42d5eeec6253944030a20b8e` |

This result is inconclusive about retail ShuttleMouse support. It does not
identify a retail hit region, prove a click, or justify mapping inventory,
attack, spell, or item actions. First establish a visible hand-pointer response
to relative mouse input; then capture a controlled click and its retail state
change before wiring gameplay actions.
