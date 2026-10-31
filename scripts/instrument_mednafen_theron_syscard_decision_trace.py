#!/usr/bin/env python3
"""Add a bounded, independent System Card BIOS decision-PC trace."""

from pathlib import Path
import sys


source = Path(sys.argv[1]) / "src/pce_fast/huc6280.cpp"
text = source.read_text()
marker = "    TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);"
if text.count(marker) != 1:
    raise SystemExit("FAIL: expected one PCE Fast instruction trace marker")
if "FIRESTAFF_THERON_SYSTEM_CARD_DECISION_TRACE" in text:
    raise SystemExit("FAIL: System Card decision trace is already instrumented")

block = r'''  {
   static FILE *system_card_decision_trace = NULL;
   static bool system_card_decision_trace_attempted = false;
   static bool system_card_decision_trace_truncated = false;
   static unsigned int system_card_decision_trace_rows = 0;
   if(!system_card_decision_trace_attempted)
   {
    const char *path = getenv("FIRESTAFF_THERON_SYSTEM_CARD_DECISION_TRACE");
    system_card_decision_trace_attempted = true;
    if(path && *path)
    {
     system_card_decision_trace = fopen(path, "w");
     if(system_card_decision_trace)
     {
      fputs("source=mednafen-pce-fast-system-card-decision-v1 event_sequence_source=TheronTraceCDSharedEventSequence limit=2048 logical_pcs=e11b,e11d,e12c,e130,e14c,e174,e176,ec05,ec69,e900,e9c5,e209,e20a\n", system_card_decision_trace);
      fflush(system_card_decision_trace);
     }
    }
   }
   if(system_card_decision_trace && !system_card_decision_trace_truncated)
   {
    const unsigned int pc = TheronTraceInstructionPC;
    const bool decision_pc =
        pc == 0xE11B || pc == 0xE11D || pc == 0xE12C || pc == 0xE130 ||
        pc == 0xE14C || pc == 0xE174 || pc == 0xE176 || pc == 0xEC05 ||
        pc == 0xEC69 || pc == 0xE900 || pc == 0xE9C5 || pc == 0xE209 ||
        pc == 0xE20A;
    if(decision_pc && system_card_decision_trace_rows < 2048)
    {
     extern uint32 TheronTraceCDSharedEventSequence;
     uint8 opcode_bytes[4] = {0, 0, 0, 0};
     uint32 physical_bytes[4] = {0, 0, 0, 0};
     unsigned int valid_mask = 0;
     unsigned int i;
     for(i = 0; i < 4; i++)
     {
      const uint16 logical = (uint16)(pc + i);
      const unsigned int slot = (logical >> 13) & 7;
      const uintptr_t mapping = HuCPU.FastPageR[slot];
      if(mapping)
      {
       opcode_bytes[i] = *(const uint8 *)(mapping + logical);
       physical_bytes[i] = ((uint32)HuCPU.MPR[slot] << 13) | (logical & 0x1FFF);
       valid_mask |= 1U << i;
      }
     }
     const uint32 event_sequence = TheronTraceCDSharedEventSequence++;
     fprintf(system_card_decision_trace, "decision index=%u event_sequence=%u logical_pc=%04x physical_pc=%06x bytes=%02x%02x%02x%02x physical_bytes=%06x,%06x,%06x,%06x valid_mask=%x a=%02x x=%02x y=%02x sp=%02x p=%02x mpr0=%02x mpr1=%02x mpr2=%02x mpr3=%02x mpr4=%02x mpr5=%02x mpr6=%02x mpr7=%02x\n",
             system_card_decision_trace_rows, event_sequence, pc,
             TheronTraceInstructionPhysicalPC,
             opcode_bytes[0], opcode_bytes[1], opcode_bytes[2], opcode_bytes[3],
             physical_bytes[0], physical_bytes[1], physical_bytes[2], physical_bytes[3], valid_mask,
             HU_A, HU_X, HU_Y, HuCPU.S, HU_P, HuCPU.MPR[0], HuCPU.MPR[1],
             HuCPU.MPR[2], HuCPU.MPR[3], HuCPU.MPR[4], HuCPU.MPR[5],
             HuCPU.MPR[6], HuCPU.MPR[7]);
     fflush(system_card_decision_trace);
     system_card_decision_trace_rows++;
     if(system_card_decision_trace_rows == 2048)
     {
      fputs("decision_truncated cap=2048\n", system_card_decision_trace);
      fflush(system_card_decision_trace);
      system_card_decision_trace_truncated = true;
     }
    }
   }
  }
'''

source.write_text(text.replace(marker, block + marker, 1))
