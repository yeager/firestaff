#include "dm2_v1_creature.h"
#include "dm2_v1_creature_tables.h"
#include <assert.h>
#include <string.h>

int main(void) {
    assert(DM2_AI_TABLE_GENUINE_SIZE == 62);
    assert(sizeof(DM2_AIDefinition) == 36);

    /* Entry 0: TREE — static, HP=700, Def=255 */
    assert(dm2_v1_ai_table_genuine[0].w0AIFlags == 0x0045);
    assert(dm2_v1_ai_table_genuine[0].BaseHP == 700);
    assert(dm2_v1_ai_table_genuine[0].Defense == 255);
    assert(dm2_v1_ai_table_genuine[0].Weight == 255);
    assert((dm2_v1_ai_table_genuine[0].w0AIFlags & DM2_AIFLAG_STATIC) != 0);

    /* Entry 19: THORN DEMON */
    assert(dm2_v1_ai_table_genuine[19].w0AIFlags == 0x0240);
    assert(dm2_v1_ai_table_genuine[19].BaseHP == 400);
    assert(dm2_v1_ai_table_genuine[19].AttackStrength == 50);
    assert(dm2_v1_ai_table_genuine[19].Defense == 120);
    assert((dm2_v1_ai_table_genuine[19].w0AIFlags & DM2_AIFLAG_STATIC) == 0);

    /* Entry 30: DRAGOTH — strongest */
    assert(dm2_v1_ai_table_genuine[30].BaseHP == 1500);
    assert(dm2_v1_ai_table_genuine[30].AttackStrength == 135);
    assert(dm2_v1_ai_table_genuine[30].Defense == 170);
    assert(dm2_v1_ai_table_genuine[30].b9x == 0x08);

    /* Entry 53: PIT GHOST — invisible, b9x=0x40 */
    assert(dm2_v1_ai_table_genuine[53].b9x == 0x40);
    assert((dm2_v1_ai_table_genuine[53].w0AIFlags & DM2_AIFLAG_INVISIBLE) != 0);

    /* Entry 61: GHOST — last entry */
    assert(dm2_v1_ai_table_genuine[61].w0AIFlags == 0x8C78);
    assert(dm2_v1_ai_table_genuine[61].BaseHP == 150);
    assert(dm2_v1_ai_table_genuine[61].AttackStrength == 40);

    /* Verify byte-level: entry 0 should decode to same as raw */
    {
        const uint8_t *raw = (const uint8_t *)&dm2_v1_ai_table_genuine[0];
        assert(raw[0] == 0x45 && raw[1] == 0x00);
        assert(raw[4] == 0xBC && raw[5] == 0x02);
    }

    return 0;
}
