#include <stdio.h>
#include "spell_area_routes_pc34_compat.h"
int main(void) {
    unsigned int ok = spell_area_routes_GetInvariant();
    unsigned int touchOk = spell_area_routes_GetTouchMatrixInvariant();
    printf("probe=firestaff_spell_area_routes\n");
    printf("sourceEvidence=%s\n", spell_area_routes_GetEvidence());
    printf("spellAreaRoutesInvariantOk=%u\n", ok);
    printf("spellAreaTouchMatrixInvariantOk=%u\n", touchOk);
    return (ok && touchOk) ? 0 : 1;
}
