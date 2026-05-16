#include <stdio.h>
#include "champion_names_hands_split_pc34_compat.h"
int main(void) { unsigned int ok = champion_names_hands_split_GetInvariant(); printf("probe=firestaff_champion_names_hands_split\n"); printf("sourceEvidence=%s\n", champion_names_hands_split_GetEvidence()); printf("championNamesHandsSplitInvariantOk=%u\n", ok); return ok ? 0 : 1; }
