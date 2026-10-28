# USA scripted mouse delta replay remains uncalibrated

Four isolated Mednafen reference captures on TRV2 used the authentic USA
Ak-Tu-Ba state (MD5 `f17f377df210b4a3ae904a13fb85a7f0`), authentic USA Track
02, and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`). The instrumented Mednafen binary SHA-256
was `c8e2836e174c8fb87540b48378c6c4e0496040e40eebf1b45fe26dc737145082`.
The wrapper snapshot is retained with the traces on the task's external-disk
evidence folder, `firestaff-theron-mouse-trace-20261029/`.

## Captured input and command

| Capture | Scripted samples | Device latch | Original result |
| --- | --- | --- | --- |
| `click-left-column` | `60,120,0;0,0,1;0,0,0` | `-60,-120` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column` | `121,98,0;0,0,1;0,0,0` | `-121,-98` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column-inverted` | `-121,-98,0;0,0,1;0,0,0` | `121,98` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column-split` | `-121,0,0;-121,0,0;0,0,1;0,0,0` | first latch `127,0` | no stable target receipt |

In the first three captures, the queued value `$50` was written from original
PC `$CCDB` and cleared from `$D3A0`. The hover resolver also wrote `$2911=$50`
from `$D293`. For example, the `click-left-column` mouse trace has SHA-256
`612954c6aaf6ca041b3715d1f2fac068a332c3f5310f5dd18a20244ee1426e7d`; its
command-RAM trace has SHA-256
`2df9e3ab67fb755e4e835af42776ca12ebbd222f402bf91609cb6b1c38e2ceb5`. The
nominal right-column mouse trace has SHA-256
`a5378a09f1a8aa5fe7faff0a251213cd9f6209d173a3ea4e57f0ce6192401a9a`; its
command-RAM trace has SHA-256
`30c426ca9288d33513a5af3b3e0eb66c5df7bb8d7d79cafca56524b0fc70b742`.

## Evidence limit

The captures do not bind the injected deltas to the guest's final pointer
coordinates. The loaded save state does not serialize Mednafen's mouse-device
fields, and the observed device-latch deltas reverse the scripted signs. The
nominal right-column candidate therefore does not reproduce the previously
captured original point `$79/$62 -> $74`; the repeated `$50` result is not
evidence that this region is a `$50` hitbox.

Every capture reports `transition=missing` and zero authenticated CD-to-RAM
receipts because save-state loading bypasses original loader provenance. These
are input-injector diagnostics, not source-bound hitbox or gameplay evidence.
The next capture must bind the device deltas to guest pointer coordinates,
reproduce `$79/$62 -> $74`, and retain an original-session provenance receipt
before any new world-click behavior is implemented.
