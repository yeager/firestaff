#include <stdio.h>
#include "rename_route_family_pc34_compat.h"
int main(void) { unsigned int ok = rename_route_family_GetInvariant(); printf("probe=firestaff_rename_route_family\n"); printf("sourceEvidence=%s\n", rename_route_family_GetEvidence()); printf("renameRouteFamilyInvariantOk=%u\n", ok); return ok ? 0 : 1; }
