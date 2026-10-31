#!/usr/bin/env python3
"""Add opt-in mouse and guest-write tracing to a disposable Mednafen tree."""

from pathlib import Path
import sys


def replace_once(root: Path, relative_path: str, old: str, new: str) -> None:
    path = root / relative_path
    source = path.read_text(encoding="utf-8")
    occurrences = source.count(old)
    if occurrences != 1:
        raise SystemExit(
            f"FAIL: expected one source anchor in {relative_path}, found {occurrences}"
        )
    path.write_text(source.replace(old, new, 1), encoding="utf-8")


def mouse_input_parser_source() -> str:
    """Return the shared bounded mouse-script parser inserted into both PCE cores."""
    return r'''static char TheronMouseInputScriptBuffer[8192];
static char *TheronMouseInputScriptCursor;
static bool TheronMouseInputScriptInitialized;
static unsigned TheronMouseInputUpdateCount;
static unsigned TheronMouseInputScriptUpdateCount;
static unsigned TheronMouseInputRepeatRemaining;
static int16 TheronMouseInputRepeatDeltaX, TheronMouseInputRepeatDeltaY;
static uint8 TheronMouseInputRepeatButtons;
static bool TheronParseMouseNumber(const char **cursor, bool allow_negative,
                                   int *result)
{
 int sign = 1;
 if(**cursor == '-')
 {
  if(!allow_negative) return false;
  sign = -1;
  (*cursor)++;
 }
 if(**cursor < '0' || **cursor > '9') return false;
 int value = 0;
 do
 {
  if(value > 10000) return false;
  value = value * 10 + (**cursor - '0');
  (*cursor)++;
 }
 while(**cursor >= '0' && **cursor <= '9');
 *result = sign * value;
 return true;
}
static bool TheronNextMouseInput(int16 *delta_x, int16 *delta_y,
                                 uint8 *buttons)
{
 TheronMouseInputUpdateCount++;
 if(!TheronMouseInputScriptInitialized)
 {
  const char *value = getenv("FIRESTAFF_THERON_MOUSE_INPUT_SCRIPT");
  TheronMouseInputScriptInitialized = true;
  if(value && strlen(value) < sizeof(TheronMouseInputScriptBuffer))
  {
   strcpy(TheronMouseInputScriptBuffer, value);
   TheronMouseInputScriptCursor = TheronMouseInputScriptBuffer;
  }
 }
 if(TheronMouseInputRepeatRemaining)
 {
  TheronMouseInputRepeatRemaining--;
  TheronMouseInputScriptUpdateCount++;
  *delta_x = TheronMouseInputRepeatDeltaX;
  *delta_y = TheronMouseInputRepeatDeltaY;
  *buttons = TheronMouseInputRepeatButtons;
  return true;
 }
 if(!TheronMouseInputScriptCursor || !*TheronMouseInputScriptCursor ||
    TheronMouseInputScriptUpdateCount >= 20000)
  return false;
 char *token = TheronMouseInputScriptCursor;
 char *separator = strchr(token, ';');
 if(separator)
 {
  *separator = 0;
  TheronMouseInputScriptCursor = separator + 1;
 }
 else TheronMouseInputScriptCursor = NULL;
 char *repeat_marker = strchr(token, '*');
 unsigned repeat = 1;
 if(repeat_marker)
 {
  *repeat_marker = 0;
  const char *repeat_text = repeat_marker + 1;
  if(!*repeat_text) return false;
  repeat = 0;
  for(const char *digit = repeat_text; *digit; digit++)
  {
   if(*digit < '0' || *digit > '9' || repeat > 2000) return false;
   repeat = repeat * 10 + (unsigned)(*digit - '0');
  }
  if(!repeat || repeat > 20000) return false;
 }
 if(repeat > 20000 - TheronMouseInputScriptUpdateCount) return false;
 const char *cursor = token;
 int dx, dy, button;
 if(!TheronParseMouseNumber(&cursor, true, &dx) || *cursor++ != ',' ||
    !TheronParseMouseNumber(&cursor, true, &dy) || *cursor++ != ',' ||
    !TheronParseMouseNumber(&cursor, false, &button) || *cursor ||
    dx < -127 || dx > 127 || dy < -127 || dy > 127 ||
    button < 0 || button > 15)
  return false;
 TheronMouseInputScriptUpdateCount++;
 TheronMouseInputRepeatRemaining = repeat - 1;
 TheronMouseInputRepeatDeltaX = (int16)dx;
 TheronMouseInputRepeatDeltaY = (int16)dy;
 TheronMouseInputRepeatButtons = (uint8)button;
 *delta_x = (int16)dx;
 *delta_y = (int16)dy;
 *buttons = (uint8)button;
 return true;
}
'''


