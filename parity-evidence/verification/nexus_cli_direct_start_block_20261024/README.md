# Nexus direct CLI startup blocker — 2026-10-24

## Scope

This check isolates the remaining Saturn Nexus launch gap using the locally
installed, hash-verified Japanese retail `DM.BIN`. It checks direct CLI startup
through the end of the title sequence and sends one Return input after the
startup frame advance. It does not claim gameplay or visual parity.

## Evidence

- Firestaff revision: `312664bd0` (`feat(nexus): add verified keyboard turn aliases (#26)`).
- Firestaff build: `v3.0.410`, Debug, macOS arm64, Apple M4.
- Selected edition: `nexus-saturn-jp`.
- Selected retail asset: `DM.BIN`, MD5 `e88d60859f65f08fa622e1992b02280f`.
- The test was run with the local original-data directory passed explicitly;
  the absolute user path is intentionally omitted from this tracked evidence.

Direct launch command, with local paths abbreviated:

```sh
firestaff --game nexus --platform saturn --data-dir <original-data>/DM.BIN \
  --boot-probe --boot-probe-frames 110 --script enter --debug --duration 0
```

The resulting launch log and boot probe report:

```text
firestaff: launch phase=game-handoff mode=direct game=nexus platform=Saturn edition=nexus-saturn-jp source=<original-data>/DM.BIN
NEXUS STARTUP RECEIPT: status=blocked gameId=nexus title=1 levelLoaded=0 menu=0 blocker=menu-bpk-prs3-capture-required
FIRESTAFF BOOT PROBE READY: ... phase=nexus-title ... frames=110 inputs=1 ... titleReady=1 levelLoaded=0 ...
```

The authentic title reaches its ready state, and the probe reports one scripted
input. The game still has no loaded level or menu because the production route
rejects the unresolved Saturn `MENU.BPK` PRS3 consumer. Do not
turn off that gate or label this route playable: the retail menu's input/action
contract and its connection to the actual menu consumer are not yet established.

The existing authentic-media menu test also passes while asserting the same
capture gate; it verifies the current behavior, not successful game start:

```sh
FIRESTAFF_NEXUS_DATA_DIR=<original-data> \
  bash tests/test_nexus_v1_native_menu_mouse_title.sh \
  <build>/firestaff <build>
```

## Next evidence required

Use a same-session retail Saturn capture or source-level trace to bind title
input, the New Game/Continue menu transition, and the real menu asset consumer.
Only then can the startup action and downstream runtime be enabled without
inventing menu behavior or falling back to placeholder graphics.
