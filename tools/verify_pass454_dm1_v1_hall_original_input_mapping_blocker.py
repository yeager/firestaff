#!/usr/bin/env python3
"""Pass454 DM1 V1 Hall original input-mapping blocker.

Diagnoses the fresh-entry/no-turn Hall capture attempts in the external
hall-true-stop artifact set.  The gate intentionally does not promote any
framebuffer parity; it records whether the automation clicked the requested PC
coordinate or an offset coordinate.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass454_dm1_v1_hall_original_input_mapping_blocker"
ARTIFACT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-true-stop-20260509")
ARTIFACT_MANIFEST = ARTIFACT / "manifest.json"
EXTERNAL_OUT_DIR = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-input-timing-state-20260509")
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"
EXTERNAL_JSON = EXTERNAL_OUT_DIR / f"{PASS}.json"
CORRECTED_ARTIFACT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509")

EXPECTED_HASHES = {
    "DUNGEON.DAT_sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT_sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE_sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

FRESH_ATTEMPTS = {
    "probe-pk-fresh-click",
    "probe-pm-fresh-click",
    "probe-known-entry",
    "probe-cancel",
}

CLICK_RE = re.compile(
    r"^(?P<button>left|right)-click-mapped (?P<pcx>\d+),(?P<pcy>\d+) -> "
    r"(?:window-relative )?(?P<px>\d+),(?P<py>\d+) window=(?P<w>\d+)x(?P<h>\d+)(?: origin=(?P<ox>-?\d+),(?P<oy>-?\d+))?"
)
CORRECTED_CLICK_RE = re.compile(
    r"^(?P<button>left|right)-click-mapped (?P<pcx>\d+),(?P<pcy>\d+) -> "
    r"absolute (?P<absx>\d+),(?P<absy>\d+) client-relative (?P<cx>\d+),(?P<cy>\d+) "
    r"window=(?P<w>\d+)x(?P<h>\d+) origin=(?P<ox>-?\d+),(?P<oy>-?\d+)"
)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def expected_window_relative(width: int, height: int, pcx: int, pcy: int) -> tuple[int, int]:
    content_aspect = 320.0 / 200.0
    content_w = float(width)
    content_h = content_w / content_aspect
    if content_h > height:
        content_h = float(height)
        content_w = content_h * content_aspect
    left = (width - content_w) / 2.0
    top = (height - content_h) / 2.0
    px = left + ((pcx + 0.5) / 320.0) * content_w
    py = top + ((pcy + 0.5) / 200.0) * content_h
    return int(round(px)), int(round(py))


def parse_clicks(run_dir: Path) -> list[dict[str, Any]]:
    keylog = run_dir / "original-viewpoint-route-keys.log"
    if not keylog.exists():
        return []
    rows: list[dict[str, Any]] = []
    for line in keylog.read_text(encoding="utf-8", errors="replace").splitlines():
        match = CLICK_RE.match(line.strip())
        if not match:
            continue
        raw = match.groupdict()
        row = {k: (int(v) if v is not None and k not in {"button"} else v) for k, v in raw.items()}
        want_x, want_y = expected_window_relative(row["w"], row["h"], row["pcx"], row["pcy"])
        row["expectedWindowRelative"] = [want_x, want_y]
        row["loggedWindowRelative"] = [row["px"], row["py"]]
        row["deltaFromExpected"] = [row["px"] - want_x, row["py"] - want_y]
        row["matchesExpected"] = abs(row["px"] - want_x) <= 1 and abs(row["py"] - want_y) <= 1
        row["outsideWindow"] = row["px"] < 0 or row["py"] < 0 or row["px"] >= row["w"] or row["py"] >= row["h"]
        if row.get("ox") is None or row.get("oy") is None:
            row["matchesExpectedPlusOrigin"] = None
        else:
            row["matchesExpectedPlusOrigin"] = abs(row["px"] - (want_x + row["ox"])) <= 1 and abs(row["py"] - (want_y + row["oy"])) <= 1
        rows.append(row)
    return rows


def corrected_rerun_summary() -> dict[str, Any]:
    run_dir = CORRECTED_ARTIFACT / "probe-initial-south-corrected"
    keylog = run_dir / "original-viewpoint-route-keys.log"
    images = sorted(run_dir.glob("image*.png"))
    hashes = [sha256(path) for path in images]
    clicks: list[dict[str, Any]] = []
    if keylog.exists():
        for line in keylog.read_text(encoding="utf-8", errors="replace").splitlines():
            match = CORRECTED_CLICK_RE.match(line.strip())
            if not match:
                continue
            row = {k: (int(v) if k != "button" else v) for k, v in match.groupdict().items()}
            want_x, want_y = expected_window_relative(row["w"], row["h"], row["pcx"], row["pcy"])
            row["expectedClientRelative"] = [want_x, want_y]
            row["clientMatchesExpected"] = abs(row["cx"] - want_x) <= 1 and abs(row["cy"] - want_y) <= 1
            row["absoluteMatchesOriginPlusClient"] = abs(row["absx"] - (row["ox"] + row["cx"])) <= 1 and abs(row["absy"] - (row["oy"] + row["cy"])) <= 1
            clicks.append(row)
    candidate_sha = "e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e"
    terminal_sha = "7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f"
    candidate_index = hashes.index(candidate_sha) if candidate_sha in hashes else -1
    terminal_index = hashes.index(terminal_sha) if terminal_sha in hashes else -1
    ok = (
        run_dir.is_dir()
        and len(clicks) >= 2
        and all(c["clientMatchesExpected"] and c["absoluteMatchesOriginPlusClient"] for c in clicks)
        and [[c["pcx"], c["pcy"]] for c in clicks[:2]] == [[111, 82], [130, 115]]
        and candidate_index > 0
        and terminal_index > candidate_index
    )
    return {
        "artifactRoot": str(CORRECTED_ARTIFACT),
        "run": "probe-initial-south-corrected",
        "exists": run_dir.is_dir(),
        "keyLog": str(keylog),
        "clickCount": len(clicks),
        "allClicksClientCorrect": bool(clicks) and all(c["clientMatchesExpected"] for c in clicks),
        "allClicksAbsoluteConsistent": bool(clicks) and all(c["absoluteMatchesOriginPlusClient"] for c in clicks),
        "requestedPcClicks": [[c["pcx"], c["pcy"]] for c in clicks],
        "imageCount": len(hashes),
        "uniqueImageSha256Count": len(set(hashes)),
        "candidateSelectSha256": candidate_sha,
        "terminalHudSha256": terminal_sha,
        "candidateIndex": candidate_index,
        "terminalIndex": terminal_index,
        "ok": ok,
    }


def image_hashes_for_attempt(manifest: dict[str, Any], name: str) -> dict[str, Any]:
    attempt = next((a for a in manifest.get("attempts", []) if a.get("run") == name), None)
    if not attempt:
        return {"run": name, "exists": False}
    hashes = [img.get("sha256") for img in attempt.get("images", []) if img.get("sha256")]
    labels = [img.get("label") for img in attempt.get("images", [])]
    return {
        "run": name,
        "exists": True,
        "status": attempt.get("status"),
        "labels": labels,
        "uniqueImageSha256Count": len(set(hashes)),
        "allImagesIdentical": len(set(hashes)) == 1 if hashes else False,
        "keyLogSha256": attempt.get("keyLogSha256"),
        "config": attempt.get("config"),
    }


def build() -> dict[str, Any]:
    if not ARTIFACT_MANIFEST.exists():
        raise SystemExit(f"missing artifact manifest: {ARTIFACT_MANIFEST}")
    manifest = json.loads(ARTIFACT_MANIFEST.read_text(encoding="utf-8"))
    provenance = manifest.get("provenance", {})
    errors: list[str] = []
    for key, expected in EXPECTED_HASHES.items():
        got = provenance.get(key)
        if got != expected:
            errors.append(f"{key} mismatch: {got} != {expected}")

    attempt_rows: list[dict[str, Any]] = []
    stale_or_mismatched_scripts: list[str] = []
    bad_clicks: list[dict[str, Any]] = []
    for attempt in manifest.get("attempts", []):
        name = attempt.get("run")
        if not name:
            continue
        run_dir = ARTIFACT / name
        clicks = parse_clicks(run_dir)
        script = run_dir / "original_viewport_route_keys_xdotool.sh"
        script_hash = sha256(script) if script.exists() else None
        row = {
            "run": name,
            "status": attempt.get("status"),
            "freshAttempt": name in FRESH_ATTEMPTS,
            "keyLogSha256": attempt.get("keyLogSha256"),
            "scriptSha256": script_hash,
            "clicks": clicks,
            "imageSummary": image_hashes_for_attempt(manifest, name),
        }
        if script.exists() and "mousemove --window" in script.read_text(encoding="utf-8", errors="replace"):
            row["usesWindowRelativeMousemove"] = True
        else:
            row["usesWindowRelativeMousemove"] = False
        for idx, click in enumerate(clicks):
            if not click["matchesExpected"]:
                bad = {"run": name, "clickIndex": idx, **click}
                bad_clicks.append(bad)
                if click.get("matchesExpectedPlusOrigin") or click.get("outsideWindow") or not click["matchesExpected"]:
                    stale_or_mismatched_scripts.append(name)
        attempt_rows.append(row)

    fresh_rows = [r for r in attempt_rows if r["freshAttempt"]]
    fresh_bad = [b for b in bad_clicks if b["run"] in FRESH_ATTEMPTS]
    fresh_static = [r for r in fresh_rows if r["imageSummary"].get("allImagesIdentical")]

    corrected_summary = corrected_rerun_summary()

    if errors:
        status = "FAIL_PASS454_PROVENANCE"
    elif corrected_summary["ok"]:
        status = "PASS_PASS454_STALE_MAPPING_BLOCKER_SUPERSEDED_BY_CORRECTED_RERUN"
    elif fresh_bad:
        status = "BLOCKED_CAPTURE_AUTOMATION_ABSOLUTE_COORDINATES_USED_WITH_WINDOW_RELATIVE_CLICK"
    elif fresh_static:
        status = "BLOCKED_FRESH_ATTEMPTS_STATIC_BUT_CLICK_MAPPING_NOT_EXPLAINED"
    else:
        status = "PASS_PASS454_NO_INPUT_MAPPING_BLOCKER_DETECTED"

    next_action = {
        "name": "rerun_fresh_initial_south_click_with_correct_window_relative_mapping",
        "requirements": [
            "Do not reuse the stale hall-true-stop click logs as candidate evidence.",
            "For a DOSBox window, map PC 320x200 coordinates to client-relative coordinates and use xdotool mousemove --window with only those client-relative coordinates.",
            "Alternatively use absolute/root coordinates without --window, but never add the window origin and also pass --window.",
            "Log both requested PC coordinate and computed client-relative/absolute coordinate before every click.",
            "Capture candidate_select only after the click primitive is proven by a movement/turn control click or debugger-visible C080 dispatch.",
        ],
        "expectedClientRelativeForObservedWindow": {
            "window": "1067x832",
            "pc_111_82": list(expected_window_relative(1067, 832, 111, 82)),
            "pc_130_115": list(expected_window_relative(1067, 832, 130, 115)),
            "pc_159_147": list(expected_window_relative(1067, 832, 159, 147)),
        },
    }

    return {
        "schema": f"{PASS}.v1",
        "timestampUtc": manifest.get("createdUtc"),
        "status": status,
        "repo": str(ROOT),
        "artifactManifest": str(ARTIFACT_MANIFEST),
        "artifactManifestSha256": sha256(ARTIFACT_MANIFEST),
        "pc34Provenance": provenance,
        "expectedPc34Provenance": EXPECTED_HASHES,
        "attemptRows": attempt_rows,
        "badClicks": bad_clicks,
        "freshStaticAttempts": [r["run"] for r in fresh_static],
        "diagnosis": "The original hall-true-stop fresh-entry logs used stale coordinate mapping. The corrected-coordinate rerun now supersedes that blocker when it logs client-relative and absolute/root coordinates separately and produces the expected candidate/terminal frame transition.",
        "correctedRerun": corrected_summary,
        "nextExecutableAction": None if corrected_summary["ok"] else next_action,
        "errors": errors,
    }


def write_report(data: dict[str, Any]) -> None:
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{data['status']}`",
        f"- artifact manifest: `{data['artifactManifest']}`",
        f"- artifact manifest sha256: `{data['artifactManifestSha256']}`",
        "- parity claim: **not made here**; this gate only retires the stale coordinate-space blocker when pass455 evidence exists.",
        "",
        "## Diagnosis",
        "",
        data["diagnosis"],
        "",
        "## Evidence summary",
        "",
        f"- stale fresh attempts with mismapped clicks: `{len(data['badClicks'])}`",
        f"- corrected rerun ok: `{data['correctedRerun']['ok']}`",
        f"- corrected requested PC clicks: `{data['correctedRerun']['requestedPcClicks']}`",
        f"- corrected images: `{data['correctedRerun']['imageCount']}` images / `{data['correctedRerun']['uniqueImageSha256Count']}` unique hashes",
        "- PC34 data provenance remained hash-locked; no filename-only comparison was used.",
        "",
    ]
    if data.get("nextExecutableAction"):
        lines += [
            "## Next executable action",
            "",
            f"- `{data['nextExecutableAction']['name']}`",
        ]
        for req in data["nextExecutableAction"]["requirements"]:
            lines.append(f"- {req}")
        lines += [
            "",
            "## Required client-relative checks for the observed 1067x832 DOSBox window",
        ]
        for key, value in data["nextExecutableAction"]["expectedClientRelativeForObservedWindow"].items():
            lines.append(f"- `{key}`: `{value}`")
        lines.append("")
    else:
        lines += [
            "## Resolution",
            "",
            "The stale hall-true-stop mapping blocker is superseded by the corrected rerun. Candidate framebuffer promotion remains owned by pass455/pass449, not by this diagnostic gate.",
            "",
        ]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    data = build()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    EXTERNAL_OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    EXTERNAL_JSON.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    write_report(data)
    print(f"{data['status']} {PASS}")
    print(f"wrote {OUT_JSON}")
    print(f"wrote {OUT_MD}")
    print(f"wrote {EXTERNAL_JSON}")
    if data["errors"]:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
