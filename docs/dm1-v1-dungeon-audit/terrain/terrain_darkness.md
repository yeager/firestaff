# DM1 V1 Darkness/Visibility — Source Audit

## ReDMCSB Source
- DATA.C:359: G0039_ai_Graphic562_LightPowerToLightAmount[16] = { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }
- TOOWNSGLB.H:1306: extern declaration for G0039
- DEFS.H:5333: extern declaration for G0039
- PANEL.C:412: Torch light accumulation: L1036_i_TotalLightAmount += (G0039[*AL1040_pi_TorchLightPower] << L1037_ui_TorchLightAmountMultiplier) >> 6
- CHAMPION.C:529: G0407_s_Party.MagicalLightAmount -= G0039[2] (torch removal)
- CHAMPION.C:645: G0407_s_Party.MagicalLightAmount += G0039[2] (torch add)
- MENU.C:1608: G0407_s_Party.MagicalLightAmount += G0039[2] (torch from menu)
- MENU.C:1936/1941: G0407_s_Party.MagicalLightAmount +/-= G0039[AL1267_ui_LightPower] (spell light)
- TIMELINE.C:1754: F0257 light decay — G0039[power] - G0039[weakerPower] = delta

## Light Power System

Light is tracked as a power ordinal (0-6 for DM1 spells), not a raw radius.
G0039 maps power ordinal to light amount integer.

Power ordinals (DEFS.H / TIMELINE.C):
  0 = boundary / no light
  1 = minimum light
  2 = torch (G0039[2] = 12)
  3 = more light (G0039[3] = 24)
  4 = bright (G0039[4] = 33)
  5 = very bright (G0039[5] = 40)
  6 = maximum (G0039[6] = 46)

Torch contributes G0039[2] = 12 units of light.
Multiple torches can stack.

## Visibility Mechanics

### Party Light (PANEL.C:412)
Party light is accumulated from all torch-bearing champions:
  L1036_i_TotalLightAmount += (G0039[torchPower] << torchMultiplier) >> 6

### Dungeon Viewport Rendering (DUNVIEW.C)
The viewport rendering uses the party's total MagicalLightAmount
to determine which squares are visible. Only squares with sufficient
light are rendered with full detail; dark squares show minimal/none.

### Darkness Spell (MENU.C:1936/1941)
- Darkness reduces MagicalLightAmount by G0039[lightPower]
- Light spell (Magic Torch) increases it by G0039[lightPower]
- Both use the same G0039 table

### Light Decay (TIMELINE.C:1754, F0257)
When light spell expires, the difference between current power and
next-lower power is subtracted from MagicalLightAmount:
  delta = G0039[power] - G0039[power-1]

## Firestaff Implementation

src/memory/memory_runtime_dynamics_pc34_compat.c:62-70:
  s_PowerOrdinalToLightAmount[7] = { 0, 3, 6, 10, 16, 24, 40 }
  NOTE: Phase 14 placeholder — real G0039 table has {0,5,12,24,33,40,46...}
  Only indices 0-6 are used by DM1-era spells.

src/memory/memory_runtime_dynamics_pc34_compat.c:
  F0865_RUNTIME_ComputeLightDecayDelta_Compat() — delta between power levels
  F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat() — schedules follow-up
  F0867_RUNTIME_ComputeTotalLightAmount_Compat() — clamps at 0
  F0864_RUNTIME_HandleLightDecay_Compat() — event handler, expires light

## Gaps / Needs Disassembly Review

1. G0039 real table values: memory_runtime_dynamics_pc34_compat.c:50-70 is a
   Phase 14 placeholder. Real values (from DATA.C:359) are {0,5,12,24,33,40...}
   but those values come from GRAPHICS.DAT entry 562 and are not yet loaded.

2. Viewport darkness rendering: DUNVIEW.C lines related to light amount
   checks for square visibility need deeper tracing. The exact visibility
   radius formula (how light amount -> visible squares) is not yet in Firestaff.

3. Party map square light visibility: The dungeon view's per-square light
   check logic that determines which squares are visible vs dark needs
   full source-lock in the viewport layer.

## STATUS: PARTIAL — Light power/decay mechanics aligned. Real G0039 table
and viewport visibility rendering require GRAPHICS.DAT loader (Phase 14).
