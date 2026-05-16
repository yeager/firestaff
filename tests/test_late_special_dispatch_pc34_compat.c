#include <stdio.h>

#include "late_special_dispatch_pc34_compat.h"

static int test_dispatch_marks_known_special_non_bitmap(void) {
    struct LateSpecialDispatchResult_Compat result;
    if (!F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(694, &result)) {
        return 0;
    }
    return result.kind == LATE_GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP &&
           !result.shouldUseBitmapPath &&
           result.shouldSkipBitmapExport;
}

static int test_dispatch_marks_known_empty_entries(void) {
    struct LateSpecialDispatchResult_Compat result;
    if (!F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(698, &result)) {
        return 0;
    }
    return result.kind == LATE_GRAPHICS_DAT_ENTRY_EMPTY &&
           !result.shouldUseBitmapPath &&
           result.shouldSkipBitmapExport;
}

static int test_dispatch_marks_suspicious_but_exportable_entries(void) {
    struct LateSpecialDispatchResult_Compat result;
    if (!F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(701, &result)) {
        return 0;
    }
    return result.kind == LATE_GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS &&
           result.shouldUseBitmapPath &&
           !result.shouldSkipBitmapExport;
}

static int test_dispatch_leaves_normal_bitmap_entries_on_bitmap_path(void) {
    struct LateSpecialDispatchResult_Compat result;
    if (!F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(693, &result)) {
        return 0;
    }
    return result.kind == LATE_GRAPHICS_DAT_ENTRY_BITMAP_SAFE &&
           result.shouldUseBitmapPath &&
           !result.shouldSkipBitmapExport;
}

int main(void) {
    if (!test_dispatch_marks_known_special_non_bitmap()) {
        fprintf(stderr, "test_dispatch_marks_known_special_non_bitmap failed\n");
        return 1;
    }
    if (!test_dispatch_marks_known_empty_entries()) {
        fprintf(stderr, "test_dispatch_marks_known_empty_entries failed\n");
        return 1;
    }
    if (!test_dispatch_marks_suspicious_but_exportable_entries()) {
        fprintf(stderr, "test_dispatch_marks_suspicious_but_exportable_entries failed\n");
        return 1;
    }
    if (!test_dispatch_leaves_normal_bitmap_entries_on_bitmap_path()) {
        fprintf(stderr, "test_dispatch_leaves_normal_bitmap_entries_on_bitmap_path failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
