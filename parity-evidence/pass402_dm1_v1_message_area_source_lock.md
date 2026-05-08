# Pass402 DM1 V1 message-area source lock

Mandatory audit order: ReDMCSB TEXT.C first, then CSBWin secondary lineage.

- ReDMCSB root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- CSBWin root: `/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin`

## ReDMCSB anchors
- `TEXT.C:1311-1344` — `F0042_TEXT_MESSAGEAREA_MoveCursor`
- `TEXT.C:1346-1405` — `F0043_TEXT_MESSAGEAREA_ClearAllRows`
- `TEXT.C:1416-1510` — `F0044_TEXT_MESSAGEAREA_ClearExpiredRows`
- `TEXT.C:1512-1564` — `F0045_TEXT_MESSAGEAREA_CreateNewRow`
- `TEXT.C:1566-1670` — `F0046_TEXT_MESSAGEAREA_PrintString`
- `TEXT.C:1670-1785` — `F0047_TEXT_MESSAGEAREA_PrintMessage`
- `TEXT.C:1801-1814` — `F0048_TEXT_MESSAGEAREA_PrintCharacter`
- `TEXT.C:1873-1880` — `F0051_TEXT_MESSAGEAREA_PrintLineFeed`
- `TEXT.C:1991-2030` — `F0054_TEXT_Initialize`
- `DRAWMSGA.C:9-385` — `F0696_UpdateMessageArea`
- `DEFS.H:3247` — `row count`
- `DEFS.H:6400` — `character width symbol`
- `DEFS.H:6401` — `line height symbol`
- `DEFS.H:6410` — `message area width symbol`

## CSBWin secondary anchors
- `CSBCode.cpp:1458-1466` — `SCROLLING_TEXT::SetPrintPosition`
- `CSBCode.cpp:1470-1508` — `SCROLLING_TEXT::RemoveTimedOutText`
- `CSBCode.cpp:1432-1455` — `SCROLLING_TEXT::DiscardText`
- `CSBCode.cpp:1565-1573` — `SCROLLING_TEXT::CreateNewTextRow`
- `CSBCode.cpp:1575-1587` — `SCROLLING_TEXT::ScrollUp`
- `CSBCode.cpp:1510-1553` — `SCROLLING_TEXT::ClockTick`
- `CSBCode.cpp:1603-1635` — `SCROLLING_TEXT::Printf`
- `CSBCode.cpp:1665-1729` — `PrintLines`
- `CSBCode.cpp:1735-1739` — `PrintLinefeed`
- `Data.h:1612` — `future queue`
- `Data.h:1651` — `four message rows`
- `Data.h:1660` — `cursor API`

## Firestaff local contract
- `dm1_v1_text_message_pc34_compat.h` — `local-contract` passed=True
- `dm1_v1_text_message_pc34_compat.c` — `local-contract` passed=True
- `test_dm1_v1_text_message_pc34_compat.c` — `local-contract` passed=True

Verifier: `scripts/verify_pass402_dm1_v1_message_area_source_lock.py --run-ctest`.
