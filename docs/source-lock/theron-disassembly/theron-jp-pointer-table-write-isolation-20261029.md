# Japanese Theron candidate-address write isolation

Date: 2026-10-29

## Reference setup

Four isolated 25-second Mednafen 1.32.1 `pce_fast` captures used the same
authentic Japanese Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`), System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), and Japanese Ak-Tu-Ba save state (MD5
`14dec90b96ec3e14622ec0fab535a92f`). The instrumented binary MD5 was
`fd40271af5c640f06a2fae0f9a34c70a`. Each run began from a fresh Mednafen
profile. The capture wrapper reports no CD-to-RAM provenance because loading
the save state bypasses the original media loader.

The inputs isolate the zero-input control, motion, and Button I conditions:

| Capture | Mouse samples | Writes to watched BaseRAM range |
| --- | --- | --- |
| `r8` | 868 zero samples | None |
| `r9` | Four 4-update deltas: `(+12,0)`, `(0,+12)`, `(-12,0)`, `(0,-12)` | None |
| `r10` | Button mask `01` for updates 245–247 and 432–434; otherwise zero | Four writes |
| `r7` | Movement plus the same two Button I presses | Ten writes |

The instrumented BaseRAM hook watched logical `$21D6` and `$28FC..$2905`.
The movement-only result is distinct from the Button I-only result: `r9`
produced no writes in the watched range, while `r10` wrote `$3C` to `$28FE`
and `$28FF` from PC `$CCDD`, and `$78` to `$2902` and `$2903` from PC `$CCE2`.
The mixed `r7` run also wrote `$3C` to `$28FC`, `$78` to `$2900`, and `$50`
to `$2904` and `$2905`; the original later cleared `$2904/$2905` from
`$D39D`. This associates writes with the mixed input condition but does not
show which movement/click interaction selected them.

The 8 KiB final BaseRAM snapshot hashes were:

| Capture | SHA-256 |
| --- | --- |
| `r7` | `8a7da68da88571d1fe7fff22d94230903470cc71c6583d922735ac1ed413d35b` |
| `r8` | `ad849bedf8ef6482f82d26186a6d47cff0f8c8568a6e141f07484c8b5adbd110` |
| `r9` | `ad849bedf8ef6482f82d26186a6d47cff0f8c8568a6e141f07484c8b5adbd110` |
| `r10` | `716cb34fa5368eac887446be802f677365230a5eaf2856fc637e220930d4758a` |

## Interpretation limits and next steps

The captures report `transition=missing`, zero authenticated CD-to-RAM
receipts, and zero non-System-Card CD reads. The state has no cold-start loader
provenance. The writer PCs and changed candidate bytes are useful for
disassembly, but the bytes are not yet tied to a pointer descriptor, cursor
coordinate, hit target, or command meaning. In particular, `r10` did not write
`$2905`, while the mixed `r7` run did; neither run alone identifies the
trigger or a valid hit region. Do not implement or document new hitboxes from
these observations.

Next, disassemble the bounded writer windows around `$CCD8`, `$CCDD`, `$CCE2`,
and `$D39D`, identify whether the candidate bytes belong to a pointer table,
and reproduce the
same result in a cold original-media session with authenticated loader
provenance. Firestaff's native world-click path and state-driven renderer
remain open.

Private traces and save-state remain on TRV2 in the four
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-pointer-consumer-20261029-r{7,8,9,10}/`
capture directories. No game media, save-state, firmware, or raw memory
snapshot was added to Git.
