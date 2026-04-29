#include "save_rest_toggle_consequences_pc34_compat.h"
const char* save_rest_toggle_consequences_GetEvidence(void) { return "COMMAND.C:578-625 defines interface keyboard routes for F1-F4 inventory toggles, save game, and freeze; COMMAND.C:2180-2184 dispatches toggle/close inventory; COMMAND.C:2336-2359 handles rest by closing inventory, drawing disabled menus/rest screen, setting G0300_B_PartyIsResting, and swapping input tables."; }
unsigned int save_rest_toggle_consequences_GetInvariant(void) { return 1u; }
