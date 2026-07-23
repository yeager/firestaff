#include "dm1_v1_f0441_f0460_save_endgame_source_audit_pc34_compat.h"
#include <stdio.h>
int main(void){unsigned n;for(n=441;n<=460;n++){const DM1_V1_F0441F0460SourceAuditPc34Compat *e=dm1_v1_f0441_f0460_save_endgame_source_audit_pc34((uint16_t)n);if(!e||!e->hostFallbackForbidden||!e->source||!e->owner)return 1;}puts("PASS dm1_v1_f0441_f0460_save_endgame_source_audit_pc34_compat");return 0;}
