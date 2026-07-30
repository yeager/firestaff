#ifndef FIRESTAFF_DM2_V1_PARTY_H
#define FIRESTAFF_DM2_V1_PARTY_H

/*
 * DM2 party and hero data structures.
 * Source: skproject SKULLWIN/c_hero.h
 *
 * c_hero is 0x107 (263) bytes — savegame-critical layout.
 * c_party holds 4 heroes plus party state.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_HERO_SIZE_1STNAME   8
#define DM2_HERO_SIZE_2NDNAME  20
#define DM2_NUM_ABILITIES       7
#define DM2_NUM_ITEMS          30
#define DM2_MAX_HEROES          4
#define DM2_CONTAINER_SIZE     32

typedef enum {
    DM2_ABILITY_LUCK      = 0,
    DM2_ABILITY_STRENGTH  = 1,
    DM2_ABILITY_DEXTERITY = 2,
    DM2_ABILITY_WIZARDRY  = 3,
    DM2_ABILITY_VITALITY  = 4,
    DM2_ABILITY_ANTIMAGIC = 5,
    DM2_ABILITY_ANTIFIRE  = 6
} DM2_Ability;

typedef enum {
    DM2_CUR = 0,
    DM2_MAX = 1
} DM2_CurMax;

typedef enum {
    DM2_PROFESSION_FIGHTER = 0,
    DM2_PROFESSION_NINJA   = 1,
    DM2_PROFESSION_WIZARD  = 2,
    DM2_PROFESSION_PRIEST  = 3
} DM2_Profession;

typedef enum {
    DM2_HERO_NONE = -1,
    DM2_HERO_0    = 0,
    DM2_HERO_1    = 1,
    DM2_HERO_2    = 2,
    DM2_HERO_3    = 3
} DM2_HeroIndex;

/* 263-byte hero struct, savegame layout (skproject c_hero @ 0x107 bytes).
 * Packed because the original layout has unaligned fields (e.g. ench_power
 * at offset 0x103). */
#pragma pack(push, 1)
typedef struct {
    char     name1[DM2_HERO_SIZE_1STNAME];  /* @0x00 */
    char     name2[DM2_HERO_SIZE_2NDNAME];  /* @0x08 */
    int8_t   absdir;                         /* @0x1C 0=N 1=E 2=S 3=W */
    int8_t   partypos;                       /* @0x1D 0=TL 1=TR 2=BR 3=BL */
    int8_t   nrunes;                         /* @0x1E */
    int8_t   poisoned;                       /* @0x1F */
    int8_t   handcmd[2];                     /* @0x20 */
    char     rune[5];                        /* @0x22 spelled rune string */
    int8_t   b_27;                           /* @0x27 unused */
    int8_t   b_28;                           /* @0x28 */
    int8_t   b_29;                           /* @0x29 */
    int8_t   handcooldown[4];                /* @0x2A */
    int16_t  timeridx;                       /* @0x2E */
    int16_t  damagesuffered;                 /* @0x30 */
    int16_t  heroflag;                       /* @0x32 */
    int16_t  bodyflag;                       /* @0x34 */
    int16_t  curHP;                          /* @0x36 */
    int16_t  maxHP;                          /* @0x38 */
    int16_t  curStamina;                     /* @0x3A */
    int16_t  maxStamina;                     /* @0x3C */
    int16_t  curMP;                          /* @0x3E */
    int16_t  maxMP;                          /* @0x40 */
    int8_t   handdefenseclass[2];            /* @0x42 */
    int16_t  food;                           /* @0x44 */
    int16_t  water;                          /* @0x46 */
    int16_t  poison;                         /* @0x48 */
    int8_t   ability[DM2_NUM_ABILITIES][2];  /* @0x4A cur/max */
    int8_t   eability[DM2_NUM_ABILITIES];    /* @0x58 enhanced */
    int32_t  skill[5][4];                    /* @0x5F */
    int8_t   sbonus[5][4];                   /* @0xAF */
    int16_t  item[DM2_NUM_ITEMS];            /* @0xC3 */
    int16_t  weight;                         /* @0xFF */
    int8_t   herotype;                       /* @0x101 */
    int8_t   ench_aura;                      /* @0x102 */
    int16_t  ench_power;                     /* @0x103 */
    int8_t   walkspeed;                      /* @0x105 */
    int8_t   b_106;                          /* @0x106 unused */
} DM2_V1_Hero;
#pragma pack(pop)

typedef struct {
    DM2_V1_Hero hero[DM2_MAX_HEROES];
    int16_t     heros_in_party;
    int16_t     absdir;
    DM2_HeroIndex curactevhero;
    int16_t     hand_container[DM2_CONTAINER_SIZE];
    int16_t     curactmode;
    int16_t     curacthero;
    int8_t      handitems[20];
} DM2_V1_Party;

void dm2_v1_hero_init(DM2_V1_Hero *hero);
void dm2_v1_party_init(DM2_V1_Party *party);

void dm2_v1_party_set_hero_flags(DM2_V1_Party *party);

int16_t dm2_v1_hero_get_adj_ability1_raw(
    DM2_V1_Hero *hero, DM2_Ability abi, DM2_CurMax curmax,
    int16_t rand16_value);

int32_t dm2_v1_hero_get_adj_ability2_raw(
    DM2_V1_Hero *hero, DM2_Ability abi, int16_t input,
    int16_t rand16_value);

int16_t dm2_v1_hero_get_stamina_adj_raw(DM2_V1_Hero *hero, int16_t input);

int16_t dm2_v1_hero_get_max_load_raw(DM2_V1_Hero *hero, int16_t rand16_value);

int dm2_v1_hero_use_luck_raw(
    DM2_V1_Hero *hero, int16_t input,
    int randbit, int16_t rand16_ability, int16_t rand16_luck);

int16_t dm2_v1_hero_2c1d_132c(int16_t a, int16_t b);

int16_t dm2_v1_hero_2c1d_0e23(int16_t weight);

int32_t dm2_v1_timproc_3a15_1da8(int32_t a, int32_t b);

void dm2_v1_hero_2c1d_0300(DM2_V1_Hero *hero, int16_t ability_idx, int16_t delta);

int32_t dm2_v1_hero_37bea(DM2_V1_Party *party, int16_t hero_idx, int16_t carried_weight);

int32_t dm2_v1_get_party_special_force(DM2_V1_Party *party, const int16_t *carried_weights);

void dm2_v1_party_reset_squad_dir(DM2_V1_Party *party, int8_t facing_dir);

void dm2_v1_party_select_champion_leader(
    DM2_V1_Party *party, int16_t new_leader,
    int16_t current_leader, int16_t next_champion_number);

void dm2_v1_hero_adjust_hand_cooldown(
    DM2_V1_Hero *hero, int16_t hand_idx, int16_t base_delay,
    int savegames1_b04);

int16_t dm2_v1_hero_use_dexterity_attribute_raw(
    DM2_V1_Hero *hero, int16_t carried_weight, int16_t max_load,
    int sleep_flag, int16_t rand7_1, int16_t rand7_2, int16_t rand7_3);

#ifdef __cplusplus
}
#endif

#endif
