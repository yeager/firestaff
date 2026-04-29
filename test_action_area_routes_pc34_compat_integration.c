#include <stdio.h>
#include "action_area_routes_pc34_compat.h"
int main(void) { unsigned int ok = action_area_routes_GetInvariant(); printf("probe=firestaff_action_area_routes\n"); printf("sourceEvidence=%s\n", action_area_routes_GetEvidence()); printf("actionAreaRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
