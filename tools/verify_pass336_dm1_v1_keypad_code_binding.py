#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "parity-evidence/verification/pass336_dm1_v1_keypad_code_binding/manifest.json"
REPORT = ROOT / "parity-evidence/pass336_dm1_v1_keypad_code_binding.md"
EXPECTED = ["004B", "004C", "004D", "004F", "0050", "0051"]
REQUIRED_SOURCES = {"IO2.C", "COMMAND.C", "GAMELOOP.C", "STARTUP2.C", "DEFS.H"}

def main() -> int:
    m = json.loads(MANIFEST.read_text(encoding="utf-8"))
    assert m["schema"] == "pass336_dm1_v1_keypad_code_binding.v1"
    assert m["status"] == "BLOCKED_PASS336_ROUTE_INJECTION_LAYER_AFTER_READY"
    codes = [b["codeHex"] for b in m["exactRuntimeBinding"]]
    assert codes == EXPECTED, codes
    sources = {a["file"] for a in m["sourceAudit"]}
    assert REQUIRED_SOURCES <= sources, sources
    assert any(a["file"] == "COMMAND.C" and "F0361" in a["function"] for a in m["sourceAudit"])
    assert len(m["runtimeComparisons"]) >= 2
    assert m["decision"]["routeInjectionLayerBug"] is True
    text = REPORT.read_text(encoding="utf-8")
    for code in EXPECTED:
        assert "0x" + code in text
    print("pass336 verifier ok")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
