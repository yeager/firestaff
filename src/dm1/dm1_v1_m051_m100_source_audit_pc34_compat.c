#include "dm1_v1_m051_m100_source_audit_pc34_compat.h"

#define M(number, name, anchor) {number, name, anchor, "macro/local state; no standalone verified Firestaff owner"}
#define ABSENT(number, name) {number, name, "ReDMCSB label inventory", "absent ReDMCSB module slot; no route"}

static const Dm1V1M051M100SourceAuditPc34 kSymbols[] = {
    M(51, "M051_CREATURE_HEIGHT", "DEFS.H:1609"),
    M(52, "M052_MAXIMUM_HORIZONTAL_OFFSET", "DEFS.H:1631"),
    M(53, "M053_MAXIMUM_VERTICAL_OFFSET", "DEFS.H:1632"),
    M(54, "M054_SIGHT_RANGE", "DEFS.H:1651"),
    M(55, "M055_SMELL_RANGE", "DEFS.H:1653"),
    M(56, "M056_ATTACK_RANGE", "DEFS.H:1654"),
    M(57, "M057_FEAR_RESISTANCE", "DEFS.H:1657"),
    M(58, "M058_EXPERIENCE", "DEFS.H:1658"),
    M(59, "M059_WARINESS", "DEFS.H:1659"),
    M(60, "M060_FIRE_RESISTANCE", "DEFS.H:1663"),
    M(61, "M061_POISON_RESISTANCE", "DEFS.H:1664"),
    M(62, "M062_NEXT_BEHAVIOR_UPDATE_AFTER_ATTACK_TICKS", "DEFS.H:1669"),
    M(63, "M063_NEXT_NON_ATTACK_ASPECT_UPDATE_TICKS", "DEFS.H:1670"),
    M(64, "M064_NEXT_ATTACK_ASPECT_UPDATE_TICKS", "DEFS.H:1671"),
    M(65, "M065_SHOOT_ATTACK", "DEFS.H:1733"),
    M(66, "M066_PROJECTILE_ASPECT_ORDINAL", "DEFS.H:1734"),
    M(67, "M067_SPELL_KIND", "DEFS.H:1755"),
    M(68, "M068_SPELL_TYPE", "DEFS.H:1756"),
    M(69, "M069_SPELL_DISABLED_TICKS", "DEFS.H:1757"),
    M(70, "M070_HAND_SLOT_INDEX", "DEFS.H:1878"),
    M(71, "M071_COORDINATE_SET", "DEFS.H:2016"),
    M(72, "M072_TRANSPARENT_COLOR", "DEFS.H:2017"),
    M(73, "M073_COLOR_09_REPLACEMENT_COLOR_SET", "DEFS.H:2018"),
    M(74, "M074_COLOR_10_REPLACEMENT_COLOR_SET", "DEFS.H:2019"),
    M(75, "M075_BITMAP_BYTE_COUNT", "DEFS.H:2159"),
    M(76, "M076_BITMAP_UNIT_COUNT", "DEFS.H:2160"),
    M(77, "M077_NORMALIZED_BYTE_WIDTH", "DEFS.H:2161"),
    M(78, "M078_SCALED_DIMENSION", "DEFS.H:2515"),
    M(79, "M079_PREVIOUS_BLOCK_SIZE", "DEFS.H:2810"),
    M(80, "M080_BLOCK_SIZE", "DEFS.H:2811"),
    M(81, "M081_USED_BLOCK_USAGE_COUNT", "DEFS.H:2812"),
    M(82, "M082_USED_BLOCK_PREVIOUS_INDEX", "DEFS.H:2813"),
    M(83, "M083_USED_BLOCK_NEXT_INDEX", "DEFS.H:2814"),
    M(84, "M084_USED_BLOCK_BITMAP_INDEX", "DEFS.H:2815"),
    M(85, "M085_USED_BLOCK_BITMAP", "DEFS.H:2817"),
    M(86, "M086_UNUSED_BLOCK_PREVIOUS_ADDRESS", "DEFS.H:2829"),
    M(87, "M087_UNUSED_BLOCK_NEXT_ADDRESS", "DEFS.H:2830"),
    M(88, "M088_GRAPHIC_BYTE_WIDTH", "DEFS.H:2833"),
    M(89, "M089_GRAPHIC_HEIGHT", "DEFS.H:2834"),
    M(90, "M090_GRAPHIC_INDEX_OR_SIZE", "DEFS.H:2835"),
    M(91, "M091_BITPLANE_SIZE", "AMIGA.H:141"),
    M(92, "M092_BLITSIZE", "AMIGA.H:142"),
    ABSENT(93, "M093_absent"),
    ABSENT(94, "M094_absent"),
    ABSENT(95, "M095_absent"),
    ABSENT(96, "M096_absent"),
    ABSENT(97, "M097_absent"),
    ABSENT(98, "M098_absent"),
    ABSENT(99, "M099_absent"),
    M(100, "M100_PIXEL_WIDTH", "DEFS.H:3444"),
};

const Dm1V1M051M100SourceAuditPc34 *
dm1_v1_m051_m100_source_audit_pc34(unsigned int number)
{
    if (number < 51U || number > 100U) return 0;
    return &kSymbols[number - 51U];
}

int dm1_v1_m051_m100_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m051_m100_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m051_m100_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv M051-M100; "
           "DEFS.H:1609-3444 and AMIGA.H:141-142. M093-M099 are absent. "
           "All present entries are macro/module expressions or local layout names, "
           "not standalone callable behavior. No generated macro, graphics, UI, input, "
           "timing, memory, or platform route.";
}
