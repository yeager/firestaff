#include <stdio.h>
#include "party_resting_input_routes_pc34_compat.h"
int main(void) { unsigned int ok = party_resting_input_routes_GetInvariant(); printf("probe=firestaff_party_resting_input_routes\n"); printf("sourceEvidence=%s\n", party_resting_input_routes_GetEvidence()); printf("partyRestingInputRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
