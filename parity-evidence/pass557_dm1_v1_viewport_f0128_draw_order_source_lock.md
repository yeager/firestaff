# Pass557 - DM1 V1 viewport F0128 draw-order source lock

Status: PASS_PASS557_DM1_V1_VIEWPORT_F0128_DRAW_ORDER_SOURCE_LOCK

This gate extracts the ReDMCSB F0128 relative-coordinate draw sequence and compares it with Firestaff's s_draw_order table.

## Locked Pairs
- 00 ok=True DM1_VIEW_SQUARE_D4L depth=4 lateral=-1 ReDMCSB DUNVIEW.C:8469 Firestaff call=F0115:D4L objects
- 01 ok=True DM1_VIEW_SQUARE_D4R depth=4 lateral=1 ReDMCSB DUNVIEW.C:8473 Firestaff call=F0115:D4R objects
- 02 ok=True DM1_VIEW_SQUARE_D4C depth=4 lateral=0 ReDMCSB DUNVIEW.C:8477 Firestaff call=F0115:D4C objects
- 03 ok=True DM1_VIEW_SQUARE_D3L2 depth=3 lateral=-2 ReDMCSB DUNVIEW.C:8482 Firestaff call=F0676_DrawD3L2
- 04 ok=True DM1_VIEW_SQUARE_D3R2 depth=3 lateral=2 ReDMCSB DUNVIEW.C:8486 Firestaff call=F0677_DrawD3R2
- 05 ok=True DM1_VIEW_SQUARE_D3L depth=3 lateral=-1 ReDMCSB DUNVIEW.C:8491 Firestaff call=F0116_DUNGEONVIEW_DrawSquareD3L
- 06 ok=True DM1_VIEW_SQUARE_D3R depth=3 lateral=1 ReDMCSB DUNVIEW.C:8495 Firestaff call=F0117_DUNGEONVIEW_DrawSquareD3R
- 07 ok=True DM1_VIEW_SQUARE_D3C depth=3 lateral=0 ReDMCSB DUNVIEW.C:8499 Firestaff call=F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
- 08 ok=True DM1_VIEW_SQUARE_D2L2 depth=2 lateral=-2 ReDMCSB DUNVIEW.C:8504 Firestaff call=F0678_DrawD2L2
- 09 ok=True DM1_VIEW_SQUARE_D2R2 depth=2 lateral=2 ReDMCSB DUNVIEW.C:8508 Firestaff call=F0679_DrawD2R2
- 10 ok=True DM1_VIEW_SQUARE_D2L depth=2 lateral=-1 ReDMCSB DUNVIEW.C:8513 Firestaff call=F0119_DUNGEONVIEW_DrawSquareD2L
- 11 ok=True DM1_VIEW_SQUARE_D2R depth=2 lateral=1 ReDMCSB DUNVIEW.C:8517 Firestaff call=F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
- 12 ok=True DM1_VIEW_SQUARE_D2C depth=2 lateral=0 ReDMCSB DUNVIEW.C:8521 Firestaff call=F0121_DUNGEONVIEW_DrawSquareD2C
- 13 ok=True DM1_VIEW_SQUARE_D1L depth=1 lateral=-1 ReDMCSB DUNVIEW.C:8525 Firestaff call=F0122_DUNGEONVIEW_DrawSquareD1L
- 14 ok=True DM1_VIEW_SQUARE_D1R depth=1 lateral=1 ReDMCSB DUNVIEW.C:8529 Firestaff call=F0123_DUNGEONVIEW_DrawSquareD1R
- 15 ok=True DM1_VIEW_SQUARE_D1C depth=1 lateral=0 ReDMCSB DUNVIEW.C:8533 Firestaff call=F0124_DUNGEONVIEW_DrawSquareD1C
- 16 ok=True DM1_VIEW_SQUARE_D0L depth=0 lateral=-1 ReDMCSB DUNVIEW.C:8537 Firestaff call=F0125_DUNGEONVIEW_DrawSquareD0L
- 17 ok=True DM1_VIEW_SQUARE_D0R depth=0 lateral=1 ReDMCSB DUNVIEW.C:8541 Firestaff call=F0126_DUNGEONVIEW_DrawSquareD0R
- 18 ok=True DM1_VIEW_SQUARE_D0C depth=0 lateral=0 ReDMCSB DUNVIEW.C:8542 Firestaff call=F0127_DUNGEONVIEW_DrawSquareD0C

## Non-claims
- This is a source-lock gate only.
- Same-viewport original-vs-Firestaff pixel capture remains outside this pass.
