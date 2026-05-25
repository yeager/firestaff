# DM1 V1 — Touchscreen Champion Panel Input

## Source Lock
ReDMCSB WIP20210206: CLIKCHAM.C F0367; CHAMDRAW.C F0287/F0288/F0289/F0290/F0291/F0292; COMMAND.C G0455/G0452/G0453.

## Champion Panel Layout (4 panels, 2×2 grid)
The champion status area occupies y=136..199 on the right half of the screen:

```
Panel 0: x=0,   y=136, w=80, h=29
Panel 1: x=80,  y=136, w=80, h=29
Panel 2: x=0,   y=165, w=80, h=29
Panel 3: x=80,  y=165, w=80, h=29
```

Each panel contains (per CHAMDRAW.C):
- Portrait (graphic 562, loaded at C69_CHAMPION_STATUS_BOX_SPACING intervals)
- Name/title text (centered within zone)
- HP/Stamina/Mana bar graphs (25px max height each, C195+C197+C199 per champion)
- Wounds indicator (per slot box)
- Two hand slots: ready hand (16×16 at panel+4,10), action hand (panel+24,10)

## Tap — Champion Box (F0367 / COMMAND.C:375-387)
Right-clicking a champion box (C007..C010) toggles the selected champion leader:

```
CLIKCHAM.C F0367 ProcessTypes12To27_ClickInChampionStatusBox:
  C007 → C151 champion 0 toggle → G0455 C016 set-leader-0
  C008 → C152 champion 1 toggle → G0455 C017 set-leader-1
  C009 → C153 champion 2 toggle → G0455 C018 set-leader-2
  C010 → C154 champion 3 toggle → G0455 C019 set-leader-3
```

Left-clicking a champion box opens the inventory screen for that champion (COMMAND.C:419-438).

## Tap — Champion Name (COMMAND.C:484-488)
Champion name click zones (43×7 each):

| Champion | Zone | Command | Action |
|----------|------|---------|--------|
| 0 | C016/C159 | set-leader | Make champion 0 party leader |
| 1 | C017/C160 | set-leader | Make champion 1 party leader |
| 2 | C018/C161 | set-leader | Make champion 2 party leader |
| 3 | C019/C162 | set-leader | Make champion 3 party leader |

Leader change clears any existing action candidate (G0299_ui_CandidateChampionOrdinal = 0) and redraws the champion panels.

## Tap — Hand Slots (COMMAND.C:489-496, CLIKCHAM.C F0360/F0361)
Hand slot click zones (16×16 each, two per champion):

| Slot | Zone | Command | Action |
|------|------|---------|--------|
| Champion 0 Ready Hand | C020/C211 | set-ready-hand | Select ready hand of champ 0 |
| Champion 0 Action Hand | C021/C212 | set-action-hand | Select action hand of champ 0 |
| Champion 1 Ready Hand | C022/C213 | set-ready-hand | Select ready hand of champ 1 |
| Champion 1 Action Hand | C023/C214 | set-action-hand | Select action hand of champ 1 |
| Champion 2 Ready Hand | C024/C215 | set-ready-hand | Select ready hand of champ 2 |
| Champion 2 Action Hand | C025/C216 | set-action-hand | Select action hand of champ 2 |
| Champion 3 Ready Hand | C026/C217 | set-ready-hand | Select ready hand of champ 3 |
| Champion 3 Action Hand | C027/C218 | set-action-hand | Select action hand of champ 3 |

Hand selection triggers:
- CLIKCHAM.C F0360: if item in hand, pick up to mouse cursor (swap to cursor)
- CLIKCHAM.C F0361: if cursor holds item, attempt to place in hand slot
- G0299_ui_CandidateChampionOrdinal set to champion index

## Action Icons (COMMAND.C:467-471, CLIKCHAM.C F0357)
Champion action icon cells (20×35, four per row at y=86):

| Cell | Zone | Command | Meaning |
|------|------|---------|---------|
| Icon 0 | C116/C089 | show-action-0 | Champion 0 current action |
| Icon 1 | C117/C090 | show-action-1 | Champion 1 current action |
| Icon 2 | C118/C091 | show-action-2 | Champion 2 current action |
| Icon 3 | C119/C092 | show-action-3 | Champion 3 current action |

Tapping an action icon cycles through available actions for that champion (attack/cast/use/throw), updating G0286_a_ui_ChampionActionType.

## Bar Graph Interaction
HP/stamina/mana bars (25px max, drawn by F0287) are display-only — taps on bar regions are routed to the champion panel zone and do not trigger gameplay actions. Bar colors (CHAMDRAW.C:958-967):

- Below max: red (C08)
- Above max: light green (C07)
- Equal to max: lightest gray (C13)

## Champion Load Color (CHAMDRAW.C F0292:958-967)
```
load > maxLoad               → red (C08)
(load << 3) > (maxLoad * 5)  → yellow (C11)
otherwise                   → lightest gray (C13)
```

## Source Evidence
- CLIKCHAM.C F0367: right-click champion box → leader toggle
- COMMAND.C:375-387 — champion box click routing to C007..C015
- COMMAND.C:484-496 — champion name/hand slot routing
- CHAMDRAW.C F0287: bar graph height = ceil(current*25/maximum)
- CHAMDRAW.C F0291: slot box graphic selection (normal/wounded/acting)
- CLIKCHAM.C F0360/F0361: hand pickup/place with mouse cursor
- CHAMDRAW.C:958-967: load color decision tree
