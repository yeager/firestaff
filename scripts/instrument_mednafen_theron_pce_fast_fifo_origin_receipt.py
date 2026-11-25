#!/usr/bin/env python3
"""Connect PCE Fast CD FIFO bytes to direct main-RAM store receipts."""

from pathlib import Path
import sys


PCECD_DECLARATION = """extern void TheronTracePCEFastQueueCDDataRead(
    uint32 source_lba, uint16 source_offset, uint8 value, bool source_valid,
    uint16 reader_pc, uint32 reader_physical_pc);

"""

PCECD_CALL = "  TheronTracePCEFastQueueCDDataRead(source_lba, user_data_offset, ret,\n" \
             "                                      source_valid, TheronTraceInstructionPC,\n" \
             "                                      TheronTraceInstructionPhysicalPC);\n"

HUC_DECLARATION = """extern void TheronTracePCEFastOriginRAMWrite(uint32 logical_address,
                                                 uint32 physical_address, uint8 value);
"""

HUC_CALL = " TheronTracePCEFastOriginRAMWrite(A, physical, V);\n"

ORIGIN_HELPERS = r'''static bool TheronPCEFastExactLBAAllowed(uint32 source_lba)
{
 static bool initialized = false;
 static bool requested = false;
 static bool valid = false;
 static uint32 exact_lba = 0;
 if(!initialized)
 {
  const char *text = getenv("FIRESTAFF_THERON_PCE_FAST_EXACT_SOURCE_LBA");
  requested = text && *text;
  if(requested)
  {
   char *end = NULL;
   const unsigned long parsed = strtoul(text, &end, 10);
   valid = end && end != text && *end == 0 && parsed <= 0xFFFFFFFFUL;
   if(valid)
    exact_lba = (uint32)parsed;
  }
  initialized = true;
 }
 return !requested || (valid && source_lba == exact_lba);
}

struct TheronPCEFastPendingCDRead
{
 bool valid;
 uint32 source_lba;
 uint16 source_offset;
 uint16 reader_pc;
 uint32 reader_physical_pc;
 uint8 value;
 uint8 opcode;
 uint8 instruction_length;
 uint64 fifo_sequence;
};
static TheronPCEFastPendingCDRead TheronPCEFastPendingCDDataRead = {};
static uint64 TheronPCEFastCDReadSequence;
static unsigned TheronPCEFastOriginRAMReceiptCount;
static FILE *TheronPCEFastOriginRAMReceiptTrace;

static uint8 TheronPCEFastReadOpcode(uint16 pc)
{
 return *(const uint8 *)(HuCPU.FastPageR[pc >> 13] + pc);
}

static uint8 TheronPCEFastLoadLength(uint8 opcode)
{
 switch(opcode)
 {
  case 0xA1: case 0xA5: case 0xA6: case 0xA4: case 0xB1: case 0xB4:
  case 0xB5: case 0xB6:
   return 2;
  case 0xAD: case 0xAE: case 0xAC: case 0xB9: case 0xBC: case 0xBD:
  case 0xBE:
   return 3;
  default:
   return 0;
 }
}

static bool TheronPCEFastLoadStoreRegisterMatches(uint8 load_opcode,
                                                  uint8 store_opcode)
{
 switch(load_opcode)
 {
  case 0xA1: case 0xA5: case 0xA9: case 0xAD: case 0xB1: case 0xB5:
  case 0xB9: case 0xBD:
   return store_opcode == 0x81 || store_opcode == 0x85 ||
          store_opcode == 0x8D || store_opcode == 0x91 ||
          store_opcode == 0x95 || store_opcode == 0x99 ||
          store_opcode == 0x9D;
  case 0xA2: case 0xA6: case 0xAE: case 0xB6: case 0xBE:
   return store_opcode == 0x86 || store_opcode == 0x8E ||
          store_opcode == 0x96;
  case 0xA0: case 0xA4: case 0xAC: case 0xB4: case 0xBC:
   return store_opcode == 0x84 || store_opcode == 0x8C ||
          store_opcode == 0x94;
  default:
   return false;
 }
}

void TheronTracePCEFastQueueCDDataRead(uint32 source_lba, uint16 source_offset,
                                       uint8 value, bool source_valid,
                                       uint16 reader_pc, uint32 reader_physical_pc)
{
 TheronPCEFastPendingCDDataRead.valid = false;
 if(!source_valid)
  return;
 const uint64 fifo_sequence = TheronPCEFastCDReadSequence++;
 if(!TheronPCEFastExactLBAAllowed(source_lba))
  return;
 const uint8 opcode = TheronPCEFastReadOpcode(reader_pc);
 const uint8 length = TheronPCEFastLoadLength(opcode);
 if(!length)
  return;
 TheronPCEFastPendingCDDataRead.valid = true;
 TheronPCEFastPendingCDDataRead.source_lba = source_lba;
 TheronPCEFastPendingCDDataRead.source_offset = source_offset;
 TheronPCEFastPendingCDDataRead.reader_pc = reader_pc;
 TheronPCEFastPendingCDDataRead.reader_physical_pc = reader_physical_pc;
 TheronPCEFastPendingCDDataRead.value = value;
 TheronPCEFastPendingCDDataRead.opcode = opcode;
 TheronPCEFastPendingCDDataRead.instruction_length = length;
 TheronPCEFastPendingCDDataRead.fifo_sequence = fifo_sequence;
}

void TheronTracePCEFastOriginRAMWrite(uint32 logical_address,
                                      uint32 physical_address, uint8 value)
{
 if(!TheronPCEFastPendingCDDataRead.valid)
  return;
 const TheronPCEFastPendingCDRead read = TheronPCEFastPendingCDDataRead;
 TheronPCEFastPendingCDDataRead.valid = false;
 const uint16 writer_pc = TheronTraceInstructionPC;
 const uint16 expected_writer_pc =
  (uint16)(read.reader_pc + read.instruction_length);
 const uint8 writer_opcode = TheronPCEFastReadOpcode(writer_pc);
 if(writer_pc != expected_writer_pc ||
    !TheronPCEFastLoadStoreRegisterMatches(read.opcode, writer_opcode) ||
    physical_address < 0x1F0000 || physical_address >= 0x1F8000 ||
    value != read.value || TheronPCEFastOriginRAMReceiptCount >= 256)
  return;
 if(!TheronPCEFastOriginRAMReceiptTrace)
 {
  const char *path = getenv("FIRESTAFF_THERON_IRQ2_CD_TRACE");
  if(!path || !*path)
   return;
  TheronPCEFastOriginRAMReceiptTrace = fopen(path, "a");
  if(!TheronPCEFastOriginRAMReceiptTrace)
  {
   fputs("FIRESTAFF_THERON_PCE_FAST_ORIGIN_RAM_RECEIPT_ERROR error=open\n", stderr);
   fflush(stderr);
   return;
  }
 }
 if(fprintf(TheronPCEFastOriginRAMReceiptTrace,
            "pce_cd_origin_ram_receipt source_lba=%u source_offset=%u fifo_sequence=%llu reader_pc=%04x reader_physical_pc=%06x writer_pc=%04x writer_physical_pc=%06x logical_destination=%04x physical=%08x read_value=%02x stored_value=%02x\n",
            read.source_lba, read.source_offset,
            (unsigned long long)read.fifo_sequence, read.reader_pc,
            read.reader_physical_pc, writer_pc,
            TheronTraceInstructionPhysicalPC, logical_address & 0xFFFF,
            physical_address, read.value, value) < 0 ||
    fflush(TheronPCEFastOriginRAMReceiptTrace) != 0)
 {
  fputs("FIRESTAFF_THERON_PCE_FAST_ORIGIN_RAM_RECEIPT_ERROR error=write\n", stderr);
  fflush(stderr);
  fclose(TheronPCEFastOriginRAMReceiptTrace);
  TheronPCEFastOriginRAMReceiptTrace = NULL;
  return;
 }
 TheronPCEFastOriginRAMReceiptCount++;
}

'''


