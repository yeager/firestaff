# Theron Track 02 buffer-setup caller candidates

## Authentic media

The observations below use the locally installed original Track 02 BINs. They
were read in place; no game media was copied into the repository.

| Edition | Track 02 identity | Setup-candidate offsets |
| --- | --- | --- |
| JP Rev. 1 | MD5 `b7afb338ad31be1025b53f9aff12d73a`; SHA-256 `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | `0x0a7344`, `0x0f0b44`, `0x13a344`, `0x183b44`, `0x1cd344`, `0x216b44`, `0x260344` |
| US | MD5 `f23601102138f87c33025877767ebf76`; SHA-256 `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | `0x0a7c74`, `0x0f1474`, `0x13ac74`, `0x184474`, `0x1cdc74`, `0x217474`, `0x260c74` |

These are the seven exact setup-fragment matches already locked by
[`theron-track02-buffer-setup-candidate-20261027.md`](theron-track02-buffer-setup-candidate-20261027.md).
Only the first JP match is currently mapped to logical `$A704` by that static
listing.

## Bytes following the setup

The 22 bytes immediately following the 26-byte setup fragment are identical
across the first six JP candidates, differ at one JP call operand in the last
candidate, and are identical across all seven US candidates. The earlier MAME
listing maps the first JP window to `$A704`; the other raw matches are kept as
file-relative candidates because their loaded logical mappings are unproven.

JP, candidates 1–6, file-relative bytes `+26..+47`:

```text
20 ab 43  9c 1f 27  9c 1d 27  a9 df  8d 1e 27  a9 01
20 05 45  20 df c1
```

JP, candidate 7, same file-relative window:

```text
20 ab 43  9c 1f 27  9c 1d 27  a9 df  8d 1e 27  a9 01
20 08 45  20 df c1
```

US, candidates 1–7, file-relative bytes `+26..+47`:

```text
20 ae 43  9c 1f 27  9c 1d 27  a9 df  8d 1e 27  a9 01
20 0b 45  20 df c1
```

The opcode decode is `JSR $43AB` (JP) or `JSR $43AE` (US), `STZ $271F`,
`STZ $271D`, `LDA #$DF`, `STA $271E`, `LDA #$01`, a regional call (`$4505`,
`$4508`, or `$450B` as listed), then `JSR $C1DF`. The exact caller bytes are
locked by `test_theron_v1_huc6280_disassembly` against every listed offset.

One static JP `$4505` candidate is disassembled in
[`theron-jp-44fb-runtime-target-candidate-20261007.asm`](theron-jp-44fb-runtime-target-candidate-20261007.asm#L25-L32). That listing shows a helper which adds zero-page `$00` to A and writes the result through HuC6280 `TAM #$40`, then restores A and returns. This is only the meaning of that exact static candidate. It does not bind any of the seven `$A704` copies to a loaded bank, prove that the differing `$4508` or US `$450B` call uses the same body, or identify the bank mapped for `$C1DF`.

## Evidence limit and next join

The caller sequence records addresses and writes, but it does not establish
that the routine executes during a retail click, what `$271D-$271F` represent,
or which code/data bytes are mapped at `$C1DF`. No literal `JSR $A704` or
`JMP $A704` occurs in either raw BIN, and no indirect entry has been
authenticated. The saved-state classifier trace is a separate emulator
observation without a Track 02 source-LBA receipt.

Before implementing native viewport mouse behavior, bind one exact setup copy
to the live HuC6280 PC/MPR and Track 02 source transfer, then bind the selected
`$C1DF` code, rectangle contents, dispatched command, and resulting original
game state in the same session. The present note is disassembly evidence only;
it does not establish mouse parity.
