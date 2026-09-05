# DM1 V1 F0111 pass1055 candidate pixel pair

Status: `CANDIDATE_PAIR_ORIGINAL_POSE_UNPROVEN`

The original PC 3.4 pass1055 viewport and Firestaff candidate are sourced from
the same hash-bound retail DUNGEON.DAT and GRAPHICS.DAT. They are **not yet an
authenticated same-pose pair**. The Firestaff capture manifest proves map 0,
party `(6,9)`, direction west. The original pass513 record is explicitly a
scaffold: `f0128MapX`, `f0128MapY`, `f0128Direction`, and `f0097Presented` are
all null. A deterministic input route and a visually similar closed door do
not replace those missing original-runtime observations.

| Area | Pixels | Changed | Changed ratio | Mean absolute channel error | Maximum channel delta |
|---|---:|---:|---:|---:|---:|
| Full 224x136 viewport | 30,464 | 6,874 | 22.5643% | 23.023930 | 255 |
| Source D1C 96x88 panel bounds `(64,18)-(160,106)` | 8,448 | 429 | 5.0781% | 3.326113 | 146 |

The raw comparison is retained because it is useful candidate evidence, but
the 429 pixels cannot be used as a renderer oracle. Eight-connected topology
places the largest components at `(93,41)-(109,54)` (129 pixels),
`(115,41)-(130,54)` (102), `(146,61)-(153,74)` (72),
`(146,41)-(153,54)` (57), and `(146,24)-(153,34)` (42). The pattern is
localized material visible through portcullis openings, not a global palette,
offset, or flip error.

Inspection of the actual Firestaff tuple confirms map 0 `DoorSet0=0`, door
Thing type 0, and therefore retail graphic 248. Door Thing 153 has ornament 0
and ends its chain. The next center square `(4,9)` contains one real junk Thing,
type 29 (apple). Experimentally admitting that farther object through the
closed-door visibility mask changed the metric from 429 to 441 pixels, so the
experiment was reverted: it did not establish that the original capture is
the same world pose or justify a production rendering change.

The comparator now reads the pass513 scaffold and will emit
`CANDIDATE_PAIR_ORIGINAL_POSE_UNPROVEN` until the original runtime record binds
the exact F0128 tuple and confirms F0097 presentation. If those fields are
captured, it can promote either an exact match or a measured authentic
same-pose mismatch without changing tolerances.

Reproduce after building the capture probe:

```sh
firestaff_dm1_v1_viewport_wall_capture_probe PC34_DATA_DIR CAPTURE_DIR
python3 tools/compare_dm1_v1_f0111_pass1055_pixel_pair.py CAPTURE_DIR --json
```

The comparator reads and hashes `DATA/DUNGEON.DAT` and `DATA/GRAPHICS.DAT`
directly inside the canonical ZIP. It rejects a Firestaff crop unless the
sibling capture manifest records the exact party tuple, and separately checks
whether the original debugger tuple is complete. It does not extract game
data, generate replacement art, hide differences behind a tolerance, or claim
parity from route labels.
