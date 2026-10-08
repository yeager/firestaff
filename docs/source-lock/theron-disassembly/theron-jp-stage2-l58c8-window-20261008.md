# Theron JP Stage-2 conditional fallthrough at `$58C8`

The JP-specific `BBR4` at `$58C5` branches to `$5879` when bit 4 of zero-page
`$C8` is clear. The 20-byte window at `$58C8` is the static fallthrough when
that condition is false; this source lock does not prove runtime selection or
execution. The JP sequence calls `$5529` and `$4FEA`, sets `$0A` to `$0A`, then
jumps to `$582C`. The same user-byte window in US is different.

## Authentic source spans

Both Track 02 images were hash-verified before disassembly:

| Edition | Track 02 SHA-256 | Raw sector | In-sector offset | Span raw offset | Span SHA-256 | Span FNV-1a-64 |
| --- | --- | ---: | ---: | ---: | --- | --- |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 1226 | 216 | 2,883,768 | `7c9f36dfa66b0e99dae397222c25d829f3603b8228faa0f61694e46644183e22` | `f79d1ef941e94432` |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 1227 | 216 | 2,886,120 | `a83a78c7b05ca571dcc4587778fd5b6af7e79fda1306526609ea571781668879` | `a9ab5d9c0cc6250c` |

The real-media test checks the JP bytes individually and verifies both edition
digests. The MAME 0.285 listings were decoded directly from the original Track
02 images; see [`theron-jp-stage2-l58c8-huc6280.asm`](theron-jp-stage2-l58c8-huc6280.asm).
No register values, user-visible behavior, or platform parity is inferred.
