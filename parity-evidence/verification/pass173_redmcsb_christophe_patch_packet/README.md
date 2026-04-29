# Pass173 — ReDMCSB Christophe patch packet handoff

## Executive take

This pass records the small, English, source-local ReDMCSB patch queue prepared after the pass172 patch-gap inventory. The goal is to help Christophe review narrow source notes/fixes instead of receiving a broad Firestaff rewrite or vague parity report.

## Packet shape

The handoff packet contains five patches:

1. `0001-document-dm1-v1-recruit-route.patch`
   - Files: `COMMAND.C`, `MOVESENS.C`
   - Purpose: document the DM1/V1 recruit route as `C007` viewport click -> `C080` dungeon-view click -> `C127_SENSOR_WALL_CHAMPION_PORTRAIT` -> `F0280_CHAMPION_AddCandidateChampionToParty`.
   - Review risk: low; documentation/source comment only.

2. `0002-document-champion-portrait-geometry.patch`
   - Files: `DUNVIEW.C`
   - Purpose: document the source portrait box `{96,127,35,63}` and the PC screen-space safe click centre `(111,82)`.
   - Review risk: low; documentation/source comment only.

3. `0003-document-bug0-71-vblank-timing.patch`
   - Files: `TITLE.C`, `ENTRANCE.C`, `ENDGAME.C`
   - Purpose: document that BUG0_71 timing must remain VBlank/frame-cadenced on fast hosts.
   - Review risk: low; documentation/source comment only.

4. `0004-document-bug0-73-command-queue-race.patch`
   - Files: `COMMAND.C`
   - Purpose: document the lost-click race and the safe-fix direction: hold the same command-queue lock across both index selection and queue write.
   - Review risk: low; documentation/source comment only.

5. `0005-fix-bug0-35-without-bug2-00-scroll-regression.patch`
   - Files: `CHAMDRAW.C`
   - Purpose: narrow code-fix candidate: apply the BUG0_35 closed-chest-to-open-chest icon increment only to closed chests, not to open scrolls; this avoids reintroducing BUG2_00.
   - Review risk: moderate; source code change, but intentionally local and tied to existing BUG0_35/BUG2_00 comments.

## Source-lock anchors

- Pass172 patch-gap inventory selected these five as the highest-value Christophe-friendly patches.
- Pass160–171 closed the ReDMCSB source-lock coverage used to avoid emulator-only guessing.
- The packet deliberately avoids DANNESBURK/local machine paths and avoids sending internal worker/runner metadata.

## Recommendation

Send/review the first four patches as comment/test-note patches first, then discuss the `CHAMDRAW.C` code-fix candidate separately. This keeps the upstream conversation friendly and reviewable.