def pointer_c1df_trace_gate_source() -> str:
    """Return the opt-in C1DF trace gate shared with the capture flag."""
    return r'''static bool TheronPointerC1DFTraceEnabledAttempted;
static bool TheronPointerC1DFTraceEnabled;
static bool TheronIsPointerC1DFTraceEnabled(void)
{
 if(!TheronPointerC1DFTraceEnabledAttempted)
 {
  const char *value = getenv("FIRESTAFF_THERON_POINTER_C1DF_TRACE");
  TheronPointerC1DFTraceEnabled = value && value[0] == '1' && value[1] == 0;
  TheronPointerC1DFTraceEnabledAttempted = true;
 }
 return TheronPointerC1DFTraceEnabled;
}
'''


def instrument(root: Path) -> None:
    replace_once(
        root,
        "src/pce/huc6280.h",
        "namespace MDFN_IEN_PCE\n{\n",
        "namespace MDFN_IEN_PCE\n{\n"
        "void TheronTraceMouseGuestRead(unsigned int logical_address,\n"
        "                               unsigned int physical_address,\n"
        "                               unsigned char value, unsigned char mpr,\n"
        "                               unsigned int pc);\n"
        "void TheronTraceMouseGuestWrite(unsigned int address, unsigned char value,\n"
        "                                unsigned char mpr, unsigned int pc);\n",
    )
    replace_once(
        root,
        "src/pce/huc6280.h",
        "\t const uint8 reader_mpr = MPR[reader_pc >> 13];\n",
        "\t const uint8 reader_mpr = MPR[reader_pc >> 13];\n"
        "\t if((address >= 0x77DC && address <= 0x77DD) ||\n"
        "\t    (address >= 0x00C5 && address <= 0x00CA) ||\n"
        "\t    address == 0x2911 || address == 0x2E1C ||\n"
        "\t    address == 0x2E1D || address == 0x2E3A ||\n"
        "\t    address == 0x2E3B || address == 0x2E7A ||\n"
        "\t    address == 0x2E7B || address == 0x2EBA ||\n"
        "\t    address == 0x2EBB)\n"
        "\t  TheronTraceMouseGuestRead(address, physical_address, ret,\n"
        "\t                            wmpr, reader_pc);\n",
    )
    replace_once(
        root,
        "src/pce/huc6280.h",
        "\t LastLogicalWriteAddr = address;\n",
        "\t LastLogicalWriteAddr = address;\n"
        "\t if(address == 0x2911 || (address >= 0x2E00 && address <= 0x2EFF) ||\n"
        "\t    address == 0xCCDB)\n"
        "\t  TheronTraceMouseGuestWrite(address, V, wmpr, PC);\n",
    )
    replace_once(
        root,
        "src/pce/huc6280.cpp",
        "static unsigned Theron3879TargetStepsRemaining;\n",
        "static FILE *TheronMouseGuestReadTrace;\n"
        "static bool TheronMouseGuestReadTraceAttempted;\n"
        "static unsigned TheronMouseGuestReadTraceCount;\n"
        "\n"
        "void TheronTraceMouseGuestRead(unsigned int logical_address,\n"
        "                               unsigned int physical_address,\n"
        "                               unsigned char value, unsigned char mpr,\n"
        "                               unsigned int pc)\n"
        "{\n"
        " static unsigned int sequence = 0;\n"
        " if(!TheronMouseGuestReadTrace)\n"
        " {\n"
        "  if(TheronMouseGuestReadTraceAttempted) return;\n"
        "  TheronMouseGuestReadTraceAttempted = true;\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_MOUSE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  TheronMouseGuestReadTrace = fopen(path, \"a\");\n"
        " }\n"
        " if(!TheronMouseGuestReadTrace || TheronMouseGuestReadTraceCount >= 20000) return;\n"
        " fprintf(TheronMouseGuestReadTrace,\n"
        "         \"guest_read sequence=%u logical=%04x physical=%06x mpr=%02x value=%02x pc=%04x\\n\",\n"
        "         sequence++, logical_address & 0xFFFF, physical_address & 0xFFFFFF,\n"
        "         mpr, value, pc & 0xFFFF);\n"
        " fflush(TheronMouseGuestReadTrace);\n"
        " TheronMouseGuestReadTraceCount++;\n"
        "}\n"
        "\n"
        "static FILE *TheronMouseGuestWriteTrace;\n"
        "static bool TheronMouseGuestWriteTraceAttempted;\n"
        "static unsigned TheronMouseGuestWriteTraceCount;\n"
        "\n"
        "void TheronTraceMouseGuestWrite(unsigned int address, unsigned char value,\n"
        "                                unsigned char mpr, unsigned int pc)\n"
        "{\n"
        " static unsigned int sequence = 0;\n"
        " if(!TheronMouseGuestWriteTrace)\n"
        " {\n"
        "  if(TheronMouseGuestWriteTraceAttempted) return;\n"
        "  TheronMouseGuestWriteTraceAttempted = true;\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_MOUSE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  TheronMouseGuestWriteTrace = fopen(path, \"a\");\n"
        " }\n"
        " if(!TheronMouseGuestWriteTrace || TheronMouseGuestWriteTraceCount >= 20000) return;\n"
        " fprintf(TheronMouseGuestWriteTrace,\n"
        "         \"guest_write sequence=%u logical=%04x physical=%06x mpr=%02x value=%02x pc=%04x\\n\",\n"
        "         sequence++, address & 0xFFFF, ((unsigned int)mpr << 13) | (address & 0x1FFF),\n"
        "         mpr, value, pc & 0xFFFF);\n"
        " fflush(TheronMouseGuestWriteTrace);\n"
        " TheronMouseGuestWriteTraceCount++;\n"
        "}\n"
        "\n"
        "static unsigned Theron3879TargetStepsRemaining;\n",
    )
    replace_once(
        root,
        "src/pce/input/mouse.cpp",
        '#include "mouse.h"\n',
        '#include "mouse.h"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n',
    )
    replace_once(
        root,
        "src/pce/input/mouse.cpp",
        "namespace MDFN_IEN_PCE\n{\n",
        "namespace MDFN_IEN_PCE\n{\n"
        "static FILE *TheronMouseDeviceTrace;\n"
        "static bool TheronMouseDeviceTraceAttempted;\n"
        "static unsigned int TheronMouseDeviceTraceCount;\n"
        + mouse_input_parser_source()
        + "static void TheronTraceMouseDevice(const char *event, int x, int y,\n"
        "                                   unsigned int buttons)\n"
        "{\n"
        " if(!TheronMouseDeviceTrace)\n"
        " {\n"
        "  if(TheronMouseDeviceTraceAttempted) return;\n"
        "  TheronMouseDeviceTraceAttempted = true;\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_MOUSE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  TheronMouseDeviceTrace = fopen(path, \"a\");\n"
        " }\n"
        " if(!TheronMouseDeviceTrace || TheronMouseDeviceTraceCount >= 20000) return;\n"
        " fprintf(TheronMouseDeviceTrace,\n"
        "         \"device_%s sequence=%u x=%d y=%d buttons=%02x\\n\",\n"
        "         event, TheronMouseDeviceTraceCount++, x, y, buttons & 0xFF);\n"
        " fflush(TheronMouseDeviceTrace);\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce/input/mouse.cpp",
        " mouse_x += (int16)MDFN_de16lsb(data + 0);\n"
        " mouse_y += (int16)MDFN_de16lsb(data + 2);\n"
        " pce_mouse_button = *(data + 4);\n",
        " int16 delta_x = (int16)MDFN_de16lsb(data + 0);\n"
        " int16 delta_y = (int16)MDFN_de16lsb(data + 2);\n"
        " uint8 scripted_buttons = 0;\n"
        " const bool scripted = TheronNextMouseInput(\n"
        "     &delta_x, &delta_y, &scripted_buttons);\n"
        " mouse_x += delta_x;\n"
        " mouse_y += delta_y;\n"
        " pce_mouse_button = scripted ? scripted_buttons : *(data + 4);\n"
        " if(delta_x || delta_y || pce_mouse_button)\n"
        "  TheronTraceMouseDevice(scripted ? \"script\" : \"update\",\n"
        "                         mouse_x, mouse_y, pce_mouse_button);\n",
    )
    replace_once(
        root,
        "src/pce/input/mouse.cpp",
        "   mouse_x += (int32)(rel_x);\n"
        "   mouse_y += (int32)(rel_y);\n",
        "   mouse_x += (int32)(rel_x);\n"
        "   mouse_y += (int32)(rel_y);\n"
        "   TheronTraceMouseDevice(\"latch\", rel_x, rel_y, pce_mouse_button);\n",
    )
    replace_once(
        root,
        "src/pce/input/mouse.cpp",
        " return(ret);\n}\n\nint PCE_Input_Mouse::StateAction",
        " if(pce_mouse_button)\n"
        "  TheronTraceMouseDevice(\"read\", ret, mouse_shifter, pce_mouse_button);\n"
        " return(ret);\n}\n\nint PCE_Input_Mouse::StateAction",
    )
    replace_once(
        root,
        "src/pce_fast/huc6280.cpp",
        "static INLINE uint8 RdMem(unsigned int A)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n uint8 value = HuCPU.PCERead[wmpr](physical_address);\n TheronTraceMainRAMConsumerRead(A, physical_address, value);\n return value;\n}\n",
        "static FILE *TheronPointerTrace;\n"
        "static bool TheronPointerTraceAttempted;\n"
        "static unsigned TheronPointerTraceCount;\n"
        "static bool TheronPointerTraceTruncationReported;\n"
        + pointer_c1df_trace_gate_source()
        + "static unsigned int TheronPointerC1DFReturnPC;\n"
        "static unsigned TheronPointerC1DFStepsRemaining;\n"
        "static bool TheronPointerC1DFActive;\n"
        "static void TheronReportPointerTraceTruncation(void)\n"
        "{\n"
        " if(!TheronPointerTrace || TheronPointerTraceCount < 20000 ||\n"
        "    TheronPointerTraceTruncationReported) return;\n"
        " TheronPointerTraceTruncationReported = true;\n"
        " fprintf(TheronPointerTrace, \"trace_truncated limit=20000\\n\");\n"
        " fflush(TheronPointerTrace);\n"
        "}\n"
        "static void TheronTracePointerAccess(const char *kind, unsigned int A,\n"
        "                                     unsigned int physical, uint8 value,\n"
        "                                     uint8 mpr)\n"
        "{\n"
        " static unsigned int sequence = 0;\n"
        " if(!TheronPointerTrace)\n"
        " {\n"
        "  if(TheronPointerTraceAttempted) return;\n"
        "  TheronPointerTraceAttempted = true;\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_MOUSE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  TheronPointerTrace = fopen(path, \"a\");\n"
        " }\n"
        " if(!TheronPointerTrace) return;\n"
        " if(TheronPointerTraceCount >= 20000)\n"
        " { TheronReportPointerTraceTruncation(); return; }\n"
        " fprintf(TheronPointerTrace,\n"
        "         \"%s sequence=%u logical=%04x physical=%06x mpr=%02x value=%02x pc=%04x\\n\",\n"
        "         kind, sequence++, A & 0xFFFF, physical & 0xFFFFFF, mpr,\n"
        "         value, TheronTraceInstructionPC & 0xFFFF);\n"
        " fflush(TheronPointerTrace);\n"
        " TheronPointerTraceCount++;\n"
        " TheronReportPointerTraceTruncation();\n"
        "}\n"
        "static void TheronTracePointerRegisters(unsigned int pc,\n"
        "                                        unsigned int physical_pc,\n"
        "                                        uint8 a, uint8 x, uint8 y,\n"
        "                                        uint8 p, uint8 s,\n"
        "                                        uint8 opcode, uint8 byte1,\n"
        "                                        uint8 byte2)\n"
        "{\n"
        " if(!TheronPointerTrace) return;\n"
        " if(TheronPointerTraceCount >= 20000)\n"
        " { TheronReportPointerTraceTruncation(); return; }\n"
        " fprintf(TheronPointerTrace,\n"
        "         \"pointer_regs sequence=%u pc=%04x physical_pc=%06x opcode=%02x byte1=%02x byte2=%02x a=%02x x=%02x y=%02x p=%02x s=%02x\\n\",\n"
        "         TheronPointerTraceCount, pc & 0xFFFF, physical_pc & 0xFFFFFF,\n"
        "         opcode, byte1, byte2, a, x, y, p, s);\n"
        " fflush(TheronPointerTrace);\n"
        " TheronPointerTraceCount++;\n"
        " TheronReportPointerTraceTruncation();\n"
        "}\n"
        "static bool TheronPointerAddress(unsigned int A)\n"
        "{\n"
        " return (A >= 0x287F && A <= 0x2896) ||\n"
        "        (A >= 0x77CE && A <= 0x77DB) ||\n"
        "        (A >= 0x77DC && A <= 0x77E9) ||\n"
        "        (A >= 0xD5A7 && A <= 0xD5B6) ||\n"
        "        A == 0x28B8 ||\n"
        "        (A >= 0x00C5 && A <= 0x00CA) ||\n"
        "        (A >= 0x271D && A <= 0x271F) || A == 0x2911 ||\n"
        "        A == 0x2912 ||\n"
        "        A == 0x2E1C || A == 0x2E1D || A == 0x2E3A ||\n"
        "        A == 0x2E3B || A == 0x2E7A || A == 0x2E7B ||\n"
        "        A == 0x2EBA || A == 0x2EBB;\n"
        "}\n"
        "static bool TheronPointerCandidateAddress(unsigned int A)\n"
        "{\n"
        " return A == 0x21D6 || (A >= 0x28FC && A <= 0x2905);\n"
        "}\n"
        "static bool TheronPointerCodePC(unsigned int pc)\n"
        "{\n"
        " if(!TheronIsPointerC1DFTraceEnabled()) return false;\n"
        " return TheronPointerC1DFActive ||\n"
        "        (pc >= 0xA71E && pc <= 0xA733) ||\n"
        "        (pc >= 0xD120 && pc <= 0xD1FF) ||\n"
        "        (pc >= 0xD2B0 && pc <= 0xD2FF) ||\n"
        "        (pc >= 0xD515 && pc <= 0xD5A6);\n"
        "}\n"
        "static INLINE uint8 RdMem(unsigned int A)\n"
        "{\n"
        " uint8 wmpr = HuCPU.MPR[A >> 13];\n"
        " uint32 physical_address = (wmpr << 13) | (A & 0x1FFF);\n"
        " uint8 value = HuCPU.PCERead[wmpr](physical_address);\n"
        " if(TheronPointerAddress(A) ||\n"
        "    TheronPointerCodePC(TheronTraceInstructionPC))\n"
        "  TheronTracePointerAccess(\"guest_read\", A, physical_address, value, wmpr);\n"
        " TheronTraceMainRAMConsumerRead(A, physical_address, value);\n"
        " return(value);\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce_fast/huc6280.cpp",
        "#define LD_IY(op)\t{ unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); op; break; }\n",
        "#define LD_IY(op)\t{ unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); if(TheronPointerCodePC(TheronTraceInstructionPC)) { uint8 wmpr = HuCPU.MPR[EA >> 13]; TheronTracePointerAccess(\"guest_effective_read\", EA, (wmpr << 13) | (EA & 0x1FFF), x, wmpr); } op; break; }\n",
    )
    replace_once(
        root,
        "src/pce_fast/huc6280.cpp",
        "\tint32 next_event;\n",
        "\tint32 next_event;\n"
        "\tbool theron_pointer_c1df_active;\n"
        "\tbool theron_pointer_c1df_return;\n"
        "\tbool theron_pointer_c1df_step_limit;\n",
    )
    replace_once(
        root,
        "src/pce_fast/huc6280.cpp",
        "  TheronTraceInstructionPC = GetRealPC();\n"
        "  TheronTraceInstructionPhysicalPC = (HuCPU.MPR[TheronTraceInstructionPC >> 13] << 13) | (TheronTraceInstructionPC & 0x1FFF);\n"
        "\n"
        "  if(Theron3879FastTargetStepsRemaining ||\n",
        "  TheronTraceInstructionPC = GetRealPC();\n"
        "  TheronTraceInstructionPhysicalPC = (HuCPU.MPR[TheronTraceInstructionPC >> 13] << 13) | (TheronTraceInstructionPC & 0x1FFF);\n"
        "  theron_pointer_c1df_active = TheronPointerC1DFStepsRemaining > 0;\n"
        "  theron_pointer_c1df_return = theron_pointer_c1df_active &&\n"
        "      TheronTraceInstructionPC == TheronPointerC1DFReturnPC;\n"
        "  theron_pointer_c1df_step_limit = theron_pointer_c1df_active &&\n"
        "      !theron_pointer_c1df_return && TheronPointerC1DFStepsRemaining == 1;\n"
        "  TheronPointerC1DFActive = theron_pointer_c1df_active;\n"
        "  if(theron_pointer_c1df_return || theron_pointer_c1df_step_limit)\n"
        "   TheronPointerC1DFStepsRemaining = 0;\n"
        "  else if(theron_pointer_c1df_active)\n"
        "   TheronPointerC1DFStepsRemaining--;\n"
        "  if(TheronPointerCodePC(TheronTraceInstructionPC))\n"
        "  {\n"
        "   TheronTracePointerAccess(\"pointer_pc\", TheronTraceInstructionPC,\n"
        "                            TheronTraceInstructionPhysicalPC, 0,\n"
        "                            HuCPU.MPR[TheronTraceInstructionPC >> 13]);\n"
        "   TheronTracePointerRegisters(TheronTraceInstructionPC,\n"
        "                              TheronTraceInstructionPhysicalPC,\n"
        "                              HU_A, HU_X, HU_Y, HU_P, HU_S,\n"
        "                              RdOp(TheronTraceInstructionPC),\n"
        "                              RdOp(TheronTraceInstructionPC + 1),\n"
        "                              RdOp(TheronTraceInstructionPC + 2));\n"
        "  }\n\n"
        "  if(theron_pointer_c1df_return)\n"
        "   TheronTracePointerAccess(\"pointer_c1df_return\",\n"
        "                            TheronTraceInstructionPC,\n"
        "                            TheronTraceInstructionPhysicalPC, 0,\n"
        "                            HuCPU.MPR[TheronTraceInstructionPC >> 13]);\n"
        "  if(theron_pointer_c1df_step_limit)\n"
        "   TheronTracePointerAccess(\"pointer_c1df_step_limit\",\n"
        "                            TheronTraceInstructionPC,\n"
        "                            TheronTraceInstructionPhysicalPC, 0,\n"
        "                            HuCPU.MPR[TheronTraceInstructionPC >> 13]);\n"
        "  if(TheronIsPointerC1DFTraceEnabled())\n"
        "  {\n"
        "   if(RdOp(TheronTraceInstructionPC) == 0x20 &&\n"
        "     RdOp(TheronTraceInstructionPC + 1) == 0xDF &&\n"
        "     RdOp(TheronTraceInstructionPC + 2) == 0xC1)\n"
        "   {\n"
        "    TheronPointerC1DFReturnPC = (TheronTraceInstructionPC + 3) & 0xFFFF;\n"
        "    TheronPointerC1DFStepsRemaining = 4096;\n"
        "    TheronTracePointerAccess(\"pointer_c1df_begin\",\n"
        "                            TheronTraceInstructionPC,\n"
        "                            TheronTraceInstructionPhysicalPC, 0,\n"
        "                            HuCPU.MPR[TheronTraceInstructionPC >> 13]);\n"
        "   }\n"
        "  }\n\n"
        "  if(Theron3879FastTargetStepsRemaining ||\n",
    )
    replace_once(
        root,
        "src/pce_fast/input.cpp",
        '#include "pce.h"\n',
        '#include "pce.h"\n#include <string.h>\n',
    )
    replace_once(
        root,
        "src/pce_fast/input.cpp",
        "namespace MDFN_IEN_PCE_FAST\n{\n",
        "namespace MDFN_IEN_PCE_FAST\n{\n"
        "static FILE *TheronMouseDeviceTrace;\n"
        "static bool TheronMouseDeviceTraceAttempted;\n"
        "static unsigned TheronMouseDeviceTraceCount;\n"
        + mouse_input_parser_source()
        + "static void TheronTraceMouseDevice(const char *event, int x, int y,\n"
        "                                   unsigned int buttons)\n"
        "{\n"
        " if(!TheronMouseDeviceTrace)\n"
        " {\n"
        "  if(TheronMouseDeviceTraceAttempted) return;\n"
        "  TheronMouseDeviceTraceAttempted = true;\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_MOUSE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  TheronMouseDeviceTrace = fopen(path, \"a\");\n"
        " }\n"
        " if(!TheronMouseDeviceTrace || TheronMouseDeviceTraceCount >= 20000) return;\n"
        " fprintf(TheronMouseDeviceTrace,\n"
        "         \"device_%s sequence=%u update=%u x=%d y=%d buttons=%02x\\n\",\n"
        "         event, TheronMouseDeviceTraceCount++, TheronMouseInputUpdateCount,\n"
        "         x, y, buttons & 0xFF);\n"
        " fflush(TheronMouseDeviceTrace);\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce_fast/input.cpp",
        "   mouse_x[x] += (int16)MDFN_de16lsb(data_ptr[x] + 0);\n"
        "   mouse_y[x] += (int16)MDFN_de16lsb(data_ptr[x] + 2);\n"
        "   pce_mouse_button[x] = *(uint8 *)(data_ptr[x] + 4);\n",
        "   int16 delta_x = (int16)MDFN_de16lsb(data_ptr[x] + 0);\n"
        "   int16 delta_y = (int16)MDFN_de16lsb(data_ptr[x] + 2);\n"
        "   uint8 scripted_buttons = 0;\n"
        "   const bool scripted = x == 0 && TheronNextMouseInput(\n"
        "       &delta_x, &delta_y, &scripted_buttons);\n"
        "   mouse_x[x] += delta_x;\n"
        "   mouse_y[x] += delta_y;\n"
        "   pce_mouse_button[x] = scripted ? scripted_buttons :\n"
        "       *(uint8 *)(data_ptr[x] + 4);\n"
        "   if(scripted || delta_x || delta_y || pce_mouse_button[x])\n"
        "    TheronTraceMouseDevice(scripted ? \"script\" : \"update\",\n"
        "                           mouse_x[x], mouse_y[x],\n"
        "                           pce_mouse_button[x]);\n",
    )
    replace_once(
        root,
        "src/pce_fast/huc6280.cpp",
        "static INLINE void WrMem(unsigned int A, uint8 V)\n{\n uint8 wmpr = HuCPU.MPR[A >> 13];\n HuCPU.PCEWrite[wmpr]((wmpr << 13) | (A & 0x1FFF), V);\n}\n",
        "static INLINE void WrMem(unsigned int A, uint8 V)\n"
        "{\n"
        " uint8 wmpr = HuCPU.MPR[A >> 13];\n"
        " uint32 physical = (wmpr << 13) | (A & 0x1FFF);\n"
        " if(TheronPointerAddress(A) || TheronPointerCandidateAddress(A) ||\n"
        "    TheronPointerCodePC(TheronTraceInstructionPC))\n"
        "  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n"
        " HuCPU.PCEWrite[wmpr](physical, V);\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce_fast/pce.cpp",
        "void TheronTraceDirectMainRAMWrite(uint32 offset, uint8 old_value, uint8 value)\n"
        "{\n"
        " (void)old_value;\n"
        " if(HuCPU.MPR[1] == 0xF8 && offset < 0x100)\n"
        "  TheronTracePartyRAMWrite(offset, value);\n"
        "}\n",
        "static void TheronTracePointerCodeWindow(char *out, size_t out_size,\n"
        "                                         unsigned int logical_pc)\n"
        "{\n"
        " unsigned i;\n"
        " if(!out || out_size < 65) return;\n"
        " for(i = 0; i < 32; ++i)\n"
        " {\n"
        "  unsigned int address = (logical_pc - 8 + i) & 0xFFFF;\n"
        "  uint8 bank = HuCPU.MPR[address >> 13];\n"
        "  uint32 physical = (bank << 13) | (address & 0x1FFF);\n"
        "  snprintf(out + i * 2, 3, \"%02x\", HuCPU.PCERead[bank](physical));\n"
        " }\n"
        " out[64] = 0;\n"
        "}\n"
        "static void TheronTracePointerCandidateRAMWrite(uint32 offset,\n"
        "                                                uint8 old_value,\n"
        "                                                uint8 value)\n"
        "{\n"
        " static FILE *trace = NULL;\n"
        " static unsigned count = 0;\n"
        " char code_window[65];\n"
        " unsigned int code_start;\n"
        " if(IsSGX || old_value == value ||\n"
        "    !(offset == 0x01D6 ||\n"
        "      (offset >= 0x08FC && offset <= 0x0905)) || count >= 4096)\n"
        "  return;\n"
        " if(!trace)\n"
        " {\n"
        "  const char *path = getenv(\"FIRESTAFF_THERON_POINTER_RAM_WRITE_TRACE\");\n"
        "  if(!path || !*path) return;\n"
        "  trace = fopen(path, \"w\");\n"
        "  if(!trace) return;\n"
        "  fputs(\"source=mednafen-pce-fast-pointer-ram-write\\n\", trace);\n"
        " }\n"
        " code_start = (TheronTraceInstructionPC - 8) & 0xFFFF;\n"
        " TheronTracePointerCodeWindow(code_window, sizeof(code_window),\n"
        "                             TheronTraceInstructionPC);\n"
        " fprintf(trace, \"pointer_ram_write sequence=%u logical=%04x physical=%06x old=%02x value=%02x writer_pc=%04x writer_physical_pc=%06x writer_mpr=%02x code_start=%04x code_window=%s\\n\",\n"
        "         count++, 0x2000 + (offset & 0x1FFF),\n"
        "         0x1F0000 + (offset & 0x1FFF), old_value, value,\n"
        "         TheronTraceInstructionPC, TheronTraceInstructionPhysicalPC,\n"
        "         HuCPU.MPR[TheronTraceInstructionPC >> 13], code_start,\n"
        "         code_window);\n"
        " fflush(trace);\n"
        "}\n"
        "void TheronTraceDirectMainRAMWrite(uint32 offset, uint8 old_value, uint8 value)\n"
        "{\n"
        " TheronTracePointerCandidateRAMWrite(offset, old_value, value);\n"
        " if(HuCPU.MPR[1] == 0xF8 && offset < 0x100)\n"
        "  TheronTracePartyRAMWrite(offset, value);\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce_fast/pce.cpp",
        "static DECLFW(BaseRAMWrite)\n"
        "{\n"
        " TheronTracePartyRAMWrite(A, V);\n"
        " const size_t offset = (size_t)A - (0xF8 * 8192);\n"
        " TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);\n"
        " BaseRAM[offset] = V;\n"
        "}\n",
        "static DECLFW(BaseRAMWrite)\n"
        "{\n"
        " TheronTracePartyRAMWrite(A, V);\n"
        " const size_t offset = (size_t)A - (0xF8 * 8192);\n"
        " TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);\n"
        " TheronTracePointerCandidateRAMWrite(offset, BaseRAM[offset], V);\n"
        " BaseRAM[offset] = V;\n"
        "}\n",
    )
    replace_once(
        root,
        "src/pce_fast/pce.cpp",
        "static DECLFW(BaseRAMWrite_Mirrored)\n"
        "{\n"
        " TheronTracePartyRAMWrite(A, V);\n"
        " const size_t offset = A & 0x1FFF;\n"
        " TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);\n"
        " BaseRAM[offset] = V;\n"
        "}\n",
        "static DECLFW(BaseRAMWrite_Mirrored)\n"
        "{\n"
        " TheronTracePartyRAMWrite(A, V);\n"
        " const size_t offset = A & 0x1FFF;\n"
        " TheronTraceA1B7BMTWrite(A, offset, BaseRAM[offset], V);\n"
        " TheronTracePointerCandidateRAMWrite(offset, BaseRAM[offset], V);\n"
        " BaseRAM[offset] = V;\n"
        "}\n",
    )


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} MEDNAFEN_SOURCE_ROOT", file=sys.stderr)
        return 2
    root = Path(sys.argv[1])
    if not (root / "src/pce/huc6280.h").is_file():
        print("FAIL: expected Mednafen 1.32.1 source root", file=sys.stderr)
        return 1
    try:
        instrument(root)
    except OSError as error:
        print(f"FAIL: could not instrument Mednafen source: {error}", file=sys.stderr)
        return 1
    print("PASS: optional PCE mouse and guest-write trace hooks applied")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
