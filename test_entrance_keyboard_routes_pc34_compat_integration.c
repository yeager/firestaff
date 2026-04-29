#include <stdio.h>
#include "entrance_keyboard_routes_pc34_compat.h"
int main(void) { unsigned int ok = entrance_keyboard_routes_GetInvariant(); printf("probe=firestaff_entrance_keyboard_routes\n"); printf("sourceEvidence=%s\n", entrance_keyboard_routes_GetEvidence()); printf("entranceKeyboardRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
