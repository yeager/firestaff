#!/usr/bin/env python3
"""Trace C852 and Drator row-reader opcode windows for source mapping."""

from pathlib import Path
import sys


TRACE_BLOCK = r'''  {
   static FILE *trace = NULL;
   static bool initialized = false;
   static unsigned c852_count = 0;
   static bool c852_truncation_reported = false;
   static unsigned cbc6_count = 0;
   static unsigned cbc6_dropped = 0;
   static unsigned c45c3_count = 0;
   static unsigned c45c3_dropped = 0;
   if(!initialized)
   {
    const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_C852_FETCH_TRACE");
    initialized = true;
    if(path && *path)
    {
     trace = fopen(path, "w");
     if(trace)
      fputs("source=mednafen-pce-fast-c852-opcode-fetch event_sequence_source=TheronTraceCDSharedEventSequence\n", trace);
     else
      fputs("FIRESTAFF_THERON_PCE_FAST_C852_FETCH_TRACE_ERROR error=open\n", stderr);
    }
   }
   if(trace && (TheronTraceInstructionPC == 0xC852 ||
                TheronTraceInstructionPC == 0xCBC6 ||
                TheronTraceInstructionPC == 0x45C3))
   {
    const bool is_c852 = TheronTraceInstructionPC == 0xC852;
    const bool is_cbc6 = TheronTraceInstructionPC == 0xCBC6;
    const unsigned limit = 4096;
    unsigned *count = is_c852 ? &c852_count :
                      (is_cbc6 ? &cbc6_count : &c45c3_count);
    unsigned *dropped = is_c852 ? NULL :
                        (is_cbc6 ? &cbc6_dropped : &c45c3_dropped);
    if(*count >= limit)
    {
     if(is_c852)
     {
      if(!c852_truncation_reported)
      {
       fprintf(trace, "c852_fetch_truncated limit=%u\n", limit);
       fflush(trace);
       c852_truncation_reported = true;
      }
     }
     else if((*dropped)++ == 0)
      fprintf(trace, "row_reader_fetch_truncated logical_pc=%04x limit=%u\n",
              TheronTraceInstructionPC, limit);
    }
    else
    {
     extern uint32 TheronTraceCDSharedEventSequence;
     const uint32 event_sequence = TheronTraceCDSharedEventSequence++;
     char code_bytes[1025] = "unavailable";
     if((TheronTraceInstructionPC & 0x1FFF) <= 0x1E00)
     {
      const uint8 *code = (const uint8 *)(HuCPU.FastPageR[
       TheronTraceInstructionPC >> 13] + TheronTraceInstructionPC);
      for(unsigned i = 0; i < 512; i++)
       snprintf(code_bytes + i * 2, 3, "%02x", code[i]);
     }
     fprintf(trace, "%s sequence=%u event_sequence=%u logical_pc=%04x physical_pc=%06x opcode=%02x code_bytes=%s mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x mpr4=%02x mpr5=%02x mpr6=%02x mpr7=%02x a=%02x x=%02x y=%02x s=%02x p=%02x\n",
             is_c852 ? "c852_fetch" : "row_reader_fetch",
             (*count)++, event_sequence, TheronTraceInstructionPC,
             TheronTraceInstructionPhysicalPC, b1, code_bytes,
             HuCPU.MPR[0], HuCPU.MPR[1], HuCPU.MPR[2], HuCPU.MPR[3],
             HuCPU.MPR[4], HuCPU.MPR[5], HuCPU.MPR[6], HuCPU.MPR[7],
             HuCPU.A, X_local, Y_local, HuCPU.S, P_local);
     if(fflush(trace) != 0)
      fputs("FIRESTAFF_THERON_PCE_FAST_C852_FETCH_TRACE_ERROR error=write\n", stderr);
    }
   }
  }
'''


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_c852_fetch_trace.py SOURCE"
        )

    source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
    if not source.is_file():
        raise SystemExit("FAIL: isolated Mednafen PCE Fast source is unavailable")

    text = source.read_text(encoding="utf-8")
    fetch = "\t  b1 = RdAtPC();\n"
    dispatch = "\t  ADDCYC(CycTable[b1]);\n"
    if text.count(fetch) != 1 or text.count(dispatch) != 1:
        raise SystemExit("FAIL: expected unique PCE Fast opcode-fetch and dispatch anchors")
    fetch_offset = text.index(fetch)
    dispatch_offset = text.index(dispatch)
    if fetch_offset >= dispatch_offset:
        raise SystemExit("FAIL: PCE Fast opcode fetch must precede cycle dispatch")
    if "source=mednafen-pce-fast-c852-opcode-fetch" in text:
        raise SystemExit("FAIL: C852 fetched-opcode trace is already installed")
    if "TheronTraceInstructionPC = GetRealPC();" not in text[:fetch_offset]:
        raise SystemExit("FAIL: instruction-PC context must be captured before opcode fetch")

    text = text.replace(dispatch, TRACE_BLOCK + dispatch, 1)
    source.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
