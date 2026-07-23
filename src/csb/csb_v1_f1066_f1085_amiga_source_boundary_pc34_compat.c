#include "csb_v1_f1066_f1085_amiga_source_boundary_pc34_compat.h"

#include <string.h>

typedef struct { CSB_V1_F1066F1085SourceKindPc34 kind; const char *symbol; const char *anchor; } Spec;
#define PLATFORM(symbol, anchor) { CSB_V1_F1066_F1085_PLATFORM_NONAPPLICABLE_PC34, symbol, anchor }
#define OWNER(symbol, anchor) { CSB_V1_F1066_F1085_EXISTING_OWNER_NO_CSB_ADMISSION_PC34, symbol, anchor }
static const Spec kSpecs[] = {
    OWNER("F1066_GetUsableChipMemoryByteCount", "AMIGINIT.C:498-560; existing source owner"),
    PLATFORM("F1067_InitAmigaStuff", "AMIGINIT.C:539-628"),
    PLATFORM("F1068_FreeAmigaStuff", "AMIGINIT.C:630-671"),
    PLATFORM("F1069_OpenDosLibrary", "AMIGINIT.C:67-77,343"),
    PLATFORM("F1070_CloseDosLibrary", "AMIGINIT.C:79"),
    PLATFORM("F1071_OpenGraphicsLibrary", "AMIGINIT.C:88-99,333-361"),
    PLATFORM("F1072_CloseGraphicsLibrary", "AMIGINIT.C:101-108,363-375"),
    PLATFORM("F1073_OpenIntuitionLibrary", "AMIGINIT.C:88-130,333-361"),
    PLATFORM("F1074_CloseIntuitionLibrary", "AMIGINIT.C:122-129,363-385"),
    PLATFORM("F1075_OpenLayersLibrary", "AMIGINIT.C:132-143,333-361"),
    PLATFORM("F1076_CloseLayersLibrary", "AMIGINIT.C:145-152,363-378"),
    PLATFORM("F1077_OpenConsoleDevice", "AMIGINIT.C:154-180"),
    PLATFORM("F1078_CloseConsoleDevice", "AMIGINIT.C:182-198"),
    PLATFORM("F1079_OpenInputDevice", "AMIGINIT.C:200-215,333-361"),
    PLATFORM("F1080_CloseInputDevice", "AMIGINIT.C:217-232,363-373"),
    PLATFORM("F1081_OpenNIL", "AMIGINIT.C:235-257"),
    PLATFORM("F1082_CloseNIL", "AMIGINIT.C:248-257"),
    PLATFORM("F1083_Allocate724Bytes", "AMIGINIT.C:260-266"),
    PLATFORM("F1084_Free724Bytes", "AMIGINIT.C:268-275"),
    PLATFORM("F1085_IntuitionVectorReplacement", "AMIGINIT.C:277; existing zero callback"),
};

int csb_v1_f1066_f1085_source_boundary_admit_pc34(unsigned int number, CSB_V1_F1066F1085SourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1066F1085SourceBoundaryReceiptPc34 receipt;
    const Spec *spec;
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1066u || number > 1085u) return 0;
    spec = &kSpecs[number - 1066u];
    receipt.function_number = number;
    receipt.source_kind = spec->kind;
    receipt.symbol = spec->symbol;
    receipt.source_anchor = spec->anchor;
    receipt.authentic_pc34_material_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1066_f1085_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C F1066-F1085 is Amiga-specific apart from existing F1066 math; no authenticated CSB PC34 consumer is proven, so all routes fail closed.";
}
