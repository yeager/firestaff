# Pass 173 route precondition C127 scan

DUNGEON.DAT helper scan (compiled transiently on N2) found the initial DM1 V1 party pose and C127 wall portrait sensors before the Xvfb runtime probe.

Key facts:

- Initial party location: map0 x=1 y=3 dir=South (`0x0861`).
- Front wall square from that pose: map0 x=1 y=4.
- That square contains sensor 16, type `C127_SENSOR_WALL_CHAMPION_PORTRAIT` (`thing=0x0c10`, `sensorData=10`).
- Therefore pass 4 required no movement route; the blocker in pass166/pass3 was entrance gating, not routing to a different tile.

Relevant helper output:

```text
initial map=0 x=1 y=3 dir=S raw=0x0861
C127 sensor=16 thing=0x0c10 map=0 level=0 x=1 y=4 elem=Wall data=10 once=0 eff=0 val=1 local=0 orn=10 target=(7,14 cell0)
ROUTE candidate sensor=16 wall=(1,4) pose=(1,3,S) steps=0 path_rev= path=
```
