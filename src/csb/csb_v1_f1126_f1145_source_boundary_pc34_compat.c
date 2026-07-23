#include "csb_v1_f1126_f1145_source_boundary_pc34_compat.h"
#include <string.h>
typedef struct { const char *symbol; const char *anchor; } Spec;
static const Spec kSpecs[] = {
 {"L1126_ui_Command", "CLIKCHAM.C:21 F0367"}, {"L1127_ui_LeaderIndex", "CLIKCHAM.C:45 F0368"},
 {"F1128_IsLeftMouseButtonDown", "FILLBOX.C:6; existing input owner"}, {"F1129_", "FILLBOX.C Amiga pattern helper"},
 {"L1130_ui_SymbolIndex", "CLIKMENU.C:356 F0369"}, {"F1131_InvertBox", "CEDTINCN.C:42; no CSB PC34 owner"},
 {"F1132_ConvertBoxCoordinates", "BLIT.C:2036; no CSB PC34 owner"}, {"F1133_AddCopperInterrupt", "AMIGINIT.C:550"},
 {"F1134_RemoveCopperInterrupt", "AMIGINIT.C:668"}, {"F1135_CopperInterrupt_CPSX", "COPERINT.C:9"},
 {"L1136_ui_MapY", "CLIKVIEW.C:13 F0372"}, {"L1137_i_MapX", "CLIKVIEW.C:87 F0373"},
 {"L1138_i_MapY", "CLIKVIEW.C:88 F0373"}, {"L1139_T_Thing", "CLIKVIEW.C:85 F0373"},
 {"F1140_InitializeColorPaletteFullBlack", "AMIGAVID.C:32"}, {"F1141_EnablePlayfieldDisplayAndCopper", "AMIGAVID.C:50"},
 {"L1142_T_Thing", "CLIKVIEW.C:139 F0374"}, {"L1143_ps_Junk", "CLIKVIEW.C:138 F0374"},
 {"L1144_i_IconIndex", "CLIKVIEW.C:145 F0374"}, {"L1145_ui_Cell", "CLIKVIEW.C:146 F0374"},
};
int csb_v1_f1126_f1145_source_boundary_admit_pc34(unsigned int number, CSB_V1_F1126F1145SourceBoundaryReceiptPc34 *out)
{
 CSB_V1_F1126F1145SourceBoundaryReceiptPc34 receipt;
 if (!out) return 0; memset(&receipt,0,sizeof(receipt)); *out=receipt;
 if(number<1126u||number>1145u)return 0;
 receipt.function_number=number; receipt.symbol=kSpecs[number-1126u].symbol; receipt.source_anchor=kSpecs[number-1126u].anchor;
 receipt.authentic_pc34_material_required=1; receipt.runtime_execution_blocked=1; receipt.no_synthetic_ui_graphics_timing=1; *out=receipt; return 0;
}
const char *csb_v1_f1126_f1145_source_boundary_evidence_pc34(void)
{ return "ReDMCSB CLIKCHAM.C, CLIKMENU.C, CLIKVIEW.C, FILLBOX.C, BLIT.C, AMIGINIT.C and AMIGAVID.C F1126-F1145: no authenticated CSB PC34 consumer is proven; all routes fail closed."; }
