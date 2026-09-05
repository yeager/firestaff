# CSB V1 leader-hand throw side (F0375/F0329)

ReDMCSB `CLIKVIEW.C` F0375 maps the left and right throw zones to side 0 and
side 1. `CHAMPION.C` F0329 forwards that side to F0328, which creates the
projectile in cell `(PartyDirection + Side) & 3` while temporarily borrowing
and then restoring the leader champion's action-hand slot.

Firestaff computed the clicked side in M11 but discarded it at the CSB runtime
boundary. The runtime then inferred a side from the champion cell, so both
viewport halves could create the same projectile cell. The explicit source
side now crosses M11, the boot/runtime bridge, and the F0328-compatible
projectile creator. Ordinary action-hand throws retain their original
champion-cell inference; only F0329's leader-hand route supplies an explicit
side. Tests also pin that leader hand is cleared, action hand is restored, and
no DM1 projectile storage is touched.
