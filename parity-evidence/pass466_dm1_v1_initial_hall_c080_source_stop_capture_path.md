# pass466_dm1_v1_initial_hall_c080_source_stop_capture_path

- status: `PASS466_SOURCE_STOP_CAPTURE_PATH_LOCKED_TERMINAL_HUD_ROWS_READY_FOR_RECAPTURE`
- generatedUtc: `2026-05-16T06:29:00.573463+00:00`
- redmcsb: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`
- parity claim: **not made**; this is a source-stop capture contract for the masked pass449 terminal HUD/status rows.

## Capture path

- `fresh_initial_hall`, first Hall frame before any movement/turn
- `candidate_click`, command `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`, PC screen click `111,82`
- `candidate_select`, after F0280 candidate append and before any C160/C161/C162 panel command
- `cancel`, command `C162_COMMAND_CLICK_IN_PANEL_CANCEL`, after REVIVE.C F0282 cancel return, following F0457_START_DrawEnabledMenus_CPSF
- `resurrect_confirm`, command `C160_COMMAND_CLICK_IN_PANEL_RESURRECT`, after REVIVE.C F0282 confirm path closes inventory and redraws enabled menus
- `reincarnate_confirm`, command `C161_COMMAND_CLICK_IN_PANEL_REINCARNATE`, after F0281 rename branch returns, F0282 closes inventory, and enabled menus are redrawn

## Terminal HUD/status rows now have source stops

- `cancel.hud_status_crop`
- `resurrect_confirm.hud_status_crop`
- `reincarnate_confirm.hud_status_crop`

## ReDMCSB source locks

- `DEFS.H:989-1013` — DUNGEON_HEADER stores the initial party tuple bits consumed at new game start. ok=True
- `LOADSAVE.C:1940-1944` — Fresh PC34 new-game state is decoded from DUNGEON_HEADER.InitialPartyLocation into map/x/y/direction globals. ok=True
- `COMMAND.C:397-403,2322-2323` — A left click in the viewport dispatches C080 to F0377; panel commands cannot substitute for the candidate click. ok=True
- `CLIKVIEW.C:311-431` — C080 subtracts the PC viewport origin, classifies the front-wall portrait zone, and then invokes F0372 for the front-wall sensor. ok=True
- `DUNVIEW.C:525,3912-3920` — The visible D1C front portrait box is viewport x=96..127 y=35..63; center is the source portrait click point. ok=True
- `COORD.C:1693-1698` — For PC34/I34E, viewport origin maps the portrait center to screen x=111 y=82. ok=True
- `MOVESENS.C:1392,1501-1502` — The source Hall transition is C127 wall champion portrait sensor dispatch into F0280 candidate append. ok=True
- `REVIVE.C:63-67,272-294` — F0280 marks candidate mode, appends the temporary champion, and updates the leader/action HUD state before panel choice. ok=True
- `COMMAND.C:228-240,1985-1991` — Only after C080/C127/F0280 may the decision panel dispatch C160/C161/C162 into F0282. ok=True
- `REVIVE.C:744-783` — Cancel terminal HUD/status capture must stop after candidate is cleared, the status slot is blacked/cleared, and enabled menus are redrawn. ok=True
- `REVIVE.C:785-899` — Resurrect terminal HUD/status capture must stop after candidate mode is cleared, resurrection message/update is emitted, inventory closes, and menus redraw. ok=True
- `REVIVE.C:785-899` — Reincarnate terminal HUD/status capture must stop after rename/reincarnation branch returns, inventory closes, and menus redraw. ok=True

## Non-claims

No new framebuffer parity, no original-vs-Firestaff pixel parity, and no promotion of existing pass173/pass449/pass455/pass464 frames is claimed here.
