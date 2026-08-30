# LZMA SDK decoder subset

This directory contains the unmodified ANSI-C decoder subset from LZMA SDK
26.02, downloaded from <https://www.7-zip.org/a/lzma2602.7z> on 2026-08-30.

The upstream source headers identify the code as public domain. Firestaff
vendors this small decoder subset so native 7z/LZMA2 media support does not
require a `7z`, `7zz`, emulator, shared library, or any other external
runtime component.

Included files:

- `Precomp.h`
- `7zTypes.h`
- `LzmaDec.h`, `LzmaDec.c`
- `Lzma2Dec.h`, `Lzma2Dec.c`

The files are intentionally kept unmodified. Firestaff-specific container
validation and size/CRC limits live outside this directory.
