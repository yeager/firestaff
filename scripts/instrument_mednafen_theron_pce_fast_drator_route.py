#!/usr/bin/env python3
"""Add the source-signature-gated Theron menu research route to Mednafen PCE Fast."""

from __future__ import annotations

import pathlib
import sys


ROUTE_HELPERS = r'''

/* Research-only route through the authenticated original menu handlers.
 * Every state-changing write requires the same instruction signatures as
 * the standard PCE reference route. */
static unsigned TheronFastDratorMenuStage;
static unsigned TheronFastDratorScenarioStep;
static unsigned TheronFastDratorOverlayAcceptCount;
static unsigned TheronFastDratorTwoChoiceStep;
static unsigned TheronFastDratorPostScenarioAcceptCount;
static unsigned TheronFastDratorGeneratorRouteStage;
static unsigned TheronFastDratorGeneratorRouteDelay;
static unsigned TheronFastDratorGeneratorInputStartFrame;

static void TheronPCEFastDratorMenuRouteStep(unsigned pc)
{
 const char *route = getenv("FIRESTAFF_THERON_MENU_ROUTE");
 if(!route || (strcmp(route, "drator") &&
                strcmp(route, "drator-generator"))) return;
 if(TheronFastDratorMenuStage == 0 && pc == 0x7557 &&
    RdMem(0x7552) == 0x44 && RdMem(0x7553) == 0xf5 &&
    RdMem(0x7554) == 0x20 && RdMem(0x7555) == 0x63 &&
    RdMem(0x7556) == 0xe0 && RdMem(0x7557) == 0xad &&
    RdMem(0x7558) == 0x2d && RdMem(0x7559) == 0x22)
 {
  WrMem(0x222d, 0x01);
  TheronFastDratorMenuStage = 1;
  fprintf(stderr, "Theron Drator route: title input pc=%04x\n", pc);
  fflush(stderr);
 }
 else if(TheronFastDratorMenuStage <= 1 && pc == 0x6e44 &&
         RdMem(0x6e3e) == 0x20 && RdMem(0x6e3f) == 0x49 &&
         RdMem(0x6e40) == 0x75 && RdMem(0x6e41) == 0x20 &&
         RdMem(0x6e42) == 0x63 && RdMem(0x6e43) == 0xe0 &&
         RdMem(0x6e44) == 0xad && RdMem(0x6e45) == 0x28 &&
         RdMem(0x6e46) == 0x22)
 {
  WrMem(0x2228, TheronFastDratorTwoChoiceStep++ == 0 ? 0x10 : 0x01);
  if(TheronFastDratorTwoChoiceStep == 2)
   TheronFastDratorMenuStage = 2;
  fprintf(stderr, "Theron Drator route: two-choice step=%u pc=%04x\n",
          TheronFastDratorTwoChoiceStep, pc);
  fflush(stderr);
 }
 else if(TheronFastDratorMenuStage >= 2 && TheronFastDratorMenuStage <= 4 &&
         TheronFastDratorOverlayAcceptCount < 32 && pc == 0x5c97 &&
         RdMem(0x5c94) == 0x20 && RdMem(0x5c95) == 0x63 &&
         RdMem(0x5c96) == 0xe0 && RdMem(0x5c97) == 0xad &&
         RdMem(0x5c98) == 0x2d && RdMem(0x5c99) == 0x22)
 {
  WrMem(0x222d, 0x01);
  if(TheronFastDratorMenuStage == 2)
   TheronFastDratorMenuStage = 3;
  TheronFastDratorOverlayAcceptCount++;
  fprintf(stderr, "Theron Drator route: overlay accept=%u pc=%04x\n",
          TheronFastDratorOverlayAcceptCount, pc);
  fflush(stderr);
 }
 else if(TheronFastDratorMenuStage == 3 && pc == 0x6dbd &&
         RdMem(0x6dba) == 0x20 && RdMem(0x6dbb) == 0x63 &&
         RdMem(0x6dbc) == 0xe0 && RdMem(0x6dbd) == 0xad &&
         RdMem(0x6dbe) == 0x28 && RdMem(0x6dbf) == 0x22)
 {
  WrMem(0x2228, 0x01);
  TheronFastDratorScenarioStep++;
  TheronFastDratorMenuStage = 4;
  fprintf(stderr, "Theron Drator route: scenario step=%u pc=%04x\n",
          TheronFastDratorScenarioStep, pc);
  fflush(stderr);
 }
 else if(TheronFastDratorMenuStage == 4 &&
         TheronFastDratorPostScenarioAcceptCount < 32 && pc == 0x7557 &&
         RdMem(0x7552) == 0x44 && RdMem(0x7553) == 0xf5 &&
         RdMem(0x7554) == 0x20 && RdMem(0x7555) == 0x63 &&
         RdMem(0x7556) == 0xe0 && RdMem(0x7557) == 0xad &&
         RdMem(0x7558) == 0x2d && RdMem(0x7559) == 0x22)
 {
  WrMem(0x222d, 0x01);
  TheronFastDratorPostScenarioAcceptCount++;
  fprintf(stderr, "Theron Drator route: post-scenario accept=%u pc=%04x\n",
          TheronFastDratorPostScenarioAcceptCount, pc);
  fflush(stderr);
 }

 if(!strcmp(route, "drator-generator") && TheronFastDratorMenuStage == 4)
 {
  const uint8 level = RdMem(0x2031);
  const uint8 direction = RdMem(0x203f);
  const uint8 party_x = RdMem(0x2040);
  const uint8 party_y = RdMem(0x2041);
  if(TheronFastDratorGeneratorRouteStage == 0 && level == 0x02 &&
     direction == 0x01 && party_x == 0x02 && party_y == 0x03)
  {
   TheronFastDratorGeneratorRouteStage = 1;
   TheronFastDratorGeneratorRouteDelay = 0;
   TheronFastDratorGeneratorInputStartFrame = TheronPCEFastScriptFrame;
   fprintf(stderr, "Theron Drator generator route: start level=%02x x=%02x y=%02x direction=%02x input_frame=%u\n",
           level, party_x, party_y, direction, TheronPCEFastScriptFrame);
   fflush(stderr);
  }
  else if(TheronFastDratorGeneratorRouteStage == 1 &&
          (party_x != 0x02 || party_y != 0x03))
  {
   TheronFastDratorGeneratorRouteStage = 2;
   TheronFastDratorGeneratorRouteDelay = 0;
   fprintf(stderr, "Theron Drator generator route: left-event x=%02x y=%02x direction=%02x input_frame=%u\n",
           party_x, party_y, direction, TheronPCEFastScriptFrame);
   fflush(stderr);
  }
  else if(TheronFastDratorGeneratorRouteStage == 2 &&
          ++TheronFastDratorGeneratorRouteDelay >= 5000)
  {
   TheronFastDratorGeneratorRouteStage = 3;
   TheronFastDratorGeneratorRouteDelay = 0;
   TheronFastDratorGeneratorInputStartFrame = TheronPCEFastScriptFrame;
  }
  else if(TheronFastDratorGeneratorRouteStage == 3 &&
          party_x == 0x02 && party_y == 0x03)
  {
   TheronFastDratorGeneratorRouteStage = 8;
   fprintf(stderr, "Theron Drator generator route: reentered-event level=%02x x=%02x y=%02x direction=%02x\n",
           level, party_x, party_y, direction);
   fflush(stderr);
  }
 }
}

uint16 TheronPCEFastDratorGeneratorInputMask(void)
{
 const char *route = getenv("FIRESTAFF_THERON_MENU_ROUTE");
 if(!route || strcmp(route, "drator-generator")) return 0;
 if(TheronFastDratorGeneratorRouteStage != 1 &&
    TheronFastDratorGeneratorRouteStage != 3) return 0;
 const unsigned route_frame = TheronPCEFastScriptFrame -
                              TheronFastDratorGeneratorInputStartFrame;
 const unsigned warmup = TheronFastDratorGeneratorRouteStage == 1 ? 900 : 120;
 if(route_frame <= warmup) return 0;
 const unsigned frame = route_frame - warmup;
 uint16 mask = 0;
 if(TheronFastDratorGeneratorRouteStage == 3 && frame <= 240)
  return 0x0010;
 const unsigned scan_frame = frame - 1 -
     (TheronFastDratorGeneratorRouteStage == 3 ? 240 : 0);
 const unsigned row = scan_frame / 270;
 const unsigned row_frame = scan_frame % 270;
 if(row < 10)
 {
  if(row_frame < 240)
  {
   const unsigned sample_frame = row_frame % 20;
   mask |= sample_frame < 15 ? ((row & 1) ? 0x0080 : 0x0020) : 0x0001;
  }
  else mask |= 0x0040;
 }
 return mask;
}
'''


