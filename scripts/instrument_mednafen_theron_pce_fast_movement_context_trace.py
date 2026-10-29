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
    """Replace one exact source anchor, rejecting missing or ambiguous copies."""
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise SystemExit(f"FAIL: expected one PCE Fast movement-context anchor in {path}")
    path.write_text(text.replace(old, new), encoding="utf-8")


def replace_macro_once(path: Path, name: str, old: str, new: str) -> None:
    """Update one post-patch macro definition without changing its neighbors."""
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    matches = [index for index, line in enumerate(lines) if line.startswith(f"#define {name}")]
    if len(matches) != 1 or old not in lines[matches[0]]:
        raise SystemExit(f"FAIL: expected one PCE Fast {name} macro anchor in {path}")
    lines[matches[0]] = lines[matches[0]].replace(old, new, 1)
    path.write_text("".join(lines), encoding="utf-8")


def main() -> None:
    """Instrument the supplied isolated Mednafen PCE Fast source tree."""
    if len(sys.argv) != 2:
        raise SystemExit("usage: instrument_mednafen_theron_pce_fast_movement_context_trace.py SOURCE")
    huc = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
    if not huc.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")

    helpers = r'''extern unsigned TheronPCEFastScriptFrame;
static FILE *TheronPCEFastMovementContextTrace;
static unsigned TheronPCEFastMovementDataReadCount;
static unsigned TheronPCEFastMovementCommandWriteCount;
static unsigned TheronPCEFastInputLatchWriteCount;
static unsigned TheronPCEFastRuntimeStateWriteCount;
static unsigned TheronPCEFastMainRAMWriteCount;
static unsigned TheronPCEFastMovementStateWriteCount;
static unsigned TheronPCEFastTIIPositionCommitCount;
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
static unsigned TheronPCEFastMovementContextStartFrame(void)
{
 static unsigned start_frame;
 static bool initialized;
 if(!initialized)
 {
  const char *text = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_START_FRAME");
  start_frame = text && *text ? strtoul(text, NULL, 10) : 0;
  initialized = true;
 }
 return start_frame;
}
static bool TheronPCEFastMovementContextMainRAMRangeContains(uint32 physical_address)
{
 static bool initialized;
 static bool enabled;
 static uint32 first;
 static uint32 last;
 if(!initialized)
 {
  const char *text = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE");
  char *end = NULL;
  unsigned long parsed_first;
  unsigned long parsed_last;
  initialized = true;
  if(!text || !*text)
   return false;
  parsed_first = strtoul(text, &end, 16);
  if(end == text || !end || *end != '-')
   return false;
  parsed_last = strtoul(end + 1, &end, 16);
  if(!end || *end || parsed_first > parsed_last || parsed_last > 0x1fffff)
   return false;
  first = (uint32)parsed_first;
  last = (uint32)parsed_last;
  enabled = true;
 }
 return enabled && physical_address >= first && physical_address <= last;
}
static void TheronPCEFastMovementContext(const char *kind, unsigned int logical_address,
                                         unsigned int physical_address, uint8 value)
{
 const uint16 pc = TheronTraceInstructionPC;
 const uint8 mpr = HuCPU.MPR[pc >> 13];
 unsigned *count = &TheronPCEFastMovementDataReadCount;
 if(!strcmp(kind, "movement_command_write"))
  count = &TheronPCEFastMovementCommandWriteCount;
 else if(!strcmp(kind, "input_latch_write"))
  count = &TheronPCEFastInputLatchWriteCount;
 else if(!strcmp(kind, "runtime_state_write"))
  count = &TheronPCEFastRuntimeStateWriteCount;
 else if(!strcmp(kind, "main_ram_write"))
  count = &TheronPCEFastMainRAMWriteCount;
 if(*count >= TheronPCEFastMovementContextLimit())
  return;
 if(TheronPCEFastScriptFrame < TheronPCEFastMovementContextStartFrame())
  return;
 if(!TheronPCEFastMovementContextTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE");
  if(path && *path)
  {
   TheronPCEFastMovementContextTrace = fopen(path, "w");
   if(TheronPCEFastMovementContextTrace)
    fputs("source=mednafen-pce-fast-theron-movement-context-v2\n", TheronPCEFastMovementContextTrace);
  }
 }
 if(TheronPCEFastMovementContextTrace)
 {
  fprintf(TheronPCEFastMovementContextTrace,
          "%s sequence=%u frame=%u logical_address=%04x physical_address=%06x value=%02x pc=%04x pc_mpr=%02x physical_pc=%06x mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x mpr4=%02x mpr5=%02x mpr6=%02x mpr7=%02x\n",
          kind, (*count)++, TheronPCEFastScriptFrame, logical_address,
          physical_address, value, pc, mpr,
          (mpr << 13) | (pc & 0x1fff), HuCPU.MPR[0], HuCPU.MPR[1],
          HuCPU.MPR[2], HuCPU.MPR[3], HuCPU.MPR[4], HuCPU.MPR[5],
          HuCPU.MPR[6], HuCPU.MPR[7]);
  fflush(TheronPCEFastMovementContextTrace);
 }
}
static void TheronPCEFastMovementStateWrite(const char *write_path,
                                               unsigned int logical_address,
                                               unsigned int physical_address,
                                               uint8 value);
static void TheronPCEFastMovementRuntimeStateWrite(uint8 zero_page_address,
                                                   uint8 value)
{
 if(HuCPU.MPR[1] != 0xF8)
  return;
 const unsigned int logical_address = 0x2000u + zero_page_address;
 const unsigned int physical_address = (0xF8u << 13) | zero_page_address;
 if(HuCPU.MPR[1] == 0xF8 &&
    (zero_page_address == 0x40 || zero_page_address == 0x41))
  TheronPCEFastMovementStateWrite("direct-zp", logical_address,
                                     physical_address, value);
 if(TheronPCEFastMovementContextMainRAMRangeContains(physical_address))
  TheronPCEFastMovementContext("main_ram_write", logical_address,
                               physical_address, value);
 if(zero_page_address != 0x31 &&
    (zero_page_address < 0x3F || zero_page_address > 0x41))
  return;
 TheronPCEFastMovementContext("runtime_state_write", logical_address,
                              physical_address, value);
}
static void TheronPCEFastMovementTIIPositionCommit(unsigned int logical_address,
                                                    unsigned int physical_address,
                                                    uint8 value)
{
 if(TheronPCEFastTIIPositionCommitCount >= TheronPCEFastMovementContextLimit() ||
    TheronPCEFastScriptFrame < TheronPCEFastMovementContextStartFrame())
  return;
 if(!TheronPCEFastMovementContextTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE");
  if(path && *path)
  {
   TheronPCEFastMovementContextTrace = fopen(path, "w");
   if(TheronPCEFastMovementContextTrace)
    fputs("source=mednafen-pce-fast-theron-movement-context-v2\n", TheronPCEFastMovementContextTrace);
  }
 }
 if(TheronPCEFastMovementContextTrace)
 {
  const unsigned int source_logical = HuCPU.bmt_src & 0xFFFF;
  const unsigned int source_mpr = HuCPU.MPR[source_logical >> 13];
  const unsigned int source_physical = (source_mpr << 13) | (source_logical & 0x1FFF);
  const uint16 pc = TheronTraceInstructionPC;
  const uint8 pc_mpr = HuCPU.MPR[pc >> 13];
  const unsigned int bmt_remaining = HuCPU.bmt_length;
  fprintf(TheronPCEFastMovementContextTrace,
          "tii_position_commit sequence=%u frame=%u logical_address=%04x physical_address=%06x value=%02x opcode_pc=%04x opcode_mpr=%02x opcode_physical_pc=%06x bmt_remaining=%04x source_logical_address=%04x source_physical_address=%06x source_value=%02x source_mpr=%02x destination_logical_address=%04x destination_physical_address=%06x destination_mpr=%02x mpr1=%02x\n",
          TheronPCEFastTIIPositionCommitCount++, TheronPCEFastScriptFrame,
          logical_address, physical_address, value, pc, pc_mpr,
          (pc_mpr << 13) | (pc & 0x1FFF), bmt_remaining, source_logical,
          source_physical, value, source_mpr,
          HuCPU.bmt_dest & 0xFFFF, physical_address,
          HuCPU.MPR[(HuCPU.bmt_dest & 0xFFFF) >> 13], HuCPU.MPR[1]);
  fflush(TheronPCEFastMovementContextTrace);
 }
}
static void TheronPCEFastMovementStateWrite(const char *write_path,
                                               unsigned int logical_address,
                                               unsigned int physical_address,
                                               uint8 value)
{
 if(TheronPCEFastMovementStateWriteCount >= TheronPCEFastMovementContextLimit() ||
    TheronPCEFastScriptFrame < TheronPCEFastMovementContextStartFrame())
  return;
 if(!TheronPCEFastMovementContextTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_MOVEMENT_CONTEXT_TRACE");
  if(path && *path)
  {
   TheronPCEFastMovementContextTrace = fopen(path, "w");
   if(TheronPCEFastMovementContextTrace)
    fputs("source=mednafen-pce-fast-theron-movement-context-v2\n", TheronPCEFastMovementContextTrace);
  }
 }
 if(TheronPCEFastMovementContextTrace)
 {
  const uint16 pc = TheronTraceInstructionPC;
  uint8 code_bytes[3];
  uint8 code_mpr[3];
  for(unsigned index = 0; index < 3; index++)
  {
   const uint16 address = (uint16)(pc + index);
   const unsigned slot = address >> 13;
   code_mpr[index] = HuCPU.MPR[slot];
   code_bytes[index] = *(const uint8 *)(HuCPU.FastPageR[slot] + address);
  }
  fprintf(TheronPCEFastMovementContextTrace,
          "movement_state_write sequence=%u frame=%u path=%s logical_address=%04x physical_address=%06x value=%02x opcode_pc=%04x opcode_physical_pc=%06x code_bytes=%02x%02x%02x code_mpr=%02x%02x%02x block_move=%u bmt_src=%04x bmt_dest=%04x bmt_length=%04x mpr1=%02x\n",
          TheronPCEFastMovementStateWriteCount++, TheronPCEFastScriptFrame,
          write_path, logical_address, physical_address, value, pc,
          TheronTraceInstructionPhysicalPC, code_bytes[0], code_bytes[1],
          code_bytes[2], code_mpr[0], code_mpr[1], code_mpr[2],
          HuCPU.in_block_move, HuCPU.bmt_src, HuCPU.bmt_dest,
          HuCPU.bmt_length, HuCPU.MPR[1]);
  fflush(TheronPCEFastMovementContextTrace);
 }
}
'''
    replace_once(huc, "static INLINE uint8 RdMem(unsigned int A)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n uint8 value = HuCPU.PCERead[wmpr](physical_address);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_read\", A, physical_address, value, wmpr);\n TheronTraceMainRAMConsumerRead(A, physical_address, value);\n return(value);\n}\n",
                 helpers + "\nstatic INLINE uint8 RdMem(unsigned int A)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n uint8 value = HuCPU.PCERead[wmpr](physical_address);\n if(TheronPointerAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_read\", A, physical_address, value, wmpr);\n TheronTraceMainRAMConsumerRead(A, physical_address, value);\n if(physical_address >= 0x0e8a00 && physical_address <= 0x0e8aff)\n  TheronPCEFastMovementContext(\"movement_data_read\", A, physical_address, value);\n return(value);\n}\n")
    replace_once(huc, "static INLINE void WrMem(unsigned int A, uint8 V)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical = (wmpr << 13) | (A & 0x1FFF);\n if(TheronPointerAddress(A) || TheronPointerCandidateAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n HuCPU.PCEWrite[wmpr](physical, V);\n}\n",
                 "static INLINE void WrMem(unsigned int A, uint8 V)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical = (wmpr << 13) | (A & 0x1FFF);\n if(TheronPointerAddress(A) || TheronPointerCandidateAddress(A) ||\n    TheronPointerCodePC(TheronTraceInstructionPC))\n  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n if(A == 0x2905)\n  TheronPCEFastMovementContext(\"movement_command_write\", A, physical, V);\n if(TheronTraceInstructionPC == 0xC1FC && HuCPU.in_block_move == IBM_TII &&\n    HuCPU.MPR[1] == 0xF8 && A >= 0x2040 && A <= 0x2041)\n  TheronPCEFastMovementTIIPositionCommit(A, physical, V);\n if(A == 0x2911 || A == 0x2912 || A == 0x2920)\n  TheronPCEFastMovementContext(\"input_latch_write\", A, physical, V);\n if(TheronPCEFastMovementContextMainRAMRangeContains(physical))\n  TheronPCEFastMovementContext(\"main_ram_write\", A, physical, V);\n if(A == 0x2031 || (A >= 0x203f && A <= 0x2041))\n  TheronPCEFastMovementContext(\"runtime_state_write\", A, physical, V);\n HuCPU.PCEWrite[wmpr](physical, V);\n}\n")

    replace_once(
        huc,
        " if(A == 0x2905)\n  TheronPCEFastMovementContext(\"movement_command_write\", A, physical, V);\n if(TheronTraceInstructionPC == 0xC1FC",
        " if(A == 0x2905)\n  TheronPCEFastMovementContext(\"movement_command_write\", A, physical, V);\n if(HuCPU.MPR[1] == 0xF8 && (A == 0x2040 || A == 0x2041))\n  TheronPCEFastMovementStateWrite(\"WrMem\", A, physical, V);\n if(TheronTraceInstructionPC == 0xC1FC",
    )

    direct_write_hook = "TheronPCEFastMovementRuntimeStateWrite(EA, {value}); "
    direct_macros = {
        "RMW_ZP(op)": ("op; if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, old, x);",
                       "op; " + direct_write_hook.format(value="x") + "if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, old, x);"),
        "RMW_ZPX(op)": ("op; if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, old, x);",
                        "op; " + direct_write_hook.format(value="x") + "if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, old, x);"),
        "ST_ZP(r)": ("if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);",
                     direct_write_hook.format(value="r") + "if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);"),
        "ST_ZPX(r)": ("if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);",
                      direct_write_hook.format(value="r") + "if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);"),
        "ST_ZPY(r)": ("if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);",
                      direct_write_hook.format(value="r") + "if(HuCPU.MPR[1] == 0xF8) TheronTraceDirectMainRAMWrite(EA, HU_Page1[EA], r);"),
    }
    for macro_name, (old, new) in direct_macros.items():
        replace_macro_once(huc, macro_name, old, new)


if __name__ == "__main__":
    main()
