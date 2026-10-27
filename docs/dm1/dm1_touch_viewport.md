# DM1 V1 — Touchscreen Viewport Input

## Source Lock
ReDMCSB WIP20210206: CLIKVIEW.C F0372/F0373/F0374/F0375; CLIKMENU.C F0365/F0366; COMMAND.C G0448/G0459.

## Viewport Geometry (224×136 at 0,0)
DM1 V1 renders the dungeon view as a 224-pixel wide, 136-pixel tall viewport occupying the top-left of the 320×200 screen. Click position within this rectangle determines both the target map square and the action taken.

The viewport is divided into four logical view cells:

| Cell | Position | Meaning |
|------|----------|---------|
| 0 (BACK_LEFT) | Left half, near (y≥68) | Party square, left side |
| 1 (BACK_RIGHT) | Right half, near (y≥68) | Party square, right side |
| 2 (FRONT_LEFT) | Left half, far (y<68) | Square in front of party, left side |
| 3 (FRONT_RIGHT) | Right half, far (y<68) | Square in front of party, right side |

Split lines: x=112 (vertical), y=68 (horizontal).

## View Cell → Map Coordinate
Coordinates are computed using two direction-step tables G0233/G0234 indexed by party direction (0=N, 1=E, 2=S, 3=W):

```
dirStepEast[4]  = { 0,  1,  0, -1 }
dirStepNorth[4] = { -1, 0,  1,  0 }
```

- Cells 0/1 (near/party square): target = party position
- Cells 2/3 (front square): target = party position + step offset

## Action Dispatch (F0372 primary path)
CLIKVIEW.C F0372 (ProcessType80_ClickInDungeonView_TouchFrontWallSensor) is the entry point for any viewport tap. The dispatch logic:

```
1. Classify click → view cell (0..3)
2. Compute targetMapX/Y from cell + party direction
3. No leader (G0411_i_LeaderIndex == -1):
     → wall sensor check only (F0275_IsTriggeredByClickOnWall)
     → If far cell (2/3) → trigger wall sensor, stop waiting
4. Leader present, hand empty:
     → Check G0292_aT_PileTopObject[cell] for grabbable pile
     → F0115 dungeon-view clickable box hit test (cells 0..3)
     → If pile top object exists → objectGrabbed, stop waiting
5. Leader present, hand holding object:
     → F0374: throw/place object on target square
     → F0375: attack creature in viewport
```

## Grab/Pickup Flow (F0373)
CLIKVIEW.C F0373 (grab object from floor) is reached only after:
- G0292_aT_PileTopObject[cell] returns a valid object ID
- Icon validity check passes
- F0267_MOVE_GetMoveResult_CPSCE resolves the move
- Leader hand placement succeeds

The grabbable state is built during the render pass (DUNVIEW.C:5113-5178 F0115) which creates/extends the clickable object zones and records the pile-top object per cell into G0292.

## Movement Arrows (CLIKMENU.C F0365/F0366)
Six on-screen movement buttons drawn at the bottom of the viewport area (y≥124):

| Zone | Position (x,y,w,h) | Command |
|------|--------------------|---------|
| Forward | 263,125 27×21 | C003 — step forward |
| Backward | 263,147 27×21 | C005 — step backward |
| Turn Left | 234,125 28×21 | C001 — rotate left 90° |
| Turn Right | 291,125 28×21 | C002 — rotate right 90° |
| Strafe Left | 234,147 28×21 | C006 — strafe left |
| Strafe Right | 291,147 28×21 | C004 — strafe right |

F0365 handles turn dispatch (no collision check — instant direction change).
F0366 handles step dispatch (collision check via F0267 before committing movement).

## Touch → Command Mapping
```
Physical tap (any touch surface)
  → scale to 320×200 logical coordinates
  → m11_click_hit_test() against registered zones
  → If viewport zone hit:
      → m11_viewport_resolve_click() → action (grab/pickup/throw/attack/wall-sensor)
  → If movement arrow zone hit:
      → queue command directly (F0365/F0366 dispatch)
```

## Source Evidence
- CLIKVIEW.C:117-126 — pile-top lookup, F0267 move result, hand placement
- CLIKVIEW.C:406-438 — empty-hand floor pickup requires G2210/G0291 dungeon-view clickable box hit for cells 0..3
- DUNVIEW.C:5113-5178 — F0115 builds grabbable zones and pile-top records
- CLIKMENU.C:519-585 — action-area child-click resolution after C111 dispatch
- COORD.C:1693-1722 — viewport origin and extent for coordinate mapping
- COORD.C:1915-1920 — inclusive point-in-zone bounds checking
