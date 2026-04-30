#!/usr/bin/env python3
"""Verify the DM1 language/version source-contract boundary for V1 evidence.

This is a static evidence gate. It intentionally does not mutate runtime code:
it records the original ReDMCSB PC launcher language contract and checks that
Firestaff current launcher language/version UI remains a documented superset,
not an original-faithful DM1 V1 language parity claim.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_REPO = Path(__file__).resolve().parents[1]

SOURCE_CITATIONS = [
    {"file": "DM.C", "lines": "11-18", "function_or_symbol": "C3_LANGUAGE_CHOICE_COUNT / C1_VGA_GRAPHICS..C3_TANDY_GRAPHICS", "claim": "PC media branch defines exactly 3 language choices and 3 video choices."},
    {"file": "DM.C", "lines": "643-646", "function_or_symbol": "G8026_Language", "claim": "Language menu entries are English, German, French with source indices C1/C2/C3."},
    {"file": "DM.C", "lines": "650-687", "function_or_symbol": "F8086_GetMenuChoice", "claim": "Menu input returns the selected entry Index after Allowed validation."},
    {"file": "DM.C", "lines": "891-897", "function_or_symbol": "main", "claim": "When menu choices are requested, language is selected before video/audio/input."},
    {"file": "DM.C", "lines": "1018-1027", "function_or_symbol": "main", "claim": "The selected language maps only to -*e, -*g, or -*f command-line parameters."},
]

EXPECTED_REDMCSB_LANGUAGE_CHOICES = [
    {"label": "English", "allowed": "C1_TRUE", "index": "C1_ENGLISH", "parameter": "-*e"},
    {"label": "German", "allowed": "C1_TRUE", "index": "C2_GERMAN", "parameter": "-*g"},
    {"label": "French", "allowed": "C1_TRUE", "index": "C3_FRENCH", "parameter": "-*f"},
]
EXPECTED_FIRESTAFF_LANGUAGE_CODES = ["EN", "SV", "FR", "DE"]
EXPECTED_FIRESTAFF_LANGUAGE_NAMES = ["ENGLISH", "SVENSKA", "FRANCAIS", "DEUTSCH"]


def source_slice(path: Path, line_range: str) -> str:
    start, end = [int(part) for part in line_range.split("-")]
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def extract_c_string_array(text: str, symbol: str) -> list[str]:
    pattern = rf"static const char\* g_{symbol}\[\] = \{{([^}}]+)\}};"
    match = re.search(pattern, text, flags=re.MULTILINE | re.DOTALL)
    if not match:
        raise ValueError(f"could not find g_{symbol} array")
    return re.findall(r'"([^"]*)"', match.group(1))


def require_all(haystack: str, needles: list[str], label: str) -> None:
    missing = [needle for needle in needles if needle not in haystack]
    if missing:
        raise SystemExit(f"{label} missing expected source text: {missing!r}")


def verify(redmcsb_source: Path, repo: Path) -> dict[str, object]:
    dm_c = redmcsb_source / "DM.C"
    menu_c = repo / "menu_startup_m12.c"
    config_c = repo / "config_m12.c"
    config_h = repo / "config_m12.h"
    for path in (dm_c, menu_c, config_c, config_h):
        if not path.exists():
            raise SystemExit(f"missing required file: {path}")

    dm_text = dm_c.read_text(errors="replace")
    require_all(source_slice(dm_c, "11-18"), ["#define C3_LANGUAGE_CHOICE_COUNT 3", "#define C1_VGA_GRAPHICS   1", "#define C2_EGA_GRAPHICS   2", "#define C3_TANDY_GRAPHICS 3"], "ReDMCSB DM.C:11-18")
    require_all(source_slice(dm_c, "643-646"), ['{ "English", C1_TRUE, C1_ENGLISH }', '{ "German",  C1_TRUE, C2_GERMAN  }', '{ "French",  C1_TRUE, C3_FRENCH  }'], "ReDMCSB DM.C:643-646")
    require_all(source_slice(dm_c, "650-687"), ["int16_t F8086_GetMenuChoice", "Please select from '*'ed options", "if (!P2743_ps_MenuChoice[L2381_i_SelectedChoice - 1].Allowed)", "return P2743_ps_MenuChoice[L2381_i_SelectedChoice - 1].Index;"], "ReDMCSB DM.C:650-687")
    require_all(source_slice(dm_c, "891-897"), ["G8019_LanguageChoice = F8086_GetMenuChoice(G8026_Language, C3_LANGUAGE_CHOICE_COUNT);", "G8016_VideoChoice = F8086_GetMenuChoice(G8023_Video, C3_VIDEO_CHOICE_COUNT);"], "ReDMCSB DM.C:891-897")
    require_all(source_slice(dm_c, "1018-1027"), ["case C1_ENGLISH:", 'F8080_AddCommandLineParameter("-*e");', "case C2_GERMAN:", 'F8080_AddCommandLineParameter("-*g");', "case C3_FRENCH:", 'F8080_AddCommandLineParameter("-*f");'], "ReDMCSB DM.C:1018-1027")

    menu_text = menu_c.read_text(errors="replace")
    config_text = config_c.read_text(errors="replace")
    config_h_text = config_h.read_text(errors="replace")
    firestaff_codes = extract_c_string_array(menu_text, "languages")
    firestaff_names = extract_c_string_array(menu_text, "languageNames")
    if firestaff_codes != EXPECTED_FIRESTAFF_LANGUAGE_CODES:
        raise SystemExit(f"Firestaff g_languages drift: expected {EXPECTED_FIRESTAFF_LANGUAGE_CODES}, got {firestaff_codes}")
    if firestaff_names != EXPECTED_FIRESTAFF_LANGUAGE_NAMES:
        raise SystemExit(f"Firestaff g_languageNames drift: expected {EXPECTED_FIRESTAFF_LANGUAGE_NAMES}, got {firestaff_names}")
    require_all(config_h_text, ["int gameVersionIndex[M12_CONFIG_GAME_COUNT];", "int gameLanguageIndex[M12_CONFIG_GAME_COUNT];"], "Firestaff config_m12.h per-game persisted fields")
    require_all(config_text, ['m12_string_equals(field, "version_index")', 'm12_string_equals(field, "language_index")', 'fprintf(fp, "game_%d_version_index = %d\\n", gi, config->gameVersionIndex[gi]);', 'fprintf(fp, "game_%d_language_index = %d\\n", gi, config->gameLanguageIndex[gi]);'], "Firestaff config_m12.c per-game parse/save fields")
    if "MEDIA733_I34M" not in dm_text:
        raise SystemExit("ReDMCSB DM.C missing MEDIA733_I34M language guard")

    redmcsb_labels = [row["label"].upper() for row in EXPECTED_REDMCSB_LANGUAGE_CHOICES]
    boundary_extra = [name for name in firestaff_names if name not in redmcsb_labels]
    boundary_missing_exact_name = [name for name in redmcsb_labels if name not in firestaff_names]

    return {
        "gate": "dm1-language-version-source-contract",
        "status": "PASS",
        "redmcsb_source_root": str(redmcsb_source),
        "repo": str(repo),
        "redmcsb_citations": SOURCE_CITATIONS,
        "redmcsb_language_choices": EXPECTED_REDMCSB_LANGUAGE_CHOICES,
        "firestaff_launcher_language_codes": firestaff_codes,
        "firestaff_launcher_language_names": firestaff_names,
        "firestaff_per_game_fields_present": ["gameVersionIndex", "gameLanguageIndex"],
        "boundary": {
            "source_contract": "DM1 PC I34M language source path is exactly English/German/French and emits -*e/-*g/-*f.",
            "firestaff_current_ui": "Firestaff persists per-game version/language options and exposes EN/SV/FR/DE launcher labels.",
            "original_label_overlap": [name for name in firestaff_names if name in redmcsb_labels],
            "firestaff_extra_language_labels": boundary_extra,
            "redmcsb_labels_not_exactly_named_in_firestaff": boundary_missing_exact_name,
            "parity_claimed": False,
            "non_claim": "This gate documents a source/version boundary only; it does not claim Firestaff language selection is original-faithful DM1 V1 parity.",
        },
    }


def write_markdown(result: dict[str, object], path: Path) -> None:
    boundary = result["boundary"]  # type: ignore[index]
    lines = [
        "# DM1 V1 language/version source-contract boundary",
        "",
        "This static evidence gate records the original ReDMCSB PC launcher language contract and keeps Firestaff current launcher language/version UI classified as a boundary, not a parity claim.",
        "",
        "## ReDMCSB citations audited first",
        "",
    ]
    for cite in result["redmcsb_citations"]:  # type: ignore[index]
        lines.append(f"- `{cite['file']}:{cite['lines']}` `{cite['function_or_symbol']}` - {cite['claim']}")
    lines += [
        "",
        "## Gate result",
        "",
        f"- Status: `{result['status']}`",
        f"- Source contract: {boundary['source_contract']}",
        f"- Firestaff current UI: {boundary['firestaff_current_ui']}",
        f"- Firestaff extra language labels vs ReDMCSB DM1 source: `{', '.join(boundary['firestaff_extra_language_labels']) or 'none'}`",
        f"- Parity claimed: `{boundary['parity_claimed']}`",
        f"- Non-claim: {boundary['non_claim']}",
        "",
        "## Next step",
        "",
        "If DM1 V1 language selection becomes runtime-significant, add a separate implementation gate that maps only the ReDMCSB English/German/French source triad to the original data/version path before claiming parity.",
    ]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--repo", type=Path, default=DEFAULT_REPO)
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    args = parser.parse_args()

    result = verify(args.redmcsb_source.resolve(), args.repo.resolve())
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2) + "\n")
    if args.markdown_out:
        write_markdown(result, args.markdown_out)
    print("PASS dm1-language-version-source-contract")
    print("redmcsb=DM.C:11-18,643-646,650-687,891-897,1018-1027")
    print("boundary=Firestaff EN/SV/FR/DE per-game language UI is not claimed as original-faithful DM1 V1 language parity")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
