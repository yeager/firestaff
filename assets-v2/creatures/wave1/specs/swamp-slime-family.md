# Swamp Slime V2 family spec

## Intent
Create a bounded V2 creature family for **Swamp Slime** that is cleaner and smoother than the original presentation without changing perceived gameplay timing.

## Current deliverables
- front-near
- front-mid
- front-far

## Scale contract
- DM1 reference role: capture during next extraction pass
- 4K masters are canonical
- 1080p exports are exact 50% derivatives
- near/mid/far should preserve the existing DM depth impression rather than invent a new distance curve

## Animation timing rule
If later passes add in-between frames, keep the original action cycle duration and perceived movement speed. Extra frames should smooth interpolation, not lengthen attacks, walks, or idle loops.

## Source policy
- first-pass generation may start from an existing project creature card reference
- upgrade to manual masks or repaint layers when heuristic extraction loses silhouette detail
- keep this family isolated from V1 runtime assets until a dedicated V2 render path is ready
