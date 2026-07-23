#ifndef FIRESTAFF_DM1_V1_F0432_FORMAT_DISK_MENU_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0432_FORMAT_DISK_MENU_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_f0424_f0427_dialog_admission_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0432FormatDiskStagePc34 {
    DM1_V1_F0432_FORMAT_DISK_STAGE_INSERT_BLANK = 0,
    DM1_V1_F0432_FORMAT_DISK_STAGE_CONFIRM_SAVE_DISK = 1,
    DM1_V1_F0432_FORMAT_DISK_STAGE_FORMATTING = 2,
    DM1_V1_F0432_FORMAT_DISK_STAGE_FORMAT_FAILED = 3
} DM1_V1_F0432FormatDiskStagePc34;

typedef enum DM1_V1_F0432DiskTypePc34 {
    DM1_V1_F0432_DISK_GAME = 0,
    DM1_V1_F0432_DISK_SAVE_WRITE_ENABLED = 1,
    DM1_V1_F0432_DISK_SAVE_WRITE_PROTECTED = 2,
    DM1_V1_F0432_DISK_UNFORMATTED = 3,
    DM1_V1_F0432_DISK_NONE = 4
} DM1_V1_F0432DiskTypePc34;

typedef enum DM1_V1_F0432FormatDiskDispositionPc34 {
    DM1_V1_F0432_FORMAT_DISK_REJECTED = 0,
    DM1_V1_F0432_FORMAT_DISK_CANCELLED,
    DM1_V1_F0432_FORMAT_DISK_SHOW_GAME_DISK_ERROR,
    DM1_V1_F0432_FORMAT_DISK_SHOW_WRITE_PROTECTED_ERROR,
    DM1_V1_F0432_FORMAT_DISK_SHOW_NO_DISK_ERROR,
    DM1_V1_F0432_FORMAT_DISK_REQUIRE_SAVE_DISK_CONFIRMATION,
    DM1_V1_F0432_FORMAT_DISK_READY_FOR_BACKEND,
    DM1_V1_F0432_FORMAT_DISK_SHOW_FORMAT_FAILED,
    DM1_V1_F0432_FORMAT_DISK_COMPLETE,
    DM1_V1_F0432_FORMAT_DISK_RETURN_TO_PROMPT
} DM1_V1_F0432FormatDiskDispositionPc34;

typedef enum DM1_V1_F0432SourceTextIdPc34 {
    DM1_V1_F0432_TEXT_PUT_BLANK_DISK = 1,
    DM1_V1_F0432_TEXT_FORMAT_FLOPPY,
    DM1_V1_F0432_TEXT_CANCEL,
    DM1_V1_F0432_TEXT_GAME_SAVE_DISK,
    DM1_V1_F0432_TEXT_FORMAT_ANYWAY,
    DM1_V1_F0432_TEXT_OK,
    DM1_V1_F0432_TEXT_FORMATTING_DISK,
    DM1_V1_F0432_TEXT_UNABLE_TO_FORMAT
} DM1_V1_F0432SourceTextIdPc34;

typedef struct DM1_V1_F0432SourceTextPc34 {
    DM1_V1_F0432SourceTextIdPc34 id;
    DM1_V1_F0424F0427DialogTextPc34 text;
} DM1_V1_F0432SourceTextPc34;

typedef struct DM1_V1_F0432FormatDiskRequestPc34 {
    DM1_V1_F0432FormatDiskStagePc34 stage;
    const DM1_V1_F0424F0427DialogMaterialPc34 *material;
    const DM1_V1_F0432SourceTextPc34 *texts[4];
    int textCount;
    int selectedChoice;
    int selectedChoiceFromSourceInput;
    DM1_V1_F0432DiskTypePc34 diskType;
    int diskTypeReceiptValid;
    uint32_t availableHeapBytes;
    int sourceMemoryReceiptValid;
    int formatResultReceiptValid;
    int formatSucceeded;
} DM1_V1_F0432FormatDiskRequestPc34;

typedef struct DM1_V1_F0432FormatDiskReceiptPc34 {
    int accepted;
    DM1_V1_F0432FormatDiskStagePc34 stage;
    DM1_V1_F0432FormatDiskDispositionPc34 disposition;
    int choiceCount;
    int usesOriginalDialogLayout;
    int requiresOriginalMemoryBuffer;
    uint32_t originalMemoryBufferBytes;
    int suppressSyntheticFallback;
    DM1_V1_F0424F0427DialogReceiptPc34 dialogReceipt;
} DM1_V1_F0432FormatDiskReceiptPc34;

/* ReDMCSB LOADSAVE.C F0432. This is an admission/receipt boundary only: it
 * accepts original dialog material, source text, source disk state, and the
 * original heap receipt. It never formats media, draws a host dialog, or
 * substitutes a font/text surface. */
int dm1_v1_f0432_format_disk_menu_admit_pc34(
    const DM1_V1_F0432FormatDiskRequestPc34 *request,
    DM1_V1_F0432FormatDiskReceiptPc34 *outReceipt);

const char *dm1_v1_f0432_format_disk_menu_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
