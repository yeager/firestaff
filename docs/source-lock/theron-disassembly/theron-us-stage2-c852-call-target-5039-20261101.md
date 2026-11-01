# Theron US `$C852` stage-two call-target candidate

## Source join

The authentic US runtime fetch window at logical `$C852` maps to US Track 02
raw file offset `$A3AD2`; its first 512 bytes match the source-lock listing
and the isolated cold Drator fetch capture. At caller address `$C85E`, those
bytes encode `JSR $5039`.

The stage-two user-data payload begins at raw file offset `$2BED90` for US and
`$2BE460` for Japanese Rev. 1, and is loaded at CPU `$4000`. Therefore the
static `$5039` candidate is at raw offsets `$2BFDC9` (US) and `$2BF499` (JP).
The eight bytes are identical in both authenticated regional BINs:

| Address | Bytes | HuC6280 instruction |
|---|---|---|
| `$5039` | `31 4f` | `AND ($4f),Y` |
| `$503b` | `20 79 38` | `JSR $3879` |
| `$503e` | `64 5b` | `STZ $5b` |
| `$5040` | `60` | `RTS` |

The target window SHA-256 is
`1035acb7b3a19a175bbd61cc1174bf0a6d902cded7e87fd8a01f23ec5eb0e859`; its
FNV-1a is `0x0bc3d6d9`. The byte listing is in
[`theron-us-stage2-c852-call-target-5039-20261101.asm`](theron-us-stage2-c852-call-target-5039-20261101.asm).

## Earlier evidence boundary

At the time this report was written, the available runtime capture retained
repeated `$C852` fetches but did not show the intervening `$C85E` or `$5039`
dispatch. A later authentic US cold capture has since recorded the ordered
call and target-address dispatch. Its live `$5039` bytes (`44 05 86`) do not
match this static candidate (`31 4f 20 79 38 64 5b 60`), so the candidate
must not be assigned to the live runtime mapping. See the
[later runtime call-window report](theron-us-c852-runtime-call-capture-20261101.md).
The live target's source owner, data read through `($4f),Y`, and `$3879`
semantics remain unresolved.

Next, source-bind the live target bytes under its captured MPR mapping, then
retain the mapped read through `($4f),Y`, entry/return at `$3879`, and later
`$CBC6`/`$45C3` row reads in one event sequence. Do not assign row, generator,
coordinate, or creature semantics from the static call edge.
