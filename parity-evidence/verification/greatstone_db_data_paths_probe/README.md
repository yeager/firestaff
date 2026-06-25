# greatstone_db_data_paths_probe

Bounded metadata-only regression gate for
`docs/FIRESTAFF_GAP_LIST.md` A5 ("Real-data regression tests
(greatstone db_data)").

## What it covers

* **10 current `db_data/` paths** — the curated examples pinned in
  the gap-list row: DM `dm_pc_34/graphics.dat/graphics.dat.html`,
  DM `dm_snes_11_jp_ntsc/smc/smc.html`, CSB
  `csb_atari_21_en_stx/graphics.dat/graphics.dat.html`, CSB
  `csb_amiga_udr2_en/hcsb.htc/hcsb.htc.html`, DM2
  `dm2_pc10_en/graphics.dat/graphics.dat.html`, DM2
  `dm2_amiga_10_enfrge/lang.ftl/lang.ftl.html`, DM2
  `dm2_segacd_10_en/stry.dat/stry.dat.html`, plus the three
  `g_dm/g_csb/g_dm2` index pages.
* **3 obsolete 404-regression paths** — the `c_dm_*`, `c_csb_*`,
  and guessed-DM2 (`dm2_pc10/...`) paths the gap list says
  "may 404" and that we want to keep 404-locked.

## How it works

* The probe (`tools/verify_greatstone_db_data_paths.py`) sends a
  bounded `Range: bytes=0-8191` GET against each URL, drops the
  body after extracting `<title>`, title SHA-256, content-type
  hint, content-length hint, and `db_data/` link count. The body
  is never written to disk.
* `200` and `206 Partial Content` are both accepted as
  "reachable" (the server honours our byte-range request). `404`
  is the only acceptable status for the obsolete rows.
* OFFLINE mode (CTest default) replays the fixture at
  `tests/fixtures/greatstone_db_data_paths/index.json`. ONLINE
  mode (`--online` or `FIRESTAFF_GREATSTONE_PROBE=1`) hits the
  live URLs.
* `--write` writes both `manifest.json` (this directory) and the
  OFFLINE fixture in a single step.

## What's not claimed

* No pixel parity, no db_data byte-for-byte mirroring, no
  copyrighted bytes are downloaded or stored. `tools/asset-validate/no_game_data_in_git.py`
  PASS is the safety net.

## How to refresh

```sh
python3 tools/verify_greatstone_db_data_paths.py --online --write
ctest --test-dir build -R greatstone_db_data_paths_probe --output-on-failure
```

The first command refreshes the live metadata + OFFLINE fixture.
The second confirms the CTest gate still passes against the
on-disk evidence.
