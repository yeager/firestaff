# CSB V1 Amiga held-input timing calibration

Date: 2026-09-16

## Scope

This is a text-only receipt for an original FS-UAE capture calibration. It
does not publish game frames, extracted media, firmware, or a Firestaff
pixel-parity claim.

## Authenticated media

| Input | SHA-256 |
| --- | --- |
| Supplied CSB Amiga disk A | `fec04e89515fe01778f3830bb0d49d94676156abad008e946a882c0e875a6c02` |
| Supplied CSB Amiga disk B | `e8b2f9f9ece38785582e71a8a3d94f86b1830412726bb40f442ad9dd659f9ad1` |
| Caller-supplied Kickstart 1.3 | `ee05862d8102a08436ac4056da7d549db31625c7d47b24dfb7b3c9a5c113ca53` |

The original two-disk set was mounted read-only. The third emulator drive was
only a second read-only mount of disk B; no save media was created or written.

## Result

The capture helper accepted the held-input transcript
`88:Return@750` and produced three native FS-UAE canvas frames at 72, 86 and
102 seconds. Their SHA-256 values, in timestamp order, were:

1. `2a5783cace37cd08c4370ad35b8ff3e04f3d885f645c0e8bc11013c6a71dbbfe`
2. `6b71b814eb14988f185c3eeb976989fd07f0f4ed34966cfe4288f497435db703`
3. `4772cb853bd91c594402e84eaf68aa10c4cc29186c545ed0fbf069eb97456579`

All three frames still showed the original title sequence. Therefore the
held Return occurred before an input-accepting title/menu surface and must
not be interpreted as a failed game input path. Future original Amiga runs
must synchronize input to an observed menu phase rather than a fixed early
wall-clock timestamp.

A later no-input phase scan of the same two-disk media produced three more
native canvas frames at 130, 160 and 190 seconds, respectively:

1. `f37bb0b0abe77391841184532f172a3c2f4455c9c7e6f59fdf1632061e57b9fa`
2. `c36cf31bd01fcf577e41106d05fc089ff774f9803c06197276ded450b38400d7`
3. `bb21f493451ddc31a6d5683959da2ca97cd2afa4fe161eb05cc51af9fc1f2423`

Those frames also remained in the title sequence. The current strict lower
bound for an unattended fixed-timestamp input is therefore greater than 190
seconds for this capture configuration. This is a timing calibration only,
not a claim that the original title has a fixed wall-clock duration.

## Tooling boundary

`scripts/capture_csb_amiga_startup.sh` accepts `seconds:key@milliseconds`
tokens (for example, `88:Return@750`). The helper is development-only; it does
not add an emulator, Kickstart, extracted media, or capture dependency to
Firestaff at runtime.
