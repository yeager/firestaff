# USA scripted mouse delta replay remains uncalibrated

Four isolated Mednafen reference captures on TRV2 used the authentic USA
Ak-Tu-Ba state (MD5 `f17f377df210b4a3ae904a13fb85a7f0`), authentic USA Track
02, and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`). The instrumented Mednafen binary SHA-256
was `c8e2836e174c8fb87540b48378c6c4e0496040e40eebf1b45fe26dc737145082`.
The wrapper snapshot is retained with the traces on the task's external-disk
evidence folder, `firestaff-theron-mouse-trace-20261029/`.

## Captured input and command

| Capture | Scripted samples | Device latch | Instrumented result |
| --- | --- | --- | --- |
| `click-left-column` | `60,120,0;0,0,1;0,0,0` | `-60,-120` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column` | `121,98,0;0,0,1;0,0,0` | `-121,-98` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column-inverted` | `-121,-98,0;0,0,1;0,0,0` | `121,98` | `$2911=$50`, `$2905=$50`, then cleared |
| `click-right-column-split` | `-121,0,0;-121,0,0;0,0,1;0,0,0` | first latch `127,0` | no stable target receipt |

In the first three captures, the RAM instrumentation attributed the `$50`
write to logical PC `$CCDB` and clear to `$D3A0`. The hover resolver also
attributed a `$2911=$50` write to `$D293`. These are saved-state trace labels,
not a live MPR/source-LBA join. For example, the `click-left-column` mouse
trace has SHA-256
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

## 2026-10-29 standard-PCE matched controls

Three additional bounded loops compared `4,0,0;0,0,1*3;0,0,0*3` against
zero-delta-plus-Button-I and the earlier `60,120` control. The authentic USA
Ak-Tu-Ba save state, USA full CUE/Track 02, System Card 3.0, and instrumented
Mednafen were held constant. PCE Fast rejected the state with CPU/input
compatibility warnings, so these traces used standard PCE. The controls and
all three small-delta loops have byte-identical command-RAM and consumer
sidecars (SHA-256 `dd9caf4f71d85760f746d9fa76f8dc4604441fce3ec9b2921a5a5ffb180d0eea`
and `2a83ab15c491eac34a268bcb79d97310fec4d764a53a9ed4ed1a5e347a160cd0`).

Every run recorded the same `$2911=$50`, `$2905=$50`, and `$20C7/$20C9=$3C/$78`
values, including zero mouse delta. The `dx=4` device sample reached the
emulated device as `x=4` and latched as `x=-4`, but produced no different
guest-side rows. This does not support a mouse-coordinate calibration from
the loaded state; it shows that this save-state route and script did not
distinguish that delta from the control. All captures still report
`transition=missing` and zero authenticated CD-to-RAM receipts.

## Logical-PC attribution correction

The authentic USA and JP `$D395` source windows are disassembled in
[`theron-d39d-static-source-disassembly-20261029.md`](theron-d39d-static-source-disassembly-20261029.md).
In the matched retail window, `$D3A0` is inside the operand of `LDA $B4`, not
a clear instruction. The saved-state instrumentation's `$D3A0` label therefore
does not identify the clearing code if that source mapping is active. Preserve
the RAM observations, but treat their logical-PC attribution as unresolved
until a same-session trace includes executed opcode bytes, active MPRs,
physical PC and authenticated CD-to-RAM provenance.
