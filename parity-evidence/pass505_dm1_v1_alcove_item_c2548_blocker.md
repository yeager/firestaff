# Pass505 DM1 V1 alcove wall-item C2548 blocker

Status: BLOCKED_DM1_V1_ALCOVE_ITEM_C2548_ZONE_NOT_MATERIALIZED

Firestaff preserves wall-square extraction and relative-cell 2 filtering, but alcove wall items still route through the normal C2500 item row helper instead of ReDMCSB PC34 C2548_ZONE_ + objectCoordinateSet*7 + G2029[viewSquare].
