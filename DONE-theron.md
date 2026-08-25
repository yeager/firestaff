# Firestaff DONE — Theron's Quest

Reviewed 2026-08-25. Completed work only.

- The supplied authentic Japanese Rev 1 CUE is hash-verified and reaches the
  native title → stage → Soul Room startup route (`theron-startup-2`).
- The same authentic JP Rev 1 MODE1/2352 Track 02 route reaches native
  `theron-runtime`: it loads the source Akutuba level and publishes party
  position `1,0,0` without treating Japanese raw sectors as US ISO media.
- A bounded public native Track 02 consumer now binds the same authentic JP
  source to all seven campaign dungeons, retaining their source maps and
  2,266 objects in total. This includes Drator (dungeon 2: eight maps and
  291 objects). Transfer semantics, graphics capture and gameplay behavior
  remain independently gated.
- Track 02 intake preserves original media identity and blocks uncaptured
  graphics/palette fallback from production presentation.
- A CUE/BIN package in a ZIP archive starts natively through both direct CLI
  and the start menu. Firestaff hashes and reads the selected Track 02 member
  in memory (`archive.zip::member`) and never writes extracted game data.
- The supplied US CloneCD ZIP starts natively through direct CLI and the
  start menu. Its `.ccd` layout identifies Track 02 as an in-memory bounded
  `.img` slice (MD5 `168bd6a63784e91885df8c47be62ab5a`); the verified
  eleven-input startup route reaches `theron-startup-2`. CloneCD's missing
  225-sector pregap has a separate, source-verified anchor map—no pregap or
  game media is synthesized or written to disk.
