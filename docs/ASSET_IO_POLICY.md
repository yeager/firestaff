# Game-data I/O policy

Firestaff does not unpack, rewrite, or cache game files during a normal run.
The selected original remains the source of truth in its original form:

- loose files are opened from their verified directory;
- ZIP, ISO, CD, and other supported containers are opened through a bounded
  in-memory reader;
- hashes and format receipts are computed over the original bytes;
- a failed packed-media reader fails closed and never falls back to a copied
  or synthetic game file.

Extraction helpers that write files exist only for development and test
fixtures. They are enabled by the test build or by the explicit
`FIRESTAFF_DEVELOPMENT_MEDIA_EXTRACTION` build definition; that definition is
not part of the production application.

This distinction is important for preservation: an `asset-cache` entry is
not an accepted runtime owner for original game data. Runtime metadata must
retain the original path or container locator, including the verified hash.

## Current reader boundary

DM2 FM Towns already uses the original ZIP/CD image and keeps extracted
payloads in bounded RAM for startup and gameplay admission. The same pattern
is required for every supported DM1 and CSB packed-media path before that
path is enabled in the launcher.

An archive that has only a development extractor is therefore reported as
verified source media but is not launchable in production until its native
RAM reader is present.
