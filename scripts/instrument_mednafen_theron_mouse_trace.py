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
        "\t const uint16 reader_pc = GetRegister(GSREG_PC);\n"
        "\t const uint8 reader_mpr = MPR[reader_pc >> 13];\n",
        "\t const uint16 reader_pc = GetRegister(GSREG_PC);\n"
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
        "static char TheronMouseInputScriptBuffer[8192];\n"
        "static char *TheronMouseInputScriptCursor;\n"
        "static bool TheronMouseInputScriptInitialized;\n"
        "static bool TheronNextMouseInput(int16 *delta_x, int16 *delta_y,\n"
        "                                uint8 *buttons)\n"
        "{\n"
        " if(!TheronMouseInputScriptInitialized)\n"
        " {\n"
        "  const char *value = getenv(\"FIRESTAFF_THERON_MOUSE_INPUT_SCRIPT\");\n"
        "  TheronMouseInputScriptInitialized = true;\n"
        "  if(value && strlen(value) < sizeof(TheronMouseInputScriptBuffer))\n"
        "  {\n"
        "   strcpy(TheronMouseInputScriptBuffer, value);\n"
        "   TheronMouseInputScriptCursor = TheronMouseInputScriptBuffer;\n"
        "  }\n"
        " }\n"
        " if(!TheronMouseInputScriptCursor || !*TheronMouseInputScriptCursor)\n"
        "  return false;\n"
        " char *token = TheronMouseInputScriptCursor;\n"
        " char *separator = strchr(token, ';');\n"
        " if(separator)\n"
        " {\n"
        "  *separator = 0;\n"
        "  TheronMouseInputScriptCursor = separator + 1;\n"
        " }\n"
        " else TheronMouseInputScriptCursor = NULL;\n"
        " int dx = 0, dy = 0, button = 0;\n"
        " if(sscanf(token, \"%d,%d,%d\", &dx, &dy, &button) != 3 ||\n"
        "    dx < -127 || dx > 127 || dy < -127 || dy > 127 ||\n"
        "    button < 0 || button > 15)\n"
        "  return false;\n"
        " *delta_x = (int16)dx; *delta_y = (int16)dy;\n"
        " *buttons = (uint8)button;\n"
        " return true;\n"
        "}\n"
        "static void TheronTraceMouseDevice(const char *event, int x, int y,\n"
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
        "static unsigned int TheronPointerC1DFReturnPC;\n"
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
        "        (A >= 0x00C5 && A <= 0x00CA) ||\n"
        "        (A >= 0x271D && A <= 0x271F) || A == 0x2911 ||\n"
        "        A == 0x2E1C || A == 0x2E1D || A == 0x2E3A ||\n"
        "        A == 0x2E3B || A == 0x2E7A || A == 0x2E7B ||\n"
        "        A == 0x2EBA || A == 0x2EBB;\n"
        "}\n"
        "static bool TheronPointerCodePC(unsigned int pc)\n"
        "{\n"
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
        "  if(TheronTraceInstructionPC >= 0xA71E &&\n"
        "     TheronTraceInstructionPC <= 0xA733 &&\n"
        "     RdOp(TheronTraceInstructionPC) == 0x20 &&\n"
        "     RdOp(TheronTraceInstructionPC + 1) == 0xDF &&\n"
        "     RdOp(TheronTraceInstructionPC + 2) == 0xC1)\n"
        "  {\n"
        "   TheronPointerC1DFReturnPC = (TheronTraceInstructionPC + 3) & 0xFFFF;\n"
        "   TheronPointerC1DFStepsRemaining = 4096;\n"
        "   TheronTracePointerAccess(\"pointer_c1df_begin\",\n"
        "                            TheronTraceInstructionPC,\n"
        "                            TheronTraceInstructionPhysicalPC, 0,\n"
        "                            HuCPU.MPR[TheronTraceInstructionPC >> 13]);\n"
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
        "static char TheronMouseInputScriptBuffer[8192];\n"
        "static char *TheronMouseInputScriptCursor;\n"
        "static bool TheronMouseInputScriptInitialized;\n"
        "static unsigned TheronMouseInputUpdateCount;\n"
        "static bool TheronNextMouseInput(int16 *delta_x, int16 *delta_y,\n"
        "                                uint8 *buttons)\n"
        "{\n"
        " if(!TheronMouseInputScriptInitialized)\n"
        " {\n"
        "  const char *value = getenv(\"FIRESTAFF_THERON_MOUSE_INPUT_SCRIPT\");\n"
        "  TheronMouseInputScriptInitialized = true;\n"
        "  if(value && strlen(value) < sizeof(TheronMouseInputScriptBuffer))\n"
        "  {\n"
        "   strcpy(TheronMouseInputScriptBuffer, value);\n"
        "   TheronMouseInputScriptCursor = TheronMouseInputScriptBuffer;\n"
        "  }\n"
        " }\n"
        " TheronMouseInputUpdateCount++;\n"
        " if(!TheronMouseInputScriptCursor || !*TheronMouseInputScriptCursor)\n"
        "  return false;\n"
        " char *token = TheronMouseInputScriptCursor;\n"
        " char *separator = strchr(token, ';');\n"
        " if(separator)\n"
        " {\n"
        "  *separator = 0;\n"
        "  TheronMouseInputScriptCursor = separator + 1;\n"
        " }\n"
        " else TheronMouseInputScriptCursor = NULL;\n"
        " int dx = 0, dy = 0, button = 0;\n"
        " if(sscanf(token, \"%d,%d,%d\", &dx, &dy, &button) != 3 ||\n"
        "    dx < -127 || dx > 127 || dy < -127 || dy > 127 ||\n"
        "    button < 0 || button > 15)\n"
        "  return false;\n"
        " *delta_x = (int16)dx; *delta_y = (int16)dy;\n"
        " *buttons = (uint8)button;\n"
        " return true;\n"
        "}\n"
        "static void TheronTraceMouseDevice(const char *event, int x, int y,\n"
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
        " if(TheronPointerAddress(A) ||\n"
        "    TheronPointerCodePC(TheronTraceInstructionPC))\n"
        "  TheronTracePointerAccess(\"guest_write\", A, physical, V, wmpr);\n"
        " HuCPU.PCEWrite[wmpr](physical, V);\n"
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
