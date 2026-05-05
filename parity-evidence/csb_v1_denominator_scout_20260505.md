# CSB V1 denominator scout — 2026-05-05

Scope: evidence-only scout on N2 after the DM1/V2 workstream was reported green enough to start looking ahead. This pass inspected existing Firestaff CSB parity notes plus the two local CSB reference-source trees requested for the denominator question. It does **not** enable CSB launch/render/gameplay and does not edit Firestaff runtime code.

## Existing Firestaff evidence reviewed

Relevant repo notes already establish the current CSB boundary:

- `docs/design/CSB_V1_BOOTSTRAP_SCOUT.md` says CSB is cataloged/asset-aware but launch-gated, and lacks the DM1-style source lock, original capture path, dungeon-data identity check, layout proof, and runtime boot/rendering path.
- `parity-evidence/csb_atari_st_v2x_source_runtime_matrix.md` curates **Atari ST English v2.x** as the current CSB renderer lane and records the v2.0 MSA root `GRAPHICS.DAT`/`DUNGEON.DAT` hashes, but also lists missing capture/runtime gates.
- `parity-evidence/pass-n2-csb-target-platform-curation-20260430.md` records the Atari-vs-Amiga graphics lineage split; Atari and Amiga graphics are not interchangeable.
- `parity-evidence/blocker-n2-csb-sample-save-search-20260430.md` records that no extracted curated `CSBGAME*.DAT`/`.BAK` sample is present under approved N2 original-game roots.

## Reference-source trees inspected

### `/home/trv2/.openclaw/data/firestaff-csb-source/CSB/src`

- Git origin: `https://github.com/zelurker/CSB.git`
- HEAD: `dda570585abb4c8113a3298d21c0b599e6cac4f9` (`2026-01-04 16:10:31 +0100`, `Adding some translations from the level 3 walls`)
- Files excluding `.git`: `154`
- Extension inventory: `.cpp=61`, `.c=9`, `.h=30`, `.txt=8`, `.hct=1`, `.dat=0`
- `README` sha256: `ac62c130353221fc2599bf6e045d09910ab498d4055c8e35893a22153421788a`
- `README` states required play files are `dungeon.dat`, `hcsb.dat`, `hcsb.hct`, `mini.dat`, `graphics.dat`, and `config.txt`; those files are not distributed with that source tree.

### `/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin`

- Git origin: `https://github.com/BeipDev/CSBWin.git`
- HEAD: `2f63d10d9b8c155e0be17888271d394255ce1bac` (`2020-11-21 17:46:50 -0800`, `Update readme with DM to CSB transfer notes`)
- Files excluding `.git`: `93`
- Extension inventory: `.cpp=44`, `.h=12`, `.txt=2`, `.hct=1`, `.dat=8`
- `README` sha256: `ac62c130353221fc2599bf6e045d09910ab498d4055c8e35893a22153421788a`
- `Game/readme.txt` documents a CSB start flow through `Play CSB.bat`, Dungeon, Utility, and `Make New Adventure`, but this is a CSBWin play workflow, not a Firestaff V1 parity denominator.

`CSBWin/Game` data hashes observed:

| file | bytes | sha256 |
| --- | ---: | --- |
| `config.txt` | 5335 | `16c773f4de1f0543acf2ebcb79102df5ca8dfaa701aba2586fd467d7c72e980a` |
| `hcsb.dat` | 30793 | `5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc` |
| `hcsb.hct` | 66172 | `1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38` |
| `mini.dat` | 42815 | `61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9` |

No `graphics.dat` or `dungeon.dat` payload is present in `CSBWin/Game`, so this tree does not by itself close the canonical runtime-input gap recorded in the existing Firestaff evidence.

## Denominator result

Result: **CSB V1 parity denominator remains unknown**.

What is countable now is only a reference-source inventory: two local CSB/CSBWin source trees, their HEADs, coarse source-file counts, and several CSBWin helper data hashes. That is not yet a valid Firestaff CSB V1 parity denominator because:

1. The curated CSB renderer lane is Atari ST English v2.x, but neither inspected source tree provides a Firestaff-verified original capture harness or a mapping from Atari ST v2.x frames/states to Firestaff V1 assertions.
2. Existing evidence already rejects platform-substitution: Atari and Amiga graphics payloads differ, and CSBWin helper data cannot be treated as equivalent to the curated v2.0 MSA root `GRAPHICS.DAT`/`DUNGEON.DAT` without a separate proof.
3. No extracted curated CSB save sample is available for the saved-game/new-adventure path, so runtime/state coverage cannot be counted from `CSBGAME.DAT` evidence.
4. A raw count of `.cpp`/`.h` files or CSBWin modules would be a misleading denominator: it measures reference-source inventory, not parity surfaces, source-locked original behaviors, screenshots, probes, or acceptance tests.

Minimum next evidence to make the denominator countable:

- define the first CSB V1 domain list (startup, dungeon load, champion/prison flow, viewport/HUD, input, save/new-adventure, etc.) as acceptance surfaces;
- attach each surface to either ReDMCSB/CSBWin source lines plus canonical Atari ST v2.x inputs, or explicitly mark it blocked;
- add an Atari ST original capture/probe path that produces stable 320x200 frame/state anchors; and
- only then count green/blocked/unknown surfaces in a CSB-specific matrix.

## Verification commands run

```sh
# Existing Firestaff evidence lookup
grep -RIn "CSB V1\|V1 denominator\|denominator\|CSB.*unknown\|csb_v1" docs parity-evidence --include="*.md"

# Reference source inventory and git identities
find /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src -maxdepth 2 -type f
find /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin -maxdepth 2 -type f
git -C /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src log -1 --format="%H %ci %s"
git -C /home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin log -1 --format="%H %ci %s"

# Count/hash inventory
python3 - <<PY
from pathlib import Path
import hashlib
for root in [Path(/home/trv2/.openclaw/data/firestaff-csb-source/CSB/src), Path(/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin)]:
    files = [p for p in root.rglob(*) if p.is_file() and /.git/ not in str(p)]
    print(root, len(files))
    for ext in [.cpp, .c, .h, .txt, .dat, .hct]:
        print(ext, sum(1 for p in files if p.suffix.lower() == ext))
PY
```
