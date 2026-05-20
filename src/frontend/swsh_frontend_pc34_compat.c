#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "swsh_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"
#include <string.h>

void SWSH_Compat_ExpandLogoToBitmap(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapRequired(graphic, bitmap);
}


typedef struct SWSH_CompatPaletteCommand {
        unsigned short word;
        unsigned int sourceLine;
} SWSH_CompatPaletteCommand;

static const SWSH_CompatPaletteCommand g_swsh_palette_commands[] = {
        {0x1777u, 282u}, {0x0001u, 283u}, {0x2777u, 284u}, {0x0001u, 285u},
        {0x3777u, 286u}, {0x0002u, 287u}, {0x4777u, 288u}, {0x0002u, 289u},
        {0x5777u, 290u}, {0x0003u, 291u}, {0x6777u, 292u}, {0x0003u, 293u},
        {0x7777u, 294u}, {0x0003u, 295u}, {0x8777u, 296u}, {0x9777u, 297u},
        {0xA555u, 298u}, {0xF777u, 299u}, {0x0003u, 300u}, {0x8000u, 301u},
        {0xB777u, 302u}, {0xC555u, 303u}, {0xD222u, 304u}, {0x0003u, 305u},
        {0xF770u, 306u}, {0xE770u, 307u}
};

static const char* SWSH_Compat_SourceEventLine(SWSH_CompatSourceEventKind kind) {
        switch (kind) {
        case SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP:
                return "SWSH.C:54-68 gets Physbase/screen bounds; SWSH.C:101-143 decodes IMG1 logo commands; SWSH.C:248-250 includes SWSHGDAT.C";
        case SWSH_COMPAT_SOURCE_EVENT_START_SOUND:
                return "SWSH.C:10-14 starts Dosound() with V0901005_SoundCommands before palette animation";
        case SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR:
                return "SWSH.C:15-29 reads a palette word, extracts color index/value, and calls XBIOS Setcolor()";
        case SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS:
                return "SWSH.C:30-38 handles palette wait commands with XBIOS Vsync() loops";
        case SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM:
                return "SWSH.C:39-47 runs START.PRG via GEMDOS Pexec() after the palette command terminator";
        }
        return "SWSH.C source event";
}

unsigned int SWSH_Compat_GetSourceAnimationStepCount(void) {
        return 2u + SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT + 1u;
}

int SWSH_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                       SWSH_CompatSourceAnimationStep* outStep) {
        SWSH_CompatSourceAnimationStep step;
        unsigned int paletteOrdinal;
        unsigned int word;

        if (!outStep || sourceStepOrdinal == 0u || sourceStepOrdinal > SWSH_Compat_GetSourceAnimationStepCount()) return 0;
        memset(&step, 0, sizeof(step));
        step.sourceStepOrdinal = sourceStepOrdinal;

        if (sourceStepOrdinal == 1u) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_LOAD_LOGO_BITMAP;
        } else if (sourceStepOrdinal == 2u) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_START_SOUND;
        } else if (sourceStepOrdinal == SWSH_Compat_GetSourceAnimationStepCount()) {
                step.kind = SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM;
        } else {
                paletteOrdinal = sourceStepOrdinal - 3u;
                word = (unsigned int)g_swsh_palette_commands[paletteOrdinal].word;
                step.sourceLine = g_swsh_palette_commands[paletteOrdinal].sourceLine;
                if (word < 8u) {
                        step.kind = SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS;
                        step.vblankCount = word;
                } else {
                        step.kind = SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR;
                        step.colorIndex = (word >> 12) & 0x0fu;
                        step.colorValue = word & 0x0777u;
                }
        }
        step.sourceLineEvidence = SWSH_Compat_SourceEventLine(step.kind);
        *outStep = step;
        return 1;
}

SWSH_CompatSourceTiming SWSH_Compat_GetSourceTimingEvidence(void) {
        SWSH_CompatSourceTiming timing;
        memset(&timing, 0, sizeof(timing));
        timing.paletteCommandCount = SWSH_COMPAT_SOURCE_PALETTE_COMMAND_COUNT;
        timing.paletteColorSetCount = SWSH_COMPAT_SOURCE_PALETTE_COLOR_SET_COUNT;
        timing.paletteWaitCommandCount = SWSH_COMPAT_SOURCE_PALETTE_WAIT_COMMAND_COUNT;
        timing.paletteWaitVblankCount = SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT;
        timing.soundRegisterWriteCount = SWSH_COMPAT_SOURCE_SOUND_REGISTER_WRITE_COUNT;
        timing.soundWaitCommandCount = SWSH_COMPAT_SOURCE_SOUND_WAIT_COMMAND_COUNT;
        timing.soundWaitVblankCount = SWSH_COMPAT_SOURCE_SOUND_WAIT_VBLANK_COUNT;
        timing.sourceFile = "ReDMCSB_WIP20210206/Toolchains/Common/Source/SWSH.C";
        timing.sourceFunction = "T0901000_PlaySoundAndAnimatePalette";
        timing.evidenceNote = "PC SWSH.C path: expand SWSHGDAT logo to screen, start Dosound() from V0901005_SoundCommands, process 26 V0901006_PaletteCommands as Setcolor()/Vsync events, then Pexec START.PRG.";
        return timing;
}

const char* SWSH_Compat_GetSourceAnimationEvidence(void) {
        return "ReDMCSB SWSH.C PC path: T0901006 expands the FTL logo bitmap to Physbase using SWSHGDAT.C, then T0901000 starts Dosound(), applies the V0901006 palette command sequence with Setcolor()/Vsync, and hands off to START.PRG via Pexec after the zero terminator.";
}
