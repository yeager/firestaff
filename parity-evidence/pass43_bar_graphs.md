# Pass 43 — Champion HP/stamina/mana bar graphs (V1 original-faithful)

Last updated: 2026-04-24
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode** — retire
`V1_BLOCKERS.md` §7 by replacing the invented V1 party-status strip with
source-faithful `CHAMDRAW.C` bar graphs.

---

## 1. What pass 43 lands

Pass 43 replaces the V1 party status box’s legacy bottom strip
(`59×2 + 59×1 + 59×1` horizontal fill bands) with the original DM1-style
vertical champion bars:

- **3 bars per champion**: HP / stamina / mana
- **container size**: `4×25` each
- **slot-local x origins**: `46`, `53`, `60`
- **slot-local top y**: `4`
- **blank color**: `C12_COLOR_DARKEST_GRAY`
- **fill color**: `G0046_auc_Graphic562_ChampionColor[championIndex]`
- **fill direction**: from the bottom upward
- **minimum visible fill**: 1 px when `current > 0`

This matches `CHAMDRAW.C:F0287_CHAMPION_DrawBarGraphs` and the recovered
`ZONES.H` placement used for the champion status box.

---

## 2. Source anchors used

### 2.1 Geometry

From the recovered zone layout already present in-tree:

- status-box frame: `67×29` (`C007_GRAPHIC_STATUS_BOX`)
- bar-graph region: local `x=43..66`, `y=0..28`
- bar containers: `4×25`
- center anchors: `d1 = 5 / 12 / 19`

That resolves to these slot-local top-left origins:

| Bar | x | y | w | h |
|---|---:|---:|---:|---:|
| HP | 46 | 4 | 4 | 25 |
| Stamina | 53 | 4 | 4 | 25 |
| Mana | 60 | 4 | 4 | 25 |

For slot 0 at panel origin `(12,160)`, the on-screen rectangles are:

| Bar | x-range | y-range |
|---|---|---|
| HP | `58..61` | `164..188` |
| Stamina | `65..68` | `164..188` |
| Mana | `72..75` | `164..188` |

### 2.2 Fill behavior

From local ReDMCSB `CHAMDRAW.C`:

```c
if (current < maximum) {
    CopyBytes(coloredBar, blankBar, sizeof(blankBar));
    if (current != 0) {
        height(blankBar) -= max(1, height(blankBar) * current / maximum);
    }
    if (height(blankBar) > 0) {
        FillScreenArea(blankBar, C12_COLOR_DARKEST_GRAY);
        top(coloredBar) = top(blankBar) + height(blankBar);
        height(coloredBar) -= height(blankBar);
    }
}
if ((current != 0) && (height(coloredBar) > 0)) {
    FillScreenArea(coloredBar, G0046_auc_Graphic562_ChampionColor[idx]);
}
```

So the Firestaff V1 path now mirrors these visible rules exactly:

- full bar when `current == maximum`
- top blank segment when `0 < current < maximum`
- **minimum 1 px** colored fill when `current > 0`
- full blank when `current == 0`

### 2.3 Champion colors

Local ReDMCSB `DATA.C` anchors the four champion colors directly:

```c
unsigned char G0046_auc_Graphic562_ChampionColor[4] = { 7, 11, 8, 14 };
```

So pass 43 uses:

| Slot | Palette index | Color |
|---|---:|---|
| 0 | 7  | green |
| 1 | 11 | yellow |
| 2 | 8  | red |
| 3 | 14 | light blue |

This retires the invented Firestaff per-stat color scheme.

---

## 3. Exact visual gain

### Before pass 43

V1 still drew an invented left-anchored horizontal strip inside each
status box:

- HP: `x+4, y+20, 59×2`
- stamina: `x+4, y+23, 59×1`
- mana: `x+4, y+25, 59×1`

### After pass 43

V1 draws the original-style right-side vertical bars instead:

- HP: `x+46, y+4, 4×25`
- stamina: `x+53, y+4, 4×25`
- mana: `x+60, y+4, 4×25`

So the concrete visual gain is:

- **horizontal status strip removed** from the left 42 px of the old bar body
- **three 4×25 vertical bars restored** in the source-faithful right column
- **source champion colors restored** (`7/11/8/14`) instead of per-stat colors
- **CHAMDRAW min-1px partial-fill behavior restored**

---

## 4. Verification

### 4.1 Required gates

All required gates stayed green on the pass-43 tree:

| Gate | Result |
|---|---|
| `./run_firestaff_m11_phase_a_probe.sh` | **18/18 invariants passed** |
| `./run_firestaff_m11_game_view_probe.sh` | **361/361 invariants passed** |
| `./run_firestaff_m11_launcher_smoke.sh` | **PASS** |
| `./run_firestaff_m10_verify.sh "$HOME/.firestaff/data/GRAPHICS.DAT"` | **20/20 phases PASS** |
| `./run_firestaff_m11_verify.sh` | **PASS** |

### 4.2 Pass-43-specific probe

New bounded probe:

```sh
./run_firestaff_m11_pass43_bar_graph_probe.sh
```

Result:

- **10/10 invariants passed**

What it proves:

1. slot-0 bar origins are exactly `58/65/72 @ y=164`
2. source champion-color table is exactly `{7,11,8,14}`
3. pass-43 geometry + mode switch are present in `m11_game_view.c`
4. 100/100 renders a full-height `4×25` bar
5. 50/100 renders top blank / bottom fill split `13 + 12`
6. 1/100 renders the CHAMDRAW **minimum 1 px** fill
7. slot colors render as green / yellow / red / light blue
8. the old left-anchored horizontal strip body no longer carries V1 stat fill

### 4.3 Screenshot artifact

Reproducible screenshot written by the pass-43 probe:

- `verification-m11/pass43-bar-graphs/pass43_party_bar_graphs.pgm`

Reproducible general HUD screenshot also written by the game-view probe:

- `verification-m11/game-view/party_hud_statusbox_gfx.pgm`

---

## 5. What remains after pass 43

Pass 43 is strictly the champion bar-graph migration.

Still outstanding before pass 44:

- spell-panel rune labels still render as text instead of the `C011` graphic
- font-bank wiring, palette, viewport bind, and final panel-origin work remain unchanged
- no new claim is made here about those areas

Pass 43 fully lands `V1_BLOCKERS.md` §7 and nothing broader.
