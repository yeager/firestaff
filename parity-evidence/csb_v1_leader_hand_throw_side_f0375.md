# CSB V1 leader-hand throw side (F0375/F0329)

ReDMCSB `CLIKVIEW.C` F0375 maps the left and right throw zones to side 0 and
side 1. `CHAMPION.C` F0329 forwards that side to F0328, which creates the
projectile in cell `(PartyDirection + Side) & 3` while temporarily borrowing
and then restoring the leader champion's action-hand slot.

Platform-boundary review of ReDMCSB `CLIKVIEW.C` F0375 (lines 191-279) found
that the Atari ST MEDIA009 path uses the same inclusive throw envelope as the
PC34 path after its left/right branches are combined: y 47..102; x 64..163
when `G0285_i_SquareAheadElement` is `C17_ELEMENT_DOOR_FRONT`; otherwise x
32..191. The left branch admits x 64..111 or 32..111, and the right branch
admits x 112..163 or 112..191 respectively. Thus the Atari-specific F20
conditional does not establish a different effective rectangle for Atari
ST; the current geometry is equivalent for those two source element cases.
The C080 caller's front-wall exclusion remains a separate F0377 route gate.

The exact Atari/MEDIA009 source uses runtime `G0285`, while Firestaff's
current click adapter classifies the front square from its M11 `frontCell`.
This review establishes the source rectangles and throw-side halves, not that
every supported CSB edition produces the same front-cell classification or
that the incomplete Atari renderer's clickable receipts match `G2210`.
Those remain open and are tracked in `TODO-csb.md`.

Firestaff computed the clicked side in M11 but discarded it at the CSB runtime
boundary. The runtime then inferred a side from the champion cell, so both
viewport halves could create the same projectile cell. The explicit source
side now crosses M11, the boot/runtime bridge, and the F0328-compatible
projectile creator. Ordinary action-hand throws retain their original
champion-cell inference; only F0329's leader-hand route supplies an explicit
side. Tests also pin that leader hand is cleared, action hand is restored, and
no DM1 projectile storage is touched.
