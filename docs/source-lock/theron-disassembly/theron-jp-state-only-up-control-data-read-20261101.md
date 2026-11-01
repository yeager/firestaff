# JP state-only UP/control data-read comparison (2026-11-01)

## Scope

This report records a matched pair of bounded Mednafen PCE Fast replays from
the same authenticated Japanese save state. One replay had no scripted
controller event; the other applied `up@61:120`. The state autoload was
verified in both captures. These are emulator reference traces, not native
Firestaff behavior or cold-start captures.

The reference media identity was the JP MODE1/2048 Track 02 image with MD5
`62a39bbf43415c9739c41c2481080a49`. The loaded state had MD5
`8bf0caf3a98b5748d2a78ba8016d2f01` (SHA-256
`b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`).
The original JP Track 02 BIN has MD5 `b7afb338ad31be1025b53f9aff12d73a`.
Both captures report System Card MD5 `ff1a674273fe3540ccef576376407d1d`
and instrumented Mednafen MD5 `5e8e3d390e7bc77836c7f0397f27aea4` (SHA-256
`0280a8e9dcce6ca55b43a29d2f8549fd30ad62c339bca940a2717b58cbca81f6`).
The input-side receipt verifies one applied scripted event followed by a
non-System-Card controller read at `$44C1`; its read included the scripted UP
mask (`raw=0010`).

## Observations

Each movement-context trace retained 256 `movement_data_read` rows over the
bounded frame-60–104 window. After normalizing the per-run event sequence,
the ordered rows are identical between the no-input and UP captures. All 256
read bytes match the authenticated JP Track 02 bytes using the source mapping
anchored at physical `$0E891D` / raw offset `$B0EED` in the
[JP Akutuba source-sector report](theron-jp-akutuba-map-source-sector-20261101.md).
The observed physical addresses begin around `$0E8A04` and `$0E8A9D`, before
the four map arrays begin at `$0E8B03`. Their field meaning is not identified.

Neither bounded movement-context sidecar contains a `movement_command_write`
row. Both wrapper receipts report `transition=missing`; the state-only
autoload path has no cold CD handoff. The UP input was read by the guest, but
the paired traces do not show it changing the observed data-read sequence.

## Boundary

This comparison proves a guest controller-read witness and an identical,
source-matched sequence of 256 opaque data reads in the two bounded replays.
It does not identify those bytes as map cells, select an active map, bind a
party pose or movement command, prove that UP caused or failed to cause a
movement outcome, or join the data to VDC/VCE/VRAM output. No retail-frame,
cold-handoff, native-runtime, or gameplay-parity claim follows from this
state-only pair.

The next useful disassembly target is the reader path at `$50D2/$50D6` and the
source region preceding the map arrays. Capture a later matched frame window
only after identifying a concrete state field or branch whose interpretation
can distinguish input handling from periodic reads. Keep source bytes and
field semantics separate until a consumer is established.

## Review artifacts

Raw traces and emulator logs remain in the external Theron evidence store; no
game media or save-state binary is added to the repository. The transition
receipts identify the same Track 02, state, System Card and instrumented
emulator for both runs. Their capture IDs and principal sidecar hashes are:

| Run | Capture ID | Transition receipt SHA-256 | Movement-context SHA-256 | Input-consumption receipt SHA-256 |
|---|---|---|---|---|
| Neutral | `cc49d2c48574439dbb52186dd3145db1` | `a99c4ebfdde1673bf2a59a25e7cadcd72cf55c396d8109682b3ee86f819e53f4` | `5ad09c0f17ff666fd0960ae6c94fb92cc7e65d6dbccefa3a811a0bd6bc358049` | Not applicable |
| UP | `2ec69bfed9bf4868ba86079029306ad1` | `67a7be242cb569b769d23d51e0d0201642b6020ac244f7671599336d5b3a0d94` | `1202ba237e16e64e40e435223a2a26200c724a28611247392ecc41b4a72dc716` | `2e24c4d43b55e334d2d62a7570287eb25af644525364db38cfcafdc30c4b4305` |

The pair was checked directly against the retained raw sidecars and authentic
JP BIN. Per-run event sequence numbers differ, so the comparison normalizes
that field before checking ordered `movement_data_read` rows. The hashes bind
the report to the reviewed capture artifacts; the state and original media
remain external.
