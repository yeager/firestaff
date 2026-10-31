#!/usr/bin/env python3
"""Add an opt-in bounded System Card I/O/RAM access receipt."""

from pathlib import Path
import re
import sys


source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
text = source.read_text()
read_marker = "static INLINE uint8 RdMem(unsigned int A)"
write_marker = "static INLINE void WrMem(unsigned int A, uint8 V)"
if text.count(read_marker) != 1 or text.count(write_marker) != 1:
    raise SystemExit("FAIL: expected one HuC6280 RdMem and WrMem")
if "FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_TRACE" in text:
    raise SystemExit("FAIL: System Card I/O/RAM trace is already instrumented")

helper = r'''extern uint32 TheronTraceCDSharedEventSequence;
extern uint16 TheronTraceInstructionPC;
extern uint32 TheronTraceInstructionPhysicalPC;

static void TheronTraceSystemCardIoRam(const char *access,
                                       unsigned int logical,
                                       unsigned int mpr,
                                       unsigned int physical,
                                       unsigned int value)
{
 static FILE *trace = NULL;
 static bool attempted = false;
 static bool truncated = false;
 static unsigned int rows = 0;
 if(!attempted)
 {
  const char *path = getenv("FIRESTAFF_THERON_SYSTEM_CARD_IO_RAM_TRACE");
  attempted = true;
  if(path && *path)
  {
   trace = fopen(path, "w");
   if(trace)
   {
    fputs("source=mednafen-pce-fast-system-card-io-ram-v1 event_sequence_source=TheronTraceCDSharedEventSequence limit=524288 logical_ranges=1800-1803,224c-2255,2273-2280,22a4\n", trace);
    fflush(trace);
   }
  }
 }
 if(!trace || truncated ||
    !((logical >= 0x1800 && logical <= 0x1803) ||
      (logical >= 0x224C && logical <= 0x2255) ||
      (logical >= 0x2273 && logical <= 0x2280) || logical == 0x22A4))
  return;
 if(rows >= 524288)
 {
  fputs("io_ram_truncated cap=524288\n", trace);
  fflush(trace);
  truncated = true;
  return;
 }
 const unsigned int pc = TheronTraceInstructionPC;
 const unsigned int pc_slot = (pc >> 13) & 7;
 uint8 code_bytes[4] = {0, 0, 0, 0};
 uint32 code_physical_bytes[4] = {0, 0, 0, 0};
 unsigned int code_valid_mask = 0;
 for(unsigned int offset = 0; offset < 4; offset++)
 {
  const uint16 code_logical = (uint16)(pc + offset);
  const unsigned int slot = (code_logical >> 13) & 7;
  const uintptr_t mapping = HuCPU.FastPageR[slot];
  if(mapping)
  {
   code_bytes[offset] = *(const uint8 *)(mapping + code_logical);
   code_physical_bytes[offset] =
       ((uint32)HuCPU.MPR[slot] << 13) | (code_logical & 0x1FFF);
   code_valid_mask |= 1U << offset;
  }
 }
 const uint32 event_sequence = TheronTraceCDSharedEventSequence++;
 fprintf(trace, "io_ram access=%s index=%u event_sequence=%u logical_address=%04x physical_address=%06x access_mpr=%02x value=%02x logical_pc=%04x physical_pc=%06x code_bytes=%02x%02x%02x%02x code_physical_bytes=%06x,%06x,%06x,%06x code_valid_mask=%x pc_mpr=%02x mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x mpr4=%02x mpr5=%02x mpr6=%02x mpr7=%02x\n",
         access, rows++, event_sequence, logical, physical, mpr, value, pc,
         TheronTraceInstructionPhysicalPC,
         (unsigned int)code_bytes[0], (unsigned int)code_bytes[1],
         (unsigned int)code_bytes[2], (unsigned int)code_bytes[3],
         code_physical_bytes[0], code_physical_bytes[1],
         code_physical_bytes[2], code_physical_bytes[3], code_valid_mask,
         (unsigned int)HuCPU.MPR[pc_slot],
         (unsigned int)HuCPU.MPR[0], (unsigned int)HuCPU.MPR[1],
         (unsigned int)HuCPU.MPR[2], (unsigned int)HuCPU.MPR[3],
         (unsigned int)HuCPU.MPR[4], (unsigned int)HuCPU.MPR[5],
         (unsigned int)HuCPU.MPR[6], (unsigned int)HuCPU.MPR[7]);
 fflush(trace);
}

'''
text = text.replace(read_marker, helper + read_marker, 1)

read_start = text.index(read_marker)
read_end = text.find("\n}", read_start)
read_body = text[read_start:read_end]
read_pattern = re.compile(
    r"^(\s*)(?:uint8\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*=\s*"
    r"HuCPU\.PCERead\[wmpr\]\(([^)]+)\);$",
    re.MULTILINE,
)
read_matches = list(read_pattern.finditer(read_body))
if len(read_matches) != 1:
    raise SystemExit("FAIL: expected one PCERead assignment in RdMem")
read_match = read_matches[0]
read_indent, read_value, read_physical = read_match.groups()
read_call = read_match.group(0)
read_log = (
    read_call
    + f'\n{read_indent}if((A >= 0x1800 && A <= 0x1803) || '
      '(A >= 0x224C && A <= 0x2255) || '
      '(A >= 0x2273 && A <= 0x2280) || A == 0x22A4)\n'
    + f'{read_indent} TheronTraceSystemCardIoRam("read", A & 0xFFFF, wmpr, '
      f'{read_physical.strip()}, {read_value});'
)
text = text[:read_start] + read_body.replace(read_call, read_log, 1) + text[read_end:]

write_start = text.index(write_marker)
write_end = text.find("\n}", write_start)
write_body = text[write_start:write_end]
write_pattern = re.compile(
    r"^(\s*)HuCPU\.PCEWrite\[wmpr\]\(([^,]+),\s*V\);$", re.MULTILINE
)
write_matches = list(write_pattern.finditer(write_body))
if len(write_matches) != 1:
    raise SystemExit("FAIL: expected one PCEWrite call in WrMem")
write_match = write_matches[0]
write_indent, write_physical = write_match.groups()
write_call = write_match.group(0)
write_log = (
    write_call
    + f'\n{write_indent}if((A >= 0x1800 && A <= 0x1803) || '
      '(A >= 0x224C && A <= 0x2255) || '
      '(A >= 0x2273 && A <= 0x2280) || A == 0x22A4)\n'
    + f'{write_indent} TheronTraceSystemCardIoRam("write", A & 0xFFFF, wmpr, '
      f'{write_physical.strip()}, V);'
)
text = text[:write_start] + write_body.replace(write_call, write_log, 1) + text[write_end:]
source.write_text(text)
