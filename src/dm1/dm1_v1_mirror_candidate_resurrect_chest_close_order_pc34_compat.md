# DM1 V1 Mirror-Candidate Resurrect Chest-Close Order

Runtime regression marker:

- `pass780_dm1_v1_mirror_candidate_resurrect_chest_close_order`

Contract-only scope:

- Synthetic, asset-free state model.
- No original-DOS pixel parity claim.
- Proves the ordered command sequence: C040 YES, chest close, forward move, wheel-up.

ReDMCSB anchors:

- `REVIVE.C F0280:124-132` publishes the C040 resurrect/reincarnate candidate.
- `REVIVE.C F0282:744-806` clears `G0299` and removes the accepted candidate chain.
- `CHEST.C F0333:30-67` opens a chest, walks the per-cell chain, and populates `G0425`.
- `CHEST.C F0334:113-132` closes `G0426`, rewrites the chest while skipping `C0xFFFF_THING_NONE`, clears `G0425`, and relinks through `F0163`.
- `DUNGEON.C F0163:1796-1837` mutates thing lists during the close rewrite.
- `CHAMPION.C F0297:243-298` puts the existing C540 leader-hand item in hand before the close.
- `CHAMPION.C F0298:270-298` removes a leader-hand item; this regression asserts it is not called by the close.
- `CHAMPION.C F0302:662-714` dispatches occupied-slot clicks after the wheel route resolves.
- `COMMAND.C F0359:1452-1662` queues the C040 YES and chest-close clicks.
- `COMMAND.C F0361:1709-1813` queues the forward move after the close.
- `COMMAND.C F0378:1956-1993` routes panel clicks to C040 or chest handlers.
- `COMMAND.C F0380:2045-2178` drains commands in order.
- `DEFS.H` anchors `C30`, `G0425`, `G0426`, `G0423`, `G0305`, `M070`, `M516`, `C040`, `C160..C162`, `C537..C544`, `C159`, `C037`, and `C038`.

Anchor deviation:

- The requested `MOUSE.C F0077:97-126` / `F0078:128-168` file is not present in this local ReDMCSB tree. The local tree defines the same `F0077/F0078` names in platform files, including `IO.C:1102-1122` and `UTAMSCR.C:141-150`. The regression therefore records the wheel write/read boundary without claiming a nonexistent `MOUSE.C` citation.
