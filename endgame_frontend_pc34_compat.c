#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "endgame_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"
#include <string.h>

void ENDGAME_Compat_ExpandCreditsToScreenBitmap(
const unsigned char* graphic SEPARATOR
unsigned char*       screenBitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapRequired(graphic, screenBitmap);
}


static const char* ENDGAME_Compat_SourceEventLine(EndgameCompatSourceEventKind kind) {
        switch (kind) {
        case ENDGAME_COMPAT_SOURCE_EVENT_OPTIONAL_LOSS_SCREAM_DELAY:
                return "ENDGAME.C:215-217 optional non-winning scream plus F0022_MAIN_Delay(240)";
        case ENDGAME_COMPAT_SOURCE_EVENT_LOAD_END_ASSETS:
                return "ENDGAME.C:241-286 loads C006_GRAPHIC_THE_END, optional C346 mirror, and C005 credits";
        case ENDGAME_COMPAT_SOURCE_EVENT_FADE_TO_BLACK:
                return "ENDGAME.C:307-318 / 316-318 fade or curtain to black before end screens";
        case ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_RENDER:
                return "ENDGAME.C:327-394 draws champion mirrors, portraits, names, titles, skill lines";
        case ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_WAIT_INPUT:
                return "ENDGAME.C:396-424 fades champion summary then waits for keyboard/mouse input";
        case ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT:
                return "ENDGAME.C:440-456 clears screen, starts music, blits C006 THE END; DATA.C:575 box {120,199,95,108}";
        case ENDGAME_COMPAT_SOURCE_EVENT_THE_END_PALETTE_FADE:
                return "ENDGAME.C:458-467 prepares dark-blue palette, sets color 15 white, fades in";
        case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY:
                return "ENDGAME.C:472-477 CHANGE7_37 waits F0022_MAIN_Delay(300) before restart controls";
        case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER:
                return "ENDGAME.C:484-543 draws restart/quit boxes and text when restart is allowed; DATA.C restart/quit boxes";
        case ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT:
                return "ENDGAME.C:564-568 waits 900 VBlanks/delay ticks for restart input before credits";
        case ENDGAME_COMPAT_SOURCE_EVENT_CREDITS_FADE:
                return "ENDGAME.C:639-680 fades to dark blue/credits palette and continues credits path";
        }
        return "ENDGAME.C source event";
}

unsigned int ENDGAME_Compat_GetSourceAnimationStepCount(void) {
        return 11u;
}

int ENDGAME_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                          EndgameCompatSourceAnimationStep* outStep) {
        EndgameCompatSourceAnimationStep step;
        if (!outStep || sourceStepOrdinal == 0u || sourceStepOrdinal > ENDGAME_Compat_GetSourceAnimationStepCount()) return 0;
        memset(&step, 0, sizeof(step));
        step.sourceStepOrdinal = sourceStepOrdinal;
        switch (sourceStepOrdinal) {
        case 1u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_OPTIONAL_LOSS_SCREAM_DELAY;
                step.delayTicks = 240u;
                break;
        case 2u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_LOAD_END_ASSETS;
                break;
        case 3u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_FADE_TO_BLACK;
                break;
        case 4u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_RENDER;
                step.x = 0u; step.y = 0u; step.width = 320u; step.height = 192u;
                break;
        case 5u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_WAIT_INPUT;
                break;
        case 6u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT;
                step.x = 120u; step.y = 95u; step.width = 80u; step.height = 14u;
                break;
        case 7u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_THE_END_PALETTE_FADE;
                break;
        case 8u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY;
                step.delayTicks = 300u;
                break;
        case 9u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER;
                step.x = 103u; step.y = 145u; step.width = 115u; step.height = 15u;
                break;
        case 10u:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT;
                step.vblankLoopCount = 900u;
                break;
        default:
                step.kind = ENDGAME_COMPAT_SOURCE_EVENT_CREDITS_FADE;
                break;
        }
        step.sourceLineEvidence = ENDGAME_Compat_SourceEventLine(step.kind);
        *outStep = step;
        return 1;
}

const char* ENDGAME_Compat_GetSourceAnimationEvidence(void) {
        return "ReDMCSB ENDGAME.C:F0444_STARTEND_Endgame PC/F20 path: optional 240-tick loss delay, load THE_END/mirror/credits assets, fade black, optional champion summary and input wait when game won, blit THE END at DATA.C box {120,199,95,108}, fade dark-blue/white palette, delay 300, draw restart controls, wait 900 ticks/VBlanks, then credits fade/path.";
}
