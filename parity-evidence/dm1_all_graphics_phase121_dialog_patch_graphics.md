# DM1 all-graphics phase 121 — dialog 1/2/4-choice patch graphics

## Problem

After pass 120, dialog message zones, choice text zones, and choice selection flow were source-backed. The remaining major `F0427_DIALOG_Draw` visual gap was the source patch graphics used when the dialog has 1, 2, or 4 choices.

ReDMCSB source:

```c
case 1:
    F0632_COORD_SetNegativeBitmapPointer(M621_NEGGRAPHIC_DIALOG_PATCH_1_CHOICE, L1315_puc_Bitmap_DialogPatch);
    F0658_BlitBitmapIndexToZoneIndexWithTransparency(M621_NEGGRAPHIC_DIALOG_PATCH_1_CHOICE, C451_ZONE_DIALOG_PATCH_1_CHOICE, CM1_COLOR_NO_TRANSPARENCY);
    ...
case 2:
    F0632_COORD_SetNegativeBitmapPointer(M622_NEGGRAPHIC_DIALOG_PATCH_2_CHOICES, L1315_puc_Bitmap_DialogPatch);
    F0658_BlitBitmapIndexToZoneIndexWithTransparency(M622_NEGGRAPHIC_DIALOG_PATCH_2_CHOICES, C452_ZONE_DIALOG_PATCH_2_CHOICES, CM1_COLOR_NO_TRANSPARENCY);
    ...
case 4:
    F0632_COORD_SetNegativeBitmapPointer(M623_NEGGRAPHIC_DIALOG_PATCH_4_CHOICES, L1315_puc_Bitmap_DialogPatch);
    F0658_BlitBitmapIndexToZoneIndexWithTransparency(M623_NEGGRAPHIC_DIALOG_PATCH_4_CHOICES, C453_ZONE_DIALOG_PATCH_4_CHOICES, CM1_COLOR_NO_TRANSPARENCY);
```

Negative bitmap metadata from ReDMCSB `COORD.C:G2002_NegativeBitmaps`:

```text
M621 index 38: src x=0   y=14  w=0/224 h=75
M622 index 39: src x=102 y=52  w=21    h=37
M623 index 40: src x=102 y=99  w=21    h=36
```

Patch destination zones from layout reconstruction:

```text
C451: type=1 parent=4 d1=0   d2=51
C452: type=1 parent=4 d1=102 d2=89
C453: type=1 parent=4 d1=102 d2=62
```

## Change

Added source patch-copy helpers:

```c
m11_copy_dm_dialog_patch(...)
m11_apply_dm_dialog_choice_patch(...)
```

When the source dialog backdrop is active:

- 1-choice dialogs copy `M621` source region into `C451`
- 2-choice dialogs copy `M622` source region into `C452`
- 4-choice dialogs copy `M623` source region into `C453`
- 3-choice dialogs remain unpatched, matching source behavior

## Gates

Added invariants:

- `INV_GV_172N` — V1 single-choice dialog applies source M621/C451 patch
- `INV_GV_172O` — V1 two/four-choice dialogs apply source M622/M623 patches

```text
PASS INV_GV_172N V1 single-choice dialog applies source M621/C451 patch
PASS INV_GV_172O V1 two/four-choice dialogs apply source M622/M623 patches
# summary: 434/434 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

## Remaining gaps

Dialog `F0427_DIALOG_Draw` is now source-backed for backdrop, version text, message zones/splitting, choice text zones, choice hit flow, and 1/2/4 patch graphics. Remaining overlay work is mainly:

- original overlay comparison captures
- endgame source visual path
