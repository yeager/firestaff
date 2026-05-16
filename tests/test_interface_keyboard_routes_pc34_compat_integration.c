#include <stdio.h>
#include "interface_keyboard_routes_pc34_compat.h"
int main(void) { unsigned int ok = interface_keyboard_routes_GetInvariant(); printf("probe=firestaff_interface_keyboard_routes\n"); printf("sourceEvidence=%s\n", interface_keyboard_routes_GetEvidence()); printf("interfaceKeyboardRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
