#!/usr/bin/env python3
"""Add a bounded PCE Fast receipt for System Card callback-state memory access."""

from pathlib import Path
import re
import sys


source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
text = source.read_text()
read_marker = "static INLINE uint8 RdMem(unsigned int A)"
write_marker = "static INLINE void WrMem(unsigned int A, uint8 V)"
if text.count(read_marker) != 1 or text.count(write_marker) != 1:
    raise SystemExit("FAIL: expected one HuC6280 RdMem and WrMem")
if "FIRESTAFF_THERON_SYSTEM_CARD_CALLBACK_STATE_TRACE" in text:
    raise SystemExit("FAIL: System Card callback-state trace is already instrumented")

helper = r'''extern uint32 TheronTraceCDSharedEventSequence;
extern uint16 TheronTraceInstructionPC;
extern uint32 TheronTraceInstructionPhysicalPC;

static void TheronTraceSystemCardCallbackState(const char *access,
                                               unsigned int logical,
                                               unsigned int physical,
                                               unsigned int mpr,
                                               unsigned int value)
{
 static FILE *trace = NULL;
 static bool attempted = false;
 static unsigned int rows = 0;
 static bool truncated = false;
 if(!attempted)
 {
  const char *path = getenv("FIRESTAFF_THERON_SYSTEM_CARD_CALLBACK_STATE_TRACE");
  attempted = true;
  if(path && *path)
  {
   trace = fopen(path, "w");
   if(trace)
   {
    fputs("source=mednafen-pce-fast-system-card-callback-state-v1 event_sequence_source=TheronTraceCDSharedEventSequence limit=524288 logical_ranges=f8-fb,2256-226b\n", trace);
    fflush(trace);
   }
  }
 }
 if(!trace || truncated || rows >= 524288 ||
    !((logical >= 0x00F8 && logical <= 0x00FB) ||
      (logical >= 0x2256 && logical <= 0x226B)))
  return;
 const uint32 event_sequence = TheronTraceCDSharedEventSequence++;
 fprintf(trace, "callback_state access=%s index=%u event_sequence=%u logical_address=%04x physical_address=%06x mpr=%02x value=%02x logical_pc=%04x physical_pc=%06x mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x mpr4=%02x mpr5=%02x mpr6=%02x mpr7=%02x\n",
         access, rows++, event_sequence, logical, physical, mpr, value,
         TheronTraceInstructionPC, TheronTraceInstructionPhysicalPC,
         HuCPU.MPR[0], HuCPU.MPR[1], HuCPU.MPR[2], HuCPU.MPR[3],
         HuCPU.MPR[4], HuCPU.MPR[5], HuCPU.MPR[6], HuCPU.MPR[7]);
 fflush(trace);
 if(rows == 524288)
 {
  fputs("callback_state_truncated cap=524288\n", trace);
  fflush(trace);
  truncated = true;
 }
}

'''
text = text.replace(read_marker, helper + read_marker, 1)
read_end = text.find("\n}", text.index(read_marker))
read_body = text[text.index(read_marker):read_end]
read_match = re.search(r"^ (?:uint8 )?([\w]+) = HuCPU\.PCERead\[wmpr\]\(([^)]+)\);$", read_body, re.MULTILINE)
if not read_match:
    raise SystemExit("FAIL: expected one PCERead assignment in RdMem")
read_value, read_physical = read_match.groups()
read_call = read_match.group(0)
read_instrumentation = (
    read_call +
    f'\n if((A >= 0x00F8 && A <= 0x00FB) || (A >= 0x2256 && A <= 0x226B))\n'
    f'  TheronTraceSystemCardCallbackState("read", A & 0xFFFF, {read_physical}, wmpr, {read_value});'
)
text = text[:text.index(read_marker)] + text[text.index(read_marker):read_end].replace(
    read_call, read_instrumentation, 1
) + text[read_end:]

write_start = text.index(write_marker)
write_end = text.find("\n}", write_start)
write_body = text[write_start:write_end]
write_match = re.search(r"^ HuCPU\.PCEWrite\[wmpr\]\(([^,]+), V\);$", write_body, re.MULTILINE)
if not write_match:
    raise SystemExit("FAIL: expected one PCEWrite call in WrMem")
write_physical = write_match.group(1).strip()
write_call = write_match.group(0)
write_instrumentation = (
    write_call +
    f'\n if((A >= 0x00F8 && A <= 0x00FB) || (A >= 0x2256 && A <= 0x226B))\n'
    f'  TheronTraceSystemCardCallbackState("write", A & 0xFFFF, {write_physical}, wmpr, V);'
)
text = text[:write_start] + text[write_start:write_end].replace(
    write_call, write_instrumentation, 1
) + text[write_end:]
source.write_text(text)
