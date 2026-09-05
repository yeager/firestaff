# Pass508 - DM1 V1 movement remaining gap after pass373/pass374

Status: BLOCKED_PASS508_DM1_V1_MOVEMENT_REMAINING_ORIGINAL_OVERLAY_GAP_PROVED

Scope: movement/forflyttning evidence only. This pass consumes pass373/pass374 and proves the next remaining gap; it does not promote pixel parity.

## ReDMCSB source audit first

- PASS COMMAND.C:2045-2156 - F0380_COMMAND_ProcessQueue_CPSC: original route proof must show keyboard/click queue reaching the turn/move dispatch boundary.
- PASS CLIKMENU.C:142-179 - F0365_COMMAND_ProcessTypes1To2_TurnParty: turn overlay proof must bind original capture to source direction mutation.
- PASS CLIKMENU.C:180-347 - F0366_COMMAND_ProcessTypes3To6_MoveParty: step overlay proof must bind original capture to destination legality, F0267 movement, stamina, cooldown, and input-wait release.
- PASS MOVESENS.C:738-818 - F0267_MOVE_GetMoveResult_CPSCE: movement parity must observe committed original party tuple and timing side effects.
- PASS GAMELOOP.C:35-97,215-219 - F0002_MAIN_GameLoop_CPSDF: original overlay comparison must use post-command viewport redraw from mutated source party state.

## Authentic PC 3.4 media

- PASS DATA/DUNGEON.DAT bytes=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS DATA/GRAPHICS.DAT bytes=363417 sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS TITLE bytes=12002 sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## Remaining movement parity gap

The next remaining gap is not Firestaff's live movement route: pass373/pass374 already credit that route into source-locked wall/door/occlusion redraw. The remaining movement gap is original-backed proof: a DOS PC/I34E keyboard-buffer or route transcript that reaches F0380, F0365/F0366, F0267, then the post-command F0128 viewport redraw, plus representative movement/HUD/viewport overlay captures tied to that tuple.

Promotion requirements:

- materialized original runtime frames or trace records, not ignored/absent capture assets
- command-specific route labels for turn and step commands
- post-vblank viewport frame hashes/crops that differ where the command changes direction or position
- party tuple evidence: direction, map index, X, Y before and after source movement
- explicit non-claim boundary for Firestaff-only source-equivalent tests until those original artifacts exist

Missing tools/artifacts if blocked: no required executable is missing in this worktree; the missing item is original runtime evidence (DOS PC/I34E keyboard-buffer/F0380 transcript and representative original movement/HUD/viewport overlay captures).
