"""Shared, small and dependency-free preferences for Firestaff Studio tools."""
from __future__ import annotations

import json
from pathlib import Path

_PATH = Path.home() / ".firestaff" / "studio_settings.json"


def _load() -> dict:
    try:
        data = json.loads(_PATH.read_text(encoding="utf-8"))
        return data if isinstance(data, dict) else {}
    except (OSError, ValueError):
        return {}


def resolve_language(app_id: str, detected: str, supported: set[str]) -> str:
    """Use a valid explicit choice; otherwise preserve the system language."""
    selected = _load().get("language", {}).get(app_id, "auto")
    return selected if selected in supported else detected


def save_language(app_id: str, language: str | None) -> None:
    """Persist an explicit language or None for the system-language policy."""
    data = _load()
    choices = data.setdefault("language", {})
    if language:
        choices[app_id] = language
    else:
        choices[app_id] = "auto"
    _PATH.parent.mkdir(parents=True, exist_ok=True)
    _PATH.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n",
                     encoding="utf-8")
