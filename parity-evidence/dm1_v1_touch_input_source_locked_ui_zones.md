# DM1 V1 touch input source-locked UI zones

Status: DM1_V1_TOUCH_INPUT_SOURCE_LOCKED_UI_ZONES_VERIFIED

Scope: evidence-only gate for viewport, movement controls, action/spell/menu parents, action child rows/icons, and the entrance mouse-table install. No runtime input behavior changes are made by this pass.

ReDMCSB anchors:
- COMMAND.C:375-395 primary in-game interface table.
- COMMAND.C:396-405 secondary movement/viewport/right-button leader-inventory table.
- COMMAND.C:461-497 action, spell, and champion-name/hand child tables.
- COMMAND.C:1379-1449 mouse table scan and button/zone hit test.
- COMMAND.C:1641-1644 primary-before-secondary lookup order.
- COORD.C:1915-1920 inclusive zone bounds check.
- CLIKMENU.C:529-582 action-area child resolution.
- CLIKVIEW.C:365-510 viewport click handler path.
- ENTRANCE.C:717-740 entrance mouse table install.

Guardrail: no parity behavior changed; this only verifies source-locked evidence in the existing touch click matrix. No DUNGEON.DAT or GRAPHICS.DAT variant was read by this verifier.
