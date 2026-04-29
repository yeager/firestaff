#include <stdio.h>
#include "dialog_keyboard_auto_choice_pc34_compat.h"
int main(void) { unsigned int ok = dialog_keyboard_auto_choice_GetInvariant(); printf("probe=firestaff_dialog_keyboard_auto_choice\n"); printf("sourceEvidence=%s\n", dialog_keyboard_auto_choice_GetEvidence()); printf("dialogKeyboardAutoChoiceInvariantOk=%u\n", ok); return ok ? 0 : 1; }
