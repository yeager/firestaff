# Firestaff DONE — Nexus

## 2026-09-28 — Japanese retail title corpus gate

- Added a dedicated CTest entry for the authentic Japanese `RLOWFIX.BIN` in
  the retail CUE. It reports CTest skip (return code 77) when the Japanese
  CUE is absent, and fails on identity, engine-open, or RES decode errors;
  English/French media cannot accidentally satisfy this Japanese gate.
- Built and ran `nexus_v1_title_res_real_japan` on TRV2 against the installed
  Japanese retail CUE. The engine opened its 137-file disc and verified
  `RLOWFIX.BIN` MD5 `bb650a4e6f7b6374ba8aa86a61f8f523`; all 14 RES entries
  decoded and the authentic champion roster seeded. CTest passed. No game
  data was copied into the checkout or Git.

## 2026-09-28 — Authentic Saturn card metadata inspection

- Extended the read-only BKR overview to show the raw language and timestamp
  bytes plus every allocated block ID. The authentic-corpus self-test checks
  those displayed values against the original header bytes, validates unique
  in-range block IDs, and keeps the payload read-only and uninterpreted.
- On TRV2, rebuilt the editor script in an isolated temporary directory and
  ran `--self-test-bkr` against authentic two- and three-champion retail-written
  BKR images. Both images are 32 KiB, each contains the 20,480-byte
  `DMNEXUS__01` payload using 354 blocks, and both passed. Their SHA-256 values
  are `3d60856119df55ffa4937d13b5dc7487157c7c6b131a999a2e61683316679634` and
  `d9c86de3b0c668f9961a16529b1d5fd72f3e095823f8f0021cf8db8bfba48e9b`. The
  authentic images remain in TRV2 capture storage; no card or game data was
  copied into Git. This does not decode Saturn save fields or enable import.

## 2026-09-28 — Regional real-media test discovery

- The regional title/RES tests now share the normal media-root lookup and fall
  back to `~/.firestaff/data/nexus` when `FIRESTAFF_NEXUS_DATA_DIR` is unset.
  On TRV2, the title/RES test was rebuilt and run with that variable explicitly
  unset; it opened the authentic Japanese CUE, verified the pinned
  `RLOWFIX.BIN` identity and real PLRD rows, and reported absent English and
  French CUEs as skips. No synthetic media was used.

## 2026-09-28 — Absolute-frame-bounded VDP1 trace generation

- Added inclusive absolute-runtime-frame filters for the external Mednafen
  VDP1 write, writer-code, writer-register and snapshot diagnostics. The raw
  capture's VDP1 markers remain relative to its selected window; the launcher
  passes and records the absolute bounds. A source-copy build on TRV2 compiled
  successfully, and the launcher regression passes with an explicitly bounded
  interval. This is capture tooling only; it proves no Nexus asset owner,
  consumer, or presentation behavior.

## 2026-09-28 — Bounded VDP1 pre-capture writer corridor

- The hash-bound 701-frame JP capture beginning at runtime frame 13197 has a
  VDP1 write trace capped at ten million records before captured frame 0.
  In that distinct pre-capture prefix, the exact starts `0x58b58` and
  `0x58c58` are each written once by master-SH-2 PC `0x0601307c`; later rows
  at the same addresses come from PC `0x06026260`. The trace is bound to raw
  SHA-256 `43b8979b79fb69ebe2bad08ae1090e42b784f0a3b700fbdf2bb99a6af130b80c`
  and trace SHA-256
  `2d169758e5fad0ca1783e68224f56a0a2aa18d92c861a8a0aa17da6bd50ce73f`.
  This is a writer-PC observation only; the prefix cannot be joined to the
  701 captured frames, and it does not prove an asset or display consumer.
- The same-session code receipt at PC `0x0601307c` is manifest-hash-bound, but
  its 96-byte instruction window has no exact match in either byte order in
  the 156 authentic files under `.firestaff/data/nexus`. The supplied
  `DM.BIN` is SHA-256
  `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`;
  the code window also does not match that file at the documented
  `0x06010040` load base. Do not infer a `DM.BIN` routine or source owner from
  the runtime PC alone. Traces and all game data remain on TRV2.

