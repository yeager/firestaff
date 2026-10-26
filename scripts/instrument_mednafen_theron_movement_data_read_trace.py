#!/usr/bin/env python3
"""Add a bounded, data-only Theron movement-read trace to an isolated build."""

from pathlib import Path
import sys


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise SystemExit(f"FAIL: expected one movement-trace anchor in {path}")
    path.write_text(text.replace(old, new), encoding="utf-8")


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: instrument_mednafen_theron_movement_data_read_trace.py SOURCE")
    source = Path(sys.argv[1])
    huc = source / "src/pce/huc6280.h"
    pcecd = source / "src/pce/pcecd.cpp"
    if not huc.is_file() or not pcecd.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE sources are unavailable")
    replace_once(huc,
        "void TheronPCECDTraceRecordWatchWrite(uint16 logical_address,\n"
        "                                      uint32 physical_address, uint8 value,\n"
        "                                      uint16 writer_pc, uint32 writer_physical_pc);\n",
        "void TheronPCECDTraceRecordWatchWrite(uint16 logical_address,\n"
        "                                      uint32 physical_address, uint8 value,\n"
        "                                      uint16 writer_pc, uint32 writer_physical_pc);\n"
        "void TheronPCECDTraceMovementDataRead(uint16 logical_address,\n"
        "                                         uint32 physical_address, uint8 value,\n"
        "                                         uint16 reader_pc, uint32 reader_physical_pc);\n"
        "void TheronPCECDTraceMovementDataWrite(uint16 logical_address,\n"
        "                                          uint32 physical_address, uint8 value,\n"
        "                                          uint16 writer_pc, uint32 writer_physical_pc);\n"
        "static bool TheronPCECDInstructionFetch;\n")
    replace_once(huc,
        "\t TheronPCECDTraceRecordWatchRead(address, physical_address, ret, reader_pc,\n"
        "\t                                 (reader_mpr << 13) | (reader_pc & 0x1FFF));\n",
        "\t TheronPCECDTraceRecordWatchRead(address, physical_address, ret, reader_pc,\n"
        "\t                                 (reader_mpr << 13) | (reader_pc & 0x1FFF));\n"
        "\t if(!TheronPCECDInstructionFetch)\n"
        "\t  TheronPCECDTraceMovementDataRead(address, physical_address, ret, reader_pc,\n"
        "\t                                     (reader_mpr << 13) | (reader_pc & 0x1FFF));\n")
    replace_once(huc,
        "\tINLINE uint8 RdOp(unsigned int address)\n\t{\n\t return RdMem(address);\n\t}\n",
        "\tINLINE uint8 RdOp(unsigned int address)\n\t{\n"
        "\t const bool previous_fetch = TheronPCECDInstructionFetch;\n"
        "\t TheronPCECDInstructionFetch = true;\n"
        "\t const uint8 value = RdMem(address);\n"
        "\t TheronPCECDInstructionFetch = previous_fetch;\n"
        "\t return value;\n\t}\n")
    replace_once(huc,
        "\t  TheronPCECDTraceMainRAMConsumerWrite(\n"
        "\t   address, physical_address, V, writer_pc, writer_physical_pc);\n",
        "\t  TheronPCECDTraceMainRAMConsumerWrite(\n"
        "\t   address, physical_address, V, writer_pc, writer_physical_pc);\n"
        "\t  TheronPCECDTraceMovementDataWrite(\n"
        "\t   address, physical_address, V, writer_pc, writer_physical_pc);\n")
    marker = "\nvoid PCECD_StateAction(StateMem *sm, const unsigned load, const bool data_only)\n"
    trace = r'''
static FILE *TheronPCECDMovementDataTrace;
static unsigned TheronPCECDMovementDataCount;
static bool TheronPCECDMovementDataArmed;
static unsigned TheronPCECDMovementDataLimit(void)
{
 static unsigned limit;
 if(!limit)
 {
  const char *text = getenv("FIRESTAFF_THERON_MOVEMENT_DATA_READ_SAMPLE_LIMIT");
  unsigned long parsed = text && *text ? strtoul(text, NULL, 10) : 128;
  limit = parsed >= 1 && parsed <= 256 ? (unsigned)parsed : 128;
 }
 return limit;
}
static void TheronPCECDOpenMovementDataTrace(void)
{
 if(!TheronPCECDMovementDataTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_MOVEMENT_DATA_READ_TRACE");
  if(path && *path)
  {
   TheronPCECDMovementDataTrace = fopen(path, "w");
   if(TheronPCECDMovementDataTrace)
    fputs("source=mednafen-pce-theron-movement-data-read-v1\n", TheronPCECDMovementDataTrace);
  }
 }
}
void TheronPCECDTraceMovementDataRead(uint16 logical_address, uint32 physical_address,
                                      uint8 value, uint16 reader_pc, uint32 reader_physical_pc)
{
 if(!TheronPCECDMovementDataArmed && logical_address == 0x2905 &&
    physical_address == 0x1f0905 && value == 0x03 && reader_pc == 0xd34d &&
    reader_physical_pc == 0x0db34d)
 {
  TheronPCECDMovementDataArmed = true;
  TheronPCECDOpenMovementDataTrace();
  if(TheronPCECDMovementDataTrace)
  {
   fprintf(TheronPCECDMovementDataTrace,
           "movement_data_arm logical_address=%04x physical_address=%06x value=%02x reader_pc=%04x reader_physical_pc=%06x\n",
           logical_address, physical_address, value, reader_pc, reader_physical_pc);
   fflush(TheronPCECDMovementDataTrace);
  }
 }
 if(TheronPCECDMovementDataArmed && physical_address >= 0x0e8a00 &&
    physical_address <= 0x0e8aff && TheronPCECDMovementDataCount < TheronPCECDMovementDataLimit())
 {
  TheronPCECDOpenMovementDataTrace();
  if(TheronPCECDMovementDataTrace)
  {
   fprintf(TheronPCECDMovementDataTrace,
           "movement_data_read sequence=%u logical_address=%04x physical_address=%06x value=%02x reader_pc=%04x reader_physical_pc=%06x\n",
           TheronPCECDMovementDataCount++, logical_address, physical_address, value, reader_pc, reader_physical_pc);
   fflush(TheronPCECDMovementDataTrace);
  }
 }
}
void TheronPCECDTraceMovementDataWrite(uint16 logical_address, uint32 physical_address,
                                       uint8 value, uint16 writer_pc, uint32 writer_physical_pc)
{
 if(TheronPCECDMovementDataArmed && logical_address == 0x2905 &&
    physical_address == 0x1f0905 && value == 0x00)
 {
  TheronPCECDOpenMovementDataTrace();
  if(TheronPCECDMovementDataTrace)
  {
   fprintf(TheronPCECDMovementDataTrace,
           "movement_data_disarm logical_address=%04x physical_address=%06x value=%02x writer_pc=%04x writer_physical_pc=%06x reads=%u\n",
           logical_address, physical_address, value, writer_pc, writer_physical_pc, TheronPCECDMovementDataCount);
   fflush(TheronPCECDMovementDataTrace);
  }
  TheronPCECDMovementDataArmed = false;
 }
}
'''
    replace_once(pcecd, marker, "\n" + trace + marker)


if __name__ == "__main__":
    main()
