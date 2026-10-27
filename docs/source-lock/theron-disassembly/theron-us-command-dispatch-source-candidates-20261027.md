# USA original mouse-command dispatch source candidates

## Runtime snapshot identity

Three isolated standard-PCE captures used the authentic USA Ak-Tu-Ba save
state and the instrumented Mednafen binary documented in
[`theron-us-scripted-mouse-state-compatibility-20261027.md`](theron-us-scripted-mouse-state-compatibility-20261027.md).
The 64 KiB command code-page snapshot was byte-identical in all three runs:
SHA-256 `57920b254fe1199799052879146b96731638afc41ebe03ce17db5e1d21e842ca`.
The original command queue trace recorded `$2905=$50` at writer `$CCDB`, reads
at `$D34D` and `$D395`, and clear at `$D3A0` in each run. The saved state
bypasses the current CD session; these observations are not a CD-sector-to-RAM
receipt.

## Authentic Track 02 candidates

The USA Track 02 BIN has MD5 `f23601102138f87c33025877767ebf76`, size
8,104,992 bytes, and SHA-256
`f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`.
Every 32-byte window at the captured logical PCs occurs at the following
seven offsets in this hash-verified raw image:

| Logical PC | Captured 32-byte SHA-256 | USA Track 02 raw file offsets |
| --- | --- | --- |
| `$CCDB` | `fbfe13d7288780ce8191ae58151dd20a9d52fe255aa5541ad248dd8b76d333e3` | `671579`, `972635`, `1273691`, `1574747`, `1875803`, `2176859`, `2477915` |
| `$D34D` | `abf492a47b244a2d3a2185b765e1bf27a2583b4cc40466d15b3c86baf9ff941a` | `673533`, `974589`, `1275645`, `1576701`, `1877757`, `2178813`, `2479869` |
| `$D395` | `1df987fd3f39cab8af44dea067ac0cf89168e1fc41ae98e85b1bde7355c33634` | `673605`, `974661`, `1275717`, `1576773`, `1877829`, `2178885`, `2479941` |
| `$D3A0` | `70f0f8beed7cefd609a0dc0db53a69f83a4046104f4efe0c6bdc3b13f5c8a760` | `673616`, `974672`, `1275728`, `1576784`, `1877840`, `2178896`, `2479952` |

The four windows maintain their observed within-candidate displacements:
`$D34D` is `+1954` raw bytes after `$CCDB`, `$D395` is `+2026`, and `$D3A0`
is `+2037`. Every candidate has raw sector offset 1259 within its respective
Track 02 member sector. The complete 64 KiB snapshot has no exact full-image
match, and all seven candidate regions are repeated retail data. This is
static correlation to authentic source bytes, not dynamic source-LBA identity.

## Evidence boundary

The captures establish original command dispatch from the authentic saved
gameplay state. Their transition receipts report `transition=missing` and zero
authenticated CD-to-RAM receipts. This candidate inventory does not prove
which Track 02 copy supplied the bytes, bind the relative mouse delta to the
viewport coordinates `$3C/$78`, show a world-state change, or establish full
native pointer parity. Keep command semantics and native world-click
publication gated until a cold-start capture joins the live input, executing
bytes, and resulting game-owned state to the same authentic runtime source.
