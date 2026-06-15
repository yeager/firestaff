# DM1 V1 Mirror-Candidate Close After Party Shuffle

Runtime regression marker:

- `pass783_dm1_v1_mirror_candidate_close_after_party_shuffle`

Contract-only scope:

- Synthetic, asset-free state model.
- No original-DOS pixel parity claim.
- Proves the ordered command sequence: two F0284 party-direction
  rotations, then a C160 Yes close click on the live C040 mirror
  candidate panel.

ReDMCSB anchors:

- `CHAMPION.C F0284:93-130` rotates the per-champion `Cell` and
  `Direction` by a delta derived from the requested direction vs.
  `G0308_i_PartyDirection`, updates `G0308`, and calls
  `F0296_CHAMPION_DrawChangedObjectIcons`. The loop runs over
  `M516_CHAMPIONS[0..G0305_ui_PartyChampionCount)`.
- `CHAMPION.C F0296` redraws the changed champion icon boxes; the
  `C038` panel priority byte, `C037` status hand box, `C159`
  champion icon, `M070` panel owner, and `M568` panel content
  remain byte-stable across the F0296 redraw pass.
- `REVIVE.C F0282:744-806` reads the appended candidate as
  `M516_CHAMPIONS[G0305_ui_PartyChampionCount - 1]` on the
  C160/C161/C162 click path and clears `G0299_ui_CandidateChampionOrdinal`.
  The C160 path also decrements `G0305_ui_PartyChampionCount` and
  disables the first sensor at the post-rotation mirror square.
- `COMMAND.C F0361:1709-1813` queues the keyboard turn input
  (TURN_LEFT/TURN_RIGHT) into the shared command queue.
- `COMMAND.C F0359:1452-1662` queues the C040 panel Yes click.
- `COMMAND.C F0380:2045-2156` drains one command at a time and
  routes C160 to F0282.
- `DEFS.H` anchors: `C040`, `C037`, `C038`, `C159`, `C160`, `C161`,
  `C162`, `M070`, `M516`.

Non-overlap scope:

- This lane is intentionally non-duplicative with:
  - `pass780_dm1_v1_mirror_candidate_resurrect_chest_close_order`
    (C040 Yes + chest close + move forward + wheel up, no F0284
    rotation).
  - `dm1_v1_mirror_candidate_party_direction` (5 turn inputs + status
    click, no C160 close path).
  - `dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate`
    (close first, then F0284 rotation, then reopen).
  - `dm1_v1_mirror_candidate_reshuffle_panel_live` (party slot
    reorder via `swap_party_slots` / `move_party_slot`, not
    direction rotation; close is via `close_candidate_panel`, not
    C160 click).
  - `dm1_v1_mirror_candidate_close_button` (close click without any
    F0284 rotation).
  - `dm1_v1_mirror_candidate_c040_chrome_inventory_owner_swap`
    (single F0284 step in the same `_run` call; not two F0284 calls
    followed by C160 close).

Anchor deviations:

- The lane uses the contract-only synthetic state model; the
  contract `ready` check rejects the run if `contractOnly`,
  `noAssetReads`, or `noOriginalDosPixelParityClaim` are unset.
- The F0282 close read in this lane is bound to the
  `post-shuffle party` (`G0305-1`) so a future regression that
  caches the candidate index across F0284 calls is caught at the
  gate.
