#include "dialog_keyboard_auto_choice_pc34_compat.h"
const char* dialog_keyboard_auto_choice_GetEvidence(void) { return "DIALOG.C:360-388 initializes selected choice to 99, loops processQueue plus M526_WaitVerticalBlank, accepts keyboard return/enter only when choiceCount == 1, and otherwise keeps waiting; DIALOG.C:381-386 contains the floppy auto-choice dead/legacy branch."; }
unsigned int dialog_keyboard_auto_choice_GetInvariant(void) { return 1u; }
