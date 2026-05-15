#!/usr/bin/env python3
from __future__ import annotations
import json, os, subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding"
OUT = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED_COMMON = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

SOURCE_LOCKS = [
    {"id":"pc34_irq09_installs_raw_keyboard_isr","file":"IBMIO.C","lines":"381-390,399-414","function":"F8088_ / S8A_Vector","needles":["mov     ax, 3509h","mov     dx, offset S8A_Vector","mov     ax, 2509h","asm     S8A_Vector:","in      al, 60h"],"claim":"PC34 IO startup saves INT 09 and installs S8A_Vector as the raw keyboard interrupt handler."},
    {"id":"pc34_irq09_buffers_scancodes_and_acks_pic","file":"IBMIO.C","lines":"527-529,581-611,623-623","function":"S8A_Vector","needles":["F8087_(G8085_ | (G8062_ & 0x1EFF));","out     61h, al","out     20h, al","iret","jmp     cs:V8088000_"],"claim":"The raw INT 09 handler queues accepted make scancodes, acknowledges the keyboard/PIC path, or chains to the saved vector."},
    {"id":"pc34_irq09_restore","file":"IBMIO.C","lines":"629-643","function":"F8089_","needles":["void F8089_","mov     dx, word ptr cs:V8088000_+2","mov     ax, 2509h","int     21h"],"claim":"PC34 IO shutdown restores the saved INT 09 vector."},
    {"id":"io_driver_slot0_slot1_are_keyboard_runtime_api","file":"IBMIO.C","lines":"646-680,2378-2381","function":"F8090_ / F8091_ / G8101_apc_IOInterruptVector","needles":["int16_t F8090_","while (!G8082_);","L2662_i_ = G8083_[G8081_];","void F8091_","(char*)F8090_, /*  0 */","(char*)F8091_, /*  1 */"],"claim":"The game-facing keyboard API is IO_DRIVER slot 0/1, not the INT 09 vector itself."},
    {"id":"io_driver_table_is_published_on_c254","file":"IBMIO.C","lines":"2550-2552,2582-2586","function":"main IO driver loop","needles":["setvect(C254_DM_IO_INTERRUPT, (void interrupt (*)())&G8101_apc_IOInterruptVector);","F8088_();","F8118_();","F8089_();"],"claim":"The IO driver publishes the G8101 table through C254 before child game execution and removes hooks after return."},
    {"id":"game_binds_g2162_from_c254","file":"CEDT026.C","lines":"217-220,238-238","function":"F0539_INPUT_Cconis / startup","needles":["return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();","G2162_IODriver = G2161_IODriver = (IO_DRIVER*)getvect(C254_DM_IO_INTERRUPT);"],"claim":"The game-side keyboard-present path is driven from G2162_IODriver loaded from C254."},
    {"id":"i34e_crawcin_reads_slot0_then_normalizes_arrows","file":"IO2.C","lines":"27-61,179-184","function":"F0540_INPUT_Crawcin / F0539_INPUT_Cconis","needles":["L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();","switch (L2944_ui_ - 0x1248)","L2944_ui_ = 'L'","L2944_ui_ = 'P'","L2944_ui_ = 'K'","L2944_ui_ = 'M'","return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();"],"claim":"I34E movement input enters via IO_DRIVER slot 0/1 and only then becomes the K/L/M/P movement-table codes."},
    {"id":"main_loop_drains_m527_m528_to_f0361_before_f0380","file":"GAMELOOP.C","lines":"164-219","function":"F0002_MAIN_GameLoop_CPSDF","needles":["while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();"],"claim":"The game loop consumes the bound IO_DRIVER keyboard API before command-queue dispatch."},
    {"id":"f0361_f0380_enqueue_dequeue_contract","file":"COMMAND.C","lines":"1734-1768,2075-2127,2150-2156","function":"F0361_COMMAND_ProcessKeyPress / F0380_COMMAND_ProcessQueue_CPSC","needles":["G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;","G2153_i_QueuedCommandsCount++;","if (G2153_i_QueuedCommandsCount == 0)","G2153_i_QueuedCommandsCount--;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"claim":"Runtime movement promotion requires F0361 enqueue/count increment and F0380 pop/count decrement/dispatch."},
]

def sh(cmd):
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()
def compact(text):
    return " ".join(text.split())
def source_window(path, ranges):
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out, nums = [], []
    for part in ranges.split(","):
        start, end = [int(value) for value in part.split("-", 1)]
        for line_no in range(start, end + 1):
            if 1 <= line_no <= len(lines):
                out.append(lines[line_no - 1]); nums.append(line_no)
    return "\n".join(out), nums
def audit_source():
    rows = []
    for lock in SOURCE_LOCKS:
        path = RED_COMMON / lock["file"]
        text, nums = source_window(path, lock["lines"]) if path.exists() else ("", [])
        missing = [needle for needle in lock["needles"] if compact(needle) not in compact(text)]
        row = {k: v for k, v in lock.items() if k != "needles"}
        row.update({"path": str(path), "ok": path.exists() and not missing, "missing": missing, "lineRangeObserved": [min(nums), max(nums)] if nums else None})
        rows.append(row)
    return rows
def load_runtime_binding():
    path_text = os.environ.get("FIRESTAFF_PASS560_RUNTIME_BINDING")
    if not path_text:
        return {"provided": False, "ok": False, "status": "not_provided"}
    path = Path(path_text)
    if not path.is_absolute():
        path = ROOT / path
    if not path.exists():
        return {"provided": True, "ok": False, "status": "missing", "path": str(path)}
    payload = json.loads(path.read_text(encoding="utf-8"))
    slots = payload.get("ioDriverSlots", {})
    checks = {"int09HandlerIsS8A": payload.get("int09HandlerSymbol") == "S8A_Vector","c254VectorIsG8101": payload.get("c254VectorSymbol") == "G8101_apc_IOInterruptVector","slot0IsF8090": slots.get("0") == "F8090_" or slots.get("IODRV_00_GetKeyboardInput") == "F8090_","slot1IsF8091": slots.get("1") == "F8091_" or slots.get("IODRV_01_IsKeyboardInputPresent") == "F8091_","m528ValueObserved": payload.get("m528Value") in ["0x004B","0x004C","0x004D","0x004F","0x0050","0x0051",0x4B,0x4C,0x4D,0x4F,0x50,0x51],"f0361EnqueueObserved": bool(payload.get("f0361EnqueueObserved")),"f0380PopObserved": bool(payload.get("f0380PopObserved"))}
    return {"provided": True, "ok": all(checks.values()), "status": "loaded", "path": str(path), "checks": checks}
def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = audit_source(); runtime = load_runtime_binding(); source_ok = all(row["ok"] for row in source)
    if not source_ok:
        status = "FAIL_PASS560_REDMCSB_SOURCE_AUDIT_FAILED"; blocker = "ReDMCSB source audit failed; do not interpret runtime binding until missing anchors are resolved."
    elif runtime["ok"]:
        status = "PASS560_DM1_V1_PC34_KEYBOARD_INTERRUPT_RUNTIME_BINDING_PROMOTABLE"; blocker = None
    elif runtime["provided"]:
        status = "BLOCKED_PASS560_RUNTIME_BINDING_INCOMPLETE"; blocker = "Runtime binding artifact exists but does not prove INT09=S8A_Vector, C254=G8101_apc_IOInterruptVector, IO_DRIVER slot0=F8090_, slot1=F8091_, M528 movement value, F0361 enqueue, and F0380 pop."
    else:
        status = "BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED"; blocker = "Pass559 null INT09-handler decode is not the game-facing movement blocker by itself: ReDMCSB routes raw IRQ09 through S8A_Vector, then exposes keyboard input to DM.EXE through C254 -> G8101_apc_IOInterruptVector slot0/slot1. Next capture must decode C254 and slots 0/1 before checking M528/F0361/F0380."
    manifest = {"schema": f"{PASS}.v1", "status": status, "repo": str(ROOT), "branch": sh(["git","branch","--show-current"]), "sourceRoot": str(RED_COMMON), "sourceAudit": source, "runtimeBinding": runtime, "blocker": blocker, "nextRequiredRuntimeProbe": ["read INT 09 and verify S8A_Vector after F8088_", "read C254 and decode G8101_apc_IOInterruptVector", "decode IO_DRIVER slot 0 as F8090_ and slot 1 as F8091_", "same bounded run: M527 true, M528 movement key, F0361 enqueue/G2153 increment, F0380 pop/G2153 decrement/dispatch"]}
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass560 - DM1 V1 PC34 keyboard interrupt/runtime binding", "", f"- Status: {status}", f"- Manifest: {MANIFEST.relative_to(ROOT)}", "", "## ReDMCSB anchors"]
    for row in source:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']} {row['function']} lines {row['lines']} - {row['claim']}")
    lines.extend(["", "## Decision", "", blocker or "Runtime binding is promotable with supplied artifact."])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(status); return 0 if source_ok else 1
if __name__ == "__main__":
    raise SystemExit(main())
