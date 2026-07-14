#include "redmcsb_f0540_input_crawcin_pc34_compat.h"

#include <stddef.h>

uint16_t redmcsb_f0540_input_crawcin_pc34_compat(
    ReDMCSBF0540InputCrawcinPc34Compat *input)
{
    uint16_t key;

    if (input == NULL || input->get_keyboard_input == NULL) {
        return 0U;
    }

    /* ReDMCSB IO2.C:37 reads IODRV_00_GetKeyboardInput once. */
    key = input->get_keyboard_input(input->keyboard_input_context);

    /* ReDMCSB IO2.C:40-45, MEDIA707_I34E_I34M. */
    if (key == 0x0C53U || key == 0x0410U) {
        input->exit_game_immediately = true;
    }

    /* ReDMCSB IO2.C:47-59. Only shifted extended arrows are translated. */
    switch (key - 0x1248U) {
    case 0U:
        return (uint16_t)'L';
    case 8U:
        return (uint16_t)'P';
    case 3U:
        return (uint16_t)'K';
    case 5U:
        return (uint16_t)'M';
    default:
        return key;
    }
}

const char *redmcsb_f0540_input_crawcin_source_evidence_pc34(void)
{
    return "ReDMCSB IO2.C:27-61 F0540_INPUT_Crawcin, MEDIA707_I34E_I34M: "
           "read IODRV_00_GetKeyboardInput once; set "
           "G2151_ExitGameImmediately for 0x0C53 Ctrl-Alt-Del or 0x0410 "
           "Ctrl-Q; normalize 0x1248/0x1250/0x124B/0x124D to L/P/K/M. "
           "COMMAND.C:677-684 consumes K/L/M/P as PC 3.4 movement keys.";
}