## 2026-09-28 — Fresh JP VDP1 window did not reproduce the retained sources

- Captured a no-input, hash-bound JP retail window on TRV2 with
  `skip_frames=13197` and 701 frames. The raw SHA-256 is
  `43b8979b79fb69ebe2bad08ae1090e42b784f0a3b700fbdf2bb99a6af130b80c`; the
  VDP1 write-trace SHA-256 is
  `2d169758e5fad0ca1783e68224f56a0a2aa18d92c861a8a0aa17da6bd50ce73f`.
  Scanning every captured frame found neither recurring source offset
  `0x58b58` nor `0x58c58`; frame 700 instead has one 1,008-byte mode-0 source
  at `0x415a0`. The ten-million-record VDP1 trace budget was reached before
  frame 0, so its writes are a pre-capture prefix and cannot be joined to
  this frame window. This fresh cold-start capture does not reproduce or
  replace the earlier retained window and does not identify a retail owner.
  Keep the existing source-ownership and presentation blockers open. The raw
  capture, traces, BIOS, disc and private game data remain on TRV2.

## 2026-09-28 — Regional MENU.BPK source-scan identity

- Corrected the VDP1 retail source-join scanner to accept the independently
  verified Japanese, English and French `MENU.BPK` SHA-256 identities. This
  prevents the Japanese retail member from being rejected merely because the
  scanner previously preferred the English alternate hash. Added a regression
  test and a dedicated CI check; unknown filenames still grant no identity.
  This improves source investigation only and does not identify either
  recurring VDP1 span, authorize a menu compositor or widen runtime output.
- The analyzer now authenticates and prepares retail members once per run,
  rather than rereading and word-swapping the full corpus for every draw. On
  TRV2, the 701-frame authentic Japanese capture completed its frame-700 join
  against 137 hash-verified retail members with zero rejected files. The
  recurring `0x58c58` and `0x58b58` source spans still have no exact or
  word-swapped retail join; other DGN surfaces did join. Menu ownership and
  semantic admission therefore remain blocked. The capture and game data stay
  on TRV2.
- The source-join summary now reports per-draw coverage instead of labeling
  the whole frame `verified` after any single hit. Re-running the authentic
  frame-700 Japanese capture reports `source_joined_draws=225/227` and
  `source_join=partial`; the two recurring spans remain the unmatched draws.

## 2026-09-28 — VDP1 write-trace pre-capture prefix

- Both VDP1 write-trace analyzers now accept valid V2 records emitted before
  the first frame marker and keep them separate from frame-bounded writes. The
  source join reports pre-capture coverage independently; the summary tool
  selects it only with `--pre-capture`. Parser tests cover the observed prefix
  format and retain V1 behavior. On the hash-bound TRV2 Start/input and control
  captures at frame 10500, both had 20,000 pre-capture rows, no writes in the
  selected frame, and the same mode-5 source span at `0x63e00..0x6c000`; the
  prefix did not cover that span. This is transport evidence only and neither
  identifies the menu asset nor admits presentation. The authentic raw dumps
  and traces remain on TRV2.

## 2026-09-28 — Retail MAPD CD-to-RAM capture integration

- Captured a fresh, hash-bound 162-frame JP retail receipt on TRV2. The real
  verifier accepts all 27,441 CD FIFO words against Track 1 and 13,312
  contiguous loader writes from LBAs 6063--6088 to WorkRAMH, locating the
  authenticated MAPD record and palette. Updated the real-media CTest to
  discover both flat legacy traces and current `traces/`-nested receipts;
  the integrated test passes on the captured retail files. This proves only
  CD-to-RAM transport; RAM-to-VDP2 ownership and semantic admission remain
  blocked. No game media or captures were added to Git.

