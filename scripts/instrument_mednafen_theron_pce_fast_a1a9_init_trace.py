#!/usr/bin/env python3
"""Add an opt-in fetched-opcode trace to an isolated Mednafen source tree."""

from pathlib import Path
import sys


TRACE_BLOCK = r'''  {
   static FILE *trace = NULL;
   static bool initialized = false;
   static bool recording = false;
   static unsigned remaining = 0;
   static unsigned sequence = 0;
   if(!initialized)
   {
    const char *path = getenv("FIRESTAFF_THERON_PCE_FAST_A1A9_INIT_TRACE");
    initialized = true;
    if(path && *path)
    {
     trace = fopen(path, "w");
     if(trace)
      fputs("source=mednafen-pce-fast-a1a9-initializer-fetch-v2\n", trace);
    }
   }
   if(trace && !recording && TheronTraceInstructionPC == 0xA1A9 &&
      TheronTraceInstructionPhysicalPC == 0x0E01A9)
   {
    recording = true;
    remaining = 128;
   }
   if(trace && recording && remaining)
   {
    const uint8 *code = (const uint8 *)(HuCPU.FastPageR[
     TheronTraceInstructionPC >> 13] + TheronTraceInstructionPC);
    fprintf(trace, "a1a9_init_instruction sequence=%u logical_pc=%04x physical_pc=%06x mpr1=%02x mpr5=%02x a=%02x x=%02x y=%02x s=%02x p=%02x opcode=%02x bytes=%02x%02x%02x%02x%02x%02x%02x\n",
            sequence++, TheronTraceInstructionPC,
            TheronTraceInstructionPhysicalPC, HuCPU.MPR[1], HuCPU.MPR[5],
            HuCPU.A, X_local, Y_local, HuCPU.S, P_local, b1,
            code[0], code[1], code[2], code[3], code[4], code[5], code[6]);
    fflush(trace);
    remaining--;
    if(!remaining)
     recording = false;
   }
  }
'''


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(
            "usage: instrument_mednafen_theron_pce_fast_a1a9_init_trace.py SOURCE"
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
    if "mednafen-pce-fast-a1a9-initializer-fetch-v2" in text:
        raise SystemExit("FAIL: A1A9 fetched-opcode trace is already installed")

    text = text.replace(dispatch, TRACE_BLOCK + dispatch, 1)
    source.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
