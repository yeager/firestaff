# LZMA SDK decoder subset

This directory contains unmodified ANSI-C archive-reader and decoder files
from LZMA SDK 26.02, downloaded from <https://www.7-zip.org/a/lzma2602.7z> on
2026-08-30.

The upstream source headers identify the code as public domain. Firestaff
vendors this small decoder subset so native 7z/LZMA2 media support does not
require a `7z`, `7zz`, emulator, shared library, or any other external
runtime component.

Included decoder files:

- `Precomp.h`
- `7zTypes.h`
- `LzmaDec.h`, `LzmaDec.c`
- `Lzma2Dec.h`, `Lzma2Dec.c`

The bounded native 7z container reader also uses these unmodified SDK files:

- `7z.h`, `7zArcIn.c`
- `7zBuf.h`, `7zBuf.c`, `7zCrc.h`, `7zCrc.c`, `7zCrcOpt.c`, `7zStream.c`
- `Bcj2.h`, `Bcj2.c`, `Bra.h`, `Delta.h`, `CpuArch.h`, `CpuArch.c`
- `7zWindows.h` for the SDK's Windows architecture declarations

`7zDec.c` is compiled with `Z7_NO_METHODS_FILTERS`; archive reading and
LZMA/LZMA2 decoding do not depend on SDK filter or PPMd implementations.
Firestaff provides the memory-only stream and bounded allocators outside the
vendor directory. The SDK sources identify themselves as public domain.

The SDK files are intentionally kept unmodified. Firestaff-specific archive
limits, path validation, and in-memory visitor behavior live outside this
directory.
