#include <stdio.h>
#include "spell_area_symbol_routes_pc34_compat.h"
int main(void) { unsigned int ok = spell_area_symbol_routes_GetInvariant(); printf("probe=firestaff_spell_area_symbol_routes\n"); printf("sourceEvidence=%s\n", spell_area_symbol_routes_GetEvidence()); printf("spellAreaSymbolRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
