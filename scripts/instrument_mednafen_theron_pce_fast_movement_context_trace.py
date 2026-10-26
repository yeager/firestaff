#!/usr/bin/env python3
"""Add a bounded PCE Fast movement-context trace to an isolated build.

This is an observation pass.  It deliberately does not reuse the PCE
interpreter's command-PC gate: PCE Fast RdOp() fetches directly from FastPageR,
while RdMem() is data-only. Authentic JP dungeon states must establish their
own PC/MPR context first. The observed latch and runtime-state writes are
opaque evidence only; they do not assign gameplay field semantics.
"""

from pathlib import Path
import sys


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise SystemExit(f"FAIL: expected one PCE Fast movement-context anchor in {path}")
    path.write_text(text.replace(old, new), encoding="utf-8")


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: instrument_mednafen_theron_pce_fast_movement_context_trace.py SOURCE")
    huc = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
    if not huc.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")

    helpers = r'''static FILE *TheronPCEFastMovementContextTrace;
static unsigned TheronPCEFastMovementDataReadCount;
static unsigned TheronPCEFastMovementCommandWriteCount;
static unsigned TheronPCEFastInputLatchWriteCount;
static unsigned TheronPCEFastRuntimeStateWriteCount;
static unsigned TheronPCEFastMovementContextLimit(void)
{
 static unsigned limit;
 if(!limit)
 {
  const char *text = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_SAMPLE_LIMIT");
  const unsigned long parsed = text && *text ? strtoul(text, NULL, 10) : 128;
  limit = parsed >= 1 && parsed <= 256 ? (unsigned)parsed : 128;
 }
 return limit;
}
static void TheronPCEFastMovementContext(const char *kind, unsigned int logical_address,
                                         unsigned int physical_address, uint8 value)
{
 const uint16 pc = GetRealPC_EXTERNAL();
 const uint8 mpr = HuCPU.MPR[pc >> 13];
 unsigned *count = &TheronPCEFastMovementDataReadCount;
 if(!strcmp(kind, "movement_command_write"))
  count = &TheronPCEFastMovementCommandWriteCount;
 else if(!strcmp(kind, "input_latch_write"))
  count = &TheronPCEFastInputLatchWriteCount;
 else if(!strcmp(kind, "runtime_state_write"))
  count = &TheronPCEFastRuntimeStateWriteCount;
 if(*count >= TheronPCEFastMovementContextLimit())
  return;
 if(!TheronPCEFastMovementContextTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE");
  if(path && *path)
  {
   TheronPCEFastMovementContextTrace = fopen(path, "w");
   if(TheronPCEFastMovementContextTrace)
    fputs("source=mednafen-pce-fast-theron-movement-context-v1\n", TheronPCEFastMovementContextTrace);
  }
 }
 if(TheronPCEFastMovementContextTrace)
 {
  fprintf(TheronPCEFastMovementContextTrace,
          "%s sequence=%u logical_address=%04x physical_address=%06x value=%02x pc=%04x pc_mpr=%02x physical_pc=%06x\n",
          kind, (*count)++, logical_address, physical_address,
          value, pc, mpr, (mpr << 13) | (pc & 0x1fff));
  fflush(TheronPCEFastMovementContextTrace);
 }
}
'''
    replace_once(huc, "static INLINE uint8 RdMem(unsigned int A)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n uint8 value = HuCPU.PCERead[wmpr](physical_address);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_read\", A, physical_address, value, wmpr);\n TheronTraceMainRAMConsumerRead(A, physical_address, value);\n return(value);\n}\n",
                 helpers + "\nstatic INLINE uint8 RdMem(unsigned int A)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n uint8 value = HuCPU.PCERead[wmpr](physical_address);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_read\", A, physical_address, value, wmpr);\n TheronTraceMainRAMConsumerRead(A, physical_address, value);\n if(physical_address >= 0x0e8a00 && physical_address <= 0x0e8aff)\n  TheronPCEFastMovementContext(\"movement_data_read\", A, physical_address, value);\n return(value);\n}\n")
    replace_once(huc, "static INLINE void WrMem(unsigned int A, uint8 V)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical = (wmpr << 13) | (A & 0x1FFF);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n HuCPU.PCEWrite[wmpr](physical, V);\n}\n",
                 "static INLINE void WrMem(unsigned int A, uint8 V)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical = (wmpr << 13) | (A & 0x1FFF);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n if(A == 0x2905)\n  TheronPCEFastMovementContext(\"movement_command_write\", A, physical, V);\n if(A == 0x2911 || A == 0x2912 || A == 0x2920)\n  TheronPCEFastMovementContext(\"input_latch_write\", A, physical, V);\n if(A == 0x2031 || (A >= 0x203f && A <= 0x2041))\n  TheronPCEFastMovementContext(\"runtime_state_write\", A, physical, V);\n HuCPU.PCEWrite[wmpr](physical, V);\n}\n")


if __name__ == "__main__":
    main()
