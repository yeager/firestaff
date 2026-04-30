#include <stdio.h>
#include "action_area_routes_pc34_compat.h"
int main(void) {
    unsigned int ok = action_area_routes_GetInvariant();
    unsigned int touchOk = action_area_routes_GetTouchMatrixInvariant();
    printf("probe=firestaff_action_area_routes\n");
    printf("sourceEvidence=%s\n", action_area_routes_GetEvidence());
    printf("actionAreaRoutesInvariantOk=%u\n", ok);
    printf("actionAreaTouchMatrixInvariantOk=%u\n", touchOk);
    return (ok && touchOk) ? 0 : 1;
}