def insert_once(text: str, anchor: str, block: str, label: str) -> str:
    if block.strip() in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, block + anchor, 1)


def replace_once(text: str, anchor: str, replacement: str, label: str) -> str:
    if replacement in text:
        return text
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor")
    return text.replace(anchor, replacement, 1)


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_fifo_origin_receipt.py SOURCE"
        )
    root = Path(sys.argv[1])
    pcecd = root / "src/pce_fast/pcecd.cpp"
    huc = root / "src/pce_fast/huc6280.cpp"
    pce = root / "src/pce_fast/pce.cpp"
    if not all(path.is_file() for path in (pcecd, huc, pce)):
        raise SystemExit("FAIL: PCE Fast sources are unavailable")

    pcecd_text = pcecd.read_text(encoding="utf-8")
    pcecd_anchor = "static INLINE uint8 read_1808(int32 timestamp)\n"
    pcecd_text = insert_once(
        pcecd_text, pcecd_anchor, PCECD_DECLARATION,
        "PCE Fast CD-origin helper declaration",
    )
    call_anchor = "  TheronTraceCDDataPortRead(TheronTraceInstructionPC, source_lba, user_data_offset, raw_sector_offset,\n" \
                  "                            PCECD_Drive_GetTrack02StartLBA(), PCECD_Drive_GetTrack02EndLBA(),\n" \
                  "                            track02_index1_file_offset, track02_sector_bytes, ret, source_valid);\n"
    pcecd_text = replace_once(
        pcecd_text, call_anchor, call_anchor + PCECD_CALL,
        "PCE Fast CD-port origin capture",
    )

    huc_text = huc.read_text(encoding="utf-8")
    huc_text = insert_once(
        huc_text, "static INLINE void WrMem(unsigned int A, uint8 V)\n",
        HUC_DECLARATION, "PCE Fast origin-RAM helper declaration",
    )
    huc_text = insert_once(
        huc_text, " TheronTracePCEFastMainRAMTargetWrite(A, physical, V);\n",
        HUC_CALL, "PCE Fast origin-RAM write hook",
    )

    pce_text = pce.read_text(encoding="utf-8")
    pce_text = insert_once(
        pce_text,
        "static void TheronTraceA1B7BMTWrite(uint32 physical_destination,\n",
        ORIGIN_HELPERS,
        "PCE Fast origin-RAM receipt implementation",
    )
    pcecd.write_text(pcecd_text, encoding="utf-8")
    huc.write_text(huc_text, encoding="utf-8")
    pce.write_text(pce_text, encoding="utf-8")


if __name__ == "__main__":
    main()
