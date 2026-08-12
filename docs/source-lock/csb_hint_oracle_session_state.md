# CSB Hint Oracle: selection and page-session source lock

`CSB_HintOracleSession` preserves the control state in ReDMCSB's Utility Disk
implementation without claiming a launcher integration.

## Selection

`HINTHINT.C`, `F1937_Hints_CPSX`, case `C09_SELECT_HINTS`, obtains
`PartyMapIndex`, `PartyMapX` and `PartyMapY` from the loaded CSB game and walks
the HTC location records in file order. A record matches when its level equals
the party map and either its coordinate equals the party coordinate or both
coordinate bytes are `255`. The original stops adding entries at seven.

`csb_hint_oracle_session_select_location()` has the same exact rule by using
the checked HTC parser and retaining only the first seven parser results. The
parser's output-too-small status is therefore expected at an eighth match; it
does not reorder, deduplicate or turn the original limit into an error.

## List and pages

`HINTDATA.C` supplies seven list rectangles and `HINTHINT.C`, case
`C06_DRAW_HINT_LIST`, maps them to commands 10–16. Selecting one invokes
`F1940_CPSX` case 36 with page one. LAST and NEXT call the same case with
`-1`/`+1`; controls are shown only while the resulting one-based page remains
inside the hint's authored page count.

The session's `open_hint_row`, `previous_page` and `next_page` reproduce these
transitions. `done` returns from a page to the list, then from the list (or the
no-clue state) to `AWAIT_LOAD`, matching the Utility Disk's prompt for another
save selection. `close` is separate for the original EXIT control.

## Boundary

`CSB_HintOracleAtariSaveSession` now supplies the native saved-game side of
this boundary. It accepts only `CSB_V1_AtariSaveInfo`, the checked GAMEBLOCK2
receipt emitted by `csb_v1_atari_save_decode_pc34_compat()` and the Atari
MINI.DAT runtime handoff. Its signed map/X/Y values must fit in the original
unsigned bytes before it calls the session. It does not accept CSBWin saves,
generic runtime coordinates or a replacement parser.

The adapter is still not connected to M11/M12 presentation. Visual layout and
frame/pixel parity remain open.

`test_csb_hint_oracle_session` creates eight matching authored records and
checks that exactly the first seven source-order entries survive, followed by
the page-boundary, no-clue, DONE and close transitions. No game data is stored
in the repository.