def replace_once(path: pathlib.Path, source: str, anchor: str, replacement: str) -> str:
    occurrences = source.count(anchor)
    if occurrences != 1:
        raise ValueError(f"{path}: expected exactly one anchor, found {occurrences}")
    return source.replace(anchor, replacement, 1)


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {pathlib.Path(sys.argv[0]).name} MEDNAFEN_SOURCE_ROOT", file=sys.stderr)
        return 2

    root = pathlib.Path(sys.argv[1])
    huc_path = root / "src/pce_fast/huc6280.cpp"
    input_path = root / "src/pce_fast/input.cpp"
    try:
        huc = huc_path.read_text()
        input_source = input_path.read_text()
        marker = "TheronPCEFastDratorMenuRouteStep(TheronTraceInstructionPC);"
        if marker in huc or "TheronPCEFastDratorGeneratorInputMask" in input_source:
            raise ValueError("PCE Fast Drator route is already instrumented")

        wrmem_ends = (
            " HuCPU.PCEWrite[wmpr](physical, V);\n}\n",
            " HuCPU.PCEWrite[wmpr]((wmpr << 13) | (A & 0x1FFF), V);\n}\n",
        )
        matching_wrmem_ends = [anchor for anchor in wrmem_ends if huc.count(anchor) == 1]
        if len(matching_wrmem_ends) != 1:
            raise ValueError(f"{huc_path}: expected one known PCE Fast WrMem tail")
        wrmem_end = matching_wrmem_ends[0]
        huc = replace_once(huc_path, huc, wrmem_end, wrmem_end + ROUTE_HELPERS)

        trace_anchor = (
            "  HuCPU.IRQMaskDelay = HuCPU.IRQMask;\n\n"
            "  TheronTraceInstructionPC = GetRealPC();\n"
            "  TheronTraceInstructionPhysicalPC = "
            "(HuCPU.MPR[TheronTraceInstructionPC >> 13] << 13) | "
            "(TheronTraceInstructionPC & 0x1FFF);\n"
        )
        huc = replace_once(
            huc_path,
            huc,
            trace_anchor,
            trace_anchor + "  " + marker + "\n",
        )

        input_anchor = "namespace MDFN_IEN_PCE_FAST\n{\n"
        input_source = replace_once(
            input_path,
            input_source,
            input_anchor,
            input_anchor + "extern uint16 TheronPCEFastDratorGeneratorInputMask(void);\n",
        )
        data_anchor = "   if(x == (int)scripted_port) new_data |= scripted_mask;\n"
        input_source = replace_once(
            input_path,
            input_source,
            data_anchor,
            data_anchor + "   if(x == 0) new_data |= TheronPCEFastDratorGeneratorInputMask();\n",
        )

        huc_path.write_text(huc)
        input_path.write_text(input_source)
    except (OSError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    print("PASS: added signature-gated Drator menu and generator route to PCE Fast")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
