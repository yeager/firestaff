# JP Stage-2 `$45xx` tier-2 source match

The authenticated JP Rev. 1 Track 02 image contains the same exact bytes as
the previously bounded US `$45xx` tier-2 windows. Both source images were
read-only inspected on trv2. Their whole-file identities are US BIN MD5
`f23601102138f87c33025877767ebf76` and JP BIN MD5
`b7afb338ad31be1025b53f9aff12d73a`.

Stage-2 starts at raw sector 1224 for US and 1223 for JP. For each window, the
comparison addresses the Stage-2 payload's normalized user-data offset, then
maps each 2048-byte user sector to raw MODE1/2352 at `sector * 2352 + 16`.
Each exact-byte SHA-256 is identical in both regions:

| Stage-2 user offset | Bytes | SHA-256 |
| --- | ---: | --- |
| `$43A1` | `0x35` | `6cd80e46b0f5f932754a8f5be2986ef9a82ec33e3eda573ed03de969ea72c9aa` |
| `$42BF` | `0x1C` | `b6962b1f27de35a9a27e15b8ea0388fa679b83feb47fd8a25bc6ec4dd93aea29` |
| `$45A6` | `0x0B` | `9c8dddb32bc1ba24c0a8cc65602e3bbcfcb78f4542106a47149a383915477a87` |

The production verifier now accepts these windows for either authenticated
BIN variant. The focused regional test checks successful receipts and
byte-for-byte equality alongside the separate tier-3 windows. A temporary
TRV2 harness compiled the production verifier against the changed source and
called tier-2 on both authentic images and tier-3 on JP; three loops passed.
The same loops confirmed that flipping one JP byte in the `$43A1` window is
rejected even when the known JP media identity is supplied.
This directly exercised the production verifier but did not run the registered
CTest target or the full CMake build for this commit. No game data was copied
into the source archive or committed. These are static source bytes and
disassembly only; they do not prove runtime bank selection, execution, return
values, VDC behavior, or gameplay semantics.
