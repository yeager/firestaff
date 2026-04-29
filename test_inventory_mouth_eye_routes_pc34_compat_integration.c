#include <stdio.h>
#include "inventory_mouth_eye_routes_pc34_compat.h"
int main(void) { unsigned int ok = inventory_mouth_eye_routes_GetInvariant(); printf("probe=firestaff_inventory_mouth_eye_routes\n"); printf("sourceEvidence=%s\n", inventory_mouth_eye_routes_GetEvidence()); printf("inventoryMouthEyeRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
