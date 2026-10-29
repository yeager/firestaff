#!/usr/bin/env python3
"""Apply a bounded, opt-in HuC6280 $2390->$23AD trace to Mednafen 1.32.1.

This edits only the disposable source copy supplied on the command line.  It
is deliberately a source transformer rather than a patch against third-party
source in the Firestaff repository.
"""

from pathlib import Path
import hashlib
import re
import sys


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"FAIL: expected one {label} anchor, found {count}")
    return text.replace(old, new, 1)


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(f"usage: {Path(sys.argv[0]).name} MEDNAFEN_SOURCE_COPY")
    root = Path(sys.argv[1]).resolve()
    cpu_path = root / "src/pce_fast/huc6280.cpp"
    ops_path = root / "src/pce_fast/huc6280_ops.inc"
    version_path = root / "configure.ac"
    if not cpu_path.is_file() or not ops_path.is_file() or not version_path.is_file():
        raise SystemExit("FAIL: expected Mednafen 1.32.1 PCE Fast source files")

    cpu = cpu_path.read_text()
    ops = ops_path.read_text()
    ops_raw = ops_path.read_bytes()
    source_identity = (
        "source_release=Mednafen-1.32.1 "
        f"huc6280_cpp_sha256={hashlib.sha256(cpu_path.read_bytes()).hexdigest()} "
        f"huc6280_ops_inc_sha256={hashlib.sha256(ops_raw).hexdigest()} "
        "game_code_identity=runtime_fetch_bytes_only game_disc_identity=unverified"
    )
    if "MEDNAFEN_VERSION='1.32.1'" not in version_path.read_text():
        raise SystemExit("FAIL: source configure.ac does not identify Mednafen 1.32.1")
    if "#define RdAtPC() RdOp(HU_PC)" not in cpu or "case 0x53: // TAMi" not in ops:
        raise SystemExit("FAIL: source anchors do not match Mednafen 1.32.1")
    if "FirestaffTheron23AD" in cpu or "FirestaffTheron23AD" in ops:
        raise SystemExit("FAIL: HuC6280 $23AD trace already appears applied")

    cpu = replace_once(
        cpu,
        '#include "vdc.h"\n',
        '#include "vdc.h"\n#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n',
        "standard headers",
    )

    # Capture is bounded and globally retained so the trace remains active when
    # a block transfer yields from one HuC6280_Run() invocation to the next.
    helper = r'''#define HU_Page1		Page1_local
// Firestaff's opt-in raw execution trace.  The source release identity is
// asserted by the caller, but the runtime trace cannot prove ROM byte identity.
static FILE *FirestaffTheron23ADFile;
static bool FirestaffTheron23ADActive;
static bool FirestaffTheron23ADStarted;
static bool FirestaffTheron23ADComplete;
static bool FirestaffTheron23ADReturnPending;
static uint16 FirestaffTheron23ADCallerCandidatePC;
static unsigned FirestaffTheron23ADCallerCandidateStage;
static uint16 FirestaffTheron23ADCallerPC;
static uint16 FirestaffTheron23ADRoutinePC;
static uint16 FirestaffTheron23ADReturnPC;
static uint16 FirestaffTheron23ADStackPC;
static bool FirestaffTheron23ADEXpectsOpcode = true;
static unsigned FirestaffTheron23ADEvents;
static const unsigned FirestaffTheron23ADEventLimit = 2000000;
static uint32 FirestaffTheron23ADPC;
static uint8 FirestaffTheron23DA, FirestaffTheron23DX, FirestaffTheron23DY;
static uint8 FirestaffTheron23DSP, FirestaffTheron23DP;
static uint8 FirestaffTheron23ADFetch(uintptr_t raw_pc, uint8 a, uint8 x, uint8 y, uint8 s, uint8 p);
static void FirestaffTheron23ADEndInstruction(void);

static void FirestaffTheron23ADOpen(void)
{
 const char *path = getenv("FIRESTAFF_THERON_HUC6280_23AD_TRACE");
 if(!path || !*path || FirestaffTheron23ADFile) return;
 FirestaffTheron23ADFile = fopen(path, "w");
 if(FirestaffTheron23ADFile)
  fputs("format=firestaff-huc6280-23ad-raw-trace-v1 SOURCE_IDENTITY_PLACEHOLDER trace_state=events-only comparable=not-yet cd_causality=not-established game_disc_identity=unverified\n", FirestaffTheron23ADFile);
}

static void FirestaffTheron23ADEmit(const char *kind, uint32 pc,
 uint32 logical, uint8 value, uint8 a, uint8 x, uint8 y, uint8 s, uint8 p)
{
 if(!FirestaffTheron23ADFile || FirestaffTheron23ADEvents >= FirestaffTheron23ADEventLimit)
  return;
 const uint8 slot = logical >> 13;
 const uint8 bank = HuCPU.MPR[slot];
 fprintf(FirestaffTheron23ADFile,
  "seq=%u kind=%s pc=%08x logical=%08x slot=%u mpr=%02x physical=%06x value=%02x a=%02x x=%02x y=%02x s=%02x p=%02x\n",
  FirestaffTheron23ADEvents++, kind, pc, logical, slot, bank,
  ((uint32)bank << 13) | (logical & 0x1fff), value, a, x, y, s, p);
 if(FirestaffTheron23ADEvents == FirestaffTheron23ADEventLimit)
  fputs("truncated=event_limit\n", FirestaffTheron23ADFile);
}

static uint8 FirestaffTheron23ADPage1Read(uint16 offset)
{
 const uint8 value = HuCPU.Page1[offset];
 if(FirestaffTheron23ADActive)
  FirestaffTheron23ADEmit("page1_read", FirestaffTheron23ADPC,
   0x2000 | (offset & 0x1fff), value, FirestaffTheron23DA,
   FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP,
   FirestaffTheron23DP);
 return value;
}

static void FirestaffTheron23ADPage1Write(uint16 offset, uint8 value)
{
 HuCPU.Page1[offset] = value;
 if(FirestaffTheron23ADActive)
  FirestaffTheron23ADEmit("page1_write", FirestaffTheron23ADPC,
   0x2000 | (offset & 0x1fff), value, FirestaffTheron23DA,
   FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP,
   FirestaffTheron23DP);
}

#define TRACE_PAGE1_READ(offset) FirestaffTheron23ADPage1Read(offset)
#define TRACE_PAGE1_WRITE(offset, value) FirestaffTheron23ADPage1Write((offset), (value))
'''
    helper = helper.replace("SOURCE_IDENTITY_PLACEHOLDER", source_identity)
    cpu = replace_once(cpu, "#define HU_Page1\t\tPage1_local\n", helper,
                       "Page1 macro")

    # Instruction fetch wrapper sees opcode and operands without changing the
    # interpreter's fast fetch path.  The BSR is armed only at $2390 and starts
    # capture only if its signed relative destination is exactly $23AD.
    fetch = '#define RdAtPC() FirestaffTheron23ADFetch(HU_PC, HU_A, HU_X, HU_Y, HU_S, HU_P)\n'
    cpu = replace_once(cpu,
        '#ifdef HUC6280_CRAZY_VERSION\n #define RdAtPC() (*(uint8*)HU_PC)\n#else\n #define RdAtPC() RdOp(HU_PC)\n#endif\n',
        fetch, "opcode fetch macro")

    # Read/write hooks cover ordinary addressing and every block-move byte.
    rd_start = cpu.find("static INLINE uint8 RdMem(unsigned int A)")
    rd_end = cpu.find("static INLINE uint16 RdMem16(unsigned int A)", rd_start)
    if rd_start < 0 or rd_end < 0:
        raise SystemExit("FAIL: cannot locate complete RdMem function")
    rd = cpu[rd_start:rd_end]
    if "FirestaffTheron23ADEmit(\"mem_read\"" not in rd:
        if "TheronTraceMainRAMConsumerRead(A, physical_address, value);" in rd:
            anchor = " TheronTraceMainRAMConsumerRead(A, physical_address, value);"
            rd = replace_once(rd, anchor,
                anchor + '\n if(FirestaffTheron23ADActive) FirestaffTheron23ADEmit("mem_read", FirestaffTheron23ADPC, A, value, FirestaffTheron23DA, FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP, FirestaffTheron23DP);',
                "instrumented RdMem return")
        elif "return(HuCPU.PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));" in rd:
            rd = replace_once(rd,
                " return(HuCPU.PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));",
                ' uint8 value = HuCPU.PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF));\n if(FirestaffTheron23ADActive) FirestaffTheron23ADEmit("mem_read", FirestaffTheron23ADPC, A, value, FirestaffTheron23DA, FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP, FirestaffTheron23DP);\n return value;',
                "RdMem return")
        else:
            raise SystemExit("FAIL: unsupported RdMem body; read hook not applied")
    cpu = cpu[:rd_start] + rd + cpu[rd_end:]

    wr_start = cpu.find("static INLINE void WrMem(unsigned int A, uint8 V)")
    wr_end = cpu.find("static INLINE uint8 RdOp(unsigned int A)", wr_start)
    if wr_start < 0 or wr_end < 0:
        raise SystemExit("FAIL: cannot locate complete WrMem function")
    wr = cpu[wr_start:wr_end]
    if "FirestaffTheron23ADEmit(\"mem_write\"" not in wr:
        anchor = " uint8 wmpr = HuCPU.MPR[A >> 13];"
        wr = replace_once(wr, anchor,
            anchor + '\n if(FirestaffTheron23ADActive) FirestaffTheron23ADEmit("mem_write", FirestaffTheron23ADPC, A, V, FirestaffTheron23DA, FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP, FirestaffTheron23DP);',
            "WrMem bank lookup")
    cpu = cpu[:wr_start] + wr + cpu[wr_end:]

    # SET_MPR is also the common TAM mapping update path. Insert an additional
    # continued macro line so each update records old and new bank values.
    cpu_lines = cpu.splitlines()
    mpr_anchor = next((i for i, line in enumerate(cpu_lines)
                       if "const unsigned int wmpr = arg_i, wbank = arg_v;" in line), None)
    if mpr_anchor is None or not cpu_lines[mpr_anchor].rstrip().endswith("\\"):
        raise SystemExit("FAIL: SET_MPR macro anchor mismatch")
    cpu_lines.insert(mpr_anchor + 1,
        ' if(FirestaffTheron23ADActive && FirestaffTheron23ADEvents < FirestaffTheron23ADEventLimit) fprintf(FirestaffTheron23ADFile, "seq=%u kind=mpr_change pc=%04x slot=%u old=%02x new=%02x\\n", FirestaffTheron23ADEvents++, FirestaffTheron23ADPC, wmpr, HuCPU.MPR[wmpr], wbank); ' + chr(92))
    cpu = "\n".join(cpu_lines) + "\n"

    # Route stack, indirect pointer, zero-page reads and writes through page-1
    # helpers.  The two remaining direct page-1 data accesses are in ops.inc.
    replacements = {
        'HU_Page1[0x100 + HU_S] = V;': 'TRACE_PAGE1_WRITE(0x100 + HU_S, V);',
        '#define POP() HU_Page1[0x100 + ++HU_S]': '#define POP() TRACE_PAGE1_READ(0x100 + ++HU_S)',
        'target=HU_Page1[tmp];': 'target=TRACE_PAGE1_READ(tmp);',
        'target|=HU_Page1[tmp]<<8;': 'target|=TRACE_PAGE1_READ(tmp)<<8;',
        'target|=HU_Page1[tmp] <<8;': 'target|=TRACE_PAGE1_READ(tmp) <<8;',
        'rt=HU_Page1[tmp];': 'rt=TRACE_PAGE1_READ(tmp);',
        'rt|=HU_Page1[tmp]<<8;': 'rt|=TRACE_PAGE1_READ(tmp)<<8;',
        'x=HU_Page1[EA];': 'x=TRACE_PAGE1_READ(EA);',
        'HU_Page1[EA] = x;': 'TRACE_PAGE1_WRITE(EA, x);',
        'HU_Page1[EA] = r;': 'TRACE_PAGE1_WRITE(EA, r);',
    }
    for old, new in replacements.items():
        if old not in cpu:
            raise SystemExit(f"FAIL: missing direct page-1 hook point: {old}")
        cpu = cpu.replace(old, new)
    # Some Firestaff Mednafen source copies already carry main-RAM trace hooks
    # that read Page1 inline. Preserve those calls, but instrument their reads
    # and all remaining direct Page1 writes as well.
    cpu, write_count = re.subn(
        r"HU_Page1\[([^\]]+)\](\s*/\*.*?\*/\s*)=\s*([^;]+);",
        r"TRACE_PAGE1_WRITE(\1, \3);\2",
        cpu,
    )
    cpu, plain_write_count = re.subn(
        r"HU_Page1\[([^\]]+)\]\s*=\s*([^;]+);",
        r"TRACE_PAGE1_WRITE(\1, \2);",
        cpu,
    )
    # Lvalue writes are now gone; remaining indexing expressions are reads.
    cpu, read_count = re.subn(
        r"HU_Page1\[([^\]]+)\]",
        r"TRACE_PAGE1_READ(\1)",
        cpu,
    )
    if not (write_count or plain_write_count or read_count):
        raise SystemExit("FAIL: no direct Page1 accesses were instrumented")
    if "HU_Page1[tmp]" in cpu or "HU_Page1[EA]" in cpu or "HU_Page1[0x100" in cpu:
        raise SystemExit("FAIL: uninstrumented direct page-1 access remains in huc6280.cpp")
    ops = replace_once(ops, "HU_A = HU_Page1[HU_X];",
                       "HU_A = TRACE_PAGE1_READ(HU_X);", "opcode 0xA5 page-1 read")
    ops = replace_once(ops, "HU_Page1[HU_X] /*PAGE1_W[HU_X]*/ =  HU_A;",
                       "TRACE_PAGE1_WRITE(HU_X, HU_A); /*PAGE1_W[HU_X]*/",
                       "opcode 0x85 page-1 write")
    if "HU_Page1[" in ops:
        raise SystemExit("FAIL: uninstrumented direct page-1 access remains in huc6280_ops.inc")

    # Log TAM's mask before its per-slot SET_MPR updates.
    cpu_lines = cpu.splitlines()
    tam_anchor = next((i for i, line in enumerate(cpu_lines)
                       if line.startswith("#define TAM ")), None)
    if tam_anchor is None or not cpu_lines[tam_anchor].rstrip().endswith("\\"):
        raise SystemExit("FAIL: TAM macro anchor mismatch")
    cpu_lines[tam_anchor] = cpu_lines[tam_anchor].replace(
        "#define TAM     for(int i = 0; i < 8; i ++) {",
        '#define TAM     if(FirestaffTheron23ADActive) FirestaffTheron23ADEmit("tam_mask", FirestaffTheron23ADPC, 0, x, FirestaffTheron23DA, FirestaffTheron23DX, FirestaffTheron23DY, FirestaffTheron23DSP, FirestaffTheron23DP); for(int i = 0; i < 8; i ++) {')
    cpu = "\n".join(cpu_lines) + "\n"

    cpu = replace_once(cpu,
        '           #include "huc6280_ops.inc"\n          }',
        '           #include "huc6280_ops.inc"\n          }\n          FirestaffTheron23ADEndInstruction();',
        "instruction dispatch end")
    cpu = replace_once(cpu,
        '\tGetOutBMT:\n\n\tSAVE_LOCALS();',
        '\tGetOutBMT:\n\n\tFirestaffTheron23ADEndInstruction();\n\tSAVE_LOCALS();',
        "HuC6280_Run yield boundary")

    cpu = replace_once(cpu,
        'static INLINE uint8 RdOp(unsigned int A)\n{\n return *(uint8*)(HuCPU.FastPageR[A >> 13] + A);\n}',
        '''static INLINE uint8 RdOp(unsigned int A)
{
 return *(uint8*)(HuCPU.FastPageR[A >> 13] + A);
}

static uint8 FirestaffTheron23ADFetch(uintptr_t raw_pc, uint8 a, uint8 x,
 uint8 y, uint8 s, uint8 p)
{
 uint32 pc;
 uint8 value;
#ifdef HUC6280_CRAZY_VERSION
 pc = (uint32)(raw_pc - HuCPU.PC_base);
 value = *(uint8*)raw_pc;
#else
 pc = (uint32)raw_pc;
 value = RdOp(pc);
#endif
 const uint8 slot = pc >> 13;
 const uint8 bank = HuCPU.MPR[slot];
 const uint32 physical_pc = ((uint32)bank << 13) | (pc & 0x1fff);
 /* The static source projection does not prove a live MPR mapping. First
  * match the caller's cheap five-opcode prefix, then hash all 30 retail bytes
  * only at its BSR candidate. */
 if(!FirestaffTheron23ADStarted && !FirestaffTheron23ADCallerPC &&
    FirestaffTheron23ADEXpectsOpcode)
 {
  static const uint8 expected_opcodes[4] = { 0x8d, 0xa5, 0x8d, 0x44 };
  static const uint8 expected_offsets[4] = { 2, 5, 7, 10 };
  if(FirestaffTheron23ADCallerCandidateStage)
  {
   const unsigned index = FirestaffTheron23ADCallerCandidateStage - 1;
   if(pc == FirestaffTheron23ADCallerCandidatePC + expected_offsets[index] &&
      value == expected_opcodes[index])
   {
    if(FirestaffTheron23ADCallerCandidateStage < 4)
     FirestaffTheron23ADCallerCandidateStage++;
    else
    {
     const uint16 caller_pc = FirestaffTheron23ADCallerCandidatePC;
     const uint16 call_pc = caller_pc + 10;
     uint32 caller_hash = 0x811c9dc5;
     for(unsigned i = 0; i < 30; i++)
      caller_hash = (caller_hash ^ RdOp(caller_pc + i)) * 0x01000193;
     const uint8 call_operand = RdOp(call_pc + 1);
     const uint16 target_pc = (call_pc + 2 + (int8)call_operand) & 0xffff;
     if(caller_hash == 0x699e8da1 && target_pc == (uint16)(caller_pc + 0x27))
     {
      FirestaffTheron23ADOpen();
      if(FirestaffTheron23ADFile)
      {
       FirestaffTheron23ADCallerPC = caller_pc;
       FirestaffTheron23ADRoutinePC = target_pc;
       FirestaffTheron23ADReturnPC = call_pc + 2;
       FirestaffTheron23ADStackPC = call_pc + 1;
       fprintf(FirestaffTheron23ADFile,
        "caller_signature fnv1a=699e8da1 logical=%04x call=%04x target=%04x return=%04x stack_return=%04x mpr=",
        caller_pc, call_pc, target_pc, FirestaffTheron23ADReturnPC,
        FirestaffTheron23ADStackPC);
       for(unsigned i = 0; i < 8; i++)
        fprintf(FirestaffTheron23ADFile, "%s%02x", i ? "," : "", HuCPU.MPR[i]);
       const uint8 caller_slot = caller_pc >> 13;
       fprintf(FirestaffTheron23ADFile, " physical=%06x\\n",
        ((uint32)HuCPU.MPR[caller_slot] << 13) | (caller_pc & 0x1fff));
      }
     }
     FirestaffTheron23ADCallerCandidateStage = 0;
    }
   }
   else
    FirestaffTheron23ADCallerCandidateStage = 0;
  }
  if(!FirestaffTheron23ADCallerCandidateStage && value == 0xa5 &&
     pc <= 0xffd8)
  {
   FirestaffTheron23ADCallerCandidatePC = pc;
   FirestaffTheron23ADCallerCandidateStage = 1;
  }
 }
 if(FirestaffTheron23ADCallerPC && !FirestaffTheron23ADStarted &&
    FirestaffTheron23ADEXpectsOpcode && pc == FirestaffTheron23ADRoutinePC &&
    value == 0xa5 && pc <= 0xfe81 &&
    HuCPU.Page1[0x100 + (uint8)(s + 1)] ==
        (uint8)FirestaffTheron23ADStackPC &&
    HuCPU.Page1[0x100 + (uint8)(s + 2)] ==
        (uint8)(FirestaffTheron23ADStackPC >> 8))
 {
  uint32 routine_hash = 0x811c9dc5;
  for(unsigned i = 0; i < 382; i++)
   routine_hash = (routine_hash ^ RdOp(pc + i)) * 0x01000193;
  if(routine_hash == 0x3056f96c)
  {
   if(FirestaffTheron23ADFile)
   {
    FirestaffTheron23ADActive = true;
    FirestaffTheron23ADStarted = true;
    fprintf(FirestaffTheron23ADFile,
     "call=BSR caller=%04x target=%04x routine_fnv1a=3056f96c return=%04x target_mpr=%02x target_physical=%06x\\n",
     FirestaffTheron23ADCallerPC, pc, FirestaffTheron23ADReturnPC,
     HuCPU.MPR[slot], physical_pc);
   }
  }
 }
 if(FirestaffTheron23ADActive)
 {
  FirestaffTheron23ADPC = FirestaffTheron23ADEXpectsOpcode ? pc : FirestaffTheron23ADPC;
  FirestaffTheron23DA = a; FirestaffTheron23DX = x;
  FirestaffTheron23DY = y; FirestaffTheron23DSP = s; FirestaffTheron23DP = p;
  FirestaffTheron23ADEmit(FirestaffTheron23ADEXpectsOpcode ? "instruction" : "instruction_byte",
   pc, pc, value, a, x, y, s, p);
  /* $2429 is offset $7c within the byte-backed routine. */
  if(FirestaffTheron23ADEXpectsOpcode &&
     pc == (uint32)FirestaffTheron23ADRoutinePC + 0x7cu &&
     value == 0x60 &&
     HuCPU.Page1[0x100 + (uint8)(s + 1)] ==
         (uint8)FirestaffTheron23ADStackPC &&
     HuCPU.Page1[0x100 + (uint8)(s + 2)] ==
         (uint8)(FirestaffTheron23ADStackPC >> 8))
  {
   FirestaffTheron23ADEmit("return_rts", pc, pc, value, a, x, y, s, p);
   FirestaffTheron23ADReturnPending = true;
  }
 }
 (void)bank;
 FirestaffTheron23ADEXpectsOpcode = false;
 return value;
}

static void FirestaffTheron23ADEndInstruction(void)
{
 FirestaffTheron23ADEXpectsOpcode = true;
 if(FirestaffTheron23ADReturnPending)
 {
  FirestaffTheron23ADActive = false;
  FirestaffTheron23ADComplete = true;
  FirestaffTheron23ADReturnPending = false;
  if(FirestaffTheron23ADFile) fflush(FirestaffTheron23ADFile);
 }
}''', "RdOp function")

    cpu_path.write_text(cpu)
    ops_path.write_text(ops)
    print("PASS: applied opt-in bounded HuC6280 $2390->$23AD trace hooks")


if __name__ == "__main__":
    main()
