# Theron original Akutuba completion capture — 2026-08-21

This source lock records a research-only original-engine transition against
the authenticated US CUE.  It does not authorize synthetic gameplay state.

The Mednafen hook enters the already-resident, byte-verified `DMS-SG.001`
dispatcher at its original `$DE21` entry.  It maps the exact System Card RAM
page `$6D`, rejects unless `DMS-SG.001` and the `$DE38` `CMP #$06` bytes are
present, and replaces only A at `$DE38` with bounded ordinal 0.  Original game
code owns the subsequent CD command and RAM mutation.

Evidence:

- authentic savestate MD5: `f17f377df210b4a3ae904a13fb85a7f0`
- initial 8 KiB BaseRAM MD5: `add6f5e61b849263f2a546164c08b496`
- initial campaign byte `$267C`: `00`
- original SCSI command: start LBA 4201, four sectors
- final 8 KiB BaseRAM MD5: `1b1fd5f634710c3ce9f3a388a0c30e24`
- final campaign byte `$267C`: `01`
- CD trace MD5: `c210f4675bcbbe689ad002d0c013cc7e`

This proves that original post-Akutuba execution sets campaign bit 0.  It does
not yet prove the later Drator bank load: `$20DB` remains 0 and `$20DA` remains
1 in the final snapshot.  Generator RNG/spawn publication therefore remains
closed.  The next capture must continue through the original restart/stage
selection until `$20DB == 1`, then enter Drator level 2 without substituting
host or generated dungeon bytes.

## Original next-program selection and backup-RAM manager

The authentic savestate's System Card RAM page `$6D` was extracted and the
resident `DMS-SG.001` body disassembled. At `$DE21` it first saves the old
campaign mask from `$267C` into `$DF0E`. The bounded ordinal path then updates
`$267C`, but `$DEFB-$DF29` uses the saved old mask to calculate the four-sector
program record, loads it at `$4000`, and jumps to `$4000`. This explains why a
state with old mask `00` could set a completion bit without selecting Drator.

A research-only state changes only `$267C` from `00` to the already proven
Akutuba completion value `01`. The original ordinal-1 dispatcher then selects
absolute LBA 4205. Its MODE1/2048 payload is the exact Track 02 byte range at
sector index 971, four sectors, MD5 `9aa38417c25bd6d1717081d545fdcfa0`
and FNV-1a `337de858`. The System Card polling path cannot finish when entered
asynchronously from the research hook, so the capture tool may replay exactly
that hash-checked 8192-byte CD_READ result into the original `$4000`
destination and resume at the dispatcher's original `$DF29` continuation.
This is synthetic execution used only to derive real data; it does not provide
production gameplay data.

The resumed original overlay made nine further real CD requests: LBA
4229/2, 4233/17, 4257/1, 4994/4, 5002/7, 5014/1, 5015/1, 5017/1 and 4271/1.
The final VDC/VCE frame changed from the Akutuba dungeon to character artwork,
but disassembly of the exact LBA-4205 overlay rejects the earlier visual
interpretation that this was a Drator introduction. The program contains the
original file-cabinet strings `CHOOSE A FILE TO DELETE`, `SURE?`, and
`THIS GAME WILL NOT BE SAVED`, calls the System Card backup-RAM APIs, and loops
on controller state `$2228`. It is a backup-RAM manager in the post-dungeon
chain. This proves original next-program selection, not entry into Drator's
dungeon or presentation. `$20DA/$20DB` remained raw values `01/00` and the
generator context count remained zero.

The capture wrapper now also emits the exact 2048-byte PCE backup-RAM snapshot.
An ordinal-0 run from the unmodified Akutuba state let original `DMS-SG.001`
execute its own `LE051` write. The resulting real backup-RAM image has MD5
`ffabc8d19b0915d4d9632a7ae2e90a97`; its campaign payload byte at file offset
`$20` is `01`, and its System Card-managed header/check fields differ from the
input image. This is the authentic Drator-unlock save record and removes the
need to invent or repair a checksum synthetically. The same run issued the
original four-sector LBA-4201 request after saving.

## Bounded ordinal sweep

The same authenticated state, System Card page, dispatcher entry and original
LBA 4201--4204 program were then run with bounded ordinals 1 through 6.  The
completed snapshots establish the following campaign-byte mapping at `$267C`:

| Dispatcher ordinal | Final `$267C` | Proven result |
| ---: | ---: | --- |
| 0 | `01` | sets bit 0 |
| 1 | `02` | sets bit 1 |
| 2 | `04` | sets bit 2 |
| 3 | `08` | sets bit 3 |
| 4 | `10` | sets bit 4 |
| 5 | `20` | sets bit 5 |

Ordinal 6 is deliberately not promoted as a campaign-bit result.  Its run
reaches the original `CMP #$06` special branch but stalls after only two of the
four requested sectors, leaving `$267C == 00`.  That is evidence of a distinct
final-stage path, not evidence that dungeon 7 has no completion state.

The ordinal-1 capture is the first direct original-engine proof that Drator's
completion identity is campaign bit 1.  It still does not load Drator's dungeon
bank: the final bank bytes remain Akutuba-owned.  Production therefore may use
the bit mapping for campaign progression, but may not infer a Drator generator
RNG/spawn sequence from this capture.
