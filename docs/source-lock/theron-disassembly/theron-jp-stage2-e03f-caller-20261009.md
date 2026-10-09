# JP Stage-2 `$E03F` call-site source lock

The authentic JP Rev. 1 Track 02 BIN contains the same 28-byte caller window
as US at Stage-2 user offset `$375`, logical listing address `$4375`. The
window ends with `JSR $E03F` at `$438E`; the corresponding scanner candidate is
the unique code-region site in the Stage-2 record at user offset `$38E`.

The comparison reads normalized user bytes from raw MODE1/2352 sectors. The
Stage-2 record starts at sector 1224 for US and 1223 for JP. Both media
identities and the identical 28-byte SHA-256 are:

| Region | Track 02 BIN MD5 | Caller SHA-256 |
| --- | --- | --- |
| US | `f23601102138f87c33025877767ebf76` | `512c204baae52e607341a4d0f251460abdb1b27200d213e9429aa0f39fd80a1f` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `512c204baae52e607341a4d0f251460abdb1b27200d213e9429aa0f39fd80a1f` |

The existing MAME HuC6280 listing for the shared US window is
[`theron-us-stage2-huc6280.asm`](theron-us-stage2-huc6280.asm), lines
`L4375..$4390`. The real-media regression now checks the exact caller bytes,
the raw-sector-to-user-offset mapping, and the unique `$E03F` candidate for
each region.

This is static caller provenance. It does not establish that the call executes,
the meaning of `$FF`, a CD track number, runtime selection, or audible output.
