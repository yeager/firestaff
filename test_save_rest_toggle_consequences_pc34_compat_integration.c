#include <stdio.h>
#include "save_rest_toggle_consequences_pc34_compat.h"
int main(void) { unsigned int ok = save_rest_toggle_consequences_GetInvariant(); printf("probe=firestaff_save_rest_toggle_consequences\n"); printf("sourceEvidence=%s\n", save_rest_toggle_consequences_GetEvidence()); printf("saveRestToggleConsequencesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
