#include "csb_v1_f1126_f1145_source_boundary_pc34_compat.h"
#include <assert.h>
#include <string.h>
int main(void) { CSB_V1_F1126F1145SourceBoundaryReceiptPc34 r; (void)r; unsigned int n; for(n=1126;n<=1145;n++){assert(csb_v1_f1126_f1145_source_boundary_admit_pc34(n,&r)==0);assert(r.function_number==n&&r.symbol&&r.source_anchor&&r.authentic_pc34_material_required&&r.runtime_execution_blocked&&r.no_synthetic_ui_graphics_timing);} assert(csb_v1_f1126_f1145_source_boundary_admit_pc34(1125,&r)==0&&!r.function_number); assert(strstr(csb_v1_f1126_f1145_source_boundary_evidence_pc34(),"all routes fail closed")); return 0; }
