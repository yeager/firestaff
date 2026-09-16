# DM1 PC 3.4 original Entrance long-press receipt

## Scope

This is a hash-only receipt for one private, original PC 3.4 runtime capture.
It proves the specified Entrance input reaches a non-blank dungeon frame. It
does not establish a Hall of Champions party state, inventory interaction,
scroll interaction, viewport pixel parity, or any Firestaff result.

## Source and route

- Original `DATA/DUNGEON.DAT` SHA-256:
  `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`.
- Capture backend: DOSBox-X; source media was staged only for private capture.
- Route: wait for the original Entrance, press Return, wait for its C407
  control, then hold left mouse at original-space `(260,50)` for 1500 ms.
- The private raw frame passed the raw-frame health gate: 320x200, seven
  indexed colours, non-black ratio `0.459047`, lower-canvas non-black ratio
  `0.543184`.

## Result

The resulting source frame contains the original dungeon viewport and its
movement controls. Its raw SHA-256 is:

`4e874f8e95c01b57f12d55b785d135ce246c90e887748f70d56db3308e7fd40b`.

The normalized private 224x136 viewport crop has SHA-256:

`fdbf7981bb8820d67ced049d8a4c987e214fe093095b0970da3a58cd004a29d4`.

The original capture remains outside the repository. These hashes bind the
reviewable private capture to the route without publishing copyrighted game
art.
