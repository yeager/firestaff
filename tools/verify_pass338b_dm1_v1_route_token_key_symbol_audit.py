#!/usr/bin/env python3
"""Verify pass338b route-token/key-symbol audit evidence."""
from pathlib import Path
import re
import sys

EVIDENCE = Path("parity-evidence/pass338b_dm1_v1_route_token_key_symbol_audit.md")
REQUIRED = {
    "status": "BLOCKED_PASS338B_KEYPAD_NUMERIC_TOKENS_ABSENT_USE_ARROW_ROUTE_TOKENS",
    "redmcsb_input_lines": "INPUT.C:531-568",
    "redmcsb_f0361_lines": "COMMAND.C:1709-1813",
    "redmcsb_i34e_lines": "COMMAND.C:677-683",
    "redmcsb_dispatch_lines": "COMMAND.C:2146-2158",
    "parser_plain_lines": "main_loop_m11.c:740-818",
    "parser_key_lines": "main_loop_m11.c:821-865",
    "parser_inject_lines": "main_loop_m11.c:867-918",
    "parser_tokenizer_lines": "main_loop_m11.c:921-943",
    "event_lines": "main_loop_m11.c:1051-1184",
    "gameview_lines": "m11_game_view.c:5227-5250",
    "remap_default_lines": "input_remap_m12.c:23-29",
    "remap_name_lines": "input_remap_m12.c:125-196",
    "plain_tokens": "Supported plain route tokens",
    "key_tokens": "Supported scripted `key:` symbols",
    "absent_tokens": "kp4",
    "numlock_absent": "numlock",
    "final_forward": "forward: `up`",
    "final_left": "turn-left: `left`",
    "final_right": "turn-right: `right`",
    "no_kp": "Do not use `kp5`/`kp4`/`kp6`",
}

def main() -> int:
    errors: list[str] = []
    if not EVIDENCE.exists():
        print(f"FAIL: missing {EVIDENCE}")
        return 1
    text = EVIDENCE.read_text(encoding="utf-8")
    for label, needle in REQUIRED.items():
        if needle not in text:
            errors.append(f"missing {label}: {needle}")
    for token in ["`up`", "`down`", "`left`", "`right`", "`key:kp-enter`", "`key:up`", "`key:left`", "`key:right`"]:
        if token not in text:
            errors.append(f"missing supported token {token}")
    for absent in ["`kp4`", "`kp5`", "`kp6`", "`kp_4`", "`kp-4`", "`numlock`"]:
        if absent not in text:
            errors.append(f"missing absent-token statement {absent}")
    if not re.search(r"Final blocker status: `BLOCKED_PASS338B_[A-Z0-9_]+`", text):
        errors.append("missing final blocker status line")
    if errors:
        print("FAIL pass338b route token/key-symbol audit")
        for error in errors:
            print(f" - {error}")
        return 1
    print("PASS pass338b route token/key-symbol audit")
    print(f"evidence={EVIDENCE}")
    print("status=BLOCKED_PASS338B_KEYPAD_NUMERIC_TOKENS_ABSENT_USE_ARROW_ROUTE_TOKENS")
    print("final_tokens=up,left,right; keypad_numeric_tokens=absent")
    return 0

if __name__ == "__main__":
    sys.exit(main())
