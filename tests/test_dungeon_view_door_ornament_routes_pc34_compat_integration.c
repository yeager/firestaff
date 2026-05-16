#include <stdio.h>
#include "dungeon_view_door_ornament_routes_pc34_compat.h"
int main(void) { unsigned int ok = dungeon_view_door_ornament_routes_GetInvariant(); printf("probe=firestaff_dungeon_view_door_ornament_routes\n"); printf("sourceEvidence=%s\n", dungeon_view_door_ornament_routes_GetEvidence()); printf("dungeonViewDoorOrnamentRoutesInvariantOk=%u\n", ok); return ok ? 0 : 1; }
