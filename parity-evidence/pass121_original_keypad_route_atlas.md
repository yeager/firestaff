# Pass 121 — original keypad route atlas

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-151524-pass121-keypad-route-atlas`
- scope: use pass120 discovery (`KP_Left`/`KP_Right` change original viewport) to map short keypad sequences and F1/F4 response after each route.

## Summary

- `left_once` keys `['KP_Left']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `right_once` keys `['KP_Right']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `left_twice` keys `['KP_Left', 'KP_Left']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Left:9fc8530431a3:dungeon_gameplay -> F1:9fc8530431a3:dungeon_gameplay -> F4:9fc8530431a3:dungeon_gameplay
- `right_twice` keys `['KP_Right', 'KP_Right']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Right:9fc8530431a3:dungeon_gameplay -> F1:9fc8530431a3:dungeon_gameplay -> F4:9fc8530431a3:dungeon_gameplay
- `left_right` keys `['KP_Left', 'KP_Right']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Right:48ed3743ab6a:dungeon_gameplay -> F1:48ed3743ab6a:dungeon_gameplay -> F4:48ed3743ab6a:dungeon_gameplay
- `right_left` keys `['KP_Right', 'KP_Left']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Left:48ed3743ab6a:dungeon_gameplay -> F1:48ed3743ab6a:dungeon_gameplay -> F4:48ed3743ab6a:dungeon_gameplay
- `left_up` keys `['KP_Left', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Up:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `right_up` keys `['KP_Right', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Up:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `left_down` keys `['KP_Left', 'KP_Down']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Down:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `right_down` keys `['KP_Right', 'KP_Down']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Down:fbeb1b82cd09:wall_closeup -> F1:fbeb1b82cd09:wall_closeup -> F4:fbeb1b82cd09:wall_closeup
- `left_left_up` keys `['KP_Left', 'KP_Left', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Left:9fc8530431a3:dungeon_gameplay -> KP_Up:9fc8530431a3:dungeon_gameplay -> F1:9fc8530431a3:dungeon_gameplay -> F4:9fc8530431a3:dungeon_gameplay
- `right_right_up` keys `['KP_Right', 'KP_Right', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Right:9fc8530431a3:dungeon_gameplay -> KP_Up:9fc8530431a3:dungeon_gameplay -> F1:9fc8530431a3:dungeon_gameplay -> F4:9fc8530431a3:dungeon_gameplay
- `right_left_up` keys `['KP_Right', 'KP_Left', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Right:fbeb1b82cd09:wall_closeup -> KP_Left:48ed3743ab6a:dungeon_gameplay -> KP_Up:48ed3743ab6a:dungeon_gameplay -> F1:48ed3743ab6a:dungeon_gameplay -> F4:48ed3743ab6a:dungeon_gameplay
- `left_right_up` keys `['KP_Left', 'KP_Right', 'KP_Up']` changed=True: baseline:48ed3743ab6a:dungeon_gameplay -> KP_Left:fbeb1b82cd09:wall_closeup -> KP_Right:48ed3743ab6a:dungeon_gameplay -> KP_Up:48ed3743ab6a:dungeon_gameplay -> F1:48ed3743ab6a:dungeon_gameplay -> F4:48ed3743ab6a:dungeon_gameplay

## Interpretation

This is route-atlas evidence only. It is successful only if a sequence produces non-repeating original gameplay states and/or changes F1/F4 away from the no-party response; otherwise the champion/party-ready blocker remains upstream of overlay parity.

