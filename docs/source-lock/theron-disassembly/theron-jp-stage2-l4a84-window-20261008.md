# Theron JP Stage-2 `$4A84` source window

This note records the bounded direct-entry body reached by the JSR at Stage-2
user offset `$4a05`. The offsets below are within the Stage-2 record's 2048-byte
user-data stream, not addresses relative to the `$4000` initial load base.

## Authentic source binding

The source images are the hash-verified original Track 02 BINs:

| Edition | Track 02 SHA-256 | INDEX 01 raw sector | Stage-2 first raw sector | Span raw offset |
| --- | --- | ---: | ---: | ---: |
| JP | `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb` | 224 | 1223 | 2,898,324 |
| US | `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565` | 225 | 1224 | 2,900,676 |

The 160-byte span `[0x4a84, 0x4b24)` is identical in both editions. Its
SHA-256 is `11eb9df09038a7cff9f372eca6de2f6e54135b5fc2bd4dffba1559ec79478f4a`
and FNV-1a-64 is `505803b126222d34`. Three independent direct reads of both
authentic BINs reproduced the same digest.

The bounded control-flow checks are the JSR at `$4a89` to `$491f`, the BSR at
`$4ae1` to the exclusive boundary `$4b24`, and the terminal RTS at `$4b23`.
The routine is also reached by the caller's JSR at `$4a05`. These edges and
bytes establish a static source window only; they do not prove runtime entry,
hardware effects, or rendering semantics.
