#!/usr/bin/env python3
"""Pass 111: DM1/V1 inventory slot namespace audit.

This is a bounded worker-only verifier for the inventory lane. It checks that
Firestaff's source-backed inventory panel exposes the full DM1 C507..C536 slot
box namespace, keeps equipment/backpack helpers separated, and keeps inventory
icons on the direct F0038/Object icon path rather than the action-area palette
remap path.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HOME = Path.home()
SOURCE = ROOT / "m11_game_view.c"
HEADER = ROOT / "m11_game_view.h"
GREATSTONE = HOME / ".openclaw/data/firestaff-greatstone-atlas/index/keyword_hits.json"
REDMCSB = HOME / ".openclaw/data/firestaff-redmcsb-source"
ORIGINAL = HOME / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA"

EXPECTED_ALL = [
    (507, 6, 53), (508, 62, 53), (509, 34, 26), (510, 34, 46),
    (511, 34, 66), (512, 34, 86), (513, 6, 90), (514, 79, 73),
    (515, 62, 90), (516, 79, 90), (517, 6, 33), (518, 6, 73),
    (519, 62, 73), (520, 66, 33), (521, 83, 16), (522, 100, 16),
    (523, 117, 16), (524, 134, 16), (525, 151, 16), (526, 168, 16),
    (527, 185, 16), (528, 202, 16), (529, 83, 33), (530, 100, 33),
    (531, 117, 33), (532, 134, 33), (533, 151, 33), (534, 168, 33),
    (535, 185, 33), (536, 202, 33),
]
EXPECTED_EQUIP_ZONE_IDS = list(range(507, 520))
EXPECTED_BACKPACK_ZONE_IDS = list(range(520, 537))
EXPECTED_CHECKSUMS = {
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}


def sha256(path: Path) -> str | None:
    if not path.exists():
        return None
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def extract_table(text: str, name: str) -> list[tuple[int, int, int]]:
    m = re.search(rf"static const M11_V1InventorySlotZone {name}\[\] = \{{(.*?)\n\}};", text, re.S)
    if not m:
        return []
    rows = []
    for zone, x, y in re.findall(r"\{\s*(\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\}", m.group(1)):
        rows.append((int(zone), int(x), int(y)))
    return rows


def record(checks: list[dict], check_id: str, ok: bool, detail: str, **extra: object) -> None:
    item = {"id": check_id, "ok": bool(ok), "detail": detail}
    item.update(extra)
    checks.append(item)


def main() -> int:
    text = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    checks: list[dict] = []

    all_slots = extract_table(text, "kV1InventorySourceSlotBoxZones")
    equip_slots = extract_table(text, "kV1InventoryEquipmentSlotZones")
    backpack_slots = extract_table(text, "kV1InventoryBackpackSlotZones")

    record(checks, "INV_P111_01", all_slots == EXPECTED_ALL,
           "C507..C536 slot-box table is the expected 30-entry source namespace",
           count=len(all_slots), first=all_slots[:1], last=all_slots[-1:])
    record(checks, "INV_P111_02", [z for z, _, _ in equip_slots] == EXPECTED_EQUIP_ZONE_IDS,
           "equipment helper covers only C507..C519", count=len(equip_slots))
    record(checks, "INV_P111_03", [z for z, _, _ in backpack_slots] == EXPECTED_BACKPACK_ZONE_IDS,
           "backpack helper covers C520..C536, including the first backpack slot plus 8x2 carried-object rows",
           count=len(backpack_slots))
    record(checks, "INV_P111_04", "return M11_GameView_GetV1SlotBoxNormalGraphicId();" in text and "C033 normal slot-box bitmap" in text,
           "inventory source slot boxes resolve to C033 normal slot-box bitmap")
    record(checks, "INV_P111_05", "M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot" in header and "M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox" in header,
           "bidirectional inventory source-slot/champion-slot mapping is exported for probes")
    record(checks, "INV_P111_06", "F0038_OBJECT_DrawIconInSlotBox" in text and "without this palette rewrite" in text,
           "inventory icons are documented on the direct F0038 path, distinct from action-area G0498 palette remap")

    gs_hits = []
    if GREATSTONE.exists():
        try:
            data = json.loads(GREATSTONE.read_text(encoding="utf-8", errors="replace"))
            gs_hits = [h for h in data if "item" in [k.lower() for k in h.get("keywords", [])] or "object" in [k.lower() for k in h.get("keywords", [])]]
        except Exception:
            gs_hits = []
    record(checks, "INV_P111_07", GREATSTONE.exists() and len(gs_hits) >= 1,
           "Greatstone atlas keyword index is present and contains item/object reference pages",
           path=str(GREATSTONE), hits=len(gs_hits))

    red_files = list(REDMCSB.rglob("*")) if REDMCSB.exists() else []
    red_names = sorted(p.name for p in red_files if p.is_file())
    record(checks, "INV_P111_08", REDMCSB.exists() and len(red_names) >= 1,
           "ReDMCSB reference directory is present on N2 (currently documentation/locator pack, not extracted C source)",
           path=str(REDMCSB), files=red_names[:8])

    for fname, expected in EXPECTED_CHECKSUMS.items():
        actual = sha256(ORIGINAL / fname)
        record(checks, f"INV_P111_SHA_{fname}", actual == expected,
               f"original DM PC 3.4 {fname} checksum matches worker reference",
               path=str(ORIGINAL / fname), sha256=actual, expected=expected)

    ok = all(c["ok"] for c in checks)
    result = {
        "pass": 111,
        "lane": "dm1_v1_inventory_slots_icons_evidence",
        "ok": ok,
        "checks": checks,
        "source": str(SOURCE),
    }
    out_dir = ROOT / "parity-evidence/overlays"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "pass111_v1_inventory_slot_namespace_audit.json"
    out_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    for c in checks:
        print(("PASS" if c["ok"] else "FAIL"), c["id"], c["detail"])
    print("Evidence:", out_path)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
