# Source-locked DM1 V1 touch/click zone matrix

This pass keeps touchscreen support at the evidence/probe layer. It records a compact active in-game click-zone matrix without changing keyboard routes or replacing M11's existing V1 mouse-command bridge.

## Source audit

- `COMMAND.C:375-395` defines `G0447_as_Graphic561_PrimaryMouseInput_Interface`: champion HUD toggles/clicks (`C151..C154`, `C187..C190`), champion icons, spell parent `C013`, action parent `C011`.
- `COMMAND.C:396-405` defines `G0448_as_Graphic561_SecondaryMouseInput_Movement`: six movement arrows (`C068..C073`), dungeon viewport `C007`, and right-click leader inventory toggle `C002`.
- `COMMAND.C:412-451` defines source-backed inventory controls: close/save/rest/music icons, inventory slot boxes `C507..C536`, mouth `C545`, eye `C546`, and panel `C101`.
- `COMMAND.C:461-483` defines active action/spell subroutes: pass `C098`, action rows `C082..C084`, action icons `C089..C092`, caster `C221`, rune symbols `C245..C250`, cast `C252`, and recant `C254`.
- `CLIKMENU.C:529-573` source-locks action-name/menu dispatch: pass clicks highlight `x=285..319 y=77..83` and call `F0391(-1)`; action rows use inclusive boxes `x=234..318` with `y=86..96`, `98..108`, and `110..120`, gated by `(command - C112) <= G0507_ui_ActionCount`, then call `F0391(0..2)`.
- `CLIKMENU.C:578-582` source-locks champion action-icon dispatch: icon boxes are `x=233..252`, `255..274`, `277..296`, `299..318`, all `y=86..120`; the derived champion index is accepted only when it is `< G0305_ui_PartyChampionCount`.
- `COMMAND.C:484-497` defines champion name/hand hit zones: leader names `C159..C162` and status hand slots `C211..C218`.
- `COMMAND.C:2314-2320` dispatches mouth/eye commands to `F0349_INVENTORY_ProcessCommand70_ClickOnMouth` and `F0352_INVENTORY_ProcessCommand71_ClickOnEye`.
- `DEFS.H:3748-3937` names the relevant `C002..M701` zone constants for the I34E family.
- `COORD.C:2036-2245` resolves layout records (`F0634_GetLayoutRecord`, `F0635_`) and `COORD.C:2451-2505` exposes `F0637_GetProportionalZone` / `F0639_LoadLayoutRanges` for runtime layout-696 data.
- `zones_h_reconstruction.json` is the verbatim `GRAPHICS.DAT` entry `C696_GRAPHIC_LAYOUT` dump for `DM1 PC 3.4 English / I34E` (SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`).

## Landed probe

- `touch_click_zone_matrix_pc34_compat.c/.h` exposes 104 source-backed active in-game zones.
- `test_touch_click_zone_matrix_pc34_compat_integration.c` gates representative movement, action, spell, champion hand, inventory viewport-relative, and smallest-zone hit-test behavior.
- `action_area_routes_pc34_compat.c` now verifies all source action name/icon rows are present in the matrix and exposes a tiny source-locked action-area resolver for command mapping, action-count/party-count gates, and inclusive endpoint tests.

## pass347 follow-up

Pass347 adds the active screen-space source-order helper for touch dispatch: primary interface routes are tested before secondary movement routes, matching `COMMAND.C:1641-1644`. Inventory/panel viewport-local zones remain explicit and are promoted only at the touch seam. Broader provider gesture work should continue to stop at coordinate normalization and must not add downstream touch-specific command shortcuts.
