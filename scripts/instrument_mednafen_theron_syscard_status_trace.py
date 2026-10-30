#!/usr/bin/env python3
"""Add an opt-in receipt for authentic System Card $1800 status reads."""

from pathlib import Path
import sys


source = Path(sys.argv[1]) / "src/pce_fast/pcecd.cpp"
text = source.read_text()
marker = "MDFN_FASTCALL uint8 PCECD_Read(uint32 timestamp, uint32 A)"
if text.count(marker) != 1:
    raise SystemExit("FAIL: expected one PCECD_Read function")
if "FIRESTAFF_THERON_SYSTEM_CARD_STATUS_TRACE" in text:
    raise SystemExit("FAIL: System Card status trace is already instrumented")

function_start = text.index(marker)
return_marker = " return(ret);"
return_at = text.find(return_marker, function_start)
if return_at < 0:
    raise SystemExit("FAIL: expected PCECD_Read return")
function_end = text.find("\n}", return_at)
if function_end < 0 or text.find(marker, return_at + len(return_marker)) >= 0:
    raise SystemExit("FAIL: could not bound PCECD_Read")

block = r''' static FILE *system_card_status_trace = NULL;
 static bool system_card_status_trace_attempted = false;
 static unsigned int system_card_status_trace_rows = 0;
 static bool system_card_status_trace_truncated = false;
 if(!system_card_status_trace_attempted)
 {
  const char *path = getenv("FIRESTAFF_THERON_SYSTEM_CARD_STATUS_TRACE");
  system_card_status_trace_attempted = true;
  if(path && *path)
  {
   system_card_status_trace = fopen(path, "w");
   if(system_card_status_trace)
   {
    fputs("source=mednafen-pce-fast-system-card-status-read-v1 event_sequence_source=TheronTraceCDSharedEventSequence pc=e95e address=1800 limit=65536\n", system_card_status_trace);
    fflush(system_card_status_trace);
   }
  }
 }
 if(system_card_status_trace && !system_card_status_trace_truncated &&
    TheronTraceInstructionPC == 0xE95E && (A & 0x1FFF) == 0x1800)
 {
  if(system_card_status_trace_rows < 65536)
  {
   const uint32 event_sequence = TheronTraceCDSharedEventSequence++;
   fprintf(system_card_status_trace, "status_read index=%u event_sequence=%u timestamp=%u logical_pc=%04x physical_pc=%06x logical_address=%04x status=%02x bsy=%u req=%u msg=%u cd=%u io=%u db=%02x\n",
           system_card_status_trace_rows, event_sequence, timestamp,
           TheronTraceInstructionPC, TheronTraceInstructionPhysicalPC,
           (unsigned int)(A & 0xFFFF), (unsigned int)ret,
           PCECD_Drive_GetBSY() ? 1U : 0U,
           PCECD_Drive_GetREQ() ? 1U : 0U,
           PCECD_Drive_GetMSG() ? 1U : 0U,
           PCECD_Drive_GetCD() ? 1U : 0U,
           PCECD_Drive_GetIO() ? 1U : 0U,
           (unsigned int)PCECD_Drive_GetDB());
   fflush(system_card_status_trace);
   system_card_status_trace_rows++;
  }
  else
  {
   fprintf(system_card_status_trace, "status_read_truncated cap=65536\n");
   fflush(system_card_status_trace);
   system_card_status_trace_truncated = true;
  }
 }
'''
text = text[:return_at] + block + text[return_at:]
source.write_text(text)
