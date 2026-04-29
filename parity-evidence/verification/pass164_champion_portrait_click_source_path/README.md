# Pass 164 — champion portrait click source path

This locks the ReDMCSB answer for the route blocker: to recruit a champion, the original input must click the visible champion portrait/wall ornament in the viewport, not jump straight to C160/C161 panel buttons.

## Source route

`COMMAND.C` C007 left-click → `C080_COMMAND_CLICK_IN_DUNGEON_VIEW` → `CLIKVIEW.C:F0377` → C05 front wall ornament/door-button hit → `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor` → `MOVESENS.C:F0275_SENSOR_IsTriggeredByClickOnWall(front square, opposite party direction)` → `C127_SENSOR_WALL_CHAMPION_PORTRAIT` → `REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty`.

## Checks

- SRC_CLICK_001 PASS — `COMMAND.C` lines 403, 2322, 2323: Left-clicks in source viewport zone C007 dispatch as command C080 into F0377.
- SRC_CLICK_002 PASS — `CLIKVIEW.C` lines 311, 348, 349: F0377 converts screen coordinates to viewport-relative coordinates before testing dungeon-view boxes/zones.
- SRC_CLICK_003 PASS — `CLIKVIEW.C` lines 407, 417, 424, 431: When leader hand is empty, clicking the front wall ornament/door-button view cell C05 and not facing an alcove calls F0372.
- SRC_CLICK_004 PASS — `CLIKVIEW.C` lines 5, 21, 23, 25: F0372 targets the square in front of the party and the opposite wall cell — exactly the wall the party is looking at.
- SRC_CLICK_005 PASS — `MOVESENS.C` lines 1309, 1392, 1394, 1501, 1502: F0275 explicitly allows C127 even with no leader, requires the clicked wall cell, then calls F0280 with sensor data.
- SRC_CLICK_006 PASS — `DUNGEON.C` lines 2608, 2612: The visible champion portrait ordinal is sourced from the same C127 sensor data later used by F0280.

## Route implication

Pass162 corrected C160/C161 panel coordinates but still collapsed to `48ed3743ab6a`. Pass163 proved F0280 must come first. Pass164 proves the missing original action is a source-faithful viewport click on the champion portrait/front-wall ornament cell that triggers C127/F0275/F0280. Next runtime pass should therefore drive the party to face a mirror square, click the portrait/ornament cell inside C05, verify candidate-state pixels, then click C160/C161.
