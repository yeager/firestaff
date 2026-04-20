#ifndef REDMCSB_LATE_SPECIAL_DISPATCH_PC34_COMPAT_H
#define REDMCSB_LATE_SPECIAL_DISPATCH_PC34_COMPAT_H

enum LateGraphicsDatEntryKind_Compat {
    LATE_GRAPHICS_DAT_ENTRY_BITMAP_SAFE = 0,
    LATE_GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS = 1,
    LATE_GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP = 2,
    LATE_GRAPHICS_DAT_ENTRY_EMPTY = 3
};

struct LateSpecialDispatchResult_Compat {
    unsigned int graphicIndex;
    enum LateGraphicsDatEntryKind_Compat kind;
    int shouldUseBitmapPath;
    int shouldSkipBitmapExport;
};

int F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(
    unsigned int graphicIndex,
    struct LateSpecialDispatchResult_Compat* outResult);

#endif
