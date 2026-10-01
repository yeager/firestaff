# Theron's Original Movement Commands

## Authentic Source

Capture `work/theron-forward-command-complete-v1` comes from the US edition's
authentic Track 02 (MD5 `ceb02343868f80cec899e9b239aff2da`), System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`) and save state
`f17f377df210b4a3ae904a13fb85a7f0`. Input is
`right@1:140,down@2:33,i@145:5`; no generated game data is used.

Button I at internal click coordinate `$8A/$8F` queues command type `$03`. The
trace contains exactly 65,536 ordered main-RAM writes. Its MD5 is
`cae8749574e4668c62b71b131fe62e74`; the authentic 64 KiB code window's MD5
is `77d2253b4aed117847d8d1d685506ca0`.

## Original Movement and Position Chain

Queue dispatch at `$D3B0..$D3CB` sends command types `$03..$06` to
`$CD87`. For the captured `$03` event, the routine reads the current position
from `$40/$41`, calculates the destination cell and passes the approved move's
destination `$03/$03` through `$45/$46`.

The persistent position commit occurs at `$C1FA`:

```asm
tii $20B4,$2040,$0002
```

The write trace records both destination changes at logical PC `$C203` and
physical PC `$0DC203`. The starting position is `$40/$41 = $02/$03`; after
the commit it is `$03/$03`. The RAM image before the command has MD5
`86f7cec0a943402daae0ec0acdf7a372`, and the image after the queue is cleared
by `$2905=$00` has MD5 `eae73a9991630a6fe2bcc48c2c99861c`.

## Backward and Side Commands

Three additional captures from the same authentic starting state complete the
panel's four movement commands:

- `$98/$A5` queues `$04`, sidestep right.
- `$8A/$A5` queues `$05`, backward.
- `$7B/$A5` queues `$06`, sidestep left.

The original routine calculates movement direction as
`(command type - 3 + $3F) & 3`, where `$3F` is the facing direction. `$05`
commits position `$02/$03 → $01/$03` through the same `$C1FA` path; the
command trace MD5 is
`0488166dec2eaec19b8874b01f7f1fba`.

From this starting state, both side cells are blocked. The `$04` and `$06`
traces therefore have no writes to `$2040/$2041`; instead, the command queue
is cleared at `$CC41`. The trace MD5s are `009168a72f01bf4e8bb6095195c880cf`
and `ff097d6bdd2345e21ca40baa6e3fa489`, respectively.

## Production Boundary

Firestaff maps `$03..$06` to the same relative directions and uses the actual
Track 02 cells and objects in the already loaded Theron world for traversal.
Backward and side steps preserve the facing direction. The captures prove
successful forward and backward movement as well as the original's blocked
side path; further special cells remain subject to their own existing
real-data gates.
