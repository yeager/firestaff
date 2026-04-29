#include <stdio.h>
#include "panel_chest_mouse_routes_pc34_compat.h"
int main(void) { unsigned int ok = panel_chest_mouse_routes_GetInvariant(); printf("probe=firestaff_panel_chest_mouse_routes\n"); printf("sourceEvidence=%s\n", panel_chest_mouse_routes_GetEvidence()); printf("panelChestMouseRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
