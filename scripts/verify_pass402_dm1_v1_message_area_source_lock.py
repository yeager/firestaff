#!/usr/bin/env python3
"""Pass402 verifier: DM1 V1 message-area source lock.

Audits ReDMCSB TEXT.C first and CSBWin as a secondary lineage check, then
checks Firestaff exposes the same cursor, row expiration, row clearing,
new-row/scroll, width/wrap, linefeed, and initialization contracts.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DEFAULT_CSBWIN = Path("~/.openclaw/data/firestaff-csbwin-source/CSBWin").expanduser()


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def line_of(text: str, needle: str) -> int:
    idx = text.find(needle)
    if idx < 0:
        raise AssertionError(f"missing needle: {needle}")
    return text.count("\n", 0, idx) + 1


def function_span(path: Path, name: str) -> tuple[int, int, str]:
    text = read(path)
    pat = re.compile(rf"^(?:STATICFUNCTION\s+)?(?:void|int|i32)\s+{re.escape(name)}\s*\(", re.M)
    m = pat.search(text)
    if not m:
        raise AssertionError(f"missing function {name} in {path}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name} in {path}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, i) + 1, text[m.start(): i + 1]
    # ReDMCSB contains variant-gated preprocessor blocks whose inactive braces
    # are not balanced as plain C text.  Fall back to the next top-level symbol.
    next_pat = re.compile(r"^(?:STATICFUNCTION\s+)?(?:void|int|i32)\s+[A-Za-z0-9_:]+\s*\(", re.M)
    n = next_pat.search(text, m.start() + 1)
    if n:
        return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, n.start()) + 1, text[m.start(): n.start()]
    return text.count("\n", 0, m.start()) + 1, text.count("\n") + 1, text[m.start():]


def compact(s: str) -> str:
    return " ".join(s.split())


def require(body: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(body):
        raise AssertionError(f"missing {label}: {needle}")


def require_order(body: str, needles: list[str], label: str) -> None:
    flat = compact(body)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing/order {needle}")
        pos = hit


def redmcsb_audit(src: Path) -> list[dict[str, Any]]:
    text_c = src / "TEXT.C"
    draw = src / "DRAWMSGA.C"
    defs = src / "DEFS.H"
    text = read(text_c)
    defs_text = read(defs)
    checks: list[dict[str, Any]] = []

    for name, assertions in {
        "F0042_TEXT_MESSAGEAREA_MoveCursor": [
            "if (P0065_i_Column < 0)",
            "G0359_i_MessageAreaCursorColumn = F0024_MAIN_GetMinimumValue(P0065_i_Column * G2087_C6_TextCharacterWidth, G2092_MessageAreaWidth - G2087_C6_TextCharacterWidth);",
            "if (P0066_i_Row >= M532_MESSAGE_AREA_ROW_COUNT)",
            "G0355_B_ScrollMessageArea = C0_FALSE;",
        ],
        "F0043_TEXT_MESSAGEAREA_ClearAllRows": [
            "F0733_FillZoneByIndex(C015_ZONE_MESSAGE_AREA, C00_COLOR_BLACK);",
            "F0042_TEXT_MESSAGEAREA_MoveCursor(0, M532_MESSAGE_AREA_ROW_COUNT - 1);",
            "G0360_al_MessageAreaRowExpirationTime[L0023_RowIndex] = -1;",
        ],
        "F0044_TEXT_MESSAGEAREA_ClearExpiredRows": [
            "L0027_l_ExpirationTime = G0360_al_MessageAreaRowExpirationTime[L0026_ui_RowIndex];",
            "(L0027_l_ExpirationTime == -1) || (L0027_l_ExpirationTime > G0313_ul_GameTime)",
            "F0732_FillScreenArea(L0028_ai_XYZ, C00_COLOR_BLACK);",
            "G0360_al_MessageAreaRowExpirationTime[L0026_ui_RowIndex] = -1;",
        ],
        "F0045_TEXT_MESSAGEAREA_CreateNewRow": [
            "if (G0358_i_MessageAreaCursorRow == M532_MESSAGE_AREA_ROW_COUNT - 1)",
            "if (G0355_B_ScrollMessageArea)",
            "F0696_UpdateMessageArea();",
            "F0008_MAIN_ClearBytes(M772_CAST_PC(G0356_puc_Bitmap_MessageAreaNewRow), (long)G2091_MessageAreaLineByteCount);",
            "G0355_B_ScrollMessageArea = C1_TRUE;",
            "G0360_al_MessageAreaRowExpirationTime[L0029_ui_RowIndex] = G0360_al_MessageAreaRowExpirationTime[L0029_ui_RowIndex + 1];",
            "G0360_al_MessageAreaRowExpirationTime[M532_MESSAGE_AREA_ROW_COUNT - 1] = -1;",
            "G0358_i_MessageAreaCursorRow++;",
        ],
        "F0046_TEXT_MESSAGEAREA_PrintString": [
            "L0030_i_StringLength = M544_STRLEN(P0068_pc_String);",
            "if (G0355_B_ScrollMessageArea)",
            "F0644_PrintTextAlt(G0356_puc_Bitmap_MessageAreaNewRow, G2092_MessageAreaWidth, G0359_i_MessageAreaCursorColumn",
            "F0053_TEXT_PrintToLogicalScreen",
            "G0360_al_MessageAreaRowExpirationTime[G0358_i_MessageAreaCursorRow] = G0313_ul_GameTime + 70;",
        ],
        "F0047_TEXT_MESSAGEAREA_PrintMessage": [
            "if (G0355_B_ScrollMessageArea)",
            "F0696_UpdateMessageArea();",
            "L3466_i_Width = F0646_GetLargestPrintableSubString(P0070_pc_String, L0033_ac_String, &L0031_CharacterIndex, G2092_MessageAreaWidth - G0359_i_MessageAreaCursorColumn);",
            "if (P0070_pc_String[L0031_CharacterIndex] ==",
            "F0045_TEXT_MESSAGEAREA_CreateNewRow();",
            "G0359_i_MessageAreaCursorColumn = 12;",
            "G0359_i_MessageAreaCursorColumn += L3466_i_Width;",
        ],
        "F0048_TEXT_MESSAGEAREA_PrintCharacter": [
            "F0047_TEXT_MESSAGEAREA_PrintMessage(P0071_i_TextColor",
        ],
        "F0051_TEXT_MESSAGEAREA_PrintLineFeed": [
            "F0047_TEXT_MESSAGEAREA_PrintMessage(C00_COLOR_BLACK",
        ],
        "F0054_TEXT_Initialize": [
            "F0042_TEXT_MESSAGEAREA_MoveCursor(0, M532_MESSAGE_AREA_ROW_COUNT - 1);",
            "G0360_al_MessageAreaRowExpirationTime[L0037_ui_LineIndex] = -1;",
        ],
    }.items():
        s, e, body = function_span(text_c, name)
        for a in assertions:
            require(body, a, f"TEXT.C:{name}")
        checks.append({"source": f"TEXT.C:{s}-{e}", "function": name, "passed": True})

    ds, de, dbody = function_span(draw, "F0696_UpdateMessageArea")
    require_order(dbody, [
        "F0638_GetZone(C015_ZONE_MESSAGE_AREA, L3503_ai_Box);",
        "EGB_partScroll(G4105_WORK, 0, 0, -G2088_C7_TextLineHeight, G4106_EGBPARA);",
        "F0132_VIDEO_Blit(G0356_puc_Bitmap_MessageAreaNewRow",
    ], "DRAWMSGA.C:F0696")
    checks.append({"source": f"DRAWMSGA.C:{ds}-{de}", "function": "F0696_UpdateMessageArea", "passed": True})

    for needle, label in [
        ("#define M532_MESSAGE_AREA_ROW_COUNT                                      4", "row count"),
        ("extern int16_t G2087_C6_TextCharacterWidth;", "character width symbol"),
        ("extern int16_t G2088_C7_TextLineHeight;", "line height symbol"),
        ("extern int16_t G2092_MessageAreaWidth;", "message area width symbol"),
    ]:
        ln = line_of(defs_text, needle)
        checks.append({"source": f"DEFS.H:{ln}", "function": label, "passed": True})
    return checks


def csbwin_audit(src: Path) -> list[dict[str, Any]]:
    code = src / "CSBCode.cpp"
    data = src / "Data.h"
    checks: list[dict[str, Any]] = []
    for name, assertions in {
        "SCROLLING_TEXT::SetPrintPosition": [
            "if (column <   0) column = 0;",
            "if (column >= 53) column = 52;",
            "if (row >=  4) row = 3;",
            "d.PrintColumn = (i16)column;",
        ],
        "SCROLLING_TEXT::RemoveTimedOutText": [
            "D5L = d.TextTimeout[D6W];",
            "if (D5L > d.Time) continue;",
            "FillRectangle(d.LogicalScreenBase",
            "d.TextTimeout[D6W] = -1;",
        ],
        "SCROLLING_TEXT::DiscardText": [
            "for (i=0; i<4; i++) m_printLinesCount[i] = -1;",
            "LOCAL_8.w.x1 = 0;",
            "LOCAL_8.w.x2 = 319;",
            "FillRectangle(d.LogicalScreenBase",
        ],
        "SCROLLING_TEXT::CreateNewTextRow": [
            "if (d.PrintRow < 3)",
            "d.PrintRow++;",
            "m_futureLines.AddText(-1,0,0,\"\",printLinesCount);",
        ],
        "SCROLLING_TEXT::ScrollUp": [
            "ClearMemory((ui8 *)d.newTextLine, 1120);",
            "d.PushTextUp = 1;",
            "d.TextScanlineScrollCount = 0;",
            "d.TextTimeout[i] = d.TextTimeout[i+1];",
            "d.TextTimeout[3] = -1;",
        ],
        "SCROLLING_TEXT::ClockTick": [
            "RemoveTimedOutText();",
            "if (pPiece->m_row == -1)",
            "ScrollUp();",
            "TextOutToScreen(pPiece->m_col*6",
            "d.TextTimeout[row] = d.Time + 70;",
        ],
        "SCROLLING_TEXT::Printf": [
            "m_futureLines.AddText(d.PrintRow, d.PrintColumn, color, text, printLinesCount);",
        ],
        "PrintLines": [
            "lastPrintCol = TextWidth()/6;",
            "if (D0W == 10)",
            "scrollingText.CreateNewTextRow(printLinesCount);",
            "if (d.PrintColumn+D6W > lastPrintCol)",
            "d.PrintColumn = 2;",
            "scrollingText.Printf(color, (char *)LOCAL_56, printLinesCount);",
            "scrollingText.ClockTick();",
        ],
        "PrintLinefeed": [
            "QuePrintLines(0, (char *)d.Byte1830);",
        ],
    }.items():
        s, e, body = function_span(code, name)
        for a in assertions:
            require(body, a, f"CSBCode.cpp:{name}")
        checks.append({"source": f"CSBCode.cpp:{s}-{e}", "function": name, "passed": True})
    data_text = read(data)
    for needle, label in [
        ("#define NUMFUTURELINE 99", "future queue"),
        ("i32       m_printLinesCount[4];", "four message rows"),
        ("void SetPrintPosition(i32 column, i32 row);", "cursor API"),
    ]:
        checks.append({"source": f"Data.h:{line_of(data_text, needle)}", "function": label, "passed": True})
    return checks


def local_audit() -> list[dict[str, Any]]:
    checks = [
        ("dm1_v1_text_message_pc34_compat.h", [
            "DM1_V1_MESSAGE_AREA_ROW_COUNT       4",
            "DM1_V1_TEXT_CHARACTER_WIDTH          6",
            "DM1_V1_TEXT_LINE_HEIGHT             7",
            "DM1_V1_MESSAGE_AREA_WIDTH           320",
            "DM1_V1_MESSAGE_ROW_EXPIRATION_TICKS 70",
            "DM1_V1_MESSAGE_CONTINUATION_INDENT  12",
            "scrollPending",
        ]),
        ("dm1_v1_text_message_pc34_compat.c", [
            "dm1_v1_text_move_cursor",
            "state->cursorColumn = column * DM1_V1_TEXT_CHARACTER_WIDTH;",
            "state->cursorColumn = maxCol;",
            "state->scrollPending = 0;",
            "dm1_v1_text_clear_all_rows",
            "dm1_v1_text_clear_expired_rows",
            "exp != -1 && exp <= state->gameTime",
            "dm1_v1_text_create_new_row",
            "dm1_v1_text_shift_rows_up(state);",
            "state->cursorColumn = DM1_V1_MESSAGE_CONTINUATION_INDENT;",
            "dm1_v1_text_print_linefeed",
        ]),
        ("test_dm1_v1_text_message_pc34_compat.c", [
            "test_cursor_movement",
            "test_clear_all_rows",
            "test_print_message_newline",
            "test_print_message_word_wrap",
            "test_row_expiration",
            "test_multiple_messages_scroll",
        ]),
    ]
    results = []
    for rel, needles in checks:
        text = (ROOT / rel).read_text(encoding="utf-8", errors="replace")
        missing = [n for n in needles if n not in text]
        results.append({"source": rel, "function": "local-contract", "passed": not missing, "missing": missing})
    return results


def run(cmd: list[str], timeout: int = 120) -> dict[str, Any]:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": cmd, "returncode": p.returncode, "passed": p.returncode == 0, "output_tail": p.stdout.splitlines()[-30:]}


def build_cache_source(build_dir: Path) -> str | None:
    cache = build_dir / "CMakeCache.txt"
    if not cache.exists():
        return None
    for line in cache.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("CMAKE_HOME_DIRECTORY:INTERNAL="):
            return line.split("=", 1)[1]
    return None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--redmcsb", type=Path, default=DEFAULT_REDMCSB)
    ap.add_argument("--csbwin", type=Path, default=DEFAULT_CSBWIN)
    ap.add_argument("--build-dir", type=Path, default=Path.home() / ".openclaw/data/firestaff-builds/pass402-text-message-area-gpt")
    ap.add_argument("--run-ctest", action="store_true")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    results = {
        "gate": "pass402_dm1_v1_message_area_source_lock",
        "redmcsb_root": str(args.redmcsb),
        "csbwin_root": str(args.csbwin),
        "redmcsb": redmcsb_audit(args.redmcsb),
        "csbwin": csbwin_audit(args.csbwin),
        "local": local_audit(),
        "runtime": [],
    }
    ok = all(r.get("passed") for group in (results["redmcsb"], results["csbwin"], results["local"]) for r in group)

    if args.run_ctest:
        cached_source = build_cache_source(args.build_dir)
        if cached_source and Path(cached_source).resolve() != ROOT.resolve():
            raise AssertionError(
                f"build dir {args.build_dir} already belongs to {cached_source}; "
                "choose --build-dir or delete the stale build tree"
            )
        args.build_dir.mkdir(parents=True, exist_ok=True)
        cfg = run(["cmake", "-S", str(ROOT), "-B", str(args.build_dir), "-G", "Ninja"], timeout=180)
        results["runtime"].append(cfg)
        if cfg["passed"]:
            bld = run(["cmake", "--build", str(args.build_dir), "--target", "test_dm1_v1_text_message_pc34_compat", "--parallel", "2"], timeout=180)
            results["runtime"].append(bld)
            if bld["passed"]:
                tst = run(["ctest", "--test-dir", str(args.build_dir), "-R", "dm1_v1_text_message_source_lock", "--output-on-failure"], timeout=120)
                results["runtime"].append(tst)
        ok = ok and all(r["passed"] for r in results["runtime"])

    results["passed"] = ok
    if args.json:
        print(json.dumps(results, indent=2, sort_keys=True))
    else:
        for group in ("redmcsb", "csbwin", "local"):
            for r in results[group]:
                print(("PASS" if r.get("passed") else "FAIL"), group, r["source"], r["function"])
                for m in r.get("missing", []):
                    print("  missing:", m)
        for r in results["runtime"]:
            print(("PASS" if r.get("passed") else "FAIL"), "runtime", " ".join(map(str, r["cmd"])))
            for line in r["output_tail"]:
                print(" ", line)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
