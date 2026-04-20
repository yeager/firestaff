#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_dialog_path_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"

int F0427_DIALOG_DrawPreloadedBackdrop_Compat(
const char*                                     graphicsDatPath    SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState    SEPARATOR
struct MemoryGraphicsDatState_Compat*           fileState          SEPARATOR
unsigned int                                    dialogGraphicIndex SEPARATOR
struct MemoryGraphicsDatSpecials_Compat*        specials           SEPARATOR
unsigned char*                                  viewportBitmap     FINAL_SEPARATOR
{
        if (!F0479_MEMORY_PreloadDialogBoxGraphic_Compat(
                graphicsDatPath,
                runtimeState,
                fileState,
                dialogGraphicIndex,
                specials)) {
                return 0;
        }
        F0427_DIALOG_DrawBackdrop_Compat(
            specials->dialogBoxGraphic,
            viewportBitmap,
            &runtimeState->widthHeight[dialogGraphicIndex]);
        return 1;
}
