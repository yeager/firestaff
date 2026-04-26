# Pass 68 candidate — TITLE cadence/menu layout bounded probe

Date: 2026-04-26

## Scope

This candidate pass adds a small, non-runtime probe for the remaining TITLE/menu gap.  It does **not** claim original wall-clock cadence or final menu layout parity.  It tightens the evidence boundary so the next worker can tell source-backed facts from still-open emulator timing/layout questions.

New files:

```sh
probes/v1/firestaff_v1_title_menu_cadence_layout_probe.c
run_firestaff_v1_title_menu_cadence_layout_probe.sh
parity-evidence/pass68_v1_title_menu_cadence_layout_probe.txt
```

## What the probe proves

The probe combines three bounded checks:

1. **TITLE manifest invariants** from the local original `TITLE` mapfile:
   - 12002 bytes
   - 59 records
   - 53 source records (`2 EN + 51 DL`)
   - palette segmentation `37 + 16`
   - source record duration sum `53` (one record per rendered frame; no inferred wall-clock cadence)
2. **Deterministic Firestaff handoff seam**:
   - step 53 still publishes TITLE frame 53 and marks handoff-ready;
   - step 54 switches to the caller-owned MENU surface only when opt-in handoff is enabled;
   - opt-out path continues to hold TITLE frame 53.
3. **Startup menu candidate asset metrics** from `GRAPHICS.DAT` header dimensions:
   - boot-script menu candidate A: graphics `304..319`, serial width sum `357`, max `32x28`;
   - candidate B left: graphics `360..367`, serial width sum `138`, max `30x28`;
   - candidate B core: graphics `368..383`, serial width sum `974`, max `160x111`;
   - candidate B right: graphics `384..391`, serial width sum `407`, max `112x29`;
   - candidate B wide: graphics `360..391`, serial width sum `1519`, max `160x111`.

These are asset/layout-groundwork metrics only.  They identify the source-backed menu graphic windows used by the existing boot scripts, but they do not yet place those graphics on the 320x200 original screen.

## Verification

```sh
./run_firestaff_v1_title_menu_cadence_layout_probe.sh \
  "$HOME/.firestaff/data/GRAPHICS.DAT" \
  "$HOME/.openclaw/data/redmcsb-original/TITLE" \
  | tee parity-evidence/pass68_v1_title_menu_cadence_layout_probe.txt
```

Result: `menuMetricInvariantOk=1`, `titleManifestInvariantOk=1`, `titleHandoffInvariantOk=1`.

## Source timing reconnaissance

Relevant source is available at:

```text
/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206/Toolchains/Common/Source/TITLE.C
```

The PC-family block guarded by `MEDIA508_F20E_F20J_X30J_P20JA_P20JB`/subguards builds 18 shrinked title bitmaps, then presents them with one `M526_WaitVerticalBlank()` per step, followed by two additional vertical blanks, a `StrikesBack` blit/fade, and a final vertical blank before freeing the title bitmap.  That is useful timing evidence, but it is not enough to close DM1 PC 3.4 yet because the exact build macro mapping for local PC 3.4 (`F20E/F20J/X30J/P20JA/P20JB`) still needs to be isolated, and the local original `TITLE` mapfile path is 53 rendered records rather than those 18 generated shrink steps.

## Remaining blocker

Original cadence remains unclaimed for the same reason as pass 67, now narrowed:

- native DOSBox raw screenshots prove still-frame identity, but the mapper path is sparse/blocking;
- this pass proves the Firestaff handoff seam and menu-asset metric window, not original wall-clock timing;
- closing cadence requires either a per-presented-frame emulator framebuffer dump or a PC-conditional source proof that maps the DM1 PC 3.4 `TITLE` mapfile/frame records to actual waits/handoff.

## Next best step

Instrument/capture per-presented-frame emulator output, then run a monotonic match against the 53 Firestaff/Greatstone TITLE frames and compare the first stable menu frame against the candidate B menu graphic window (`360..391`) metrics above.