## 2026-09-28 — Paired Start table-reader observation

- A same-scope cold-start control/Start pair on TRV2 captured frames
  10500--10507 with identical JP BIOS, retail CUE, raw VDP bytes and rendered
  frame 10507. The broadened SH-2 trace reached the same table read at
  `0x0602c940` / PC `0x0601462c`; `R4` was `0` in control and `0x10` in the
  one-frame Start-input run. The analyzer now reports this register state
  rather than assuming a button value as a chain requirement. It labels the
  chain observed, not semantically verified; no rendered/menu transition was
  observed and semantic admission remains blocked. Artifacts remain on TRV2.

## 2026-09-28 — SH-2 RAM read receipt V2 analysis support

- The fail-closed controller-buffer receipt analyzer accepts both V1 and the
  current V2 register-owner format, rejects slave-SH-2 register ownership, and
  can select one frame from a bounded multi-frame trace. Parser regression
  cases cover V1 compatibility, V2 selection and owner rejection. The actual
  TRV2 Start-only V2 receipts parse at frame 10507; their bounded address
  filter omits downstream linked-buffer/table reads, so full consumer-chain
  verification and all menu/action semantics remain blocked.

## 2026-09-03 — Native CUE-media regression audit

- Re-ran the native production boundaries for combat, magic, light, rest,
  experience and world actions, plus CUE data-track and CDDA binding. The
  retail CUE route and native title mouse route passed without BIOS or
  emulator runtime dependencies. ISO-only probes remained skip-safe because
  the staged retail set is CUE/BIN/WAV media, not a standalone ISO.

Reviewed 2026-08-26. Completed work only.

- Native CUE-media corpus checks read the supplied Saturn title resources,
  SAL, MAP, SLEV and raw data without a runtime emulator dependency.
- The full `LEV00.DGN`–`LEV15.DGN` Structure3 mesh corpus is read directly
  from the supplied retail Track-1 CUE/BIN into RAM and checked against its
  canonical per-level MD5 before parsing.  This is source verification only:
  it does not authorize an unbound Saturn mesh renderer.
- The native sound owner now binds retail CUE-declared audio Tracks 02–09 to
  their original BIN payloads and rejects host WAV/OGG/MP3/FLAC substitutes.
  This is source selection only; playback remains closed without a verified
  Saturn decoder and dispatcher.
- An authenticated Saturn capture verifies NBG1 hardware state: enabled
  bitmap mode, 256-colour code, BMPNA palette bank 0 and scroll origin
  `(0,0)`. It does not identify the bitmap/CLUT source or authorize drawing.
- The same authentic frame's full 512×256 indexed NBG1 span and 256-entry
  CRAM decode in native code using the capture's recorded Saturn byte order.
  This is capture-only evidence: it does not identify an asset owner or
  authorize production presentation.
- The real Japanese CUE independently proves Track-1 `STABG.BIN` reaches the
  native STMP/DMWeb first-map consumer (320×168 with retained source palette);
  it remains `no_draw` without an exact VDP source join.
- Development-only VDP tracing has deterministic emulation-frame filters and
  retains fail-closed source correlation for unbound rendering writes.
- A same-session Japanese retail receipt verifies one SH-2 RAM-to-VDP1 copy:
  PC `0x060135e8` transfers 2 KiB from `0x06027874..0x06028074` to
  VDP1 `0x10a00..0x11200` with Saturn word byte order. It is a completed
  transport observation, not an asset, palette, command or title-rendering
  admission.
- A post-intro Japanese retail title receipt binds the full `TITLE.CG` payload
  to word-swapped VDP2 VRAM `0x24020` and the MAPD palette to word-swapped
  CRAM `0x400`. The raw MAPD planes, tilemap transform, layer placement and
  timing remain unbound, so this does not authorize title rendering.
- The post-intro input receipt is completed as a negative observation: title
  frames 13000–13039 are bit-identical with and without verified Start/A
  pulses, so that window is not an interactive start-menu transition.
