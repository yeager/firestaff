#include "inventory_mouth_eye_routes_pc34_compat.h"
const char* inventory_mouth_eye_routes_GetEvidence(void) { return "COMMAND.C:426-427 maps inventory secondary mouse input C070 mouth to C545_ZONE_MOUTH and C071 eye to C546_ZONE_EYE; COMMAND.C:2314-2320 dispatches them to F0349_INVENTORY_ProcessCommand70_ClickOnMouth and F0352_INVENTORY_ProcessCommand71_ClickOnEye."; }
unsigned int inventory_mouth_eye_routes_GetInvariant(void) { return 1u; }
