/*
 * sound_event_snd3_map_v1.c — Pass 52.
 *
 * Source anchors:
 *   - ReDMCSB Toolchains/Common/Source/DEFS.H, MEDIA485/I34E sound macros
 *   - ReDMCSB Toolchains/Common/Source/DATA.C, MEDIA719_I34E_I34M
 *     G0060_as_Graphic562_Sounds table
 *   - Pass 51 graphics_dat_snd3_loader_v1 labels for GRAPHICS.DAT SND3 items
 *
 * No SDL/runtime playback integration is performed here.
 */

#include "sound_event_snd3_map_v1.h"
#include "graphics_dat_snd3_loader_v1.h"

#include <stddef.h>

static const V1_SoundEventSnd3MapEntry kMap[V1_DM_SOUND_EVENT_COUNT] = {
    { 0, 671,  0, "C00_SOUND_METALLIC_THUD", "falling item / weapon projectile impact" },
    { 1, 672,  1, "C01_SOUND_SWITCH", "audible sensor / door button" },
    { 2, 673,  2, "C02_SOUND_DOOR_RATTLE", "door opening or closing" },
    { 3, 673,  2, "C03_SOUND_DOOR_RATTLE_ENTRANCE", "entrance door rattle, same SND3 item as C02 in I34E" },
    { 4, 674,  3, "C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM", "wooden thud / wall touch / trolin-stone-golem attack" },
    { 5, 675,  4, "C05_SOUND_STRONG_EXPLOSION", "strong explosion / fireball" },
    { 6, 675,  4, "M541_SOUND_WEAK_EXPLOSION", "weak explosion, same SND3 item as C05 in I34E" },
    { 7, 677,  5, "M561_SOUND_SCREAM", "falling / party death scream" },
    { 8, 678,  6, "C08_SOUND_SWALLOW", "champion eats or drinks" },
    { 9, 679,  7, "C09_SOUND_CHAMPION_0_DAMAGED", "champion 0 wounded" },
    {10, 680,  8, "C10_SOUND_CHAMPION_1_DAMAGED", "champion 1 wounded" },
    {11, 681,  9, "C11_SOUND_CHAMPION_2_DAMAGED", "champion 2 wounded" },
    {12, 682, 10, "C12_SOUND_CHAMPION_3_DAMAGED", "champion 3 wounded" },
    {13, 684, 12, "M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT", "party melee/shoot/throw + skeleton/animated-armour attack" },
    {14, 685, 13, "M560_SOUND_BUZZ", "teleporter / group generator buzz" },
    {15, 687, 14, "M562_SOUND_PARTY_DAMAGED", "running into wall / hit by closing door" },
    {16, 683, 11, "M542_SOUND_SPELL", "exploding spell / spell impact" },
    {17, 707, 27, "M619_SOUND_WAR_CRY", "war cry action" },
    {18, 704, 24, "M620_SOUND_BLOW_HORN", "horn of fear action" },
    {19, 690, 17, "C19_SOUND_ATTACK_SCREAMER_OITU", "screamer / oitu attack" },
    {20, 691, 18, "C20_SOUND_ATTACK_GIANT_SCORPION_SCORPION", "giant scorpion attack" },
    {21, 692, 19, "C21_SOUND_ATTACK_MAGENTA_WORM_WORM", "magenta worm attack" },
    {22, 693, 20, "C22_SOUND_ATTACK_GIGGLER", "giggler attack" },
    {23, 688, 15, "C23_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON", "pain rat / red dragon attack" },
    {24, 708, 28, "C24_SOUND_ATTACK_ROCK_ROCKPILE", "rockpile attack" },
    {25, 689, 16, "C25_SOUND_ATTACK_MUMMY_GHOST_RIVE", "mummy / ghost attack" },
    {26, 709, 29, "C26_SOUND_ATTACK_WATER_ELEMENTAL", "water elemental attack" },
    {27, 710, 30, "C27_SOUND_ATTACK_COUATL", "couatl attack" },
    {28, 701, 21, "C28_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT", "animated armour movement" },
    {29, 702, 22, "C29_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER", "couatl / giant wasp movement" },
    {30, 703, 23, "C30_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON", "mummy/trolin/golem/giggler/vexirk/demon movement" },
    {31, 705, 25, "C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU", "screamer/rockpile/worm/rat/scorpion/oitu movement" },
    {32, 706, 26, "C32_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL", "swamp slime / water elemental movement" },
    {33, 711, 31, "C33_SOUND_MOVE_RED_DRAGON", "red dragon movement" },
    {34, 712, 32, "C34_SOUND_MOVE_SKELETON", "skeleton movement" }
};

const V1_SoundEventSnd3MapEntry* V1_SoundEventSnd3_Map(unsigned int* outCount) {
    if (outCount) *outCount = V1_DM_SOUND_EVENT_COUNT;
    return kMap;
}

const V1_SoundEventSnd3MapEntry* V1_SoundEventSnd3_Find(int soundIndex) {
    unsigned int i;
    for (i = 0; i < V1_DM_SOUND_EVENT_COUNT; ++i) {
        if (kMap[i].soundIndex == soundIndex) return &kMap[i];
    }
    return NULL;
}

int V1_SoundEventSnd3_ItemForSoundIndex(int soundIndex) {
    const V1_SoundEventSnd3MapEntry* e = V1_SoundEventSnd3_Find(soundIndex);
    return e ? (int)e->snd3ItemIndex : V1_DM_SOUND_EVENT_NONE;
}

const char* V1_SoundEventSnd3_LabelForSoundIndex(int soundIndex) {
    const V1_SoundEventSnd3MapEntry* e = V1_SoundEventSnd3_Find(soundIndex);
    if (!e) return "(not a DM PC v3.4 sound event)";
    return V1_GraphicsSnd3_ItemLabel(e->snd3ItemIndex);
}
