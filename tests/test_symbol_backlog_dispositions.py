#!/usr/bin/env python3
"""Exercise symbol_backlog disposition filtering against a temporary fixture."""
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run_backlog(disposition_file: Path, include_disposed: bool = False) -> dict[str, object]:
    command = [
        sys.executable,
        str(ROOT / "tools/symbol_backlog.py"),
        "--game",
        "DM1",
        "--limit",
        "999",
        "--json",
        "--dispositions",
        str(disposition_file),
    ]
    if include_disposed:
        command.append("--include-disposed")
    completed = subprocess.run(command, cwd=ROOT, check=True, text=True, capture_output=True)
    return json.loads(completed.stdout)


def main() -> int:
    with tempfile.TemporaryDirectory() as temp_dir:
        disposition_file = Path(temp_dir) / "dispositions.tsv"
        disposition_file.write_text(
            "reference\tsymbol\tdisposition\towner\tevidence\tnotes\n"
            "ReDMCSB\tF0139_DUNGEON_IsCreatureAllowedOnMap\tSOURCE_NONAPPLICABLE\t"
            "test\tfixture\tcovered by fixture\n",
            encoding="utf-8",
        )
        hidden = run_backlog(disposition_file)
        visible = run_backlog(disposition_file, include_disposed=True)
    hidden_symbols = {item["symbol"] for item in hidden["items"]}
    visible_items = {item["symbol"]: item for item in visible["items"]}
    assert "F0139_DUNGEON_IsCreatureAllowedOnMap" not in hidden_symbols
    item = visible_items["F0139_DUNGEON_IsCreatureAllowedOnMap"]
    assert item["disposition"] == "SOURCE_NONAPPLICABLE"
    assert item["disposition_evidence"] == "fixture"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
