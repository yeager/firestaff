# Nexus A-button attack probe (2026-10-25)

## Question

The Japanese retail manual says A attacks in movement mode. This probe checked
whether an A press in an authentic gameplay savestate produced an attributable
combat action. It did not prove an attack.

## Authentic inputs

- Retail cue: SHA-256 `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`.
- `DM.BIN`: SHA-256 `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.
- Japanese IPL BIOS: SHA-256 `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.
- Gameplay savestate: SHA-256 `942154f51530094c4f553d3254573098d056f90da2fbb5966448aefdefeb3372`.
- Internal BUP: SHA-256 `87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`.
- Ymir 0.4.1 executable SHA-256 `2a668687605b4df2054d603be81ddec4d827a86e2f53bef1f5554a3eeb4a4ce5`.
- Port 1 was a ControlPad; Ymir mapped host J to Saturn A. The private profile
  and all raw captures were under `/dev/shm` on TRV2.

The no-input and A-input captures each reloaded the same savestate. A
same-duration no-input run was also captured as a timing control. The game view
did not show an enemy, so it could not establish a successful or failed attack
against a target.

## Result and limits

The pair analyzer reported a queue count of zero before and after A. Its read
cursor moved from 0 to 1, and one stale queue-storage entry changed from event
56 (`EV_SAVELOAD`) to event 29 (`EV_DYNAMIC`). A zero-count queue does not
identify a live event, and this delta does not link the button press to the
event. Candidate context-10 item pairs matched the timed no-input capture; no
champion ownership or combat-state change was proven. The screen captures are
authentic Ymir output, not Firestaff output.

Raw 1 MiB high-WRAM dumps are retained outside Git at
`/Volumes/Extern-disk/nexus-a-button-debug-20261025/raw-memory/`. Their hashes:

| Capture | SHA-256 |
| --- | --- |
| Short no-input baseline | `d39c5032b2932291f48433d77b0359a74cce738e5e273c7bb6d94bc2b82d8d60` |
| Timed no-input control | `8b011fe171d01587ec03dfd4ade7cd8a661357e1cf3632c42529cfbf8574a257` |
| ControlPad A press | `7cabac209d7c8b95bb6ce162eb53d874ad0f142537e85ec97465f6dc19f60621` |

The analyzer reports raw deltas only. The stale event and context slot words
must not be interpreted as attack evidence.

## Screenshots

| Capture | SHA-256 |
| --- | --- |
| [Timed no-input control](captures/no-input-control.png) | `35de5095005c4b59317a7352ef44f4bdb3ad47127b5c306a38b699584b541146` |
| [ControlPad A press](captures/controlpad-a.png) | `c2f3025b28f4cb3d5f876f3b5bbe3559532e560d536ac2a182bed37c7954ca46` |
