# Pass 147 — nonstatic 48ed long replay

- run base: `<N2_RUNS>/20260428-195541-pass147-nonstatic-48ed-long-replay`
- source: pass145 recommended replaying non-static 48ed scenarios with longer waits and movement/action sequences.
- completed: 5
- errors: 0

## Movement/wait hash summary

- `key4_right_long`: unique_hashes=4 hashes=014ed52c71a0,a9da0dd6b19e,17bd7e878157,17bd7e878157,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a classes=entrance_menu,title_or_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay
- `return_right_long`: unique_hashes=4 hashes=014ed52c71a0,c1313b9f8f61,17bd7e878157,17bd7e878157,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a classes=entrance_menu,title_or_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay
- `space_down_long`: unique_hashes=4 hashes=014ed52c71a0,29e45c3966eb,17bd7e878157,17bd7e878157,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a classes=entrance_menu,title_or_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay
- `topclick_right_long`: unique_hashes=4 hashes=014ed52c71a0,2edc8130117c,17bd7e878157,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a classes=entrance_menu,title_or_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay
- `panelclick_right_long`: unique_hashes=4 hashes=014ed52c71a0,e0ab1c722115,17bd7e878157,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a,48ed3743ab6a classes=entrance_menu,title_or_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay

## Interpretation

If these long replays keep producing multiple dungeon hashes after movement, the route is a real dynamic-control candidate; if they collapse back to static menu/dungeon hashes, pivot to Hall-of-Champions/source-backed champion setup.
