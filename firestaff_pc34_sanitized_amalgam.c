#define EXEID 84
#define __TURBOC__ 1
#include "COMPILE.H"

/* BEGIN STRING.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#if EXETYPE == C03_GAME
int16_t F0090_strlen(
REGISTER char* P2296_pc_String FINAL_SEPARATOR
{
        REGISTER char* L3111_pc_Character = P2296_pc_String;


        while (*P2296_pc_String++ != 0);
        return (int16_t)(P2296_pc_String - L3111_pc_Character) - 1;
}

char* F0086_strcat(
REGISTER char* P2297_pc_StringA SEPARATOR
REGISTER char* P2298_pc_StringB FINAL_SEPARATOR
{
        REGISTER char* L3112_pc_Character = P2297_pc_StringA;


        while (*P2297_pc_StringA++);
        P2297_pc_StringA--;
        while (*P2297_pc_StringA++ = *P2298_pc_StringB++);
        return L3112_pc_Character;
}

int16_t F0087_strcmp(
REGISTER char* P2299_pc_StringA SEPARATOR
REGISTER char* P2300_pc_StringB FINAL_SEPARATOR
{
        while (*P2299_pc_StringA && (*P2299_pc_StringA == *P2300_pc_StringB)) {
                P2299_pc_StringA++;
                P2300_pc_StringB++;
        }
        return *P2299_pc_StringA - *P2300_pc_StringB;
}

char* F0088_strcpy(
REGISTER char* P2301_pc_StringA SEPARATOR
REGISTER char* P2302_pc_StringB FINAL_SEPARATOR
{
        REGISTER char* L3113_pc_Character = P2301_pc_StringA;


        while (*P2301_pc_StringA++ = *P2302_pc_StringB++);
        return L3113_pc_Character;
}

char* F0089_strncpy(
REGISTER char*   P2303_pc_StringA SEPARATOR
REGISTER char*   P2304_pc_StringB SEPARATOR
REGISTER int16_t P2305_i_Count    FINAL_SEPARATOR
{
        REGISTER char* L3114_pc_Character = P2303_pc_StringA;


        while ((P2305_i_Count--) && (*P2303_pc_StringA++ = *P2304_pc_StringB++));
        return L3114_pc_Character;
}

char* F0091_strchr(
REGISTER char* P2306_pc_String   SEPARATOR
REGISTER char  P2307_c_Character FINAL_SEPARATOR
{
        REGISTER char L3115_c_Character;


        while ((L3115_c_Character = *P2306_pc_String) && (L3115_c_Character != P2307_c_Character)) {
                P2306_pc_String++;
        }
        return (L3115_c_Character) ? P2306_pc_String : NULL;
}


BOOLEAN F0670_ReplaceCharacterByString(
REGISTER char* P2313_pc_SourceString      SEPARATOR
char*          P2314_pc_ReplacementString SEPARATOR
REGISTER char  P2315_c_Character          SEPARATOR
REGISTER char* P2316_pc_DestinationString FINAL_SEPARATOR
{
        REGISTER char* L1278_pc_TildeCharacter;
        REGISTER unsigned int16_t L1279_ui_TildeCharacterPosition;


        if ((L1278_pc_TildeCharacter = M549_STRCHR(P2313_pc_SourceString, P2315_c_Character)) == NULL) {
                F0088_strcpy(P2316_pc_DestinationString, P2313_pc_SourceString);
        } else {
                F0089_strncpy(P2316_pc_DestinationString, P2313_pc_SourceString, L1279_ui_TildeCharacterPosition = L1278_pc_TildeCharacter - P2313_pc_SourceString);
                P2316_pc_DestinationString[L1279_ui_TildeCharacterPosition] = '\0';
                F0086_strcat(P2316_pc_DestinationString, P2314_pc_ReplacementString);
                F0086_strcat(P2316_pc_DestinationString, L1278_pc_TildeCharacter + 1);
        }
        return (L1278_pc_TildeCharacter != NULL);
}

void F0671_ConvertLongToString(
REGISTER long P2317_l_Value   SEPARATOR
char*         P2318_pc_String FINAL_SEPARATOR
{
        char L3123_ac_String[14];
        REGISTER unsigned int16_t L3124_i_CharacterIndex;
        REGISTER BOOLEAN L3125_B_IsNegative;


        L3124_i_CharacterIndex = 13;
        if ((L3125_B_IsNegative = P2317_l_Value < 0) != 0) {
                P2317_l_Value = -P2317_l_Value;
        }
        L3123_ac_String[L3124_i_CharacterIndex] = 0;
        do {
                L3123_ac_String[--L3124_i_CharacterIndex] = '0' + (P2317_l_Value % 10);
        } while (P2317_l_Value /= 10);
        if (L3125_B_IsNegative) {
                L3123_ac_String[--L3124_i_CharacterIndex] = '-';
        }
        F0088_strcpy(P2318_pc_String, &L3123_ac_String[L3124_i_CharacterIndex]);
}
#endif
/* END STRING.C */

/* BEGIN DUNGEON.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
int16_t G0233_ai_Graphic559_DirectionToStepEastCount[4] = {
        0,    /* North */
        1,    /* East */
        0,    /* West */
        -1 }; /* South */
int16_t G0234_ai_Graphic559_DirectionToStepNorthCount[4] = {
        -1,  /* North */
        0,   /* East */
        1,   /* West */
        0 }; /* South */
unsigned char G0235_auc_Graphic559_ThingDataByteCount[16] = {
        4,   /* Door */
        6,   /* Teleporter */
        4,   /* Text String */
        8,   /* Sensor */
        16,  /* Group */
        4,   /* Weapon */
        4,   /* Armour */
        4,   /* Scroll */
        4,   /* Potion */
        8,   /* Container */
        4,   /* Junk */
        0,   /* Unused */
        0,   /* Unused */
        0,   /* Unused */
        8,   /* Projectile */
        4 }; /* Explosion */
unsigned char G0236_auc_Graphic559_AdditionalThingCounts[16] = {
        0,    /* Door */
        0,    /* Teleporter */
        0,    /* Text String */
        0,    /* Sensor */
        75,   /* Group */
        100,  /* Weapon */
        120,  /* Armour */
        0,    /* Scroll */
        5,    /* Potion */
        0,    /* Container */
        140,  /* Junk */
        0,    /* Unused */
        0,    /* Unused */
        0,    /* Unused */
        60,   /* Projectile */
        50 }; /* Explosion */
OBJECT_INFO G0237_as_Graphic559_ObjectInfo[180] = {
        /* { Type, ObjectAspectIndex, ActionSetIndex, AllowedSlots } */
        {  30,  1,  0, 0x0500 },   /*   0 Scroll                          Pouch/Chest */
        { 144,  0,  0, 0x0200 },   /*   1 Chest                           Hands */
        { 148, 67,  0, 0x0500 },   /*   2 Mon Potion                      Pouch/Chest */
        { 149, 67,  0, 0x0500 },   /*   3 Um Potion                       Pouch/Chest */
        { 150, 67,  0, 0x0500 },   /*   4 Des Potion                      Pouch/Chest */
        { 151, 67, 42, 0x0500 },   /*   5 Ven Potion                      Pouch/Chest */
        { 152, 67,  0, 0x0500 },   /*   6 Sar Potion                      Pouch/Chest */
        { 153, 67,  0, 0x0500 },   /*   7 Zo Potion                       Pouch/Chest */
        { 154,  2,  0, 0x0501 },   /*   8 Ros Potion                      Mouth/Pouch/Chest */
        { 155,  2,  0, 0x0501 },   /*   9 Ku Potion                       Mouth/Pouch/Chest */
        { 156,  2,  0, 0x0501 },   /*  10 Dane Potion                     Mouth/Pouch/Chest */
        { 157,  2,  0, 0x0501 },   /*  11 Neta Potion                     Mouth/Pouch/Chest */
        { 158,  2,  0, 0x0501 },   /*  12 Bro Potion / Antivenin          Mouth/Pouch/Chest */
        { 159,  2,  0, 0x0501 },   /*  13 Ma Potion / Mon Potion          Mouth/Pouch/Chest */
        { 160,  2,  0, 0x0501 },   /*  14 Ya Potion                       Mouth/Pouch/Chest */
        { 161,  2,  0, 0x0501 },   /*  15 Ee Potion                       Mouth/Pouch/Chest */
        { 162,  2,  0, 0x0501 },   /*  16 Vi Potion                       Mouth/Pouch/Chest */
        { 163,  2,  0, 0x0501 },   /*  17 Water Flask                     Mouth/Pouch/Chest */
        { 164, 68,  0, 0x0500 },   /*  18 Kath Bomb                       Pouch/Chest */
        { 165, 68,  0, 0x0500 },   /*  19 Pew Bomb                        Pouch/Chest */
        { 166, 68,  0, 0x0500 },   /*  20 Ra Bomb                         Pouch/Chest */
        { 167, 68, 42, 0x0500 },   /*  21 Ful Bomb                        Pouch/Chest */
        { 195, 80,  0, 0x0500 },   /*  22 Empty Flask                     Pouch/Chest */
        {  16, 38, 43, 0x0500 },   /*  23 Eye Of Time                     Pouch/Chest */
        {  18, 38,  7, 0x0500 },   /*  24 Stormring                       Pouch/Chest */
        {   4, 35,  5, 0x0400 },   /*  25 Torch                           Chest */
        {  14, 37,  6, 0x0400 },   /*  26 Flamitt                         Chest */
        {  20, 11,  8, 0x0040 },   /*  27 Staff Of Claws                  Quiver 1 */
        {  23, 12,  9, 0x0040 },   /*  28 Bolt Blade / Storm              Quiver 1 */
        {  25, 12, 10, 0x0040 },   /*  29 Fury / Ra Blade                 Quiver 1 */
        {  27, 39, 11, 0x0040 },   /*  30 The Firestaff                   Quiver 1 */
        {  32, 17, 12, 0x05C0 },   /*  31 Dagger                          Quiver 1/Quiver 2/Pouch/Chest */
        {  33, 12, 13, 0x0040 },   /*  32 Falchion                        Quiver 1 */
        {  34, 12, 13, 0x0040 },   /*  33 Sword                           Quiver 1 */
        {  35, 12, 14, 0x0040 },   /*  34 Rapier                          Quiver 1 */
        {  36, 12, 15, 0x0040 },   /*  35 Sabre / Biter                   Quiver 1 */
        {  37, 12, 15, 0x0040 },   /*  36 Samurai Sword                   Quiver 1 */
        {  38, 12, 16, 0x0040 },   /*  37 Delta / Side Splitter           Quiver 1 */
        {  39, 12, 17, 0x0040 },   /*  38 Diamond Edge                    Quiver 1 */
        {  40, 42, 18, 0x0040 },   /*  39 Vorpal Blade                    Quiver 1 */
        {  41, 12, 19, 0x0040 },   /*  40 The Inquisitor / Dragon Fang    Quiver 1 */
        {  42, 13, 20, 0x0040 },   /*  41 Axe                             Quiver 1 */
        {  43, 13, 21, 0x0040 },   /*  42 Hardcleave / Executioner        Quiver 1 */
        {  44, 21, 22, 0x0040 },   /*  43 Mace                            Quiver 1 */
        {  45, 21, 22, 0x0040 },   /*  44 Mace Of Order                   Quiver 1 */
        {  46, 33, 23, 0x0440 },   /*  45 Morningstar                     Quiver 1/Chest */
        {  47, 43, 24, 0x0040 },   /*  46 Club                            Quiver 1 */
        {  48, 44, 24, 0x0040 },   /*  47 Stone Club                      Quiver 1 */
        {  49, 14, 27, 0x0040 },   /*  48 Bow / Claw Bow                  Quiver 1 */
        {  50, 45, 27, 0x0040 },   /*  49 Crossbow                        Quiver 1 */
        {  51, 16, 26, 0x05C0 },   /*  50 Arrow                           Quiver 1/Quiver 2/Pouch/Chest */
        {  52, 46, 26, 0x05C0 },   /*  51 Slayer                          Quiver 1/Quiver 2/Pouch/Chest */
        {  53, 11, 27, 0x0440 },   /*  52 Sling                           Quiver 1/Chest */
        {  54, 47, 42, 0x05C0 },   /*  53 Rock                            Quiver 1/Quiver 2/Pouch/Chest */
        {  55, 48, 40, 0x05C0 },   /*  54 Poison Dart                     Quiver 1/Quiver 2/Pouch/Chest */
        {  56, 49, 42, 0x05C0 },   /*  55 Throwing Star                   Quiver 1/Quiver 2/Pouch/Chest */
        {  57, 50,  5, 0x0040 },   /*  56 Stick                           Quiver 1 */
        {  58, 11,  5, 0x0040 },   /*  57 Staff                           Quiver 1 */
        {  59, 31, 28, 0x0540 },   /*  58 Wand                            Quiver 1/Pouch/Chest */
        {  60, 31, 29, 0x0540 },   /*  59 Teowand                         Quiver 1/Pouch/Chest */
        {  61, 11, 30, 0x0040 },   /*  60 Yew Staff                       Quiver 1 */
        {  62, 11, 31, 0x0040 },   /*  61 Staff Of Manar / Staff Of Irra  Quiver 1 */
        {  63, 11, 32, 0x0040 },   /*  62 Snake Staff / Cross Of Neta     Quiver 1 Atari ST Version 1.0 1987-12-08: ObjectAspectIndex = 35 */
        {  64, 51, 33, 0x0040 },   /*  63 The Conduit / Serpent Staff     Quiver 1 */
        {  65, 32,  5, 0x0440 },   /*  64 Dragon Spit                     Quiver 1/Chest */
        {  66, 30, 35, 0x0040 },   /*  65 Sceptre Of Lyf                  Quiver 1 */
        { 135, 65, 36, 0x0440 },   /*  66 Horn Of Fear                    Quiver 1/Chest */
        { 143, 45, 27, 0x0040 },   /*  67 Speedbow                        Quiver 1 */
        {  28, 82,  1, 0x0040 },   /*  68 The Firestaff (Complete)        Quiver 1 */
        {  80, 23,  0, 0x040C },   /*  69 Cape                            Neck/Torso/Chest */
        {  81, 23,  0, 0x040C },   /*  70 Cloak Of Night                  Neck/Torso/Chest */
        {  82, 23,  0, 0x0410 },   /*  71 Barbarian Hide / Tattered Pants Legs/Chest */
        { 112, 55,  0, 0x0420 },   /*  72 Sandals                         Feet/Chest */
        { 114,  8,  0, 0x0420 },   /*  73 Leather Boots                   Feet/Chest */
        {  67, 24,  0, 0x0408 },   /*  74 Robe (Body) / Tattered Shirt    Torso/Chest */
        {  83, 24,  0, 0x0410 },   /*  75 Robe (Legs)                     Legs/Chest */
        {  68, 24,  0, 0x0408 },   /*  76 Fine Robe (Body)                Torso/Chest */
        {  84, 24,  0, 0x0410 },   /*  77 Fine Robe (Legs)                Legs/Chest */
        {  69, 69,  0, 0x0408 },   /*  78 Kirtle                          Torso/Chest */
        {  70, 24,  0, 0x0408 },   /*  79 Silk Shirt                      Torso/Chest */
        {  85, 24,  0, 0x0410 },   /*  80 Tabard                          Legs/Chest */
        {  86, 69,  0, 0x0410 },   /*  81 Gunna                           Legs/Chest */
        {  71,  7,  0, 0x0408 },   /*  82 Elven Doublet                   Torso/Chest */
        {  87,  7,  0, 0x0410 },   /*  83 Elven Huke                      Legs/Chest */
        { 119, 57,  0, 0x0420 },   /*  84 Elven Boots                     Feet/Chest */
        {  72, 23,  0, 0x0408 },   /*  85 Leather Jerkin                  Torso/Chest */
        {  88, 23,  0, 0x0410 },   /*  86 Leather Pants                   Legs/Chest */
        { 113, 29,  0, 0x0420 },   /*  87 Suede Boots                     Feet/Chest */
        {  89, 69,  0, 0x0410 },   /*  88 Blue Pants                      Legs/Chest */
        {  73, 69,  0, 0x0408 },   /*  89 Tunic                           Torso/Chest */
        {  74, 24,  0, 0x0408 },   /*  90 Ghi                             Torso/Chest */
        {  90, 24,  0, 0x0410 },   /*  91 Ghi Trousers                    Legs/Chest */
        { 103, 53,  0, 0x0402 },   /*  92 Calista                         Head/Chest */
        { 104, 53,  0, 0x0402 },   /*  93 Crown Of Nerra                  Head/Chest */
        {  96,  9,  0, 0x0402 },   /*  94 Bezerker Helm                   Head/Chest */
        {  97,  9,  0, 0x0402 },   /*  95 Helmet                          Head/Chest */
        {  98,  9,  0, 0x0402 },   /*  96 Basinet                         Head/Chest */
        { 105, 54, 41, 0x0400 },   /*  97 Buckler / Neta Shield           Chest */
        { 106, 54, 41, 0x0200 },   /*  98 Hide Shield / Crystal Shield    Hands */
        { 108, 10, 41, 0x0200 },   /*  99 Wooden Shield                   Hands */
        { 107, 54, 41, 0x0200 },   /* 100 Small Shield                    Hands */
        {  75, 19,  0, 0x0408 },   /* 101 Mail Aketon                     Torso/Chest */
        {  91, 19,  0, 0x0410 },   /* 102 Leg Mail                        Legs/Chest */
        {  76, 19,  0, 0x0408 },   /* 103 Mithral Aketon                  Torso/Chest */
        {  92, 19,  0, 0x0410 },   /* 104 Mithral Mail                    Legs/Chest */
        {  99,  9,  0, 0x0402 },   /* 105 Casque'n Coif                   Head/Chest */
        { 115, 19,  0, 0x0420 },   /* 106 Hosen                           Feet/Chest */
        { 100, 52,  0, 0x0402 },   /* 107 Armet                           Head/Chest */
        {  77, 20,  0, 0x0008 },   /* 108 Torso Plate                     Torso */
        {  93, 22,  0, 0x0010 },   /* 109 Leg Plate                       Legs */
        { 116, 56,  0, 0x0420 },   /* 110 Foot Plate                      Feet/Chest */
        { 109, 10, 41, 0x0200 },   /* 111 Large Shield / Sar Shield       Hands */
        { 101, 52,  0, 0x0402 },   /* 112 Helm Of Lyte / Helm Of Ra       Head/Chest */
        {  78, 20,  0, 0x0008 },   /* 113 Plate Of Lyte / Plate Of Ra     Torso */
        {  94, 22,  0, 0x0010 },   /* 114 Poleyn Of Lyte / Poleyn Of Ra   Legs */
        { 117, 56,  0, 0x0420 },   /* 115 Greave Of Lyte / Greave Of Ra   Feet/Chest */
        { 110, 10, 41, 0x0200 },   /* 116 Shield Of Lyte / Shield Of Ra   Hands */
        { 102, 52,  0, 0x0402 },   /* 117 Helm Of Darc / Dragon Helm      Head/Chest */
        {  79, 20,  0, 0x0008 },   /* 118 Plate Of Darc / Dragon Plate    Torso */
        {  95, 22,  0, 0x0010 },   /* 119 Poleyn Of Darc / Dragon Poleyn  Legs */
        { 118, 56,  0, 0x0420 },   /* 120 Greave Of Darc / Dragon Greave  Feet/Chest */
        { 111, 10, 41, 0x0200 },   /* 121 Shield Of Darc / Dragon Shield  Hands */
        { 140, 52,  0, 0x0402 },   /* 122 Dexhelm                         Head/Chest */
        { 141, 19,  0, 0x0408 },   /* 123 Flamebain                       Torso/Chest */
        { 142, 22,  0, 0x0010 },   /* 124 Powertowers                     Legs */
        { 194, 81,  0, 0x0420 },   /* 125 Boots Of Speed                  Feet/Chest */
        { 196, 84,  0, 0x0408 },   /* 126 Halter                          Torso/Chest */
        {   0, 34,  0, 0x0500 },   /* 127 Compass                         Pouch/Chest */
        {   8,  6,  0, 0x0501 },   /* 128 Water / Waterskin               Mouth/Pouch/Chest */
        {  10, 15,  0, 0x0504 },   /* 129 Jewel Symal                     Neck/Pouch/Chest */
        {  12, 15,  0, 0x0504 },   /* 130 Illumulet                       Neck/Pouch/Chest */
        { 146, 40,  0, 0x0500 },   /* 131 Ashes                           Pouch/Chest */
        { 147, 41,  0, 0x0400 },   /* 132 Bones                           Chest */
        { 125,  4, 37, 0x0500 },   /* 133 Copper Coin / Sar Coin          Pouch/Chest */
        { 126, 83, 37, 0x0500 },   /* 134 Silver Coin                     Pouch/Chest */
        { 127,  4, 37, 0x0500 },   /* 135 Gold Coin / Gor Coin            Pouch/Chest */
        { 176, 18,  0, 0x0500 },   /* 136 Iron Key                        Pouch/Chest */
        { 177, 18,  0, 0x0500 },   /* 137 Key Of B                        Pouch/Chest */
        { 178, 18,  0, 0x0500 },   /* 138 Solid Key                       Pouch/Chest */
        { 179, 18,  0, 0x0500 },   /* 139 Square Key                      Pouch/Chest */
        { 180, 18,  0, 0x0500 },   /* 140 Tourquoise Key                  Pouch/Chest */
        { 181, 18,  0, 0x0500 },   /* 141 Cross Key                       Pouch/Chest */
        { 182, 18,  0, 0x0500 },   /* 142 Onyx Key                        Pouch/Chest */
        { 183, 18,  0, 0x0500 },   /* 143 Skeleton Key                    Pouch/Chest */
        { 184, 62,  0, 0x0500 },   /* 144 Gold Key                        Pouch/Chest */
        { 185, 62,  0, 0x0500 },   /* 145 Winged Key                      Pouch/Chest */
        { 186, 62,  0, 0x0500 },   /* 146 Topaz Key                       Pouch/Chest */
        { 187, 62,  0, 0x0500 },   /* 147 Sapphire Key                    Pouch/Chest */
        { 188, 62,  0, 0x0500 },   /* 148 Emerald Key                     Pouch/Chest */
        { 189, 62,  0, 0x0500 },   /* 149 Ruby Key                        Pouch/Chest */
        { 190, 62,  0, 0x0500 },   /* 150 Ra Key                          Pouch/Chest */
        { 191, 62,  0, 0x0500 },   /* 151 Master Key                      Pouch/Chest */
        { 128, 76,  0, 0x0200 },   /* 152 Boulder                         Hands */
        { 129,  3,  0, 0x0500 },   /* 153 Blue Gem                        Pouch/Chest */
        { 130, 60,  0, 0x0500 },   /* 154 Orange Gem                      Pouch/Chest */
        { 131, 61,  0, 0x0500 },   /* 155 Green Gem                       Pouch/Chest */
        { 168, 27,  0, 0x0501 },   /* 156 Apple                           Mouth/Pouch/Chest */
        { 169, 28,  0, 0x0501 },   /* 157 Corn                            Mouth/Pouch/Chest */
        { 170, 25,  0, 0x0501 },   /* 158 Bread                           Mouth/Pouch/Chest */
        { 171, 26,  0, 0x0501 },   /* 159 Cheese                          Mouth/Pouch/Chest */
        { 172, 71,  0, 0x0401 },   /* 160 Screamer Slice                  Mouth/Chest */
        { 173, 70,  0, 0x0401 },   /* 161 Worm Round                      Mouth/Chest */
        { 174,  5,  0, 0x0501 },   /* 162 Drumstick / Shank               Mouth/Pouch/Chest */
        { 175, 66,  0, 0x0501 },   /* 163 Dragon Steak                    Mouth/Pouch/Chest */
        { 120, 15,  0, 0x0504 },   /* 164 Gem Of Ages                     Neck/Pouch/Chest */
        { 121, 15,  0, 0x0504 },   /* 165 Ekkhard Cross                   Neck/Pouch/Chest */
        { 122, 58,  0, 0x0504 },   /* 166 Moonstone                       Neck/Pouch/Chest */
        { 123, 59,  0, 0x0504 },   /* 167 The Hellion                     Neck/Pouch/Chest */
        { 124, 59,  0, 0x0504 },   /* 168 Pendant Feral                   Neck/Pouch/Chest */
        { 132, 79, 38, 0x0500 },   /* 169 Magical Box (Blue)              Pouch/Chest */
        { 133, 63, 38, 0x0500 },   /* 170 Magical Box (Green)             Pouch/Chest */
        { 134, 64,  0, 0x0500 },   /* 171 Mirror Of Dawn                  Pouch/Chest */
        { 136, 72, 39, 0x0400 },   /* 172 Rope                            Chest */
        { 137, 73,  0, 0x0500 },   /* 173 Rabbit's Foot                   Pouch/Chest */
        { 138, 74,  0, 0x0500 },   /* 174 Corbamite / Corbum              Pouch/Chest */
        { 139, 75,  0, 0x0504 },   /* 175 Choker                          Neck/Pouch/Chest */
        { 192, 77,  0, 0x0500 },   /* 176 Lock Picks                      Pouch/Chest */
        { 193, 78,  0, 0x0500 },   /* 177 Magnifier                       Pouch/Chest */
        { 197, 74,  0, 0x0000 },   /* 178 Zokathra Spell                  */
        { 198, 41,  0, 0x0400 } }; /* 179 Bones                           Chest */
WEAPON_INFO G0238_as_Graphic559_WeaponInfo[46] = {
        /* { Weight, Class, Strength, KineticEnergy, Attributes } */
        {   1, 130,   2,   0, 0x2000 },   /* EYE OF TIME */
        {   1, 131,   2,   0, 0x2000 },   /* STORMRING */
        {  11,   0,   8,   2, 0x2000 },   /* TORCH */
        {  12, 112,  10,  80, 0x2028 },   /* FLAMITT */
        {   9, 129,  16,   7, 0x2000 },   /* STAFF OF CLAWS */
        {  30, 113,  49, 110, 0x0942 },   /* BOLT BLADE */
        {  47,   0,  55,  20, 0x0900 },   /* FURY */
        {  24, 255,  25,  10, 0x20FF },   /* THE FIRESTAFF */
        {   5,   2,  10,  19, 0x0200 },   /* DAGGER */
        {  33,   0,  30,   8, 0x0900 },   /* FALCHION */
        {  32,   0,  34,  10, 0x0900 },   /* SWORD */
        {  26,   0,  38,  10, 0x0900 },   /* RAPIER */
        {  35,   0,  42,  11, 0x0900 },   /* SABRE */
        {  36,   0,  46,  12, 0x0900 },   /* SAMURAI SWORD */
        {  33,   0,  50,  14, 0x0900 },   /* DELTA */
        {  37,   0,  62,  14, 0x0900 },   /* DIAMOND EDGE */
        {  30,   0,  48,  13, 0x0000 },   /* VORPAL BLADE */
        {  39,   0,  58,  15, 0x0900 },   /* THE INQUISITOR */
        {  43,   2,  49,  33, 0x0300 },   /* AXE */
        {  65,   2,  70,  44, 0x0300 },   /* HARDCLEAVE */
        {  31,   0,  32,  10, 0x2000 },   /* MACE */
        {  41,   0,  42,  13, 0x2000 },   /* MACE OF ORDER */
        {  50,   0,  60,  15, 0x2000 },   /* MORNINGSTAR */
        {  36,   0,  19,  10, 0x2700 },   /* CLUB */
        { 110,   0,  44,  22, 0x2600 },   /* STONE CLUB */
        {  10,  20,   1,  50, 0x2032 },   /* BOW */
        {  28,  30,   1, 180, 0x2078 },   /* CROSSBOW */
        {   2,  10,   2,  10, 0x0100 },   /* ARROW */
        {   2,  10,   2,  28, 0x0500 },   /* SLAYER */
        {  19,  39,   5,  20, 0x2032 },   /* SLING */
        {  10,  11,   6,  18, 0x2000 },   /* ROCK */
        {   3,  12,   7,  23, 0x0800 },   /* POISON DART */
        {   1,   1,   3,  19, 0x0A00 },   /* THROWING STAR */
        {  8,    0,   4,   4, 0x2000 },   /* STICK */
        {  26, 129,  12,   4, 0x2000 },   /* STAFF */
        {   1, 130,   0,   0, 0x2000 },   /* WAND */
        {   2, 140,   1,  20, 0x2000 },   /* TEOWAND */
        {  35, 128,  18,   6, 0x2000 },   /* YEW STAFF */
        {  29, 159,   0,   4, 0x2000 },   /* STAFF OF MANAR */
        {  21, 131,   0,   3, 0x2000 },   /* SNAKE STAFF */
        {  33, 136,   0,   7, 0x2000 },   /* THE CONDUIT */
        {   8, 132,   3,   1, 0x2000 },   /* DRAGON SPIT */
        {  18, 131,   9,   4, 0x2000 },   /* SCEPTRE OF LYF */
        {   8, 192,   1,   1, 0x2000 },   /* HORN OF FEAR */
        {  30,  26,   1, 220, 0x207D },   /* SPEEDBOW */
        {  36, 255, 100,  50, 0x20FF } }; /* THE FIRESTAFF */
ARMOUR_INFO G0239_as_Graphic559_ArmourInfo[58] = {
        /* { Weight, Defense, Attributes, Unreferenced } */
        {   3,   5,                          1, 0 },   /* CAPE */
        {   4,  10,                          1, 0 },   /* CLOAK OF NIGHT */
        {   3,   4,                          1, 0 },   /* BARBARIAN HIDE */
        {   6,   5,                          2, 0 },   /* SANDALS */
        {  16,  25,                          4, 0 },   /* LEATHER BOOTS */
        {   4,   5,                          0, 0 },   /* ROBE */
        {   4,   5,                          0, 0 },   /* ROBE */
        {   3,   7,                          1, 0 },   /* FINE ROBE */
        {   3,   7,                          1, 0 },   /* FINE ROBE */
        {   4,   6,                          1, 0 },   /* KIRTLE */
        {   2,   4,                          0, 0 },   /* SILK SHIRT */
        {   4,   5,                          1, 0 },   /* TABARD */
        {   5,   7,                          1, 0 },   /* GUNNA */
        {   3,  11,                          2, 0 },   /* ELVEN DOUBLET */
        {   3,  13,                          2, 0 },   /* ELVEN HUKE */
        {   4,  13,                          2, 0 },   /* ELVEN BOOTS */
        {   6,  17,                          3, 0 },   /* LEATHER JERKIN */
        {   8,  20,                          3, 0 },   /* LEATHER PANTS */
        {  14,  20,                          3, 0 },   /* SUEDE BOOTS */
        {   6,  12,                          2, 0 },   /* BLUE PANTS */
        {   5,   9,                          1, 0 },   /* TUNIC */
        {   5,   8,                          1, 0 },   /* GHI */
        {   5,   9,                          1, 0 },   /* GHI TROUSERS */
        {   4,   1,                          4, 0 },   /* CALISTA */
        {   6,   5,                          4, 0 },   /* CROWN OF NERRA */
        {  11,  12,                          5, 0 },   /* BEZERKER HELM */
        {  14,  17,                          5, 0 },   /* HELMET */
        {  15,  20,                          5, 0 },   /* BASINET */
        {  11,  22, MASK0x0080_IS_A_SHIELD | 5, 0 },   /* BUCKLER */
        {  10,  16, MASK0x0080_IS_A_SHIELD | 2, 0 },   /* HIDE SHIELD */
        {  14,  20, MASK0x0080_IS_A_SHIELD | 3, 0 },   /* WOODEN SHIELD */
        {  21,  35, MASK0x0080_IS_A_SHIELD | 4, 0 },   /* SMALL SHIELD */
        {  65,  35,                          5, 0 },   /* MAIL AKETON */
        {  53,  35,                          5, 0 },   /* LEG MAIL */
        {  52,  70,                          7, 0 },   /* MITHRAL AKETON */
        {  41,  55,                          7, 0 },   /* MITHRAL MAIL */
        {  16,  25,                          6, 0 },   /* CASQUE 'N COIF */
        {  16,  30,                          6, 0 },   /* HOSEN */
        {  19,  40,                          7, 0 },   /* ARMET */
        { 120,  65,                          4, 0 },   /* TORSO PLATE */
        {  80,  56,                          4, 0 },   /* LEG PLATE */
        {  28,  37,                          5, 0 },   /* FOOT PLATE */
        {  34,  56, MASK0x0080_IS_A_SHIELD | 4, 0 },   /* LARGE SHIELD */
        {  17,  62,                          5, 0 },   /* HELM OF LYTE */
        { 108, 125,                          4, 0 },   /* PLATE OF LYTE */
        {  72,  90,                          4, 0 },   /* POLEYN OF LYTE */
        {  24,  50,                          5, 0 },   /* GREAVE OF LYTE */
        {  30,  85, MASK0x0080_IS_A_SHIELD | 4, 0 },   /* SHIELD OF LYTE */
        {  35,  76,                          4, 0 },   /* HELM OF DARC */
        { 141, 160,                          4, 0 },   /* PLATE OF DARC */
        {  90, 101,                          4, 0 },   /* POLEYN OF DARC */
        {  31,  60,                          5, 0 },   /* GREAVE OF DARC */
        {  40, 100, MASK0x0080_IS_A_SHIELD | 4, 0 },   /* SHIELD OF DARC */
        {  14,  54,                          6, 0 },   /* DEXHELM */
        {  57,  60,                          7, 0 },   /* FLAMEBAIN */
        {  81,  88,                          4, 0 },   /* POWERTOWERS */
        {   3,  16,                          2, 0 },   /* BOOTS OF SPEED */
        {   2,   3,                          3, 0 } }; /* HALTER */
unsigned char G0241_auc_Graphic559_JunkInfo[53] = {
        1,   /* COMPASS */
        3,   /* WATERSKIN */
        2,   /* JEWEL SYMAL */
        2,   /* ILLUMULET */
        4,   /* ASHES */
        15,  /* BONES */
        1,   /* COPPER COIN */
        1,   /* SILVER COIN */
        1,   /* GOLD COIN */
        2,   /* IRON KEY */
        1,   /* KEY OF B */
        1,   /* SOLID KEY */
        1,   /* SQUARE KEY */
        1,   /* TOURQUOISE KEY */
        1,   /* CROSS KEY */
        1,   /* ONYX KEY */
        1,   /* SKELETON KEY */
        1,   /* GOLD KEY */
        1,   /* WINGED KEY */
        1,   /* TOPAZ KEY */
        1,   /* SAPPHIRE KEY */
        1,   /* EMERALD KEY */
        1,   /* RUBY KEY */
        1,   /* RA KEY */
        1,   /* MASTER KEY */
        81,  /* BOULDER */
        2,   /* BLUE GEM */
        3,   /* ORANGE GEM */
        2,   /* GREEN GEM */
        4,   /* APPLE */
        4,   /* CORN */
        3,   /* BREAD */
        8,   /* CHEESE */
        5,   /* SCREAMER SLICE */
        11,  /* WORM ROUND */
        4,   /* DRUMSTICK */
        6,   /* DRAGON STEAK */
        2,   /* GEM OF AGES */
        3,   /* EKKHARD CROSS */
        2,   /* MOONSTONE */
        2,   /* THE HELLION */
        2,   /* PENDANT FERAL */
        6,   /* MAGICAL BOX */
        9,   /* MAGICAL BOX */
        3,   /* MIRROR OF DAWN */
        10,  /* ROPE */
        1,   /* RABBIT'S FOOT */
        0,   /* CORBAMITE */
        1,   /* CHOKER */
        1,   /* LOCK PICKS */
        2,   /* MAGNIFIER */
        0,   /* ZOKATHRA SPELL */
        8 }; /* BONES */
int16_t G0242_ai_Graphic559_FoodAmounts[8] = {
        500,    /* Apple */
        600,    /* Corn */
        650,    /* Bread */
        820,    /* Cheese */
        550,    /* Screamer Slice */
        350,    /* Worm round */
        990,    /* Drumstick / Shank */
        1400 }; /* Dragon steak */
DOOR_INFO G0254_as_Graphic559_DoorInfo[4] = {
        /* { Attributes, Defense } Melee attacks can only destroy wooden doors because melee attacks are limited to 100 */
        { MASK0x0002_PROJECTILES_CAN_PASS_THROUGH | MASK0x0001_CREATURES_CAN_SEE_THROUGH, 110 },   /* Door type 0 Portcullis */
        { 0,                                                                               42 },   /* Door type 1 Wooden door */
        { 0,                                                                              230 },   /* Door type 2 Iron door */
        { MASK0x0004_ANIMATED | MASK0x0001_CREATURES_CAN_SEE_THROUGH,                     255 } }; /* Door type 3 Ra door */
char G0255_aac_Graphic559_MessageAndScrollEscapeReplacementStrings[32][8] = {
        { 'x',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { '?',  0,  0,  0, 0, 0, 0, 0 }, */
        { 'y',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { '!',  0,  0,  0, 0, 0, 0, 0 }, */
        { 'T','H','E',' ', 0, 0, 0, 0 },
        { 'Y','O','U',' ', 0, 0, 0, 0 },
        { 'z',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { '{',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { '|',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { '}',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { '~',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { '',  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 },
        {   0,  0,  0,  0, 0, 0, 0, 0 } };
char G0256_aac_Graphic559_EscapeReplacementCharacters[32][2] = {
        { 'a', 0 },
        { 'b', 0 },
        { 'c', 0 },
        { 'd', 0 },
        { 'e', 0 },
        { 'f', 0 },
        { 'g', 0 },
        { 'h', 0 },
        { 'i', 0 },
        { 'j', 0 },
        { 'k', 0 },
        { 'l', 0 },
        { 'm', 0 },
        { 'n', 0 },
        { 'o', 0 },
        { 'p', 0 },
        { 'q', 0 },
        { 'r', 0 },
        { 's', 0 },
        { 't', 0 },
        { 'u', 0 },
        { 'v', 0 },
        { 'w', 0 },
        { 'x', 0 },
        { '0', 0 },
        { '1', 0 },
        { '2', 0 },
        { '3', 0 },
        { '4', 0 },
        { '5', 0 },
        { '6', 0 },
        { '7', 0 } };
char G0257_aac_Graphic559_InscriptionEscapeReplacementStrings[32][8] = {
        { 28,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 29,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 19,  7,  4, 26, 0, 0, 0, 0 },
        { 24, 14, 20, 26, 0, 0, 0, 0 },
        { 30,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 31,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 32,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 33,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 34,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        { 35,  0,  0,  0, 0, 0, 0, 0 }, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 },
        {  0,  0,  0,  0, 0, 0, 0, 0 } };
unsigned char G0258_auc_Graphic559_GroupDirections[4] = { 0x00, 0x55, 0xAA, 0xFF };
CREATURE_INFO G0243_as_Graphic559_CreatureInfo[C027_CREATURE_TYPE_COUNT] = {
        {  0,  4,  0x0482,  0x623D,   8,  20,  55, 150, 150, 240,  55, 0, 0x1153, 0x29AB, 0x0876, 0x0254, 0xFD40, 4, 0 },
        {  1, 14,  0x0480,  0xA625,  15,  32,  20, 110,  80,  15,  20, 0, 0x3132, 0x33A9, 0x0E42, 0x0384, 0xFC41, 3, 0 },
        {  2,  6,  0x4510,  0x6198,   3,   5,  50,  10,  10,   0, 110, 0, 0x1376, 0x710A, 0x0235, 0x0222, 0xFD20, 0, 0 },
        {  3,  0,  0x04B4,  0xB225,  10,  21,  30,  40,  58,   0,  80, 0, 0x320A, 0x96AA, 0x0B3C, 0x0113, 0xF910, 5, 0 },
        {  4, 18,  0x0701,  0xA3B8,   9,   8,  45, 101,  90,   0,  65, 0, 0x1554, 0x58FF, 0x0A34, 0x0143, 0xFE93, 4, 0 },
        {  5, 17,  0x0581,  0x539D,  20,  18, 100,  60,  30,   0,  30, 0, 0x1232, 0x4338, 0x0583, 0x0265, 0xFFD6, 3, 0 },
        {  6,  3,  0x070C,  0x0020, 120,  10,   5, 165,   5,   0,   5, 0, 0x1111, 0x10F1, 0x0764, 0x02F2, 0xFC84, 6, 0 },
        {  7,  7,  0x0300,  0x0220, 185,  15, 170,  50,  40,   5,  10, 0, 0x1463, 0x25C4, 0x06E3, 0x01F4, 0xFD93, 4, 0 },
        {  8,  2,  0x5864,  0x5225,  11,  16,  15,  30,  55,   0,  80, 0, 0x1423, 0x4684, 0x0FC8, 0x0116, 0xFB30, 6, 0 },
        {  9, 10,  0x0282,  0x71B8,  21,  14, 240, 120, 219,   0,  35, 0, 0x1023, 0x3BFF, 0x0FF7, 0x04F3, 0xF920, 3, 0 },
        { 10, 13,  0x1480,  0x11B8,  17,  12,  25,  33,  20,   0,  40, 0, 0x1224, 0x5497, 0x0F15, 0x0483, 0xFB20, 3, 0 },
        { 11,  0,  0x18C6,  0x0225, 255,   8,  45,  80, 105,   0,  60, 0, 0x1312, 0x55A5, 0x0FF9, 0x0114, 0xFD95, 1, 0 },
        { 12, 11,  0x1280,  0x6038,   7,   7,  22,  20,  22,   0,  80, 0, 0x1013, 0x6596, 0x0F63, 0x0132, 0xFA30, 4, 0 },
        { 13,  9,  0x14A2,  0xB23D,   5,  10,  42,  39,  90, 100,  88, 0, 0x1343, 0x5745, 0x0638, 0x0112, 0xFA30, 4, 0 },
        { 14, 16,  0x05B8,  0x1638,  10,  20,  47,  44,  75,   0,  90, 0, 0x4335, 0xD952, 0x035B, 0x0664, 0xFD60, 5, 0 },
        { 15,  5,  0x0381,  0x523D,  18,  19,  72,  70,  45,  35,  35, 0, 0x1AA1, 0x15AB, 0x0B93, 0x0253, 0xFFC5, 4, 0 },
        { 16, 10,  0x0680,  0xA038,  13,   8,  28,  20,  25,   0,  41, 0, 0x1343, 0x2148, 0x0321, 0x0332, 0xFC30, 3, 0 },
        { 17, 15,  0x04A0,  0xF23D,   1,  16, 180,   8,  28,  20, 150, 0, 0x1432, 0x19FD, 0x0004, 0x0112, 0xF710, 4, 0 },
        { 18, 12,  0x0280,  0xA3BD,  14,   6, 140,  60, 105,   0,  70, 0, 0x1005, 0x7AFF, 0x0FFA, 0x0143, 0xFA30, 4, 0 },
        { 19,  0,  0x4060,  0xE23D,   5,  18,  15,  33,  61,   0,  65, 0, 0x3258, 0xAC97, 0x0F56, 0x0117, 0xFC40, 5, 0 },
        { 20,  8,  0x10DE,  0x0225,  25,  25,  75, 144,  66,   0,  50, 0, 0x1381, 0x7699, 0x0EA7, 0x0345, 0xFD93, 3, 0 },
        { 21,  3,  0x0082,  0xA3BD,   7,  15,  33,  77, 130,   0,  60, 0, 0x1592, 0x696A, 0x0859, 0x0224, 0xFC30, 4, 0 },
        { 22, 16,  0x1480,  0x53BD,  10,  14,  68, 100, 100,   0,  75, 0, 0x4344, 0xBDF9, 0x0A5D, 0x0124, 0xF920, 3, 0 },
        { 23,  0,  0x78AA,  0x0038,  12,  22, 255, 180, 210,   0, 130, 0, 0x6369, 0xFFA7, 0x0FBF, 0x0564, 0xFB52, 5, 0 },
        { 24,  1,  0x068A,  0x97BD,  13,  28, 110, 255, 255,   0,  70, 0, 0x3645, 0xBFBC, 0x06CD, 0x0445, 0xFC30, 4, 0 },
        { 25,  0,  0x78AA,  0x0000,  12,  22, 255, 180, 210,   0, 130, 0, 0x6369, 0xFFBC, 0x0FBF, 0x0564, 0xFB52, 5, 0 },
        { 26,  0,  0x78AA,  0x0000, 255, 255, 255, 180, 210,   0, 130, 0, 0x6369, 0xFFF0, 0x0FBF, 0x0564, 0xFB52, 5, 0 } };
char G2003_aauc_CreatureSounds[18][2] = { /* Replaces G0244_auc_Graphic559_CreatureAttackSounds */
        { C23_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON,                C33_SOUND_MOVE_RED_DRAGON },
        { C25_SOUND_ATTACK_MUMMY_GHOST_RIVE,                             CM1_SOUND_NONE },
        { C19_SOUND_ATTACK_SCREAMER_OITU,                                C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU },
        { C20_SOUND_ATTACK_GIANT_SCORPION_SCORPION,                      C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU },
        { C21_SOUND_ATTACK_MAGENTA_WORM_WORM,                            C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU },
        { C22_SOUND_ATTACK_GIGGLER,                                      C30_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON },
        { C24_SOUND_ATTACK_ROCK_ROCKPILE,                                C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU },
        { C26_SOUND_ATTACK_WATER_ELEMENTAL,                              C32_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL },
        { C27_SOUND_ATTACK_COUATL,                                       C29_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER },
        { C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM,        C30_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON },
        { M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, C34_SOUND_MOVE_SKELETON },
        { M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, C28_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT },
        { C25_SOUND_ATTACK_MUMMY_GHOST_RIVE,                             C30_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON },
        { CM1_SOUND_NONE,                                                C32_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL },
        { CM1_SOUND_NONE,                                                C29_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER },
        { CM1_SOUND_NONE,                                                C30_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON },
        { CM1_SOUND_NONE,                                                C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU },
        { C23_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON,                C31_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU } };
unsigned int16_t G0245_aui_Graphic559_FixedPossessionsCreature12Skeleton[3] = {
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C09_WEAPON_FALCHION,
        C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + C30_ARMOUR_WOODEN_SHIELD, 0 };
unsigned int16_t G0246_aui_Graphic559_FixedPossessionsCreature09StoneGolem[2] = {
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C24_WEAPON_STONE_CLUB, 0 };
unsigned int16_t G0247_aui_Graphic559_FixedPossessionsCreature16Trolin_Antman[2] = {
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C23_WEAPON_CLUB, 0 };
unsigned int16_t G0248_aui_Graphic559_FixedPossessionsCreature18AnimatedArmour_DethKnight[7] = {
        C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + C41_ARMOUR_FOOT_PLATE,
        C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + C40_ARMOUR_LEG_PLATE,
        C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + C39_ARMOUR_TORSO_PLATE,
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C10_WEAPON_SWORD,
        C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + C38_ARMOUR_ARMET,
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C10_WEAPON_SWORD, 0 };
unsigned int16_t G0249_aui_Graphic559_FixedPossessionsCreature07Rock_RockPile[5] = {
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C25_JUNK_BOULDER,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C25_JUNK_BOULDER | MASK0x8000_RANDOM_DROP,
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C30_WEAPON_ROCK | MASK0x8000_RANDOM_DROP,
        C023_OBJECT_INFO_INDEX_FIRST_WEAPON + C30_WEAPON_ROCK | MASK0x8000_RANDOM_DROP, 0 };
unsigned int16_t G0250_aui_Graphic559_FixedPossessionsCreature04PainRat_Hellhound[3] = {
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C35_JUNK_DRUMSTICK_SHANK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C35_JUNK_DRUMSTICK_SHANK | MASK0x8000_RANDOM_DROP, 0 };
unsigned int16_t G0251_aui_Graphic559_FixedPossessionsCreature06Screamer[3] = {
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C33_JUNK_SCREAMER_SLICE,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C33_JUNK_SCREAMER_SLICE | MASK0x8000_RANDOM_DROP, 0 };
unsigned int16_t G0252_aui_Graphic559_FixedPossessionsCreature15MagentaWorm_Worm[4] = {
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C34_JUNK_WORM_ROUND,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C34_JUNK_WORM_ROUND | MASK0x8000_RANDOM_DROP,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C34_JUNK_WORM_ROUND | MASK0x8000_RANDOM_DROP, 0 };
unsigned int16_t G0253_aui_Graphic559_FixedPossessionsCreature24RedDragon[11] = {
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK | MASK0x8000_RANDOM_DROP,
        C127_OBJECT_INFO_INDEX_FIRST_JUNK + C36_JUNK_DRAGON_STEAK | MASK0x8000_RANDOM_DROP, 0 };
unsigned int16_t* G0260_pui_DungeonTextData;
unsigned char G0261_auc_CurrentMapWallOrnamentIndices[16];
unsigned char G0262_auc_CurrentMapFloorOrnamentIndices[16]; /* Last byte is not used */
unsigned char G0263_auc_CurrentMapDoorOrnamentIndices[17]; /* Last two bytes are not used */
unsigned char* G0264_puc_CurrentMapAllowedCreatureTypes;
int16_t G0265_i_CurrentMapInscriptionWallOrnamentIndex;
int16_t G0266_i_CurrentMapViAltarWallOrnamentIndex;
int16_t G0267_ai_CurrentMapAlcoveOrnamentIndices[C003_ALCOVE_ORNAMENT_COUNT];
int16_t G0268_ai_CurrentMapFountainOrnamentIndices[C001_FOUNTAIN_ORNAMENT_COUNT];
MAP* G0269_ps_CurrentMap;
unsigned int16_t* G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount; /* Pointer to value in G0280_pui_DungeonColumnsCumulativeSquareThingCount for the first column of the current map */
unsigned char** G0271_ppuc_CurrentMapData;
int16_t G0272_i_CurrentMapIndex = CM1_MAP_INDEX_NONE;
int16_t G0273_i_CurrentMapWidth;
int16_t G0274_i_CurrentMapHeight;
DOOR_INFO G0275_as_CurrentMapDoorInfo[2];
unsigned char* G0276_puc_DungeonRawMapData;
MAP* G0277_ps_DungeonMaps;
DUNGEON_HEADER* G0278_ps_DungeonHeader;
unsigned char*** G0279_pppuc_DungeonMapData;
unsigned int16_t* G0280_pui_DungeonColumnsCumulativeSquareThingCount; /* For each column of each map in the dungeon, cumulative count of things in G0283_pT_SquareFirstThings present in preceding columns */
unsigned int16_t* G0281_pui_DungeonMapsFirstColumnIndex;
unsigned int16_t G0282_ui_DungeonColumnCount;
THING* G0283_pT_SquareFirstThings;
unsigned char* G0284_apuc_ThingData[16];
int16_t G0285_i_SquareAheadElement;
BOOLEAN G0286_B_FacingAlcove;
BOOLEAN G0287_B_FacingViAltar;
BOOLEAN G0288_B_FacingFountain;
int16_t G0289_i_DungeonView_ChampionPortraitOrdinal;
THING G0290_T_DungeonView_InscriptionThing;
int16_t G2210_aai_XYZ_DungeonViewClickable[6][4]; /* Replaces G0291_aauc_DungeonViewClickableBoxes. Four zones for objects on top of piles on the floor, one zone for object on top of pile in alcove and one zone for a door button or a wall ornament */
THING G0292_aT_PileTopObject[5];



BOOLEAN F0139_DUNGEON_IsCreatureAllowedOnMap(
THING            P0234_T_Thing     SEPARATOR
unsigned int16_t P0235_ui_MapIndex FINAL_SEPARATOR
{
        REGISTER unsigned char* L0236_puc_Multiple;
#define AL0236_puc_Group               L0236_puc_Multiple
#define AL0236_puc_AllowedCreatureType L0236_puc_Multiple
        REGISTER MAP* L0237_ps_Map;
        REGISTER int16_t L0234_i_Counter;
        REGISTER unsigned int16_t L0235_ui_CreatureType;


        AL0236_puc_Group = F0156_DUNGEON_GetThingData(P0234_T_Thing);
        L0235_ui_CreatureType = ((GROUP*)AL0236_puc_Group)->Type;
        L0237_ps_Map = &G0277_ps_DungeonMaps[P0235_ui_MapIndex];
        AL0236_puc_AllowedCreatureType = G0279_pppuc_DungeonMapData[P0235_ui_MapIndex][L0237_ps_Map->A.Width] + L0237_ps_Map->A.Height + 1;
        for (L0234_i_Counter = L0237_ps_Map->C.CreatureTypeCount; L0234_i_Counter > 0; L0234_i_Counter--) {
                if (*AL0236_puc_AllowedCreatureType++ == L0235_ui_CreatureType) {
                        return C1_TRUE;
                }
        }
        return C0_FALSE;
}

unsigned int16_t F0140_DUNGEON_GetObjectWeight(
REGISTER THING P0236_T_Thing FINAL_SEPARATOR
{
        REGISTER JUNK* L0238_ps_Junk;
        REGISTER unsigned int16_t L0239_ui_Weight;


        if (P0236_T_Thing == C0xFFFF_THING_NONE) {
                return 0;
        }
        L0238_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(P0236_T_Thing);
        switch M012_TYPE(P0236_T_Thing) {
                case C05_THING_TYPE_WEAPON:
                        L0239_ui_Weight = G0238_as_Graphic559_WeaponInfo[((WEAPON*)L0238_ps_Junk)->Type].Weight;
                        break;
                case C06_THING_TYPE_ARMOUR:
                        L0239_ui_Weight = G0239_as_Graphic559_ArmourInfo[((ARMOUR*)L0238_ps_Junk)->Type].Weight;
                        break;
                case C10_THING_TYPE_JUNK:
                        L0239_ui_Weight = G0241_auc_Graphic559_JunkInfo[L0238_ps_Junk->Type];
                        if (L0238_ps_Junk->Type == C01_JUNK_WATERSKIN) {
                               L0239_ui_Weight += L0238_ps_Junk->ChargeCount << 1;
                        }
                        break;
                case C09_THING_TYPE_CONTAINER:
                        L0239_ui_Weight = 50;
                        P0236_T_Thing = ((CONTAINER*)L0238_ps_Junk)->Slot;
                        while (P0236_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                                L0239_ui_Weight += F0140_DUNGEON_GetObjectWeight(P0236_T_Thing);
                                P0236_T_Thing = F0159_DUNGEON_GetNextThing(P0236_T_Thing);
                        }
                        break;
                case C08_THING_TYPE_POTION:
                        if (((POTION*)L0238_ps_Junk)->Type == C20_POTION_EMPTY_FLASK) {
                                L0239_ui_Weight = 1;
                        } else {
                                L0239_ui_Weight = 3;
                        }
                        break;
                case C07_THING_TYPE_SCROLL:
                        L0239_ui_Weight = 1;
                        break;
        }
        return L0239_ui_Weight;
}

int16_t F0141_DUNGEON_GetObjectInfoIndex(
THING P0237_T_Thing FINAL_SEPARATOR
{
        REGISTER GENERIC* L0240_ps_Generic;


        L0240_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0237_T_Thing);
        switch M012_TYPE(P0237_T_Thing) {
                case C07_THING_TYPE_SCROLL:
                        return C000_OBJECT_INFO_INDEX_FIRST_SCROLL;
                case C09_THING_TYPE_CONTAINER:
                        return C001_OBJECT_INFO_INDEX_FIRST_CONTAINER + ((CONTAINER*)L0240_ps_Generic)->Type;
                case C10_THING_TYPE_JUNK:
                        return C127_OBJECT_INFO_INDEX_FIRST_JUNK + ((JUNK*)L0240_ps_Generic)->Type;
                case C05_THING_TYPE_WEAPON:
                        return C023_OBJECT_INFO_INDEX_FIRST_WEAPON + ((WEAPON*)L0240_ps_Generic)->Type;
                case C06_THING_TYPE_ARMOUR:
                        return C069_OBJECT_INFO_INDEX_FIRST_ARMOUR + ((ARMOUR*)L0240_ps_Generic)->Type;
                case C08_THING_TYPE_POTION:
                        return C002_OBJECT_INFO_INDEX_FIRST_POTION + ((POTION*)L0240_ps_Generic)->Type;
                default:
                        return -1;
        }
}

/* Returns a negative value (PROJECTIL_ASPECT ordinal) or positive value (OBJECT_ASPECT index) */
int16_t F0142_DUNGEON_GetProjectileAspect(
REGISTER THING P0238_T_Thing FINAL_SEPARATOR
{
        REGISTER WEAPON_INFO* L0243_ps_WeaponInfo;
        REGISTER int16_t L0241_i_ThingType;
        REGISTER int16_t L0242_i_ProjectileAspectOrdinal;


        if ((L0241_i_ThingType = M012_TYPE(P0238_T_Thing)) == C15_THING_TYPE_EXPLOSION) {
                if (P0238_T_Thing == C0xFF80_THING_EXPLOSION_FIREBALL) {
                        return -M000_INDEX_TO_ORDINAL(C10_PROJECTILE_ASPECT_EXPLOSION_FIREBALL);
                }
                else {
                        if (P0238_T_Thing == C0xFF81_THING_EXPLOSION_SLIME) {
                                return -M000_INDEX_TO_ORDINAL(C12_PROJECTILE_ASPECT_EXPLOSION_SLIME);
                        }
                        else {
                                if (P0238_T_Thing == C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT) {
                                        return -M000_INDEX_TO_ORDINAL(C03_PROJECTILE_ASPECT_EXPLOSION_LIGHTNING_BOLT);
                                }
                                else {
                                        if ((P0238_T_Thing == C0xFF86_THING_EXPLOSION_POISON_BOLT) || (P0238_T_Thing == C0xFF87_THING_EXPLOSION_POISON_CLOUD)) {
                                                return -M000_INDEX_TO_ORDINAL(C13_PROJECTILE_ASPECT_EXPLOSION_POISON_BOLT_POISON_CLOUD);
                                        }
                                        else {
                                                return -M000_INDEX_TO_ORDINAL(C11_PROJECTILE_ASPECT_EXPLOSION_DEFAULT);
                                        }
                                }
                        }
                }
        } else {
                if (L0241_i_ThingType == C05_THING_TYPE_WEAPON) {
                        L0243_ps_WeaponInfo = F0158_DUNGEON_GetWeaponInfo(P0238_T_Thing);
                        if (L0242_i_ProjectileAspectOrdinal = M066_PROJECTILE_ASPECT_ORDINAL(L0243_ps_WeaponInfo->Attributes)) {
                                return -L0242_i_ProjectileAspectOrdinal;
                        }
                }
        }
        return G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(P0238_T_Thing)].ObjectAspectIndex;
}

unsigned int16_t F0143_DUNGEON_GetArmourDefense(
REGISTER ARMOUR_INFO* P0239_ps_ArmourInfo     SEPARATOR
BOOLEAN               P0240_B_UseSharpDefense FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0244_ui_Defense;


        L0244_ui_Defense = P0239_ps_ArmourInfo->Defense;
        if (P0240_B_UseSharpDefense) {
                L0244_ui_Defense = F0030_MAIN_GetScaledProduct(L0244_ui_Defense, 3, M007_GET(P0239_ps_ArmourInfo->Attributes, MASK0x0007_SHARP_DEFENSE) + 4);
        }
        return L0244_ui_Defense;
}

unsigned int16_t F0144_DUNGEON_GetCreatureAttributes(
THING P0241_T_Thing FINAL_SEPARATOR
{
        GROUP* L0245_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0241_T_Thing);


        return G0243_as_Graphic559_CreatureInfo[L0245_ps_Group->Type].Attributes;
}

unsigned int16_t F0145_DUNGEON_GetGroupCells(
GROUP*           P0242_ps_Group    SEPARATOR
unsigned int16_t P0243_ui_MapIndex FINAL_SEPARATOR
{
        REGISTER unsigned char L0246_uc_Cells;


        L0246_uc_Cells = P0242_ps_Group->Cells;
        if (P0243_ui_MapIndex == G0309_i_PartyMapIndex) {
                L0246_uc_Cells = G0375_ps_ActiveGroups[L0246_uc_Cells].Cells;
        }
        return L0246_uc_Cells;
}

void F0146_DUNGEON_SetGroupCells(
GROUP*           P0244_ps_Group    SEPARATOR
unsigned int16_t P0245_ui_Cells    SEPARATOR
unsigned int16_t P0246_ui_MapIndex FINAL_SEPARATOR
{
        if (P0246_ui_MapIndex == G0309_i_PartyMapIndex) {
                G0375_ps_ActiveGroups[P0244_ps_Group->ActiveGroupIndex].Cells = P0245_ui_Cells;
        } else {
                P0244_ps_Group->Cells = P0245_ui_Cells;
        }
}

unsigned int16_t F0147_DUNGEON_GetGroupDirections(
GROUP*           P0247_ps_Group    SEPARATOR
unsigned int16_t P0248_ui_MapIndex FINAL_SEPARATOR
{
        if (P0248_ui_MapIndex == G0309_i_PartyMapIndex) {
                return G0375_ps_ActiveGroups[P0247_ps_Group->ActiveGroupIndex].Directions;
        }
        else {
                return G0258_auc_Graphic559_GroupDirections[P0247_ps_Group->Direction];
        }
}

void F0148_DUNGEON_SetGroupDirections(
GROUP*           P0249_ps_Group      SEPARATOR
unsigned int16_t P0250_ui_Directions SEPARATOR
unsigned int16_t P0251_ui_MapIndex   FINAL_SEPARATOR
{
        if (P0251_ui_MapIndex == G0309_i_PartyMapIndex) {
                G0375_ps_ActiveGroups[P0249_ps_Group->ActiveGroupIndex].Directions = P0250_ui_Directions;
        } else {
                P0249_ps_Group->Direction = M021_NORMALIZE(P0250_ui_Directions);
        }
}

BOOLEAN F0149_DUNGEON_IsWallOrnamentAnAlcove(
int16_t P0252_i_WallOrnamentIndex FINAL_SEPARATOR
{
        REGISTER int16_t L0247_i_Counter;


        if (P0252_i_WallOrnamentIndex >= 0) {
                for (L0247_i_Counter = 0; L0247_i_Counter < C003_ALCOVE_ORNAMENT_COUNT; L0247_i_Counter++) {
                        if (G0267_ai_CurrentMapAlcoveOrnamentIndices[L0247_i_Counter] == P0252_i_WallOrnamentIndex) {
                                return C1_TRUE;
                        }
                }
        }
        return C0_FALSE;
}

BOOLEAN F0601_IsWallOrnamentAFountain(
int16_t P2002_i_WallOrnamentIndex FINAL_SEPARATOR
{
        REGISTER int16_t L0247_i_Counter;


        if (P2002_i_WallOrnamentIndex >= 0) {
                for (L0247_i_Counter = 0; L0247_i_Counter < C001_FOUNTAIN_ORNAMENT_COUNT; L0247_i_Counter++) {
                        if (G0268_ai_CurrentMapFountainOrnamentIndices[L0247_i_Counter] == P2002_i_WallOrnamentIndex) {
                                return C1_TRUE;
                        }
                }
        }
        return C0_FALSE;
}


void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(
REGISTER int16_t  P0253_i_Direction         SEPARATOR
REGISTER int16_t  P0254_i_StepsForwardCount SEPARATOR
REGISTER int16_t  P0255_i_StepsRightCount   SEPARATOR
REGISTER int16_t* P0256_pi_MapX             SEPARATOR
REGISTER int16_t* P0257_pi_MapY             FINAL_SEPARATOR
{
        *P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount, *P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0254_i_StepsForwardCount;
        P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */
        *P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount, *P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0255_i_StepsRightCount;
}

unsigned char F0151_DUNGEON_GetSquare(
REGISTER int16_t P0258_i_MapX SEPARATOR
REGISTER int16_t P0259_i_MapY FINAL_SEPARATOR
{
        REGISTER int16_t L0248_i_Multiple;
#define AL0248_B_IsMapXInBounds L0248_i_Multiple
#define AL0248_i_SquareType     L0248_i_Multiple
        REGISTER int16_t L0249_i_Multiple;
#define AL0249_B_IsMapYInBounds L0249_i_Multiple
#define AL0249_i_SquareType     L0249_i_Multiple


        AL0249_B_IsMapYInBounds = (P0259_i_MapY >= 0) && (P0259_i_MapY < G0274_i_CurrentMapHeight);
        if ((AL0248_B_IsMapXInBounds = (P0258_i_MapX >= 0) && (P0258_i_MapX < G0273_i_CurrentMapWidth)) && AL0249_B_IsMapYInBounds) {
                return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY];
        }
        else {
                /* If processing goes below this line then either P0258_i_MapX or P0259_i_MapY is out of the map bounds and the returned square type will be C00_ELEMENT_WALL. The use of uninitialized variables in conditions does not have any visible consequence */
                if (AL0249_B_IsMapYInBounds) {
                        if (((P0258_i_MapX == -1) && ((AL0249_i_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[0][P0259_i_MapY])) == C01_ELEMENT_CORRIDOR)) || (AL0249_i_SquareType == C02_ELEMENT_PIT)) { /* BUG0_01 Use of uninitialized variable AL0249_i_SquareType in comparison (AL0249_i_SquareType == C02_ELEMENT_PIT) if P0258_i_MapX != -1. No consequence here as AL0249_i_SquareType = AL0249_B_IsMapYInBounds = 0 (C0_FALSE) or 1 (C1_TRUE) and thus cannot be equal to 2 (C02_ELEMENT_PIT) */
                                return M035_SQUARE(C00_ELEMENT_WALL, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);
                        }
                        else {
                                if (((P0258_i_MapX == G0273_i_CurrentMapWidth) && ((AL0249_i_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[G0273_i_CurrentMapWidth - 1][P0259_i_MapY])) == C01_ELEMENT_CORRIDOR)) || (AL0249_i_SquareType == C02_ELEMENT_PIT)) { /* BUG0_01 Use of uninitialized variable AL0249_i_SquareType in comparison (AL0249_i_SquareType == C02_ELEMENT_PIT) if P0258_i_MapX != G0273_i_CurrentMapWidth. If P0258_i_MapX == -1 then AL0249_i_SquareType contains the square type from the previous test: no consequence here as AL0249_i_SquareType cannot be equal to 2 (C02_ELEMENT_PIT) otherwise the previous test would have succeeded and the function would have returned. If P0258_i_MapX != -1 then AL0249_i_SquareType = AL0249_B_IsMapYInBounds = 0 (C0_FALSE) or 1 (C1_TRUE): no consequence as it cannot be equal to 2 (C02_ELEMENT_PIT) */ /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                        return M035_SQUARE(C00_ELEMENT_WALL, MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED);
                                }
                        }
                } else {
                        if (AL0248_B_IsMapXInBounds) {
                                if (((P0259_i_MapY == -1) && ((AL0248_i_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[P0258_i_MapX][0])) == C01_ELEMENT_CORRIDOR)) || (AL0248_i_SquareType == C02_ELEMENT_PIT)) { /* BUG0_01 Use of uninitialized variable AL0248_i_SquareType in comparison (AL0248_i_SquareType == C02_ELEMENT_PIT) if P0259_i_MapY != -1. If the value of register D5 (storing AL0248_i_SquareType) is 2 before the function call then the condition is true and if P0259_i_MapY == G0274_i_CurrentMapHeight then this can prevent a wall on the south border of the map to allow a random ornament on its north side. No consequence as the only case where this would have a consequence is in F0172_DUNGEON_SetSquareAspect where register D5 contains the same value as P0259_i_MapY so it is not possible to have both AL0248_i_SquareType == 2 and P0259_i_MapY == G0274_i_CurrentMapHeight */
                                        return M035_SQUARE(C00_ELEMENT_WALL, MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED);
                                }
                                else {
                                        if (((P0259_i_MapY == G0274_i_CurrentMapHeight) && ((AL0248_i_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[P0258_i_MapX][G0274_i_CurrentMapHeight - 1])) == C01_ELEMENT_CORRIDOR)) || (AL0248_i_SquareType == C02_ELEMENT_PIT)) { /* BUG0_01 Use of uninitialized variable AL0248_i_SquareType in comparison (AL0248_i_SquareType == C02_ELEMENT_PIT) if P0259_i_MapY != G0274_i_CurrentMapHeight. If P0259_i_MapY == -1 then AL0248_i_SquareType contains the square type from the previous test: no consequence here as AL0248_i_SquareType cannot be equal to 2 (C02_ELEMENT_PIT) otherwise the previous test would have succeeded and the function would have returned. If P0259_i_MapY != -1 then AL0248_i_SquareType contains the value of D5 before the function call which cannot be 2 (see previous condition) */
                                                return M035_SQUARE(C00_ELEMENT_WALL, MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED);
                                        }
                                }
                        }
                }
                return M035_SQUARE(C00_ELEMENT_WALL, 0);
        }
}

unsigned char F0152_DUNGEON_GetRelativeSquare(
int16_t P0260_i_Direction         SEPARATOR
int16_t P0261_i_StepsForwardCount SEPARATOR
int16_t P0262_i_StepsRightCount   SEPARATOR
int16_t P0263_i_MapX              SEPARATOR
int16_t P0264_i_MapY              FINAL_SEPARATOR
{
        F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0260_i_Direction, P0261_i_StepsForwardCount, P0262_i_StepsRightCount, &P0263_i_MapX, &P0264_i_MapY);
        return F0151_DUNGEON_GetSquare(P0263_i_MapX, P0264_i_MapY);
}

int16_t F0153_DUNGEON_GetRelativeSquareType(
int16_t P0265_i_Direction         SEPARATOR
int16_t P0266_i_StepsForwardCount SEPARATOR
int16_t P0267_i_StepsRightCount   SEPARATOR
int16_t P0268_i_MapX              SEPARATOR
int16_t P0269_i_MapY              FINAL_SEPARATOR
{
        return M034_SQUARE_TYPE(F0152_DUNGEON_GetRelativeSquare(P0265_i_Direction, P0266_i_StepsForwardCount, P0267_i_StepsRightCount, P0268_i_MapX, P0269_i_MapY));
}

int16_t F0154_DUNGEON_GetLocationAfterLevelChange(
int16_t  P0270_i_MapIndex   SEPARATOR
int16_t  P0271_i_LevelDelta SEPARATOR
int16_t* P0272_pi_MapX      SEPARATOR
int16_t* P0273_pi_MapY      FINAL_SEPARATOR
{
        REGISTER MAP* L0254_ps_Map;
        REGISTER int16_t L0253_i_Offset;
        REGISTER int16_t L0250_i_NewMapX;
        REGISTER int16_t L0251_i_NewMapY;
        REGISTER int16_t L0255_i_TargetMapIndex;
        REGISTER int16_t L0252_i_NewLevel;


        if (G0309_i_PartyMapIndex == C255_MAP_INDEX_ENTRANCE) {
                return CM1_MAP_INDEX_NONE;
        }
        L0254_ps_Map = G0277_ps_DungeonMaps + P0270_i_MapIndex;
        L0250_i_NewMapX = L0254_ps_Map->OffsetMapX + *P0272_pi_MapX;
        L0251_i_NewMapY = L0254_ps_Map->OffsetMapY + *P0273_pi_MapY;
        L0252_i_NewLevel = L0254_ps_Map->A.Level + P0271_i_LevelDelta;
        L0254_ps_Map = G0277_ps_DungeonMaps;
        for (L0255_i_TargetMapIndex = 0; L0255_i_TargetMapIndex < G0278_ps_DungeonHeader->MapCount; L0255_i_TargetMapIndex++) {
                if ((L0254_ps_Map->A.Level == L0252_i_NewLevel) && (L0250_i_NewMapX >= (L0253_i_Offset = L0254_ps_Map->OffsetMapX)) && (L0250_i_NewMapX <= (L0253_i_Offset + L0254_ps_Map->A.Width)) && (L0251_i_NewMapY >= (L0253_i_Offset = L0254_ps_Map->OffsetMapY)) && (L0251_i_NewMapY <= (L0253_i_Offset + L0254_ps_Map->A.Height))) {
                        *P0273_pi_MapY = L0251_i_NewMapY - L0253_i_Offset;
                        *P0272_pi_MapX = L0250_i_NewMapX - L0254_ps_Map->OffsetMapX;
                        return L0255_i_TargetMapIndex;
                }
                L0254_ps_Map++;
        }
        return CM1_MAP_INDEX_NONE;
}

int16_t F0155_DUNGEON_GetStairsExitDirection(
REGISTER int16_t P0274_i_MapX SEPARATOR
REGISTER int16_t P0275_i_MapY FINAL_SEPARATOR
{
        REGISTER BOOLEAN L0257_B_NorthSouthOrientedStairs;
        REGISTER int16_t L0256_i_SquareType;


        if (L0257_B_NorthSouthOrientedStairs = !M007_GET(F0151_DUNGEON_GetSquare(P0274_i_MapX, P0275_i_MapY), MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION)) {
                P0274_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[C1_DIRECTION_EAST], P0275_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[C1_DIRECTION_EAST];
        } else {
                P0274_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[C0_DIRECTION_NORTH], P0275_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[C0_DIRECTION_NORTH];
        }
        return ((((L0256_i_SquareType = M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(P0274_i_MapX, P0275_i_MapY))) == C00_ELEMENT_WALL) || (L0256_i_SquareType == C03_ELEMENT_STAIRS)) << 1) + L0257_B_NorthSouthOrientedStairs;
}

unsigned char* F0156_DUNGEON_GetThingData(
REGISTER THING P0276_T_Thing FINAL_SEPARATOR
{
        return G0284_apuc_ThingData[M012_TYPE(P0276_T_Thing)] + (M013_INDEX(P0276_T_Thing) * G0235_auc_Graphic559_ThingDataByteCount[M012_TYPE(P0276_T_Thing)]);
}

unsigned char* F0157_DUNGEON_GetSquareFirstThingData(
int16_t P0277_i_MapX SEPARATOR
int16_t P0278_i_MapY FINAL_SEPARATOR
{
        return F0156_DUNGEON_GetThingData(F0161_DUNGEON_GetSquareFirstThing(P0277_i_MapX, P0278_i_MapY));
}

WEAPON_INFO* F0158_DUNGEON_GetWeaponInfo(
THING P0279_T_Thing FINAL_SEPARATOR
{
        WEAPON* L0258_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(P0279_T_Thing);


        return &G0238_as_Graphic559_WeaponInfo[L0258_ps_Weapon->Type];
}

THING F0159_DUNGEON_GetNextThing(
THING P0280_T_Thing FINAL_SEPARATOR
{
        GENERIC* L0259_ps_Generic;


        L0259_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0280_T_Thing);
        return L0259_ps_Generic->Next;
}

STATICFUNCTION int16_t F0160_DUNGEON_GetSquareFirstThingIndex(
REGISTER int16_t P0281_i_MapX SEPARATOR
REGISTER int16_t P0282_i_MapY FINAL_SEPARATOR
{
        REGISTER unsigned char* L0262_puc_Square;
        REGISTER unsigned int16_t L0260_ui_ThingIndex;
        REGISTER int16_t L0261_i_MapY;


        L0262_puc_Square = G0271_ppuc_CurrentMapData[P0281_i_MapX];
        if (!((P0281_i_MapX >= 0) && (P0281_i_MapX < G0273_i_CurrentMapWidth)) || !((P0282_i_MapY >= 0) && (P0282_i_MapY < G0274_i_CurrentMapHeight)) || !M007_GET(L0262_puc_Square[P0282_i_MapY], MASK0x0010_THING_LIST_PRESENT)) {
                return -1;
        }
        L0261_i_MapY = 0;
        L0260_ui_ThingIndex = G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount[P0281_i_MapX];
        while (L0261_i_MapY++ != P0282_i_MapY) {
                if (M007_GET(*L0262_puc_Square++, MASK0x0010_THING_LIST_PRESENT)) {
                        L0260_ui_ThingIndex++;
                }
        }
        return L0260_ui_ThingIndex;
}

THING F0161_DUNGEON_GetSquareFirstThing(
int16_t P0283_i_MapX SEPARATOR
int16_t P0284_i_MapY FINAL_SEPARATOR
{
        int16_t L0263_i_SquareFirstThingIndex;


        if ((L0263_i_SquareFirstThingIndex = F0160_DUNGEON_GetSquareFirstThingIndex(P0283_i_MapX, P0284_i_MapY)) == -1) {
                return C0xFFFE_THING_ENDOFLIST;
        }
        else {
                return G0283_pT_SquareFirstThings[L0263_i_SquareFirstThingIndex];
        }
}

THING F0162_DUNGEON_GetSquareFirstObject(
int16_t P0285_i_MapX SEPARATOR
int16_t P0286_i_MapY FINAL_SEPARATOR
{
        REGISTER THING L0264_T_Thing;


        L0264_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0285_i_MapX, P0286_i_MapY);
        while ((L0264_T_Thing != C0xFFFE_THING_ENDOFLIST) && (M012_TYPE(L0264_T_Thing) < C04_THING_TYPE_GROUP)) {
                L0264_T_Thing = F0159_DUNGEON_GetNextThing(L0264_T_Thing);
        }
        return L0264_T_Thing;
}

void F0163_DUNGEON_LinkThingToList(
THING            P0287_T_ThingToLink SEPARATOR /* This thing is linked at the end of the thing list and is made the last thing (It is not possible to link a list of things to another list) */
THING            P0288_T_ThingInList SEPARATOR /* Any thing currently in the list. The function searches for the last thing in the list before linking the new thing */
REGISTER int16_t P0289_i_MapX        SEPARATOR
REGISTER int16_t P0290_i_MapY        FINAL_SEPARATOR
{
        REGISTER THING* L0267_pT_Thing;
        REGISTER unsigned char* L0268_puc_Square;
        REGISTER GENERIC* L0269_ps_Generic;
        REGISTER THING L0265_T_Thing;
        REGISTER unsigned int16_t L0266_ui_Multiple;
#define AL0266_ui_Column                L0266_ui_Multiple
#define AL0266_ui_SquareFirstThingIndex L0266_ui_Multiple
        REGISTER unsigned int16_t L0270_ui_MapY;


        if (P0287_T_ThingToLink == C0xFFFE_THING_ENDOFLIST)
                return;
        L0269_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0287_T_ThingToLink);
        L0269_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;
        /* If P0289_i_MapX >= 0 then the thing is linked to the list of things on the specified square else it is linked at the end of the specified thing list */
        if (P0289_i_MapX >= 0) {
                L0268_puc_Square = &G0271_ppuc_CurrentMapData[P0289_i_MapX][P0290_i_MapY];
                if (M007_GET(*L0268_puc_Square, MASK0x0010_THING_LIST_PRESENT)) {
                        P0288_T_ThingInList = F0161_DUNGEON_GetSquareFirstThing(P0289_i_MapX, P0290_i_MapY);
                } else {
                        if (G0283_pT_SquareFirstThings[G0278_ps_DungeonHeader->SquareFirstThingCount - 1] != C0xFFFF_THING_NONE) {
                                F0019_MAIN_DisplayErrorAndStop(C72_ERROR_INVALID_SQUARE_THING_LIST);
                        }
                        M008_SET(*L0268_puc_Square, MASK0x0010_THING_LIST_PRESENT);
                        L0267_pT_Thing = G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount + P0289_i_MapX + 1;
                        AL0266_ui_Column = G0282_ui_DungeonColumnCount - (G0281_pui_DungeonMapsFirstColumnIndex[G0272_i_CurrentMapIndex] + P0289_i_MapX) - 1;
                        while (AL0266_ui_Column--) { /* For each column starting from and after the column containing the square where the thing is added */
                                ++(*L0267_pT_Thing++); /* Increment the cumulative first thing count */
                        }
                        L0270_ui_MapY = 0;
                        L0268_puc_Square -= P0290_i_MapY;
                        AL0266_ui_SquareFirstThingIndex = G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount[P0289_i_MapX];
                        while (L0270_ui_MapY++ != P0290_i_MapY) {
                                if (M007_GET(*L0268_puc_Square++, MASK0x0010_THING_LIST_PRESENT)) {
                                        AL0266_ui_SquareFirstThingIndex++;
                                }
                        }
                        L0267_pT_Thing = &G0283_pT_SquareFirstThings[AL0266_ui_SquareFirstThingIndex]; /* BUG0_08 Objects disappear in the dungeon and data is corrupted in memory. The game manages an array of limited size containing the first thing on each square where there is at least one thing. There is no check to stop adding things to this array when it is full and this causes data corruptions in memory. With the original Dungeon Master and Chaos Strikes back dungeons the array is large enough so that most players will not get into trouble, however it is still possible to fill it by placing one thing on as many squares as possible in the dungeon. The number of free slots to store first square things is 676 in DM for Atari ST 1.0a EN, 678 in DM for Atari ST 1.0b EN, 679 in DM for Atari ST 1.1 EN, 1.2 EN, 1.2 GE, 1.3a FR and 1.3b FR, 690 in CSB for Atari ST 2.0 EN and 2.1 EN (each dungeon contains a number of free slots and the game adds 300 more when loading the dungeon).
                        If all THING entries in G0283_pT_SquareFirstThings are already used then inserting an additional one will overwrite the word of data in memory that follows the buffer allocated for G0283_pT_SquareFirstThings. If inserting a THING at the end of a full G0283_pT_SquareFirstThings then AL0266_ui_SquareFirstThingIndex is out of bounds and the next call will corrupt memory because the specified size will be negative (= large high positive value) */
                        F0007_MAIN_CopyBytes(M772_CAST_PC(L0267_pT_Thing), M772_CAST_PC(L0267_pT_Thing + 1), M543_BYTE_COUNT_INT((G0278_ps_DungeonHeader->SquareFirstThingCount - AL0266_ui_SquareFirstThingIndex - 1) << 1));
                        *L0267_pT_Thing = P0287_T_ThingToLink;
                        return;
                }
        }
        L0265_T_Thing = F0159_DUNGEON_GetNextThing(P0288_T_ThingInList);
        while (L0265_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                L0265_T_Thing = F0159_DUNGEON_GetNextThing(P0288_T_ThingInList = L0265_T_Thing);
        }
        L0269_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0288_T_ThingInList);
        L0269_ps_Generic->Next = P0287_T_ThingToLink;
}

void F0164_DUNGEON_UnlinkThingFromList(
REGISTER THING   P0291_T_ThingToUnlink SEPARATOR
REGISTER THING   P0292_T_ThingInList   SEPARATOR
REGISTER int16_t P0293_i_MapX          SEPARATOR
int16_t          P0294_i_MapY          FINAL_SEPARATOR
{
        REGISTER THING* L0275_pui_Multiple;
#define AL0275_pT_Thing                      L0275_pui_Multiple
#define AL0275_pui_CumulativeFirstThingCount L0275_pui_Multiple
        REGISTER GENERIC* L0274_ps_Generic;
        REGISTER THING L0273_T_Thing;
        REGISTER unsigned int16_t L0272_ui_Multiple;
#define AL0272_ui_SquareFirstThingIndex L0272_ui_Multiple
#define AL0272_ui_Column                L0272_ui_Multiple
        REGISTER unsigned int16_t L0271_ui_SquareFirstThingIndex;


        if (P0291_T_ThingToUnlink == C0xFFFE_THING_ENDOFLIST) {
                return;
        }
        M009_CLEAR(P0291_T_ThingToUnlink, MASK0xC000_THING_CELL);
        if (P0293_i_MapX >= 0) {
                L0274_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0291_T_ThingToUnlink);
                AL0275_pT_Thing = &G0283_pT_SquareFirstThings[L0271_ui_SquareFirstThingIndex = F0160_DUNGEON_GetSquareFirstThingIndex(P0293_i_MapX, P0294_i_MapY)]; /* BUG0_01 Coding error without consequence. The engine does not check that there are things at the specified square coordinates. F0160_DUNGEON_GetSquareFirstThingIndex would return -1 for an empty square. No consequence as the function is never called with the coordinates of an empty square (except in the case of BUG0_59) */
                if ((L0274_ps_Generic->Next == C0xFFFE_THING_ENDOFLIST) && (M014_TYPE_AND_INDEX(*AL0275_pT_Thing) == P0291_T_ThingToUnlink)) { /* If the thing to unlink is the last thing on the square */
                        M009_CLEAR(G0271_ppuc_CurrentMapData[P0293_i_MapX][P0294_i_MapY], MASK0x0010_THING_LIST_PRESENT);
                        F0007_MAIN_CopyBytes(M772_CAST_PC(AL0275_pT_Thing + 1), M772_CAST_PC(AL0275_pT_Thing), M543_BYTE_COUNT_INT(((AL0272_ui_SquareFirstThingIndex = G0278_ps_DungeonHeader->SquareFirstThingCount - 1) - L0271_ui_SquareFirstThingIndex) << 1));
                        G0283_pT_SquareFirstThings[AL0272_ui_SquareFirstThingIndex] = C0xFFFF_THING_NONE;
                        AL0275_pui_CumulativeFirstThingCount = G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount + P0293_i_MapX + 1;
                        AL0272_ui_Column = G0282_ui_DungeonColumnCount - (G0281_pui_DungeonMapsFirstColumnIndex[G0272_i_CurrentMapIndex] + P0293_i_MapX) - 1;
                        while (AL0272_ui_Column--) { /* For each column starting from and after the column containing the square where the thing is unlinked */
                                --(*AL0275_pui_CumulativeFirstThingCount++); /* Decrement the cumulative first thing count */
                        }
                        goto T0164011;
                }
                else {
                        if (M014_TYPE_AND_INDEX(*AL0275_pT_Thing) == P0291_T_ThingToUnlink) {
                                *AL0275_pT_Thing = L0274_ps_Generic->Next;
                                goto T0164011;
                        }
                }
                P0292_T_ThingInList = *AL0275_pT_Thing;
        }
        L0273_T_Thing = F0159_DUNGEON_GetNextThing(P0292_T_ThingInList);
        while (M014_TYPE_AND_INDEX(L0273_T_Thing) != P0291_T_ThingToUnlink) {
                if ((L0273_T_Thing == C0xFFFE_THING_ENDOFLIST) || (L0273_T_Thing == C0xFFFF_THING_NONE)) {
                        goto T0164011;
                }
                L0273_T_Thing = F0159_DUNGEON_GetNextThing(P0292_T_ThingInList = L0273_T_Thing);
        }
        L0274_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0292_T_ThingInList);
        L0274_ps_Generic->Next = F0159_DUNGEON_GetNextThing(L0273_T_Thing);
        L0274_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(P0291_T_ThingToUnlink);
        T0164011:
        L0274_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;
}

STATICFUNCTION THING F0165_DUNGEON_GetDiscardedThing(
REGISTER unsigned int16_t P0295_ui_ThingType FINAL_SEPARATOR
{
        REGISTER unsigned char* L0280_puc_Square;
        REGISTER GENERIC* L0282_ps_Generic;
        REGISTER THING* L0281_pT_SquareFirstThing;
        REGISTER unsigned int16_t L0279_ui_MapIndex;
        REGISTER THING L0278_T_Thing;
        REGISTER unsigned int16_t L0276_ui_MapX;
        REGISTER unsigned int16_t L0277_ui_MapY;
        unsigned int16_t L0285_ui_MapWidth;
        unsigned int16_t L0286_ui_MapHeight;
        unsigned int16_t L0283_ui_DiscardThingMapIndex;
        int16_t L0284_i_CurrentMapIndex;
        unsigned int16_t L0287_ui_ThingType;
        static unsigned char G0294_auc_LastDiscardedThingMapIndex[16];


        if (P0295_ui_ThingType == C15_THING_TYPE_EXPLOSION) {
                return C0xFFFF_THING_NONE;
        }
        L0284_i_CurrentMapIndex = G0272_i_CurrentMapIndex;
        if (((L0279_ui_MapIndex = G0294_auc_LastDiscardedThingMapIndex[P0295_ui_ThingType]) == G0309_i_PartyMapIndex) && (++L0279_ui_MapIndex >= G0278_ps_DungeonHeader->MapCount)) {
                L0279_ui_MapIndex = 0;
        }
        L0283_ui_DiscardThingMapIndex = L0279_ui_MapIndex;
        for (;;) { /*_Infinite loop_*/
                L0285_ui_MapWidth = G0277_ps_DungeonMaps[L0279_ui_MapIndex].A.Width;
                L0286_ui_MapHeight = G0277_ps_DungeonMaps[L0279_ui_MapIndex].A.Height;
                L0280_puc_Square = G0279_pppuc_DungeonMapData[L0279_ui_MapIndex][0];
                L0281_pT_SquareFirstThing = &G0283_pT_SquareFirstThings[G0280_pui_DungeonColumnsCumulativeSquareThingCount[G0281_pui_DungeonMapsFirstColumnIndex[L0279_ui_MapIndex]]];
                for (L0276_ui_MapX = 0; L0276_ui_MapX <= L0285_ui_MapWidth; L0276_ui_MapX++) {
                        for (L0277_ui_MapY = 0; L0277_ui_MapY <= L0286_ui_MapHeight; L0277_ui_MapY++) {
                                if (M007_GET(*L0280_puc_Square++, MASK0x0010_THING_LIST_PRESENT)) {
                                        L0278_T_Thing = *L0281_pT_SquareFirstThing++;
                                        if ((L0279_ui_MapIndex == G0309_i_PartyMapIndex) && ((L0276_ui_MapX - G0306_i_PartyMapX + 5) <= 10) && ((L0277_ui_MapY - G0307_i_PartyMapY + 5) <= 10)) /* If square is too close to the party */
                                                goto T0165029;
                                        do {
                                                if ((L0287_ui_ThingType = M012_TYPE(L0278_T_Thing)) == C03_THING_TYPE_SENSOR) {
                                                        L0282_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0278_T_Thing);
                                                        if (M039_TYPE((SENSOR*)L0282_ps_Generic)) /* If sensor is not disabled */
                                                                break;
                                                } else {
                                                        if (L0287_ui_ThingType == P0295_ui_ThingType) {
                                                                L0282_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0278_T_Thing);
                                                                switch (P0295_ui_ThingType) {
                                                                        case C04_THING_TYPE_GROUP:
                                                                                if ((((GROUP*)L0282_ps_Generic)->DoNotDiscard)
                                                                                   || (((GROUP*)L0282_ps_Generic)->Slot != C0xFFFE_THING_ENDOFLIST)
                                                                                   )
                                                                                        continue;
                                                                        case C14_THING_TYPE_PROJECTILE:
                                                                                F0173_DUNGEON_SetCurrentMap(L0279_ui_MapIndex);
                                                                                if (P0295_ui_ThingType == C04_THING_TYPE_GROUP) {
                                                                                        F0188_GROUP_DropGroupPossessions(L0276_ui_MapX, L0277_ui_MapY, L0278_T_Thing, CM1_MODE_DO_NOT_PLAY_SOUND);
                                                                                        F0189_GROUP_Delete(L0276_ui_MapX, L0277_ui_MapY);
                                                                                } else {
                                                                                        F0214_PROJECTILE_DeleteEvent(L0278_T_Thing);
                                                                                        F0164_DUNGEON_UnlinkThingFromList(L0278_T_Thing, 0, L0276_ui_MapX, L0277_ui_MapY);
#ifdef PC_FIX_CODE_SIZE
L0278_T_Thing++;
L0278_T_Thing++;
L0278_T_Thing++;
#endif
                                                                                        F0215_PROJECTILE_Delete(L0278_T_Thing, NULL, L0276_ui_MapX, L0277_ui_MapY); /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE opt */
                                                                                }
                                                                                break;
                                                                        case C06_THING_TYPE_ARMOUR:
                                                                                if (((ARMOUR*)L0282_ps_Generic)->DoNotDiscard)
                                                                                        continue;
                                                                                goto T0165026;
                                                                        case C05_THING_TYPE_WEAPON:
                                                                                if (((WEAPON*)L0282_ps_Generic)->DoNotDiscard)
                                                                                        continue;
                                                                                goto T0165026;
                                                                        case C10_THING_TYPE_JUNK:
                                                                                if (((JUNK*)L0282_ps_Generic)->DoNotDiscard)
                                                                                        continue;
                                                                                goto T0165026;
                                                                        case C08_THING_TYPE_POTION:
                                                                                if (((POTION*)L0282_ps_Generic)->DoNotDiscard)
                                                                                        continue;
                                                                                T0165026:
                                                                                F0173_DUNGEON_SetCurrentMap(L0279_ui_MapIndex);
                                                                                F0267_MOVE_GetMoveResult_CPSCE(L0278_T_Thing, L0276_ui_MapX, L0277_ui_MapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);
                                                                }
                                                                F0173_DUNGEON_SetCurrentMap(L0284_i_CurrentMapIndex);
                                                                G0294_auc_LastDiscardedThingMapIndex[P0295_ui_ThingType] = L0279_ui_MapIndex;
                                                                return M014_TYPE_AND_INDEX(L0278_T_Thing);
                                                        }
                                                }
                                        } while ((L0278_T_Thing = F0159_DUNGEON_GetNextThing(L0278_T_Thing)) != C0xFFFE_THING_ENDOFLIST);
                                        T0165029: ;
                                }
                        }
                }
                if ((L0279_ui_MapIndex == G0309_i_PartyMapIndex) || (G0278_ps_DungeonHeader->MapCount <= 1)) {
                        G0294_auc_LastDiscardedThingMapIndex[P0295_ui_ThingType] = L0279_ui_MapIndex;
                        return C0xFFFF_THING_NONE;
                }
                do {
                        if (++L0279_ui_MapIndex >= G0278_ps_DungeonHeader->MapCount) {
                                L0279_ui_MapIndex = 0;
                        }
                } while (L0279_ui_MapIndex == G0309_i_PartyMapIndex);
                if (L0279_ui_MapIndex == L0283_ui_DiscardThingMapIndex) {
                        L0279_ui_MapIndex = G0309_i_PartyMapIndex;
                }
        }
}

THING F0166_DUNGEON_GetUnusedThing(
REGISTER unsigned int16_t P0296_ui_ThingType FINAL_SEPARATOR
{
        REGISTER GENERIC* L0291_ps_Generic;
        REGISTER int16_t L0288_i_ThingIndex;
        REGISTER int16_t L0290_i_ThingCount;
        REGISTER int16_t L0289_i_ThingDataByteCount;
        REGISTER THING L0292_T_Thing;


        L0290_i_ThingCount = G0278_ps_DungeonHeader->ThingCount[M007_GET(P0296_ui_ThingType, MASK0x7FFF_THING_TYPE)];
        if (P0296_ui_ThingType == (MASK0x8000_CHAMPION_BONES | C10_THING_TYPE_JUNK)) {
                P0296_ui_ThingType = C10_THING_TYPE_JUNK;
        } else {
                if (P0296_ui_ThingType == C10_THING_TYPE_JUNK) {
                        L0290_i_ThingCount -= 3; /* Always keep 3 unused JUNK things for the bones of dead champions */
                }
        }
        L0288_i_ThingIndex = L0290_i_ThingCount;
        L0289_i_ThingDataByteCount = G0235_auc_Graphic559_ThingDataByteCount[P0296_ui_ThingType] >> 1;
        L0291_ps_Generic = (GENERIC*)G0284_apuc_ThingData[P0296_ui_ThingType];
        for (;;) { /*_Infinite loop_*/
                if (L0291_ps_Generic->Next == C0xFFFF_THING_NONE) { /* If thing data is unused */
                        L0292_T_Thing = (P0296_ui_ThingType << 10) | (L0290_i_ThingCount - L0288_i_ThingIndex);
                        break;
                }
                else {
                        if (--L0288_i_ThingIndex) { /* If there are thing data left to process */
                                L0291_ps_Generic += L0289_i_ThingDataByteCount; /* Proceed to the next thing data */
                        } else {
                                if ((L0292_T_Thing = F0165_DUNGEON_GetDiscardedThing(P0296_ui_ThingType)) == C0xFFFF_THING_NONE) {
                                        return C0xFFFF_THING_NONE;
                                }
                                L0291_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0292_T_Thing);
                                break;
                        }
                }
        }
        F0008_MAIN_ClearBytes(M772_CAST_PC(L0291_ps_Generic), M543_BYTE_COUNT_INT(L0289_i_ThingDataByteCount << 1));
        L0291_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;
        return L0292_T_Thing;
}

THING F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
REGISTER unsigned int16_t P0297_ui_IconIndex FINAL_SEPARATOR
{
        REGISTER JUNK* L0296_ps_Junk;
        REGISTER unsigned int16_t L0293_ui_Type;
        REGISTER THING L0295_T_Thing;
        REGISTER int16_t L0294_i_ThingType;


        L0294_i_ThingType = C05_THING_TYPE_WEAPON;
        if ((P0297_ui_IconIndex >= C004_ICON_WEAPON_TORCH_UNLIT) && (P0297_ui_IconIndex <= C007_ICON_WEAPON_TORCH_LIT)) {
                P0297_ui_IconIndex = C004_ICON_WEAPON_TORCH_UNLIT;
        }
        switch (P0297_ui_IconIndex) {
                case C054_ICON_WEAPON_ROCK:
                        L0293_ui_Type = C30_WEAPON_ROCK;
                        break;
                case C128_ICON_JUNK_BOULDER:
                        L0293_ui_Type = C25_JUNK_BOULDER;
                        L0294_i_ThingType = C10_THING_TYPE_JUNK;
                        break;
                case C051_ICON_WEAPON_ARROW:
                        L0293_ui_Type = C27_WEAPON_ARROW;
                        break;
                case C052_ICON_WEAPON_SLAYER:
                        L0293_ui_Type = C28_WEAPON_SLAYER;
                        break;
                case C055_ICON_WEAPON_POISON_DART:
                        L0293_ui_Type = C31_WEAPON_POISON_DART;
                        break;
                case C056_ICON_WEAPON_THROWING_STAR:
                        L0293_ui_Type = C32_WEAPON_THROWING_STAR;
                        break;
                case C032_ICON_WEAPON_DAGGER:
                        L0293_ui_Type = C08_WEAPON_DAGGER;
                        break;
                case C004_ICON_WEAPON_TORCH_UNLIT:
                        L0293_ui_Type = C02_WEAPON_TORCH;
                        break;
                default:
                        return C0xFFFF_THING_NONE;
        }
        if ((L0295_T_Thing = F0166_DUNGEON_GetUnusedThing(L0294_i_ThingType)) == C0xFFFF_THING_NONE) {
                return C0xFFFF_THING_NONE;
        }
        L0296_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L0295_T_Thing);
        L0296_ps_Junk->Type = L0293_ui_Type; /* Also works for WEAPON in cases other than Boulder */
        if ((P0297_ui_IconIndex == C004_ICON_WEAPON_TORCH_UNLIT) && ((WEAPON*)L0296_ps_Junk)->Lit) { /* BUG0_65 Torches created by object generator or projectile launcher sensors have no charges. Charges are only defined if the Torch is lit which is not possible at the time it is created */
                ((WEAPON*)L0296_ps_Junk)->ChargeCount = 15;
        }
        return L0295_T_Thing;
}

void F0168_DUNGEON_DecodeText(
REGISTER unsigned char* P0298_pc_String  SEPARATOR /* There is no check to see if the decoded text fits in this string buffer */
THING                   P0299_T_Thing    SEPARATOR
REGISTER int16_t        P0300_i_TextType FINAL_SEPARATOR
{
        REGISTER char* L0303_pc_EscapeReplacementString;
        REGISTER unsigned int16_t* L0300_pui_CodeWord;
        REGISTER unsigned int16_t L0299_ui_Code;
        REGISTER unsigned int16_t L0298_ui_CodeCounter;
        REGISTER int16_t L0302_i_EscapeCharacter;
        REGISTER unsigned int16_t L0297_ui_Codes;
        char L0301_c_SeparatorCharacter;
        TEXTSTRING L0304_s_TextString;
        L0304_s_TextString = ((TEXTSTRING*)G0284_apuc_ThingData[C02_THING_TYPE_TEXTSTRING])[M013_INDEX(P0299_T_Thing)];
        if ((L0304_s_TextString.Visible) || M007_GET(P0300_i_TextType, MASK0x8000_DECODE_EVEN_IF_INVISIBLE)) {
                M009_CLEAR(P0300_i_TextType, MASK0x8000_DECODE_EVEN_IF_INVISIBLE);
                if (P0300_i_TextType == C1_TEXT_TYPE_MESSAGE) {
                        *P0298_pc_String++ = '\n'; /* New line */
                        L0301_c_SeparatorCharacter = ' ';
                } else {
                        if (P0300_i_TextType == C0_TEXT_TYPE_INSCRIPTION) {
                                L0301_c_SeparatorCharacter = '\200'; /* Octal. Hexadecimal: 0x80 (Megamax C does not support hexadecimal character constants) */
                        } else {
                                L0301_c_SeparatorCharacter = '\n'; /* New line */
                        }
                }
                L0298_ui_CodeCounter = 0;
                L0302_i_EscapeCharacter = 0;
                L0300_pui_CodeWord = G0260_pui_DungeonTextData + L0304_s_TextString.TextDataWordOffset;
                for (;;) { /*_Infinite loop_*/
                        if (!L0298_ui_CodeCounter) {
                                L0297_ui_Codes = *L0300_pui_CodeWord++;
                                L0299_ui_Code = (L0297_ui_Codes >> 10) & 0x001F;
                        } else {
                                if (L0298_ui_CodeCounter == 1) {
                                        L0299_ui_Code = (L0297_ui_Codes >> 5) & 0x001F;
                                } else {
                                        L0299_ui_Code = L0297_ui_Codes & 0x001F;
                                }
                        }
                        L0298_ui_CodeCounter = ++L0298_ui_CodeCounter % 3;
                        if (L0302_i_EscapeCharacter) {
                                *P0298_pc_String = '\0';
                                if (L0302_i_EscapeCharacter == 30) {
                                        if (P0300_i_TextType != C0_TEXT_TYPE_INSCRIPTION) {
                                                L0303_pc_EscapeReplacementString = G0255_aac_Graphic559_MessageAndScrollEscapeReplacementStrings[L0299_ui_Code];
                                        } else {
#ifdef PC_FIX_CODE_SIZE
        P0300_i_TextType++;
        P0300_i_TextType++;
        P0300_i_TextType++;
        P0300_i_TextType++;
        P0300_i_TextType++;
        P0300_i_TextType++;
#endif
                                                L0303_pc_EscapeReplacementString = G0257_aac_Graphic559_InscriptionEscapeReplacementStrings[L0299_ui_Code]; /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE opt */
                                        }
                                } else {
                                        if (P0300_i_TextType != C0_TEXT_TYPE_INSCRIPTION) { /* BUG0_00 Useless code. The same statement is executed in both cases */
                                                L0303_pc_EscapeReplacementString = G0256_aac_Graphic559_EscapeReplacementCharacters[L0299_ui_Code];
                                        } else {
                                                L0303_pc_EscapeReplacementString = G0256_aac_Graphic559_EscapeReplacementCharacters[L0299_ui_Code];
                                        }
                                }
                                M545_STRCAT(M772_CAST_PC(P0298_pc_String), L0303_pc_EscapeReplacementString);
                                P0298_pc_String += M544_STRLEN(L0303_pc_EscapeReplacementString);
                                L0302_i_EscapeCharacter = 0;
                        } else {
                                if (L0299_ui_Code < 28) {
                                        if (P0300_i_TextType != C0_TEXT_TYPE_INSCRIPTION) {
                                                if (L0299_ui_Code == 26) {
                                                        L0299_ui_Code = ' ';
                                                } else {
                                                        if (L0299_ui_Code == 27) {
                                                                L0299_ui_Code = '.';
                                                        } else {
                                                                L0299_ui_Code += 'A';
                                                        }
                                                }
                                        }
                                        *P0298_pc_String++ = L0299_ui_Code;
                                } else {
                                        if (L0299_ui_Code == 28) {
                                                *P0298_pc_String++ = L0301_c_SeparatorCharacter;
                                        } else {
                                                if (L0299_ui_Code <= 30) { /* Codes 29 and 30 are escape characters */
                                                        L0302_i_EscapeCharacter = L0299_ui_Code;
                                                } else { /* Code 31 marks the end of the text */
                                                        break;
                                                }
                                        }
                                }
                        }
                }
        }
        if (P0300_i_TextType == C0_TEXT_TYPE_INSCRIPTION) {
                *P0298_pc_String = '\201'; /* Octal. Hexadecimal: 0x81 (Megamax C does not support hexadecimal character constants) */
        } else {
                *P0298_pc_String = '\0';
        }
}

STATICFUNCTION int16_t F0169_DUNGEON_GetRandomOrnamentIndex(
unsigned int16_t P0301_ui_Value1 SEPARATOR
unsigned int16_t P0302_ui_Value2 SEPARATOR
unsigned int16_t P0303_i_Modulo  FINAL_SEPARATOR
{
        return ((((P0301_ui_Value1 * 31417) >> 1) + (P0302_ui_Value2 * 11) + G0278_ps_DungeonHeader->OrnamentRandomSeed) >> 2) % P0303_i_Modulo; /* Pseudorandom number generator */
}

STATICFUNCTION int16_t F0170_DUNGEON_GetRandomOrnamentOrdinal(
BOOLEAN P0304_B_RandomOrnamentAllowed SEPARATOR
int16_t P0305_i_OrnamentCount         SEPARATOR
int16_t P0306_i_MapX                  SEPARATOR
int16_t P0307_i_MapY                  SEPARATOR
int16_t P0308_i_Modulo                FINAL_SEPARATOR
{
        int16_t L0305_i_RandomOrnamentIndex;


        if (P0304_B_RandomOrnamentAllowed && ((L0305_i_RandomOrnamentIndex = F0169_DUNGEON_GetRandomOrnamentIndex(2000 + (P0306_i_MapX << 5) + P0307_i_MapY, 3000 + (G0272_i_CurrentMapIndex << 6) + G0273_i_CurrentMapWidth + G0274_i_CurrentMapHeight, P0308_i_Modulo)) < P0305_i_OrnamentCount)) {
                return M000_INDEX_TO_ORDINAL(L0305_i_RandomOrnamentIndex);
        }
        else {
                return 0;
        }
}

STATICFUNCTION void F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
REGISTER int16_t* P0309_pi_SquareAspect                  SEPARATOR
BOOLEAN           P2003_B_BackRandomWallOrnamentAllowed  SEPARATOR
BOOLEAN           P0310_B_LeftRandomWallOrnamentAllowed  SEPARATOR
BOOLEAN           P0311_B_FrontRandomWallOrnamentAllowed SEPARATOR
BOOLEAN           P0312_B_RightRandomWallOrnamentAllowed SEPARATOR
REGISTER int16_t  P0313_i_Direction                      SEPARATOR
REGISTER int16_t  P0314_i_MapX                           SEPARATOR
REGISTER int16_t  P0315_i_MapY                           FINAL_SEPARATOR
{
        REGISTER int16_t L0306_i_Multiple;
#define AL0306_i_RandomWallOrnamentCount L0306_i_Multiple
#define AL0306_i_SideIndex               L0306_i_Multiple


        AL0306_i_RandomWallOrnamentCount = G0269_ps_CurrentMap->B.RandomWallOrnamentCount;
        P0309_pi_SquareAspect[C3_BACK_WALL_ORNAMENT_ORDINAL] = F0170_DUNGEON_GetRandomOrnamentOrdinal(P2003_B_BackRandomWallOrnamentAllowed, AL0306_i_RandomWallOrnamentCount, P0314_i_MapX, ++P0315_i_MapY * (M021_NORMALIZE(P0313_i_Direction) + 1), 30);
        P0309_pi_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL] = F0170_DUNGEON_GetRandomOrnamentOrdinal(P0310_B_LeftRandomWallOrnamentAllowed, AL0306_i_RandomWallOrnamentCount, P0314_i_MapX, P0315_i_MapY * (M021_NORMALIZE(++P0313_i_Direction) + 1), 30);
        P0309_pi_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL] = F0170_DUNGEON_GetRandomOrnamentOrdinal(P0311_B_FrontRandomWallOrnamentAllowed, AL0306_i_RandomWallOrnamentCount, P0314_i_MapX, P0315_i_MapY * (M021_NORMALIZE(++P0313_i_Direction) + 1), 30);
        P0309_pi_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL] = F0170_DUNGEON_GetRandomOrnamentOrdinal(P0312_B_RightRandomWallOrnamentAllowed, AL0306_i_RandomWallOrnamentCount, P0314_i_MapX, P0315_i_MapY-- * (M021_NORMALIZE(++P0313_i_Direction) + 1), 30);
        if (
           (P0314_i_MapX < 0) || (P0314_i_MapX >= G0273_i_CurrentMapWidth) || (P0315_i_MapY < 0) || (P0315_i_MapY >= G0274_i_CurrentMapHeight)) { /* If square is a fake wall or is out of map bounds */
                for (AL0306_i_SideIndex = C3_BACK_WALL_ORNAMENT_ORDINAL; AL0306_i_SideIndex <= M553_LEFT_WALL_ORNAMENT_ORDINAL; AL0306_i_SideIndex++) { /* Loop to remove any random ornament that is an alcove */
                        if (F0149_DUNGEON_IsWallOrnamentAnAlcove(M001_ORDINAL_TO_INDEX(P0309_pi_SquareAspect[AL0306_i_SideIndex]))) {
                                P0309_pi_SquareAspect[AL0306_i_SideIndex] = 0;
                        }
                }
        }
}

void F0172_DUNGEON_SetSquareAspect(
REGISTER int16_t*          P0317_pui_SquareAspect SEPARATOR
REGISTER int16_t           P0318_i_Direction      SEPARATOR
REGISTER int16_t           P0319_i_MapX           SEPARATOR
int16_t                    P0320_i_MapY           FINAL_SEPARATOR
#define AP0318_i_ThingType P0318_i_Direction
{
        REGISTER SENSOR* L0308_ps_Sensor;
        REGISTER unsigned char L0307_uc_Multiple;
#define AL0307_uc_Square            L0307_uc_Multiple
#define AL0307_uc_FootprintsAllowed L0307_uc_Multiple
#define AL0307_uc_ScentOrdinal      L0307_uc_Multiple
        REGISTER THING L0314_T_Thing;
        REGISTER int16_t L0310_i_Multiple;
        BOOLEAN L0309_B_LeftRandomWallOrnamentAllowed;
#define AL0310_B_FrontRandomWallOrnamentAllowed L0310_i_Multiple
#define AL0310_i_SideIndex                      L0310_i_Multiple
        BOOLEAN L0311_B_RightRandomWallOrnamentAllowed;
        BOOLEAN L2423_B_BackRandomWallOrnamentAllowed; /* Added to get aspect of back wall for the magic map in CSB 3.x */
        BOOLEAN L0313_B_SquareIsFakeWall;
        int16_t L0312_i_ThingType;


        F0008_MAIN_ClearBytes(M772_CAST_PC(P0317_pui_SquareAspect), M543_BYTE_COUNT_INT(M559_SQUARE_ASPECT_INT_COUNT * sizeof(int16_t)));
        L0314_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0319_i_MapX, P0320_i_MapY);
        P0317_pui_SquareAspect[C1_SQUARE] =
        AL0307_uc_Square = F0151_DUNGEON_GetSquare(P0319_i_MapX, P0320_i_MapY);
        switch (P0317_pui_SquareAspect[C0_ELEMENT] = M034_SQUARE_TYPE(AL0307_uc_Square)) {
                case C00_ELEMENT_WALL:
                        switch (P0318_i_Direction) {
                                case C0_DIRECTION_NORTH:
                                        L2423_B_BackRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED);
                                        L0309_B_LeftRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);
                                        AL0310_B_FrontRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED);
                                        L0311_B_RightRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED);
                                        break;
                                case C1_DIRECTION_EAST:
                                        L2423_B_BackRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);
                                        L0309_B_LeftRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED);
                                        AL0310_B_FrontRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED);
                                        L0311_B_RightRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED);
                                        break;
                                case C2_DIRECTION_SOUTH:
                                        L2423_B_BackRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED);
                                        L0309_B_LeftRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED);
                                        AL0310_B_FrontRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED);
                                        L0311_B_RightRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);
                                        break;
                                case C3_DIRECTION_WEST:
                                        L2423_B_BackRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED);
                                        L0309_B_LeftRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED);
                                        AL0310_B_FrontRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED);
                                        L0311_B_RightRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED);
                        }
                        G0289_i_DungeonView_ChampionPortraitOrdinal = 0; /* BUG0_75 Multiple champion portraits are drawn (one at a time) then the game crashes. This variable is only reset to 0 when at least one square in the dungeon view is a wall. If the party is in front of a wall with a champion portrait and the next time the dungeon view is drawn there is no wall square in the view and the square in front of the party is a fake wall with a random ornament then the same champion portrait will be drawn again because the variable was not reset to 0. Each time F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF draws the portrait, G0289_i_DungeonView_ChampionPortraitOrdinal is decremented so that the portait is different each time the dungeon view is drawn until the game crashes */
                        L0313_B_SquareIsFakeWall = C0_FALSE;
                        F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(P0317_pui_SquareAspect, L2423_B_BackRandomWallOrnamentAllowed, L0309_B_LeftRandomWallOrnamentAllowed, AL0310_B_FrontRandomWallOrnamentAllowed, L0311_B_RightRandomWallOrnamentAllowed, P0318_i_Direction, P0319_i_MapX, P0320_i_MapY);
                        T0172010_ClosedFakeWall: /* On version 3.x, random decorations are not allowed on fake walls (F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals is not executed for fake walls). This avoids for example the fountain on the northern wall at (04,14,10) */
                        while ((L0314_T_Thing != C0xFFFE_THING_ENDOFLIST) && ((L0312_i_ThingType = M012_TYPE(L0314_T_Thing)) <= C03_THING_TYPE_SENSOR)) {
                                AL0310_i_SideIndex = M021_NORMALIZE(M011_CELL(L0314_T_Thing) - P0318_i_Direction) + 3;
                                        L0308_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0314_T_Thing);
                                        if (L0312_i_ThingType == C02_THING_TYPE_TEXTSTRING) {
                                                if (((TEXTSTRING*)L0308_ps_Sensor)->Visible) {
                                                        P0317_pui_SquareAspect[AL0310_i_SideIndex] = G0265_i_CurrentMapInscriptionWallOrnamentIndex + 1;
                                                        if (AL0310_i_SideIndex == M552_FRONT_WALL_ORNAMENT_ORDINAL) {
                                                                G0290_T_DungeonView_InscriptionThing = L0314_T_Thing; /* BUG0_76 The same text is drawn on multiple sides of a wall square. The engine stores only a single text to draw on a wall in a global variable. Even if different texts are placed on different sides of the wall, the same text is drawn on each affected side */
                                                        }
                                                }
                                        } else {
                                                P0317_pui_SquareAspect[AL0310_i_SideIndex] = L0308_ps_Sensor->Remote.OrnamentOrdinal;
                                                if (M039_TYPE(L0308_ps_Sensor) == C127_SENSOR_WALL_CHAMPION_PORTRAIT) {
                                                        if (AL0310_i_SideIndex == M552_FRONT_WALL_ORNAMENT_ORDINAL) {
                                                                G0289_i_DungeonView_ChampionPortraitOrdinal = M000_INDEX_TO_ORDINAL(M040_DATA(L0308_ps_Sensor));
                                                        }
                                                }
                                        }
                                L0314_T_Thing = F0159_DUNGEON_GetNextThing(L0314_T_Thing);
                        }
                        if (L0313_B_SquareIsFakeWall && (G0306_i_PartyMapX != P0319_i_MapX) && (G0307_i_PartyMapY != P0320_i_MapY)) {
                                P0317_pui_SquareAspect[M550_FIRST_THING] = C0xFFFE_THING_ENDOFLIST;
                                return;
                        }
                        break;
                case C02_ELEMENT_PIT:
                        if (M007_GET(AL0307_uc_Square, MASK0x0008_PIT_OPEN)) {
                                P0317_pui_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] = M007_GET(AL0307_uc_Square, MASK0x0004_PIT_INVISIBLE);
                                AL0307_uc_FootprintsAllowed &= MASK0x0001_PIT_IMAGINARY;
                        } else { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE opt */
#ifdef PC_FIX_CODE_SIZE
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
        L0314_T_Thing++;
#endif
                                P0317_pui_SquareAspect[C0_ELEMENT] = C01_ELEMENT_CORRIDOR;
                                AL0307_uc_FootprintsAllowed = C1_TRUE;
                        }
                        goto T0172030_Pit;
                case C06_ELEMENT_FAKEWALL:
                        if (!M007_GET(AL0307_uc_Square, MASK0x0004_FAKEWALL_OPEN)) {
                                P0317_pui_SquareAspect[C0_ELEMENT] = C00_ELEMENT_WALL;
                                L2423_B_BackRandomWallOrnamentAllowed = L0309_B_LeftRandomWallOrnamentAllowed = AL0310_B_FrontRandomWallOrnamentAllowed = L0311_B_RightRandomWallOrnamentAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_FAKEWALL_RANDOM_ORNAMENT_OR_FOOTPRINTS_ALLOWED);
                                L0313_B_SquareIsFakeWall = C1_TRUE;
                                goto T0172010_ClosedFakeWall;
                        }
                        P0317_pui_SquareAspect[C0_ELEMENT] = C01_ELEMENT_CORRIDOR;
                        AL0307_uc_FootprintsAllowed = M007_GET(AL0307_uc_Square, MASK0x0008_FAKEWALL_RANDOM_ORNAMENT_OR_FOOTPRINTS_ALLOWED) ? 8 : 0;
                case C01_ELEMENT_CORRIDOR:
                        P0317_pui_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL] = F0170_DUNGEON_GetRandomOrnamentOrdinal(M007_GET(AL0307_uc_Square, MASK0x0008_CORRIDOR_RANDOM_ORNAMENT_ALLOWED), G0269_ps_CurrentMap->B.RandomFloorOrnamentCount, P0319_i_MapX, P0320_i_MapY, 30);
                        T0172029_Teleporter:
                        AL0307_uc_FootprintsAllowed = C1_TRUE;
                        T0172030_Pit:
                        while ((L0314_T_Thing != C0xFFFE_THING_ENDOFLIST) && ((AP0318_i_ThingType = M012_TYPE(L0314_T_Thing)) <= C03_THING_TYPE_SENSOR)) {
                                if (AP0318_i_ThingType == C03_THING_TYPE_SENSOR) {
                                        L0308_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0314_T_Thing);
                                        P0317_pui_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL] = L0308_ps_Sensor->Remote.OrnamentOrdinal;
                                }
                                L0314_T_Thing = F0159_DUNGEON_GetNextThing(L0314_T_Thing);
                        }
                        goto T0172049_Footprints;
                case C05_ELEMENT_TELEPORTER:
                        if ((!M007_GET(AL0307_uc_Square, MASK0x0008_TELEPORTER_OPEN) || !M007_GET(AL0307_uc_Square, MASK0x0004_TELEPORTER_VISIBLE))) {
                                P0317_pui_SquareAspect[C0_ELEMENT] = C01_ELEMENT_CORRIDOR;
                        }
                        goto T0172029_Teleporter;
                case C03_ELEMENT_STAIRS:
                        P0317_pui_SquareAspect[C0_ELEMENT] = ((M007_GET(AL0307_uc_Square, MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION) >> 3) == M016_IS_ORIENTED_WEST_EAST(P0318_i_Direction)) ? C18_ELEMENT_STAIRS_SIDE : C19_ELEMENT_STAIRS_FRONT;
                        P0317_pui_SquareAspect[M555_STAIRS_UP] = M007_GET(AL0307_uc_Square, MASK0x0004_STAIRS_UP);
                        AL0307_uc_FootprintsAllowed = C0_FALSE;
                        goto T0172046_Stairs;
                case C04_ELEMENT_DOOR:
                        if ((M007_GET(AL0307_uc_Square, MASK0x0008_DOOR_NORTH_SOUTH_ORIENTATION) >> 3) == M016_IS_ORIENTED_WEST_EAST(P0318_i_Direction)) {
                                P0317_pui_SquareAspect[C0_ELEMENT] = C16_ELEMENT_DOOR_SIDE;
                        } else {
                                P0317_pui_SquareAspect[C0_ELEMENT] = C17_ELEMENT_DOOR_FRONT;
                        }
                                P0317_pui_SquareAspect[M556_DOOR_STATE] = M036_DOOR_STATE(AL0307_uc_Square);
                                P0317_pui_SquareAspect[M557_DOOR_THING_INDEX] = M013_INDEX(F0161_DUNGEON_GetSquareFirstThing(P0319_i_MapX, P0320_i_MapY));
                        AL0307_uc_FootprintsAllowed = C1_TRUE;
                        T0172046_Stairs:
                        while ((L0314_T_Thing != C0xFFFE_THING_ENDOFLIST) && (M012_TYPE(L0314_T_Thing) <= C03_THING_TYPE_SENSOR)) {
                                L0314_T_Thing = F0159_DUNGEON_GetNextThing(L0314_T_Thing);
                        }
                        T0172049_Footprints:
                        if (AL0307_uc_FootprintsAllowed && (AL0307_uc_ScentOrdinal = F0315_CHAMPION_GetScentOrdinal(P0319_i_MapX, P0320_i_MapY)) && (--AL0307_uc_ScentOrdinal >= G0407_s_Party.FirstScentIndex) && (AL0307_uc_ScentOrdinal < G0407_s_Party.LastScentIndex)) {
                                M008_SET(P0317_pui_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], MASK0x8000_FOOTPRINTS);
                        }
        }
        P0317_pui_SquareAspect[M550_FIRST_THING] = L0314_T_Thing;
}

void F0173_DUNGEON_SetCurrentMap(
REGISTER unsigned int16_t P0321_ui_MapIndex FINAL_SEPARATOR
{
        if (G0272_i_CurrentMapIndex == P0321_ui_MapIndex)
                return;
        G0272_i_CurrentMapIndex = P0321_ui_MapIndex;
        G0271_ppuc_CurrentMapData = G0279_pppuc_DungeonMapData[P0321_ui_MapIndex];
        G0269_ps_CurrentMap = G0277_ps_DungeonMaps + P0321_ui_MapIndex;
        G0273_i_CurrentMapWidth = G0269_ps_CurrentMap->A.Width + 1;
        G0274_i_CurrentMapHeight = G0269_ps_CurrentMap->A.Height + 1;
        G0275_as_CurrentMapDoorInfo[0] = G0254_as_Graphic559_DoorInfo[G0269_ps_CurrentMap->D.DoorSet0];
        G0275_as_CurrentMapDoorInfo[1] = G0254_as_Graphic559_DoorInfo[G0269_ps_CurrentMap->D.DoorSet1];
        G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount = &G0280_pui_DungeonColumnsCumulativeSquareThingCount[G0281_pui_DungeonMapsFirstColumnIndex[P0321_ui_MapIndex]];
}

void F0174_DUNGEON_SetCurrentMapAndPartyMap(
int16_t          P0322_i_MapIndex FINAL_SEPARATOR
{
        REGISTER unsigned char* L0316_puc_MapMetaData;
        REGISTER int16_t L0315_i_FloorOrnamentCount;


        F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex = P0322_i_MapIndex);
        L0316_puc_MapMetaData = G0271_ppuc_CurrentMapData[G0273_i_CurrentMapWidth - 1] + G0274_i_CurrentMapHeight;
        G0264_puc_CurrentMapAllowedCreatureTypes = L0316_puc_MapMetaData;
        F0007_MAIN_CopyBytes(M772_CAST_PC(L0316_puc_MapMetaData += G0269_ps_CurrentMap->C.CreatureTypeCount), M772_CAST_PC(G0261_auc_CurrentMapWallOrnamentIndices), M543_BYTE_COUNT_INT(G0265_i_CurrentMapInscriptionWallOrnamentIndex = G0269_ps_CurrentMap->B.WallOrnamentCount));
        F0007_MAIN_CopyBytes(M772_CAST_PC(L0316_puc_MapMetaData += G0265_i_CurrentMapInscriptionWallOrnamentIndex), M772_CAST_PC(G0262_auc_CurrentMapFloorOrnamentIndices), M543_BYTE_COUNT_INT(L0315_i_FloorOrnamentCount = G0269_ps_CurrentMap->B.FloorOrnamentCount));
        F0007_MAIN_CopyBytes(M772_CAST_PC(L0316_puc_MapMetaData + L0315_i_FloorOrnamentCount), M772_CAST_PC(G0263_auc_CurrentMapDoorOrnamentIndices), M543_BYTE_COUNT_INT(G0269_ps_CurrentMap->C.DoorOrnamentCount));
        G0261_auc_CurrentMapWallOrnamentIndices[G0265_i_CurrentMapInscriptionWallOrnamentIndex] = C0_WALL_ORNAMENT_INSCRIPTION;
}
/* END DUNGEON.C */

/* BEGIN GROUP.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
ACTIVE_GROUP* G0375_ps_ActiveGroups;
unsigned int16_t G0376_ui_MaximumActiveGroupCount;
unsigned int16_t G0377_ui_CurrentActiveGroupCount = 0;
int16_t G0378_i_CurrentGroupMapX;
int16_t G0379_i_CurrentGroupMapY;
THING G0380_T_CurrentGroupThing;
unsigned int16_t G0381_ui_CurrentGroupDistanceToParty;
int16_t G0382_i_CurrentGroupPrimaryDirectionToParty;
int16_t G0383_i_CurrentGroupSecondaryDirectionToParty;
unsigned char G0384_auc_GroupMovementTestedDirections[4];
char G0385_ac_FluxCages[4];
unsigned int16_t G0386_ui_FluxCageCount;
BOOLEAN G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter;
THING G0388_T_GroupMovementBlockedByGroupThing;
BOOLEAN G0389_B_GroupMovementBlockedByDoor;
BOOLEAN G0390_B_GroupMovementBlockedByParty;
unsigned int16_t G0391_ui_DropMovingCreatureFixedPossessionsCellCount;
unsigned char G0392_auc_DropMovingCreatureFixedPossessionsCells[4];

extern BOOLEAN F0197_GROUP_IsViewPartyBlocked();
extern BOOLEAN F0198_GROUP_IsSmellPartyBlocked();



THING F0175_GROUP_GetThing(
int16_t P0323_i_MapX SEPARATOR
int16_t P0324_i_MapY FINAL_SEPARATOR
{
        REGISTER THING L0317_T_Thing;


        L0317_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0323_i_MapX, P0324_i_MapY);
        while ((L0317_T_Thing != C0xFFFE_THING_ENDOFLIST) && (M012_TYPE(L0317_T_Thing) != C04_THING_TYPE_GROUP)) {
                L0317_T_Thing = F0159_DUNGEON_GetNextThing(L0317_T_Thing);
        }
        return L0317_T_Thing;
}

int16_t F0176_GROUP_GetCreatureOrdinalInCell(
REGISTER GROUP*           P0325_ps_Group SEPARATOR
REGISTER unsigned int16_t P0326_ui_Cell  FINAL_SEPARATOR
{
        REGISTER unsigned char L0319_uc_CreatureIndex;
        REGISTER unsigned char L0318_uc_GroupCells;
        REGISTER unsigned char L0320_uc_CreatureCell;


        if ((L0318_uc_GroupCells = F0145_DUNGEON_GetGroupCells(P0325_ps_Group, G0272_i_CurrentMapIndex)) == C0xFF_SINGLE_CENTERED_CREATURE) { /* A single centered creature in the group is present on all cells */
                return M000_INDEX_TO_ORDINAL(0);
        }
        L0319_uc_CreatureIndex = P0325_ps_Group->Count;
        if (M007_GET(G0243_as_Graphic559_CreatureInfo[P0325_ps_Group->Type].Attributes, MASK0x0003_SIZE) == C1_SIZE_HALF_SQUARE) {
                if ((F0147_DUNGEON_GetGroupDirections(P0325_ps_Group, G0272_i_CurrentMapIndex) & 0x0001) == (P0326_ui_Cell & 0x0001)) {
                        P0326_ui_Cell = M020_PREVIOUS(P0326_ui_Cell);
                }
                do {
                        if (((L0320_uc_CreatureCell = M050_CREATURE_VALUE(L0318_uc_GroupCells, L0319_uc_CreatureIndex)) == P0326_ui_Cell) || (L0320_uc_CreatureCell == M017_NEXT(P0326_ui_Cell))) {
                                return M000_INDEX_TO_ORDINAL(L0319_uc_CreatureIndex);
                        }
                } while (L0319_uc_CreatureIndex--);
        } else {
                do {
                        if (M050_CREATURE_VALUE(L0318_uc_GroupCells, L0319_uc_CreatureIndex) == P0326_ui_Cell) {
                                return M000_INDEX_TO_ORDINAL(L0319_uc_CreatureIndex);
                        }
                } while (L0319_uc_CreatureIndex--);
        }
        return 0;
}

int16_t F0177_GROUP_GetMeleeTargetCreatureOrdinal(
int16_t          P0327_i_GroupMapX     SEPARATOR
int16_t          P0328_i_GroupMapY     SEPARATOR
int16_t          P0329_i_PartyMapX     SEPARATOR
int16_t          P0330_i_PartyMapY     SEPARATOR
unsigned int16_t P0331_ui_ChampionCell FINAL_SEPARATOR
{
        REGISTER GROUP* L0324_ps_Group;
        REGISTER unsigned int16_t L0321_ui_Counter;
        REGISTER int16_t L0322_i_CreatureOrdinal;
        REGISTER THING L0323_T_GroupThing;
        unsigned char L0325_auc_OrderedCellsToAttack[4];


        if ((L0323_T_GroupThing = F0175_GROUP_GetThing(P0327_i_GroupMapX, P0328_i_GroupMapY)) == C0xFFFE_THING_ENDOFLIST) {
        }
        else {
                L0324_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0323_T_GroupThing);
                F0229_GROUP_SetOrderedCellsToAttack(M772_CAST_PC(L0325_auc_OrderedCellsToAttack), P0327_i_GroupMapX, P0328_i_GroupMapY, P0329_i_PartyMapX, P0330_i_PartyMapY, P0331_ui_ChampionCell);
                L0321_ui_Counter = 0;
                for (;;) { /*_Infinite loop_*/
                        if (L0321_ui_Counter == 4) {
                                return 0;
                        }
                        if (L0322_i_CreatureOrdinal = F0176_GROUP_GetCreatureOrdinalInCell(L0324_ps_Group, L0325_auc_OrderedCellsToAttack[L0321_ui_Counter])) {
                                return L0322_i_CreatureOrdinal;
                        }
                        L0321_ui_Counter++;
                }
        }
        return 0;
}

BOOLEAN F0223_GROUP_IsLordChaosAllowed(
int16_t P0468_i_MapX SEPARATOR
int16_t P0469_i_MapY FINAL_SEPARATOR
{
        REGISTER int16_t L0544_i_SquareType;


        return (((L0544_i_SquareType = M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(P0468_i_MapX, P0469_i_MapY))) == C01_ELEMENT_CORRIDOR)
                || (L0544_i_SquareType == C05_ELEMENT_TELEPORTER) || (L0544_i_SquareType == C02_ELEMENT_PIT) || (L0544_i_SquareType == C04_ELEMENT_DOOR));
}

unsigned int16_t F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
unsigned int16_t          P0332_ui_GroupValue    SEPARATOR
REGISTER unsigned int16_t P0333_ui_CreatureIndex SEPARATOR
REGISTER unsigned int16_t P0334_ui_CreatureValue FINAL_SEPARATOR
{
        P0334_ui_CreatureValue &= 0x0003;
        P0334_ui_CreatureValue <<= (P0333_ui_CreatureIndex <<= 1);
        return P0334_ui_CreatureValue | (P0332_ui_GroupValue & ~(3 << P0333_ui_CreatureIndex));
}

STATICFUNCTION long F0179_GROUP_GetCreatureAspectUpdateTime(
REGISTER ACTIVE_GROUP* P0335_ps_ActiveGroup  SEPARATOR
REGISTER int16_t       P0336_i_CreatureIndex SEPARATOR
BOOLEAN                P0337_B_IsAttacking   FINAL_SEPARATOR
{
        REGISTER GROUP* L0329_ps_Group;
        REGISTER unsigned int16_t L0326_ui_Multiple;
#define AL0326_ui_Aspect         L0326_ui_Multiple
#define AL0326_ui_AnimationTicks L0326_ui_Multiple
        REGISTER int16_t L0328_i_Offset;
        REGISTER unsigned int16_t L0327_ui_CreatureGraphicInfo;
        REGISTER unsigned int16_t L0331_ui_CreatureType;
        BOOLEAN L0330_B_ProcessGroup;


        L0329_ps_Group = &(((GROUP*)G0284_apuc_ThingData[C04_THING_TYPE_GROUP])[P0335_ps_ActiveGroup->GroupThingIndex]);
        L0327_ui_CreatureGraphicInfo = G0243_as_Graphic559_CreatureInfo[L0331_ui_CreatureType = L0329_ps_Group->Type].GraphicInfo;
        if (L0330_B_ProcessGroup = (P0336_i_CreatureIndex < 0)) { /* If the creature index is negative then all creatures in the group are processed */
                P0336_i_CreatureIndex = L0329_ps_Group->Count;
        }
        do {
                AL0326_ui_Aspect = P0335_ps_ActiveGroup->Aspect[P0336_i_CreatureIndex] & (MASK0x0080_IS_ATTACKING | MASK0x0040_FLIP_BITMAP);
                if (L0328_i_Offset = M052_MAXIMUM_HORIZONTAL_OFFSET(L0327_ui_CreatureGraphicInfo)) {
                        L0328_i_Offset = M002_RANDOM(L0328_i_Offset);
                        if (M005_RANDOM(2)) {
                                L0328_i_Offset = (-L0328_i_Offset) & 0x0007;
                        }
                        M024_SET_HORIZONTAL_OFFSET(AL0326_ui_Aspect, L0328_i_Offset);
                }
                if (L0328_i_Offset = M053_MAXIMUM_VERTICAL_OFFSET(L0327_ui_CreatureGraphicInfo)) {
                        L0328_i_Offset = M002_RANDOM(L0328_i_Offset);
                        if (M005_RANDOM(2)) {
                                L0328_i_Offset = (-L0328_i_Offset) & 0x0007;
                        }
                        M025_SET_VERTICAL_OFFSET(AL0326_ui_Aspect, L0328_i_Offset);
                }
                if (P0337_B_IsAttacking) {
                        if (M007_GET(L0327_ui_CreatureGraphicInfo, MASK0x0200_FLIP_ATTACK)) {
                                if (M007_GET(AL0326_ui_Aspect, MASK0x0080_IS_ATTACKING) && (L0331_ui_CreatureType == C18_CREATURE_ANIMATED_ARMOUR_DETH_KNIGHT)) {
                                        if (M005_RANDOM(2)) {
                                                M010_TOGGLE(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                                F0064_SOUND_RequestPlay_CPSD(M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, G0378_i_CurrentGroupMapX, G0379_i_CurrentGroupMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                        }
                                } else {
                                        if (!M007_GET(AL0326_ui_Aspect, MASK0x0080_IS_ATTACKING) || !M007_GET(L0327_ui_CreatureGraphicInfo, MASK0x0400_FLIP_DURING_ATTACK)) {
                                                if (M005_RANDOM(2)) {
                                                        M008_SET(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                                } else {
#ifdef PC_FIX_CODE_SIZE
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
#endif
                                                        M009_CLEAR(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                                }
                                        }
                                }
                        } else {
                                M009_CLEAR(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                        }
                        M008_SET(AL0326_ui_Aspect, MASK0x0080_IS_ATTACKING);
                } else {
                        if (M007_GET(L0327_ui_CreatureGraphicInfo, MASK0x0004_FLIP_NON_ATTACK)) {
                                if (L0331_ui_CreatureType == C13_CREATURE_COUATL) {
                                        if (M005_RANDOM(2)) {
                                                M010_TOGGLE(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                                F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound(C13_CREATURE_COUATL), G0378_i_CurrentGroupMapX, G0379_i_CurrentGroupMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                        }
                                } else {
                                        if (M005_RANDOM(2)) {
                                                M008_SET(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                        } else {
#ifdef PC_FIX_CODE_SIZE
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
        L0328_i_Offset++;
#endif
                                                M009_CLEAR(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                                        }
                                }
                        } else {
                                M009_CLEAR(AL0326_ui_Aspect, MASK0x0040_FLIP_BITMAP);
                        }
                        M009_CLEAR(AL0326_ui_Aspect, MASK0x0080_IS_ATTACKING);
                }
                P0335_ps_ActiveGroup->Aspect[P0336_i_CreatureIndex] = AL0326_ui_Aspect;
        } while (L0330_B_ProcessGroup && (P0336_i_CreatureIndex--));
        AL0326_ui_AnimationTicks = G0243_as_Graphic559_CreatureInfo[L0329_ps_Group->Type].AnimationTicks;
        return G0313_ul_GameTime + (P0337_B_IsAttacking ? M064_NEXT_ATTACK_ASPECT_UPDATE_TICKS(AL0326_ui_AnimationTicks) : M063_NEXT_NON_ATTACK_ASPECT_UPDATE_TICKS(AL0326_ui_AnimationTicks)) + M005_RANDOM(2);
}

void F0180_GROUP_StartWandering(
int16_t P0338_i_MapX SEPARATOR
int16_t P0339_i_MapY FINAL_SEPARATOR
{
        REGISTER GROUP* L0332_ps_Group;
        EVENT L0333_s_Event;


        L0332_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(F0175_GROUP_GetThing(P0338_i_MapX, P0339_i_MapY));
        if (L0332_ps_Group->Behavior >= C4_BEHAVIOR_USELESS) {
                L0332_ps_Group->Behavior = C0_BEHAVIOR_WANDER;
        }
        M033_SET_MAP_AND_TIME(L0333_s_Event.Map_Time, G0272_i_CurrentMapIndex, (G0313_ul_GameTime + 1));
        L0333_s_Event.A.A.Type = C37_EVENT_UPDATE_BEHAVIOR_GROUP;
        L0333_s_Event.A.A.Priority = 255 - G0243_as_Graphic559_CreatureInfo[L0332_ps_Group->Type].MovementTicks; /* The fastest creatures (with small MovementTicks value) get higher event priority */
        L0333_s_Event.C.Ticks = 0;
        L0333_s_Event.B.Location.MapX = P0338_i_MapX;
        L0333_s_Event.B.Location.MapY = P0339_i_MapY;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0333_s_Event);
}

void F0181_GROUP_DeleteEvents(
REGISTER unsigned int16_t P0340_i_MapX SEPARATOR
REGISTER unsigned int16_t P0341_i_MapY FINAL_SEPARATOR
{
        REGISTER EVENT* L0336_ps_Event;
        REGISTER unsigned int16_t L0334_ui_EventIndex;
        REGISTER unsigned int16_t L0335_ui_EventType;


        L0336_ps_Event = G0370_ps_Events;
        for (L0334_ui_EventIndex = 0; L0334_ui_EventIndex < G2009_i_LargestUsedEventOrdinal; L0334_ui_EventIndex++) {
                if ((M029_MAP(L0336_ps_Event->Map_Time) == G0272_i_CurrentMapIndex) &&
                    ((L0335_ui_EventType = L0336_ps_Event->A.A.Type) > C29_EVENT_GROUP_REACTION_DANGER_ON_SQUARE - 1) && (L0335_ui_EventType < C41_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1) &&
                    (L0336_ps_Event->B.Location.MapX == P0340_i_MapX) && (L0336_ps_Event->B.Location.MapY == P0341_i_MapY)) {
                        F0237_TIMELINE_DeleteEvent(L0334_ui_EventIndex);
                }
                L0336_ps_Event++;
        }
}

void F0182_GROUP_StopAttacking(
ACTIVE_GROUP* P0342_ps_ActiveGroup SEPARATOR
int16_t       P0343_i_MapX         SEPARATOR
int16_t       P0344_i_MapY         FINAL_SEPARATOR
{
        REGISTER int16_t L0337_i_CreatureIndex;


        for (L0337_i_CreatureIndex = 0; L0337_i_CreatureIndex < 4; M009_CLEAR(P0342_ps_ActiveGroup->Aspect[L0337_i_CreatureIndex++], MASK0x0080_IS_ATTACKING));
        F0181_GROUP_DeleteEvents(P0343_i_MapX, P0344_i_MapY);
}

void F0183_GROUP_AddActiveGroup(
THING   P0345_T_GroupThing SEPARATOR
int16_t P0346_i_MapX       SEPARATOR
int16_t P0347_i_MapY       FINAL_SEPARATOR
{
        REGISTER ACTIVE_GROUP* L0341_ps_ActiveGroup;
        REGISTER GROUP* L0340_ps_Group;
        REGISTER unsigned int16_t L0339_ui_CreatureIndex;
        REGISTER unsigned int16_t L0344_ui_ActiveGroupIndex;


        L0341_ps_ActiveGroup = G0375_ps_ActiveGroups;
        L0344_ui_ActiveGroupIndex = 0;
        while (L0341_ps_ActiveGroup->GroupThingIndex >= 0) {
                if (++L0344_ui_ActiveGroupIndex >= G0376_ui_MaximumActiveGroupCount) {
                        F0019_MAIN_DisplayErrorAndStop(C71_ERROR_TOO_MANY_ACTIVE_GROUPS);
                }
                L0341_ps_ActiveGroup++;
        }
        G0377_ui_CurrentActiveGroupCount++;
        L0340_ps_Group = ((GROUP*)G0284_apuc_ThingData[C04_THING_TYPE_GROUP]) + (L0341_ps_ActiveGroup->GroupThingIndex = M013_INDEX(P0345_T_GroupThing));
        L0341_ps_ActiveGroup->Cells = L0340_ps_Group->Cells;
        L0340_ps_Group->ActiveGroupIndex = L0344_ui_ActiveGroupIndex;
        L0341_ps_ActiveGroup->PriorMapX = L0341_ps_ActiveGroup->HomeMapX = P0346_i_MapX;
        L0341_ps_ActiveGroup->PriorMapY = L0341_ps_ActiveGroup->HomeMapY = P0347_i_MapY;
        L0341_ps_ActiveGroup->LastMoveTime = G0313_ul_GameTime - 127;
        L0339_ui_CreatureIndex = L0340_ps_Group->Count;
        do {
                L0341_ps_ActiveGroup->Directions = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0341_ps_ActiveGroup->Directions, L0339_ui_CreatureIndex, L0340_ps_Group->Direction);
                L0341_ps_ActiveGroup->Aspect[L0339_ui_CreatureIndex] = 0;
        } while (L0339_ui_CreatureIndex--);
        F0179_GROUP_GetCreatureAspectUpdateTime(L0341_ps_ActiveGroup, CM1_WHOLE_CREATURE_GROUP, C0_FALSE);
}

void F0184_GROUP_RemoveActiveGroup(
REGISTER unsigned int16_t P0348_ui_ActiveGroupIndex FINAL_SEPARATOR
{
        REGISTER ACTIVE_GROUP* L0347_ps_ActiveGroup;
        REGISTER GROUP* L0348_ps_Group;


        if ((P0348_ui_ActiveGroupIndex > G0376_ui_MaximumActiveGroupCount) || (G0375_ps_ActiveGroups[P0348_ui_ActiveGroupIndex].GroupThingIndex < 0)) {
                return;
        }
        L0347_ps_ActiveGroup = &G0375_ps_ActiveGroups[P0348_ui_ActiveGroupIndex];
        L0348_ps_Group = &((GROUP*)G0284_apuc_ThingData[C04_THING_TYPE_GROUP])[L0347_ps_ActiveGroup->GroupThingIndex];
        G0377_ui_CurrentActiveGroupCount--;
        L0348_ps_Group->Cells = L0347_ps_ActiveGroup->Cells;
        L0348_ps_Group->Direction = M021_NORMALIZE(L0347_ps_ActiveGroup->Directions);
        if (L0348_ps_Group->Behavior >= C4_BEHAVIOR_USELESS) {
                L0348_ps_Group->Behavior = C0_BEHAVIOR_WANDER;
        }
        L0347_ps_ActiveGroup->GroupThingIndex = -1;
}

THING F0185_GROUP_GetGenerated(
int16_t                   P0349_i_CreatureType     SEPARATOR
int16_t                   P0350_i_HealthMultiplier SEPARATOR
REGISTER unsigned int16_t P0351_ui_CreatureCount   SEPARATOR
unsigned int16_t          P0352_i_Direction        SEPARATOR
int16_t                   P0353_i_MapX             SEPARATOR
int16_t                   P0354_i_MapY             FINAL_SEPARATOR
{
        REGISTER GROUP* L0353_ps_Group;
        REGISTER CREATURE_INFO* L0354_ps_CreatureInfo;
        REGISTER unsigned int16_t L0352_ui_GroupCells;
        REGISTER unsigned int16_t L0351_ui_Cell;
        REGISTER THING L0349_T_GroupThing;
        REGISTER unsigned int16_t L0350_ui_BaseHealth;
        BOOLEAN L0355_B_SeveralCreaturesInGroup;


        if (((G0377_ui_CurrentActiveGroupCount >= (G0376_ui_MaximumActiveGroupCount - 5)) && (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex)) || ((L0349_T_GroupThing = F0166_DUNGEON_GetUnusedThing(C04_THING_TYPE_GROUP)) == C0xFFFF_THING_NONE)) {
                return C0xFFFF_THING_NONE;
        }
        L0353_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0349_T_GroupThing);
        L0353_ps_Group->Slot = C0xFFFE_THING_ENDOFLIST;
        L0353_ps_Group->DoNotDiscard = C0_FALSE;
        L0353_ps_Group->Direction = P0352_i_Direction;
        L0353_ps_Group->Count = P0351_ui_CreatureCount;
        if (L0355_B_SeveralCreaturesInGroup = P0351_ui_CreatureCount) {
                L0351_ui_Cell = M004_RANDOM(4);
        } else {
                L0352_ui_GroupCells = C0xFF_SINGLE_CENTERED_CREATURE;
        }
        L0354_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[L0353_ps_Group->Type = P0349_i_CreatureType];
        L0350_ui_BaseHealth = L0354_ps_CreatureInfo->BaseHealth;
        do {
                L0353_ps_Group->Health[P0351_ui_CreatureCount] = (L0350_ui_BaseHealth * P0350_i_HealthMultiplier) + M002_RANDOM((L0350_ui_BaseHealth >> 2) + 1);
                if (L0355_B_SeveralCreaturesInGroup) {
                        L0352_ui_GroupCells = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0352_ui_GroupCells, P0351_ui_CreatureCount, L0351_ui_Cell++);
                        if (M007_GET(L0354_ps_CreatureInfo->Attributes, MASK0x0003_SIZE) == C1_SIZE_HALF_SQUARE) {
                                L0351_ui_Cell++;
                        }
                        L0351_ui_Cell &= 0x0003;
                }
        } while (P0351_ui_CreatureCount--);
        L0353_ps_Group->Cells = L0352_ui_GroupCells;
        if (F0267_MOVE_GetMoveResult_CPSCE(L0349_T_GroupThing, CM1_MAPX_NOT_ON_A_SQUARE, 0, P0353_i_MapX, P0354_i_MapY)) { /* If F0267_MOVE_GetMoveResult_CPSCE returns C1_TRUE then the group was either killed by a projectile impact (in which case the thing data was marked as unused) or the party is on the destination square and an event is created to move the creature into the dungeon later (in which case the thing is referenced in the event) */
                return C0xFFFF_THING_NONE;
        }
        F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, P0353_i_MapX, P0354_i_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
        return L0349_T_GroupThing;
}

STATICFUNCTION void F0186_GROUP_DropCreatureFixedPossessions(
unsigned int16_t P0355_ui_CreatureType SEPARATOR
int16_t          P0356_i_MapX          SEPARATOR
int16_t          P0357_i_MapY          SEPARATOR
unsigned int16_t P0358_ui_Cell         SEPARATOR
int16_t          P0359_i_Mode          FINAL_SEPARATOR
{
        REGISTER unsigned int16_t* L0359_pui_FixedPossessions;
        REGISTER WEAPON* L0360_ps_Weapon;
        REGISTER unsigned int16_t L0356_ui_FixedPossession;
        REGISTER THING L0358_T_Thing;
        REGISTER int16_t L0357_i_ThingType;
        REGISTER unsigned int16_t L0361_B_Cursed;
        REGISTER BOOLEAN L0362_B_WeaponDropped;


        L0361_B_Cursed = C0_FALSE;
        L0362_B_WeaponDropped = C0_FALSE;
        switch (P0355_ui_CreatureType) {
                default:
                        return;
                case C12_CREATURE_SKELETON:
                        L0359_pui_FixedPossessions = G0245_aui_Graphic559_FixedPossessionsCreature12Skeleton;
                        break;
                case C09_CREATURE_STONE_GOLEM:
                        L0359_pui_FixedPossessions = G0246_aui_Graphic559_FixedPossessionsCreature09StoneGolem;
                        break;
                case C16_CREATURE_TROLIN_ANTMAN:
                        L0359_pui_FixedPossessions = G0247_aui_Graphic559_FixedPossessionsCreature16Trolin_Antman;
                        break;
                case C18_CREATURE_ANIMATED_ARMOUR_DETH_KNIGHT:
                        L0361_B_Cursed = C1_TRUE;
                        L0359_pui_FixedPossessions = G0248_aui_Graphic559_FixedPossessionsCreature18AnimatedArmour_DethKnight;
                        break;
                case C07_CREATURE_ROCK_ROCKPILE:
                        L0359_pui_FixedPossessions = G0249_aui_Graphic559_FixedPossessionsCreature07Rock_RockPile;
                        break;
                case C04_CREATURE_PAIN_RAT_HELLHOUND:
                        L0359_pui_FixedPossessions = G0250_aui_Graphic559_FixedPossessionsCreature04PainRat_Hellhound;
                        break;
                case C06_CREATURE_SCREAMER:
                        L0359_pui_FixedPossessions = G0251_aui_Graphic559_FixedPossessionsCreature06Screamer;
                        break;
                case C15_CREATURE_MAGENTA_WORM_WORM:
                        L0359_pui_FixedPossessions = G0252_aui_Graphic559_FixedPossessionsCreature15MagentaWorm_Worm;
                        break;
                case C24_CREATURE_RED_DRAGON:
                        L0359_pui_FixedPossessions = G0253_aui_Graphic559_FixedPossessionsCreature24RedDragon;
        }
        while (L0356_ui_FixedPossession = *L0359_pui_FixedPossessions++) {
                if (M007_GET(L0356_ui_FixedPossession, MASK0x8000_RANDOM_DROP) && M005_RANDOM(2))
                        continue;
                if (M009_CLEAR(L0356_ui_FixedPossession, MASK0x8000_RANDOM_DROP) >= C127_OBJECT_INFO_INDEX_FIRST_JUNK) {
                        L0357_i_ThingType = C10_THING_TYPE_JUNK;
                        L0356_ui_FixedPossession -= C127_OBJECT_INFO_INDEX_FIRST_JUNK;
                } else {
                        if (L0356_ui_FixedPossession >= C069_OBJECT_INFO_INDEX_FIRST_ARMOUR) {
                                L0357_i_ThingType = C06_THING_TYPE_ARMOUR;
                                L0356_ui_FixedPossession -= C069_OBJECT_INFO_INDEX_FIRST_ARMOUR;
                        } else {
                                L0362_B_WeaponDropped = C1_TRUE;
                                L0357_i_ThingType = C05_THING_TYPE_WEAPON;
                                L0356_ui_FixedPossession -= C023_OBJECT_INFO_INDEX_FIRST_WEAPON;
                        }
                }
                if ((L0358_T_Thing = F0166_DUNGEON_GetUnusedThing(L0357_i_ThingType)) == C0xFFFF_THING_NONE) {
                        continue;
                }
                L0360_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(L0358_T_Thing);
                L0360_ps_Weapon->Type = L0356_ui_FixedPossession; /* The same pointer type is used no matter the actual type C05_THING_TYPE_WEAPON, C06_THING_TYPE_ARMOUR or C10_THING_TYPE_JUNK */
                L0360_ps_Weapon->Cursed = L0361_B_Cursed;
                L0358_T_Thing = M015_THING_WITH_NEW_CELL(L0358_T_Thing, ((P0358_ui_Cell == C0xFF_SINGLE_CENTERED_CREATURE) || !M004_RANDOM(4)) ? M004_RANDOM(4) : P0358_ui_Cell);
                F0267_MOVE_GetMoveResult_CPSCE(L0358_T_Thing, CM1_MAPX_NOT_ON_A_SQUARE, 0, P0356_i_MapX, P0357_i_MapY);
        }
        F0064_SOUND_RequestPlay_CPSD(L0362_B_WeaponDropped ? C00_SOUND_METALLIC_THUD : C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, P0356_i_MapX, P0357_i_MapY, P0359_i_Mode);
}

void F0187_GROUP_DropMovingCreatureFixedPossessions(
THING   P0360_T_Thing SEPARATOR
int16_t P0361_i_MapX  SEPARATOR
int16_t P0362_i_MapY  FINAL_SEPARATOR
{
        GROUP* L0363_ps_Group;
        int16_t L0364_i_CreatureType;


        if (G0391_ui_DropMovingCreatureFixedPossessionsCellCount) {
                L0363_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0360_T_Thing);
                L0364_i_CreatureType = L0363_ps_Group->Type;
                while (G0391_ui_DropMovingCreatureFixedPossessionsCellCount) {
                        F0186_GROUP_DropCreatureFixedPossessions(L0364_i_CreatureType, P0361_i_MapX, P0362_i_MapY, G0392_auc_DropMovingCreatureFixedPossessionsCells[--G0391_ui_DropMovingCreatureFixedPossessionsCellCount], C02_MODE_PLAY_ONE_TICK_LATER);
                }
        }
}

void F0188_GROUP_DropGroupPossessions(
int16_t          P0363_i_MapX       SEPARATOR
int16_t          P0364_i_MapY       SEPARATOR
THING            P0365_T_GroupThing SEPARATOR
REGISTER int16_t P0366_i_Mode       FINAL_SEPARATOR
{
        REGISTER GROUP* L0367_ps_Group;
        REGISTER THING L0365_T_CurrentThing;
        REGISTER unsigned int16_t L0369_i_CreatureIndex;
        REGISTER unsigned int16_t L0370_ui_GroupCells;
        REGISTER BOOLEAN L0371_B_WeaponDropped;
        REGISTER THING L0366_T_NextThing;
        unsigned int16_t L0368_ui_CreatureType;


        L0367_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0365_T_GroupThing);
        if ((P0366_i_Mode >= C00_MODE_PLAY_IMMEDIATELY) && M007_GET(G0243_as_Graphic559_CreatureInfo[L0368_ui_CreatureType = L0367_ps_Group->Type].Attributes, MASK0x0200_DROP_FIXED_POSSESSIONS)) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                L0369_i_CreatureIndex = L0367_ps_Group->Count;
                L0370_ui_GroupCells = F0145_DUNGEON_GetGroupCells(L0367_ps_Group, G0272_i_CurrentMapIndex);
                do {
                        F0186_GROUP_DropCreatureFixedPossessions(L0368_ui_CreatureType, P0363_i_MapX, P0364_i_MapY, (L0370_ui_GroupCells == C0xFF_SINGLE_CENTERED_CREATURE) ? C0xFF_SINGLE_CENTERED_CREATURE : M050_CREATURE_VALUE(L0370_ui_GroupCells, L0369_i_CreatureIndex), P0366_i_Mode);
                } while (L0369_i_CreatureIndex--);
        }
        if ((L0365_T_CurrentThing = L0367_ps_Group->Slot) != C0xFFFE_THING_ENDOFLIST) {
                L0371_B_WeaponDropped = C0_FALSE;
                do {
                        L0366_T_NextThing = F0159_DUNGEON_GetNextThing(L0365_T_CurrentThing);
                        L0365_T_CurrentThing = M015_THING_WITH_NEW_CELL(L0365_T_CurrentThing, M004_RANDOM(4));
                        if (M012_TYPE(L0365_T_CurrentThing) == C05_THING_TYPE_WEAPON) {
                                L0371_B_WeaponDropped = C1_TRUE;
                        }
                        F0267_MOVE_GetMoveResult_CPSCE(L0365_T_CurrentThing, CM1_MAPX_NOT_ON_A_SQUARE, 0, P0363_i_MapX, P0364_i_MapY);
                } while ((L0365_T_CurrentThing = L0366_T_NextThing) != C0xFFFE_THING_ENDOFLIST);
                if (P0366_i_Mode >= C00_MODE_PLAY_IMMEDIATELY) {
                        F0064_SOUND_RequestPlay_CPSD(L0371_B_WeaponDropped ? C00_SOUND_METALLIC_THUD : C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, P0363_i_MapX, P0364_i_MapY, P0366_i_Mode);
                }
        }
}

void F0189_GROUP_Delete(
int16_t P0367_i_MapX SEPARATOR
int16_t P0368_i_MapY FINAL_SEPARATOR
{
        REGISTER GROUP* L0373_ps_Group;
        REGISTER THING L0372_T_GroupThing;


        if ((L0372_T_GroupThing = F0175_GROUP_GetThing(P0367_i_MapX, P0368_i_MapY)) == C0xFFFE_THING_ENDOFLIST) {
                return;
        }
        L0373_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0372_T_GroupThing);
        F0008_MAIN_ClearBytes(M772_CAST_PC(L0373_ps_Group->Health), M543_BYTE_COUNT_INT(sizeof(L0373_ps_Group->Health)));
        F0267_MOVE_GetMoveResult_CPSCE(L0372_T_GroupThing, P0367_i_MapX, P0368_i_MapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);
        L0373_ps_Group->Next = C0xFFFF_THING_NONE;
        if (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) {
                G0375_ps_ActiveGroups[L0373_ps_Group->ActiveGroupIndex].GroupThingIndex = -1;
                G0377_ui_CurrentActiveGroupCount--;
        }
        F0181_GROUP_DeleteEvents(P0367_i_MapX, P0368_i_MapY);
}

int16_t F0190_GROUP_GetDamageCreatureOutcome(
REGISTER GROUP*  P0369_ps_Group         SEPARATOR
unsigned int16_t P0370_ui_CreatureIndex SEPARATOR
unsigned int16_t          P0371_i_MapX  SEPARATOR
unsigned int16_t          P0372_i_MapY  SEPARATOR
int16_t          P0373_i_Damage         SEPARATOR
BOOLEAN          P0374_B_NotMoving      FINAL_SEPARATOR /* When a creature is killed while not moving then possessions are dropped in this function on the current map. When a creature is killed while moving (like falling in a pit) then possessions are not dropped in this function but rather in F0267_MOVE_GetMoveResult_CPSCE on the destination map which may be different from the current (source) map */
{
        REGISTER CREATURE_INFO* L0376_ps_CreatureInfo;
        REGISTER EVENT* L0377_ps_Event;
        ACTIVE_GROUP* L0378_ps_ActiveGroup;
        REGISTER unsigned int16_t L0374_ui_Multiple;
#define AL0374_ui_EventIndex    L0374_ui_Multiple
#define AL0374_ui_CreatureIndex L0374_ui_Multiple
#define AL0374_ui_CreatureSize  L0374_ui_Multiple
#define AL0374_ui_Attack        L0374_ui_Multiple
        REGISTER unsigned int16_t L0375_ui_Multiple;
#define AL0375_ui_Outcome           L0375_ui_Multiple
#define AL0375_ui_EventType         L0375_ui_Multiple
#define AL0375_ui_NextCreatureIndex L0375_ui_Multiple
#define AL0380_ui_CreatureType   L0380_ui_Multiple
#define AL0380_ui_FearResistance L0380_ui_Multiple
        REGISTER unsigned int16_t L0381_ui_GroupCells;
        REGISTER unsigned int16_t L0382_ui_GroupDirections;
        REGISTER unsigned int16_t L0380_ui_Multiple;
        unsigned int16_t L0379_ui_CreatureCount;
        unsigned int16_t L0384_ui_Cell;
        BOOLEAN L0383_B_CurrentMapIsPartyMap;


        L0376_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[AL0380_ui_CreatureType = P0369_ps_Group->Type];
        if (L0376_ps_CreatureInfo->Defense == C255_IMMUNE_TO_DAMAGE) /* Lord Chaos cannot be damaged */
                goto T0190024;
        if (P0369_ps_Group->Health[P0370_ui_CreatureIndex] <= P0373_i_Damage) {
                L0381_ui_GroupCells = F0145_DUNGEON_GetGroupCells(P0369_ps_Group, G0272_i_CurrentMapIndex);
                L0384_ui_Cell = (L0381_ui_GroupCells == C0xFF_SINGLE_CENTERED_CREATURE) ? C0xFF_SINGLE_CENTERED_CREATURE : M050_CREATURE_VALUE(L0381_ui_GroupCells, P0370_ui_CreatureIndex);
                if (!(L0379_ui_CreatureCount = P0369_ps_Group->Count)) { /* If there is a single creature in the group */
                        if (P0374_B_NotMoving) {
                                F0188_GROUP_DropGroupPossessions(P0371_i_MapX, P0372_i_MapY, F0175_GROUP_GetThing(P0371_i_MapX, P0372_i_MapY), C02_MODE_PLAY_ONE_TICK_LATER);
                                F0189_GROUP_Delete(P0371_i_MapX, P0372_i_MapY);
                        }
                        AL0375_ui_Outcome = C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP;
                } else { /* If there are several creatures in the group */
                        L0382_ui_GroupDirections = F0147_DUNGEON_GetGroupDirections(P0369_ps_Group, G0272_i_CurrentMapIndex);
                        if (M007_GET(L0376_ps_CreatureInfo->Attributes, MASK0x0200_DROP_FIXED_POSSESSIONS)) {
                                if (P0374_B_NotMoving) {
                                        F0186_GROUP_DropCreatureFixedPossessions(AL0380_ui_CreatureType, P0371_i_MapX, P0372_i_MapY, L0384_ui_Cell, C02_MODE_PLAY_ONE_TICK_LATER);
                                } else {
                                        G0392_auc_DropMovingCreatureFixedPossessionsCells[G0391_ui_DropMovingCreatureFixedPossessionsCellCount++] = L0384_ui_Cell;
                                }
                        }
                        if (L0383_B_CurrentMapIsPartyMap = (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex)) {
                                L0378_ps_ActiveGroup = &G0375_ps_ActiveGroups[P0369_ps_Group->ActiveGroupIndex];
                        }
                        if (P0369_ps_Group->Behavior == C6_BEHAVIOR_ATTACK) {
                                L0377_ps_Event = G0370_ps_Events;
                                for (AL0374_ui_EventIndex = 0; AL0374_ui_EventIndex < G2009_i_LargestUsedEventOrdinal; AL0374_ui_EventIndex++) {
                                        if ((M029_MAP(L0377_ps_Event->Map_Time) == G0272_i_CurrentMapIndex) &&
                                            (L0377_ps_Event->B.Location.MapX == P0371_i_MapX) &&
                                            (L0377_ps_Event->B.Location.MapY == P0372_i_MapY) &&
                                            ((AL0375_ui_EventType = L0377_ps_Event->A.A.Type) > C32_EVENT_UPDATE_ASPECT_GROUP) &&
                                            (AL0375_ui_EventType < C41_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1)) {
                                                if (AL0375_ui_EventType < C37_EVENT_UPDATE_BEHAVIOR_GROUP) {
                                                        AL0375_ui_EventType -= C33_EVENT_UPDATE_ASPECT_CREATURE_0; /* Get creature index for events 33 to 36 */
                                                } else {
                                                        AL0375_ui_EventType -= C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0; /* Get creature index for events 38 to 41 */
                                                }
                                                if (AL0375_ui_NextCreatureIndex == P0370_ui_CreatureIndex) {
                                                        F0237_TIMELINE_DeleteEvent(AL0374_ui_EventIndex);
                                                } else {
                                                        if (AL0375_ui_NextCreatureIndex > P0370_ui_CreatureIndex) {
                                                                L0377_ps_Event->A.A.Type--;
                                                                F0236_TIMELINE_FixPlacement(F0235_TIMELINE_GetIndex(AL0374_ui_EventIndex));
                                                        }
                                                }
                                        }
                                        L0377_ps_Event++;
                                }
                                if (L0383_B_CurrentMapIsPartyMap && ((AL0380_ui_FearResistance = M057_FEAR_RESISTANCE(L0376_ps_CreatureInfo->Properties)) != C15_IMMUNE_TO_FEAR) && ((AL0380_ui_FearResistance += L0379_ui_CreatureCount - 1) < (M003_RANDOM(16)))) { /* Test if the death of a creature frigthens the remaining creatures in the group */
                                        L0378_ps_ActiveGroup->DelayFleeingFromTarget = M002_RANDOM(100 - (AL0380_ui_FearResistance << 2)) + 20;
                                        P0369_ps_Group->Behavior = C5_BEHAVIOR_FLEE;
                                }
                        }
                        for (AL0375_ui_NextCreatureIndex = AL0374_ui_CreatureIndex = P0370_ui_CreatureIndex; AL0374_ui_CreatureIndex < L0379_ui_CreatureCount; AL0374_ui_CreatureIndex++) {
                                AL0375_ui_NextCreatureIndex++;
                                P0369_ps_Group->Health[AL0374_ui_CreatureIndex] = P0369_ps_Group->Health[AL0375_ui_NextCreatureIndex];
                                L0382_ui_GroupDirections = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0382_ui_GroupDirections, AL0374_ui_CreatureIndex, M050_CREATURE_VALUE(L0382_ui_GroupDirections, AL0375_ui_NextCreatureIndex));
                                L0381_ui_GroupCells = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0381_ui_GroupCells, AL0374_ui_CreatureIndex, M050_CREATURE_VALUE(L0381_ui_GroupCells, AL0375_ui_NextCreatureIndex));
                                if (L0383_B_CurrentMapIsPartyMap) {
                                        L0378_ps_ActiveGroup->Aspect[AL0374_ui_CreatureIndex] = L0378_ps_ActiveGroup->Aspect[AL0375_ui_NextCreatureIndex];
                                }
                        }
                        L0381_ui_GroupCells &= 0x003F;
                        F0146_DUNGEON_SetGroupCells(P0369_ps_Group, L0381_ui_GroupCells, G0272_i_CurrentMapIndex);
                        F0148_DUNGEON_SetGroupDirections(P0369_ps_Group, L0382_ui_GroupDirections, G0272_i_CurrentMapIndex);
                        P0369_ps_Group->Count--;
                        AL0375_ui_Outcome = C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP;
                }
                if ((AL0374_ui_CreatureSize = M007_GET(L0376_ps_CreatureInfo->Attributes, MASK0x0003_SIZE)) == C0_SIZE_QUARTER_SQUARE) {
                        AL0374_ui_Attack = 110;
                } else {
                        if (AL0374_ui_CreatureSize == C1_SIZE_HALF_SQUARE) {
                                AL0374_ui_Attack = 190;
                        } else {
                                AL0374_ui_Attack = 255;
                        }
                }
                F0213_EXPLOSION_Create(C0xFFA8_THING_EXPLOSION_SMOKE, AL0374_ui_Attack, P0371_i_MapX, P0372_i_MapY, L0384_ui_Cell); /* BUG0_66 Smoke is placed on the source map instead of the destination map when a creature dies by falling through a pit. The game has a special case to correctly drop the creature possessions on the destination map but there is no such special case for the smoke. Note that the death must be caused by the damage of the fall (there is no smoke if the creature is removed because its type is not allowed on the destination map). However this bug has no visible consequence because of BUG0_26: the smoke explosion falls in the pit right after being placed in the dungeon and before being drawn on screen so it is only visible on the destination square */
                return AL0375_ui_Outcome;
        }
        else {
                if (P0373_i_Damage > 0) {
                        P0369_ps_Group->Health[P0370_ui_CreatureIndex] -= P0373_i_Damage;
                }
                T0190024:
                return C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
        }
}

int16_t F0191_GROUP_GetDamageAllCreaturesOutcome(
REGISTER GROUP*  P0375_ps_Group    SEPARATOR
int16_t          P0376_i_MapX      SEPARATOR
int16_t          P0377_i_MapY      SEPARATOR
REGISTER int16_t P0378_i_Attack    SEPARATOR
BOOLEAN          P0379_B_NotMoving FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0385_ui_RandomAttack;
        REGISTER int16_t L0386_i_CreatureIndex;
        REGISTER BOOLEAN L0388_B_KilledSomeCreatures;
        REGISTER BOOLEAN L0389_B_KilledAllCreatures;
        int16_t L0387_i_Outcome;


        L0388_B_KilledSomeCreatures = C0_FALSE;
        L0389_B_KilledAllCreatures = C1_TRUE;
        G0391_ui_DropMovingCreatureFixedPossessionsCellCount = 0;
        if (P0378_i_Attack > 0) {
                L0386_i_CreatureIndex = P0375_ps_Group->Count;
                P0378_i_Attack -= (L0385_ui_RandomAttack = (P0378_i_Attack >> 3) + 1);
                L0385_ui_RandomAttack <<= 1;
                do {
                        L0389_B_KilledAllCreatures = (L0387_i_Outcome = F0190_GROUP_GetDamageCreatureOutcome(P0375_ps_Group, L0386_i_CreatureIndex, P0376_i_MapX, P0377_i_MapY, P0378_i_Attack + M002_RANDOM(L0385_ui_RandomAttack), P0379_B_NotMoving)) && L0389_B_KilledAllCreatures;
                        L0388_B_KilledSomeCreatures = L0388_B_KilledSomeCreatures || L0387_i_Outcome;
                } while (L0386_i_CreatureIndex--);
                if (L0389_B_KilledAllCreatures) {
                        return C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP;
                }
                else {
                        if (L0388_B_KilledSomeCreatures) {
                                return C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP;
                        }
                        else {
                                return C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
                        }
                }
        } else {
                return C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
        }
}

int16_t F0192_GROUP_GetResistanceAdjustedPoisonAttack(
unsigned int16_t P0380_ui_CreatureType SEPARATOR
unsigned int16_t P0381_i_PoisonAttack  FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0390_i_PoisonResistance;


        if (!P0381_i_PoisonAttack || ((L0390_i_PoisonResistance = M061_POISON_RESISTANCE(G0243_as_Graphic559_CreatureInfo[P0380_ui_CreatureType].Resistances)) == C15_IMMUNE_TO_POISON)) {
                return 0;
        }
        else {
                return ((P0381_i_PoisonAttack + M004_RANDOM(4)) << 3) / ++L0390_i_PoisonResistance;
        }
}

STATICFUNCTION void F0193_GROUP_StealFromChampion(
REGISTER GROUP*  P0382_ps_Group         SEPARATOR
unsigned int16_t P0383_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0395_ps_Champion;
        REGISTER int16_t L0391_i_Percentage;
        REGISTER unsigned int16_t L0392_ui_StealFromSlotIndex;
        REGISTER unsigned int16_t L0393_ui_Counter;
        REGISTER THING L0394_T_Thing;
        REGISTER BOOLEAN L0396_B_ObjectStolen;


        L0396_B_ObjectStolen = C0_FALSE;
        L0391_i_Percentage = 100 - F0311_CHAMPION_GetDexterity(L0395_ps_Champion = &M516_CHAMPIONS[P0383_ui_ChampionIndex]);
        L0393_ui_Counter = M003_RANDOM(8);
        while ((L0391_i_Percentage > 0) && !F0308_CHAMPION_IsLucky(L0395_ps_Champion, L0391_i_Percentage)) {
                L0392_ui_StealFromSlotIndex = G0025_auc_Graphic562_StealFromSlotIndices[L0393_ui_Counter];
                if (((L0394_T_Thing = L0395_ps_Champion->Slots[L0392_ui_StealFromSlotIndex]) != C0xFFFF_THING_NONE)) {
                        L0396_B_ObjectStolen = C1_TRUE;
                        L0394_T_Thing = F0300_CHAMPION_GetObjectRemovedFromSlot(P0383_ui_ChampionIndex, L0392_ui_StealFromSlotIndex);
                        if (P0382_ps_Group->Slot == C0xFFFE_THING_ENDOFLIST) {
                                P0382_ps_Group->Slot = L0394_T_Thing; /* BUG0_12 An object is cloned and appears at two different locations in the dungeon and/or inventory. The game may crash when interacting with this object. If a Giggler with no possessions steals an object that was previously in a chest and was not the last object in the chest then the objects that followed it are cloned. In the chest, the object is part of a linked list of objects that is not reset when the object is removed from the chest and placed in the inventory (but not in the dungeon), nor when it is stolen and added as the first Giggler possession. If the Giggler already has a possession before stealing the object then this does not create a cloned object.
                                The following statement is missing: L0394_T_Thing->Next = C0xFFFE_THING_ENDOFLIST;
                                This creates cloned things if L0394_T_Thing->Next is not C0xFFFE_THING_ENDOFLIST which is the case when the object comes from a chest in which it was not the last object */
                        } else {
                                F0163_DUNGEON_LinkThingToList(L0394_T_Thing, P0382_ps_Group->Slot, CM1_MAPX_NOT_ON_A_SQUARE, 0);
                        }
                        F0292_CHAMPION_DrawState(P0383_ui_ChampionIndex);
                }
                ++L0393_ui_Counter;
                L0393_ui_Counter &= 0x0007;
                L0391_i_Percentage -= 20;
        }
        if (!M003_RANDOM(8) || (L0396_B_ObjectStolen && M005_RANDOM(2))) {
                G0375_ps_ActiveGroups[P0382_ps_Group->ActiveGroupIndex].DelayFleeingFromTarget = M003_RANDOM(64) + 20;
                P0382_ps_Group->Behavior = C5_BEHAVIOR_FLEE;
        }
}

void F0194_GROUP_RemoveAllActiveGroups(
void
)
{
        REGISTER unsigned int16_t L0397_ui_ActiveGroupIndex;


        for (L0397_ui_ActiveGroupIndex = 0; G0377_ui_CurrentActiveGroupCount > 0; L0397_ui_ActiveGroupIndex++) {
                if (G0375_ps_ActiveGroups[L0397_ui_ActiveGroupIndex].GroupThingIndex >= 0) {
                        F0184_GROUP_RemoveActiveGroup(L0397_ui_ActiveGroupIndex);
                }
        }
}

void F0195_GROUP_AddAllActiveGroups(
void
)
{
        REGISTER unsigned char* L0401_puc_Square;
        REGISTER THING* L0402_pT_SquareFirstThing;
        REGISTER THING L0400_T_Thing;
        REGISTER unsigned int16_t L0398_ui_MapX;
        REGISTER unsigned int16_t L0399_ui_MapY;


        L0401_puc_Square = G0271_ppuc_CurrentMapData[0];
        L0402_pT_SquareFirstThing = &G0283_pT_SquareFirstThings[G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount[0]];
        for (L0398_ui_MapX = 0; L0398_ui_MapX < G0273_i_CurrentMapWidth; L0398_ui_MapX++) {
                for (L0399_ui_MapY = 0; L0399_ui_MapY < G0274_i_CurrentMapHeight; L0399_ui_MapY++) {
                        if (M007_GET(*L0401_puc_Square++, MASK0x0010_THING_LIST_PRESENT)) {
                                L0400_T_Thing = *L0402_pT_SquareFirstThing++;
                                do {
                                        if (M012_TYPE(L0400_T_Thing) == C04_THING_TYPE_GROUP) {
                                                F0181_GROUP_DeleteEvents(L0398_ui_MapX, L0399_ui_MapY);
                                                F0183_GROUP_AddActiveGroup(L0400_T_Thing, L0398_ui_MapX, L0399_ui_MapY);
                                                F0180_GROUP_StartWandering(L0398_ui_MapX, L0399_ui_MapY);
                                                break;
                                        }
                                } while ((L0400_T_Thing = F0159_DUNGEON_GetNextThing(L0400_T_Thing)) != C0xFFFE_THING_ENDOFLIST);
                        }
                }
        }
}

void F0196_GROUP_InitializeActiveGroups(
void
)
{
        REGISTER unsigned int16_t L0403_ui_ActiveGroupIndex;
        REGISTER unsigned int16_t L1875_ui_MaximumActiveGroupCount;


        if (G0298_B_NewGame) {
                G0376_ui_MaximumActiveGroupCount = 110;
        }
        L1875_ui_MaximumActiveGroupCount = F0025_MAIN_GetMaximumValue(G0376_ui_MaximumActiveGroupCount, 110);
        G0375_ps_ActiveGroups = (ACTIVE_GROUP*)M533_F0468_MEMORY_Allocate((long)(L1875_ui_MaximumActiveGroupCount * sizeof(ACTIVE_GROUP)), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
        for (L0403_ui_ActiveGroupIndex = 0; L0403_ui_ActiveGroupIndex < L1875_ui_MaximumActiveGroupCount; ) {
                G0375_ps_ActiveGroups[L0403_ui_ActiveGroupIndex].GroupThingIndex = -1;
                L0403_ui_ActiveGroupIndex++;
        }
}

/* Returns 0 if at least one square on the path from source to destination is blocked else returns the distance between the squares */
STATICFUNCTION BOOLEAN F0197_GROUP_IsViewPartyBlocked(
unsigned int16_t P0384_ui_MapX SEPARATOR
unsigned int16_t P0385_ui_MapY FINAL_SEPARATOR
{
        REGISTER DOOR* L0407_ps_Door;
        REGISTER unsigned int16_t L0404_ui_Square;
        REGISTER int16_t L0405_i_SquareType;
        REGISTER int16_t L0406_i_DoorState;


        if ((L0405_i_SquareType = M034_SQUARE_TYPE(L0404_ui_Square = G0271_ppuc_CurrentMapData[P0384_ui_MapX][P0385_ui_MapY])) == C04_ELEMENT_DOOR) {
                L0407_ps_Door = (DOOR*)F0157_DUNGEON_GetSquareFirstThingData(P0384_ui_MapX, P0385_ui_MapY);
                return (((L0406_i_DoorState = M036_DOOR_STATE(L0404_ui_Square)) == C3_DOOR_STATE_CLOSED_THREE_FOURTH) || (L0406_i_DoorState == C4_DOOR_STATE_CLOSED)) && !M007_GET(G0275_as_CurrentMapDoorInfo[L0407_ps_Door->Type].Attributes, MASK0x0001_CREATURES_CAN_SEE_THROUGH);
        }
                else {
        return (L0405_i_SquareType == C00_ELEMENT_WALL) || ((L0405_i_SquareType == C06_ELEMENT_FAKEWALL) && !M007_GET(L0404_ui_Square, MASK0x0004_FAKEWALL_OPEN));
                }
}

STATICFUNCTION BOOLEAN F0198_GROUP_IsSmellPartyBlocked(
unsigned int16_t P0386_ui_MapX SEPARATOR
unsigned int16_t P0387_ui_MapY FINAL_SEPARATOR
{
        REGISTER int16_t L0409_i_SquareType;
        REGISTER unsigned int16_t L0408_ui_Square;


        return ((L0409_i_SquareType = M034_SQUARE_TYPE(L0408_ui_Square = G0271_ppuc_CurrentMapData[P0386_ui_MapX][P0387_ui_MapY])) == C00_ELEMENT_WALL) || ((L0409_i_SquareType == C06_ELEMENT_FAKEWALL) && !M007_GET(L0408_ui_Square, MASK0x0004_FAKEWALL_OPEN | MASK0x0001_FAKEWALL_IMAGINARY));
}

/* Returns 0 if at least one square on the path from source to destination is blocked else returns the distance between the squares */
STATICFUNCTION int16_t F0199_GROUP_GetDistanceBetweenUnblockedSquares(
int16_t            P0388_i_SourceMapX                     SEPARATOR
int16_t            P0389_i_SourceMapY                     SEPARATOR
int16_t            P0390_i_DestinationMapX                SEPARATOR
int16_t            P0391_i_DestinationMapY                SEPARATOR
REGISTER BOOLEAN (*P0392_pfB_IsBlockedCallbackFunction)(
int, int
) FINAL_SEPARATOR
{
        REGISTER int16_t L0414_i_LargestAxisDistance;
        REGISTER int16_t L0412_i_Multiple;
#define AL0412_i_DistanceX L0412_i_Multiple
#define AL0412_i_PathMapX  L0412_i_Multiple
        REGISTER int16_t L0413_i_Multiple;
#define AL0413_i_DistanceY L0413_i_Multiple
#define AL0413_i_PathMapY  L0413_i_Multiple
        REGISTER int16_t L0410_i_XAxisStep;
        REGISTER int16_t L0411_i_YAxisStep;
        BOOLEAN L0415_B_DistanceXSmallerThanDistanceY;
        int16_t L0416_i_ValueA;
        int16_t L0417_i_ValueB;
        BOOLEAN L0418_B_DistanceXEqualsDistanceY;
        int16_t L0419_i_ValueC;


        if (M038_DISTANCE(P0388_i_SourceMapX, P0389_i_SourceMapY, P0390_i_DestinationMapX, P0391_i_DestinationMapY) <= 1) {
                return 1;
        }
        L0415_B_DistanceXSmallerThanDistanceY = (AL0412_i_DistanceX = ((AL0412_i_DistanceX = P0390_i_DestinationMapX - P0388_i_SourceMapX) < 0) ? -AL0412_i_DistanceX : AL0412_i_DistanceX) < (AL0413_i_DistanceY = ((AL0413_i_DistanceY = P0391_i_DestinationMapY - P0389_i_SourceMapY) < 0) ? -AL0413_i_DistanceY : AL0413_i_DistanceY);
        L0418_B_DistanceXEqualsDistanceY = (AL0412_i_DistanceX == AL0413_i_DistanceY);
        L0410_i_XAxisStep = (((AL0412_i_PathMapX = P0390_i_DestinationMapX) - P0388_i_SourceMapX) > 0) ? -1 : 1;
        L0411_i_YAxisStep = (((AL0413_i_PathMapY = P0391_i_DestinationMapY) - P0389_i_SourceMapY) > 0) ? -1 : 1;
        L0419_i_ValueC = L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY - P0389_i_SourceMapY) ? ((AL0412_i_PathMapX - P0388_i_SourceMapX) << 6) / L0414_i_LargestAxisDistance : 0x80)
                                                               : ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX - P0388_i_SourceMapX) ? ((AL0413_i_PathMapY - P0389_i_SourceMapY) << 6) / L0414_i_LargestAxisDistance : 0x80);
        /* 0x80 = 128 when the creature is on the same row or column as the party */
        do {
                if (L0418_B_DistanceXEqualsDistanceY) {
                        if (((*P0392_pfB_IsBlockedCallbackFunction)(AL0412_i_PathMapX + L0410_i_XAxisStep, AL0413_i_PathMapY) && (*P0392_pfB_IsBlockedCallbackFunction)(AL0412_i_PathMapX, AL0413_i_PathMapY + L0411_i_YAxisStep)) || (*P0392_pfB_IsBlockedCallbackFunction)(AL0412_i_PathMapX += L0410_i_XAxisStep, AL0413_i_PathMapY += L0411_i_YAxisStep)) {
                                return 0;
                        }
                } else {
                        if ((L0416_i_ValueA = ((L0414_i_LargestAxisDistance = (L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY - P0389_i_SourceMapY) ? ((AL0412_i_PathMapX + L0410_i_XAxisStep - P0388_i_SourceMapX) << 6) / L0414_i_LargestAxisDistance : 0x80) : ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX + L0410_i_XAxisStep - P0388_i_SourceMapX) ? ((AL0413_i_PathMapY - P0389_i_SourceMapY) << 6) / L0414_i_LargestAxisDistance : 0x80)) - L0419_i_ValueC) < 0) ? -L0414_i_LargestAxisDistance : L0414_i_LargestAxisDistance) < (L0417_i_ValueB = ((L0414_i_LargestAxisDistance = (L0415_B_DistanceXSmallerThanDistanceY ? ((L0414_i_LargestAxisDistance = AL0413_i_PathMapY + L0411_i_YAxisStep - P0389_i_SourceMapY) ? ((AL0412_i_PathMapX - P0388_i_SourceMapX) << 6) / L0414_i_LargestAxisDistance : 0x80) : ((L0414_i_LargestAxisDistance = AL0412_i_PathMapX - P0388_i_SourceMapX) ? ((AL0413_i_PathMapY + L0411_i_YAxisStep - P0389_i_SourceMapY) << 6) / L0414_i_LargestAxisDistance : 0x80)) - L0419_i_ValueC) < 0) ? -L0414_i_LargestAxisDistance : L0414_i_LargestAxisDistance)) {
                                AL0412_i_PathMapX += L0410_i_XAxisStep;
                        } else {
                                AL0413_i_PathMapY += L0411_i_YAxisStep;
                        }
                        if ((*P0392_pfB_IsBlockedCallbackFunction)(AL0412_i_PathMapX, AL0413_i_PathMapY) && ((L0416_i_ValueA != L0417_i_ValueB) || (*P0392_pfB_IsBlockedCallbackFunction)(AL0412_i_PathMapX += L0410_i_XAxisStep, AL0413_i_PathMapY -= L0411_i_YAxisStep))) {
                                return 0;
                        }
                }
        } while (M038_DISTANCE(AL0412_i_PathMapX, AL0413_i_PathMapY, P0388_i_SourceMapX, P0389_i_SourceMapY) > 1);
        return F0226_GROUP_GetDistanceBetweenSquares(P0388_i_SourceMapX, P0389_i_SourceMapY, P0390_i_DestinationMapX, P0391_i_DestinationMapY);
}

STATICFUNCTION int16_t F0200_GROUP_GetDistanceToVisibleParty(
REGISTER GROUP*  P0393_ps_Group        SEPARATOR
REGISTER int16_t P0394_i_CreatureIndex SEPARATOR
int16_t          P0395_i_MapX          SEPARATOR
int16_t          P0396_i_MapY          FINAL_SEPARATOR
{
        REGISTER CREATURE_INFO* L0424_ps_CreatureInfo;
        REGISTER int16_t L0421_i_CreatureViewDirectionCount; /* Count of directions to test in L0425_ai_CreatureViewDirections */
        REGISTER int16_t L0422_i_Multiple;
#define AL0422_i_Counter    L0422_i_Multiple
#define AL0422_i_SightRange L0422_i_Multiple
      REGISTER int16_t L0420_i_CreatureDirection;
        REGISTER unsigned int16_t L0423_ui_GroupDirections;
        int16_t L0425_ai_CreatureViewDirections[4]; /* List of directions to test */


        L0424_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[P0393_ps_Group->Type];
        if (M007_GET(L0424_ps_CreatureInfo->Attributes, MASK0x0004_SIDE_ATTACK)) /* If creature can see in all directions */
                goto T0200011;
        else {
                L0423_ui_GroupDirections = G0375_ps_ActiveGroups[P0393_ps_Group->ActiveGroupIndex].Directions;
                if (P0394_i_CreatureIndex < 0) { /* Negative index means test if each creature in the group can see the party in their respective direction */
                        L0421_i_CreatureViewDirectionCount = 0;
                        for (P0394_i_CreatureIndex = P0393_ps_Group->Count; P0394_i_CreatureIndex >= 0; P0394_i_CreatureIndex--) {
                                L0420_i_CreatureDirection = M021_NORMALIZE(L0423_ui_GroupDirections >> (P0394_i_CreatureIndex << 1));
                                AL0422_i_Counter = L0421_i_CreatureViewDirectionCount;
                                while (AL0422_i_Counter--) {
                                        if (L0425_ai_CreatureViewDirections[AL0422_i_Counter] == L0420_i_CreatureDirection) /* If the creature looks in the same direction as another one in the group */
                                                goto T0200006;
                                }
                                L0425_ai_CreatureViewDirections[L0421_i_CreatureViewDirectionCount++] = L0420_i_CreatureDirection;
                                T0200006: ;
                        }
                } else { /* Positive index means test only if the specified creature in the group can see the party in its direction */
                        L0425_ai_CreatureViewDirections[0] = M050_CREATURE_VALUE(L0423_ui_GroupDirections, P0394_i_CreatureIndex);
                        L0421_i_CreatureViewDirectionCount = 1;
                }
        }
        while (L0421_i_CreatureViewDirectionCount--) {
                if (F0227_GROUP_IsDestinationVisibleFromSource(L0425_ai_CreatureViewDirections[L0421_i_CreatureViewDirectionCount], P0395_i_MapX, P0396_i_MapY, G0306_i_PartyMapX, G0307_i_PartyMapY)) {
                        T0200011:
                        AL0422_i_SightRange = M054_SIGHT_RANGE(L0424_ps_CreatureInfo->Ranges);
                        if (G0407_s_Party.Event71Count_Invisibility && !M007_GET(L0424_ps_CreatureInfo->Attributes, MASK0x0800_SEE_INVISIBLE)) {
                                AL0422_i_SightRange = -10;
                        } else {
                                if (!M007_GET(L0424_ps_CreatureInfo->Attributes, MASK0x1000_NIGHT_VISION)) {
                                        AL0422_i_SightRange -= G0304_i_DungeonViewPaletteIndex >> 1;
                                }
                        }
                        if (G0381_ui_CurrentGroupDistanceToParty > AL0422_i_SightRange) {
                                if (G0381_ui_CurrentGroupDistanceToParty == 1) {
                                        AL0422_i_SightRange += M002_RANDOM(M102_XXX_RANGE(L0424_ps_CreatureInfo->Ranges) + 1) + M002_RANDOM(M055_SMELL_RANGE(L0424_ps_CreatureInfo->Ranges) + 1);
                                        if (!M003_RANDOM(8)) {
                                                AL0422_i_SightRange += M005_RANDOM(1) + 5;
                                        }
                                }
                                if (G0381_ui_CurrentGroupDistanceToParty > AL0422_i_SightRange + M003_RANDOM(8) - 3) {
                                        return 0;
                                }
                        }
                        return F0199_GROUP_GetDistanceBetweenUnblockedSquares(P0395_i_MapX, P0396_i_MapY, G0306_i_PartyMapX, G0307_i_PartyMapY, (BOOLEAN (*)(int, int))F0197_GROUP_IsViewPartyBlocked);
                }
        }
        return 0;
}

STATICFUNCTION int16_t F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal(
CREATURE_INFO* P0397_ps_CreatureInfo SEPARATOR
int16_t        P0398_i_MapY          SEPARATOR
int16_t        P0399_i_MapX          FINAL_SEPARATOR
{
        REGISTER int16_t L0427_i_ScentOrdinal;
        REGISTER unsigned int16_t L0426_ui_SmellRange;


        if (!(L0426_ui_SmellRange = M055_SMELL_RANGE(P0397_ps_CreatureInfo->Ranges))) {
                return 0;
        }
        if ((((L0426_ui_SmellRange + 1) >> 1) >= G0381_ui_CurrentGroupDistanceToParty) && F0199_GROUP_GetDistanceBetweenUnblockedSquares(P0398_i_MapY, P0399_i_MapX, G0306_i_PartyMapX, G0307_i_PartyMapY, (BOOLEAN (*)(int, int))F0198_GROUP_IsSmellPartyBlocked)) {
                G0363_i_SecondaryDirectionToOrFromParty = G0383_i_CurrentGroupSecondaryDirectionToParty;
                return M000_INDEX_TO_ORDINAL(G0382_i_CurrentGroupPrimaryDirectionToParty);
        }
        if ((L0427_i_ScentOrdinal = F0315_CHAMPION_GetScentOrdinal(P0398_i_MapY, P0399_i_MapX)) && ((G0407_s_Party.ScentStrengths[M001_ORDINAL_TO_INDEX(L0427_i_ScentOrdinal)] + M004_RANDOM(4)) > (30 - (L0426_ui_SmellRange << 1)))) { /* If there is a fresh enough party scent on the group square */
                return M000_INDEX_TO_ORDINAL(F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0398_i_MapY, P0399_i_MapX, G0407_s_Party.Scents[L0427_i_ScentOrdinal].Location.MapX, G0407_s_Party.Scents[L0427_i_ScentOrdinal].Location.MapY));
        }
        return 0;
}

STATICFUNCTION BOOLEAN F0202_GROUP_IsMovementPossible(
REGISTER CREATURE_INFO* P0400_ps_CreatureInfo                              SEPARATOR
int16_t                 P0401_i_MapX                                       SEPARATOR
int16_t                 P0402_i_MapY                                       SEPARATOR
unsigned int16_t        P0403_ui_Direction                                 SEPARATOR
BOOLEAN                 P0404_B_AllowMovementOverImaginaryPitsAndFakeWalls FINAL_SEPARATOR
{
        REGISTER TELEPORTER* L0432_ps_Teleporter;
        REGISTER unsigned int16_t L0430_ui_Square;
        REGISTER int16_t L0428_i_MapX;
        REGISTER int16_t L0429_i_MapY;
        REGISTER int16_t L0431_i_SquareType;
        REGISTER THING L0433_T_Thing;


        G0384_auc_GroupMovementTestedDirections[P0403_ui_Direction] = C1_TRUE;
        G0388_T_GroupMovementBlockedByGroupThing = C0xFFFE_THING_ENDOFLIST;
        G0389_B_GroupMovementBlockedByDoor = C0_FALSE;
        G0390_B_GroupMovementBlockedByParty = C0_FALSE;
        if (P0400_ps_CreatureInfo->MovementTicks == C255_IMMOBILE) {
                return C0_FALSE;
        }
        F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0403_ui_Direction, 1, 0, &P0401_i_MapX, &P0402_i_MapY);
        L0428_i_MapX = P0401_i_MapX;
        L0429_i_MapY = P0402_i_MapY;
        if (G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter =
                !(((L0428_i_MapX >= 0) && (L0428_i_MapX < G0273_i_CurrentMapWidth)) &&
                ((L0429_i_MapY >= 0) && (L0429_i_MapY < G0274_i_CurrentMapHeight)) &&
                ((L0431_i_SquareType = M034_SQUARE_TYPE(L0430_ui_Square = G0271_ppuc_CurrentMapData[L0428_i_MapX][L0429_i_MapY])) != C00_ELEMENT_WALL) &&
                (L0431_i_SquareType != C03_ELEMENT_STAIRS) &&
                ((L0431_i_SquareType != C02_ELEMENT_PIT) || (M007_GET(L0430_ui_Square, MASK0x0001_PIT_IMAGINARY) && P0404_B_AllowMovementOverImaginaryPitsAndFakeWalls) || !M007_GET(L0430_ui_Square, MASK0x0008_PIT_OPEN) || M007_GET(P0400_ps_CreatureInfo->Attributes, MASK0x0020_LEVITATION)) &&
                ((L0431_i_SquareType != C06_ELEMENT_FAKEWALL) || M007_GET(L0430_ui_Square, MASK0x0004_FAKEWALL_OPEN) || (M007_GET(L0430_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY) && P0404_B_AllowMovementOverImaginaryPitsAndFakeWalls)))) {
                return C0_FALSE;
        }
        if (M007_GET(P0400_ps_CreatureInfo->Attributes, MASK0x2000_ARCHENEMY)) {
                L0433_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0428_i_MapX, L0429_i_MapY);
                while (L0433_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                        if (M012_TYPE(L0433_T_Thing) == C15_THING_TYPE_EXPLOSION) {
                                L0432_ps_Teleporter = (TELEPORTER*)F0156_DUNGEON_GetThingData(L0433_T_Thing);
                                if (((EXPLOSION*)L0432_ps_Teleporter)->Type == C050_EXPLOSION_FLUXCAGE) {
                                        G0385_ac_FluxCages[P0403_ui_Direction] = C1_TRUE;
                                        G0386_ui_FluxCageCount++;
                                        G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter = C1_TRUE;
                                        return C0_FALSE;
                                }
                        }
                        L0433_T_Thing = F0159_DUNGEON_GetNextThing(L0433_T_Thing);
                }
        }
        if ((L0431_i_SquareType == C05_ELEMENT_TELEPORTER) && M007_GET(L0430_ui_Square, MASK0x0008_TELEPORTER_OPEN) && (M059_WARINESS(P0400_ps_CreatureInfo->Properties) >= 10)) {
                L0432_ps_Teleporter = (TELEPORTER*)F0157_DUNGEON_GetSquareFirstThingData(L0428_i_MapX, L0429_i_MapY);
                if (M007_GET(L0432_ps_Teleporter->Scope, MASK0x0001_SCOPE_CREATURES) && !F0139_DUNGEON_IsCreatureAllowedOnMap(G0380_T_CurrentGroupThing, L0432_ps_Teleporter->TargetMapIndex)) {
                        G0387_B_GroupMovementBlockedByWallStairsPitFakeWallFluxcageTeleporter = C1_TRUE;
                        return C0_FALSE; /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE */
                }
        }
        if (G0390_B_GroupMovementBlockedByParty = (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0428_i_MapX == G0306_i_PartyMapX) && (L0429_i_MapY == G0307_i_PartyMapY)) {
                return C0_FALSE;
        }
        if (L0431_i_SquareType == C04_ELEMENT_DOOR) {
                L0432_ps_Teleporter = (TELEPORTER*)F0157_DUNGEON_GetSquareFirstThingData(L0428_i_MapX, L0429_i_MapY);
                if (!(((M036_DOOR_STATE(L0430_ui_Square)) <= (((DOOR*)L0432_ps_Teleporter)->Vertical ? M051_CREATURE_HEIGHT(P0400_ps_CreatureInfo->Attributes) : 1)) || ((M036_DOOR_STATE(L0430_ui_Square)) == C5_DOOR_STATE_DESTROYED) || M007_GET(P0400_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL))) {
                        G0389_B_GroupMovementBlockedByDoor = C1_TRUE;
                        return C0_FALSE;
                }
        }
        return (G0388_T_GroupMovementBlockedByGroupThing = F0175_GROUP_GetThing(L0428_i_MapX, L0429_i_MapY)) == C0xFFFE_THING_ENDOFLIST;
}

STATICFUNCTION int16_t F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal(
CREATURE_INFO* P0405_ps_CreatureInfo                              SEPARATOR
int16_t        P0406_i_MapX                                       SEPARATOR
int16_t        P0407_i_MapY                                       SEPARATOR
BOOLEAN        P0408_B_AllowMovementOverImaginaryPitsAndFakeWalls FINAL_SEPARATOR
{
        REGISTER int16_t L0434_i_Direction;


        for (L0434_i_Direction = C0_DIRECTION_NORTH; L0434_i_Direction <= C3_DIRECTION_WEST; L0434_i_Direction++) {
                if ((!G0384_auc_GroupMovementTestedDirections[L0434_i_Direction]) && F0202_GROUP_IsMovementPossible(P0405_ps_CreatureInfo, P0406_i_MapX, P0407_i_MapY, L0434_i_Direction, P0408_B_AllowMovementOverImaginaryPitsAndFakeWalls)) {
                        return M000_INDEX_TO_ORDINAL(L0434_i_Direction);
                }
        }
        return 0;
}

STATICFUNCTION BOOLEAN F0204_GROUP_IsArchenemyDoubleMovementPossible(
CREATURE_INFO*            P0409_ps_CreatureInfo SEPARATOR
int16_t                   P0410_i_MapX          SEPARATOR
int16_t                   P0411_i_MapY          SEPARATOR
REGISTER unsigned int16_t P0412_ui_Direction    FINAL_SEPARATOR
{
        if (G0385_ac_FluxCages[P0412_ui_Direction]) {
                return C0_FALSE;
        }
        P0410_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0412_ui_Direction], P0411_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0412_ui_Direction];
        return F0202_GROUP_IsMovementPossible(P0409_ps_CreatureInfo, P0410_i_MapX, P0411_i_MapY, P0412_ui_Direction, C0_FALSE);
}

STATICFUNCTION void F0205_GROUP_SetDirection(
REGISTER ACTIVE_GROUP*    P0413_ps_ActiveGroup                SEPARATOR
REGISTER unsigned int16_t P0414_ui_Direction                  SEPARATOR
REGISTER unsigned int16_t P0415_ui_CreatureIndex              SEPARATOR
BOOLEAN                   P0416_B_TwoHalfSquareSizedCreatures FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0435_ui_GroupDirections;
        static long G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime; /* These two variables are used to prevent setting direction of half square sized creatures twice at the same game time */
        static ACTIVE_GROUP* G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup;


        if (P0416_B_TwoHalfSquareSizedCreatures && (G0313_ul_GameTime == G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime) && (P0413_ps_ActiveGroup == G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup)) {
                return;
        }
        if (M021_NORMALIZE((int16_t)M050_CREATURE_VALUE(L0435_ui_GroupDirections = P0413_ps_ActiveGroup->Directions, P0415_ui_CreatureIndex) - P0414_ui_Direction) == 2) { /* If current and new direction are opposites then change direction only one step at a time */
                L0435_ui_GroupDirections = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, P0415_ui_CreatureIndex, P0414_ui_Direction = M017_NEXT((M006_RANDOM(65536) & 0x0002) + P0414_ui_Direction));
        } else {
                L0435_ui_GroupDirections = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, P0415_ui_CreatureIndex, P0414_ui_Direction);
        }
        if (P0416_B_TwoHalfSquareSizedCreatures) {
                L0435_ui_GroupDirections = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0435_ui_GroupDirections, P0415_ui_CreatureIndex ^ 1, P0414_ui_Direction); /* Set direction of the second half square sized creature */
                G0395_l_TwoHalfSquareSizedCreaturesGroupLastDirectionSetTime = G0313_ul_GameTime;
                G0396_ps_TwoHalfSquareSizedCreaturesGroupLastDirectionSetActiveGroup = P0413_ps_ActiveGroup;
        }
        P0413_ps_ActiveGroup->Directions = L0435_ui_GroupDirections;
}

/* Sets direction of at least one creature in the group */
STATICFUNCTION void F0206_GROUP_SetDirectionGroup(
ACTIVE_GROUP*    P0417_ps_ActiveGroup  SEPARATOR
int16_t          P0418_i_Direction     SEPARATOR
REGISTER int16_t P0419_i_CreatureIndex SEPARATOR
int16_t          P0420_i_CreatureSize  FINAL_SEPARATOR
{
        REGISTER BOOLEAN L0436_B_TwoHalfSquareSizedCreatures;


        if (L0436_B_TwoHalfSquareSizedCreatures = P0419_i_CreatureIndex && (P0420_i_CreatureSize == C1_SIZE_HALF_SQUARE)) {
                P0419_i_CreatureIndex--;
        }
        do {
                if (!P0419_i_CreatureIndex || M005_RANDOM(2)) {
                        F0205_GROUP_SetDirection(P0417_ps_ActiveGroup, P0418_i_Direction, P0419_i_CreatureIndex, L0436_B_TwoHalfSquareSizedCreatures);
                }
        } while (P0419_i_CreatureIndex--);
}

STATICFUNCTION BOOLEAN F0207_GROUP_IsCreatureAttacking(
REGISTER GROUP*  P0421_ps_Group         SEPARATOR
REGISTER int16_t P0422_i_MapX           SEPARATOR
int16_t          P0423_i_MapY           SEPARATOR
unsigned int16_t P0424_ui_CreatureIndex FINAL_SEPARATOR
{
        REGISTER CREATURE_INFO* L0441_ps_CreatureInfo;
        REGISTER CHAMPION* L0442_ps_Champion;
#define AL0440_i_KineticEnergy      L0440_i_Multiple
#define AL0440_i_Counter            L0440_i_Multiple
#define AL0440_i_Damage             L0440_i_Multiple
#define AL0440_i_AttackSoundOrdinal L0440_i_Multiple
        REGISTER unsigned int16_t L0437_ui_Multiple;
#define AL0437_ui_CreatureType L0437_ui_Multiple
#define AL0437_T_Thing         L0437_ui_Multiple
        REGISTER int16_t L0439_i_Multiple;
#define AL0439_i_GroupCells    L0439_i_Multiple
#define AL0439_i_TargetCell    L0439_i_Multiple
#define AL0439_i_ChampionIndex L0439_i_Multiple
        REGISTER int16_t L0440_i_Multiple;
        REGISTER unsigned int16_t L0438_ui_PrimaryDirectionToParty;
        ACTIVE_GROUP L0443_s_ActiveGroup;
        BOOLEAN L1876_B_UseSpellSoundAsFallback;


        L1876_B_UseSpellSoundAsFallback = C0_FALSE;
        G0361_l_LastCreatureAttackTime = G0313_ul_GameTime;
        L0443_s_ActiveGroup = G0375_ps_ActiveGroups[P0421_ps_Group->ActiveGroupIndex];
        L0441_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[AL0437_ui_CreatureType = P0421_ps_Group->Type];
        L0438_ui_PrimaryDirectionToParty = G0382_i_CurrentGroupPrimaryDirectionToParty;
        if ((AL0439_i_GroupCells = L0443_s_ActiveGroup.Cells) == C0xFF_SINGLE_CENTERED_CREATURE) {
                AL0439_i_TargetCell = M005_RANDOM(2);
        } else {
                AL0439_i_TargetCell = ((M050_CREATURE_VALUE(AL0439_i_GroupCells, P0424_ui_CreatureIndex) + 5 - L0438_ui_PrimaryDirectionToParty) & 0x0002) >> 1;
        }
        AL0439_i_TargetCell += L0438_ui_PrimaryDirectionToParty;
        AL0439_i_TargetCell &= 0x0003;
        if ((M056_ATTACK_RANGE(L0441_ps_CreatureInfo->Ranges) > 1) && ((G0381_ui_CurrentGroupDistanceToParty > 1) || M005_RANDOM(2))) {
                L1876_B_UseSpellSoundAsFallback = C1_TRUE;
                switch (AL0437_ui_CreatureType) {
                        default:
                                AL0437_T_Thing = C0xFF80_THING_EXPLOSION_FIREBALL;
                                break;
                        case C14_CREATURE_VEXIRK:
                        case C23_CREATURE_LORD_CHAOS:
                                if (M005_RANDOM(2)) {
                                        AL0437_T_Thing = C0xFF80_THING_EXPLOSION_FIREBALL;
                                } else {
                                        switch (M004_RANDOM(4)) {
                                                case 0:
                                                        AL0437_T_Thing = C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL;
                                                        break;
                                                case 1:
                                                        AL0437_T_Thing = C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT;
                                                        break;
                                                case 2:
                                                        AL0437_T_Thing = C0xFF87_THING_EXPLOSION_POISON_CLOUD;
                                                        break;
                                                case 3:
#ifdef PC_FIX_CODE_SIZE
        L0439_i_Multiple++;
        L0439_i_Multiple++;
        L0439_i_Multiple++;
#endif
                                                        AL0437_T_Thing = C0xFF84_THING_EXPLOSION_OPEN_DOOR;
                                        }
                                }
                                break;
                        case C01_CREATURE_SWAMP_SLIME_SLIME_DEVIL:
                                L1876_B_UseSpellSoundAsFallback = C0_FALSE;
                                AL0437_T_Thing = C0xFF81_THING_EXPLOSION_SLIME;
                                break;
                        case C03_CREATURE_WIZARD_EYE_FLYING_EYE:
                                if (M003_RANDOM(8)) {
                                        AL0437_T_Thing = C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT;
                                } else {
                                        AL0437_T_Thing = C0xFF84_THING_EXPLOSION_OPEN_DOOR;
                                }
                                break;
                        case C19_CREATURE_MATERIALIZER_ZYTAZ:
                                if (M005_RANDOM(2)) {
                                        AL0437_T_Thing = C0xFF87_THING_EXPLOSION_POISON_CLOUD;
                                        break;
                                }
                        case C22_CREATURE_DEMON:
                        case C24_CREATURE_RED_DRAGON:
                                AL0437_T_Thing = C0xFF80_THING_EXPLOSION_FIREBALL;
                } /* BUG0_13 The game may crash when 'Lord Order' or 'Grey Lord' cast spells. This cannot happen with the original dungeons as they do not contain any groups of these types. 'Lord Order' and 'Grey Lord' creatures can cast spells (attack range > 1) but no projectile type is defined for them in the code. If these creatures are present in a dungeon they will cast projectiles containing undefined things because the variable is not initialized */
                AL0440_i_KineticEnergy = (L0441_ps_CreatureInfo->Attack >> 2) + 1;
                AL0440_i_KineticEnergy += M002_RANDOM(AL0440_i_KineticEnergy);
                AL0440_i_KineticEnergy += M002_RANDOM(AL0440_i_KineticEnergy);
                F0212_PROJECTILE_Create(AL0437_T_Thing, P0422_i_MapX, P0423_i_MapY, AL0439_i_TargetCell, G0382_i_CurrentGroupPrimaryDirectionToParty, F0026_MAIN_GetBoundedValue(20, AL0440_i_KineticEnergy, 255), L0441_ps_CreatureInfo->Dexterity, 8);
        } else {
                if (M007_GET(L0441_ps_CreatureInfo->Attributes, MASK0x0010_ATTACK_ANY_CHAMPION)) {
                        AL0439_i_ChampionIndex = M004_RANDOM(4);
                        for (AL0440_i_Counter = 0; (AL0440_i_Counter < 4) && !M516_CHAMPIONS[AL0439_i_ChampionIndex].CurrentHealth; AL0440_i_Counter++) {
                                AL0439_i_ChampionIndex = M017_NEXT(AL0439_i_ChampionIndex);
                        }
                        if (AL0440_i_Counter == 4) {
                                return C0_FALSE;
                        }
                } else {
                        if ((AL0439_i_ChampionIndex = F0286_CHAMPION_GetTargetChampionIndex(P0422_i_MapX, P0423_i_MapY, AL0439_i_TargetCell)) < 0) {
                                return C0_FALSE;
                        }
                }
                if (AL0437_ui_CreatureType == C02_CREATURE_GIGGLER) {
                        F0193_GROUP_StealFromChampion(P0421_ps_Group, AL0439_i_ChampionIndex);
                } else {
                        AL0440_i_Damage = F0230_GROUP_GetChampionDamage(P0421_ps_Group, AL0439_i_ChampionIndex) + 1;
                        L0442_ps_Champion = &M516_CHAMPIONS[AL0439_i_ChampionIndex];
                        if (AL0440_i_Damage > L0442_ps_Champion->MaximumDamageReceived) {
                                L0442_ps_Champion->MaximumDamageReceived = AL0440_i_Damage;
                                L0442_ps_Champion->DirectionMaximumDamageReceived = M018_OPPOSITE(L0438_ui_PrimaryDirectionToParty);
                        }
                }
        }
        if ((AL0440_i_AttackSoundOrdinal = L0441_ps_CreatureInfo->AttackSoundOrdinal) && ((AL0440_i_AttackSoundOrdinal = G2003_aauc_CreatureSounds[--AL0440_i_AttackSoundOrdinal][C0_ATTACK_SOUND]) != CM1_SOUND_NONE)) {
                F0064_SOUND_RequestPlay_CPSD(AL0440_i_AttackSoundOrdinal, P0422_i_MapX, P0423_i_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
        } else {
                if (L1876_B_UseSpellSoundAsFallback) {
                        F0064_SOUND_RequestPlay_CPSD(M542_SOUND_SPELL, P0422_i_MapX, P0423_i_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                }
        }
        return C1_TRUE;
}

STATICFUNCTION void F0208_GROUP_AddEvent(
REGISTER EVENT*        P0425_ps_Event SEPARATOR
REGISTER unsigned long P0426_ul_Time  FINAL_SEPARATOR
{
        if (P0426_ul_Time < M030_TIME(P0425_ps_Event->Map_Time)) {
                P0425_ps_Event->A.A.Type -= 5;
                P0425_ps_Event->C.Ticks = M030_TIME(P0425_ps_Event->Map_Time) - P0426_ul_Time;
                M032_SET_TIME(P0425_ps_Event->Map_Time, P0426_ul_Time);
        } else {
                P0425_ps_Event->C.Ticks = P0426_ul_Time - M030_TIME(P0425_ps_Event->Map_Time);
        }
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0425_ps_Event);
}

void F0209_GROUP_ProcessEvents29to41(
REGISTER int16_t P0427_i_EventMapX SEPARATOR
REGISTER int16_t P0428_i_EventMapY SEPARATOR
int16_t          P0429_i_EventType SEPARATOR
unsigned int16_t P0430_ui_Ticks    FINAL_SEPARATOR
{
        REGISTER ACTIVE_GROUP* L0445_ps_ActiveGroup;
        REGISTER GROUP* L0444_ps_Group;
        REGISTER int16_t L0446_i_Multiple;
#define AL0446_i_EventType           L0446_i_Multiple
#define AL0446_i_Direction           L0446_i_Multiple
#define AL0446_i_Ticks               L0446_i_Multiple
#define AL0446_i_Distance            L0446_i_Multiple
#define AL0446_i_Behavior2Or3        L0446_i_Multiple
#define AL0446_i_CreatureAspectIndex L0446_i_Multiple
#define AL0446_i_Range               L0446_i_Multiple
#define AL0446_i_CreatureAttributes  L0446_i_Multiple
#define AL0446_i_Cell                L0446_i_Multiple
#define AL0446_i_GroupCellsCriteria  L0446_i_Multiple
        REGISTER int16_t L0447_i_Multiple;
#define AL0447_i_Behavior           L0447_i_Multiple
#define AL0447_i_CreatureIndex      L0447_i_Multiple
#define AL0447_i_ReferenceDirection L0447_i_Multiple
#define AL0447_i_Ticks              L0447_i_Multiple
        REGISTER int16_t L0450_i_Multiple;
#define AL0450_i_DestinationMapX  L0450_i_Multiple
#define AL0450_i_DistanceXToParty L0450_i_Multiple
#define AL0450_i_TargetMapX       L0450_i_Multiple
        int16_t L0451_i_Multiple;
#define AL0451_i_DestinationMapY  L0451_i_Multiple
#define AL0451_i_DistanceYToParty L0451_i_Multiple
#define AL0451_i_TargetMapY       L0451_i_Multiple
        THING L0449_T_GroupThing;
        int16_t L0452_i_DistanceToVisibleParty;
        BOOLEAN L0453_B_NewGroupDirectionFound;
        int16_t L0454_i_PrimaryDirectionToOrFromParty;
        BOOLEAN L0455_B_CurrentEventTypeIsNotUpdateBehavior;
        BOOLEAN L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls;
        BOOLEAN L0458_B_SetBehavior7_ApproachAfterReaction;
        int16_t L0459_i_CreatureSize;
        unsigned int16_t L0460_ui_CreatureCount;
        int16_t L0461_i_MovementTicks;
        int16_t L0462_i_TicksSinceLastMove;
        BOOLEAN L0463_B_Archenemy;
        long L0464_l_NextAspectUpdateTime;
        CREATURE_INFO L0448_s_CreatureInfo;
        EVENT L0465_s_NextEvent;


        /* If the party is not on the map specified in the event and the event type is not one of 32, 33, 37, 38 then the event is ignored */
        if ((G0272_i_CurrentMapIndex != G0309_i_PartyMapIndex) && ((AL0446_i_EventType = P0429_i_EventType) != C37_EVENT_UPDATE_BEHAVIOR_GROUP) && (AL0446_i_EventType != C32_EVENT_UPDATE_ASPECT_GROUP) && (AL0446_i_EventType != C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0) && (AL0446_i_EventType != C33_EVENT_UPDATE_ASPECT_CREATURE_0))
                goto T0209139_Return;
        /* If there is no creature at the location specified in the event then the event is ignored */
        if ((L0449_T_GroupThing = F0175_GROUP_GetThing(P0427_i_EventMapX, P0428_i_EventMapY)) == C0xFFFE_THING_ENDOFLIST) {
                goto T0209139_Return;
        }
        L0444_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0449_T_GroupThing);
        L0448_s_CreatureInfo = G0243_as_Graphic559_CreatureInfo[L0444_ps_Group->Type];
        /* Update the event */
        M033_SET_MAP_AND_TIME(L0465_s_NextEvent.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime);
        L0465_s_NextEvent.A.A.Priority = 255 - L0448_s_CreatureInfo.MovementTicks; /* The fastest creatures (with small MovementTicks value) get higher event priority */
        L0465_s_NextEvent.B.Location.MapX = P0427_i_EventMapX;
        L0465_s_NextEvent.B.Location.MapY = P0428_i_EventMapY;
        /* If the creature is not on the party map then try and move the creature in a random direction and place a new event 37 in the timeline for the next creature movement. Note that 'Freeze Life' does not apply to creatures located on maps other than the party map */
        if (G0272_i_CurrentMapIndex != G0309_i_PartyMapIndex) {
                if (F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = M004_RANDOM(4), C0_FALSE)) { /* BUG0_67 A group that is not on the party map may wrongly move or not move into a teleporter. Normally, a creature type with Wariness >= 10 (Vexirk, Materializer / Zytaz, Demon, Lord Chaos, Red Dragon / Dragon) would only move into a teleporter if the creature type is allowed on the destination map. However, the variable G0380_T_CurrentGroupThing identifying the group is not set before being used by F0139_DUNGEON_IsCreatureAllowedOnMap called by F0202_GROUP_IsMovementPossible so the check to see if the creature type is allowed may operate on another creature type and thus return an incorrect result, causing the creature to teleport while it should not, or not to teleport while it should */
                        AL0450_i_DestinationMapX = P0427_i_EventMapX;
                        AL0451_i_DestinationMapY = P0428_i_EventMapY;
                        AL0450_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[AL0446_i_Direction], AL0451_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[AL0446_i_Direction];
                        if (F0267_MOVE_GetMoveResult_CPSCE(L0449_T_GroupThing, P0427_i_EventMapX, P0428_i_EventMapY, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY))
                                goto T0209139_Return;
                        L0465_s_NextEvent.B.Location.MapX = G0397_i_MoveResultMapX;
                        L0465_s_NextEvent.B.Location.MapY = G0398_i_MoveResultMapY;
                }
                L0465_s_NextEvent.A.A.Type = C37_EVENT_UPDATE_BEHAVIOR_GROUP;
                AL0446_i_Ticks = F0025_MAIN_GetMaximumValue(F0023_MAIN_GetAbsoluteValue(G0272_i_CurrentMapIndex - G0309_i_PartyMapIndex) << 4, L0448_s_CreatureInfo.MovementTicks << 1);
                /* BUG0_68 A group moves or acts with a wrong timing. Event is added below but L0465_s_NextEvent.C.Ticks has not been initialized. No consequence while the group is not on the party map. When the party enters the group map the first group event may have a wrong timing */
                T0209005_AddEventAndReturn:
                L0465_s_NextEvent.Map_Time += AL0446_i_Ticks;
                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0465_s_NextEvent);
                goto T0209139_Return;
        }
        /* If the creature is Lord Chaos then ignore the event if the game is won. Initialize data to analyze Fluxcages */
        if (L0463_B_Archenemy = M007_GET(L0448_s_CreatureInfo.Attributes, MASK0x2000_ARCHENEMY)) {
                if (G0302_B_GameWon) {
                        goto T0209139_Return;
                }
                G0386_ui_FluxCageCount = 0;
                (*(long*)G0385_ac_FluxCages) = 0;
        }
        L0445_ps_ActiveGroup = &G0375_ps_ActiveGroups[L0444_ps_Group->ActiveGroupIndex];
        if ((L0462_i_TicksSinceLastMove = (unsigned char)G0313_ul_GameTime - L0445_ps_ActiveGroup->LastMoveTime) < 0) {
                L0462_i_TicksSinceLastMove += 256;
        }
        if ((L0461_i_MovementTicks = L0448_s_CreatureInfo.MovementTicks) == C255_IMMOBILE) {
                L0461_i_MovementTicks = 100;
        }
        if (G0407_s_Party.FreezeLifeTicks && !L0463_B_Archenemy) { /* If life is frozen and the creature is not Lord Chaos (Lord Chaos is immune to Freeze Life) then reschedule the event later (except for reactions which are ignored when life if frozen) */
                if (P0429_i_EventType < 0)
                        goto T0209139_Return;
                L0465_s_NextEvent.A.A.Type = P0429_i_EventType;
                L0465_s_NextEvent.C.Ticks = P0430_ui_Ticks;
                AL0446_i_Ticks = 4; /* Retry in 4 ticks */
                goto T0209005_AddEventAndReturn;
        }
        /* If the specified event type is a 'reaction' instead of a real event from the timeline then create the corresponding reaction event with a delay:
                For event CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT, the reaction time is 1 tick
                For event CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE and CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE, the reaction time may be 1 tick or slower: slow moving creatures react more slowly. The more recent the last creature move, the slower the reaction */
        if (P0429_i_EventType < 0) {
                L0465_s_NextEvent.A.A.Type = P0429_i_EventType + C32_EVENT_UPDATE_ASPECT_GROUP;
                if ((P0429_i_EventType == CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT) || ((AL0446_i_Ticks = ((L0461_i_MovementTicks + 2) >> 2) - L0462_i_TicksSinceLastMove) < 1)) { /* AL0446_i_Ticks is the reaction time */
                        AL0446_i_Ticks = 1; /* Retry in 1 tick */
                }
                goto T0209005_AddEventAndReturn; /* BUG0_68 A group moves or acts with a wrong timing. Event is added but L0465_s_NextEvent.C.Ticks has not been initialized */
        }
        AL0447_i_Behavior = L0444_ps_Group->Behavior;
        L0460_ui_CreatureCount = L0444_ps_Group->Count;
        L0459_i_CreatureSize = M007_GET(L0448_s_CreatureInfo.Attributes, MASK0x0003_SIZE);
        AL0450_i_DistanceXToParty = ((AL0446_i_Distance = P0427_i_EventMapX - G0306_i_PartyMapX) < 0) ? -AL0446_i_Distance : AL0446_i_Distance;
        AL0451_i_DistanceYToParty = ((AL0446_i_Distance = P0428_i_EventMapY - G0307_i_PartyMapY) < 0) ? -AL0446_i_Distance : AL0446_i_Distance;
        G0378_i_CurrentGroupMapX = P0427_i_EventMapX;
        G0379_i_CurrentGroupMapY = P0428_i_EventMapY;
        G0380_T_CurrentGroupThing = L0449_T_GroupThing;
        (*(long*)G0384_auc_GroupMovementTestedDirections) = 0;
        G0381_ui_CurrentGroupDistanceToParty = F0226_GROUP_GetDistanceBetweenSquares(P0427_i_EventMapX, P0428_i_EventMapY, G0306_i_PartyMapX, G0307_i_PartyMapY);
        G0382_i_CurrentGroupPrimaryDirectionToParty = F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0427_i_EventMapX, P0428_i_EventMapY, G0306_i_PartyMapX, G0307_i_PartyMapY);
        G0383_i_CurrentGroupSecondaryDirectionToParty = G0363_i_SecondaryDirectionToOrFromParty;
        L0464_l_NextAspectUpdateTime = 0;
        L0455_B_CurrentEventTypeIsNotUpdateBehavior = C1_TRUE;
        if (P0429_i_EventType <= C31_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT) { /* Process Reaction events 29 to 31 */
                switch (P0429_i_EventType -= C32_EVENT_UPDATE_ASPECT_GROUP) {
                        case CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT: /* This event is used when the party bumps into a group or attacks a group physically (not with a spell). It causes the creature behavior to change to attack if it is not already attacking the party or fleeing from target */
                                if (
                                   (L0448_s_CreatureInfo.AttackTicks != C255_UNKNOWN) &&
                                   (AL0447_i_Behavior != C6_BEHAVIOR_ATTACK) && (AL0447_i_Behavior != C5_BEHAVIOR_FLEE)) {
                                        F0181_GROUP_DeleteEvents(P0427_i_EventMapX, P0428_i_EventMapY);
                                        goto T0209044_SetBehavior6_Attack;
                                }
                                else {
                                        L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                        L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                                        goto T0209139_Return;
                                }
                        case CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE: /* This event is used for the reaction of a group after a projectile impacted with one creature in the group (some creatures may have been killed) */
                                if ((AL0447_i_Behavior == C6_BEHAVIOR_ATTACK) || (AL0447_i_Behavior == C5_BEHAVIOR_FLEE)) /* If the creature is attacking the party or fleeing from the target then there is no reaction */
                                        goto T0209139_Return;
                                if ((AL0446_i_Behavior2Or3 = ((AL0447_i_Behavior == C3_BEHAVIOR_USELESS) || (AL0447_i_Behavior == C2_BEHAVIOR_USELESS))) || (M004_RANDOM(4))) { /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is thus: if 3/4 chances */
                                        if (!F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) { /* If the group cannot see the party then look in a random direction to try and search for the party */
                                                L0458_B_SetBehavior7_ApproachAfterReaction = L0453_B_NewGroupDirectionFound = C0_FALSE;
                                                goto T0209073_SetDirectionGroup;
                                        }
                                        else {
                                                if (AL0446_i_Behavior2Or3 || (M004_RANDOM(4))) /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is thus: if 3/4 chances then no reaction */
                                                        goto T0209139_Return;
                                        }
                                } /* No 'break': proceed to instruction after the next 'case' below. Reaction is to move in a random direction to try and avoid other projectiles */
                        case CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE: /* This event is used when some creatures in the group were killed by a Poison Cloud or by a closing door or if Lord Chaos is surrounded by 3 Fluxcages. It causes the creature to move in a random direction to avoid the danger */
                                L0458_B_SetBehavior7_ApproachAfterReaction = (AL0447_i_Behavior == C6_BEHAVIOR_ATTACK); /* If the creature behavior is 'Attack' and it has to move to avoid danger then it will change its behavior to 'Approach' after the movement */
                                L0453_B_NewGroupDirectionFound = C0_FALSE;
                                goto T0209058_MoveInRandomDirection;
                        default:
                                goto T0209139_Return;
                }
        }
        if (P0429_i_EventType < C37_EVENT_UPDATE_BEHAVIOR_GROUP) { /* Process Update Aspect events 32 to 36 */
                L0465_s_NextEvent.A.A.Type = P0429_i_EventType + 5;
                if (F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) {
                        if ((AL0447_i_Behavior != C6_BEHAVIOR_ATTACK) && (AL0447_i_Behavior != C5_BEHAVIOR_FLEE)) {
                                if (
                                   (L0448_s_CreatureInfo.AttackTicks != C255_UNKNOWN) &&
                                   (M038_DISTANCE(G0306_i_PartyMapX, G0307_i_PartyMapY, P0427_i_EventMapX, P0428_i_EventMapY) <= 1))
                                        goto T0209044_SetBehavior6_Attack;
                                else {
                                        if (((AL0447_i_Behavior == C0_BEHAVIOR_WANDER) || (AL0447_i_Behavior == C3_BEHAVIOR_USELESS)) && (AL0447_i_Behavior != C7_BEHAVIOR_APPROACH)) /* BUG0_00 Useless code. Behavior cannot be 3 because this value is never used. Moreover, the second condition in the && is redundant (if the value is 0 or 3, it cannot be 7). The actual condition is: if (AL0447_i_Behavior == C0_BEHAVIOR_WANDER) */
                                                goto T0209054_SetBehavior7_Approach;
                                }
                        }
                        L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                        L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                }
                if (AL0447_i_Behavior == C6_BEHAVIOR_ATTACK) {
                        AL0446_i_CreatureAspectIndex = P0429_i_EventType - C33_EVENT_UPDATE_ASPECT_CREATURE_0; /* Value -1 for event 32, meaning aspect will be updated for all creatures in the group */
                        L0464_l_NextAspectUpdateTime = F0179_GROUP_GetCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0446_i_CreatureAspectIndex, M007_GET(L0445_ps_ActiveGroup->Aspect[AL0446_i_CreatureAspectIndex], MASK0x0080_IS_ATTACKING));
                        goto T0209136;
                }
                else {
                        if ((AL0450_i_DistanceXToParty > 3) || (AL0451_i_DistanceYToParty > 3)) {
                                L0464_l_NextAspectUpdateTime = G0313_ul_GameTime + M063_NEXT_NON_ATTACK_ASPECT_UPDATE_TICKS(L0448_s_CreatureInfo.AnimationTicks);
                                goto T0209136;
                        }
                        else {
                                goto T0209135;
                        }
                }
        }
        /* Process Update Behavior events 37 to 41 */
        L0455_B_CurrentEventTypeIsNotUpdateBehavior = C0_FALSE;
        if (P0430_ui_Ticks) {
                L0464_l_NextAspectUpdateTime = G0313_ul_GameTime;
        }
        if (P0429_i_EventType == C37_EVENT_UPDATE_BEHAVIOR_GROUP) { /* Process event 37, Update Group Behavior */
                if ((AL0447_i_Behavior == C0_BEHAVIOR_WANDER) || (AL0447_i_Behavior == C2_BEHAVIOR_USELESS) || (AL0447_i_Behavior == C3_BEHAVIOR_USELESS)) { /* BUG0_00 Useless code. Behavior cannot be 2 nor 3 because these values are never used. The actual condition is: if (AL0447_i_Behavior == C0_BEHAVIOR_WANDER) */
                        if (L0452_i_DistanceToVisibleParty = F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                if ((L0452_i_DistanceToVisibleParty <= M056_ATTACK_RANGE(L0448_s_CreatureInfo.Ranges)) &&
                                   (L0448_s_CreatureInfo.AttackTicks != C255_UNKNOWN) &&
                                   ((!AL0450_i_DistanceXToParty) || (!AL0451_i_DistanceYToParty))) { /* If the creature is in range for attack and on the same row or column as the party on the map */
                                        T0209044_SetBehavior6_Attack:
                                        if (P0429_i_EventType == CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE) {
                                                F0181_GROUP_DeleteEvents(P0427_i_EventMapX, P0428_i_EventMapY);
                                        }
                                        L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                        L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                                        L0444_ps_Group->Behavior = C6_BEHAVIOR_ATTACK;
                                        AL0446_i_Direction = G0382_i_CurrentGroupPrimaryDirectionToParty;
                                        for (AL0447_i_CreatureIndex = L0460_ui_CreatureCount; AL0447_i_CreatureIndex >= 0; AL0447_i_CreatureIndex--) {
                                                if ((M050_CREATURE_VALUE(L0445_ps_ActiveGroup->Directions, AL0447_i_CreatureIndex) != AL0446_i_Direction) &&
                                                    !(AL0447_i_CreatureIndex && M005_RANDOM(2))) {
                                                        F0205_GROUP_SetDirection(L0445_ps_ActiveGroup, AL0446_i_Direction, AL0447_i_CreatureIndex, L0460_ui_CreatureCount && (L0459_i_CreatureSize == C1_SIZE_HALF_SQUARE));
                                                        M032_SET_TIME(L0465_s_NextEvent.Map_Time, G0313_ul_GameTime + M004_RANDOM(4) + 2); /* Random delay represents the time for the creature to turn */
                                                } else {
                                                        M032_SET_TIME(L0465_s_NextEvent.Map_Time, G0313_ul_GameTime + 1);
                                                }
                                                if (L0455_B_CurrentEventTypeIsNotUpdateBehavior) {
                                                        L0465_s_NextEvent.Map_Time += F0024_MAIN_GetMinimumValue((L0448_s_CreatureInfo.AttackTicks >> 1) + M004_RANDOM(4), P0430_ui_Ticks);
                                                }
                                                L0465_s_NextEvent.A.A.Type = C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + AL0447_i_CreatureIndex;
                                                F0208_GROUP_AddEvent(&L0465_s_NextEvent, F0179_GROUP_GetCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, C0_FALSE));
                                        }
                                        goto T0209139_Return;
                                }
                                else {
                                        if (AL0447_i_Behavior != C2_BEHAVIOR_USELESS) { /* BUG0_00 Useless code. Behavior cannot be 2 because this value is never used */
                                                T0209054_SetBehavior7_Approach:
                                                L0444_ps_Group->Behavior = C7_BEHAVIOR_APPROACH;
                                                L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                                L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                                                L0465_s_NextEvent.Map_Time++;
                                                goto T0209134_SetEvent37;
                                        }
                                }
                        } else {
                                if (AL0447_i_Behavior == C0_BEHAVIOR_WANDER) {
                                        if (L0454_i_PrimaryDirectionToOrFromParty = F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                                L0454_i_PrimaryDirectionToOrFromParty--;
                                                L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = C0_FALSE;
                                                goto T0209085_SingleSquareMove;
                                        }
                                        L0453_B_NewGroupDirectionFound = C0_FALSE;
                                        if (M005_RANDOM(2)) {
                                                T0209058_MoveInRandomDirection:
                                                AL0446_i_Direction = M004_RANDOM(4);
                                                AL0447_i_ReferenceDirection = AL0446_i_Direction;
                                                do {
                                                        AL0450_i_DestinationMapX = P0427_i_EventMapX;
                                                        AL0451_i_DestinationMapY = P0428_i_EventMapY;
                                                        AL0450_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[AL0446_i_Direction], AL0451_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[AL0446_i_Direction];
                                                        if (((L0445_ps_ActiveGroup->PriorMapX != AL0450_i_DestinationMapX) ||
                                                             (L0445_ps_ActiveGroup->PriorMapY != AL0451_i_DestinationMapY) ||
                                                             !M004_RANDOM(4)) /* 1/4 chance of moving back to the square that the creature comes from */
                                                            && F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction, C0_FALSE)) {
                                                                T0209061_MoveGroup:
                                                                if (L0453_B_NewGroupDirectionFound = (AL0447_i_Ticks = (L0461_i_MovementTicks >> 1) - L0462_i_TicksSinceLastMove) <= 0) {
                                                                        if (F0267_MOVE_GetMoveResult_CPSCE(L0449_T_GroupThing, P0427_i_EventMapX, P0428_i_EventMapY, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY))
                                                                                goto T0209139_Return;
                                                                        L0465_s_NextEvent.B.Location.MapX = G0397_i_MoveResultMapX;
                                                                        L0465_s_NextEvent.B.Location.MapY = G0398_i_MoveResultMapY;
                                                                        L0445_ps_ActiveGroup->PriorMapX = P0427_i_EventMapX;
                                                                        L0445_ps_ActiveGroup->PriorMapY = P0428_i_EventMapY;
                                                                        L0445_ps_ActiveGroup->LastMoveTime = G0313_ul_GameTime;
                                                                } else {
                                                                        L0461_i_MovementTicks = AL0447_i_Ticks;
                                                                        L0462_i_TicksSinceLastMove = -1;
                                                                }
                                                                break;
                                                        }
                                                        if (G0390_B_GroupMovementBlockedByParty) {
                                                                if ((P0429_i_EventType != CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE) &&
                                                                    (L0448_s_CreatureInfo.AttackTicks != C255_UNKNOWN) &&
                                                                    ((L0444_ps_Group->Behavior != C5_BEHAVIOR_FLEE) ||
                                                                    !F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, C0_FALSE) ||
                                                                    M005_RANDOM(2)))
                                                                        goto T0209044_SetBehavior6_Attack;
                                                                L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                                                L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                                                        }
                                                } while ((AL0446_i_Direction = M017_NEXT(AL0446_i_Direction)) != AL0447_i_ReferenceDirection);
                                        }
                                        if (!L0453_B_NewGroupDirectionFound &&
                                            (L0462_i_TicksSinceLastMove != -1) &&
                                            L0463_B_Archenemy &&
                                            ((P0429_i_EventType == CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE) || !M004_RANDOM(4))) { /* BUG0_15 The game hangs when you close a door on Lord Chaos. A condition is missing in the code to manage creatures and this may create an infinite loop between two parts in the code */
                                                G0363_i_SecondaryDirectionToOrFromParty = M017_NEXT(L0454_i_PrimaryDirectionToOrFromParty = M004_RANDOM(4));
                                                goto T0209089_DoubleSquareMove; /* BUG0_69 Memory corruption when you close a door on Lord Chaos. The local variable (L0454_i_PrimaryDirectionToOrFromParty) containing the direction where Lord Chaos tries to move may be used as an array index without being initialized and cause memory corruption */
                                        }
                                        if (L0453_B_NewGroupDirectionFound || ((!M004_RANDOM(4) || (L0452_i_DistanceToVisibleParty <= M055_SMELL_RANGE(L0448_s_CreatureInfo.Ranges))) && (P0429_i_EventType != CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE))) {
                                                T0209073_SetDirectionGroup:
                                                if (!L0453_B_NewGroupDirectionFound && (L0462_i_TicksSinceLastMove >= 0)) { /* If direction is not found yet then look around in a random direction */
                                                        AL0446_i_Direction = M004_RANDOM(4);
                                                }
                                                F0206_GROUP_SetDirectionGroup(L0445_ps_ActiveGroup, AL0446_i_Direction, L0460_ui_CreatureCount, L0459_i_CreatureSize);
                                        }
                                        /* If event is CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE or CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE */
                                        if (P0429_i_EventType < CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT) {
                                                if (!L0453_B_NewGroupDirectionFound)
                                                        goto T0209139_Return;
                                                if (L0458_B_SetBehavior7_ApproachAfterReaction) {
                                                        L0444_ps_Group->Behavior = C7_BEHAVIOR_APPROACH;
                                                }
                                                F0182_GROUP_StopAttacking(L0445_ps_ActiveGroup, P0427_i_EventMapX, P0428_i_EventMapY);
                                        }
                                }
                        }
                } else {
                        if (AL0447_i_Behavior == C7_BEHAVIOR_APPROACH) {
                                if (L0452_i_DistanceToVisibleParty = F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                        if ((L0452_i_DistanceToVisibleParty <= M056_ATTACK_RANGE(L0448_s_CreatureInfo.Ranges)) &&
                                           (L0448_s_CreatureInfo.AttackTicks != C255_UNKNOWN) &&
                                           ((!AL0450_i_DistanceXToParty) || (!AL0451_i_DistanceYToParty))) /* If the creature is in range for attack and on the same row or column as the party on the map */
                                                goto T0209044_SetBehavior6_Attack;
                                        T0209081_RunTowardParty:
                                        L0461_i_MovementTicks++;
                                        L0461_i_MovementTicks >>= 1; /* Running speed is half the movement ticks */
                                        AL0450_i_TargetMapX = (L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX);
                                        AL0451_i_TargetMapY = (L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY);
                                } else {
                                        T0209082_WalkTowardTarget:
                                        AL0450_i_TargetMapX = L0445_ps_ActiveGroup->TargetMapX;
                                        AL0451_i_TargetMapY = L0445_ps_ActiveGroup->TargetMapY;
                                        /* If the creature reached its target but the party is not there anymore */
                                        if ((P0427_i_EventMapX == AL0450_i_TargetMapX) && (P0428_i_EventMapY == AL0451_i_TargetMapY)) {
                                                L0453_B_NewGroupDirectionFound = C0_FALSE;
                                                L0444_ps_Group->Behavior = C0_BEHAVIOR_WANDER;
                                                goto T0209073_SetDirectionGroup;
                                        }
                                }
                                L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = C1_TRUE;
                                T0209084_SingleSquareMoveTowardTargetSquare:
                                L0454_i_PrimaryDirectionToOrFromParty = F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0427_i_EventMapX, P0428_i_EventMapY, AL0450_i_TargetMapX, AL0451_i_TargetMapY);
                                T0209085_SingleSquareMove:
                                if (F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = L0454_i_PrimaryDirectionToOrFromParty, L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls) ||
                                    F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = G0363_i_SecondaryDirectionToOrFromParty, L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls && M005_RANDOM(2)) ||
                                    F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = M018_OPPOSITE(AL0446_i_Direction), C0_FALSE) ||
                                    (!M004_RANDOM(4) && F0202_GROUP_IsMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = M018_OPPOSITE(L0454_i_PrimaryDirectionToOrFromParty), C0_FALSE))) {
                                        AL0450_i_DestinationMapX = P0427_i_EventMapX;
                                        AL0451_i_DestinationMapY = P0428_i_EventMapY;
                                        AL0450_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[AL0446_i_Direction], AL0451_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[AL0446_i_Direction];
                                        goto T0209061_MoveGroup;
                                }
                                if (L0463_B_Archenemy) {
                                        T0209089_DoubleSquareMove:
                                        F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, C0_FALSE); /* BUG0_00 Useless code. Returned value is ignored. When Lord Chaos teleports two squares away the ability to move to the first square is ignored which means Lord Chaos can teleport through walls or any other obstacle */
                                        if (F0204_GROUP_IsArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = L0454_i_PrimaryDirectionToOrFromParty) ||
                                            F0204_GROUP_IsArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = G0363_i_SecondaryDirectionToOrFromParty) ||
                                            (G0386_ui_FluxCageCount && F0204_GROUP_IsArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = M018_OPPOSITE(AL0446_i_Direction))) ||
                                            ((G0386_ui_FluxCageCount >= 2) && F0204_GROUP_IsArchenemyDoubleMovementPossible(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, AL0446_i_Direction = M018_OPPOSITE(L0454_i_PrimaryDirectionToOrFromParty)))) {
                                                AL0450_i_DestinationMapX = P0427_i_EventMapX;
                                                AL0451_i_DestinationMapY = P0428_i_EventMapY;
                                                AL0450_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[AL0446_i_Direction] * 2, AL0451_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[AL0446_i_Direction] * 2;
                                                F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, AL0450_i_DestinationMapX, AL0451_i_DestinationMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                                goto T0209061_MoveGroup;
                                        }
                                }
                                F0206_GROUP_SetDirectionGroup(L0445_ps_ActiveGroup, L0454_i_PrimaryDirectionToOrFromParty, L0460_ui_CreatureCount, L0459_i_CreatureSize);
                        } else {
                                if (AL0447_i_Behavior == C5_BEHAVIOR_FLEE) {
                                        T0209094_FleeFromTarget:
                                        L0456_B_AllowMovementOverImaginaryPitsAndFakeWalls = C1_TRUE;
                                        /* If the creature can see the party then update target coordinates */
                                        if (L0452_i_DistanceToVisibleParty = F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                                AL0450_i_TargetMapX = (L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX);
                                                AL0451_i_TargetMapY = (L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY);
                                        } else {
                                                if (!(--(L0445_ps_ActiveGroup->DelayFleeingFromTarget))) { /* If the creature is not afraid anymore then stop fleeing from target */
                                                        T0209096_SetBehavior0_Wander:
                                                        L0453_B_NewGroupDirectionFound = C0_FALSE;
                                                        L0444_ps_Group->Behavior = C0_BEHAVIOR_WANDER;
                                                        goto T0209073_SetDirectionGroup;
                                                }
                                                else {
                                                        if (M005_RANDOM(2)) {
                                                                /* If the creature cannot move and the party is adjacent then stop fleeing */
                                                                if (!F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal(&L0448_s_CreatureInfo, P0427_i_EventMapX, P0428_i_EventMapY, C0_FALSE)) {
                                                                        if (M038_DISTANCE(P0427_i_EventMapX, P0428_i_EventMapY, G0306_i_PartyMapX, G0307_i_PartyMapY) <= 1)
                                                                                goto T0209096_SetBehavior0_Wander;
                                                                }
                                                                /* Set creature target to the home square where the creature was located when the party entered the map */
                                                                AL0450_i_TargetMapX = L0445_ps_ActiveGroup->HomeMapX;
                                                                AL0451_i_TargetMapY = L0445_ps_ActiveGroup->HomeMapY;
                                                                goto T0209084_SingleSquareMoveTowardTargetSquare;
                                                        }
                                                        else {
                                                                AL0450_i_TargetMapX = L0445_ps_ActiveGroup->TargetMapX;
                                                                AL0451_i_TargetMapY = L0445_ps_ActiveGroup->TargetMapY;
                                                        }
                                                }
                                        }
                                        /* Try and flee from the party (opposite direction) */
                                        L0454_i_PrimaryDirectionToOrFromParty = M018_OPPOSITE(F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0427_i_EventMapX, P0428_i_EventMapY, AL0450_i_TargetMapX, AL0451_i_TargetMapY));
                                        G0363_i_SecondaryDirectionToOrFromParty = M018_OPPOSITE(G0363_i_SecondaryDirectionToOrFromParty);
                                        L0461_i_MovementTicks -= (L0461_i_MovementTicks >> 2);
                                        goto T0209085_SingleSquareMove;
                                }
                        }
                }
        } else { /* Process events 38 to 41, Update Creature Behavior */
                if (AL0447_i_Behavior == C5_BEHAVIOR_FLEE) {
                        if (L0460_ui_CreatureCount) {
                                F0182_GROUP_StopAttacking(L0445_ps_ActiveGroup, P0427_i_EventMapX, P0428_i_EventMapY);
                        }
                        goto T0209094_FleeFromTarget;
                }
                /* If the creature is attacking, then compute the next aspect update time and the next attack time */
                if (M007_GET(L0445_ps_ActiveGroup->Aspect[AL0447_i_CreatureIndex = P0429_i_EventType - C38_EVENT_UPDATE_BEHAVIOR_CREATURE_0], MASK0x0080_IS_ATTACKING)) {
                        L0464_l_NextAspectUpdateTime = F0179_GROUP_GetCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, C0_FALSE);
                        L0465_s_NextEvent.Map_Time += ((AL0447_i_Ticks = L0448_s_CreatureInfo.AttackTicks) + M004_RANDOM(4) - 1);
                        if (AL0447_i_Ticks > 15) {
                                L0465_s_NextEvent.Map_Time += M003_RANDOM(8) - 2;
                        }
                } else { /* If the creature is not attacking, then try attacking if possible */
                        if (AL0447_i_CreatureIndex > L0460_ui_CreatureCount) { /* Ignore event if it is for a creature that is not in the group */
                                goto T0209139_Return;
                        }
                        L0454_i_PrimaryDirectionToOrFromParty = G0382_i_CurrentGroupPrimaryDirectionToParty;
                        /* If the party is visible, update the target coordinates */
                        if (L0452_i_DistanceToVisibleParty = F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, AL0447_i_CreatureIndex, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                        }
                        /* If there is a single creature in the group that is not full square sized and 1/4 chance */
                        if (!L0460_ui_CreatureCount && (L0459_i_CreatureSize != C2_SIZE_FULL_SQUARE) && !((AL0446_i_GroupCellsCriteria = M006_RANDOM(65536)) & 0x00C0)) {
                                if (L0445_ps_ActiveGroup->Cells != C0xFF_SINGLE_CENTERED_CREATURE) {
                                        /* If the creature is not already on the center of the square then change its cell */
                                        if (AL0446_i_GroupCellsCriteria & 0x0038) { /* 7/8 chances of changing cell to the center of the square */
                                                L0445_ps_ActiveGroup->Cells = C0xFF_SINGLE_CENTERED_CREATURE;
                                        } else { /* 1/8 chance of changing cell to the next or previous cell on the square */
                                                AL0446_i_GroupCellsCriteria = M021_NORMALIZE(M021_NORMALIZE(L0445_ps_ActiveGroup->Cells) + ((AL0446_i_GroupCellsCriteria & 0x0001) ? 1 : -1));
                                        }
                                }
                                /* If 1/8 chance and the creature is not adjacent to the party and is a quarter square sized creature then process projectile impacts and update the creature cell if still alive. When the creature is not in front of the party, it has 7/8 chances of dodging a projectile by moving to another cell or staying in the center of the square */
                                if (!(AL0446_i_GroupCellsCriteria & 0x0038) && (L0452_i_DistanceToVisibleParty != 1) && (L0459_i_CreatureSize == C0_SIZE_QUARTER_SQUARE)) {
                                        if (F0218_PROJECTILE_GetImpactCount(CM1_ELEMENT_CREATURE, P0427_i_EventMapX, P0428_i_EventMapY, L0445_ps_ActiveGroup->Cells) && (G0364_i_CreatureDamageOutcome == C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP)) /* This call to F0218_PROJECTILE_GetImpactCount works fine because there is a single creature in the group so L0445_ps_ActiveGroup->Cells contains only one cell index */
                                                goto T0209139_Return;
                                        L0445_ps_ActiveGroup->Cells = M021_NORMALIZE(AL0446_i_GroupCellsCriteria);
                                }
                        }
                        /* If the creature can see the party and is looking in the party direction or can attack in all direction */
                        if (L0452_i_DistanceToVisibleParty &&
                            (M007_GET(L0448_s_CreatureInfo.Attributes, MASK0x0004_SIDE_ATTACK) ||
                            M050_CREATURE_VALUE(L0445_ps_ActiveGroup->Directions, AL0447_i_CreatureIndex) == L0454_i_PrimaryDirectionToOrFromParty)) {
                                /* If the creature is in range to attack the party and random test succeeds */
                                if ((L0452_i_DistanceToVisibleParty <= (AL0446_i_Range = M056_ATTACK_RANGE(L0448_s_CreatureInfo.Ranges))) &&
                                    (!AL0450_i_DistanceXToParty || !AL0451_i_DistanceYToParty
                                    || ((AL0446_i_Range > 1) && !M003_RANDOM(8))
                                    ) &&
                                    (AL0446_i_Range <= (M003_RANDOM(16) + 1))) {
                                        if ((AL0446_i_Range == 1) &&
                                            !(M007_GET(AL0446_i_CreatureAttributes = L0448_s_CreatureInfo.Attributes, MASK0x0008_PREFER_BACK_ROW) && M004_RANDOM(4) && M007_GET(AL0446_i_CreatureAttributes, MASK0x0010_ATTACK_ANY_CHAMPION)) &&
                                            (L0459_i_CreatureSize == C0_SIZE_QUARTER_SQUARE) &&
                                            (L0445_ps_ActiveGroup->Cells != C0xFF_SINGLE_CENTERED_CREATURE) &&
                                            ((AL0446_i_Cell = M050_CREATURE_VALUE(L0445_ps_ActiveGroup->Cells, AL0447_i_CreatureIndex)) != L0454_i_PrimaryDirectionToOrFromParty) &&
                                            (AL0446_i_Cell != M017_NEXT(L0454_i_PrimaryDirectionToOrFromParty))) { /* If the creature cannot cast spells (range = 1) and is not on a cell where it can attack the party directly and is a quarter square sized creature not in the center of the square then the creature moves to another cell and attack does not occur immediately */
                                                if (!L0460_ui_CreatureCount && M005_RANDOM(2)) {
                                                        L0445_ps_ActiveGroup->Cells = C0xFF_SINGLE_CENTERED_CREATURE;
                                                } else {
                                                        if ((L0454_i_PrimaryDirectionToOrFromParty & 0x0001) == (AL0446_i_Cell & 0x0001)) {
                                                                AL0446_i_Cell--;
                                                        } else {
                                                                AL0446_i_Cell++;
                                                        }
                                                        if (!F0176_GROUP_GetCreatureOrdinalInCell(L0444_ps_Group, AL0446_i_Cell &= 3) ||
                                                            (M005_RANDOM(2) && !F0176_GROUP_GetCreatureOrdinalInCell(L0444_ps_Group, AL0446_i_Cell = M018_OPPOSITE(AL0446_i_Cell)))) { /* If the selected cell (or the opposite cell) is not already occupied by a creature */
                                                                if (F0218_PROJECTILE_GetImpactCount(CM1_ELEMENT_CREATURE, P0427_i_EventMapX, P0428_i_EventMapY, L0445_ps_ActiveGroup->Cells) && (G0364_i_CreatureDamageOutcome == C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP)) /* BUG0_70 A projectile impact on a creature may be ignored. The function F0218_PROJECTILE_GetImpactCount to detect projectile impacts when a quarter square sized creature moves inside a group (to another cell on the same square) may fail if there are several creatures in the group because the function expects a single cell index for its last parameter. The function should be called once for each cell where there is a creature */
                                                                        goto T0209139_Return;
                                                                if (G0364_i_CreatureDamageOutcome != C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP) {
                                                                        L0445_ps_ActiveGroup->Cells = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0445_ps_ActiveGroup->Cells, AL0447_i_CreatureIndex, AL0446_i_Cell);
                                                                }
                                                        }
                                                }
                                                L0465_s_NextEvent.Map_Time += F0025_MAIN_GetMaximumValue(1, (L0448_s_CreatureInfo.MovementTicks >> 1) + M005_RANDOM(2)); /* Time for the creature to change cell */
                                                L0465_s_NextEvent.A.A.Type = P0429_i_EventType;
                                                goto T0209135;
                                        }
                                        L0464_l_NextAspectUpdateTime = F0179_GROUP_GetCreatureAspectUpdateTime(L0445_ps_ActiveGroup, AL0447_i_CreatureIndex, F0207_GROUP_IsCreatureAttacking(L0444_ps_Group, P0427_i_EventMapX, P0428_i_EventMapY, AL0447_i_CreatureIndex));
#ifdef PC_FIX_CODE_SIZE
L0451_i_Multiple++;
L0451_i_Multiple++;
L0451_i_Multiple++;
L0451_i_Multiple++;
#endif
                                        L0465_s_NextEvent.Map_Time += M062_NEXT_BEHAVIOR_UPDATE_AFTER_ATTACK_TICKS(L0448_s_CreatureInfo.AnimationTicks) + M005_RANDOM(2);
                                } else {
                                        L0444_ps_Group->Behavior = C7_BEHAVIOR_APPROACH;
                                        if (L0460_ui_CreatureCount) {
                                                F0182_GROUP_StopAttacking(L0445_ps_ActiveGroup, P0427_i_EventMapX, P0428_i_EventMapY);
                                        }
                                        goto T0209081_RunTowardParty;
                                }
                        } else {
                                /* If the party is visible, update target coordinates */
                                if (F0200_GROUP_GetDistanceToVisibleParty(L0444_ps_Group, CM1_WHOLE_CREATURE_GROUP, P0427_i_EventMapX, P0428_i_EventMapY)) {
                                        L0445_ps_ActiveGroup->TargetMapX = G0306_i_PartyMapX;
                                        L0445_ps_ActiveGroup->TargetMapY = G0307_i_PartyMapY;
                                        F0205_GROUP_SetDirection(L0445_ps_ActiveGroup, L0454_i_PrimaryDirectionToOrFromParty, AL0447_i_CreatureIndex, L0460_ui_CreatureCount && (L0459_i_CreatureSize == C1_SIZE_HALF_SQUARE));
                                        L0465_s_NextEvent.Map_Time += 2;
                                        L0464_l_NextAspectUpdateTime = M030_TIME(L0465_s_NextEvent.Map_Time);
                                } else { /* If the party is not visible, move to the target (last known party location) */
                                        L0444_ps_Group->Behavior = C7_BEHAVIOR_APPROACH;
                                        if (L0460_ui_CreatureCount) {
                                                F0182_GROUP_StopAttacking(L0445_ps_ActiveGroup, P0427_i_EventMapX, P0428_i_EventMapY);
                                        }
                                        goto T0209082_WalkTowardTarget;
                                }
                        }
                }
                L0465_s_NextEvent.A.A.Type = P0429_i_EventType;
                goto T0209136;
        }
        L0465_s_NextEvent.Map_Time += F0025_MAIN_GetMaximumValue(1, M004_RANDOM(4) + L0461_i_MovementTicks - 1);
        T0209134_SetEvent37:
        L0465_s_NextEvent.A.A.Type = C37_EVENT_UPDATE_BEHAVIOR_GROUP;
        T0209135:
        if (!L0464_l_NextAspectUpdateTime) {
                L0464_l_NextAspectUpdateTime = F0179_GROUP_GetCreatureAspectUpdateTime(L0445_ps_ActiveGroup, CM1_WHOLE_CREATURE_GROUP, C0_FALSE);
        }
        T0209136:
        if (L0455_B_CurrentEventTypeIsNotUpdateBehavior) {
                L0465_s_NextEvent.Map_Time += P0430_ui_Ticks;
        } else {
                L0464_l_NextAspectUpdateTime += P0430_ui_Ticks;
        }
        F0208_GROUP_AddEvent(&L0465_s_NextEvent, L0464_l_NextAspectUpdateTime);
        T0209139_Return: ;
}
/* END GROUP.C */

/* BEGIN TIMELINE.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
int16_t G0369_EventMaximumCount;
int16_t G2009_i_LargestUsedEventOrdinal;
EVENT* G0370_ps_Events;
unsigned int16_t* G0371_pui_Timeline;
unsigned int16_t G0372_ui_EventCount;
unsigned int16_t G0373_ui_FirstUnusedEventIndex;



void F0233_TIMELINE_Initialize_CPSE(
void
)
{
        REGISTER EVENT* L0577_ps_Event;
        REGISTER unsigned int16_t L0576_ui_EventIndex;


        G0370_ps_Events = (EVENT*)M533_F0468_MEMORY_Allocate((long)(G0369_EventMaximumCount * sizeof(EVENT)), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
        G0371_pui_Timeline = (unsigned int16_t*)M533_F0468_MEMORY_Allocate((long)(G0369_EventMaximumCount * sizeof(unsigned int16_t)), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
        if (G0298_B_NewGame) {
                L0577_ps_Event = G0370_ps_Events;
                for (L0576_ui_EventIndex = 0; L0576_ui_EventIndex < G0369_EventMaximumCount; L0577_ps_Event++) {
                        L0577_ps_Event->A.A.Type = C00_EVENT_NONE;
                        ((UNUSED_EVENT*)L0577_ps_Event)->NextUnusedEventIndex = ++L0576_ui_EventIndex;
                }
                G2009_i_LargestUsedEventOrdinal = M000_INDEX_TO_ORDINAL(-1);
                ((UNUSED_EVENT*)(L0577_ps_Event - 1))->NextUnusedEventIndex = -1;
                G0372_ui_EventCount = 0;
                G0373_ui_FirstUnusedEventIndex = 0;
        }
}

void F0651_TIMELINE_InitializeOptimizedManagement(
void
)
{
        REGISTER int16_t L3029_i_EventIndex;
        REGISTER int16_t L3030_i_UnusedEventIndex;


        G2009_i_LargestUsedEventOrdinal = M000_INDEX_TO_ORDINAL(-1);
        G0373_ui_FirstUnusedEventIndex = -1;
        for (L3029_i_EventIndex = 0; L3029_i_EventIndex < G0369_EventMaximumCount; L3029_i_EventIndex++) {
                if (G0370_ps_Events[L3029_i_EventIndex].A.A.Type == C00_EVENT_NONE) {
                        if ((int16_t)G0373_ui_FirstUnusedEventIndex == -1) {
                                G0373_ui_FirstUnusedEventIndex = L3029_i_EventIndex;
                        } else {
                                ((UNUSED_EVENT*)G0370_ps_Events)[L3030_i_UnusedEventIndex].NextUnusedEventIndex = L3029_i_EventIndex;
                        }
                        ((UNUSED_EVENT*)G0370_ps_Events)[L3029_i_EventIndex].NextUnusedEventIndex = -1;
                        L3030_i_UnusedEventIndex = L3029_i_EventIndex;
                        continue;
                }
                G2009_i_LargestUsedEventOrdinal = M000_INDEX_TO_ORDINAL(L3029_i_EventIndex);
        }
}

STATICFUNCTION BOOLEAN F0234_TIMELINE_IsEventABeforeEventB(
EVENT*          P0509_ps_EventA SEPARATOR
EVENT*          P0510_ps_EventB FINAL_SEPARATOR
{
        REGISTER BOOLEAN L0578_B_Simultaneous;


        return (M030_TIME(P0509_ps_EventA->Map_Time) < M030_TIME(P0510_ps_EventB->Map_Time)) ||
               (((L0578_B_Simultaneous = (M030_TIME(P0509_ps_EventA->Map_Time) == M030_TIME(P0510_ps_EventB->Map_Time))) != C0_FALSE) && (P0509_ps_EventA->A.A.Type > P0510_ps_EventB->A.A.Type)) ||
               (L0578_B_Simultaneous && ((L0578_B_Simultaneous = (P0509_ps_EventA->A.A.Type == P0510_ps_EventB->A.A.Type)) != C0_FALSE) && (P0509_ps_EventA->A.A.Priority > P0510_ps_EventB->A.A.Priority)) ||
               (L0578_B_Simultaneous && (P0509_ps_EventA <= P0510_ps_EventB));
}

unsigned int16_t F0235_TIMELINE_GetIndex(
unsigned int16_t          P0511_ui_EventIndex FINAL_SEPARATOR
{
        REGISTER unsigned int16_t* L0580_pui_TimelineEntry;
        REGISTER int16_t L0579_TimelineIndex;


        for (L0580_pui_TimelineEntry = G0371_pui_Timeline, L0579_TimelineIndex = 0; L0579_TimelineIndex < G2009_i_LargestUsedEventOrdinal; L0579_TimelineIndex++) {
                if (*L0580_pui_TimelineEntry++ == P0511_ui_EventIndex)
                        break;
        }
        if (L0579_TimelineIndex >= G2009_i_LargestUsedEventOrdinal) { /* BUG0_00 Useless code. The function is always called with event indices that are in the timeline */
                F0019_MAIN_DisplayErrorAndStop(C70_ERROR_REQUESTED_EVENT_NOT_IN_TIMELINE);
        }
        return L0579_TimelineIndex;
}

/* This function fixes the placement of the specified timeline entry. When this function returns the first event index in the timeline is the earliest event.
Timeline entries are sorted in a binary tree of nodes so that each parent node refers to an event that will occurs before (or at the same time) as both of its children nodes. The binary tree is stored in an array. Each node contains an event index.
First node is located at index 0 in the array
Each node located at index i in the array has one parent located at index (i - 1) / 2 and two children located at indices 2i + 1 and 2i + 2 */
void F0236_TIMELINE_FixPlacement(
REGISTER unsigned int16_t P0512_ui_TimelineIndex FINAL_SEPARATOR
{
        REGISTER EVENT* L0584_ps_Event;
        REGISTER unsigned int16_t L0581_ui_TimelineIndex;
        REGISTER unsigned int16_t L0583_ui_Multiple;
#define AL0583_ui_EventCount        L0583_ui_Multiple
#define AL0583_ui_HalfTimelineIndex L0583_ui_Multiple
        REGISTER BOOLEAN L0585_B_PlacementFixed;
        REGISTER unsigned int16_t L0582_ui_EventIndex;


        if ((AL0583_ui_EventCount = G0372_ui_EventCount) == 1) /* No placement to fix if there is only one event in the timeline */
                return;
        L0584_ps_Event = &G0370_ps_Events[L0582_ui_EventIndex = G0371_pui_Timeline[P0512_ui_TimelineIndex]]; /* Event specified by the parameter */
        L0585_B_PlacementFixed = C0_FALSE;
        while (P0512_ui_TimelineIndex > 0) { /* Check if the event should be moved earlier in the timeline */
                L0581_ui_TimelineIndex = (P0512_ui_TimelineIndex - 1) >> 1; /* Index of parent node */
                if (F0234_TIMELINE_IsEventABeforeEventB(L0584_ps_Event, &G0370_ps_Events[G0371_pui_Timeline[L0581_ui_TimelineIndex]])) { /* If the specified event occurs before its parent in the tree */
                        G0371_pui_Timeline[P0512_ui_TimelineIndex] = G0371_pui_Timeline[L0581_ui_TimelineIndex];
                        P0512_ui_TimelineIndex = L0581_ui_TimelineIndex;
                        L0585_B_PlacementFixed = C1_TRUE;
                } else {
                        break;
                }
        }
        if (L0585_B_PlacementFixed)
                goto T0236011;
        AL0583_ui_HalfTimelineIndex = ((AL0583_ui_EventCount - 1) - 1) >> 1; /* Index of parent node of the last event */
        while (P0512_ui_TimelineIndex <= AL0583_ui_HalfTimelineIndex) { /* Check if the event should be moved later in the timeline */
                L0581_ui_TimelineIndex = (P0512_ui_TimelineIndex << 1) + 1; /* Index of first child of the node */
                if (((L0581_ui_TimelineIndex + 1) < G0372_ui_EventCount) && (F0234_TIMELINE_IsEventABeforeEventB(&G0370_ps_Events[G0371_pui_Timeline[L0581_ui_TimelineIndex + 1]], &G0370_ps_Events[G0371_pui_Timeline[L0581_ui_TimelineIndex]]))) {
                        L0581_ui_TimelineIndex++;
                }
                if (F0234_TIMELINE_IsEventABeforeEventB(&G0370_ps_Events[G0371_pui_Timeline[L0581_ui_TimelineIndex]], L0584_ps_Event)) {
                        G0371_pui_Timeline[P0512_ui_TimelineIndex] = G0371_pui_Timeline[L0581_ui_TimelineIndex];
                        P0512_ui_TimelineIndex = L0581_ui_TimelineIndex;
                } else {
                        break;
                }
        }
        T0236011:
        G0371_pui_Timeline[P0512_ui_TimelineIndex] = L0582_ui_EventIndex;
}

void F0237_TIMELINE_DeleteEvent(
REGISTER unsigned int16_t P0513_ui_EventIndex FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0586_ui_TimelineIndex;
        REGISTER unsigned int16_t L0587_ui_EventCount;


        G0370_ps_Events[P0513_ui_EventIndex].A.A.Type = C00_EVENT_NONE;
        ((UNUSED_EVENT*)G0370_ps_Events)[P0513_ui_EventIndex].NextUnusedEventIndex = G0373_ui_FirstUnusedEventIndex;
        G0373_ui_FirstUnusedEventIndex = P0513_ui_EventIndex;
        if ((L0587_ui_EventCount = --G0372_ui_EventCount) == 0)
                return;
        L0586_ui_TimelineIndex = F0235_TIMELINE_GetIndex(P0513_ui_EventIndex);
        if (L0586_ui_TimelineIndex == L0587_ui_EventCount) /* If the deleted event is the last in the timeline, no need to fix the timeline */
                return;
        G0371_pui_Timeline[L0586_ui_TimelineIndex] = G0371_pui_Timeline[L0587_ui_EventCount]; /* Replace the deleted event in the timeline by the last event in the timeline */
        F0236_TIMELINE_FixPlacement(L0586_ui_TimelineIndex);
}

int16_t F0652_MergeEvent(
REGISTER EVENT* P2214_ps_Event FINAL_SEPARATOR
{
        REGISTER EVENT* L3031_ps_Event;
        REGISTER int16_t L3032_i_Multiple;
#define A3032_i_EventType  L3032_i_Multiple
#define A3032_i_EventIndex L3032_i_Multiple


        A3032_i_EventType = P2214_ps_Event->A.A.Type;
        if ((A3032_i_EventType >= C05_EVENT_CORRIDOR) && (A3032_i_EventType <= C10_EVENT_DOOR)) {
                L3031_ps_Event = G0370_ps_Events;
                for (A3032_i_EventIndex = 0; A3032_i_EventIndex < G2009_i_LargestUsedEventOrdinal; A3032_i_EventIndex++, L3031_ps_Event++) {
                        if ((L3031_ps_Event->A.A.Type >= C05_EVENT_CORRIDOR) && (L3031_ps_Event->A.A.Type <= C10_EVENT_DOOR)) {
                                if ((P2214_ps_Event->Map_Time == L3031_ps_Event->Map_Time) && (P2214_ps_Event->B.MapXY == L3031_ps_Event->B.MapXY) && ((L3031_ps_Event->A.A.Type != C06_EVENT_WALL) || (L3031_ps_Event->C.A.Cell == P2214_ps_Event->C.A.Cell))) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                        L3031_ps_Event->C.A.Effect = P2214_ps_Event->C.A.Effect;
                                        return A3032_i_EventIndex;
                                }
                                continue;
                        } else {
                                if ((L3031_ps_Event->A.A.Type == C01_EVENT_DOOR_ANIMATION) && (P2214_ps_Event->Map_Time == L3031_ps_Event->Map_Time) && (P2214_ps_Event->B.MapXY == L3031_ps_Event->B.MapXY)) {
                                        if (P2214_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                                                P2214_ps_Event->C.A.Effect = 1 - L3031_ps_Event->C.A.Effect;
                                        }
                                        F0237_TIMELINE_DeleteEvent(A3032_i_EventIndex);
                                        break;
                                }
                        }
                }
        } else {
                if (A3032_i_EventType == C01_EVENT_DOOR_ANIMATION) {
                        L3031_ps_Event = G0370_ps_Events;
                        for (A3032_i_EventIndex = 0; A3032_i_EventIndex < G2009_i_LargestUsedEventOrdinal; A3032_i_EventIndex++, L3031_ps_Event++) {
                                if ((P2214_ps_Event->Map_Time == L3031_ps_Event->Map_Time) && (P2214_ps_Event->B.MapXY == L3031_ps_Event->B.MapXY)) {
                                        if (L3031_ps_Event->A.A.Type == C10_EVENT_DOOR) {
                                                if (L3031_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                                                        L3031_ps_Event->C.A.Effect = 1 - P2214_ps_Event->C.A.Effect;
                                                }
                                                return A3032_i_EventIndex;
                                        }
                                        if (L3031_ps_Event->A.A.Type == C01_EVENT_DOOR_ANIMATION) {
                                                L3031_ps_Event->C.A.Effect = P2214_ps_Event->C.A.Effect;
                                                return A3032_i_EventIndex;
                                        }
                                }
                        }
                } else {
                        if (A3032_i_EventType == C02_EVENT_DOOR_DESTRUCTION) {
                                L3031_ps_Event = G0370_ps_Events;
                                for (A3032_i_EventIndex = 0; A3032_i_EventIndex < G2009_i_LargestUsedEventOrdinal; A3032_i_EventIndex++, L3031_ps_Event++) {
                                        if ((P2214_ps_Event->B.MapXY == L3031_ps_Event->B.MapXY) && (M029_MAP(P2214_ps_Event->Map_Time) == M029_MAP(L3031_ps_Event->Map_Time))) {
                                                if ((L3031_ps_Event->A.A.Type == C01_EVENT_DOOR_ANIMATION) || (L3031_ps_Event->A.A.Type == C10_EVENT_DOOR)) {
                                                        F0237_TIMELINE_DeleteEvent(A3032_i_EventIndex);
                                                }
                                        }
                                }
                        }
                }
        }
        return -1;
}

int16_t F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(
REGISTER EVENT* P0514_ps_Event FINAL_SEPARATOR
{
        REGISTER int16_t L0590_i_NewEventIndex;


        if (P0514_ps_Event->A.A.Type == C00_EVENT_NONE) {
                return -1;
        }
        if (G0372_ui_EventCount == G0369_EventMaximumCount) {
                F0019_MAIN_DisplayErrorAndStop(C45_ERROR_TIMELINE_FULL);
        }
        if ((L0590_i_NewEventIndex = F0652_MergeEvent(P0514_ps_Event)) >= 0) {
                return L0590_i_NewEventIndex;
        }
        L0590_i_NewEventIndex = G0373_ui_FirstUnusedEventIndex;
        G0373_ui_FirstUnusedEventIndex = ((UNUSED_EVENT*)G0370_ps_Events)[L0590_i_NewEventIndex].NextUnusedEventIndex;
        G0370_ps_Events[L0590_i_NewEventIndex] = *P0514_ps_Event; /* Copy the event data (structure assignment) */
        if (G2009_i_LargestUsedEventOrdinal <= L0590_i_NewEventIndex) {
                G2009_i_LargestUsedEventOrdinal = M000_INDEX_TO_ORDINAL(L0590_i_NewEventIndex);
        }
        G0371_pui_Timeline[G0372_ui_EventCount] = L0590_i_NewEventIndex;
        F0236_TIMELINE_FixPlacement(G0372_ui_EventCount++);
        return L0590_i_NewEventIndex;
}

STATICFUNCTION void F0239_TIMELINE_ExtractFirstEvent(
EVENT* P0515_ps_Event FINAL_SEPARATOR
{
        unsigned int16_t L0592_ui_EventIndex;


        *P0515_ps_Event = G0370_ps_Events[L0592_ui_EventIndex = G0371_pui_Timeline[0]]; /* Structure assignment */
        F0237_TIMELINE_DeleteEvent(L0592_ui_EventIndex);
}

STATICFUNCTION BOOLEAN F0240_TIMELINE_IsFirstEventExpired_CPSE(
void
)
{
        return (G0372_ui_EventCount && (M030_TIME(G0370_ps_Events[G0371_pui_Timeline[0]].Map_Time) <= G0313_ul_GameTime))
        ;
}

STATICFUNCTION void F0241_TIMELINE_ProcessEvent1_DoorAnimation(
REGISTER EVENT* P0516_ps_Event FINAL_SEPARATOR
{
        REGISTER unsigned char* L0597_puc_Square;
        REGISTER DOOR* L0598_ps_Door;
        REGISTER int16_t L0596_i_DoorState;
        REGISTER unsigned int16_t L0593_ui_MapX;
        REGISTER unsigned int16_t L0594_ui_MapY;
        REGISTER int16_t L0595_i_Effect;
        REGISTER THING L0599_T_GroupThing;
        unsigned int16_t L0602_ui_Multiple;
#define AL0602_ui_VerticalDoor L0602_ui_Multiple
#define AL0602_ui_Height       L0602_ui_Multiple
        unsigned int16_t L0600_ui_CreatureAttributes;


        L0597_puc_Square = &G0271_ppuc_CurrentMapData[L0593_ui_MapX = P0516_ps_Event->B.Location.MapX][L0594_ui_MapY = P0516_ps_Event->B.Location.MapY];
        if ((L0596_i_DoorState = M036_DOOR_STATE(*L0597_puc_Square)) == C5_DOOR_STATE_DESTROYED)
                return;
        P0516_ps_Event->Map_Time++;
        L0595_i_Effect = P0516_ps_Event->C.A.Effect;
        if (L0595_i_Effect == C01_EFFECT_CLEAR) {
                L0598_ps_Door = (DOOR*)F0157_DUNGEON_GetSquareFirstThingData(L0593_ui_MapX, L0594_ui_MapY);
                AL0602_ui_VerticalDoor = L0598_ps_Door->Vertical;
                if ((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0593_ui_MapX == G0306_i_PartyMapX) && (L0594_ui_MapY == G0307_i_PartyMapY) && (L0596_i_DoorState != C0_DOOR_STATE_OPEN)) {
                        if (G0305_ui_PartyChampionCount > 0) {
                                M037_SET_DOOR_STATE(*L0597_puc_Square, C0_DOOR_STATE_OPEN);
                                if (F0324_CHAMPION_DamageAll_GetDamagedChampionCount(5, MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND, C2_ATTACK_SELF)) { /* BUG0_78 A closing horizontal door wounds champions to the head instead of to the hands. Missing parenthesis in the condition cause all doors to wound the head in addition to the torso because MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor is always non zero. The expected behavior would be for vertical doors to wound torso and head and for horizontal doors to wound hands and torso. The code should be:
                                F0324_CHAMPION_DamageAll_GetDamagedChampionCount(5, MASK0x0008_WOUND_TORSO | (AL0602_ui_VerticalDoor ? MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND), C2_ATTACK_SELF) */
                                        F0064_SOUND_RequestPlay_CPSD(M562_SOUND_PARTY_DAMAGED, L0593_ui_MapX, L0594_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                }
                        }
                        P0516_ps_Event->Map_Time++;
                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0516_ps_Event);
                        return;
                }
                else {
                        if (((L0599_T_GroupThing = F0175_GROUP_GetThing(L0593_ui_MapX, L0594_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) && !M007_GET(L0600_ui_CreatureAttributes = F0144_DUNGEON_GetCreatureAttributes(L0599_T_GroupThing), MASK0x0040_NON_MATERIAL)) {
                                if (L0596_i_DoorState >= (AL0602_ui_Height ? M051_CREATURE_HEIGHT(L0600_ui_CreatureAttributes) : 1)) { /* Creature height or 1 */
                                        if (F0191_GROUP_GetDamageAllCreaturesOutcome((GROUP*)F0156_DUNGEON_GetThingData(L0599_T_GroupThing), L0593_ui_MapX, L0594_ui_MapY, 5, C1_TRUE) != C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP) {
                                                F0209_GROUP_ProcessEvents29to41(L0593_ui_MapX, L0594_ui_MapY, CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE, 0);
                                        }
                                        L0596_i_DoorState = (L0596_i_DoorState == C0_DOOR_STATE_OPEN) ? C0_DOOR_STATE_OPEN : (L0596_i_DoorState - 1);
                                        M037_SET_DOOR_STATE(*L0597_puc_Square, L0596_i_DoorState);
                                        F0064_SOUND_RequestPlay_CPSD(C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, L0593_ui_MapX, L0594_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                        P0516_ps_Event->Map_Time++;
                                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0516_ps_Event);
                                        return;
                                }
                        }
                }
        }
        if (((L0595_i_Effect == C00_EFFECT_SET) && (L0596_i_DoorState == C0_DOOR_STATE_OPEN)) || ((L0595_i_Effect == C01_EFFECT_CLEAR) && (L0596_i_DoorState == C4_DOOR_STATE_CLOSED))) {
                goto T0241020_Return;
        }
        L0596_i_DoorState += (L0595_i_Effect == C00_EFFECT_SET) ? -1 : 1;
        M037_SET_DOOR_STATE(*L0597_puc_Square, L0596_i_DoorState);
        F0064_SOUND_RequestPlay_CPSD(C02_SOUND_DOOR_RATTLE, L0593_ui_MapX, L0594_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
        if (L0595_i_Effect == C00_EFFECT_SET) {
                if (L0596_i_DoorState == C0_DOOR_STATE_OPEN)
                        return;
        } else {
                if (L0596_i_DoorState == C4_DOOR_STATE_CLOSED)
                        return;
        }
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0516_ps_Event);
        T0241020_Return: ;
}

STATICFUNCTION void F0242_TIMELINE_ProcessEvent7_Square_FakeWall(
REGISTER EVENT* P0517_ps_Event FINAL_SEPARATOR
{
        REGISTER unsigned char* L0607_puc_Square;
        REGISTER int16_t L0605_i_Effect;
        REGISTER unsigned int16_t L0603_ui_MapX;
        REGISTER unsigned int16_t L0604_ui_MapY;
        REGISTER THING L0606_T_Thing;


        L0607_puc_Square = &G0271_ppuc_CurrentMapData[L0603_ui_MapX = P0517_ps_Event->B.Location.MapX][L0604_ui_MapY = P0517_ps_Event->B.Location.MapY];
        L0605_i_Effect = P0517_ps_Event->C.A.Effect;
        if (L0605_i_Effect == C02_EFFECT_TOGGLE) {
                L0605_i_Effect = M007_GET(*L0607_puc_Square, MASK0x0004_FAKEWALL_OPEN) ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
        }
        if (L0605_i_Effect == C01_EFFECT_CLEAR) {
                if ((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0603_ui_MapX == G0306_i_PartyMapX) && (L0604_ui_MapY == G0307_i_PartyMapY)) {
                        P0517_ps_Event->Map_Time++;
                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0517_ps_Event);
                } else {
#ifdef PC_FIX_CODE_SIZE
                        if (((L0606_T_Thing = F0175_GROUP_GetThing(L0603_ui_MapX, L0604_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) && !M007_GET(F0144_DUNGEON_GetCreatureAttributes(L0606_T_Thing) + 2, MASK0x0040_NON_MATERIAL)) {
#endif
#ifndef PC_FIX_CODE_SIZE /* CODE COMMENTED TO DECREASE SIZE OF COMPILED CODE */
                        if (((L0606_T_Thing = F0175_GROUP_GetThing(L0603_ui_MapX, L0604_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) && !M007_GET(F0144_DUNGEON_GetCreatureAttributes(L0606_T_Thing), MASK0x0040_NON_MATERIAL)) {
                                P0517_ps_Event->Map_Time++;
                                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0517_ps_Event);
#endif
                        } else {
                                M009_CLEAR(*L0607_puc_Square, MASK0x0004_FAKEWALL_OPEN);
                        }
                }
        } else {
                M008_SET(*L0607_puc_Square, MASK0x0004_FAKEWALL_OPEN);
        }
}

STATICFUNCTION void F0243_TIMELINE_ProcessEvent2_DoorDestruction(
REGISTER EVENT* P0518_ps_Event FINAL_SEPARATOR
{
        REGISTER unsigned char* L0608_puc_Square;


        L0608_puc_Square = &G0271_ppuc_CurrentMapData[P0518_ps_Event->B.Location.MapX][P0518_ps_Event->B.Location.MapY];
        M037_SET_DOOR_STATE(*L0608_puc_Square, C5_DOOR_STATE_DESTROYED);
}

STATICFUNCTION void F0244_TIMELINE_ProcessEvent10_Square_Door(
REGISTER EVENT* P0519_ps_Event FINAL_SEPARATOR
{
        REGISTER int16_t L0609_i_DoorState;


        if ((L0609_i_DoorState = M036_DOOR_STATE(G0271_ppuc_CurrentMapData[P0519_ps_Event->B.Location.MapX][P0519_ps_Event->B.Location.MapY])) == C5_DOOR_STATE_DESTROYED) {
                return;
        }
        if (P0519_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                P0519_ps_Event->C.A.Effect = (L0609_i_DoorState == C0_DOOR_STATE_OPEN) ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
        } else {
                if (P0519_ps_Event->C.A.Effect == C00_EFFECT_SET) {
                        if (L0609_i_DoorState == C0_DOOR_STATE_OPEN)
                                return;
                } else {
                        if (L0609_i_DoorState == C4_DOOR_STATE_CLOSED)
                                return;
                }
        }
        P0519_ps_Event->A.A.Type = C01_EVENT_DOOR_ANIMATION;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0519_ps_Event);
}

STATICFUNCTION void F0245_TIMELINE_ProcessEvent5_Square_Corridor(
REGISTER EVENT* P0520_ps_Event FINAL_SEPARATOR
{
        REGISTER SENSOR* L0614_ps_Sensor;
        REGISTER TEXTSTRING* L0615_ps_TextString;
        REGISTER THING L0613_T_Thing;
#define AL0618_ui_HealthMultiplier L0618_ui_Multiple
#define AL0618_ui_Ticks            L0618_ui_Multiple
        REGISTER unsigned int16_t L0618_ui_Multiple;
        REGISTER unsigned int16_t L0612_ui_CreatureCount;
        REGISTER unsigned int16_t L0616_ui_MapX;
        REGISTER unsigned int16_t L0617_ui_MapY;
        REGISTER BOOLEAN L0611_B_TextCurrentlyVisible;
        REGISTER int16_t L0610_i_ThingType;
        EVENT L0619_s_Event;


        L0613_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0616_ui_MapX = P0520_ps_Event->B.Location.MapX, L0617_ui_MapY = P0520_ps_Event->B.Location.MapY);
        while (L0613_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if ((L0610_i_ThingType = M012_TYPE(L0613_T_Thing)) == C02_THING_TYPE_TEXTSTRING) {
                        L0615_ps_TextString = (TEXTSTRING*)F0156_DUNGEON_GetThingData(L0613_T_Thing);
                        L0611_B_TextCurrentlyVisible = L0615_ps_TextString->Visible;
                        if (P0520_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                                L0615_ps_TextString->Visible = !L0611_B_TextCurrentlyVisible;
                        } else {
                                L0615_ps_TextString->Visible = (P0520_ps_Event->C.A.Effect == C00_EFFECT_SET);
                        }
                        if (!L0611_B_TextCurrentlyVisible && L0615_ps_TextString->Visible && (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0616_ui_MapX == G0306_i_PartyMapX) && (L0617_ui_MapY == G0307_i_PartyMapY)) {
                                F0168_DUNGEON_DecodeText(M774_CAST_PUC(G0353_ac_StringBuildBuffer), L0613_T_Thing, C1_TEXT_TYPE_MESSAGE);
                                F0047_TEXT_MESSAGEAREA_PrintMessage(C15_COLOR_WHITE, G0353_ac_StringBuildBuffer);
                        }
                } else {
                        if (L0610_i_ThingType == C03_THING_TYPE_SENSOR) {
                                L0614_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0613_T_Thing);
                                if (M039_TYPE(L0614_ps_Sensor) == C006_SENSOR_FLOOR_GROUP_GENERATOR) {
                                        L0612_ui_CreatureCount = L0614_ps_Sensor->Remote.Value;
                                        if (M007_GET(L0612_ui_CreatureCount, MASK0x0008_RANDOMIZE_GENERATED_CREATURE_COUNT)) {
                                                L0612_ui_CreatureCount = M002_RANDOM(M007_GET(L0612_ui_CreatureCount, MASK0x0007_GENERATED_CREATURE_COUNT));
                                        } else {
                                                L0612_ui_CreatureCount--;
                                        }
                                        if ((AL0618_ui_HealthMultiplier = M045_HEALTH_MULTIPLIER(L0614_ps_Sensor)) == 0) {
                                                AL0618_ui_HealthMultiplier = G0269_ps_CurrentMap->C.Difficulty;
                                        }
                                        F0185_GROUP_GetGenerated(M040_DATA(L0614_ps_Sensor), AL0618_ui_HealthMultiplier, L0612_ui_CreatureCount, M004_RANDOM(4), L0616_ui_MapX, L0617_ui_MapY);
                                        if (L0614_ps_Sensor->Remote.Audible) {
                                                F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, L0616_ui_MapX, L0617_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                        }
                                        if (L0614_ps_Sensor->Remote.OnceOnly) {
                                                M044_SET_TYPE_DISABLED(L0614_ps_Sensor);
                                        } else {
                                                if ((AL0618_ui_Ticks = M046_TICKS(L0614_ps_Sensor)) != 0) {
                                                        M044_SET_TYPE_DISABLED(L0614_ps_Sensor);
                                                        if (AL0618_ui_Ticks > 127) {
                                                                AL0618_ui_Ticks = (AL0618_ui_Ticks - 126) << 6;
                                                        }
                                                        L0619_s_Event.A.A.Type = C65_EVENT_ENABLE_GROUP_GENERATOR;
                                                        M033_SET_MAP_AND_TIME(L0619_s_Event.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime + AL0618_ui_Ticks);
                                                        L0619_s_Event.A.A.Priority = 0;
                                                        L0619_s_Event.B.Location.MapX = L0616_ui_MapX;
                                                        L0619_s_Event.B.Location.MapY = L0617_ui_MapY;
                                                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0619_s_Event);
                                                }
                                        }
                                }
                        }
                }
                L0613_T_Thing = F0159_DUNGEON_GetNextThing(L0613_T_Thing);
        }
}

/* This function enables the first disabled sensor on the square by changing its type to C006_SENSOR_FLOOR_GROUP_GENERATOR */
STATICFUNCTION void F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator(
REGISTER EVENT* P0521_ps_Event FINAL_SEPARATOR
{
        REGISTER SENSOR* L0621_ps_Sensor;
        REGISTER THING L0620_T_Thing;


        L0620_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0521_ps_Event->B.Location.MapX, P0521_ps_Event->B.Location.MapY);
        while (L0620_T_Thing != C0xFFFF_THING_NONE) {
                if (M012_TYPE(L0620_T_Thing) == C03_THING_TYPE_SENSOR) {
                        L0621_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0620_T_Thing);
                        if (M039_TYPE(L0621_ps_Sensor) == C000_SENSOR_DISABLED) {
                                L0621_ps_Sensor->Remote.Type_Data |= C006_SENSOR_FLOOR_GROUP_GENERATOR;
                                return;
                        }
                }
                L0620_T_Thing = F0159_DUNGEON_GetNextThing(L0620_T_Thing);
        }
}

STATICFUNCTION void F0247_TIMELINE_TriggerProjectileLauncher(
REGISTER SENSOR* P0522_ps_Sensor SEPARATOR
REGISTER EVENT*  P0523_ps_Event  FINAL_SEPARATOR
{
        REGISTER THING L0622_T_FirstProjectileAssociatedThing;
        REGISTER THING L0623_T_SecondProjectileAssociatedThing;
        REGISTER unsigned int16_t L0624_ui_Cell;
        REGISTER int16_t L0625_i_SensorType;
        REGISTER int16_t L0626_i_MapX;
        int16_t L0627_i_MapY;
        BOOLEAN L0632_B_LaunchSingleProjectile;
        unsigned int16_t L0633_ui_ThingCell;
        unsigned int16_t L0628_ui_ProjectileCell;
        int16_t L0629_i_SensorData;
        int16_t L0630_i_KineticEnergy;
        int16_t L0631_i_StepEnergy;


        L0626_i_MapX = P0523_ps_Event->B.Location.MapX;
        L0627_i_MapY = P0523_ps_Event->B.Location.MapY;
        L0624_ui_Cell = P0523_ps_Event->C.A.Cell;
        L0628_ui_ProjectileCell = M018_OPPOSITE(L0624_ui_Cell);
        L0625_i_SensorType = M039_TYPE(P0522_ps_Sensor);
        L0629_i_SensorData = M040_DATA(P0522_ps_Sensor);
        L0630_i_KineticEnergy = M047_KINETIC_ENERGY(P0522_ps_Sensor);
        L0631_i_StepEnergy = M048_STEP_ENERGY(P0522_ps_Sensor);
        L0632_B_LaunchSingleProjectile = (L0625_i_SensorType == C007_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT) ||
                                         (L0625_i_SensorType == C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION) ||
                                         (L0625_i_SensorType == C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT);
        if ((L0625_i_SensorType == C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION) || (L0625_i_SensorType == C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION)) {
                L0622_T_FirstProjectileAssociatedThing = L0623_T_SecondProjectileAssociatedThing = L0629_i_SensorData + C0xFF80_THING_FIRST_EXPLOSION;
        } else {
                if ((L0625_i_SensorType == C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT) || (L0625_i_SensorType == C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT)) {
                        L0622_T_FirstProjectileAssociatedThing = F0161_DUNGEON_GetSquareFirstThing(L0626_i_MapX, L0627_i_MapY);
                        while (L0622_T_FirstProjectileAssociatedThing != C0xFFFF_THING_NONE) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. C0xFFFF_THING_NONE should be C0xFFFE_THING_ENDOFLIST. If there are no more objects on the square then this loop may return an undefined value, this can crash the game. In the original DM and CSB dungeons, the number of times that these sensors are triggered is always controlled to be equal to the number of available objects (with a countdown sensor or a number of once only sensors) */
                                L0633_ui_ThingCell = M011_CELL(L0622_T_FirstProjectileAssociatedThing);
                                if ((M012_TYPE(L0622_T_FirstProjectileAssociatedThing) > C03_THING_TYPE_SENSOR) && ((L0633_ui_ThingCell == L0624_ui_Cell) || (L0633_ui_ThingCell == M017_NEXT(L0624_ui_Cell))))
                                        break;
                                L0622_T_FirstProjectileAssociatedThing = F0159_DUNGEON_GetNextThing(L0622_T_FirstProjectileAssociatedThing);
                        }
                        if (L0622_T_FirstProjectileAssociatedThing == C0xFFFF_THING_NONE) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. C0xFFFF_THING_NONE should be C0xFFFE_THING_ENDOFLIST */
                                return;
                        }
                        F0164_DUNGEON_UnlinkThingFromList(L0622_T_FirstProjectileAssociatedThing, 0, L0626_i_MapX, L0627_i_MapY); /* The object is removed without triggering any sensor effects */
                        if (!L0632_B_LaunchSingleProjectile) {
                                L0623_T_SecondProjectileAssociatedThing = F0161_DUNGEON_GetSquareFirstThing(L0626_i_MapX, L0627_i_MapY);
                                while (L0623_T_SecondProjectileAssociatedThing != C0xFFFF_THING_NONE) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. C0xFFFF_THING_NONE should be C0xFFFE_THING_ENDOFLIST. If there are no more objects on the square then this loop may return an undefined value, this can crash the game */
                                        L0633_ui_ThingCell = M011_CELL(L0623_T_SecondProjectileAssociatedThing);
                                        if ((M012_TYPE(L0623_T_SecondProjectileAssociatedThing) > C03_THING_TYPE_SENSOR) && ((L0633_ui_ThingCell == L0624_ui_Cell) || (L0633_ui_ThingCell == M017_NEXT(L0624_ui_Cell))))
                                                break;
                                        L0623_T_SecondProjectileAssociatedThing = F0159_DUNGEON_GetNextThing(L0623_T_SecondProjectileAssociatedThing);
                                }
                                if (L0623_T_SecondProjectileAssociatedThing == C0xFFFF_THING_NONE) { /* BUG0_19 The game crashes when an object launcher sensor is triggered. C0xFFFF_THING_NONE should be C0xFFFE_THING_ENDOFLIST */
                                        L0632_B_LaunchSingleProjectile = C1_TRUE;
                                } else {
                                        F0164_DUNGEON_UnlinkThingFromList(L0623_T_SecondProjectileAssociatedThing, 0, L0626_i_MapX, L0627_i_MapY); /* The object is removed without triggering any sensor effects */
                                }
                        }
                } else {
                        if ((L0622_T_FirstProjectileAssociatedThing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(L0629_i_SensorData)) == C0xFFFF_THING_NONE) {
                                return;
                        }
                        if (!L0632_B_LaunchSingleProjectile && ((L0623_T_SecondProjectileAssociatedThing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(L0629_i_SensorData)) == C0xFFFF_THING_NONE)) {
                                L0632_B_LaunchSingleProjectile = C1_TRUE;
                        }
                }
        }
        if (L0632_B_LaunchSingleProjectile) {
                L0628_ui_ProjectileCell = M021_NORMALIZE(L0628_ui_ProjectileCell + M005_RANDOM(2));
        }
        L0626_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[L0624_ui_Cell], L0627_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[L0624_ui_Cell]; /* BUG0_20 The game crashes if the launcher sensor is on a map boundary and shoots in a direction outside the map */
        G0365_B_CreateLauncherProjectile = C1_TRUE;
        F0212_PROJECTILE_Create(L0622_T_FirstProjectileAssociatedThing, L0626_i_MapX, L0627_i_MapY, L0628_ui_ProjectileCell, L0624_ui_Cell, L0630_i_KineticEnergy, 100, L0631_i_StepEnergy);
        if (!L0632_B_LaunchSingleProjectile) {
                F0212_PROJECTILE_Create(L0623_T_SecondProjectileAssociatedThing, L0626_i_MapX, L0627_i_MapY, M017_NEXT(L0628_ui_ProjectileCell), L0624_ui_Cell, L0630_i_KineticEnergy, 100, L0631_i_StepEnergy);
        }
        G0365_B_CreateLauncherProjectile = C0_FALSE;
}

STATICFUNCTION void F0248_TIMELINE_ProcessEvent6_Square_Wall(
REGISTER EVENT* P0524_ps_Event FINAL_SEPARATOR
{
        REGISTER SENSOR* L0638_ps_Sensor;
        REGISTER TEXTSTRING* L0639_ps_TextString;
        REGISTER unsigned int16_t L0637_ui_SensorData;
        REGISTER int16_t L0636_i_Multiple;
#define AL0636_B_TriggerSetEffect L0636_i_Multiple
#define AL0636_i_BitMask          L0636_i_Multiple
        REGISTER THING L0634_T_Thing;
        REGISTER unsigned int16_t L0640_ui_SensorType;
        REGISTER int16_t L0641_i_MapX;
        int16_t L0642_i_MapY;
        unsigned int16_t L0643_ui_Cell;
        REGISTER int16_t L0635_i_ThingType;


        L0634_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0641_i_MapX = P0524_ps_Event->B.Location.MapX, L0642_i_MapY = P0524_ps_Event->B.Location.MapY);
        L0643_ui_Cell = P0524_ps_Event->C.A.Cell;
        while (L0634_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if (((L0635_i_ThingType = M012_TYPE(L0634_T_Thing)) == C02_THING_TYPE_TEXTSTRING) && (M011_CELL(L0634_T_Thing) == P0524_ps_Event->C.A.Cell)) {
                        L0639_ps_TextString = (TEXTSTRING*)F0156_DUNGEON_GetThingData(L0634_T_Thing);
                        if (P0524_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                                L0639_ps_TextString->Visible = !L0639_ps_TextString->Visible;
                        } else {
#ifdef PC_FIX_CODE_SIZE
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
#endif
                                L0639_ps_TextString->Visible = (P0524_ps_Event->C.A.Effect == C00_EFFECT_SET);
                        }
                } else {
                        if (L0635_i_ThingType == C03_THING_TYPE_SENSOR) {
                                L0638_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0634_T_Thing);
                                L0640_ui_SensorType = M039_TYPE(L0638_ps_Sensor);
                                L0637_ui_SensorData = M040_DATA(L0638_ps_Sensor);
                                if (L0640_ui_SensorType == C006_SENSOR_WALL_COUNTDOWN) {
                                        if (L0637_ui_SensorData > 0) {
                                                if (P0524_ps_Event->C.A.Effect == C00_EFFECT_SET) {
                                                        if (L0637_ui_SensorData < 511) {
                                                                L0637_ui_SensorData++;
                                                        }
                                                } else {
                                                        L0637_ui_SensorData--;
                                                }
                                                M041_SET_DATA(L0638_ps_Sensor, L0637_ui_SensorData);
                                                if (L0638_ps_Sensor->Remote.Effect == C03_EFFECT_HOLD) {
                                                        AL0636_B_TriggerSetEffect = ((L0637_ui_SensorData == 0) != L0638_ps_Sensor->Remote.RevertEffect);
#ifdef PC_FIX_CODE_SIZE
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
#endif
                                                        F0272_SENSOR_TriggerEffect(L0638_ps_Sensor, AL0636_B_TriggerSetEffect ? C00_EFFECT_SET : C01_EFFECT_CLEAR, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
                                                } else {
                                                        if (L0637_ui_SensorData == 0) {
#ifdef PC_FIX_CODE_SIZE
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
#endif
                                                                F0272_SENSOR_TriggerEffect(L0638_ps_Sensor, L0638_ps_Sensor->Remote.Effect, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
                                                        }
                                                }
                                        }
                                } else {
                                        if (L0640_ui_SensorType == C005_SENSOR_WALL_AND_OR_GATE) {
                                                AL0636_i_BitMask = 1 << (P0524_ps_Event->C.A.Cell);
                                                if (P0524_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                                                        if (M007_GET(L0637_ui_SensorData, AL0636_i_BitMask)) {
                                                                M009_CLEAR(L0637_ui_SensorData, AL0636_i_BitMask);
                                                        } else {
                                                                M008_SET(L0637_ui_SensorData, AL0636_i_BitMask);
                                                        }
                                                } else {
                                                        if (P0524_ps_Event->C.A.Effect) {
#ifdef PC_FIX_CODE_SIZE
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
#endif
                                                                M009_CLEAR(L0637_ui_SensorData, AL0636_i_BitMask);
                                                        } else {
                                                                M008_SET(L0637_ui_SensorData, AL0636_i_BitMask);
                                                        }
                                                }
                                                M041_SET_DATA(L0638_ps_Sensor, L0637_ui_SensorData);
                                                AL0636_B_TriggerSetEffect = (M042_MASK_CURRENT(L0637_ui_SensorData) == M043_MASK_REFERENCE(L0637_ui_SensorData)) != L0638_ps_Sensor->Remote.RevertEffect;
                                                if (L0638_ps_Sensor->Remote.Effect == C03_EFFECT_HOLD) {
#ifdef PC_FIX_CODE_SIZE
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
        L0637_ui_SensorData++;
#endif
                                                        F0272_SENSOR_TriggerEffect(L0638_ps_Sensor, AL0636_B_TriggerSetEffect ? C00_EFFECT_SET : C01_EFFECT_CLEAR, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
                                                } else {
                                                        if (AL0636_B_TriggerSetEffect) {
                                                                F0272_SENSOR_TriggerEffect(L0638_ps_Sensor, L0638_ps_Sensor->Remote.Effect, L0641_i_MapX, L0642_i_MapY, L0643_ui_Cell);
                                                        }
                                                }
                                        } else {
                                                if ((((L0640_ui_SensorType >= C007_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT) && (L0640_ui_SensorType <= C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION)) || (L0640_ui_SensorType == C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT) || (L0640_ui_SensorType == C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT)) && (M011_CELL(L0634_T_Thing) == P0524_ps_Event->C.A.Cell)) {
                                                        F0247_TIMELINE_TriggerProjectileLauncher(L0638_ps_Sensor, P0524_ps_Event);
                                                        if (L0638_ps_Sensor->Remote.OnceOnly) {
                                                                M044_SET_TYPE_DISABLED(L0638_ps_Sensor);
                                                        }
                                                } else {
                                                        if (L0640_ui_SensorType == C018_SENSOR_WALL_END_GAME) {
                                                                F0022_MAIN_Delay(60 * L0638_ps_Sensor->Remote.Value);
                                                                G0524_B_RestartGameAllowed = C0_FALSE;
                                                                G0302_B_GameWon = C1_TRUE;
                                                                F0444_STARTEND_Endgame(C1_TRUE);
                                                        }
                                                }
                                        }
                                }
                        }
                }
                L0634_T_Thing = F0159_DUNGEON_GetNextThing(L0634_T_Thing);
        }
        F0271_SENSOR_ProcessRotationEffect();
}

/* This function moves all things on the specified square to the same location in order to apply the square effects when a teleporter or pit is open */
STATICFUNCTION void F0249_TIMELINE_MoveTeleporterOrPitSquareThings(
REGISTER unsigned int16_t P0525_ui_MapX SEPARATOR
REGISTER unsigned int16_t P0526_ui_MapY FINAL_SEPARATOR
{
        REGISTER EVENT* L0647_ps_Event;
        REGISTER PROJECTILE* L0646_ps_Projectile;
        REGISTER THING L0645_T_Thing;
        REGISTER unsigned int16_t L0644_ui_Multiple;
#define AL0644_ui_ThingType  L0644_ui_Multiple
#define AL0644_ui_EventIndex L0644_ui_Multiple
        REGISTER THING L0648_T_NextThing;
        int16_t L0649_i_ThingsToMoveCount;


        if ((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (P0525_ui_MapX == G0306_i_PartyMapX) && (P0526_ui_MapY == G0307_i_PartyMapY)) {
                F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, P0525_ui_MapX, P0526_ui_MapY, P0525_ui_MapX, P0526_ui_MapY);
                F0296_CHAMPION_DrawChangedObjectIcons();
        }
        if ((L0645_T_Thing = F0175_GROUP_GetThing(P0525_ui_MapX, P0526_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) {
                F0267_MOVE_GetMoveResult_CPSCE(L0645_T_Thing, P0525_ui_MapX, P0526_ui_MapY, P0525_ui_MapX, P0526_ui_MapY);
        }
        L0645_T_Thing = F0162_DUNGEON_GetSquareFirstObject(P0525_ui_MapX, P0526_ui_MapY);
        L0648_T_NextThing = L0645_T_Thing;
        L0649_i_ThingsToMoveCount = 0;
        while (L0645_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if (M012_TYPE(L0645_T_Thing) > C04_THING_TYPE_GROUP) {
                        L0649_i_ThingsToMoveCount++;
                }
                L0645_T_Thing = F0159_DUNGEON_GetNextThing(L0645_T_Thing);
        }
        L0645_T_Thing = L0648_T_NextThing;
        while ((L0645_T_Thing != C0xFFFE_THING_ENDOFLIST) && L0649_i_ThingsToMoveCount) {
                L0649_i_ThingsToMoveCount--;
                L0648_T_NextThing = F0159_DUNGEON_GetNextThing(L0645_T_Thing);
                AL0644_ui_ThingType = M012_TYPE(L0645_T_Thing);
                if (AL0644_ui_ThingType > C04_THING_TYPE_GROUP) {
                        F0267_MOVE_GetMoveResult_CPSCE(L0645_T_Thing, P0525_ui_MapX, P0526_ui_MapY, P0525_ui_MapX, P0526_ui_MapY);
                }
                if (AL0644_ui_ThingType == C14_THING_TYPE_PROJECTILE) {
                        L0646_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(L0645_T_Thing);
                        L0647_ps_Event = &G0370_ps_Events[L0646_ps_Projectile->EventIndex];
                        L0647_ps_Event->C.Projectile.MapX = G0397_i_MoveResultMapX;
                        L0647_ps_Event->C.Projectile.MapY = G0398_i_MoveResultMapY;
                        L0647_ps_Event->C.Projectile.Direction = G0400_i_MoveResultDirection;
                        L0647_ps_Event->B.Slot = M015_THING_WITH_NEW_CELL(L0645_T_Thing, G0401_ui_MoveResultCell);
                        M031_SET_MAP(L0647_ps_Event->Map_Time, G0399_ui_MoveResultMapIndex);
                } else {
                        if (AL0644_ui_ThingType == C15_THING_TYPE_EXPLOSION) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                L0647_ps_Event = G0370_ps_Events;
                                for (AL0644_ui_EventIndex = 0; AL0644_ui_EventIndex < G2009_i_LargestUsedEventOrdinal; L0647_ps_Event++, AL0644_ui_EventIndex++) {
                                        if ((L0647_ps_Event->A.A.Type == C25_EVENT_EXPLOSION) && (L0647_ps_Event->C.Slot == L0645_T_Thing)) { /* BUG0_23 A Fluxcage explosion remains on a square forever. If you open a pit or teleporter on a square where there is a Fluxcage explosion, the Fluxcage explosion is moved but the associated event is not updated (because Fluxcage explosions do not use C25_EVENT_EXPLOSION but rather C24_EVENT_REMOVE_FLUXCAGE) causing the Fluxcage explosion to remain in the dungeon forever on its destination square. When the C24_EVENT_REMOVE_FLUXCAGE expires the explosion thing is not removed, but it is marked as unused. Consequently, any objects placed on the Fluxcage square after it was moved but before it expires become orphans upon expiration. After expiration, any object placed on the fluxcage square is cloned when picked up */
                                                L0647_ps_Event->B.Location.MapX = G0397_i_MoveResultMapX;
                                                L0647_ps_Event->B.Location.MapY = G0398_i_MoveResultMapY;
                                                L0647_ps_Event->C.Slot = M015_THING_WITH_NEW_CELL(L0645_T_Thing, G0401_ui_MoveResultCell);
                                                M031_SET_MAP(L0647_ps_Event->Map_Time, G0399_ui_MoveResultMapIndex);
                                        }
                                }
                        }
                }
                L0645_T_Thing = L0648_T_NextThing;
        }
}

STATICFUNCTION void F0250_TIMELINE_ProcessEvent8_Square_Teleporter(
REGISTER EVENT* P0527_ps_Event FINAL_SEPARATOR
{
        REGISTER unsigned char* L0652_puc_Square;
        REGISTER unsigned int16_t L0650_ui_MapX;
        REGISTER unsigned int16_t L0651_ui_MapY;


        L0652_puc_Square = &G0271_ppuc_CurrentMapData[L0650_ui_MapX = P0527_ps_Event->B.Location.MapX][L0651_ui_MapY = P0527_ps_Event->B.Location.MapY];
        if (P0527_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                P0527_ps_Event->C.A.Effect = M007_GET(*L0652_puc_Square, MASK0x0008_TELEPORTER_OPEN) ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
        }
        if (P0527_ps_Event->C.A.Effect == C00_EFFECT_SET) {
                M008_SET(*L0652_puc_Square, MASK0x0008_TELEPORTER_OPEN);
                F0249_TIMELINE_MoveTeleporterOrPitSquareThings(L0650_ui_MapX, L0651_ui_MapY);
        } else {
                M009_CLEAR(*L0652_puc_Square, MASK0x0008_TELEPORTER_OPEN);
        }
}

STATICFUNCTION void F0251_TIMELINE_ProcessEvent9_Square_Pit(
REGISTER EVENT* P0528_ps_Event FINAL_SEPARATOR
{
        REGISTER unsigned char* L0655_puc_Square;
        REGISTER unsigned int16_t L0653_ui_MapX;
        REGISTER unsigned int16_t L0654_ui_MapY;


        L0655_puc_Square = &G0271_ppuc_CurrentMapData[L0653_ui_MapX = P0528_ps_Event->B.Location.MapX][L0654_ui_MapY = P0528_ps_Event->B.Location.MapY];
        if (P0528_ps_Event->C.A.Effect == C02_EFFECT_TOGGLE) {
                P0528_ps_Event->C.A.Effect = M007_GET(*L0655_puc_Square, MASK0x0008_PIT_OPEN) ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
        }
        if (P0528_ps_Event->C.A.Effect == C00_EFFECT_SET) {
                M008_SET(*L0655_puc_Square, MASK0x0008_PIT_OPEN);
                F0249_TIMELINE_MoveTeleporterOrPitSquareThings(L0653_ui_MapX, L0654_ui_MapY);
        } else {
                M009_CLEAR(*L0655_puc_Square, MASK0x0008_PIT_OPEN);
        }
}

STATICFUNCTION void F0252_TIMELINE_ProcessEvents60to61_MoveGroup(
REGISTER EVENT* P0529_ps_Event FINAL_SEPARATOR
{
        REGISTER GROUP* L0658_ps_Group;
        REGISTER unsigned int16_t L0656_ui_MapX;
        REGISTER unsigned int16_t L0657_ui_MapY;
        REGISTER BOOLEAN L0659_B_RandomDirectionMoveRetried;


        L0659_B_RandomDirectionMoveRetried = C0_FALSE;
        L0656_ui_MapX = P0529_ps_Event->B.Location.MapX;
        L0657_ui_MapY = P0529_ps_Event->B.Location.MapY;
        T0252001:
        if (!((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0656_ui_MapX == G0306_i_PartyMapX) && (L0657_ui_MapY == G0307_i_PartyMapY)) && (F0175_GROUP_GetThing(L0656_ui_MapX, L0657_ui_MapY) == C0xFFFE_THING_ENDOFLIST)) { /* BUG0_24 Lord Chaos may teleport into one of the Black Flames and become invisible until the Black Flame is killed. In this case, F0175_GROUP_GetThing returns the Black Flame thing and the Lord Chaos thing is not moved into the dungeon until the Black Flame is killed */
                if (P0529_ps_Event->A.A.Type == C61_EVENT_MOVE_GROUP_AUDIBLE) {
                        F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, L0656_ui_MapX, L0657_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                }
                F0267_MOVE_GetMoveResult_CPSCE(P0529_ps_Event->C.Slot, CM1_MAPX_NOT_ON_A_SQUARE, 0, L0656_ui_MapX, L0657_ui_MapY);
        } else {
                if (!L0659_B_RandomDirectionMoveRetried) {
                        L0659_B_RandomDirectionMoveRetried = C1_TRUE;
                        L0658_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0529_ps_Event->C.Slot);
                        if ((L0658_ps_Group->Type == C23_CREATURE_LORD_CHAOS) && !M004_RANDOM(4)) {
                                switch (M004_RANDOM(4)) {
                                        case 0:
                                                L0656_ui_MapX--;
                                                break;
                                        case 1:
                                                L0656_ui_MapX++;
                                                break;
                                        case 2:
                                                L0657_ui_MapY--;
                                                break;
                                        case 3:
                                                L0657_ui_MapY++;
                                }
                                if (F0223_GROUP_IsLordChaosAllowed(L0656_ui_MapX, L0657_ui_MapY))
                                        goto T0252001;
                        }
                        goto T0252002;
                }
                else {
                        T0252002:
                        P0529_ps_Event->Map_Time += 5;
                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0529_ps_Event);
                }
        }
}

STATICFUNCTION void F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction(
REGISTER unsigned int16_t P0530_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0662_ps_Champion;
        REGISTER int16_t L0661_i_QuiverSlotIndex;
        REGISTER int16_t L0660_i_SlotIndex;


        L0662_ps_Champion = &M516_CHAMPIONS[P0530_ui_ChampionIndex];
        L0662_ps_Champion->EnableActionEventIndex = -1;
        M009_CLEAR(L0662_ps_Champion->Attributes, MASK0x0008_DISABLE_ACTION);
        if (L0662_ps_Champion->ActionIndex != C0xFF_ACTION_NONE) {
                L0662_ps_Champion->ActionDefense -= G0495_ac_Graphic560_ActionDefense[L0662_ps_Champion->ActionIndex];
        }
        if (L0662_ps_Champion->CurrentHealth) {
                if ((L0662_ps_Champion->ActionIndex == C032_ACTION_SHOOT) && (L0662_ps_Champion->Slots[C00_SLOT_READY_HAND] == C0xFFFF_THING_NONE)) {
                        if (F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon(P0530_ui_ChampionIndex, C01_SLOT_ACTION_HAND, L0660_i_SlotIndex = C12_SLOT_QUIVER_LINE1_1)) {
                                T0253002:
                                F0301_CHAMPION_AddObjectInSlot(P0530_ui_ChampionIndex, F0300_CHAMPION_GetObjectRemovedFromSlot(P0530_ui_ChampionIndex, L0660_i_SlotIndex), C00_SLOT_READY_HAND);
                        } else {
                                for (L0661_i_QuiverSlotIndex = 0; L0661_i_QuiverSlotIndex < 3; L0661_i_QuiverSlotIndex++) {
                                        if (F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon(P0530_ui_ChampionIndex, C01_SLOT_ACTION_HAND, L0660_i_SlotIndex = L0661_i_QuiverSlotIndex + C07_SLOT_QUIVER_LINE2_1))
                                                goto T0253002;
                                }
                        }
                }
                M008_SET(L0662_ps_Champion->Attributes, MASK0x8000_ACTION_HAND);
                F0292_CHAMPION_DrawState(P0530_ui_ChampionIndex);
        }
        L0662_ps_Champion->ActionIndex = C0xFF_ACTION_NONE;
}

STATICFUNCTION void F0254_TIMELINE_ProcessEvent12_HideDamageReceived(
REGISTER unsigned int16_t P0531_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0663_ps_Champion;


        L0663_ps_Champion = &M516_CHAMPIONS[P0531_ui_ChampionIndex];
        L0663_ps_Champion->HideDamageReceivedEventIndex = -1;
        if (!L0663_ps_Champion->CurrentHealth) {
                return;
        }
        if (M000_INDEX_TO_ORDINAL(P0531_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal) {
                F0077_MOUSE_EnableScreenUpdate_CPSE();
                F0354_INVENTORY_DrawStatusBoxPortrait(P0531_ui_ChampionIndex);
                F0078_MOUSE_DisableScreenUpdate();
        } else {
                M008_SET(L0663_ps_Champion->Attributes, MASK0x0080_NAME_TITLE);
                F0292_CHAMPION_DrawState(P0531_ui_ChampionIndex);
        }
}

STATICFUNCTION void F0255_TIMELINE_ProcessEvent13_ViAltarRebirth(
REGISTER EVENT* P0532_ps_Event FINAL_SEPARATOR
{
        REGISTER JUNK* L0668_ps_Junk;
        REGISTER THING L0667_T_Thing;
        REGISTER int16_t L0664_i_MapX;
        REGISTER int16_t L0665_i_MapY;
        REGISTER unsigned int16_t L0666_ui_Cell;
        REGISTER int16_t L0669_i_IconIndex;
        unsigned int16_t L0671_ui_ChampionIndex;
        unsigned int16_t L0670_ui_Step;


        L0664_i_MapX = P0532_ps_Event->B.Location.MapX;
        L0665_i_MapY = P0532_ps_Event->B.Location.MapY;
        L0666_ui_Cell = P0532_ps_Event->C.A.Cell;
        L0671_ui_ChampionIndex = P0532_ps_Event->A.A.Priority;
        switch (L0670_ui_Step = P0532_ps_Event->C.A.Effect) { /* Rebirth is a 3 steps process (Step 2 -> Step 1 -> Step 0). Step is stored in the Effect value of the event */
                case 2:
                        F0213_EXPLOSION_Create(C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1, 0, L0664_i_MapX, L0665_i_MapY, L0666_ui_Cell);
                        P0532_ps_Event->Map_Time += 5;
                        T0255002:
                        P0532_ps_Event->C.A.Effect = --L0670_ui_Step;
                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(P0532_ps_Event);
                        break;
                case 1:
                        L0667_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0664_i_MapX, L0665_i_MapY);
                        while (L0667_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                                if ((M011_CELL(L0667_T_Thing) == L0666_ui_Cell) && (M012_TYPE(L0667_T_Thing) == C10_THING_TYPE_JUNK)) {
                                        L0669_i_IconIndex = F0033_OBJECT_GetIconIndex(L0667_T_Thing);
                                        if (L0669_i_IconIndex == C147_ICON_JUNK_CHAMPION_BONES) {
                                                L0668_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L0667_T_Thing);
                                                if (L0668_ps_Junk->ChargeCount == L0671_ui_ChampionIndex) {
                                                        F0164_DUNGEON_UnlinkThingFromList(L0667_T_Thing, 0, L0664_i_MapX, L0665_i_MapY); /* BUG0_25 When a champion dies, no bones object is created so it is not possible to bring the champion back to life at an altar of Vi. Each time a champion is brought back to life, the bones object is removed from the dungeon but it is not marked as unused and thus becomes an orphan. After a large number of champion deaths, all JUNK things are exhausted and the game cannot create any more. This also affects the creation of JUNK things dropped by some creatures when they die (Screamer, Rockpile, Magenta Worm, Pain Rat, Red Dragon) */
                                                        L0668_ps_Junk->Next = C0xFFFF_THING_NONE;
                                                        P0532_ps_Event->Map_Time++;
                                                        goto T0255002;
                                                }
                                        }
                                }
                                L0667_T_Thing = F0159_DUNGEON_GetNextThing(L0667_T_Thing);
                        }
                        break;
                case 0:
                        F0283_CHAMPION_ViAltarRebirth(P0532_ps_Event->A.A.Priority);
        }
}


STATICFUNCTION void F0257_TIMELINE_ProcessEvent70_Light(
EVENT*          P0534_ps_Event FINAL_SEPARATOR
{
        REGISTER int16_t L0674_i_Multiple;
#define AL0674_i_LightPower  L0674_i_Multiple
#define AL0674_i_LightAmount L0674_i_Multiple
        REGISTER int16_t L0673_i_WeakerLightPower;
        REGISTER BOOLEAN L0675_B_NegativeLightPower;
        EVENT L0676_s_Event;


        if ((AL0674_i_LightPower = P0534_ps_Event->B.LightPower) == 0) {
                return;
        }
        if (L0675_B_NegativeLightPower = (AL0674_i_LightPower < 0)) {
                AL0674_i_LightPower = -AL0674_i_LightPower;
        }
        L0673_i_WeakerLightPower = AL0674_i_LightPower - 1;
        AL0674_i_LightAmount = G0039_ai_Graphic562_LightPowerToLightAmount[AL0674_i_LightPower] - G0039_ai_Graphic562_LightPowerToLightAmount[L0673_i_WeakerLightPower];
        if (L0675_B_NegativeLightPower) {
                AL0674_i_LightAmount = -AL0674_i_LightAmount;
                L0673_i_WeakerLightPower = -L0673_i_WeakerLightPower;
        }
        G0407_s_Party.MagicalLightAmount += AL0674_i_LightAmount;
        if (L0673_i_WeakerLightPower) {
                L0676_s_Event.A.A.Type = C70_EVENT_LIGHT;
                L0676_s_Event.B.LightPower = L0673_i_WeakerLightPower;
                M033_SET_MAP_AND_TIME(L0676_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + 4);
                L0676_s_Event.A.A.Priority = 0;
                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0676_s_Event);
        }
}

STATICFUNCTION BOOLEAN F0258_TIMELINE_HasWeaponMovedToSlot(
int16_t                   P0535_i_ChampionIndex        SEPARATOR
CHAMPION*                 P0536_ps_Champion            SEPARATOR
unsigned int16_t          P0537_ui_SourceSlotIndex     SEPARATOR
int16_t                   P0538_i_DestinationSlotIndex FINAL_SEPARATOR
{
        if (M012_TYPE(P0536_ps_Champion->Slots[P0537_ui_SourceSlotIndex]) == C05_THING_TYPE_WEAPON) {
                F0301_CHAMPION_AddObjectInSlot(P0535_i_ChampionIndex, F0300_CHAMPION_GetObjectRemovedFromSlot(P0535_i_ChampionIndex, P0537_ui_SourceSlotIndex), P0538_i_DestinationSlotIndex);
                return C1_TRUE;
        }
        return C0_FALSE;
}

STATICFUNCTION void F0259_TIMELINE_ProcessEvent11Part2_MoveWeaponFromQuiverToSlot(
REGISTER unsigned int16_t P0539_ui_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0540_ui_SlotIndex     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0678_ps_Champion;
        REGISTER unsigned int16_t L0677_ui_SlotIndex;


        L0678_ps_Champion = &M516_CHAMPIONS[P0539_ui_ChampionIndex];
        if (L0678_ps_Champion->Slots[P0540_ui_SlotIndex] != C0xFFFF_THING_NONE)
                return;
        if (F0258_TIMELINE_HasWeaponMovedToSlot(P0539_ui_ChampionIndex, L0678_ps_Champion, C12_SLOT_QUIVER_LINE1_1, P0540_ui_SlotIndex)) {
                return;
        }
        for (L0677_ui_SlotIndex = C07_SLOT_QUIVER_LINE2_1; L0677_ui_SlotIndex <= C09_SLOT_QUIVER_LINE2_2; L0677_ui_SlotIndex++) {
                if (F0258_TIMELINE_HasWeaponMovedToSlot(P0539_ui_ChampionIndex, L0678_ps_Champion, L0677_ui_SlotIndex, P0540_ui_SlotIndex))
                        break;
        }
}


void F0261_TIMELINE_Process_CPSEF(
void
)
{
        REGISTER EVENT* L0681_ps_Event;
        REGISTER unsigned int16_t L0680_ui_Multiple;
#define AL0680_ui_EventType     L0680_ui_Multiple
#define AL0680_ui_ChampionIndex L0680_ui_Multiple
        EVENT L0682_s_Event;


        while (F0240_TIMELINE_IsFirstEventExpired_CPSE()) {
                L0681_ps_Event = &L0682_s_Event;
                F0239_TIMELINE_ExtractFirstEvent(L0681_ps_Event);
                F0173_DUNGEON_SetCurrentMap(M029_MAP(L0682_s_Event.Map_Time));
                        AL0680_ui_EventType = L0682_s_Event.A.A.Type;
                if ((AL0680_ui_EventType > (C29_EVENT_GROUP_REACTION_DANGER_ON_SQUARE - 1)) && (AL0680_ui_EventType < (C41_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1))) {
                        F0209_GROUP_ProcessEvents29to41(L0682_s_Event.B.Location.MapX, L0682_s_Event.B.Location.MapY, AL0680_ui_EventType, L0682_s_Event.C.Ticks);
                } else {
                        switch (AL0680_ui_EventType) {
                                case C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
                                case C49_EVENT_MOVE_PROJECTILE:
                                        F0219_PROJECTILE_ProcessEvents48To49_Projectile(L0681_ps_Event);
                                        break;
                                case C01_EVENT_DOOR_ANIMATION:
                                        F0241_TIMELINE_ProcessEvent1_DoorAnimation(L0681_ps_Event);
                                        break;
                                case C25_EVENT_EXPLOSION:
                                        F0220_EXPLOSION_ProcessEvent25_Explosion(L0681_ps_Event);
                                        break;
                                case C07_EVENT_FAKEWALL:
                                        F0242_TIMELINE_ProcessEvent7_Square_FakeWall(L0681_ps_Event);
                                        break;
                                case C02_EVENT_DOOR_DESTRUCTION:
                                        F0243_TIMELINE_ProcessEvent2_DoorDestruction(L0681_ps_Event);
                                        break;
                                case C10_EVENT_DOOR:
                                        F0244_TIMELINE_ProcessEvent10_Square_Door(L0681_ps_Event);
                                        break;
                                case C09_EVENT_PIT:
                                        F0251_TIMELINE_ProcessEvent9_Square_Pit(L0681_ps_Event);
                                        break;
                                case C08_EVENT_TELEPORTER:
                                        F0250_TIMELINE_ProcessEvent8_Square_Teleporter(L0681_ps_Event);
                                        break;
                                case C06_EVENT_WALL:
                                        F0248_TIMELINE_ProcessEvent6_Square_Wall(L0681_ps_Event);
                                        break;
                                case C05_EVENT_CORRIDOR:
                                        F0245_TIMELINE_ProcessEvent5_Square_Corridor(L0681_ps_Event);
                                        break;
                                case C60_EVENT_MOVE_GROUP_SILENT:
                                case C61_EVENT_MOVE_GROUP_AUDIBLE:
                                        F0252_TIMELINE_ProcessEvents60to61_MoveGroup(L0681_ps_Event);
                                        break;
                                case C65_EVENT_ENABLE_GROUP_GENERATOR:
                                        F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator(L0681_ps_Event);
                                        break;
                                case C20_EVENT_PLAY_SOUND:
                                        F0064_SOUND_RequestPlay_CPSD(L0682_s_Event.C.SoundIndex, L0682_s_Event.B.Location.MapX, L0682_s_Event.B.Location.MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                        break;
                                case C24_EVENT_REMOVE_FLUXCAGE:
                                        if (!G0302_B_GameWon) {
                                                F0164_DUNGEON_UnlinkThingFromList(L0682_s_Event.C.Slot, 0, L0682_s_Event.B.Location.MapX, L0682_s_Event.B.Location.MapY);
                                                L0681_ps_Event = (EVENT*)F0156_DUNGEON_GetThingData(L0682_s_Event.C.Slot);
                                                ((EXPLOSION*)L0681_ps_Event)->Next = C0xFFFF_THING_NONE;
                                        }
                                        break;
                                case C11_EVENT_ENABLE_CHAMPION_ACTION:
                                        F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction(L0682_s_Event.A.A.Priority);
                                        if (L0682_s_Event.B.SlotOrdinal) {
                                                F0259_TIMELINE_ProcessEvent11Part2_MoveWeaponFromQuiverToSlot(L0682_s_Event.A.A.Priority, M001_ORDINAL_TO_INDEX(L0682_s_Event.B.SlotOrdinal));
                                        }
                                        goto T0261048;
                                case C12_EVENT_HIDE_DAMAGE_RECEIVED:
                                        F0254_TIMELINE_ProcessEvent12_HideDamageReceived(L0682_s_Event.A.A.Priority);
                                        break;
                                case C70_EVENT_LIGHT:
                                        F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex);
                                        F0257_TIMELINE_ProcessEvent70_Light(L0681_ps_Event);
                                        F0337_INVENTORY_SetDungeonViewPalette();
                                        break;
                                case C71_EVENT_INVISIBILITY:
                                        if (!(--G0407_s_Party.Event71Count_Invisibility)) {
                                                if (G0423_i_InventoryChampionOrdinal) {
                                                        M008_SET(M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)].Attributes, MASK0x1000_STATUS_BOX);
                                                }
                                                F0293_CHAMPION_DrawAllChampionStates(MASK0x0400_ICON);
                                        }
                                        break;
                                case C72_EVENT_CHAMPION_SHIELD:
                                        M516_CHAMPIONS[L0682_s_Event.A.A.Priority].ShieldDefense -= L0682_s_Event.B.Defense;
                                        M008_SET(M516_CHAMPIONS[L0682_s_Event.A.A.Priority].Attributes, MASK0x1000_STATUS_BOX);
                                        T0261048:
                                        F0292_CHAMPION_DrawState(L0682_s_Event.A.A.Priority);
                                        break;
                                case C73_EVENT_THIEVES_EYE:
                                        --G0407_s_Party.Event73Count_ThievesEye;
                                        break;
                                case C74_EVENT_PARTY_SHIELD:
                                        G0407_s_Party.ShieldDefense -= L0682_s_Event.B.Defense;
                                        T0261053:
                                        F0293_CHAMPION_DrawAllChampionStates(MASK0x1000_STATUS_BOX);
                                        break;
                                case C77_EVENT_SPELLSHIELD:
                                        G0407_s_Party.SpellShieldDefense -= L0682_s_Event.B.Defense;
                                        goto T0261053;
                                case C78_EVENT_FIRESHIELD:
                                        G0407_s_Party.FireShieldDefense -= L0682_s_Event.B.Defense;
                                        goto T0261053;
                                case C75_EVENT_POISON_CHAMPION:
                                        M516_CHAMPIONS[AL0680_ui_ChampionIndex = L0682_s_Event.A.A.Priority].PoisonEventCount--;
                                        F0322_CHAMPION_Poison(AL0680_ui_ChampionIndex, L0682_s_Event.B.Attack);
                                        break;
                                case C13_EVENT_VI_ALTAR_REBIRTH:
                                        F0255_TIMELINE_ProcessEvent13_ViAltarRebirth(L0681_ps_Event);
                                        break;
                                case C79_EVENT_FOOTPRINTS:
                                        --G0407_s_Party.Event79Count_Footprints;
                                        break;
                        }
                }
                F0173_DUNGEON_SetCurrentMap(G0309_i_PartyMapIndex);
        }
}

/* END TIMELINE.C */

/* BEGIN CHAMPION.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
CHAMPION G1068_as_Champions[4];
PARTY_INFO G0407_s_Party;
int16_t G0409_ai_ChampionPendingDamage[4];
unsigned int16_t G0410_ai_ChampionPendingWounds[4];
int16_t G0411_i_LeaderIndex = CM1_CHAMPION_NONE;
LEADER_HAND_OBJECT G4055_s_LeaderHandObject;
unsigned int16_t G0415_ui_LeaderEmptyHanded;
int16_t G0416_i_DelayMouse_Useless; /* BUG0_00 Useless code */
int16_t G2139_i_RenameCommand;





void F0284_CHAMPION_SetPartyDirection(
REGISTER int16_t P0600_i_Direction FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0835_ps_Champion;
        REGISTER int16_t L0834_i_Delta;
        REGISTER unsigned int16_t L0833_ui_ChampionIndex;


        if (P0600_i_Direction == G0308_i_PartyDirection)
                return;
        if ((L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection) < 0) {
                L0834_i_Delta += 4;
        }
        L0835_ps_Champion = M516_CHAMPIONS;
        for (L0833_ui_ChampionIndex = C00_CHAMPION_FIRST; L0833_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0833_ui_ChampionIndex++) {
                L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta);
                L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta);
                L0835_ps_Champion++;
        }
        G0308_i_PartyDirection = P0600_i_Direction;
        F0296_CHAMPION_DrawChangedObjectIcons();
}

int16_t F0285_CHAMPION_GetIndexInCell(
unsigned int16_t P0601_ui_Cell FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0837_ps_Champion;
        REGISTER unsigned int16_t L0836_ui_ChampionIndex;


        L0837_ps_Champion = M516_CHAMPIONS;
        for (L0836_ui_ChampionIndex = C00_CHAMPION_FIRST; L0836_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0836_ui_ChampionIndex++, L0837_ps_Champion++) {
                if ((L0837_ps_Champion->Cell == P0601_ui_Cell) && L0837_ps_Champion->CurrentHealth) {
                        return L0836_ui_ChampionIndex;
                }
        }
        return CM1_CHAMPION_NONE;
}

int16_t F0286_CHAMPION_GetTargetChampionIndex(
int16_t          P0602_i_MapX  SEPARATOR
int16_t          P0603_i_MapY  SEPARATOR
unsigned int16_t P0604_ui_Cell FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0838_ui_Counter;
        REGISTER int16_t L0839_i_ChampionIndex;
        unsigned char L0840_auc_OrderedCellsToAttack[4];


        if (G0305_ui_PartyChampionCount && (M038_DISTANCE(P0602_i_MapX, P0603_i_MapY, G0306_i_PartyMapX, G0307_i_PartyMapY) <= 1)) {
                F0229_GROUP_SetOrderedCellsToAttack(M772_CAST_PC(L0840_auc_OrderedCellsToAttack), G0306_i_PartyMapX, G0307_i_PartyMapY, P0602_i_MapX, P0603_i_MapY, P0604_ui_Cell);
                for (L0838_ui_Counter = 0; L0838_ui_Counter < 4; L0838_ui_Counter++) {
                        if ((L0839_i_ChampionIndex = F0285_CHAMPION_GetIndexInCell(L0840_auc_OrderedCellsToAttack[L0838_ui_Counter])) >= 0) {
                                return L0839_i_ChampionIndex;
                        }
                }
        }
        return CM1_CHAMPION_NONE;
}

#include "AMMO.C"

void F0297_CHAMPION_PutObjectInLeaderHand(
THING   P0621_T_Thing                             SEPARATOR
BOOLEAN P0622_B_SetMousePointerToObjectInMainLoop FINAL_SEPARATOR /* If C1_TRUE, the mouse pointer is refreshed on screen by F0002_MAIN_GameLoop_CPSDF() but only if the viewport does not contain an inventory (G0423_i_InventoryChampionOrdinal = 0). If C0_FALSE, the mouse pointer is refreshed immediately. This is used when the viewport contains an inventory (G0423_i_InventoryChampionOrdinal != 0) */
{
        if (P0621_T_Thing == C0xFFFF_THING_NONE) {
                return;
        }
        G0415_ui_LeaderEmptyHanded = C0_FALSE;
        F0036_OBJECT_ExtractIconFromBitmap(G4055_s_LeaderHandObject.IconIndex = F0033_OBJECT_GetIconIndex(G4055_s_LeaderHandObject.Thing = P0621_T_Thing), M775_CAST_PL(G4055_s_LeaderHandObject.BitmapIconForMousePointer));
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0034_OBJECT_DrawLeaderHandObjectName(P0621_T_Thing);
        if (P0622_B_SetMousePointerToObjectInMainLoop) {
                G0325_B_SetMousePointerToObjectInMainLoop = C1_TRUE;
        } else {
                F0068_MOUSE_SetPointerToObject(G4055_s_LeaderHandObject.BitmapIconForMousePointer);
        }
        F0078_MOUSE_DisableScreenUpdate();
        if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                M516_CHAMPIONS[G0411_i_LeaderIndex].Load += F0140_DUNGEON_GetObjectWeight(P0621_T_Thing);
                M008_SET(M516_CHAMPIONS[G0411_i_LeaderIndex].Attributes, MASK0x0200_LOAD);
                F0292_CHAMPION_DrawState(G0411_i_LeaderIndex);
        }
}

THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand(
void
)
{
        REGISTER THING L0890_T_LeaderHandObject;


        G0415_ui_LeaderEmptyHanded = C1_TRUE;
        if ((L0890_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing) != C0xFFFF_THING_NONE) {
                G4055_s_LeaderHandObject.Thing = C0xFFFF_THING_NONE;
                G4055_s_LeaderHandObject.IconIndex = C0xFFFF_ICON_NONE;
                F0077_MOUSE_EnableScreenUpdate_CPSE();
                F0035_OBJECT_ClearLeaderHandObjectName();
                F0069_MOUSE_SetPointer();
                F0078_MOUSE_DisableScreenUpdate();
                if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                        M516_CHAMPIONS[G0411_i_LeaderIndex].Load -= F0140_DUNGEON_GetObjectWeight(L0890_T_LeaderHandObject);
                        M008_SET(M516_CHAMPIONS[G0411_i_LeaderIndex].Attributes, MASK0x0200_LOAD);
                        F0292_CHAMPION_DrawState(G0411_i_LeaderIndex);
                }
        }
        return L0890_T_LeaderHandObject;
}

STATICFUNCTION void F0299_CHAMPION_ApplyObjectModifiersToStatistics(
REGISTER CHAMPION*        P0623_ps_Champion      SEPARATOR
REGISTER unsigned int16_t P0624_ui_SlotIndex     SEPARATOR
REGISTER unsigned int16_t P0625_ui_IconIndex     SEPARATOR
int16_t                   P0626_i_ModifierFactor SEPARATOR
THING                     P0627_T_Thing          FINAL_SEPARATOR
#define AP0624_ui_StatisticValueIndex P0624_ui_SlotIndex
{
        REGISTER WEAPON* L0893_ps_Weapon;
        REGISTER int16_t L0892_i_Modifier;
        REGISTER int16_t L0891_i_Multiple;
#define AL0891_i_StatisticIndex L0891_i_Multiple
#define AL0891_i_ThingType      L0891_i_Multiple


        L0892_i_Modifier = 0;
        if ((((AL0891_i_ThingType = M012_TYPE(P0627_T_Thing)) == C05_THING_TYPE_WEAPON) || (AL0891_i_ThingType == C06_THING_TYPE_ARMOUR)) && (P0624_ui_SlotIndex >= C00_SLOT_READY_HAND) && (P0624_ui_SlotIndex <= C12_SLOT_QUIVER_LINE1_1)) {
                L0893_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(P0627_T_Thing);
                if (((AL0891_i_ThingType == C05_THING_TYPE_WEAPON) && L0893_ps_Weapon->Cursed) || ((AL0891_i_ThingType == C06_THING_TYPE_ARMOUR) && ((ARMOUR*)L0893_ps_Weapon)->Cursed)) {
                        AL0891_i_StatisticIndex = C0_STATISTIC_LUCK;
                        L0892_i_Modifier = -3;
                        goto T0299044_ApplyModifier;
                }
        }
        if ((P0625_ui_IconIndex == C137_ICON_JUNK_RABBITS_FOOT) && (P0624_ui_SlotIndex < C30_SLOT_CHEST_1)) {
                AL0891_i_StatisticIndex = C0_STATISTIC_LUCK;
                L0892_i_Modifier = 10;
        } else {
                if (P0624_ui_SlotIndex == C01_SLOT_ACTION_HAND) {
                        if (P0625_ui_IconIndex == C045_ICON_WEAPON_MACE_OF_ORDER) {
                                AL0891_i_StatisticIndex = C1_STATISTIC_STRENGTH;
#ifdef PC_FIX_CODE_SIZE
        L0891_i_Multiple = 40;
        L0891_i_Multiple = 30;
#endif
                                L0892_i_Modifier = 5;
                        } else {
                                AL0891_i_StatisticIndex = C8_STATISTIC_MANA;
                                if ((P0625_ui_IconIndex >= C020_ICON_WEAPON_STAFF_OF_CLAWS_EMPTY) && (P0625_ui_IconIndex <= C022_ICON_WEAPON_STAFF_OF_CLAWS_FULL)) {
                                        L0892_i_Modifier = 4;
                                } else {
                                        if ((P0625_ui_IconIndex >= C058_ICON_WEAPON_STAFF) && (P0625_ui_IconIndex <= C066_ICON_WEAPON_SCEPTRE_OF_LYF)) {
                                                switch (P0625_ui_IconIndex) {
                                                        case C058_ICON_WEAPON_STAFF:
                                                                L0892_i_Modifier = 2;
                                                                break;
                                                        case C059_ICON_WEAPON_WAND:
                                                                L0892_i_Modifier = 1;
                                                                break;
                                                        case C060_ICON_WEAPON_TEOWAND:
                                                                L0892_i_Modifier = 6;
                                                                break;
                                                        case C061_ICON_WEAPON_YEW_STAFF:
#ifdef PC_FIX_CODE_SIZE
        L0891_i_Multiple++;
        L0891_i_Multiple++;
        L0891_i_Multiple = 28;
#endif
                                                                L0892_i_Modifier = 4;
                                                                break;
                                                        case C062_ICON_WEAPON_STAFF_OF_MANAR_STAFF_OF_IRRA:
                                                                L0892_i_Modifier = 10;
                                                                break;
                                                        case C063_ICON_WEAPON_SNAKE_STAFF_CROSS_OF_NETA:
                                                                L0892_i_Modifier = 8;
                                                                break;
                                                        case C064_ICON_WEAPON_THE_CONDUIT_SERPENT_STAFF:
                                                                L0892_i_Modifier = 16;
                                                                break;
                                                        case C065_ICON_WEAPON_DRAGON_SPIT:
                                                                L0892_i_Modifier = 7;
                                                                break;
                                                        case C066_ICON_WEAPON_SCEPTRE_OF_LYF:
                                                                L0892_i_Modifier = 5;
                                                }
                                        } else {
                                                switch (P0625_ui_IconIndex) {
                                                        case C038_ICON_WEAPON_DELTA_SIDE_SPLITTER:
                                                                L0892_i_Modifier = 1;
                                                                break;
                                                        case C041_ICON_WEAPON_THE_INQUISITOR_DRAGON_FANG:
                                                                L0892_i_Modifier = 2;
                                                                break;
                                                        case C040_ICON_WEAPON_VORPAL_BLADE:
                                                                L0892_i_Modifier = 4;
                                                }
                                        }
                                }
                        }
                } else {
                        if (P0624_ui_SlotIndex == C04_SLOT_LEGS) {
                                if (P0625_ui_IconIndex == C142_ICON_ARMOUR_POWERTOWERS) {
                                        AL0891_i_StatisticIndex = C1_STATISTIC_STRENGTH;
#ifdef PC_FIX_CODE_SIZE
        L0891_i_Multiple = 21;
        L0891_i_Multiple = 22;
#endif
                                        L0892_i_Modifier = 10;
                                }
                        } else {
                                if (P0624_ui_SlotIndex == C02_SLOT_HEAD) {
                                        if (P0625_ui_IconIndex == C104_ICON_ARMOUR_CROWN_OF_NERRA) {
                                                AL0891_i_StatisticIndex = C3_STATISTIC_WISDOM;
#ifdef PC_FIX_CODE_SIZE
        L0891_i_Multiple++;
        L0891_i_Multiple++;
        L0891_i_Multiple = 23;
#endif
                                                L0892_i_Modifier = 10;
                                        } else {
                                                if (P0625_ui_IconIndex == C140_ICON_ARMOUR_DEXHELM) {
                                                        AL0891_i_StatisticIndex = C2_STATISTIC_DEXTERITY;
                                                        L0892_i_Modifier = 10;
                                                }
                                        }
                                } else {
                                        if (P0624_ui_SlotIndex == C03_SLOT_TORSO) {
                                                if (P0625_ui_IconIndex == C141_ICON_ARMOUR_FLAMEBAIN) {
                                                        AL0891_i_StatisticIndex = C6_STATISTIC_ANTIFIRE;
                                                        L0892_i_Modifier = 12;
                                                } else {
                                                        if (P0625_ui_IconIndex == C081_ICON_ARMOUR_CLOAK_OF_NIGHT) {
#ifdef PC_FIX_CODE_SIZE
        L0891_i_Multiple++;
        L0891_i_Multiple++;
        L0891_i_Multiple = 25;
        L0891_i_Multiple = 26;
#endif
                                                                AL0891_i_StatisticIndex = C2_STATISTIC_DEXTERITY;
                                                                L0892_i_Modifier = 8;
                                                        }
                                                }
                                        } else {
                                                if (P0624_ui_SlotIndex == C10_SLOT_NECK) {
                                                        if ((P0625_ui_IconIndex >= C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED) && (P0625_ui_IconIndex <= C011_ICON_JUNK_JEWEL_SYMAL_EQUIPPED)) {
                                                                AL0891_i_StatisticIndex = C5_STATISTIC_ANTIMAGIC;
                                                                L0892_i_Modifier = 15;
                                                        } else {
                                                                if (P0625_ui_IconIndex == C081_ICON_ARMOUR_CLOAK_OF_NIGHT) {
                                                                        AL0891_i_StatisticIndex = C2_STATISTIC_DEXTERITY;
                                                                        L0892_i_Modifier = 8;
                                                                } else {
                                                                        if (P0625_ui_IconIndex == C122_ICON_JUNK_MOONSTONE) {
                                                                                AL0891_i_StatisticIndex = C8_STATISTIC_MANA;
                                                                                L0892_i_Modifier = 3;
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
        T0299044_ApplyModifier:
        if (L0892_i_Modifier) {
                L0892_i_Modifier *= P0626_i_ModifierFactor;
                if (AL0891_i_StatisticIndex == C8_STATISTIC_MANA) {
                        P0623_ps_Champion->MaximumMana += L0892_i_Modifier;
                } else {
                        if (AL0891_i_StatisticIndex < C6_STATISTIC_ANTIFIRE + 1) {
                                for (AP0624_ui_StatisticValueIndex = C0_MAXIMUM; AP0624_ui_StatisticValueIndex <= C2_MINIMUM; P0623_ps_Champion->Statistics[AL0891_i_StatisticIndex][AP0624_ui_StatisticValueIndex++] += L0892_i_Modifier); /* BUG0_38 A champion is much luckier than expected during combat when equipped with at least 4 cursed objects. Champion statistic values are stored as one byte unsigned integers which can store values between 0 to 255. Each champion has a minimum, a maximum and a current value for each of the statistics. When an object modifier is applied to a statistic, all three values are modified but there is no check to make sure the new values stay in the bounds of 0 to 255. This is not an issue in most cases because nearly all modifiers are positive and with relatively small values so that any combination of champion statistic value with these modifiers will result in valid values. The only issue is with the hidden luck statistic and the modifier for cursed objects which is the only negative modifier in the game (-3). The minimum luck value for all champions is set to 10 at the beginning of the game and is only changed by object modifiers. If enough cursed objects are equipped (between 4 and 6) then the minimum value 'wraps around' and becomes a large value (254 with 4 cursed objects, 251 with 5 cursed objects and 248 with 6 cursed objects). The first time that the luck of the champion is tested after the cursed objects are equipped, the current luck value is set to its minimum value (248, 251 or 254) because the game detects that the current value is smaller than the minimum allowed. During the following luck tests, if the test succeeds, the current luck is decreased by 2 and then bounded between the minimum and maximum values. The current value is then smaller than the minimum value so the current luck value is set to its minimum value (248, 251 or 254). If the test fails, the current luck is increased by 2 and then bounded between the minimum and maximum values. The current value is then larger than the minimum value, however it is larger than the maximum value so the current value is set to its maximum value. During the next luck test, the current value will again be smaller than the minimum and be reset to the minimum value. As a consequence, while the cursed objects are equipped, the current luck value can only have two possible values: either its maximum value, or its minimum value (which is even larger at 248, 251 or 254) so the champion is always luckier than he should be */
                        }
                }
                M008_SET(P0623_ps_Champion->Attributes, MASK0x0800_PANEL | MASK0x0100_STATISTICS);
        }
}

THING F0300_CHAMPION_GetObjectRemovedFromSlot(
REGISTER unsigned int16_t P0628_ui_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0629_ui_SlotIndex     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0896_ps_Champion;
        REGISTER WEAPON* L0897_ps_Weapon;
        REGISTER THING L0894_T_Thing;
        REGISTER int16_t L0895_i_IconIndex;
        REGISTER BOOLEAN L0898_B_IsInventoryChampion;


        L0896_ps_Champion = &M516_CHAMPIONS[P0628_ui_ChampionIndex];
        if (P0629_ui_SlotIndex >= C30_SLOT_CHEST_1) {
                L0894_T_Thing = G0425_aT_ChestSlots[P0629_ui_SlotIndex - C30_SLOT_CHEST_1];
                G0425_aT_ChestSlots[P0629_ui_SlotIndex - C30_SLOT_CHEST_1] = C0xFFFF_THING_NONE;
        } else {
                L0894_T_Thing = L0896_ps_Champion->Slots[P0629_ui_SlotIndex];
                L0896_ps_Champion->Slots[P0629_ui_SlotIndex] = C0xFFFF_THING_NONE;
        }
        if (L0894_T_Thing == C0xFFFF_THING_NONE) {
                return C0xFFFF_THING_NONE;
        }
        L0898_B_IsInventoryChampion = (M000_INDEX_TO_ORDINAL(P0628_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal);
        L0895_i_IconIndex = F0033_OBJECT_GetIconIndex(L0894_T_Thing);
        F0299_CHAMPION_ApplyObjectModifiersToStatistics(L0896_ps_Champion, P0629_ui_SlotIndex, L0895_i_IconIndex, -1, L0894_T_Thing); /* Remove objet modifiers */
        L0897_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(L0894_T_Thing);
        if (P0629_ui_SlotIndex == C10_SLOT_NECK) {
                if ((L0895_i_IconIndex >= C012_ICON_JUNK_ILLUMULET_UNEQUIPPED) && (L0895_i_IconIndex <= C013_ICON_JUNK_ILLUMULET_EQUIPPED)) {
                        ((JUNK*)L0897_ps_Weapon)->ChargeCount = 0;
                        G0407_s_Party.MagicalLightAmount -= G0039_ai_Graphic562_LightPowerToLightAmount[2];
                        F0337_INVENTORY_SetDungeonViewPalette();
                } else {
                        if ((L0895_i_IconIndex >= C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED) && (L0895_i_IconIndex <= C011_ICON_JUNK_JEWEL_SYMAL_EQUIPPED)) {
                                ((JUNK*)L0897_ps_Weapon)->ChargeCount = 0;
                        }
                }
        }
        F0291_CHAMPION_DrawSlot(P0628_ui_ChampionIndex, P0629_ui_SlotIndex);
        if (L0898_B_IsInventoryChampion) {
                M008_SET(L0896_ps_Champion->Attributes, MASK0x4000_VIEWPORT);
        }
        if (P0629_ui_SlotIndex < C02_SLOT_HEAD) {
                if (P0629_ui_SlotIndex == C01_SLOT_ACTION_HAND) {
                        M008_SET(L0896_ps_Champion->Attributes, MASK0x8000_ACTION_HAND);
                        if (G0506_ui_ActingChampionOrdinal == M000_INDEX_TO_ORDINAL(P0628_ui_ChampionIndex)) {
                                F0388_MENUS_ClearActingChampion();
                        }
                        if ((L0895_i_IconIndex >= C030_ICON_SCROLL_SCROLL_OPEN) && (L0895_i_IconIndex <= C031_ICON_SCROLL_SCROLL_CLOSED)) {
                                ((SCROLL*)L0897_ps_Weapon)->Closed = C1_TRUE;
                                F0296_CHAMPION_DrawChangedObjectIcons();
                        }
                }
                if ((L0895_i_IconIndex >= C004_ICON_WEAPON_TORCH_UNLIT) && (L0895_i_IconIndex <= C007_ICON_WEAPON_TORCH_LIT)) {
                        L0897_ps_Weapon->Lit = C0_FALSE;
                        F0337_INVENTORY_SetDungeonViewPalette();
                        F0296_CHAMPION_DrawChangedObjectIcons();
                }
                if (L0898_B_IsInventoryChampion && (P0629_ui_SlotIndex == C01_SLOT_ACTION_HAND)) {
                        if (L0895_i_IconIndex == C144_ICON_CONTAINER_CHEST_CLOSED) {
                                if (L0894_T_Thing == G0426_T_OpenChest)
                                        F0334_INVENTORY_CloseChest();
                                goto T0300011;
                        }
                        else {
                                if ((L0895_i_IconIndex >= C030_ICON_SCROLL_SCROLL_OPEN) && (L0895_i_IconIndex <= C031_ICON_SCROLL_SCROLL_CLOSED)) {
                                        T0300011:
                                        M008_SET(L0896_ps_Champion->Attributes, MASK0x0800_PANEL);
                                }
                        }
                }
        }
        L0896_ps_Champion->Load -= F0140_DUNGEON_GetObjectWeight(L0894_T_Thing);
        M008_SET(L0896_ps_Champion->Attributes, MASK0x0200_LOAD);
        return L0894_T_Thing;
}

void F0301_CHAMPION_AddObjectInSlot(
REGISTER unsigned int16_t P0630_ui_ChampionIndex SEPARATOR
REGISTER THING            P0631_T_Thing          SEPARATOR
REGISTER unsigned int16_t P0632_ui_SlotIndex     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0900_ps_Champion;
        REGISTER WEAPON* L0901_ps_Weapon;
        REGISTER int16_t L0899_i_IconIndex;
        REGISTER BOOLEAN L0902_B_IsInventoryChampion;


        if (P0631_T_Thing == C0xFFFF_THING_NONE)
                return;
        L0900_ps_Champion = &M516_CHAMPIONS[P0630_ui_ChampionIndex];
        if (P0632_ui_SlotIndex >= C30_SLOT_CHEST_1) {
                G0425_aT_ChestSlots[P0632_ui_SlotIndex - C30_SLOT_CHEST_1] = P0631_T_Thing;
        } else {
                L0900_ps_Champion->Slots[P0632_ui_SlotIndex] = P0631_T_Thing;
        }
        L0900_ps_Champion->Load += F0140_DUNGEON_GetObjectWeight(P0631_T_Thing);
        M008_SET(L0900_ps_Champion->Attributes, MASK0x0200_LOAD);
        L0899_i_IconIndex = F0033_OBJECT_GetIconIndex(P0631_T_Thing);
        L0902_B_IsInventoryChampion = (M000_INDEX_TO_ORDINAL(P0630_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal);
        F0299_CHAMPION_ApplyObjectModifiersToStatistics(L0900_ps_Champion, P0632_ui_SlotIndex, L0899_i_IconIndex, 1, P0631_T_Thing); /* Add objet modifiers */
        L0901_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(P0631_T_Thing);
        if (P0632_ui_SlotIndex < C02_SLOT_HEAD) {
                if (P0632_ui_SlotIndex == C01_SLOT_ACTION_HAND) {
                        M008_SET(L0900_ps_Champion->Attributes, MASK0x8000_ACTION_HAND);
                        if (G0506_ui_ActingChampionOrdinal == M000_INDEX_TO_ORDINAL(P0630_ui_ChampionIndex)) {
                                F0388_MENUS_ClearActingChampion();
                        }
                        if ((L0899_i_IconIndex >= C030_ICON_SCROLL_SCROLL_OPEN) && (L0899_i_IconIndex <= C031_ICON_SCROLL_SCROLL_CLOSED)) {
                                ((SCROLL*)L0901_ps_Weapon)->Closed = C0_FALSE;
                                F0296_CHAMPION_DrawChangedObjectIcons();
                        }
                }
                if (L0899_i_IconIndex == C004_ICON_WEAPON_TORCH_UNLIT) {
                        L0901_ps_Weapon->Lit = C1_TRUE;
                        F0337_INVENTORY_SetDungeonViewPalette();
                        F0296_CHAMPION_DrawChangedObjectIcons();
                } else {
                        if (L0902_B_IsInventoryChampion && (P0632_ui_SlotIndex == C01_SLOT_ACTION_HAND) &&
                            ((L0899_i_IconIndex == C144_ICON_CONTAINER_CHEST_CLOSED) || ((L0899_i_IconIndex >= C030_ICON_SCROLL_SCROLL_OPEN) && (L0899_i_IconIndex <= C031_ICON_SCROLL_SCROLL_CLOSED)))) {
                                M008_SET(L0900_ps_Champion->Attributes, MASK0x0800_PANEL);
                        }
                }
        } else {
                if (P0632_ui_SlotIndex == C10_SLOT_NECK) {
                        if ((L0899_i_IconIndex >= C012_ICON_JUNK_ILLUMULET_UNEQUIPPED) && (L0899_i_IconIndex <= C013_ICON_JUNK_ILLUMULET_EQUIPPED)) {
                                ((JUNK*)L0901_ps_Weapon)->ChargeCount = 1;
                                G0407_s_Party.MagicalLightAmount += G0039_ai_Graphic562_LightPowerToLightAmount[2];
                                F0337_INVENTORY_SetDungeonViewPalette();
                                L0899_i_IconIndex++;
                        } else {
                                if ((L0899_i_IconIndex >= C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED) && (L0899_i_IconIndex <= C011_ICON_JUNK_JEWEL_SYMAL_EQUIPPED)) {
                                        ((JUNK*)L0901_ps_Weapon)->ChargeCount = 1;
                                        L0899_i_IconIndex++;
                                }
                        }
                }
        }
        F0291_CHAMPION_DrawSlot(P0630_ui_ChampionIndex, P0632_ui_SlotIndex);
        if (L0902_B_IsInventoryChampion) {
                M008_SET(L0900_ps_Champion->Attributes, MASK0x4000_VIEWPORT);
        }
}

void F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(
REGISTER unsigned int16_t P0633_ui_SlotBoxIndex FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0903_ui_ChampionIndex;
        REGISTER unsigned int16_t L0904_ui_SlotIndex;
        REGISTER THING L0905_T_LeaderHandObject;
        REGISTER THING L0906_T_SlotThing;


        if (P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT) {
                if (G0299_ui_CandidateChampionOrdinal)
                        return;
                L0903_ui_ChampionIndex = P0633_ui_SlotBoxIndex >> 1;
                if ((L0903_ui_ChampionIndex >= G0305_ui_PartyChampionCount) || (M000_INDEX_TO_ORDINAL(L0903_ui_ChampionIndex) == (int16_t)G0423_i_InventoryChampionOrdinal) || !M516_CHAMPIONS[L0903_ui_ChampionIndex].CurrentHealth)
                        return;
                L0904_ui_SlotIndex = M070_HAND_SLOT_INDEX(P0633_ui_SlotBoxIndex);
        } else {
                L0903_ui_ChampionIndex = M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal);
                L0904_ui_SlotIndex = P0633_ui_SlotBoxIndex - C08_SLOT_BOX_INVENTORY_FIRST_SLOT;
        }
        L0905_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing;
        if (L0904_ui_SlotIndex >= C30_SLOT_CHEST_1) {
                L0906_T_SlotThing = G0425_aT_ChestSlots[L0904_ui_SlotIndex - C30_SLOT_CHEST_1];
        } else {
                L0906_T_SlotThing = M516_CHAMPIONS[L0903_ui_ChampionIndex].Slots[L0904_ui_SlotIndex];
        }
        if ((L0906_T_SlotThing == C0xFFFF_THING_NONE) && (L0905_T_LeaderHandObject == C0xFFFF_THING_NONE)) {
                return;
        }
        if ((L0905_T_LeaderHandObject != C0xFFFF_THING_NONE) && (!(G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(L0905_T_LeaderHandObject)].AllowedSlots & G0038_ai_Graphic562_SlotMasks[L0904_ui_SlotIndex]))) {
                return;
        }
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        if (L0905_T_LeaderHandObject != C0xFFFF_THING_NONE) {
                F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
        }
        if (L0906_T_SlotThing != C0xFFFF_THING_NONE) {
                F0300_CHAMPION_GetObjectRemovedFromSlot(L0903_ui_ChampionIndex, L0904_ui_SlotIndex);
                F0297_CHAMPION_PutObjectInLeaderHand(L0906_T_SlotThing, C0_FALSE); /* BUG0_39 In the leader inventory, the Food/Water/Poisoned panel is briefly drawn when replacing a scroll or chest by another scroll or chest in the action hand. This does not occur for champions other than the leader. When swapping the objects between the action hand and the leader hand, the program first removes the object in the action hand with a call to F0300_CHAMPION_GetObjectRemovedFromSlot. If the removed object is a chest or a scroll then this sets the MASK0x0800_PANEL flag in the champion data to refresh the panel. Then the object in the leader hand is removed with a call to F0297_CHAMPION_PutObjectInLeaderHand. The weight of the object in the leader hand is part of the load of the leader, so this sets another flag MASK0x0200_LOAD in the champion data of the leader to refresh the Load value and then calls the function F0292_CHAMPION_DrawState to draw the leader champion state. If the champion is not the leader, nothing is drawn. If the champion is the leader, the Food/Water/Poisoned panel is drawn because at this point the action hand is empty. Then the object that was removed from the leader hand is put in the action hand with a call to F0301_CHAMPION_AddObjectInSlot and this draws the panel again, this time showing the chest contents or scroll text. The fix would be to swap the calls to F0300_CHAMPION_GetObjectRemovedFromSlot and F0297_CHAMPION_PutObjectInLeaderHand (note this would not fix the double refresh of the Load value after removing the first object and after putting the second object) */
        }
        if (L0905_T_LeaderHandObject != C0xFFFF_THING_NONE) {
                F0301_CHAMPION_AddObjectInSlot(L0903_ui_ChampionIndex, L0905_T_LeaderHandObject, L0904_ui_SlotIndex);
        }
        F0292_CHAMPION_DrawState(L0903_ui_ChampionIndex);
        F0078_MOUSE_DisableScreenUpdate();
}

unsigned int16_t F0303_CHAMPION_GetSkillLevel(
int16_t                   P0634_i_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0635_ui_SkillIndex   FINAL_SEPARATOR
{
        REGISTER SKILL* L0911_ps_Skill;
        REGISTER CHAMPION* L0912_ps_Champion;
        REGISTER int16_t L0908_i_SkillLevel;
        REGISTER unsigned long L0907_Experience;
        REGISTER int16_t L0909_i_NeckIconIndex;
        REGISTER int16_t L0910_i_ActionHandIconIndex;
        BOOLEAN L0913_B_IgnoreTemporaryExperience;
        BOOLEAN L0914_B_IgnoreObjectModifiers;


        if (G0300_B_PartyIsResting) {
                return 1;
        }
        L0913_B_IgnoreTemporaryExperience = M007_GET(P0635_ui_SkillIndex, MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE);
        L0914_B_IgnoreObjectModifiers = M007_GET(P0635_ui_SkillIndex, MASK0x4000_IGNORE_OBJECT_MODIFIERS);
        M009_CLEAR(P0635_ui_SkillIndex, MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE | MASK0x4000_IGNORE_OBJECT_MODIFIERS);
        L0912_ps_Champion = &M516_CHAMPIONS[P0634_i_ChampionIndex];
        L0911_ps_Skill = &L0912_ps_Champion->Skills[P0635_ui_SkillIndex];
        L0907_Experience = L0911_ps_Skill->Experience;
        if (!L0913_B_IgnoreTemporaryExperience) {
                L0907_Experience += L0911_ps_Skill->TemporaryExperience;
        }
        if (P0635_ui_SkillIndex > C03_SKILL_WIZARD) { /* Hidden skill */
                L0911_ps_Skill = &L0912_ps_Champion->Skills[(P0635_ui_SkillIndex - C04_SKILL_SWING) >> 2];
                L0907_Experience += L0911_ps_Skill->Experience; /* Add experience in the base skill */
                if (!L0913_B_IgnoreTemporaryExperience) {
                        L0907_Experience += L0911_ps_Skill->TemporaryExperience;
                }
                L0907_Experience >>= 1; /* Halve experience to get average of base skill + hidden skill experience */
        }
        L0908_i_SkillLevel = 1;
        while (L0907_Experience >= 500) {
                L0907_Experience >>= 1;
                L0908_i_SkillLevel++;
        }
        if (!L0914_B_IgnoreObjectModifiers) {
                if ((L0910_i_ActionHandIconIndex = F0033_OBJECT_GetIconIndex(L0912_ps_Champion->Slots[C01_SLOT_ACTION_HAND])) == C027_ICON_WEAPON_THE_FIRESTAFF) {
                        L0908_i_SkillLevel++;
                } else {
                        if (L0910_i_ActionHandIconIndex == C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE) {
                                L0908_i_SkillLevel += 2;
                        }
                }
                                L0909_i_NeckIconIndex = F0033_OBJECT_GetIconIndex(L0912_ps_Champion->Slots[C10_SLOT_NECK]);
                                switch (P0635_ui_SkillIndex) {
                                        case C03_SKILL_WIZARD:
#ifdef PC_FIX_CODE_SIZE
        L0908_i_SkillLevel++;
#endif
                                                if (L0909_i_NeckIconIndex == C124_ICON_JUNK_PENDANT_FERAL) {
                                                        L0908_i_SkillLevel++;
                                                }
                                                break;
                                        case C15_SKILL_DEFEND:
#ifdef PC_FIX_CODE_SIZE
        L0908_i_SkillLevel++;
#endif
                                                if (L0909_i_NeckIconIndex == C121_ICON_JUNK_EKKHARD_CROSS) {
                                                        L0908_i_SkillLevel++;
                                                }
                                                break;
                                        case C13_SKILL_HEAL:
#ifdef PC_FIX_CODE_SIZE
        L0908_i_SkillLevel++;
#endif
                                                if ((L0909_i_NeckIconIndex == C120_ICON_JUNK_GEM_OF_AGES) || (L0910_i_ActionHandIconIndex == C066_ICON_WEAPON_SCEPTRE_OF_LYF)) { /* The skill modifiers of these two objects are not cumulative */
                                                        L0908_i_SkillLevel++;
                                                }
                                                break;
                                        case C14_SKILL_INFLUENCE:
                                                if (L0909_i_NeckIconIndex == C122_ICON_JUNK_MOONSTONE) {
                                                        L0908_i_SkillLevel++;
                                                }
                                                break;
                                }
        }
        return L0908_i_SkillLevel;
}

void F0304_CHAMPION_AddSkillExperience(
unsigned int16_t          P0636_ui_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0637_ui_SkillIndex    SEPARATOR
REGISTER unsigned int16_t P0638_ui_Experience    FINAL_SEPARATOR
#define AP0638_ui_SkillLevelAfter P0638_ui_Experience
#define AP0638_ui_ChampionColor   P0638_ui_Experience
{
        REGISTER CHAMPION* L0919_ps_Champion;
        REGISTER SKILL* L0918_ps_Skill;
        REGISTER char *L2248_pc_;
        char *L2249_pc_;
        REGISTER unsigned int16_t L0915_ui_Multiple;
#define AL0915_ui_MapDifficulty    L0915_ui_Multiple
#define AL0915_ui_SkillLevelBefore L0915_ui_Multiple
#define AL0915_ui_VitalityAmount   L0915_ui_Multiple
#define AL0915_ui_StaminaAmount    L0915_ui_Multiple
        REGISTER unsigned int16_t L0916_ui_BaseSkillIndex;
        REGISTER int16_t L0920_i_MinorStatisticIncrease;
        int16_t L0921_i_MajorStatisticIncrease;
        int16_t L0922_i_BaseSkillLevel;


        if ((P0637_ui_SkillIndex >= C04_SKILL_SWING) && (P0637_ui_SkillIndex <= C11_SKILL_SHOOT) && (G0361_l_LastCreatureAttackTime < (G0313_ul_GameTime - 150))) {
                P0638_ui_Experience >>= 1;
        }
        if (P0638_ui_Experience) {
                if (AL0915_ui_MapDifficulty = G0269_ps_CurrentMap->C.Difficulty) {
                        P0638_ui_Experience *= AL0915_ui_MapDifficulty;
                }
                L0919_ps_Champion = &M516_CHAMPIONS[P0636_ui_ChampionIndex];
                if (P0637_ui_SkillIndex >= C04_SKILL_SWING) {
                        L0916_ui_BaseSkillIndex = (P0637_ui_SkillIndex - C04_SKILL_SWING) >> 2;
                } else {
                        L0916_ui_BaseSkillIndex = P0637_ui_SkillIndex;
                }
                AL0915_ui_SkillLevelBefore = F0303_CHAMPION_GetSkillLevel(P0636_ui_ChampionIndex, L0916_ui_BaseSkillIndex | (MASK0x4000_IGNORE_OBJECT_MODIFIERS | MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE));
                if ((P0637_ui_SkillIndex >= C04_SKILL_SWING) && (G0361_l_LastCreatureAttackTime > (G0313_ul_GameTime - 25))) {
                        P0638_ui_Experience <<= 1;
                }
                L0918_ps_Skill = &L0919_ps_Champion->Skills[P0637_ui_SkillIndex];
                L0918_ps_Skill->Experience += P0638_ui_Experience;
                if (L0918_ps_Skill->TemporaryExperience < 32000) {
                        L0918_ps_Skill->TemporaryExperience += F0026_MAIN_GetBoundedValue(1, P0638_ui_Experience >> 3, 100);
                }
                L0918_ps_Skill = &L0919_ps_Champion->Skills[L0916_ui_BaseSkillIndex];
                if (P0637_ui_SkillIndex >= C04_SKILL_SWING) {
                        L0918_ps_Skill->Experience += P0638_ui_Experience;
                }
                AP0638_ui_SkillLevelAfter = F0303_CHAMPION_GetSkillLevel(P0636_ui_ChampionIndex, L0916_ui_BaseSkillIndex | (MASK0x4000_IGNORE_OBJECT_MODIFIERS | MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE));
                if (AP0638_ui_SkillLevelAfter > AL0915_ui_SkillLevelBefore) {
                        L0922_i_BaseSkillLevel = AP0638_ui_SkillLevelAfter;
                        L0920_i_MinorStatisticIncrease = M005_RANDOM(2);
                        L0921_i_MajorStatisticIncrease = 1 + M005_RANDOM(2);
                        AL0915_ui_VitalityAmount = M005_RANDOM(2); /* For Priest skill, the amount is 0 or 1 for all skill levels */
                        if (L0916_ui_BaseSkillIndex != C02_SKILL_PRIEST) {
                                AL0915_ui_VitalityAmount &= AP0638_ui_SkillLevelAfter; /* For non Priest skills the amount is 0 for even skill levels. The amount is 0 or 1 for odd skill levels */
                        }
                        L0919_ps_Champion->Statistics[C4_STATISTIC_VITALITY][C0_MAXIMUM] += AL0915_ui_VitalityAmount;
                        AL0915_ui_StaminaAmount = L0919_ps_Champion->MaximumStamina;
                        L0919_ps_Champion->Statistics[C6_STATISTIC_ANTIFIRE][C0_MAXIMUM] += M005_RANDOM(2) & ~AP0638_ui_SkillLevelAfter; /* The amount is 0 for odd skill levels. The amount is 0 or 1 for even skill levels */
                        switch (L0916_ui_BaseSkillIndex) {
                                case C00_SKILL_FIGHTER:
#ifdef X732_I34E
                                        L2248_pc_ = "";
                                        L2249_pc_ = " JUST GAINED A FIGHTER LEVEL!";
#endif
#ifdef X736_I34M
                                        L2248_pc_ = F0758_TranslateLanguage(C61_TEXT);
                                        L2249_pc_ = F0758_TranslateLanguage(C62_TEXT);
#endif
                                        AL0915_ui_StaminaAmount >>= 4;
                                        AP0638_ui_SkillLevelAfter *= 3;
                                        L0919_ps_Champion->Statistics[C1_STATISTIC_STRENGTH][C0_MAXIMUM] += L0921_i_MajorStatisticIncrease;
                                        L0919_ps_Champion->Statistics[C2_STATISTIC_DEXTERITY][C0_MAXIMUM] += L0920_i_MinorStatisticIncrease;
                                        break;
                                case C01_SKILL_NINJA:
#ifdef X732_I34E
                                        L2248_pc_ = "";
                                        L2249_pc_ = " JUST GAINED A NINJA LEVEL!";
#endif
#ifdef X736_I34M
                                        L2248_pc_ = F0758_TranslateLanguage(C63_TEXT);
                                        L2249_pc_ = F0758_TranslateLanguage(C64_TEXT);
#endif
                                        AL0915_ui_StaminaAmount /= 21;
                                        AP0638_ui_SkillLevelAfter <<= 1;
                                        L0919_ps_Champion->Statistics[C1_STATISTIC_STRENGTH][C0_MAXIMUM] += L0920_i_MinorStatisticIncrease;
                                        L0919_ps_Champion->Statistics[C2_STATISTIC_DEXTERITY][C0_MAXIMUM] += L0921_i_MajorStatisticIncrease;
                                        break;
                                case C03_SKILL_WIZARD:
#ifdef X732_I34E
                                        L2248_pc_ = "";
                                        L2249_pc_ = " JUST GAINED A WIZARD LEVEL!";
#endif
#ifdef X736_I34M
                                        L2248_pc_ = F0758_TranslateLanguage(C67_TEXT);
                                        L2249_pc_ = F0758_TranslateLanguage(C68_TEXT);
#endif
                                        AL0915_ui_StaminaAmount >>= 5;
                                        L0919_ps_Champion->MaximumMana += AP0638_ui_SkillLevelAfter + (AP0638_ui_SkillLevelAfter >> 1);
                                        L0919_ps_Champion->Statistics[C3_STATISTIC_WISDOM][C0_MAXIMUM] += L0921_i_MajorStatisticIncrease;
                                        goto T0304016;
                                case C02_SKILL_PRIEST:
#ifdef X732_I34E
                                        L2248_pc_ = "";
                                        L2249_pc_ = " JUST GAINED A PRIEST LEVEL!";
#endif
#ifdef X736_I34M
                                        L2248_pc_ = F0758_TranslateLanguage(C65_TEXT);
                                        L2249_pc_ = F0758_TranslateLanguage(C66_TEXT);
#endif
                                        AL0915_ui_StaminaAmount /= 25;
                                        L0919_ps_Champion->MaximumMana += AP0638_ui_SkillLevelAfter;
                                        AP0638_ui_SkillLevelAfter += (AP0638_ui_SkillLevelAfter + 1) >> 1;
                                        L0919_ps_Champion->Statistics[C3_STATISTIC_WISDOM][C0_MAXIMUM] += L0920_i_MinorStatisticIncrease;
                                        T0304016:
                                        if ((L0919_ps_Champion->MaximumMana += F0024_MAIN_GetMinimumValue(M004_RANDOM(4), L0922_i_BaseSkillLevel - 1)) > 900) {
                                                L0919_ps_Champion->MaximumMana = 900;
                                        }
                                        L0919_ps_Champion->Statistics[C5_STATISTIC_ANTIMAGIC][C0_MAXIMUM] += M004_RANDOM(4);
                        }
                        if ((L0919_ps_Champion->MaximumHealth += AP0638_ui_SkillLevelAfter + M002_RANDOM((AP0638_ui_SkillLevelAfter >> 1) + 1)) > 999) {
                                L0919_ps_Champion->MaximumHealth = 999;
                        }
                        if ((L0919_ps_Champion->MaximumStamina += AL0915_ui_StaminaAmount + M002_RANDOM((AL0915_ui_StaminaAmount >> 1) + 1)) > 9999) {
                                L0919_ps_Champion->MaximumStamina = 9999;
                        }
                        M008_SET(L0919_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                        G2016_ai_SkillRecentlyUpgraded[P0636_ui_ChampionIndex][L0916_ui_BaseSkillIndex] = C1_TRUE;
                        F0292_CHAMPION_DrawState(P0636_ui_ChampionIndex);
                        F0051_TEXT_MESSAGEAREA_PrintLineFeed();
                        AP0638_ui_ChampionColor = G0046_auc_Graphic562_ChampionColor[P0636_ui_ChampionIndex];
                        F0047_TEXT_MESSAGEAREA_PrintMessage(AP0638_ui_ChampionColor, L2248_pc_);
                        F0047_TEXT_MESSAGEAREA_PrintMessage(AP0638_ui_ChampionColor, L0919_ps_Champion->Name);
                        F0047_TEXT_MESSAGEAREA_PrintMessage(AP0638_ui_ChampionColor, L2249_pc_);
                }
        }
}

int16_t F0305_CHAMPION_GetThrowingStaminaCost(
THING P0639_T_Thing FINAL_SEPARATOR
{
        REGISTER int16_t L0923_i_Weight;
        REGISTER int16_t L0924_i_StaminaCost;


        L0924_i_StaminaCost = F0026_MAIN_GetBoundedValue(1, L0923_i_Weight = F0140_DUNGEON_GetObjectWeight(P0639_T_Thing) >> 1, 10);
        while ((L0923_i_Weight -= 10) > 0) {
                L0924_i_StaminaCost += L0923_i_Weight >> 1;
        }
        return L0924_i_StaminaCost;
}

STATICFUNCTION int16_t F0306_CHAMPION_GetStaminaAdjustedValue(
REGISTER CHAMPION* P0640_ps_Champion SEPARATOR
REGISTER int16_t   P0641_i_Value     FINAL_SEPARATOR
{
        REGISTER int16_t L0925_i_CurrentStamina;
        REGISTER int16_t L0926_i_HalfMaximumStamina;


        if ((L0925_i_CurrentStamina = P0640_ps_Champion->CurrentStamina) < (L0926_i_HalfMaximumStamina = P0640_ps_Champion->MaximumStamina >> 1)) {
                return (P0641_i_Value >>= 1) + (int16_t)(((long)P0641_i_Value * (long)L0925_i_CurrentStamina) / L0926_i_HalfMaximumStamina); /* BUGX_XX: A champion may temporarily benefit from a larger than expected Strength or Maximum Load value. If the stamina of a champion drops below half of its maximum value, the Strength and Maximum Load values of the champion are decreased. However, while computing the decreased value, two sub expressions are not evaluated in the same order by all compilers. In order to compute the correct adjusted value, the first operand of the addition must be evaluated first so that its updated value is used in the second operand. However, there is no guarantee that the code produced by the compiler will evaluate the operands in this particular order. The following compilers produce code showing the expected behavior where the first operand of the addition is executed first: Megamax C (DM 1.x and CSB 2.x for Atari ST), High C (DM 2.0 and CSB 3.1 for FM-Towns), THINK C 4.0 (CSB 3.x for Amiga and X68000, DM 3.0 for X68000). The following compilers produce code showing the unexpected behavior where the second operand of the addition is executed first: Aztec C 3.6a (DM 2.x for Amiga), MPW IIGS C compiler (DM 2.x for Apple IIGS), Turbo C 2.0 (DM 2.0 and CSB 3.1 for PC-98), Turbo C++ 1.01 (DM 3.4 for PC), THINK C 5.0 (DM 3.6 Amiga). For example, on platforms where operands are evaluated in the expected order a champion with a maximum stamina of 100 and strong enough to carry 40 KG when his current stamina is more than half its maximum is able to carry only 29 KG if its current stamina drops to 45. On platforms where operands are not evaluated in the expected order the resulting maximum load is 38 KG instead which is higher than expected */
        }
        else {
                return P0641_i_Value;
        }
}

unsigned int16_t F0307_CHAMPION_GetStatisticAdjustedAttack(
CHAMPION*        P0642_ps_Champion       SEPARATOR
unsigned int16_t P0643_ui_StatisticIndex SEPARATOR
unsigned int16_t P0644_ui_Attack         FINAL_SEPARATOR
{
        REGISTER int16_t L0927_i_Factor;


        if ((L0927_i_Factor = 170 - P0642_ps_Champion->Statistics[P0643_ui_StatisticIndex][C1_CURRENT]) < 16) { /* BUG0_41 The Antifire and Antimagic statistics are completely ignored. The Vitality statistic is ignored against poison and to determine the probability of being wounded. Vitality is still used normally to compute the defense against wounds and the speed of health regeneration. A bug in the Megamax C compiler produces wrong machine code for this statement. It always returns 0 for the current statistic value so that L0927_i_Factor = 170 in all cases */
                return P0644_ui_Attack >> 3;
        }
        return F0030_MAIN_GetScaledProduct(P0644_ui_Attack, 7, L0927_i_Factor);
}

BOOLEAN F0308_CHAMPION_IsLucky(
CHAMPION*                 P0645_ps_Champion   SEPARATOR
REGISTER unsigned int16_t P0646_ui_Percentage FINAL_SEPARATOR
#define AP0646_ui_IsLucky P0646_ui_Percentage
{
        REGISTER char* L0928_Statistic;


        if (M005_RANDOM(2) && (M002_RANDOM(100) > P0646_ui_Percentage)) {
                return C1_TRUE;
        }
        L0928_Statistic = P0645_ps_Champion->Statistics[C0_STATISTIC_LUCK];
        if (L0928_Statistic[C1_CURRENT] <= 0) {
                AP0646_ui_IsLucky = 0;
        } else {
                AP0646_ui_IsLucky = (M002_RANDOM((unsigned char)L0928_Statistic[C1_CURRENT] << 1) > P0646_ui_Percentage);
        }
        L0928_Statistic[C1_CURRENT] = F0026_MAIN_GetBoundedValue(L0928_Statistic[C2_MINIMUM], L0928_Statistic[C1_CURRENT] + (AP0646_ui_IsLucky ? -2 : 2), L0928_Statistic[C0_MAXIMUM]);
        return AP0646_ui_IsLucky;
}

unsigned int16_t F0309_CHAMPION_GetMaximumLoad(
REGISTER CHAMPION* P0647_ps_Champion FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0929_ui_MaximumLoad;
        REGISTER unsigned int16_t L0930_ui_Wounds;


        L0929_ui_MaximumLoad = (P0647_ps_Champion->Statistics[C1_STATISTIC_STRENGTH][C1_CURRENT] << 3) + 100;
        L0929_ui_MaximumLoad = F0306_CHAMPION_GetStaminaAdjustedValue(P0647_ps_Champion, L0929_ui_MaximumLoad);
        if (L0930_ui_Wounds = P0647_ps_Champion->Wounds) {
                L0929_ui_MaximumLoad -= L0929_ui_MaximumLoad >> (M007_GET(L0930_ui_Wounds, MASK0x0010_WOUND_LEGS) ? 2 : 3);
        }
        if (F0033_OBJECT_GetIconIndex(P0647_ps_Champion->Slots[C05_SLOT_FEET]) == C119_ICON_ARMOUR_ELVEN_BOOTS) {
                L0929_ui_MaximumLoad += L0929_ui_MaximumLoad >> 4;
        }
        L0929_ui_MaximumLoad += 9;
        L0929_ui_MaximumLoad -= L0929_ui_MaximumLoad % 10; /* Round the value to the next multiple of 10 */
        return L0929_ui_MaximumLoad;
}

int16_t F0310_CHAMPION_GetMovementTicks(
REGISTER CHAMPION* P0648_ps_Champion FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0931_ui_Multiple;
#define AL0931_ui_Load       L0931_ui_Multiple
#define AL0931_ui_WoundTicks L0931_ui_Multiple
        REGISTER unsigned int16_t L0933_ui_Ticks;
        REGISTER unsigned int16_t L0932_ui_MaximumLoad;


        if ((L0932_ui_MaximumLoad = F0309_CHAMPION_GetMaximumLoad(P0648_ps_Champion)) > (AL0931_ui_Load = P0648_ps_Champion->Load)) { /* BUG0_72 The party moves very slowly even though no champion 'Load' value is drawn in red. When the Load of a champion has exactly the maximum value he can carry then the Load is drawn in yellow but the speed is the same as when the champion is overloaded (when the Load is drawn in red). The comparison operator should be >= instead of > */
                L0933_ui_Ticks = 2;
                if (((long)AL0931_ui_Load << 3) > ((long)L0932_ui_MaximumLoad * 5)) {
                        L0933_ui_Ticks++;
                }
                AL0931_ui_WoundTicks = 1;
        } else {
                L0933_ui_Ticks = 4 + (((AL0931_ui_Load - L0932_ui_MaximumLoad) << 2) / L0932_ui_MaximumLoad);
                AL0931_ui_WoundTicks = 2;
        }
        if (M007_GET(P0648_ps_Champion->Wounds, MASK0x0020_WOUND_FEET)) {
                L0933_ui_Ticks += AL0931_ui_WoundTicks;
        }
        if (F0033_OBJECT_GetIconIndex(P0648_ps_Champion->Slots[C05_SLOT_FEET]) == C194_ICON_ARMOUR_BOOT_OF_SPEED) {
                L0933_ui_Ticks--;
        }
        return L0933_ui_Ticks;
}

unsigned int16_t F0311_CHAMPION_GetDexterity(
REGISTER CHAMPION* P0649_ps_Champion FINAL_SEPARATOR
{
        REGISTER int16_t L0934_i_Dexterity;


        L0934_i_Dexterity = M003_RANDOM(8) + P0649_ps_Champion->Statistics[C2_STATISTIC_DEXTERITY][C1_CURRENT];
        L0934_i_Dexterity -= (int16_t)(((long)(L0934_i_Dexterity >> 1) * (long)P0649_ps_Champion->Load) / F0309_CHAMPION_GetMaximumLoad(P0649_ps_Champion));
        L0934_i_Dexterity = F0025_MAIN_GetMaximumValue(2, L0934_i_Dexterity);
        if (G0300_B_PartyIsResting) {
                L0934_i_Dexterity >>= 1;
        }
        return F0026_MAIN_GetBoundedValue(1 + M003_RANDOM(8), L0934_i_Dexterity >> 1, 100 - M003_RANDOM(8));
}

unsigned int16_t F0312_CHAMPION_GetStrength(
REGISTER int16_t P0650_i_ChampionIndex SEPARATOR
int16_t          P0651_i_SlotIndex     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0939_ps_Champion;
        REGISTER WEAPON_INFO* L0940_ps_WeaponInfo;
        REGISTER unsigned int16_t L0937_ui_Multiple;
        REGISTER int16_t L0935_i_Strength;
        REGISTER unsigned int16_t L0936_ui_Multiple;
#define AL0936_ui_ObjectWeight L0936_ui_Multiple
#define AL0936_ui_SkillLevel   L0936_ui_Multiple
#define AL0937_ui_OneSixteenthMaximumLoad L0937_ui_Multiple
#define AL0937_ui_Class                   L0937_ui_Multiple
        REGISTER THING L0938_T_Thing;
        unsigned int16_t L0941_ui_LoadThreshold;


        L0939_ps_Champion = &M516_CHAMPIONS[P0650_i_ChampionIndex];
        L0935_i_Strength = M003_RANDOM(16) + L0939_ps_Champion->Statistics[C1_STATISTIC_STRENGTH][C1_CURRENT];
        L0938_T_Thing = L0939_ps_Champion->Slots[P0651_i_SlotIndex];
        if ((AL0936_ui_ObjectWeight = F0140_DUNGEON_GetObjectWeight(L0938_T_Thing)) <= (AL0937_ui_OneSixteenthMaximumLoad = F0309_CHAMPION_GetMaximumLoad(L0939_ps_Champion) >> 4)) {
                L0935_i_Strength += AL0936_ui_ObjectWeight - 12;
        } else {
                if (AL0936_ui_ObjectWeight <= (L0941_ui_LoadThreshold = AL0937_ui_OneSixteenthMaximumLoad + ((AL0937_ui_OneSixteenthMaximumLoad - 12) >> 1))) {
                        L0935_i_Strength += (AL0936_ui_ObjectWeight - AL0937_ui_OneSixteenthMaximumLoad) >> 1;
                } else {
                        L0935_i_Strength -= (AL0936_ui_ObjectWeight - L0941_ui_LoadThreshold) << 1;
                }
        }
        if (M012_TYPE(L0938_T_Thing) == C05_THING_TYPE_WEAPON) {
                L0940_ps_WeaponInfo = F0158_DUNGEON_GetWeaponInfo(L0938_T_Thing);
                L0935_i_Strength += L0940_ps_WeaponInfo->Strength;
                AL0936_ui_SkillLevel = 0;
                AL0937_ui_Class = L0940_ps_WeaponInfo->Class;
                if ((AL0937_ui_Class == C000_CLASS_SWING_WEAPON) || (AL0937_ui_Class == C002_CLASS_DAGGER_AND_AXES)) {
                        AL0936_ui_SkillLevel = F0303_CHAMPION_GetSkillLevel(P0650_i_ChampionIndex, C04_SKILL_SWING);
                }
                if ((AL0937_ui_Class != C000_CLASS_SWING_WEAPON) && (AL0937_ui_Class < C016_CLASS_FIRST_BOW)) {
                        AL0936_ui_SkillLevel += F0303_CHAMPION_GetSkillLevel(P0650_i_ChampionIndex, C10_SKILL_THROW);
                }
                if ((AL0937_ui_Class >= C016_CLASS_FIRST_BOW) && (AL0937_ui_Class < C112_CLASS_FIRST_MAGIC_WEAPON)) {
                        AL0936_ui_SkillLevel += F0303_CHAMPION_GetSkillLevel(P0650_i_ChampionIndex, C11_SKILL_SHOOT);
                }
                L0935_i_Strength += AL0936_ui_SkillLevel << 1;
        }
        L0935_i_Strength = F0306_CHAMPION_GetStaminaAdjustedValue(L0939_ps_Champion, L0935_i_Strength);
        if (M007_GET(L0939_ps_Champion->Wounds, (P0651_i_SlotIndex == C00_SLOT_READY_HAND) ? MASK0x0001_WOUND_READY_HAND : MASK0x0002_WOUND_ACTION_HAND)) {
                L0935_i_Strength >>= 1;
        }
        return F0026_MAIN_GetBoundedValue(0, L0935_i_Strength >> 1, 100);
}

int16_t F0313_CHAMPION_GetWoundDefense(
int16_t                   P0652_i_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0653_ui_WoundIndex   FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0946_ps_Champion;
        REGISTER ARMOUR_INFO* L0947_ps_ArmourInfo;
        REGISTER int16_t L0942_i_Multiple;
#define AL0942_i_SlotIndex    L0942_i_Multiple
#define AL0942_i_WoundDefense L0942_i_Multiple
        REGISTER BOOLEAN L0944_B_UseSharpDefense;
        REGISTER THING L0945_T_Thing;
        REGISTER int16_t L0943_ui_ArmourShieldDefense;


        L0946_ps_Champion = &M516_CHAMPIONS[P0652_i_ChampionIndex];
        if (L0944_B_UseSharpDefense = M007_GET(P0653_ui_WoundIndex, MASK0x8000_USE_SHARP_DEFENSE)) {
                M009_CLEAR(P0653_ui_WoundIndex, MASK0x8000_USE_SHARP_DEFENSE);
        }
        for (L0943_ui_ArmourShieldDefense = 0, AL0942_i_SlotIndex = C00_SLOT_READY_HAND; AL0942_i_SlotIndex <= C01_SLOT_ACTION_HAND; AL0942_i_SlotIndex++) {
                if (M012_TYPE(L0945_T_Thing = L0946_ps_Champion->Slots[AL0942_i_SlotIndex]) == C06_THING_TYPE_ARMOUR) {
                        L0947_ps_ArmourInfo = (ARMOUR_INFO*)F0156_DUNGEON_GetThingData(L0945_T_Thing);
                        L0947_ps_ArmourInfo = &G0239_as_Graphic559_ArmourInfo[((ARMOUR*)L0947_ps_ArmourInfo)->Type];
                        if (M007_GET(L0947_ps_ArmourInfo->Attributes, MASK0x0080_IS_A_SHIELD)) {
                                L0943_ui_ArmourShieldDefense += ((F0312_CHAMPION_GetStrength(P0652_i_ChampionIndex, AL0942_i_SlotIndex) + F0143_DUNGEON_GetArmourDefense(L0947_ps_ArmourInfo, L0944_B_UseSharpDefense)) * G0050_auc_Graphic562_WoundDefenseFactor[P0653_ui_WoundIndex]) >> ((AL0942_i_SlotIndex == P0653_ui_WoundIndex) ? 4 : 5);
                        }
                }
        }
        AL0942_i_WoundDefense = M002_RANDOM((L0946_ps_Champion->Statistics[C4_STATISTIC_VITALITY][C1_CURRENT] >> 3) + 1);
        if (L0944_B_UseSharpDefense) {
                AL0942_i_WoundDefense >>= 1;
        }
        AL0942_i_WoundDefense += L0946_ps_Champion->ActionDefense + L0946_ps_Champion->ShieldDefense + G0407_s_Party.ShieldDefense + L0943_ui_ArmourShieldDefense;
        if ((P0653_ui_WoundIndex > C01_SLOT_ACTION_HAND) && (M012_TYPE(L0945_T_Thing = L0946_ps_Champion->Slots[P0653_ui_WoundIndex]) == C06_THING_TYPE_ARMOUR)) {
                L0947_ps_ArmourInfo = (ARMOUR_INFO*)F0156_DUNGEON_GetThingData(L0945_T_Thing);
                AL0942_i_WoundDefense += F0143_DUNGEON_GetArmourDefense(&G0239_as_Graphic559_ArmourInfo[((ARMOUR*)L0947_ps_ArmourInfo)->Type], L0944_B_UseSharpDefense);
        }
        if (M007_GET(L0946_ps_Champion->Wounds, 1 << P0653_ui_WoundIndex)) {
                AL0942_i_WoundDefense -= 8 + M004_RANDOM(4);
        }
        if (G0300_B_PartyIsResting) {
                AL0942_i_WoundDefense >>= 1;
        }
        return F0026_MAIN_GetBoundedValue(0, AL0942_i_WoundDefense >> 1, 100);
}




void F0314_CHAMPION_WakeUp(
void
)
{


        G0321_B_StopWaitingForPlayerInput = C1_TRUE;
        G0300_B_PartyIsResting = C0_FALSE;
        G0318_i_WaitForInputMaximumVerticalBlankCount = 12;
        F0098_DUNGEONVIEW_DrawFloorAndCeiling();
        G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;
        G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;
        G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;
        G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;
        F0357_COMMAND_DiscardAllInput();
}

int16_t F0315_CHAMPION_GetScentOrdinal(
unsigned int16_t P0654_i_MapX SEPARATOR
unsigned int16_t P0655_i_MapY FINAL_SEPARATOR
{
        REGISTER unsigned int16_t* L0951_pui_Scent;
        REGISTER int16_t L0950_i_ScentIndex;
        REGISTER unsigned int16_t L0949_ui_SearchedScent;
        SCENT L0952_s_SearchedScent;


        if (L0950_i_ScentIndex = G0407_s_Party.ScentCount) {
                L0952_s_SearchedScent.Location.MapX = P0654_i_MapX;
                L0952_s_SearchedScent.Location.MapY = P0655_i_MapY;
                L0952_s_SearchedScent.Location.MapIndex = G0272_i_CurrentMapIndex;
                L0949_ui_SearchedScent = L0952_s_SearchedScent.Scent;
                L0951_pui_Scent = (unsigned int16_t*)&G0407_s_Party.Scents[L0950_i_ScentIndex--];
                do {
                        if ((*(--L0951_pui_Scent)) == L0949_ui_SearchedScent) {
                                return M000_INDEX_TO_ORDINAL(L0950_i_ScentIndex);
                        }
                } while (L0950_i_ScentIndex--);
        }
        return 0;
}

void F0316_CHAMPION_DeleteScent(
REGISTER unsigned int16_t P0656_ui_ScentIndex FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0953_ui_Count;


        if (L0953_ui_Count = --G0407_s_Party.ScentCount - P0656_ui_ScentIndex) {
                F0007_MAIN_CopyBytes(M772_CAST_PC(&G0407_s_Party.Scents[P0656_ui_ScentIndex + 1]), M772_CAST_PC(&G0407_s_Party.Scents[P0656_ui_ScentIndex]), M543_BYTE_COUNT_INT(L0953_ui_Count * sizeof(SCENT)));
                F0007_MAIN_CopyBytes(M772_CAST_PC(&G0407_s_Party.ScentStrengths[P0656_ui_ScentIndex + 1]), M772_CAST_PC(&G0407_s_Party.ScentStrengths[P0656_ui_ScentIndex]), M543_BYTE_COUNT_INT(L0953_ui_Count));
        }
        if (P0656_ui_ScentIndex < G0407_s_Party.FirstScentIndex) {
                G0407_s_Party.FirstScentIndex--;
        }
        if (P0656_ui_ScentIndex < G0407_s_Party.LastScentIndex) {
                G0407_s_Party.LastScentIndex--;
        }
}

void F0317_CHAMPION_AddScentStrength(
unsigned int16_t          P0657_i_MapX        SEPARATOR
unsigned int16_t          P0658_i_MapY        SEPARATOR
REGISTER unsigned int16_t P0659_ui_CycleCount FINAL_SEPARATOR
{
        REGISTER SCENT* L0957_ps_Scent; /* BUG0_00 Useless code */
        REGISTER int16_t L0954_i_ScentIndex;
        REGISTER BOOLEAN L0956_B_CycleCountDefined;
        REGISTER BOOLEAN L0955_B_Merge;
        SCENT L0958_s_Scent; /* BUG0_00 Useless code */


        if (L0954_i_ScentIndex = G0407_s_Party.ScentCount) {
                if (L0955_B_Merge = M007_GET(P0659_ui_CycleCount, MASK0x8000_MERGE_CYCLES)) {
                        M009_CLEAR(P0659_ui_CycleCount, MASK0x8000_MERGE_CYCLES);
                }
                L0958_s_Scent.Location.MapX = P0657_i_MapX; /* BUG0_00 Useless code */
                L0958_s_Scent.Location.MapY = P0658_i_MapY; /* BUG0_00 Useless code */
                L0958_s_Scent.Location.MapIndex = G0272_i_CurrentMapIndex; /* BUG0_00 Useless code */
                L0957_ps_Scent = G0407_s_Party.Scents; /* BUG0_00 Useless code */
                L0956_B_CycleCountDefined = C0_FALSE;
                while (L0954_i_ScentIndex--) {
                        if ((*L0957_ps_Scent++).Scent == L0958_s_Scent.Scent) {
                                if (!L0956_B_CycleCountDefined) {
                                        L0956_B_CycleCountDefined = C1_TRUE;
                                        if (L0955_B_Merge) {
                                                P0659_ui_CycleCount = F0025_MAIN_GetMaximumValue(G0407_s_Party.ScentStrengths[L0954_i_ScentIndex], P0659_ui_CycleCount);
                                        } else {
                                                P0659_ui_CycleCount = F0024_MAIN_GetMinimumValue(80, G0407_s_Party.ScentStrengths[L0954_i_ScentIndex] + P0659_ui_CycleCount);
                                        }
                                }
                                G0407_s_Party.ScentStrengths[L0954_i_ScentIndex] = P0659_ui_CycleCount;
                        }
                }
        }
}

STATICFUNCTION void F0318_CHAMPION_DropAllObjects(
REGISTER unsigned int16_t P0660_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0961_ui_SlotIndex;
        REGISTER THING L0960_T_Thing;
        REGISTER unsigned int16_t L0959_ui_Cell;


        L0959_ui_Cell = M516_CHAMPIONS[P0660_ui_ChampionIndex].Cell;
        for (L0961_ui_SlotIndex = C00_SLOT_READY_HAND; L0961_ui_SlotIndex < C30_SLOT_CHEST_1; L0961_ui_SlotIndex++) {
                if ((L0960_T_Thing = F0300_CHAMPION_GetObjectRemovedFromSlot(P0660_ui_ChampionIndex, G0057_ai_Graphic562_SlotDropOrder[L0961_ui_SlotIndex])) != C0xFFFF_THING_NONE) {
                        F0267_MOVE_GetMoveResult_CPSCE(M015_THING_WITH_NEW_CELL(L0960_T_Thing, L0959_ui_Cell), CM1_MAPX_NOT_ON_A_SQUARE, 0, G0306_i_PartyMapX, G0307_i_PartyMapY);
                }
        }
}

void F0319_CHAMPION_Kill(
REGISTER unsigned int16_t P0661_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0965_ps_Champion;
        REGISTER JUNK* L0966_ps_Junk;
        REGISTER unsigned int16_t L0962_ui_Multiple;
#define AL0962_ui_Cell              L0962_ui_Multiple
#define AL0962_ui_ChampionIconIndex L0962_ui_Multiple
        REGISTER unsigned int16_t L0963_ui_AliveChampionIndex;
        REGISTER THING L0964_T_Thing;


        L0965_ps_Champion = &M516_CHAMPIONS[P0661_ui_ChampionIndex];
        if (P0661_ui_ChampionIndex == M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal)) {
                F0388_MENUS_ClearActingChampion();
        }
        L0965_ps_Champion->CurrentHealth = 0;
        M008_SET(L0965_ps_Champion->Attributes, MASK0x1000_STATUS_BOX);
        if (M000_INDEX_TO_ORDINAL(P0661_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal) {
                if (G0331_B_PressingEye) {
                        G0331_B_PressingEye = C0_FALSE;
                        G0597_B_IgnoreMouseMovements = C0_FALSE;
                        if (!G0415_ui_LeaderEmptyHanded) {
                                F0034_OBJECT_DrawLeaderHandObjectName(G4055_s_LeaderHandObject.Thing);
                        }
                        G0587_i_HideMousePointerRequestCount = 1;
                        M523_MOUSE_ShowPointer();
                }
                        if (G0333_B_PressingMouth) {
                                G0333_B_PressingMouth = C0_FALSE;
                                G0597_B_IgnoreMouseMovements = C0_FALSE;
                                G0587_i_HideMousePointerRequestCount = 1;
                                M523_MOUSE_ShowPointer();
                        }
                F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY);
        }
        if ((P0661_ui_ChampionIndex == G0411_i_LeaderIndex) && G2152_B_PressingClosedImaginaryFakeWall) {
                G2152_B_PressingClosedImaginaryFakeWall = C0_FALSE;
                G0597_B_IgnoreMouseMovements = C0_FALSE;
                G0587_i_HideMousePointerRequestCount = 1;
                M523_MOUSE_ShowPointer();
        }
        F0318_CHAMPION_DropAllObjects(P0661_ui_ChampionIndex);
        L0964_T_Thing = F0166_DUNGEON_GetUnusedThing(MASK0x8000_CHAMPION_BONES | C10_THING_TYPE_JUNK);
        if (L0964_T_Thing == C0xFFFF_THING_NONE) {
        } else {
                L0966_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L0964_T_Thing);
                L0966_ps_Junk->Type = C05_JUNK_BONES;
                L0966_ps_Junk->DoNotDiscard = C1_TRUE;
                L0966_ps_Junk->ChargeCount = P0661_ui_ChampionIndex;
                AL0962_ui_Cell = L0965_ps_Champion->Cell;
                F0267_MOVE_GetMoveResult_CPSCE(M015_THING_WITH_NEW_CELL(L0964_T_Thing, AL0962_ui_Cell), CM1_MAPX_NOT_ON_A_SQUARE, 0, G0306_i_PartyMapX, G0307_i_PartyMapY);
        }
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        L0965_ps_Champion->SymbolStep = 0;
        L0965_ps_Champion->Symbols[0] = '\0';
        L0965_ps_Champion->Direction = G0308_i_PartyDirection;
        L0965_ps_Champion->MaximumDamageReceived = 0;
        AL0962_ui_ChampionIconIndex = M026_CHAMPION_ICON_INDEX(AL0962_ui_Cell, G0308_i_PartyDirection);
        if (M000_INDEX_TO_ORDINAL(AL0962_ui_ChampionIconIndex) == G0599_ui_UseChampionIconOrdinalAsMousePointerBitmap) {
                G0598_B_MousePointerBitmapUpdated = C1_TRUE;
                G0599_ui_UseChampionIconOrdinalAsMousePointerBitmap = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);
                G0592_B_BuildMousePointerScreenAreaRequested = C1_TRUE;
        } else {
                F0621_ClearChampionIconBox(AL0962_ui_ChampionIconIndex);
        }
        if (L0965_ps_Champion->PoisonEventCount) {
                F0323_CHAMPION_Unpoison(P0661_ui_ChampionIndex);
        }
        F0700_TriggerImmediateMouseEvent();
        F0292_CHAMPION_DrawState(P0661_ui_ChampionIndex);
        for (L0963_ui_AliveChampionIndex = C00_CHAMPION_FIRST, L0965_ps_Champion = M516_CHAMPIONS; L0963_ui_AliveChampionIndex < G0305_ui_PartyChampionCount; L0963_ui_AliveChampionIndex++, L0965_ps_Champion++) {
                if (L0965_ps_Champion->CurrentHealth)
                        break;
        }
        if (L0963_ui_AliveChampionIndex == G0305_ui_PartyChampionCount) { /* BUG0_43 The game does not end if the last living champion in the party is killed while looking at a candidate champion in a portrait. The condition to end the game when the whole party is killed is not true because the code considers the candidate champion as alive (in the loop above) */
                G0303_B_PartyDead = C1_TRUE;
        } else {
                if (P0661_ui_ChampionIndex == G0411_i_LeaderIndex) {
                        F0368_COMMAND_SetLeader(L0963_ui_AliveChampionIndex);
                }
                if (P0661_ui_ChampionIndex == G0514_i_MagicCasterChampionIndex) {
                        F0394_MENUS_SetMagicCasterAndDrawSpellArea(L0963_ui_AliveChampionIndex);
                } else {
                        F0393_MENUS_DrawSpellAreaControls(G0514_i_MagicCasterChampionIndex);
                }
        }
        F0078_MOUSE_DisableScreenUpdate();
}

void F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds(
void
)
{
        REGISTER CHAMPION* L0971_ps_Champion;
        REGISTER unsigned int16_t L0967_ui_ChampionIndex;
        REGISTER int16_t L0969_i_Multiple;
#define AL0969_i_Health     L0969_i_Multiple
#define AL0969_i_X          L0969_i_Multiple
#define AL0969_i_EventIndex L0969_i_Multiple
        REGISTER unsigned int16_t L0970_i_PendingWounds;
        REGISTER unsigned int16_t L0968_ui_PendingDamage;
        EVENT L0974_s_Event;


        L0971_ps_Champion = M516_CHAMPIONS;
        for (L0967_ui_ChampionIndex = C00_CHAMPION_FIRST; L0967_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0967_ui_ChampionIndex++, L0971_ps_Champion++) {
                M008_SET(L0971_ps_Champion->Wounds, L0970_i_PendingWounds = G0410_ai_ChampionPendingWounds[L0967_ui_ChampionIndex]);
                G0410_ai_ChampionPendingWounds[L0967_ui_ChampionIndex] = 0;
                if (!(L0968_ui_PendingDamage = G0409_ai_ChampionPendingDamage[L0967_ui_ChampionIndex]))
                        continue;
                G0409_ai_ChampionPendingDamage[L0967_ui_ChampionIndex] = 0;
                if (!(AL0969_i_Health = L0971_ps_Champion->CurrentHealth))
                        continue;
                if ((AL0969_i_Health -= L0968_ui_PendingDamage) <= 0) {
                        F0319_CHAMPION_Kill(L0967_ui_ChampionIndex);
                } else {
                        L0971_ps_Champion->CurrentHealth = AL0969_i_Health;
                        M008_SET(L0971_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                        if (L0970_i_PendingWounds) {
                                M008_SET(L0971_ps_Champion->Attributes, MASK0x2000_WOUNDS);
                        }
                        F0623_DrawDamageToChampion_F0320_sub(L0967_ui_ChampionIndex, L0968_ui_PendingDamage);
                        if ((AL0969_i_EventIndex = L0971_ps_Champion->HideDamageReceivedEventIndex) == -1) {
                                L0974_s_Event.A.A.Type = C12_EVENT_HIDE_DAMAGE_RECEIVED;
                                M033_SET_MAP_AND_TIME(L0974_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + 5);
                                L0974_s_Event.A.A.Priority = L0967_ui_ChampionIndex;
                                L0971_ps_Champion->HideDamageReceivedEventIndex = F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0974_s_Event);
                        } else {
                                M033_SET_MAP_AND_TIME(G0370_ps_Events[AL0969_i_EventIndex].Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + 5);
                                F0236_TIMELINE_FixPlacement(F0235_TIMELINE_GetIndex(AL0969_i_EventIndex));
                        }
                }
        }
}

int16_t F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(
REGISTER int16_t P0662_i_ChampionIndex  SEPARATOR
REGISTER int16_t P0663_i_Attack         SEPARATOR
unsigned int16_t P0664_ui_AllowedWounds SEPARATOR
unsigned int16_t P0665_ui_AttackType    FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0979_ps_Champion;
        REGISTER int16_t L0976_i_Multiple;
#define AL0976_i_WoundIndex     L0976_i_Multiple
#define AL0976_i_WisdomFactor   L0976_i_Multiple
#define AL0976_i_AdjustedAttack L0976_i_Multiple
        REGISTER unsigned int16_t L0977_ui_Defense;
        REGISTER unsigned int16_t L0978_ui_WoundCount;


        if ((P0662_i_ChampionIndex == CM1_CHAMPION_NONE) || (M000_INDEX_TO_ORDINAL(P0662_i_ChampionIndex) == G0299_ui_CandidateChampionOrdinal || G0302_B_GameWon)) { /* BUG0_00 Useless code. This condition is never true (except in the context of BUG0_43) */
                return 0;
        }
        if (P0663_i_Attack <= 0) {
                return 0;
        }
        L0979_ps_Champion = &M516_CHAMPIONS[P0662_i_ChampionIndex];
        if (!L0979_ps_Champion->CurrentHealth) {
                T0321004:
                return 0;
        }
        if (P0665_ui_AttackType != C0_ATTACK_NORMAL) {
                for (L0978_ui_WoundCount = 0, AL0976_i_WoundIndex = C00_SLOT_READY_HAND, L0977_ui_Defense = 0; AL0976_i_WoundIndex <= C05_SLOT_FEET; AL0976_i_WoundIndex++) {
                        if (P0664_ui_AllowedWounds & (1 << AL0976_i_WoundIndex)) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE test */
                                L0978_ui_WoundCount++;
                                L0977_ui_Defense += F0313_CHAMPION_GetWoundDefense(P0662_i_ChampionIndex, AL0976_i_WoundIndex | ((P0665_ui_AttackType == C4_ATTACK_SHARP) ? MASK0x8000_USE_SHARP_DEFENSE : MASK0x0000_DO_NOT_USE_SHARP_DEFENSE));
                        }
                }
                if (L0978_ui_WoundCount) {
                        L0977_ui_Defense /= L0978_ui_WoundCount;
                }
                switch (P0665_ui_AttackType) {
                        case C6_ATTACK_PSYCHIC:
                                if ((AL0976_i_WisdomFactor = 115 - L0979_ps_Champion->Statistics[C3_STATISTIC_WISDOM][C1_CURRENT]) <= 0) {
                                        P0663_i_Attack = 0;
                                } else {
#ifdef PC_FIX_CODE_SIZE
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
        P0663_i_Attack++;
#endif
                                        P0663_i_Attack = F0030_MAIN_GetScaledProduct(P0663_i_Attack, 6, AL0976_i_WisdomFactor);
                                }
                                goto T0321024;
                        case C5_ATTACK_MAGIC:
                                P0663_i_Attack = F0307_CHAMPION_GetStatisticAdjustedAttack(L0979_ps_Champion, C5_STATISTIC_ANTIMAGIC, P0663_i_Attack);
                                P0663_i_Attack -= G0407_s_Party.SpellShieldDefense;
                                goto T0321024;
                        case C1_ATTACK_FIRE:
                                P0663_i_Attack = F0307_CHAMPION_GetStatisticAdjustedAttack(L0979_ps_Champion, C6_STATISTIC_ANTIFIRE, P0663_i_Attack);
                                P0663_i_Attack -= G0407_s_Party.FireShieldDefense;
                                break;
                        case C2_ATTACK_SELF:
                                L0977_ui_Defense >>= 1;
                                L0977_ui_Defense += F0303_CHAMPION_GetSkillLevel(P0662_i_ChampionIndex, C01_SKILL_NINJA);
                        case C3_ATTACK_BLUNT:
                        case C4_ATTACK_SHARP:
                        case C7_ATTACK_LIGHTNING:
                                break;
                }
                if (P0663_i_Attack <= 0)
                        goto T0321004;
                P0663_i_Attack = F0030_MAIN_GetScaledProduct(P0663_i_Attack, 6, 130 - L0977_ui_Defense); /* BUG0_44 A champion may take much more damage than expected after a Black Flame attack or an impact with a Fireball projectile. If the party has a fire shield defense value higher than the fire attack value then the resulting intermediary attack value is negative and damage should be 0. However, the negative value is still used for further computations and the result may be a very high positive attack value which may kill a champion. This can occur only for C1_ATTACK_FIRE and if P0663_i_Attack is negative before calling F0030_MAIN_GetScaledProduct */
                T0321024:
                if (P0663_i_Attack <= 0)
                        goto T0321004;
                if (P0663_i_Attack > (AL0976_i_AdjustedAttack = F0307_CHAMPION_GetStatisticAdjustedAttack(L0979_ps_Champion, C4_STATISTIC_VITALITY, M003_RANDOM(128) + 10))) { /* BUG0_45 Champions with a high vitality are more likely to get wounded. This bug is present in all versions but Atari ST versions are not affected because of BUG0_41 that ignores Vitality while determining the probability of being wounded. The behavior is be the opposite of what it should be: the higher the vitality of a champion, the lower the result of F0307_CHAMPION_GetStatisticAdjustedAttack and the more likely the champion could get wounded (because of more iterations in the loop below) */
                        do {
                                M008_SET(G0410_ai_ChampionPendingWounds[P0662_i_ChampionIndex], (1 << M003_RANDOM(8)) & P0664_ui_AllowedWounds);
                        } while ((P0663_i_Attack > (AL0976_i_AdjustedAttack <<= 1)) && AL0976_i_AdjustedAttack);
                }
                if (G0300_B_PartyIsResting) {
                        F0314_CHAMPION_WakeUp();
                }
        }
        if (P0663_i_Attack <= 0) { /* BUG0_00 Useless code */
                P0663_i_Attack = 0;
        }
        G0409_ai_ChampionPendingDamage[P0662_i_ChampionIndex] += P0663_i_Attack;
        return P0663_i_Attack;
}

void F0322_CHAMPION_Poison(
REGISTER int16_t          P0666_i_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0667_ui_Attack       FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0981_ps_Champion;
        EVENT L0980_s_Event;


        if ((P0666_i_ChampionIndex == CM1_CHAMPION_NONE) || (M000_INDEX_TO_ORDINAL(P0666_i_ChampionIndex) == G0299_ui_CandidateChampionOrdinal))
                return;
        L0981_ps_Champion = &M516_CHAMPIONS[P0666_i_ChampionIndex];
        F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(P0666_i_ChampionIndex, F0025_MAIN_GetMaximumValue(1, P0667_ui_Attack >> 6), MASK0x0000_WOUND_NONE, C0_ATTACK_NORMAL);
        M008_SET(L0981_ps_Champion->Attributes, MASK0x0100_STATISTICS);
        if ((M000_INDEX_TO_ORDINAL(P0666_i_ChampionIndex) == G0423_i_InventoryChampionOrdinal) && (G0424_i_PanelContent == M565_PANEL_FOOD_WATER_POISONED)) {
                M008_SET(L0981_ps_Champion->Attributes, MASK0x0800_PANEL);
        }
        if (--P0667_ui_Attack) {
                L0981_ps_Champion->PoisonEventCount++;
                L0980_s_Event.A.A.Type = C75_EVENT_POISON_CHAMPION;
                L0980_s_Event.A.A.Priority = P0666_i_ChampionIndex;
                M033_SET_MAP_AND_TIME(L0980_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + 36);
                L0980_s_Event.B.Attack = P0667_ui_Attack;
                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0980_s_Event);
        }
        F0292_CHAMPION_DrawState(P0666_i_ChampionIndex);
}

void F0323_CHAMPION_Unpoison(
int16_t P0668_i_ChampionIndex FINAL_SEPARATOR
{
        REGISTER EVENT* L0983_ps_Event;
        REGISTER unsigned int16_t L0982_ui_EventIndex;


        if (P0668_i_ChampionIndex == CM1_CHAMPION_NONE) {
                return;
        }
        for (L0982_ui_EventIndex = 0, L0983_ps_Event = G0370_ps_Events; L0982_ui_EventIndex < G2009_i_LargestUsedEventOrdinal; L0983_ps_Event++, L0982_ui_EventIndex++) {
                if ((L0983_ps_Event->A.A.Type == C75_EVENT_POISON_CHAMPION) && (L0983_ps_Event->A.A.Priority == P0668_i_ChampionIndex)) {
                        F0237_TIMELINE_DeleteEvent(L0982_ui_EventIndex);
                }
        }
        M516_CHAMPIONS[P0668_i_ChampionIndex].PoisonEventCount = 0;
}

int16_t F0324_CHAMPION_DamageAll_GetDamagedChampionCount(
REGISTER unsigned int16_t P0669_ui_Attack    SEPARATOR
int16_t                   P0670_i_Wounds     SEPARATOR
int16_t                   P0671_i_AttackType FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0984_ui_ChampionIndex;
        REGISTER unsigned int16_t L0985_ui_RandomAttack;
        REGISTER int16_t L0986_i_DamagedChampionCount;


        if (!P0669_ui_Attack) {
                return 0;
        }
        P0669_ui_Attack -= (L0985_ui_RandomAttack = (P0669_ui_Attack >> 3) + 1);
        L0985_ui_RandomAttack <<= 1;
        for (L0986_i_DamagedChampionCount = 0, L0984_ui_ChampionIndex = C00_CHAMPION_FIRST; L0984_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0984_ui_ChampionIndex++) {
                if (F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(L0984_ui_ChampionIndex, F0025_MAIN_GetMaximumValue(1, P0669_ui_Attack + M002_RANDOM(L0985_ui_RandomAttack)), P0670_i_Wounds, P0671_i_AttackType)) { /* Actual attack is P0669_ui_Attack +/- (P0669_ui_Attack / 8) */
                        L0986_i_DamagedChampionCount++;
                }
        }
        return L0986_i_DamagedChampionCount;
}

void F0325_CHAMPION_DecrementStamina(
int16_t P0672_i_ChampionIndex SEPARATOR
int16_t P0673_i_Decrement     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0989_ps_Champion;
        REGISTER int16_t L0988_i_Stamina;


        if (P0672_i_ChampionIndex == CM1_CHAMPION_NONE) {
                return;
        }
        L0989_ps_Champion = &M516_CHAMPIONS[P0672_i_ChampionIndex];
        if ((L0988_i_Stamina = (L0989_ps_Champion->CurrentStamina -= P0673_i_Decrement)) <= 0) {
                L0989_ps_Champion->CurrentStamina = 0;
                F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(P0672_i_ChampionIndex, (-L0988_i_Stamina) >> 1, MASK0x0000_WOUND_NONE, C0_ATTACK_NORMAL);
        } else {
                if (L0988_i_Stamina > L0989_ps_Champion->MaximumStamina) {
                        L0989_ps_Champion->CurrentStamina = L0989_ps_Champion->MaximumStamina;
                }
        }
        M008_SET(L0989_ps_Champion->Attributes, MASK0x0200_LOAD | MASK0x0100_STATISTICS);
}

void F0326_CHAMPION_ShootProjectile(
REGISTER CHAMPION* P0674_ps_Champion     SEPARATOR
THING              P0675_T_Thing         SEPARATOR
int16_t            P0676_i_KineticEnergy SEPARATOR
int16_t            P0677_i_Attack        SEPARATOR
int16_t            P0678_i_StepEnergy    FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0990_ui_Direction;


        L0990_ui_Direction = P0674_ps_Champion->Direction;
        F0212_PROJECTILE_Create(P0675_T_Thing, G0306_i_PartyMapX, G0307_i_PartyMapY, M021_NORMALIZE((((P0674_ps_Champion->Cell - L0990_ui_Direction + 1) & 0x0002) >> 1) + L0990_ui_Direction), L0990_ui_Direction, P0676_i_KineticEnergy, P0677_i_Attack, P0678_i_StepEnergy);
        /* BUG0_46 You can run into a projectile shot by a champion. When a champion throws an object, movement is disabled for a short time in the direction in which the object was thrown to prevent the party from running into the projectile. However, when a champion shoots an object or casts a spell, movement is not disabled and the party can run into the projectile. This applies to the actions 'Shoot' (Bow/Claw Bow, Crossbow, Speedbow, Sling), 'Lightning' (Stormring, Bolt Blade/Storm, The Conduit/Serpent Staff), 'Dispell' (Yew Staff, Staff Of Manar/Staff Of Irra), 'Fireball' (Flamitt, Fury/Ra Blade), 'Spit' (none), 'Invoke' (The Firestaff) and to all projectile spells (Fireball, Lightning Bolt, Harm Non Material, Open Door, Poison Bolt, Poison Cloud) */
        G0311_i_ProjectileDisabledMovementTicks = 4;
        G0312_i_LastProjectileDisabledMovementDirection = L0990_ui_Direction;
}

BOOLEAN F0327_CHAMPION_IsProjectileSpellCast(
unsigned int16_t P0679_ui_ChampionIndex      SEPARATOR
THING            P0680_T_Thing               SEPARATOR
REGISTER int16_t P0681_i_KineticEnergy       SEPARATOR
unsigned int16_t P0682_ui_RequiredManaAmount FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0992_ps_Champion;
        REGISTER int16_t L0991_i_StepEnergy;


        L0992_ps_Champion = &M516_CHAMPIONS[P0679_ui_ChampionIndex];
        if (L0992_ps_Champion->CurrentMana < P0682_ui_RequiredManaAmount) {
                return C0_FALSE;
        }
        L0992_ps_Champion->CurrentMana -= P0682_ui_RequiredManaAmount;
        M008_SET(L0992_ps_Champion->Attributes, MASK0x0100_STATISTICS);
        L0991_i_StepEnergy = 10 - F0024_MAIN_GetMinimumValue(8, L0992_ps_Champion->MaximumMana >> 3);
        if (P0681_i_KineticEnergy < (L0991_i_StepEnergy << 2)) {
                P0681_i_KineticEnergy += 3;
                L0991_i_StepEnergy--;
        }
        F0326_CHAMPION_ShootProjectile(L0992_ps_Champion, P0680_T_Thing, P0681_i_KineticEnergy, 90, L0991_i_StepEnergy);
        /* BUG0_01 Coding error without consequence. Undefined return value. A 'return C1_TRUE;' statement is missing. No consequence because the actual value returned cannot be 0: it is the value of register D0 at the end of execution of F0326_CHAMPION_ShootProjectile, which is the return value of F0212_PROJECTILE_Create, which in turn is the return value of F0238_TIMELINE_AddEvent_GetEventIndex_CPSE, which is the index of the event added for the projectile. This index cannot be 0 because event 0 is always event C53_EVENT_WATCHDOG */
        return C1_TRUE;
}

BOOLEAN F0328_CHAMPION_IsObjectThrown(
REGISTER unsigned int16_t P0683_ui_ChampionIndex SEPARATOR
int16_t                   P0684_i_SlotIndex      SEPARATOR
int16_t                   P0685_i_Side           FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0997_ps_Champion;
        REGISTER WEAPON_INFO* L0998_ps_WeaponInfo;
        REGISTER int16_t L0995_i_Multiple;
#define AL0995_i_WeaponKineticEnergy L0995_i_Multiple
#define AL0995_i_SkillLevel          L0995_i_Multiple
#define AL0995_i_StepEnergy          L0995_i_Multiple
        REGISTER THING L0996_T_Thing;
        REGISTER int16_t L0994_i_Multiple;
#define AL0994_i_Experience L0994_i_Multiple
#define AL0994_i_Attack     L0994_i_Multiple
        REGISTER int16_t L0993_i_KineticEnergy;
        THING L0999_T_ActionHandThing;
        BOOLEAN L1000_B_ThrowingLeaderHandObject;


        L1000_B_ThrowingLeaderHandObject = C0_FALSE;
        if (P0684_i_SlotIndex < 0) { /* Throw object in leader hand, which is temporarily placed in action hand */
                if (G0415_ui_LeaderEmptyHanded) {
                        return C0_FALSE;
                }
                L0996_T_Thing = F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
                L0997_ps_Champion = &M516_CHAMPIONS[P0683_ui_ChampionIndex];
                L0999_T_ActionHandThing = L0997_ps_Champion->Slots[C01_SLOT_ACTION_HAND];
                L0997_ps_Champion->Slots[C01_SLOT_ACTION_HAND] = L0996_T_Thing;
                P0684_i_SlotIndex = C01_SLOT_ACTION_HAND;
                L1000_B_ThrowingLeaderHandObject = C1_TRUE;
        }
        L0993_i_KineticEnergy = F0312_CHAMPION_GetStrength(P0683_ui_ChampionIndex, P0684_i_SlotIndex);
        if (L1000_B_ThrowingLeaderHandObject) {
                L0997_ps_Champion->Slots[P0684_i_SlotIndex] = L0999_T_ActionHandThing;
        } else {
                if ((L0996_T_Thing = F0300_CHAMPION_GetObjectRemovedFromSlot(P0683_ui_ChampionIndex, P0684_i_SlotIndex)) == C0xFFFF_THING_NONE) {
                        return C0_FALSE;
                }
        }
        F0064_SOUND_RequestPlay_CPSD(M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, G0306_i_PartyMapX, G0307_i_PartyMapY, C01_MODE_PLAY_IF_PRIORITIZED);
        F0325_CHAMPION_DecrementStamina(P0683_ui_ChampionIndex, F0305_CHAMPION_GetThrowingStaminaCost(L0996_T_Thing));
        F0330_CHAMPION_DisableAction(P0683_ui_ChampionIndex, 4);
        AL0994_i_Experience = 8;
        AL0995_i_WeaponKineticEnergy = 1;
        if (M012_TYPE(L0996_T_Thing) == C05_THING_TYPE_WEAPON) {
                AL0994_i_Experience += 4;
                L0998_ps_WeaponInfo = F0158_DUNGEON_GetWeaponInfo(L0996_T_Thing);
                if (L0998_ps_WeaponInfo->Class <= C012_CLASS_POISON_DART) {
                        AL0994_i_Experience += (AL0995_i_WeaponKineticEnergy = L0998_ps_WeaponInfo->KineticEnergy) >> 2;
                }
        }
        F0304_CHAMPION_AddSkillExperience(P0683_ui_ChampionIndex, C10_SKILL_THROW, AL0994_i_Experience);
        L0993_i_KineticEnergy += AL0995_i_WeaponKineticEnergy;
        AL0995_i_SkillLevel = F0303_CHAMPION_GetSkillLevel(P0683_ui_ChampionIndex, C10_SKILL_THROW);
        L0993_i_KineticEnergy += M003_RANDOM(16) + (L0993_i_KineticEnergy >> 1) + AL0995_i_SkillLevel;
        AL0994_i_Attack = F0026_MAIN_GetBoundedValue(40, (AL0995_i_SkillLevel << 3) + M003_RANDOM(32), 200);
        AL0995_i_StepEnergy = F0025_MAIN_GetMaximumValue(5, 11 - AL0995_i_SkillLevel);
        F0212_PROJECTILE_Create(L0996_T_Thing, G0306_i_PartyMapX, G0307_i_PartyMapY, M021_NORMALIZE(G0308_i_PartyDirection + P0685_i_Side), G0308_i_PartyDirection, L0993_i_KineticEnergy, AL0994_i_Attack, AL0995_i_StepEnergy);
        G0311_i_ProjectileDisabledMovementTicks = 4;
        G0312_i_LastProjectileDisabledMovementDirection = G0308_i_PartyDirection;
        F0292_CHAMPION_DrawState(P0683_ui_ChampionIndex);
        return C1_TRUE;
}

BOOLEAN F0329_CHAMPION_IsLeaderHandObjectThrown(
int16_t P0686_i_Side FINAL_SEPARATOR
{
        if (G0411_i_LeaderIndex == CM1_CHAMPION_NONE) {
                return C0_FALSE;
        }
        return F0328_CHAMPION_IsObjectThrown(G0411_i_LeaderIndex, CM1_SLOT_LEADER_HAND, P0686_i_Side);
}

void F0330_CHAMPION_DisableAction(
REGISTER unsigned int16_t P0687_ui_ChampionIndex SEPARATOR
unsigned int16_t          P0688_ui_Ticks         FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1004_ps_Champion;
        REGISTER long L1001_l_UpdatedEnableActionEventTime;
        REGISTER long L1002_l_CurrentEnableActionEventTime;
        REGISTER int16_t L1003_i_EventIndex;
        EVENT L1005_s_Event;


        L1004_ps_Champion = &M516_CHAMPIONS[P0687_ui_ChampionIndex];
        L1001_l_UpdatedEnableActionEventTime = G0313_ul_GameTime + P0688_ui_Ticks;
        L1005_s_Event.A.A.Type = C11_EVENT_ENABLE_CHAMPION_ACTION;
        L1005_s_Event.A.A.Priority = P0687_ui_ChampionIndex;
        L1005_s_Event.B.SlotOrdinal = 0;
        if ((L1003_i_EventIndex = L1004_ps_Champion->EnableActionEventIndex) >= 0) {
                L1002_l_CurrentEnableActionEventTime = M030_TIME(G0370_ps_Events[L1003_i_EventIndex].Map_Time);
                if (L1001_l_UpdatedEnableActionEventTime >= L1002_l_CurrentEnableActionEventTime) {
                        L1001_l_UpdatedEnableActionEventTime += (L1002_l_CurrentEnableActionEventTime - G0313_ul_GameTime) >> 1;
                } else {
                        L1001_l_UpdatedEnableActionEventTime = L1002_l_CurrentEnableActionEventTime + (P0688_ui_Ticks >> 1);
                }
                F0237_TIMELINE_DeleteEvent(L1003_i_EventIndex);
        } else {
                M008_SET(L1004_ps_Champion->Attributes, MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION);
                F0292_CHAMPION_DrawState(P0687_ui_ChampionIndex);
        }
        M033_SET_MAP_AND_TIME(L1005_s_Event.Map_Time, G0309_i_PartyMapIndex, L1001_l_UpdatedEnableActionEventTime);
        L1004_ps_Champion->EnableActionEventIndex = F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1005_s_Event);
}

void F0331_CHAMPION_ApplyTimeEffects_CPSF(
void
)
{
        REGISTER CHAMPION* L1010_ps_Champion;
        REGISTER unsigned char* L1011_puc_Statistic;
#define AL1007_ScentIndex         L1007_Multiple
#define AL1007_ManaGain           L1007_Multiple
#define AL1007_StatisticIndex     L1007_Multiple
#define AL9995_ui_StaminaGainCycleCount L1006_ui_Multiple
        REGISTER int16_t L1007_Multiple;
#define AL9996_i_SkillIndex       L1007_Multiple
#define AL9994_i_StaminaMagnitude L1007_Multiple
#define AL9993_i_StaminaLoss      L1007_Multiple
        REGISTER unsigned int16_t L1008_ui_Multiple;
#define AL1008_ui_WizardPriestSkillLevel L1008_ui_Multiple
#define AL1008_ui_Delay                  L1008_ui_Multiple
#define AL1008_ui_StaminaAboveHalf       L1008_ui_Multiple
#define AL1008_ui_StatisticMaximum       L1008_ui_Multiple
#define AL1013_i_StaminaAmount L1013_i_Multiple
#define AL1013_i_HealthGain    L1013_i_Multiple
#define AL9998_ui_GameTime L1008_ui_Multiple
        REGISTER unsigned int16_t L1006_ui_Multiple;
        REGISTER unsigned int16_t L1009_i_Multiple;
#define AL9997_ui_ChampionIndex L1009_i_Multiple
        REGISTER int16_t L1013_i_Multiple;
        unsigned int16_t L1012_ui_TimeCriteria;
        SCENT L1014_s_Scent;


        if (!G0305_ui_PartyChampionCount) {
                return;
        }
        L1014_s_Scent.Location.MapX = G0306_i_PartyMapX;
        L1014_s_Scent.Location.MapY = G0307_i_PartyMapY;
        L1014_s_Scent.Location.MapIndex = G0309_i_PartyMapIndex;
        AL1007_ScentIndex = 0;
        while ((int16_t)AL1007_ScentIndex < (int16_t)G0407_s_Party.ScentCount - 1) {
                if (G0407_s_Party.Scents[AL1007_ScentIndex].Scent != L1014_s_Scent.Scent) {
                        if (!(G0407_s_Party.ScentStrengths[AL1007_ScentIndex] = F0025_MAIN_GetMaximumValue(0, G0407_s_Party.ScentStrengths[AL1007_ScentIndex] - 1)) && !AL1007_ScentIndex) {
                                F0316_CHAMPION_DeleteScent(0);
                                continue;
                        }
                }
                AL1007_ScentIndex++;
        }
        AL9998_ui_GameTime = G0313_ul_GameTime;
        L1012_ui_TimeCriteria = (((AL9998_ui_GameTime & 0x0080) + ((AL9998_ui_GameTime & 0x0100) >> 2)) + ((AL9998_ui_GameTime & 0x0040) << 2)) >> 2;
        for (AL9997_ui_ChampionIndex = C00_CHAMPION_FIRST, L1010_ps_Champion = M516_CHAMPIONS; AL9997_ui_ChampionIndex < G0305_ui_PartyChampionCount; AL9997_ui_ChampionIndex++, L1010_ps_Champion++) {
                if (L1010_ps_Champion->CurrentHealth && (M000_INDEX_TO_ORDINAL(AL9997_ui_ChampionIndex) != G0299_ui_CandidateChampionOrdinal)) {
                        if ((L1010_ps_Champion->CurrentMana < L1010_ps_Champion->MaximumMana) && (L1012_ui_TimeCriteria < (L1010_ps_Champion->Statistics[C3_STATISTIC_WISDOM][C1_CURRENT] + (AL1008_ui_WizardPriestSkillLevel = F0303_CHAMPION_GetSkillLevel(AL9997_ui_ChampionIndex, C03_SKILL_WIZARD) + F0303_CHAMPION_GetSkillLevel(AL9997_ui_ChampionIndex, C02_SKILL_PRIEST))))) {
                                AL1007_ManaGain = L1010_ps_Champion->MaximumMana / 40;
                                if (G0300_B_PartyIsResting) {
                                        AL1007_ManaGain <<= 1;
                                }
                                AL1007_ManaGain++;
                                F0325_CHAMPION_DecrementStamina(AL9997_ui_ChampionIndex, AL1007_ManaGain * F0025_MAIN_GetMaximumValue(7, 16 - AL1008_ui_WizardPriestSkillLevel));
                                L1010_ps_Champion->CurrentMana += F0024_MAIN_GetMinimumValue(AL1007_ManaGain, L1010_ps_Champion->MaximumMana - L1010_ps_Champion->CurrentMana);
                        } else {
                                if (L1010_ps_Champion->CurrentMana > L1010_ps_Champion->MaximumMana) {
                                        L1010_ps_Champion->CurrentMana--;
                                }
                        }
                        for (AL9996_i_SkillIndex = C19_SKILL_WATER; AL9996_i_SkillIndex >= C00_SKILL_FIGHTER; AL9996_i_SkillIndex--) {
                                if (L1010_ps_Champion->Skills[AL9996_i_SkillIndex].TemporaryExperience > 0) {
                                        L1010_ps_Champion->Skills[AL9996_i_SkillIndex].TemporaryExperience--;
                                }
                        }
                        AL9995_ui_StaminaGainCycleCount = 4;
                        AL9994_i_StaminaMagnitude = L1010_ps_Champion->MaximumStamina;
                        while (L1010_ps_Champion->CurrentStamina < (AL9994_i_StaminaMagnitude >>= 1)) {
                                AL9995_ui_StaminaGainCycleCount += 2;
                        }
                        AL9993_i_StaminaLoss = 0;
                        AL1013_i_StaminaAmount = F0026_MAIN_GetBoundedValue(1, (L1010_ps_Champion->MaximumStamina >> 8) - 1, 6);
                        if (G0300_B_PartyIsResting) {
                                AL1013_i_StaminaAmount <<= 1;
                        }
                        if ((AL1008_ui_Delay = (G0313_ul_GameTime - G0362_l_LastPartyMovementTime)) > 80) {
                                AL1013_i_StaminaAmount++;
                                if (AL1008_ui_Delay > 250) {
                                        AL1013_i_StaminaAmount++;
                                }
                        }
                        do {
                                AL1008_ui_StaminaAboveHalf = (AL9995_ui_StaminaGainCycleCount <= 4);
                                if (L1010_ps_Champion->Food < -512) {
                                        if (AL1008_ui_StaminaAboveHalf) {
                                               AL9993_i_StaminaLoss += AL1013_i_StaminaAmount;
                                               L1010_ps_Champion->Food -= 2;
                                        }
                                } else {
                                        if (L1010_ps_Champion->Food >= 0) {
                                                AL9993_i_StaminaLoss -= AL1013_i_StaminaAmount;
                                        }
                                        L1010_ps_Champion->Food -= AL1008_ui_StaminaAboveHalf ? 2 : AL9995_ui_StaminaGainCycleCount >> 1;
                                }
                                if (L1010_ps_Champion->Water < -512) {
                                        if (AL1008_ui_StaminaAboveHalf) {
                                               AL9993_i_StaminaLoss += AL1013_i_StaminaAmount;
                                               L1010_ps_Champion->Water--;
                                        }
                                } else {
                                        if (L1010_ps_Champion->Water >= 0) {
                                                AL9993_i_StaminaLoss -= AL1013_i_StaminaAmount;
                                        }
                                        L1010_ps_Champion->Water -= AL1008_ui_StaminaAboveHalf ? 1 : AL9995_ui_StaminaGainCycleCount >> 2;
                                }
                        } while (--AL9995_ui_StaminaGainCycleCount && ((L1010_ps_Champion->CurrentStamina - AL9993_i_StaminaLoss) < L1010_ps_Champion->MaximumStamina));
                        F0325_CHAMPION_DecrementStamina(AL9997_ui_ChampionIndex, AL9993_i_StaminaLoss);
                        if (L1010_ps_Champion->Food < -1024) {
                                L1010_ps_Champion->Food = -1024;
                        }
                        if (L1010_ps_Champion->Water < -1024) {
                                L1010_ps_Champion->Water = -1024;
                        }
                        if ((L1010_ps_Champion->CurrentHealth < L1010_ps_Champion->MaximumHealth) && (L1010_ps_Champion->CurrentStamina >= (L1010_ps_Champion->MaximumStamina >> 2)) && (L1012_ui_TimeCriteria < (L1010_ps_Champion->Statistics[C4_STATISTIC_VITALITY][C1_CURRENT] + 12))) {
                                AL1013_i_HealthGain = (L1010_ps_Champion->MaximumHealth >> 7) + 1;
                                if (G0300_B_PartyIsResting) {
                                        AL1013_i_HealthGain <<= 1;
                                }
                                if (F0033_OBJECT_GetIconIndex(L1010_ps_Champion->Slots[C10_SLOT_NECK]) == C121_ICON_JUNK_EKKHARD_CROSS) {
                                        AL1013_i_HealthGain += (AL1013_i_HealthGain >> 1) + 1;
                                }
                                L1010_ps_Champion->CurrentHealth += F0024_MAIN_GetMinimumValue(AL1013_i_HealthGain, L1010_ps_Champion->MaximumHealth - L1010_ps_Champion->CurrentHealth);
                        }
                        if (!((unsigned int16_t)G0313_ul_GameTime & (G0300_B_PartyIsResting ? 63 : 255))) {
                                for (AL1007_StatisticIndex = C0_STATISTIC_LUCK; AL1007_StatisticIndex <= C6_STATISTIC_ANTIFIRE; AL1007_StatisticIndex++) {
                                        L1011_puc_Statistic = L1010_ps_Champion->Statistics[AL1007_StatisticIndex];
                                        AL1008_ui_StatisticMaximum = L1011_puc_Statistic[C0_MAXIMUM];
                                        if (L1011_puc_Statistic[C1_CURRENT] < AL1008_ui_StatisticMaximum) {
                                                L1011_puc_Statistic[C1_CURRENT]++;
                                        } else {
                                                if (L1011_puc_Statistic[C1_CURRENT] > AL1008_ui_StatisticMaximum) {
                                                        L1011_puc_Statistic[C1_CURRENT] -= L1011_puc_Statistic[C1_CURRENT] / AL1008_ui_StatisticMaximum;
                                                }
                                        }
                                }
                        }
                        if (!G0300_B_PartyIsResting && (L1010_ps_Champion->Direction != G0308_i_PartyDirection) && (G0361_l_LastCreatureAttackTime < (G0313_ul_GameTime - 60))) {
                                L1010_ps_Champion->Direction = G0308_i_PartyDirection;
                                L1010_ps_Champion->MaximumDamageReceived = 0;
                                M008_SET(L1010_ps_Champion->Attributes, MASK0x0400_ICON);
                        }
                        M008_SET(L1010_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                        if (M000_INDEX_TO_ORDINAL(AL9997_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal) {
                                if ((G0424_i_PanelContent == M565_PANEL_FOOD_WATER_POISONED) || (G0424_i_PanelContent == C02_PANEL_SKILLS_AND_STATISTICS)) {
                                        M008_SET(L1010_ps_Champion->Attributes, MASK0x0800_PANEL);
                                }
                        }
                }
        }
        F0293_CHAMPION_DrawAllChampionStates(MASK0x0000_NONE);
}
/* END CHAMPION.C */

/* BEGIN OBJECT.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
unsigned char** G0352_apc_ObjectNames;
int16_t G2007_ai_XYZ[4];


void F0031_OBJECT_LoadNames(
void
)
{
        REGISTER unsigned char* L0001_puc_ObjectNames;
        REGISTER unsigned int16_t L0003_ui_ObjectNameIndex;
#ifdef X557_I34E
        REGISTER unsigned char* L0002_puc_Graphic;
#endif


        F0625_GetZoneInitializedFromDimensions(G2007_ai_XYZ, G2075_ObjectIconWidth, G2076_ObjectIconHeight);
        G4055_s_LeaderHandObject.BitmapIconForMousePointer = F0606_AllocateMemForGraphic(G2075_ObjectIconWidth, G2076_ObjectIconHeight, C1_ALLOCATION_PERMANENT);
        G0352_apc_ObjectNames = (unsigned char**)M533_F0468_MEMORY_Allocate((long)(C199_OBJECT_NAME_COUNT * sizeof(char*)), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
#ifdef X060_I34E
        L0001_puc_ObjectNames = (unsigned char*)M533_F0468_MEMORY_Allocate((long)(F0494_MEMORY_GetGraphicDecompressedByteCount(M564_GRAPHIC_OBJECT_NAMES) + C199_OBJECT_NAME_COUNT), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
        L0002_puc_Graphic = M533_F0468_MEMORY_Allocate((long)(C199_OBJECT_NAME_COUNT * C014_OBJECT_NAME_MAXIMUM_LENGTH), C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP, MASK0x0400_MEMREQ);
#endif
#ifdef X735_I34M
        L0001_puc_ObjectNames = (unsigned char*)M533_F0468_MEMORY_Allocate((long)F0494_MEMORY_GetGraphicDecompressedByteCount(M564_GRAPHIC_OBJECT_NAMES), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
#endif
#ifdef X702_I34E
        F0490_MEMORY_LoadDecompressAndExpandGraphic(MASK0x8000_NOT_EXPANDED | MASK0x4000_DO_NOT_COPY_DIMENSIONS | M564_GRAPHIC_OBJECT_NAMES, L0002_puc_Graphic);
#endif
#ifdef X735_I34M
        F0490_MEMORY_LoadDecompressAndExpandGraphic(MASK0x8000_NOT_EXPANDED | MASK0x4000_DO_NOT_COPY_DIMENSIONS | M564_GRAPHIC_OBJECT_NAMES, L0001_puc_ObjectNames);
#endif
#ifdef X060_I34E
        for (L0003_ui_ObjectNameIndex = 0; L0003_ui_ObjectNameIndex < C199_OBJECT_NAME_COUNT; L0003_ui_ObjectNameIndex++) {
                G0352_apc_ObjectNames[L0003_ui_ObjectNameIndex] = L0001_puc_ObjectNames;
                while (!(*L0002_puc_Graphic & 0x80)) { /* The last character of each object name has bit 7 set */
                        *L0001_puc_ObjectNames++ = *L0002_puc_Graphic++; /* Write characters until last character is found */
                }
                *L0001_puc_ObjectNames++ = *L0002_puc_Graphic++ & 0x7F; /* Write last character without bit 7 */
                *L0001_puc_ObjectNames++ = '\0'; /* Write string termination */
        }
        F0469_MEMORY_FreeAtHeapTop((long)(C199_OBJECT_NAME_COUNT * C014_OBJECT_NAME_MAXIMUM_LENGTH));
#endif
#ifdef X574_I34M
        for (L0003_ui_ObjectNameIndex = 0; L0003_ui_ObjectNameIndex < C199_OBJECT_NAME_COUNT; L0003_ui_ObjectNameIndex++) {
                G0352_apc_ObjectNames[L0003_ui_ObjectNameIndex] = L0001_puc_ObjectNames;
                while (*L0001_puc_ObjectNames++);
        }
#endif
}

int16_t F0032_OBJECT_GetType(
THING P0039_T_Thing FINAL_SEPARATOR
{
        REGISTER int16_t L0004_i_Multiple;
#define AL0004_i_ObjectInfoIndex L0004_i_Multiple
#define AL0004_i_ObjectType      L0004_i_Multiple


        if (P0039_T_Thing == C0xFFFF_THING_NONE) {
                return C0xFFFF_ICON_NONE;
        }
        AL0004_i_ObjectInfoIndex = F0141_DUNGEON_GetObjectInfoIndex(P0039_T_Thing);
        if (AL0004_i_ObjectInfoIndex != -1) {
                AL0004_i_ObjectType = G0237_as_Graphic559_ObjectInfo[AL0004_i_ObjectInfoIndex].Type;
        }
        return AL0004_i_ObjectType;
}

int16_t F0033_OBJECT_GetIconIndex(
THING P0040_T_Thing FINAL_SEPARATOR
{
        REGISTER JUNK* L0006_ps_Junk;
        REGISTER int16_t L0005_i_IconIndex;


        if ((L0005_i_IconIndex = F0032_OBJECT_GetType(P0040_T_Thing)) != -1) {
                if (((L0005_i_IconIndex < C032_ICON_WEAPON_DAGGER) && (L0005_i_IconIndex >= C000_ICON_JUNK_COMPASS_NORTH)) ||
                    ((L0005_i_IconIndex >= C148_ICON_POTION_MA_POTION_MON_POTION) && (L0005_i_IconIndex <= C163_ICON_POTION_WATER_FLASK)) ||
                    (L0005_i_IconIndex == C195_ICON_POTION_EMPTY_FLASK)) {
                        L0006_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(P0040_T_Thing);
                        switch (L0005_i_IconIndex) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                case C000_ICON_JUNK_COMPASS_NORTH:
                                        L0005_i_IconIndex += G0308_i_PartyDirection;
                                        break;
                                case C004_ICON_WEAPON_TORCH_UNLIT:
                                        if (((WEAPON*)L0006_ps_Junk)->Lit) {
                                                L0005_i_IconIndex += G0029_auc_Graphic562_ChargeCountToTorchType[((WEAPON*)L0006_ps_Junk)->ChargeCount];
                                        }
                                        break;
                                case C030_ICON_SCROLL_SCROLL_OPEN:
#ifdef PC_FIX_CODE_SIZE
        L0005_i_IconIndex++;
#endif
                                        if (((SCROLL*)L0006_ps_Junk)->Closed) {
                                                L0005_i_IconIndex++; /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE opt */
                                        }
                                        break;
                                case C008_ICON_JUNK_WATER:
                                case C012_ICON_JUNK_ILLUMULET_UNEQUIPPED:
                                case C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED:
#ifdef PC_FIX_CODE_SIZE
        L0005_i_IconIndex++;
#endif
                                        if (L0006_ps_Junk->ChargeCount) {
                                                L0005_i_IconIndex++; /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE opt */
                                        }
                                        break;
                                case C023_ICON_WEAPON_BOLT_BLADE_STORM_EMPTY:
                                case C014_ICON_WEAPON_FLAMITT_EMPTY:
                                case C018_ICON_WEAPON_STORMRING_EMPTY:
                                case C025_ICON_WEAPON_FURY_RA_BLADE_EMPTY:
                                case C016_ICON_WEAPON_EYE_OF_TIME_EMPTY:
                                case C020_ICON_WEAPON_STAFF_OF_CLAWS_EMPTY:
                                        if (((WEAPON*)L0006_ps_Junk)->ChargeCount) {
                                                L0005_i_IconIndex++;
                                        }
                                        break;
                        }
                }
        }
        return L0005_i_IconIndex;
}

void F0034_OBJECT_DrawLeaderHandObjectName(
REGISTER THING P0041_T_Thing FINAL_SEPARATOR
{
        REGISTER char* L0008_pc_ObjectName;
        REGISTER JUNK* L0009_ps_Junk;
        REGISTER int16_t L0007_i_IconIndex;
        char L0010_ac_ObjectName[16];


        F0035_OBJECT_ClearLeaderHandObjectName();
        L0007_i_IconIndex = F0033_OBJECT_GetIconIndex(P0041_T_Thing);
        if (L0007_i_IconIndex == C147_ICON_JUNK_CHAMPION_BONES) {
                L0009_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(P0041_T_Thing);
#ifdef X736_I34M
                if (G2000_Language == C1_FRENCH) {
#endif
#ifdef X262_I34M /* CHANGE5_00_LOCALIZATION Translation to French language */
                        M547_STRCPY(L0010_ac_ObjectName, G0352_apc_ObjectNames[L0007_i_IconIndex]); /* No space is added between the strings as the space is included directly in the object name */
                        M545_STRCAT(L0010_ac_ObjectName, M516_CHAMPIONS[L0009_ps_Junk->ChargeCount].Name);
#endif
#ifdef X736_I34M
                } else {
#endif
                        M547_STRCPY(L0010_ac_ObjectName, M516_CHAMPIONS[L0009_ps_Junk->ChargeCount].Name);
/* BUG7_00 When holding a champion's dead bones in hand, a space character is missing between the champion name and the object name */
                        M545_STRCAT(L0010_ac_ObjectName, M772_CAST_PC(G0352_apc_ObjectNames[L0007_i_IconIndex]));
#ifdef X736_I34M
                }
#endif
                L0008_pc_ObjectName = L0010_ac_ObjectName;
        } else {
                L0008_pc_ObjectName = (char*)G0352_apc_ObjectNames[L0007_i_IconIndex];
        }
        M722_F0768_TEXT_PrintToZoneWithTrailingSpaces(G0348_Bitmap_Screen, G2071_C320_ScreenPixelWidth, C017_ZONE_LEADER_HAND_OBJECT_NAME, C04_COLOR_CYAN, C00_COLOR_BLACK, L0008_pc_ObjectName, C014_OBJECT_NAME_MAXIMUM_LENGTH, G2072_C200_ScreenPixelHeight);
}

void F0035_OBJECT_ClearLeaderHandObjectName(
void
)
{
        F0733_FillZoneByIndex(C017_ZONE_LEADER_HAND_OBJECT_NAME, C00_COLOR_BLACK);
}

void F0036_OBJECT_ExtractIconFromBitmap(
REGISTER unsigned int16_t P0042_ui_IconIndex SEPARATOR
REGISTER long*            P0043_pl_Bitmap    FINAL_SEPARATOR
{
        long* L3358_pl_;
        REGISTER unsigned int16_t L0011_ui_Counter;
        REGISTER long* L0012_pl_Bitmap_Icon;


        L3358_pl_ = P0043_pl_Bitmap;
        M100_PIXEL_WIDTH(L3358_pl_) = G2075_ObjectIconWidth;
        M101_PIXEL_HEIGHT(L3358_pl_) = G2076_ObjectIconHeight;
        for (L0011_ui_Counter = 0; L0011_ui_Counter < 7; L0011_ui_Counter++) {
                if (G0026_ai_Graphic562_IconGraphicFirstIconIndex[L0011_ui_Counter] > P0042_ui_IconIndex)
                        break;
        }
        L0012_pl_Bitmap_Icon = (long*)F0489_MEMORY_GetNativeBitmapOrGraphic(C042_GRAPHIC_OBJECT_ICONS_000_TO_031 + --L0011_ui_Counter);
        P0042_ui_IconIndex -= G0026_ai_Graphic562_IconGraphicFirstIconIndex[L0011_ui_Counter];
        F0654_Call_F0132_VIDEO_Blit(L0012_pl_Bitmap_Icon, L3358_pl_, G2007_ai_XYZ, (P0042_ui_IconIndex & 0x000F) * G2076_ObjectIconHeight, (P0042_ui_IconIndex >> 4) * G2075_ObjectIconWidth, CM1_COLOR_NO_TRANSPARENCY);
}


void F0618_LoadZone(
int16_t  P2053_i_ZoneIndex SEPARATOR
int16_t* P2054_pi_XYZ      FINAL_SEPARATOR
{
        int16_t L3359_i_X;
        int16_t L3360_i_Y;


        L3359_i_X = G2075_ObjectIconWidth;
        L3360_i_Y = G2076_ObjectIconHeight;
        F0635_(NULL, P2054_pi_XYZ, P2053_i_ZoneIndex, &L3359_i_X, &L3360_i_Y);
}

void F0619_GetSlotBoxBorderCoordinates(
int16_t           P2055_i_ZoneIndex SEPARATOR
REGISTER int16_t* P2056_pi_XYZ      FINAL_SEPARATOR
{
        F0618_LoadZone(P2055_i_ZoneIndex, P2056_pi_XYZ);
        F0628_AddZoneMargin(P2056_pi_XYZ, G2077_C1_TextMargin, G2077_C1_TextMargin);
}

void F0038_OBJECT_DrawIconInSlotBox(
REGISTER unsigned int16_t P0047_ui_SlotBoxIndex SEPARATOR
REGISTER int16_t          P0048_IconIndex    FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0015_ui_IconGraphicIndex;
        REGISTER unsigned char* L0018_puc_Bitmap_Icons;
        REGISTER SLOT_BOX* L0017_ps_SlotBox;
        REGISTER unsigned char* L0020_puc_Bitmap_Destination;
        REGISTER int16_t L0016_i_ByteWidth;
        int16_t L3361_ai_XYZ[4];


        L0017_ps_SlotBox = &G0030_as_Graphic562_SlotBoxes[P0047_ui_SlotBoxIndex];
        L0017_ps_SlotBox->IconIndex = P0048_IconIndex;
        if (P0048_IconIndex == C0xFFFF_ICON_NONE)
                return;
        F0618_LoadZone(L0017_ps_SlotBox->ZoneIndex, L3361_ai_XYZ);
        for (L0015_ui_IconGraphicIndex = 0; L0015_ui_IconGraphicIndex < 7; L0015_ui_IconGraphicIndex++) {
                if (G0026_ai_Graphic562_IconGraphicFirstIconIndex[L0015_ui_IconGraphicIndex] > P0048_IconIndex)
                        break;
        }
        L0015_ui_IconGraphicIndex--;
        L0018_puc_Bitmap_Icons = F0489_MEMORY_GetNativeBitmapOrGraphic(L0015_ui_IconGraphicIndex + C042_GRAPHIC_OBJECT_ICONS_000_TO_031);
        P0048_IconIndex -= G0026_ai_Graphic562_IconGraphicFirstIconIndex[L0015_ui_IconGraphicIndex];
        if (P0047_ui_SlotBoxIndex >= C08_SLOT_BOX_INVENTORY_FIRST_SLOT) {
                L0020_puc_Bitmap_Destination = G0296_puc_Bitmap_Viewport;
                L0016_i_ByteWidth = G2073_C224_ViewportPixelWidth;
        } else {
                L0020_puc_Bitmap_Destination = G0348_Bitmap_Screen;
                L0016_i_ByteWidth = G2071_C320_ScreenPixelWidth;
        }
        F0132_VIDEO_Blit(L0018_puc_Bitmap_Icons, L0020_puc_Bitmap_Destination, L3361_ai_XYZ, (P0048_IconIndex & 0x000F) * G2075_ObjectIconWidth, ((P0048_IconIndex & 0x0FF0) >> 4) * G2076_ObjectIconHeight, M100_PIXEL_WIDTH(L0018_puc_Bitmap_Icons), L0016_i_ByteWidth, CM1_COLOR_NO_TRANSPARENCY, MASK0x0000_NO_FLIP);
}

int16_t F0039_OBJECT_GetIconIndexInSlotBox(
unsigned int16_t P0049_ui_SlotBoxIndex FINAL_SEPARATOR
{
        return G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex].IconIndex;
}
/* END OBJECT.C */

/* BEGIN COMMAND.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#if (EXETYPE == C03_GAME)
COMMAND G0432_as_CommandQueue[M529_COMMAND_QUEUE_SIZE + 1]; /* Circular queue. Can only contain up to 4 actual commands at a time */
int16_t G2153_i_QueuedCommandsCount;
int16_t G0433_i_CommandQueueFirstIndex; /* The queue contains a single command if G0434_i_CommandQueueLastIndex = G0433_i_CommandQueueFirstIndex */
int16_t G0434_i_CommandQueueLastIndex = M529_COMMAND_QUEUE_SIZE; /* The queue is empty if (G0434_i_CommandQueueLastIndex + 1) % 4 = G0433_i_CommandQueueFirstIndex */
BOOLEAN G0435_B_CommandQueueLocked = C1_TRUE;
BOOLEAN G0436_B_PendingClickPresent;
int16_t G0437_i_PendingClickX;
int16_t G0438_i_PendingClickY;
int16_t G0439_i_PendingClickButtonsStatus;
MOUSE_INPUT* G0441_ps_PrimaryMouseInput;
MOUSE_INPUT* G0442_ps_SecondaryMouseInput;
KEYBOARD_INPUT* G0443_ps_PrimaryKeyboardInput;
KEYBOARD_INPUT* G0444_ps_SecondaryKeyboardInput;
MOUSE_INPUT G0445_as_Graphic561_PrimaryMouseInput_Entrance[6] = {
        { C200_COMMAND_ENTRANCE_ENTER_DUNGEON,       CM1_SCREEN_RELATIVE, C407_ZONE_ENTRANCE_ENTER,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON   },
        { C201_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON, CM1_SCREEN_RELATIVE, C407_ZONE_ENTRANCE_ENTER,   0, 0, MASK0x0010_MOUSE_BONUS_DUNGEON },
        { M566_COMMAND_ENTRANCE_RESUME,              CM1_SCREEN_RELATIVE, C409_ZONE_ENTRANCE_RESUME,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON   },
        { C216_COMMAND_QUIT,                         CM1_SCREEN_RELATIVE, C434_ZONE_ENTRANCE_QUIT,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON   },
        { M567_COMMAND_ENTRANCE_DRAW_CREDITS,        CM1_SCREEN_RELATIVE, C411_ZONE_ENTRANCE_CREDITS, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON   },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0446_as_Graphic561_PrimaryMouseInput_RestartGame[3] = {
        { C215_COMMAND_RESTART_GAME, 103, 217, 140, 154, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C216_COMMAND_QUIT,         142, 178, 165, 179, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0447_as_Graphic561_PrimaryMouseInput_Interface[20] = {
        { C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0,         CM1_SCREEN_RELATIVE, C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1,         CM1_SCREEN_RELATIVE, C152_ZONE_CHAMPION_1_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2,         CM1_SCREEN_RELATIVE, C153_ZONE_CHAMPION_2_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3,         CM1_SCREEN_RELATIVE, C154_ZONE_CHAMPION_3_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0,         CM1_SCREEN_RELATIVE, C187_ZONE_CHAMPION_0_STATUS_BOX_BAR_GRAPHS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1,         CM1_SCREEN_RELATIVE, C188_ZONE_CHAMPION_1_STATUS_BOX_BAR_GRAPHS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2,         CM1_SCREEN_RELATIVE, C189_ZONE_CHAMPION_2_STATUS_BOX_BAR_GRAPHS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3,         CM1_SCREEN_RELATIVE, C190_ZONE_CHAMPION_3_STATUS_BOX_BAR_GRAPHS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX,      CM1_SCREEN_RELATIVE, C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C013_COMMAND_CLICK_IN_CHAMPION_1_STATUS_BOX,      CM1_SCREEN_RELATIVE, C152_ZONE_CHAMPION_1_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C014_COMMAND_CLICK_IN_CHAMPION_2_STATUS_BOX,      CM1_SCREEN_RELATIVE, C153_ZONE_CHAMPION_2_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C015_COMMAND_CLICK_IN_CHAMPION_3_STATUS_BOX,      CM1_SCREEN_RELATIVE, C154_ZONE_CHAMPION_3_STATUS_BOX_NAME_HANDS,   0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT,     CM1_SCREEN_RELATIVE, C113_ZONE_CHAMPION_ICON_TOP_LEFT,             0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT,    CM1_SCREEN_RELATIVE, C114_ZONE_CHAMPION_ICON_TOP_RIGHT,            0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C127_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_RIGHT, CM1_SCREEN_RELATIVE, C115_ZONE_CHAMPION_ICON_BOTTOM_RIGHT,         0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C128_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_LEFT,  CM1_SCREEN_RELATIVE, C116_ZONE_CHAMPION_ICON_BOTTOM_LEFT,          0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C100_COMMAND_CLICK_IN_SPELL_AREA,                 CM1_SCREEN_RELATIVE, C013_ZONE_SPELL_AREA,                         0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C111_COMMAND_CLICK_IN_ACTION_AREA,                CM1_SCREEN_RELATIVE, C011_ZONE_ACTION_AREA,                        0,   0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C147_COMMAND_FREEZE_GAME,                                                                                    0, 1, 198, 199, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement[9] = {
        { C001_COMMAND_TURN_LEFT,               CM1_SCREEN_RELATIVE, C068_ZONE_TURN_LEFT,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C003_COMMAND_MOVE_FORWARD,            CM1_SCREEN_RELATIVE, C070_ZONE_MOVE_FORWARD,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C002_COMMAND_TURN_RIGHT,              CM1_SCREEN_RELATIVE, C069_ZONE_TURN_RIGHT,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C006_COMMAND_MOVE_LEFT,               CM1_SCREEN_RELATIVE, C073_ZONE_MOVE_LEFT,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C005_COMMAND_MOVE_BACKWARD,           CM1_SCREEN_RELATIVE, C072_ZONE_MOVE_BACKWARD, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C004_COMMAND_MOVE_RIGHT,              CM1_SCREEN_RELATIVE, C071_ZONE_MOVE_RIGHT,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT,      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C083_COMMAND_TOGGLE_INVENTORY_LEADER, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN,        0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory[39] = {
        { C011_COMMAND_CLOSE_INVENTORY,                                 CM1_SCREEN_RELATIVE,   C002_ZONE_SCREEN,                                 0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { C140_COMMAND_SAVE_GAME,                                       CM2_VIEWPORT_RELATIVE, C562_ZONE_SAVE_GAME_ICON,                         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C145_COMMAND_REST,                                            CM2_VIEWPORT_RELATIVE, C564_ZONE_REST_ICON,                              0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C011_COMMAND_CLOSE_INVENTORY,                                 CM2_VIEWPORT_RELATIVE, C566_ZONE_CLOSE_INVENTORY_ICON,                   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C141_COMMAND_TOGGLE_MUSIC,                                    CM2_VIEWPORT_RELATIVE, M701_ZONE_TOGGLE_MUSIC_ICON,                      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND,       CM2_VIEWPORT_RELATIVE, C507_ZONE_SLOT_BOX_08_INVENTORY_READY_HAND,       0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C029_COMMAND_CLICK_ON_SLOT_BOX_09_INVENTORY_ACTION_HAND,      CM2_VIEWPORT_RELATIVE, C508_ZONE_SLOT_BOX_09_INVENTORY_ACTION_HAND,      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C030_COMMAND_CLICK_ON_SLOT_BOX_10_INVENTORY_HEAD,             CM2_VIEWPORT_RELATIVE, C509_ZONE_SLOT_BOX_10_INVENTORY_HEAD,             0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C031_COMMAND_CLICK_ON_SLOT_BOX_11_INVENTORY_TORSO,            CM2_VIEWPORT_RELATIVE, C510_ZONE_SLOT_BOX_11_INVENTORY_TORSO,            0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C032_COMMAND_CLICK_ON_SLOT_BOX_12_INVENTORY_LEGS,             CM2_VIEWPORT_RELATIVE, C511_ZONE_SLOT_BOX_12_INVENTORY_LEGS,             0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C033_COMMAND_CLICK_ON_SLOT_BOX_13_INVENTORY_FEET,             CM2_VIEWPORT_RELATIVE, C512_ZONE_SLOT_BOX_13_INVENTORY_FEET,             0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C034_COMMAND_CLICK_ON_SLOT_BOX_14_INVENTORY_POUCH_2,          CM2_VIEWPORT_RELATIVE, C513_ZONE_SLOT_BOX_14_INVENTORY_POUCH_2,          0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C070_COMMAND_CLICK_ON_MOUTH,                                  CM2_VIEWPORT_RELATIVE, C545_ZONE_MOUTH,                                  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C071_COMMAND_CLICK_ON_EYE,                                    CM2_VIEWPORT_RELATIVE, C546_ZONE_EYE,                                    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C035_COMMAND_CLICK_ON_SLOT_BOX_15_INVENTORY_QUIVER_LINE2_1,   CM2_VIEWPORT_RELATIVE, C514_ZONE_SLOT_BOX_15_INVENTORY_QUIVER_LINE2_1,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C036_COMMAND_CLICK_ON_SLOT_BOX_16_INVENTORY_QUIVER_LINE1_2,   CM2_VIEWPORT_RELATIVE, C515_ZONE_SLOT_BOX_16_INVENTORY_QUIVER_LINE1_2,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C037_COMMAND_CLICK_ON_SLOT_BOX_17_INVENTORY_QUIVER_LINE2_2,   CM2_VIEWPORT_RELATIVE, C516_ZONE_SLOT_BOX_17_INVENTORY_QUIVER_LINE2_2,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C038_COMMAND_CLICK_ON_SLOT_BOX_18_INVENTORY_NECK,             CM2_VIEWPORT_RELATIVE, C517_ZONE_SLOT_BOX_18_INVENTORY_NECK,             0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C039_COMMAND_CLICK_ON_SLOT_BOX_19_INVENTORY_POUCH_1,          CM2_VIEWPORT_RELATIVE, C518_ZONE_SLOT_BOX_19_INVENTORY_POUCH_1,          0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1,   CM2_VIEWPORT_RELATIVE, C519_ZONE_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C041_COMMAND_CLICK_ON_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1, CM2_VIEWPORT_RELATIVE, C520_ZONE_SLOT_BOX_21_INVENTORY_BACKPACK_LINE1_1, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C042_COMMAND_CLICK_ON_SLOT_BOX_22_INVENTORY_BACKPACK_LINE2_2, CM2_VIEWPORT_RELATIVE, C521_ZONE_SLOT_BOX_22_INVENTORY_BACKPACK_LINE2_2, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C043_COMMAND_CLICK_ON_SLOT_BOX_23_INVENTORY_BACKPACK_LINE2_3, CM2_VIEWPORT_RELATIVE, C522_ZONE_SLOT_BOX_23_INVENTORY_BACKPACK_LINE2_3, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C044_COMMAND_CLICK_ON_SLOT_BOX_24_INVENTORY_BACKPACK_LINE2_4, CM2_VIEWPORT_RELATIVE, C523_ZONE_SLOT_BOX_24_INVENTORY_BACKPACK_LINE2_4, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C045_COMMAND_CLICK_ON_SLOT_BOX_25_INVENTORY_BACKPACK_LINE2_5, CM2_VIEWPORT_RELATIVE, C524_ZONE_SLOT_BOX_25_INVENTORY_BACKPACK_LINE2_5, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C046_COMMAND_CLICK_ON_SLOT_BOX_26_INVENTORY_BACKPACK_LINE2_6, CM2_VIEWPORT_RELATIVE, C525_ZONE_SLOT_BOX_26_INVENTORY_BACKPACK_LINE2_6, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C047_COMMAND_CLICK_ON_SLOT_BOX_27_INVENTORY_BACKPACK_LINE2_7, CM2_VIEWPORT_RELATIVE, C526_ZONE_SLOT_BOX_27_INVENTORY_BACKPACK_LINE2_7, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C048_COMMAND_CLICK_ON_SLOT_BOX_28_INVENTORY_BACKPACK_LINE2_8, CM2_VIEWPORT_RELATIVE, C527_ZONE_SLOT_BOX_28_INVENTORY_BACKPACK_LINE2_8, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C049_COMMAND_CLICK_ON_SLOT_BOX_29_INVENTORY_BACKPACK_LINE2_9, CM2_VIEWPORT_RELATIVE, C528_ZONE_SLOT_BOX_29_INVENTORY_BACKPACK_LINE2_9, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C050_COMMAND_CLICK_ON_SLOT_BOX_30_INVENTORY_BACKPACK_LINE1_2, CM2_VIEWPORT_RELATIVE, C529_ZONE_SLOT_BOX_30_INVENTORY_BACKPACK_LINE1_2, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C051_COMMAND_CLICK_ON_SLOT_BOX_31_INVENTORY_BACKPACK_LINE1_3, CM2_VIEWPORT_RELATIVE, C530_ZONE_SLOT_BOX_31_INVENTORY_BACKPACK_LINE1_3, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C052_COMMAND_CLICK_ON_SLOT_BOX_32_INVENTORY_BACKPACK_LINE1_4, CM2_VIEWPORT_RELATIVE, C531_ZONE_SLOT_BOX_32_INVENTORY_BACKPACK_LINE1_4, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C053_COMMAND_CLICK_ON_SLOT_BOX_33_INVENTORY_BACKPACK_LINE1_5, CM2_VIEWPORT_RELATIVE, C532_ZONE_SLOT_BOX_33_INVENTORY_BACKPACK_LINE1_5, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C054_COMMAND_CLICK_ON_SLOT_BOX_34_INVENTORY_BACKPACK_LINE1_6, CM2_VIEWPORT_RELATIVE, C533_ZONE_SLOT_BOX_34_INVENTORY_BACKPACK_LINE1_6, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C055_COMMAND_CLICK_ON_SLOT_BOX_35_INVENTORY_BACKPACK_LINE1_7, CM2_VIEWPORT_RELATIVE, C534_ZONE_SLOT_BOX_35_INVENTORY_BACKPACK_LINE1_7, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C056_COMMAND_CLICK_ON_SLOT_BOX_36_INVENTORY_BACKPACK_LINE1_8, CM2_VIEWPORT_RELATIVE, C535_ZONE_SLOT_BOX_36_INVENTORY_BACKPACK_LINE1_8, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C057_COMMAND_CLICK_ON_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9, CM2_VIEWPORT_RELATIVE, C536_ZONE_SLOT_BOX_37_INVENTORY_BACKPACK_LINE1_9, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C081_COMMAND_CLICK_IN_PANEL,                                  CM2_VIEWPORT_RELATIVE, C101_ZONE_PANEL,                                  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0450_as_Graphic561_PrimaryMouseInput_PartyResting[3] = {
        { C146_COMMAND_WAKE_UP, CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C146_COMMAND_WAKE_UP, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN,   0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0451_as_Graphic561_PrimaryMouseInput_FrozenGame[3] = {
        { C148_COMMAND_UNFREEZE_GAME, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C148_COMMAND_UNFREEZE_GAME, CM1_SCREEN_RELATIVE, C002_ZONE_SCREEN, 0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0452_as_Graphic561_MouseInput_ActionAreaNames[5] = {
        { C112_COMMAND_CLICK_IN_ACTION_AREA_PASS,     CM1_SCREEN_RELATIVE, C098_ZONE_ACTION_AREA_PASS,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C113_COMMAND_CLICK_IN_ACTION_AREA_ACTION_0, CM1_SCREEN_RELATIVE, C082_ZONE_ACTION_AREA_ACTION_0, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C114_COMMAND_CLICK_IN_ACTION_AREA_ACTION_1, CM1_SCREEN_RELATIVE, C083_ZONE_ACTION_AREA_ACTION_1, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C115_COMMAND_CLICK_IN_ACTION_AREA_ACTION_2, CM1_SCREEN_RELATIVE, C084_ZONE_ACTION_AREA_ACTION_2, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0453_as_Graphic561_MouseInput_ActionAreaIcons[5] = {
        { C116_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION, CM1_SCREEN_RELATIVE, C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C117_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_1_ACTION, CM1_SCREEN_RELATIVE, C090_ZONE_ACTION_AREA_CHAMPION_1_ACTION, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C118_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_2_ACTION, CM1_SCREEN_RELATIVE, C091_ZONE_ACTION_AREA_CHAMPION_2_ACTION, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C119_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_3_ACTION, CM1_SCREEN_RELATIVE, C092_ZONE_ACTION_AREA_CHAMPION_3_ACTION, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0454_as_Graphic561_MouseInput_SpellArea[10] = {
        { C109_COMMAND_CLICK_IN_SPELL_AREA_SET_MAGIC_CASTER, CM1_SCREEN_RELATIVE, C221_ZONE_SPELL_AREA_SET_MAGIC_CASTER, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1,         CM1_SCREEN_RELATIVE, C245_ZONE_SPELL_AREA_SYMBOL_1,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C102_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_2,         CM1_SCREEN_RELATIVE, C246_ZONE_SPELL_AREA_SYMBOL_2,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C103_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_3,         CM1_SCREEN_RELATIVE, C247_ZONE_SPELL_AREA_SYMBOL_3,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C104_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_4,         CM1_SCREEN_RELATIVE, C248_ZONE_SPELL_AREA_SYMBOL_4,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C105_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_5,         CM1_SCREEN_RELATIVE, C249_ZONE_SPELL_AREA_SYMBOL_5,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C106_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_6,         CM1_SCREEN_RELATIVE, C250_ZONE_SPELL_AREA_SYMBOL_6,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL,       CM1_SCREEN_RELATIVE, C252_ZONE_SPELL_AREA_CAST_SPELL,       0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C107_COMMAND_CLICK_IN_SPELL_AREA_RECANT_SYMBOL,    CM1_SCREEN_RELATIVE, C254_ZONE_SPELL_AREA_RECANT_SYMBOL,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0455_as_Graphic561_MouseInput_ChampionNamesHands[13] = {
        { C016_COMMAND_SET_LEADER_CHAMPION_0,                                  CM1_SCREEN_RELATIVE, C159_ZONE_CHAMPION_0_STATUS_BOX_NAME,                    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C017_COMMAND_SET_LEADER_CHAMPION_1,                                  CM1_SCREEN_RELATIVE, C160_ZONE_CHAMPION_1_STATUS_BOX_NAME,                    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C018_COMMAND_SET_LEADER_CHAMPION_2,                                  CM1_SCREEN_RELATIVE, C161_ZONE_CHAMPION_2_STATUS_BOX_NAME,                    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C019_COMMAND_SET_LEADER_CHAMPION_3,                                  CM1_SCREEN_RELATIVE, C162_ZONE_CHAMPION_3_STATUS_BOX_NAME,                    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND,  CM1_SCREEN_RELATIVE, C211_ZONE_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C021_COMMAND_CLICK_ON_SLOT_BOX_01_CHAMPION_0_STATUS_BOX_ACTION_HAND, CM1_SCREEN_RELATIVE, C212_ZONE_SLOT_BOX_01_CHAMPION_0_STATUS_BOX_ACTION_HAND, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C022_COMMAND_CLICK_ON_SLOT_BOX_02_CHAMPION_1_STATUS_BOX_READY_HAND,  CM1_SCREEN_RELATIVE, C213_ZONE_SLOT_BOX_02_CHAMPION_1_STATUS_BOX_READY_HAND,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C023_COMMAND_CLICK_ON_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND, CM1_SCREEN_RELATIVE, C214_ZONE_SLOT_BOX_03_CHAMPION_1_STATUS_BOX_ACTION_HAND, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C024_COMMAND_CLICK_ON_SLOT_BOX_04_CHAMPION_2_STATUS_BOX_READY_HAND,  CM1_SCREEN_RELATIVE, C215_ZONE_SLOT_BOX_04_CHAMPION_2_STATUS_BOX_READY_HAND,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C025_COMMAND_CLICK_ON_SLOT_BOX_05_CHAMPION_2_STATUS_BOX_ACTION_HAND, CM1_SCREEN_RELATIVE, C216_ZONE_SLOT_BOX_05_CHAMPION_2_STATUS_BOX_ACTION_HAND, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C026_COMMAND_CLICK_ON_SLOT_BOX_06_CHAMPION_3_STATUS_BOX_READY_HAND,  CM1_SCREEN_RELATIVE, C217_ZONE_SLOT_BOX_06_CHAMPION_3_STATUS_BOX_READY_HAND,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND, CM1_SCREEN_RELATIVE, C218_ZONE_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0456_as_Graphic561_MouseInput_PanelChest[9] = {
        { C058_COMMAND_CLICK_ON_SLOT_BOX_38_CHEST_1, CM2_VIEWPORT_RELATIVE, C537_ZONE_SLOT_BOX_38_CHEST_1, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C059_COMMAND_CLICK_ON_SLOT_BOX_39_CHEST_2, CM2_VIEWPORT_RELATIVE, C538_ZONE_SLOT_BOX_39_CHEST_2, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C060_COMMAND_CLICK_ON_SLOT_BOX_40_CHEST_3, CM2_VIEWPORT_RELATIVE, C539_ZONE_SLOT_BOX_40_CHEST_3, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C061_COMMAND_CLICK_ON_SLOT_BOX_41_CHEST_4, CM2_VIEWPORT_RELATIVE, C540_ZONE_SLOT_BOX_41_CHEST_4, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C062_COMMAND_CLICK_ON_SLOT_BOX_42_CHEST_5, CM2_VIEWPORT_RELATIVE, C541_ZONE_SLOT_BOX_42_CHEST_5, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C063_COMMAND_CLICK_ON_SLOT_BOX_43_CHEST_6, CM2_VIEWPORT_RELATIVE, C542_ZONE_SLOT_BOX_43_CHEST_6, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C064_COMMAND_CLICK_ON_SLOT_BOX_44_CHEST_7, CM2_VIEWPORT_RELATIVE, C543_ZONE_SLOT_BOX_44_CHEST_7, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8, CM2_VIEWPORT_RELATIVE, C544_ZONE_SLOT_BOX_45_CHEST_8, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel[4] = {
        { C160_COMMAND_CLICK_IN_PANEL_RESURRECT,   CM2_VIEWPORT_RELATIVE, M664_ZONE_RESURRECT,   0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C161_COMMAND_CLICK_IN_PANEL_REINCARNATE, CM2_VIEWPORT_RELATIVE, M665_ZONE_REINCARNATE, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C162_COMMAND_CLICK_IN_PANEL_CANCEL,      CM2_VIEWPORT_RELATIVE, M666_ZONE_CANCEL,      0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G2045_as_MouseInput_PanelChampionRename[36] = {
        { C165_COMMAND_RENAME_BACKSPACE, CM2_VIEWPORT_RELATIVE, M667_ZONE_RENAME_BACKSPACE, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C166_COMMAND_RENAME_OK,        CM2_VIEWPORT_RELATIVE, M668_ZONE_RENAME_OK,        0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C167_COMMAND_RENAME_TITLE,     CM2_VIEWPORT_RELATIVE, M669_ZONE_RENAME_TITLE,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C168_COMMAND_RENAME_A,         CM2_VIEWPORT_RELATIVE, M670_ZONE_RENAME_A,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C169_COMMAND_RENAME_B,         CM2_VIEWPORT_RELATIVE, M671_ZONE_RENAME_B,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C170_COMMAND_RENAME_C,         CM2_VIEWPORT_RELATIVE, M672_ZONE_RENAME_C,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C171_COMMAND_RENAME_D,         CM2_VIEWPORT_RELATIVE, M673_ZONE_RENAME_D,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C172_COMMAND_RENAME_E,         CM2_VIEWPORT_RELATIVE, M674_ZONE_RENAME_E,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C173_COMMAND_RENAME_F,         CM2_VIEWPORT_RELATIVE, M675_ZONE_RENAME_F,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C174_COMMAND_RENAME_G,         CM2_VIEWPORT_RELATIVE, M676_ZONE_RENAME_G,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C175_COMMAND_RENAME_H,         CM2_VIEWPORT_RELATIVE, M677_ZONE_RENAME_H,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C176_COMMAND_RENAME_I,         CM2_VIEWPORT_RELATIVE, M678_ZONE_RENAME_I,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C177_COMMAND_RENAME_J,         CM2_VIEWPORT_RELATIVE, M679_ZONE_RENAME_J,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C178_COMMAND_RENAME_K,         CM2_VIEWPORT_RELATIVE, M680_ZONE_RENAME_K,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C179_COMMAND_RENAME_L,         CM2_VIEWPORT_RELATIVE, M681_ZONE_RENAME_L,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C180_COMMAND_RENAME_M,         CM2_VIEWPORT_RELATIVE, M682_ZONE_RENAME_M,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C181_COMMAND_RENAME_N,         CM2_VIEWPORT_RELATIVE, M683_ZONE_RENAME_N,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C182_COMMAND_RENAME_O,         CM2_VIEWPORT_RELATIVE, M684_ZONE_RENAME_O,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C183_COMMAND_RENAME_P,         CM2_VIEWPORT_RELATIVE, M685_ZONE_RENAME_P,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C184_COMMAND_RENAME_Q,         CM2_VIEWPORT_RELATIVE, M686_ZONE_RENAME_Q,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C185_COMMAND_RENAME_R,         CM2_VIEWPORT_RELATIVE, M687_ZONE_RENAME_R,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C186_COMMAND_RENAME_S,         CM2_VIEWPORT_RELATIVE, M688_ZONE_RENAME_S,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C187_COMMAND_RENAME_T,         CM2_VIEWPORT_RELATIVE, M689_ZONE_RENAME_T,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C188_COMMAND_RENAME_U,         CM2_VIEWPORT_RELATIVE, M690_ZONE_RENAME_U,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C189_COMMAND_RENAME_V,         CM2_VIEWPORT_RELATIVE, M691_ZONE_RENAME_V,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C190_COMMAND_RENAME_W,         CM2_VIEWPORT_RELATIVE, M692_ZONE_RENAME_W,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C191_COMMAND_RENAME_X,         CM2_VIEWPORT_RELATIVE, M693_ZONE_RENAME_X,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C192_COMMAND_RENAME_Y,         CM2_VIEWPORT_RELATIVE, M694_ZONE_RENAME_Y,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C193_COMMAND_RENAME_Z,         CM2_VIEWPORT_RELATIVE, M695_ZONE_RENAME_Z,         0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C194_COMMAND_RENAME_COMMA,     CM2_VIEWPORT_RELATIVE, M696_ZONE_RENAME_COMMA,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C195_COMMAND_RENAME_PERIOD,    CM2_VIEWPORT_RELATIVE, M697_ZONE_RENAME_PERIOD,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C196_COMMAND_RENAME_SEMICOLON, CM2_VIEWPORT_RELATIVE, M698_ZONE_RENAME_SEMICOLON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C197_COMMAND_RENAME_COLON,     CM2_VIEWPORT_RELATIVE, M699_ZONE_RENAME_COLON,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C198_COMMAND_RENAME_SPACE,     CM2_VIEWPORT_RELATIVE, M700_ZONE_RENAME_SPACE,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON  },
        { C198_COMMAND_RENAME_SPACE,     CM1_SCREEN_RELATIVE,   C002_ZONE_SCREEN,           0, 0, MASK0x0001_MOUSE_RIGHT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
KEYBOARD_INPUT G2195_as_PrimaryKeyboardInput_Entrance[3] = {
        { C200_COMMAND_ENTRANCE_ENTER_DUNGEON, 0x001C },
        { C216_COMMAND_QUIT,                   0x0410 },
        { 0, 0 } };
KEYBOARD_INPUT G0458_as_Graphic561_PrimaryKeyboardInput_Interface[8] = {
        { C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0, 0x0002 },
        { C008_COMMAND_TOGGLE_INVENTORY_CHAMPION_1, 0x0003 },
        { C009_COMMAND_TOGGLE_INVENTORY_CHAMPION_2, 0x0004 },
        { C010_COMMAND_TOGGLE_INVENTORY_CHAMPION_3, 0x0005 },
        { C140_COMMAND_SAVE_GAME,                   0x081F },
        { C140_COMMAND_SAVE_GAME,                   0x081F },
        { C147_COMMAND_FREEZE_GAME,                      1 },
        { 0, 0 } };
KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7] = {
        { C001_COMMAND_TURN_LEFT,     0x004B },
        { C003_COMMAND_MOVE_FORWARD,  0x004C },
        { C002_COMMAND_TURN_RIGHT,    0x004D },
        { C006_COMMAND_MOVE_LEFT,     0x004F },
        { C005_COMMAND_MOVE_BACKWARD, 0x0050 },
        { C004_COMMAND_MOVE_RIGHT,    0x0051 },
        { 0, 0 } };
KEYBOARD_INPUT G0460_as_Graphic561_PrimaryKeyboardInput_PartyResting[3] = {
        { C146_COMMAND_WAKE_UP,     0x001C },
        { C147_COMMAND_FREEZE_GAME, 0x0001 },
        { 0, 0 } };
KEYBOARD_INPUT G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame[2] = {
        { C148_COMMAND_UNFREEZE_GAME, 0x0001 },
        { 0, 0 } };
KEYBOARD_INPUT G2046_as_PrimaryKeyboardInput_PanelChampionRename[60] = {
        { C165_COMMAND_RENAME_BACKSPACE, 0x000E },
        { C167_COMMAND_RENAME_TITLE,     0x001C },
        { C168_COMMAND_RENAME_A,         0x021E },
        { C169_COMMAND_RENAME_B,         0x0230 },
        { C170_COMMAND_RENAME_C,         0x022E },
        { C171_COMMAND_RENAME_D,         0x0220 },
        { C172_COMMAND_RENAME_E,         0x0212 },
        { C173_COMMAND_RENAME_F,         0x0221 },
        { C174_COMMAND_RENAME_G,         0x0222 },
        { C175_COMMAND_RENAME_H,         0x0223 },
        { C176_COMMAND_RENAME_I,         0x0217 },
        { C177_COMMAND_RENAME_J,         0x0224 },
        { C178_COMMAND_RENAME_K,         0x0225 },
        { C179_COMMAND_RENAME_L,         0x0226 },
        { C180_COMMAND_RENAME_M,         0x0232 },
        { C181_COMMAND_RENAME_N,         0x0231 },
        { C182_COMMAND_RENAME_O,         0x0218 },
        { C183_COMMAND_RENAME_P,         0x0219 },
        { C184_COMMAND_RENAME_Q,         0x0210 },
        { C185_COMMAND_RENAME_R,         0x0213 },
        { C186_COMMAND_RENAME_S,         0x021F },
        { C187_COMMAND_RENAME_T,         0x0214 },
        { C188_COMMAND_RENAME_U,         0x0216 },
        { C189_COMMAND_RENAME_V,         0x022F },
        { C190_COMMAND_RENAME_W,         0x0211 },
        { C191_COMMAND_RENAME_X,         0x022D },
        { C192_COMMAND_RENAME_Y,         0x0215 },
        { C193_COMMAND_RENAME_Z,         0x022C },
        { C168_COMMAND_RENAME_A,         0x001E },
        { C169_COMMAND_RENAME_B,         0x0030 },
        { C170_COMMAND_RENAME_C,         0x002E },
        { C171_COMMAND_RENAME_D,         0x0020 },
        { C172_COMMAND_RENAME_E,         0x0012 },
        { C173_COMMAND_RENAME_F,         0x0021 },
        { C174_COMMAND_RENAME_G,         0x0022 },
        { C175_COMMAND_RENAME_H,         0x0023 },
        { C176_COMMAND_RENAME_I,         0x0017 },
        { C177_COMMAND_RENAME_J,         0x0024 },
        { C178_COMMAND_RENAME_K,         0x0025 },
        { C179_COMMAND_RENAME_L,         0x0026 },
        { C180_COMMAND_RENAME_M,         0x0032 },
        { C181_COMMAND_RENAME_N,         0x0031 },
        { C182_COMMAND_RENAME_O,         0x0018 },
        { C183_COMMAND_RENAME_P,         0x0019 },
        { C184_COMMAND_RENAME_Q,         0x0010 },
        { C185_COMMAND_RENAME_R,         0x0013 },
        { C186_COMMAND_RENAME_S,         0x001F },
        { C187_COMMAND_RENAME_T,         0x0014 },
        { C188_COMMAND_RENAME_U,         0x0016 },
        { C189_COMMAND_RENAME_V,         0x002F },
        { C190_COMMAND_RENAME_W,         0x0011 },
        { C191_COMMAND_RENAME_X,         0x002D },
        { C192_COMMAND_RENAME_Y,         0x0015 },
        { C193_COMMAND_RENAME_Z,         0x002C },
        { C194_COMMAND_RENAME_COMMA,     0x0033 },
        { C195_COMMAND_RENAME_PERIOD,    0x0034 },
        { C196_COMMAND_RENAME_SEMICOLON, 0x0027 },
        { C197_COMMAND_RENAME_COLON,     0x0227 },
        { C198_COMMAND_RENAME_SPACE,     0x0039 },
        { 0, 0 } };



/*#ifdef MEDIA667_P31J*/
STATICFUNCTION void F0673_SetMouseInputBoxFromZone(REGISTER MOUSE_INPUT* P2005_ps_MouseInput);
/*#endif*/

STATICFUNCTION void F0360_COMMAND_ProcessPendingClick();



void F0672_InitializeAllMouseInput(
void
)
{
        F0673_SetMouseInputBoxFromZone(G0447_as_Graphic561_PrimaryMouseInput_Interface);
        F0673_SetMouseInputBoxFromZone(G0448_as_Graphic561_SecondaryMouseInput_Movement);
        F0673_SetMouseInputBoxFromZone(G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory);
        F0673_SetMouseInputBoxFromZone(G0452_as_Graphic561_MouseInput_ActionAreaNames);
        F0673_SetMouseInputBoxFromZone(G0453_as_Graphic561_MouseInput_ActionAreaIcons);
        F0673_SetMouseInputBoxFromZone(G0454_as_Graphic561_MouseInput_SpellArea);
        F0673_SetMouseInputBoxFromZone(G0455_as_Graphic561_MouseInput_ChampionNamesHands);
        F0673_SetMouseInputBoxFromZone(G0456_as_Graphic561_MouseInput_PanelChest);
        F0673_SetMouseInputBoxFromZone(G2045_as_MouseInput_PanelChampionRename);
}

STATICFUNCTION void F0673_SetMouseInputBoxFromZone(
REGISTER MOUSE_INPUT* P2005_ps_MouseInput FINAL_SEPARATOR
{
        int16_t L2283_ai_XYZ[4];


        while (P2005_ps_MouseInput->Command != C000_COMMAND_NONE) {
                if ((P2005_ps_MouseInput->Box.X1 < 0) && F0638_GetZone(P2005_ps_MouseInput->Box.X2, L2283_ai_XYZ)) {
                        if (P2005_ps_MouseInput->Box.X1 == -2) {
                                M704_ZONE_LEFT(L2283_ai_XYZ) += G2067_i_ViewportScreenX;
                                M706_ZONE_TOP(L2283_ai_XYZ) += G2068_i_ViewportScreenY;
                        } else {
                                if (P2005_ps_MouseInput->Box.X1 == -3) {
                                        M704_ZONE_LEFT(L2283_ai_XYZ) += (G2071_C320_ScreenPixelWidth - G2073_C224_ViewportPixelWidth) >> 1;
                                        M706_ZONE_TOP(L2283_ai_XYZ) += (G2072_C200_ScreenPixelHeight - G2074_C136_ViewportHeight) >> 1;
                                }
                        }
                        P2005_ps_MouseInput->Box.X2 = (P2005_ps_MouseInput->Box.X1 = M704_ZONE_LEFT(L2283_ai_XYZ)) + M708_ZONE_WIDTH(L2283_ai_XYZ) - 1;
                        P2005_ps_MouseInput->Box.Y2 = (P2005_ps_MouseInput->Box.Y1 = M706_ZONE_TOP(L2283_ai_XYZ)) + M709_ZONE_HEIGHT(L2283_ai_XYZ) - 1;
                }
                P2005_ps_MouseInput++;
        }
}


/* Discard all commands in queue except commands C129_COMMAND_RELEASE_CHAMPION_ICON and C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL */
void F0357_COMMAND_DiscardAllInput(
void
)
{
        REGISTER int16_t L2284_i_SourceCommandQueueIndex;
        REGISTER int16_t L2285_i_DestinationCommandQueueIndex;


        while (M527_IsCharacterInKeyboardBuffer()) { /* Read and ignore all keyboard input */
                M528_GetCharacterInKeyboardBuffer();
        }
        G0435_B_CommandQueueLocked = C1_TRUE;
        L2285_i_DestinationCommandQueueIndex = G0433_i_CommandQueueFirstIndex;
        if ((L2284_i_SourceCommandQueueIndex = G0434_i_CommandQueueLastIndex + 1) > M529_COMMAND_QUEUE_SIZE) {
                L2284_i_SourceCommandQueueIndex = 0;
        }
        if (L2284_i_SourceCommandQueueIndex != L2285_i_DestinationCommandQueueIndex) {
                L2284_i_SourceCommandQueueIndex = L2285_i_DestinationCommandQueueIndex;
                G2153_i_QueuedCommandsCount = 0;
                for (;;) { /*_Infinite loop_*/
                        if ((G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].Command == C129_COMMAND_RELEASE_CHAMPION_ICON) || (G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].Command == C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL)) {
                                G0432_as_CommandQueue[L2285_i_DestinationCommandQueueIndex].Command = G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].Command;
                                G0432_as_CommandQueue[L2285_i_DestinationCommandQueueIndex].X = G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].X;
                                G0432_as_CommandQueue[L2285_i_DestinationCommandQueueIndex].Y = G0432_as_CommandQueue[L2284_i_SourceCommandQueueIndex].Y;
                                if (++L2285_i_DestinationCommandQueueIndex > M529_COMMAND_QUEUE_SIZE) {
                                        L2285_i_DestinationCommandQueueIndex = 0;
                                }
                                G2153_i_QueuedCommandsCount++;
                        }
                        if (L2284_i_SourceCommandQueueIndex == G0434_i_CommandQueueLastIndex) {
                                break;
                        }
                        if (++L2284_i_SourceCommandQueueIndex > M529_COMMAND_QUEUE_SIZE) {
                                L2284_i_SourceCommandQueueIndex = 0;
                        }
                }
                if (L2285_i_DestinationCommandQueueIndex-- == 0) {
                        L2285_i_DestinationCommandQueueIndex = M529_COMMAND_QUEUE_SIZE;
                }
                G0434_i_CommandQueueLastIndex = L2285_i_DestinationCommandQueueIndex;
        }
        G0435_B_CommandQueueLocked = C0_FALSE;
        F0360_COMMAND_ProcessPendingClick();
}

int16_t F0358_COMMAND_GetCommandFromMouseInput_CPSC(
REGISTER MOUSE_INPUT* P0721_ps_MouseInput   SEPARATOR
REGISTER int16_t      P0722_i_X             SEPARATOR
REGISTER int16_t      P0723_i_Y             SEPARATOR
REGISTER int16_t      P0724_i_ButtonsStatus FINAL_SEPARATOR
{
        REGISTER int16_t L1107_Command;
        int16_t L2286_ai_XYZ[4];


        if ((P0721_ps_MouseInput == NULL)
           ) {
                return C000_COMMAND_NONE;
        }
        while (L1107_Command = P0721_ps_MouseInput->Command) {
                if (P0721_ps_MouseInput->Box.X1 < 0) {
                        if ((P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button) && F0638_GetZone(P0721_ps_MouseInput->Box.X2, L2286_ai_XYZ)) {
                                if (P0721_ps_MouseInput->Box.X1 == -2) {
                                        if (F0798_COMMAND_IsPointInZone(L2286_ai_XYZ, P0722_i_X - G2067_i_ViewportScreenX, P0723_i_Y - G2068_i_ViewportScreenY))
                                                break;
                                } else {
                                        if (P0721_ps_MouseInput->Box.X1 == -3) {
                                                if (F0798_COMMAND_IsPointInZone(L2286_ai_XYZ, P0722_i_X - ((G2071_C320_ScreenPixelWidth - G2073_C224_ViewportPixelWidth) >> 1), P0723_i_Y - ((G2072_C200_ScreenPixelHeight - G2074_C136_ViewportHeight) >> 1)))
                                                        break;
                                        } else {
                                                if (F0798_COMMAND_IsPointInZone(L2286_ai_XYZ, P0722_i_X, P0723_i_Y))
                                                        break;
                                        }
                                }
                        }
                } else {
                        if ((P0722_i_X <= P0721_ps_MouseInput->Box.X2) && (P0722_i_X >= P0721_ps_MouseInput->Box.X1) &&
                            (P0723_i_Y >= P0721_ps_MouseInput->Box.Y1) && (P0723_i_Y <= P0721_ps_MouseInput->Box.Y2) &&
                            (P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button))
                                break;
                }
                                P0721_ps_MouseInput++;
        }
        return L1107_Command;
}

void F0359_COMMAND_ProcessClick_CPSC(
REGISTER int16_t P0725_i_X             SEPARATOR
REGISTER int16_t P0726_i_Y             SEPARATOR
REGISTER int16_t P0727_i_ButtonsStatus FINAL_SEPARATOR
{
        REGISTER int16_t L1108_i_CommandQueueIndex;
        REGISTER int16_t L1109_i_Command;
        int16_t L2287_i_MaximumRegularCommandsInQueue;


        if ((P0727_i_ButtonsStatus == C04_MOUSE_EVENT_LEFT_BUTTON_UP) || (P0727_i_ButtonsStatus == C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION)) {
                L2287_i_MaximumRegularCommandsInQueue = C7_UNKNOWN; /* Allow usage of all commands in queue */
        } else {
                L2287_i_MaximumRegularCommandsInQueue = C5_UNKNOWN; /* Restrict usage to reserve two commands in queue */
        }
        if ((L1108_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 2) > M529_COMMAND_QUEUE_SIZE) {
                L1108_i_CommandQueueIndex -= M529_COMMAND_QUEUE_SIZE + 1;
        }
        if (G2153_i_QueuedCommandsCount < L2287_i_MaximumRegularCommandsInQueue) {
                if (P0727_i_ButtonsStatus == C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION) {
                        L1109_i_Command = C129_COMMAND_RELEASE_CHAMPION_ICON;
                } else {
                        if (P0727_i_ButtonsStatus == C04_MOUSE_EVENT_LEFT_BUTTON_UP) {
                                L1109_i_Command = C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL;
                        } else {
                                L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput, P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus);
                                if (L1109_i_Command == C000_COMMAND_NONE) {
                                        L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput, P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus);
                                }
                        }
                }
                if (L1109_i_Command != C000_COMMAND_NONE) {
                        G2153_i_QueuedCommandsCount++;
                        if (!L1108_i_CommandQueueIndex--) {
                                L1108_i_CommandQueueIndex = M529_COMMAND_QUEUE_SIZE;
                        }
                        G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;
                        G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;
                        G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y;
                }
                G0435_B_CommandQueueLocked = C0_FALSE;
        }
}

STATICFUNCTION void F0360_COMMAND_ProcessPendingClick(
void
)
{
        if (G0436_B_PendingClickPresent) { /* Condition is always false in PC version, so this function has no effect in PC version */
                G0436_B_PendingClickPresent = C0_FALSE;
                F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus);
        }
}

void F0361_COMMAND_ProcessKeyPress(
REGISTER int16_t P0728_KeyCode FINAL_SEPARATOR
{
        REGISTER KEYBOARD_INPUT* L1112_ps_KeyboardInput;
        REGISTER int16_t L1110_i_CommandQueueIndex;
        REGISTER int16_t L1111_i_Command;


        F0617_PrintMemoryUsage(P0728_KeyCode);
        if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL) {
                return;
        }
        G0435_B_CommandQueueLocked = C1_TRUE;
        if (G2153_i_QueuedCommandsCount < C5_UNKNOWN) { /* Restrict usage to reserve two commands in queue */ /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                if ((L1110_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 2) > M529_COMMAND_QUEUE_SIZE) {
                        L1110_i_CommandQueueIndex -= M529_COMMAND_QUEUE_SIZE + 1;
                }
                /* BUG0_73 A mouse click may be ignored. At this point, the command index where the keyboard command will be stored in the command queue is determined by L1110_i_CommandQueueIndex. If a mouse click interrupt occurs before the command is actually written into the command queue below then the mouse command will be stored at the same location in the queue and then be overwritten by the keyboard command */
                while (L1111_i_Command = L1112_ps_KeyboardInput->Command) {
                        if (P0728_KeyCode == L1112_ps_KeyboardInput->Code) {
                                T0361xxx:
                                if (!L1110_i_CommandQueueIndex--) {
                                        L1110_i_CommandQueueIndex = M529_COMMAND_QUEUE_SIZE;
                                }
                                G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;
                                G2153_i_QueuedCommandsCount++;
                                goto T0361014;
                        }
                        else {
                                L1112_ps_KeyboardInput++;
                        }
                }
                if ((L1112_ps_KeyboardInput = G0444_ps_SecondaryKeyboardInput) == NULL)
                        goto T0361014;
                while (L1111_i_Command = L1112_ps_KeyboardInput->Command) {
                        if (P0728_KeyCode == L1112_ps_KeyboardInput->Code) {
                                goto T0361xxx;
                        }
                        else {
                                L1112_ps_KeyboardInput++;
                        }
                }
        }
        T0361014:
        G0435_B_CommandQueueLocked = C0_FALSE;
        F0360_COMMAND_ProcessPendingClick();
}



#include "CLIKCHAM.C"

MOUSE_INPUT G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice[2] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM2_VIEWPORT_RELATIVE, C456_ZONE_DIALOG_BOTTOM_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices[3] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM2_VIEWPORT_RELATIVE, C457_ZONE_DIALOG_TOP_BUTTON,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM2_VIEWPORT_RELATIVE, C456_ZONE_DIALOG_BOTTOM_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices[4] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM2_VIEWPORT_RELATIVE, C457_ZONE_DIALOG_TOP_BUTTON,          0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM2_VIEWPORT_RELATIVE, C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3, CM2_VIEWPORT_RELATIVE, C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices[5] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM2_VIEWPORT_RELATIVE, C458_ZONE_DIALOG_TOP_LEFT_BUTTON,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM2_VIEWPORT_RELATIVE, C459_ZONE_DIALOG_TOP_RIGHT_BUTTON,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3, CM2_VIEWPORT_RELATIVE, C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4, CM2_VIEWPORT_RELATIVE, C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice[2] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM3_SCREEN_CENTERED_DIALOG, C456_ZONE_DIALOG_BOTTOM_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices[3] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM3_SCREEN_CENTERED_DIALOG, C457_ZONE_DIALOG_TOP_BUTTON,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM3_SCREEN_CENTERED_DIALOG, C456_ZONE_DIALOG_BOTTOM_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices[4] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM3_SCREEN_CENTERED_DIALOG, C457_ZONE_DIALOG_TOP_BUTTON,          0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM3_SCREEN_CENTERED_DIALOG, C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3, CM3_SCREEN_CENTERED_DIALOG, C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices[5] = {
        { C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1, CM3_SCREEN_CENTERED_DIALOG, C458_ZONE_DIALOG_TOP_LEFT_BUTTON,     0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C211_COMMAND_CLICK_ON_DIALOG_CHOICE_2, CM3_SCREEN_CENTERED_DIALOG, C459_ZONE_DIALOG_TOP_RIGHT_BUTTON,    0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C212_COMMAND_CLICK_ON_DIALOG_CHOICE_3, CM3_SCREEN_CENTERED_DIALOG, C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON,  0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4, CM3_SCREEN_CENTERED_DIALOG, C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON, 0, 0, MASK0x0002_MOUSE_LEFT_BUTTON },
        { 0, 0, 0, 0, 0, 0 } };
MOUSE_INPUT* G0480_aaps_PrimaryMouseInput_DialogSets[2][4] = {
        { G0471_as_Graphic561_PrimaryMouseInput_ViewportDialog1Choice,
          G0472_as_Graphic561_PrimaryMouseInput_ViewportDialog2Choices,
          G0473_as_Graphic561_PrimaryMouseInput_ViewportDialog3Choices,
          G0474_as_Graphic561_PrimaryMouseInput_ViewportDialog4Choices },
        { G0475_as_Graphic561_PrimaryMouseInput_ScreenDialog1Choice,
          G0476_as_Graphic561_PrimaryMouseInput_ScreenDialog2Choices,
          G0477_as_Graphic561_PrimaryMouseInput_ScreenDialog3Choices,
          G0478_as_Graphic561_PrimaryMouseInput_ScreenDialog4Choices } };


STATICFUNCTION void F0378_COMMAND_ProcessType81_ClickInPanel(
REGISTER int16_t P0754_i_X SEPARATOR
REGISTER int16_t P0755_i_Y FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L1157_ui_Command;
        REGISTER BOOLEAN L1158_B_NoLeader = (G0411_i_LeaderIndex == CM1_CHAMPION_NONE);


        switch (G2008_i_PanelContent) {
                case M569_PANEL_CHEST:
                        if (L1158_B_NoLeader) {
                                return;
                        }
#ifdef PC_FIX_CODE_SIZE
        L1157_ui_Command++;
#endif
                        L1157_ui_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0456_as_Graphic561_MouseInput_PanelChest, P0754_i_X, P0755_i_Y, MASK0x0002_MOUSE_LEFT_BUTTON);
                        if (L1157_ui_Command != C000_COMMAND_NONE) {
                                F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(L1157_ui_Command - C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND);
                        }
                        break;
                case M568_PANEL_RESURRECT_REINCARNATE:
                        if (!G0415_ui_LeaderEmptyHanded)
                                break;
                        L1157_ui_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel, P0754_i_X, P0755_i_Y, MASK0x0002_MOUSE_LEFT_BUTTON);
                        if (L1157_ui_Command != C000_COMMAND_NONE) {
                                F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel(L1157_ui_Command);
                        }
                        break;
        }
}

void F0379_COMMAND_DrawRestScreen(
void
)
{
        F0134_VIDEO_FillBitmap(M772_CAST_PC(G0296_puc_Bitmap_Viewport), C00_COLOR_BLACK, G2073_C224_ViewportPixelWidth, G2074_C136_ViewportHeight);
#ifdef X561_I34E
        F0649_PrintCenteredTextToViewportZone(C006_ZONE_VIEWPORT_CENTERED_TEXT, C04_COLOR_CYAN, C00_COLOR_BLACK, "WAKE UP");
#endif
#ifdef X736_I34M
        F0649_PrintCenteredTextToViewportZone(C006_ZONE_VIEWPORT_CENTERED_TEXT, C04_COLOR_CYAN, C00_COLOR_BLACK, F0758_TranslateLanguage(C19_WAKE_UP));
#endif
}


void F0380_COMMAND_ProcessQueue_CPSC(
void
)
{
        REGISTER int16_t L1160_i_Command;
        REGISTER int16_t L1159_i_Multiple;
#define AL1159_i_CommandQueueIndex L1159_i_Multiple
#define AL1159_i_ChampionIndex     L1159_i_Multiple
#define AL1159_i_Checksum          L1159_i_Multiple
        REGISTER int16_t L1161_i_CommandX;
        REGISTER int16_t L1162_i_CommandY;
        static KEYBOARD_INPUT* G0481_ps_PrimaryKeyboardInputBackup;
        static KEYBOARD_INPUT* G0482_ps_SecondaryKeyboardInputBackup;
        static MOUSE_INPUT* G0483_ps_PrimaryMouseInputBackup;
        static MOUSE_INPUT* G0484_ps_SecondaryMouseInputBackup;


        G0435_B_CommandQueueLocked = C1_TRUE;
        if ((AL1159_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 1) > M529_COMMAND_QUEUE_SIZE) {
                AL1159_i_CommandQueueIndex = 0;
        }
        if (G2153_i_QueuedCommandsCount == 0) {
                goto T0380xxx; /* This is possibly a compiler optimization */
        }
        L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;
        if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks && (G0312_i_LastProjectileDisabledMovementDirection == (M021_NORMALIZE(G0308_i_PartyDirection + L1160_i_Command - C003_COMMAND_MOVE_FORWARD)))))) { /* If movement is disabled */
                T0380xxx:
                G0435_B_CommandQueueLocked = C0_FALSE;
                F0360_COMMAND_ProcessPendingClick();
                goto T0380042;
        }
        L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;
        L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;
        G2153_i_QueuedCommandsCount--;
        if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE) {
                G0433_i_CommandQueueFirstIndex = 0;
        }
        G0435_B_CommandQueueLocked = C0_FALSE;
        F0360_COMMAND_ProcessPendingClick();
        if (L1160_i_Command == C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL) {
                if (G0331_B_PressingEye) {
                        G0331_B_PressingEye = C0_FALSE;
                        G0597_B_IgnoreMouseMovements = C0_FALSE;
                        F0353_INVENTORY_DrawStopPressingEye();
                }
                if (G0333_B_PressingMouth) {
                        G0333_B_PressingMouth = C0_FALSE;
                        G0597_B_IgnoreMouseMovements = C0_FALSE;
                        F0350_INVENTORY_DrawStopPressingMouth();
                }
                if (!G2152_B_PressingClosedImaginaryFakeWall) {
                        return;
                }
                G2152_B_PressingClosedImaginaryFakeWall = C0_FALSE;
                G0597_B_IgnoreMouseMovements = C0_FALSE;
                G0587_i_HideMousePointerRequestCount = 1;
                M523_MOUSE_ShowPointer();
                return;
        }
        if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT)) {
                F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);
                goto T0380042;
        }
        if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)) {
                F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);
                goto T0380042;
        }
        if ((L1160_i_Command >= C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX) && (L1160_i_Command <= C015_COMMAND_CLICK_IN_CHAMPION_3_STATUS_BOX)) {
                if (((AL1159_i_ChampionIndex = L1160_i_Command - C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX) < G0305_ui_PartyChampionCount) && !G0299_ui_CandidateChampionOrdinal) {
                        F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox(AL1159_i_ChampionIndex, L1161_i_CommandX, L1162_i_CommandY);
                }
                goto T0380042;
        }
        if ((L1160_i_Command >= C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT) && (L1160_i_Command <= C128_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_LEFT)) {
                F0070_MOUSE_ProcessCommands125To128_ClickOnChampionIcon(L1160_i_Command - C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT);
                goto T0380042;
        }
        if (L1160_i_Command == C129_COMMAND_RELEASE_CHAMPION_ICON) {
                F0703_ReleaseChampionIcon();
                return;
        }
        if ((L1160_i_Command >= C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND) && (L1160_i_Command < (C065_COMMAND_CLICK_ON_SLOT_BOX_45_CHEST_8 + 1))) {
                if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                        F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(L1160_i_Command - C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND);
                }
                goto T0380042;
        }
        if ((L1160_i_Command >= C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0) && (L1160_i_Command <= C011_COMMAND_CLOSE_INVENTORY)) {
                if ((((AL1159_i_ChampionIndex = L1160_i_Command - C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0) == C04_CHAMPION_CLOSE_INVENTORY) || (AL1159_i_ChampionIndex < G0305_ui_PartyChampionCount)) && !G0299_ui_CandidateChampionOrdinal) {
                        F0355_INVENTORY_Toggle_CPSE(AL1159_i_ChampionIndex);
                }
                goto T0380042;
        }
        if (L1160_i_Command == C083_COMMAND_TOGGLE_INVENTORY_LEADER) {
                if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                        F0355_INVENTORY_Toggle_CPSE(G0411_i_LeaderIndex);
                }
                goto T0380042;
        }
        if (L1160_i_Command == C100_COMMAND_CLICK_IN_SPELL_AREA) {
                if ((!G0299_ui_CandidateChampionOrdinal) && (G0514_i_MagicCasterChampionIndex != CM1_CHAMPION_NONE)) {
                        F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE(L1161_i_CommandX, L1162_i_CommandY);
                }
                goto T0380042;
        }
        if (L1160_i_Command == C111_COMMAND_CLICK_IN_ACTION_AREA) {
                if (!G0299_ui_CandidateChampionOrdinal) {
                        F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE(L1161_i_CommandX, L1162_i_CommandY);
                }
                goto T0380042;
        }
        if (L1160_i_Command == C070_COMMAND_CLICK_ON_MOUTH) {
                F0349_INVENTORY_ProcessCommand70_ClickOnMouth();
                goto T0380042;
        }
        if (L1160_i_Command == C071_COMMAND_CLICK_ON_EYE) {
                F0352_INVENTORY_ProcessCommand71_ClickOnEye();
                goto T0380042;
        }
        if (L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW) {
                F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY);
                goto T0380042;
        }
        if (L1160_i_Command == C081_COMMAND_CLICK_IN_PANEL) {
                F0378_COMMAND_ProcessType81_ClickInPanel(L1161_i_CommandX, L1162_i_CommandY);
                goto T0380042;
        }
        /* BUG0_52 The remaining commands should be ignored while pressing the eye or mouth (pressing the mouth is only possible when the panel does not contain the Food and Water bars but a scroll, a chest or the resurrect/reincarnate panel). If you press CTRL-S on the keyboard while pressing the eye or mouth with the mouse, the save game menu appears but the mouse pointer is missing on screen. If you press ESC on the keyboard while pressing the eye or the mouth, the game is frozen but the panel is drawn on screen when you release the mouse button (Food and Water bars, scroll in hand, open chest or resurrect/reincarnate). If you press ESC again to unfreeze before releasing the mouse button, the panel is drawn over the dungeon view, and the game may crash because mouse input is not configured correctly (you can still click on the eye or mouth while the dungeon view is displayed) */
        if (G0331_B_PressingEye || G0333_B_PressingMouth) {
                goto T0380042;
        }
        if (L1160_i_Command == C145_COMMAND_REST) { /* BUG0_53 It is possible to rest while the inventory of a candidate champion is open. When the party wakes up, it is possible to add a duplicate copy of the same champion to the party. All the champion possessions are then cloned objects. It is also possible to grab an object from the candidate champion before resting and this will clone that object. The rest icon is erased when the inventory of a candidate champion is opened but it is still possible to click on the rest icon location on the top right of the viewport */
                if (!G0299_ui_CandidateChampionOrdinal) {
                        if (G0423_i_InventoryChampionOrdinal) {
                                F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY);
                        }
                        F0456_START_DrawDisabledMenus();
                        G0300_B_PartyIsResting = C1_TRUE;
                        F0379_COMMAND_DrawRestScreen();
                        F0097_DUNGEONVIEW_DrawViewport(C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);
                        G0318_i_WaitForInputMaximumVerticalBlankCount = 0;
                        G0441_ps_PrimaryMouseInput = G0450_as_Graphic561_PrimaryMouseInput_PartyResting;
                        G0442_ps_SecondaryMouseInput = NULL;
                        G0443_ps_PrimaryKeyboardInput = G0460_as_Graphic561_PrimaryKeyboardInput_PartyResting;
                        G0444_ps_SecondaryKeyboardInput = NULL;
#ifdef PC_FIX_CODE_SIZE
        L1160_i_Command++;
#endif
                        F0357_COMMAND_DiscardAllInput();
                }
                goto T0380042;
        }
        if (L1160_i_Command == C146_COMMAND_WAKE_UP) {
                F0314_CHAMPION_WakeUp();
                goto T0380042;
        }
        if (L1160_i_Command == C140_COMMAND_SAVE_GAME) {
                if ((G0305_ui_PartyChampionCount > 0) && !G0299_ui_CandidateChampionOrdinal) {
                        F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF();
                }
                goto T0380042;
        }
        if (L1160_i_Command == C141_COMMAND_TOGGLE_MUSIC) {
                G2024_B_PendingMusicOn = !G2024_B_PendingMusicOn;
                if (G0423_i_InventoryChampionOrdinal) {
                        M008_SET(M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)].Attributes, MASK0x4000_VIEWPORT);
                        F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal));
                }
                return;
        }
        if (L1160_i_Command == C147_COMMAND_FREEZE_GAME) {
                G0301_B_GameTimeTicking = C0_FALSE;
                F0456_START_DrawDisabledMenus();
                F0134_VIDEO_FillBitmap(M772_CAST_PC(G0296_puc_Bitmap_Viewport), C00_COLOR_BLACK, G2073_C224_ViewportPixelWidth, G2074_C136_ViewportHeight);
#ifdef X561_I34E
                F0649_PrintCenteredTextToViewportZone(C006_ZONE_VIEWPORT_CENTERED_TEXT, C04_COLOR_CYAN, C00_COLOR_BLACK, "GAME FROZEN");
#endif
#ifdef X736_I34M
                F0649_PrintCenteredTextToViewportZone(C006_ZONE_VIEWPORT_CENTERED_TEXT, C04_COLOR_CYAN, C00_COLOR_BLACK, F0758_TranslateLanguage(C20_GAME_FROZEN));
#endif
                F0097_DUNGEONVIEW_DrawViewport(C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);
                G0483_ps_PrimaryMouseInputBackup = G0441_ps_PrimaryMouseInput;
                G0484_ps_SecondaryMouseInputBackup = G0442_ps_SecondaryMouseInput;
                G0481_ps_PrimaryKeyboardInputBackup = G0443_ps_PrimaryKeyboardInput;
                G0482_ps_SecondaryKeyboardInputBackup = G0444_ps_SecondaryKeyboardInput;
                G0441_ps_PrimaryMouseInput = G0451_as_Graphic561_PrimaryMouseInput_FrozenGame;
                G0442_ps_SecondaryMouseInput = NULL;
                G0443_ps_PrimaryKeyboardInput = G0461_as_Graphic561_PrimaryKeyboardInput_FrozenGame;
                G0444_ps_SecondaryKeyboardInput = NULL;
                F0357_COMMAND_DiscardAllInput();
                F0740_MUSIC_Pause();
                goto T0380042;
        }
        if (L1160_i_Command == C148_COMMAND_UNFREEZE_GAME) {
                F0738_MUSIC_Continue();
                G0301_B_GameTimeTicking = C1_TRUE;
                F0457_START_DrawEnabledMenus_CPSF();
                G0441_ps_PrimaryMouseInput = G0483_ps_PrimaryMouseInputBackup;
                G0442_ps_SecondaryMouseInput = G0484_ps_SecondaryMouseInputBackup;
                G0443_ps_PrimaryKeyboardInput = G0481_ps_PrimaryKeyboardInputBackup;
                G0444_ps_SecondaryKeyboardInput = G0482_ps_SecondaryKeyboardInputBackup;
                F0357_COMMAND_DiscardAllInput();
                goto T0380042;
        }
        if (L1160_i_Command == C200_COMMAND_ENTRANCE_ENTER_DUNGEON) {
                G0298_B_NewGame = C001_MODE_LOAD_DUNGEON;
                goto T0380042;
        }
        if (L1160_i_Command == C201_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON) {
                G0298_B_NewGame = C001_MODE_LOAD_DUNGEON;
                G1147_B_LoadBonusDungeon = C1_TRUE;
                goto T0380042;
        }
        if (L1160_i_Command == M566_COMMAND_ENTRANCE_RESUME) {
                G0298_B_NewGame = C000_MODE_LOAD_SAVED_GAME;
                goto T0380042;
        }
        if ((L1160_i_Command >= C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1) && (L1160_i_Command <= C213_COMMAND_CLICK_ON_DIALOG_CHOICE_4)) {
                G0335_ui_SelectedDialogChoice = L1160_i_Command - (C210_COMMAND_CLICK_ON_DIALOG_CHOICE_1 - 1);
                goto T0380042;
        }
        if (L1160_i_Command == C215_COMMAND_RESTART_GAME) {
                G0523_B_RestartGameRequested = C1_TRUE;
                goto T0380042;
        }
        if (L1160_i_Command == C216_COMMAND_QUIT) {
                F0666_endgame();
                goto T0380042;
        }
        if ((L1160_i_Command >= C165_COMMAND_RENAME_BACKSPACE) && (L1160_i_Command <= C198_COMMAND_RENAME_SPACE)) { /* Champion rename */
                G2139_i_RenameCommand = L1160_i_Command;
        }
        T0380042: ;
}

#endif
/* END COMMAND.C */

/* BEGIN MENU.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
/*#ifdef MEDIA459_P20JA_P20JB_P31J*/
static BOOLEAN F0407_MENUS_IsActionPerformed(
REGISTER unsigned int16_t P0787_ui_ChampionIndex SEPARATOR
int16_t                   P0788_i_ActionIndex
);
/*#endif*/


unsigned char G0485_aauc_Graphic560_SymbolBaseManaCost[4][6] = {
        { 1, 2, 3, 4, 5, 6 },   /* Power 1 */
        { 2, 3, 4, 5, 6, 7 },   /* Power 2 */
        { 4, 5, 6, 7, 7, 9 },   /* Power 3 */
        { 2, 2, 3, 4, 6, 7 } }; /* Power 4 */
unsigned char G0486_auc_Graphic560_SymbolManaCostMultiplier[6] = { 8, 12, 16, 20, 24, 28 };
SPELL G0487_as_Graphic560_Spells[M530_SPELL_COUNT] = {
        /* { Symbols, BaseRequiredSkillLevel, SkillIndex, Attributes } Kind, Type, Duration, Symbol names, Description */
        { 0x00666F00, 2, C15_SKILL_DEFEND,   0x7843 }, /* 3   4  30     Ya Ir           Shield (Party) */
        { 0x00667073, 1, C18_SKILL_EARTH,    0x4863 }, /* 3   6  18     Ya Bro Ros      Magic Footprints */
        { 0x00686D77, 3, C17_SKILL_AIR,      0xB433 }, /* 3   3  45     Oh Ew Sar       Invisibility (Party) */
        { 0x00686C00, 3, C19_SKILL_WATER,    0x6C72 }, /* 2   7  27     Oh Ven          Poison Cloud */
        { 0x00686D76, 3, C18_SKILL_EARTH,    0x8423 }, /* 3   2  33     Oh Ew Ra        See Through Walls = Thieve's Eye */
        { 0x00686E76, 4, C17_SKILL_AIR,      0x7822 }, /* 2   2  30     Oh Kath Ra      Lightning Bolt */
        { 0x00686F76, 4, C17_SKILL_AIR,      0x5803 }, /* 3   0  22     Oh Ir Ra        Light */
        { 0x00690000, 1, C16_SKILL_FIRE,     0x3C53 }, /* 3   5  15     Ful             Torch */
        { 0x00696F00, 3, C16_SKILL_FIRE,     0xA802 }, /* 2   0  42     Ful Ir          Fireball */
        { 0x00697072, 4, C13_SKILL_HEAL,     0x3C71 }, /* 1   7  15     Ful Bro Ku      Strength Potion (Ku Potion) */
        { 0x00697075, 4, C15_SKILL_DEFEND,   0x7083 }, /* 3   8  28     Ful Bro Neta    Fire Shield (Party) */
        { 0x006A6D00, 1, C18_SKILL_EARTH,    0x5032 }, /* 2   3  20     Des Ew          Weaken Nonmaterial Beings */
        { 0x006A6C00, 1, C19_SKILL_WATER,    0x4062 }, /* 2   6  16     Des Ven         Poison Bolt */
        { 0x006A6F77, 1, C15_SKILL_DEFEND,   0x3013 }, /* 3   1  12     Des Ir Sar      Darkness */
        { 0x006B0000, 1, C17_SKILL_AIR,      0x3C42 }, /* 2   4  15     Zo              Open Door */
        { 0x00667000, 2, C15_SKILL_DEFEND,   0x64C1 }, /* 1  12  25     Ya Bro          Shield Potion (Ya Potion) */
        { 0x00660000, 2, C13_SKILL_HEAL,     0x3CB1 }, /* 1  11  15     Ya              Stamina Potion (DM: Ma Potion, Mon Potion B) (CSB: Mon Potion) */
        { 0x00667074, 4, C13_SKILL_HEAL,     0x3C81 }, /* 1   8  15     Ya Bro Dain     Wisdom Potion (Dane Potion) */
        { 0x00667075, 4, C13_SKILL_HEAL,     0x3C91 }, /* 1   9  15     Ya Bro Neta     Vitality Potion (Neta Potion) */
        { 0x00670000, 1, C13_SKILL_HEAL,     0x80E1 }, /* 1  14  32     Vi              Health Potion (Vi Potion) */
        { 0x00677000, 1, C13_SKILL_HEAL,     0x68A1 }, /* 1  10  26     Vi Bro          Cure Poison Potion (DM: Bro Potion) (CSB: Antivenin) */
        { 0x00687073, 4, C13_SKILL_HEAL,     0x3C61 }, /* 1   6  15     Oh Bro Ros      Dexterity Potion (Ros Potion) */
        { 0x006B7076, 3, C02_SKILL_PRIEST,   0xFCD1 }, /* 1  13  63     Zo Bro Ra       Mana Potion (Ee Potion) */
        { 0x006B6C00, 2, C19_SKILL_WATER,    0x7831 }, /* 1   3  30     Zo Ven          Poison Potion (Ven Potion) */
        { 0x006B6E76, 0, C03_SKILL_WIZARD,   0x3C73 }  /* 3   7  15     Zo Kath Ra      Zokathra Spell */
        };
ACTION_SET G0489_as_Graphic560_ActionSets[44] = { /* 6 bytes per entry instead of 7 */
        /* { ActionIndices[0], ActionIndices[1], ActionIndices[2], ActionProperties[0], ActionProperties[1], Useless } */
        { 255, 255, 255, 0                                    , 0                                    ,  0 },
        {  27,  43,  35, 0                                    , 0                                    ,  3 },
        {   6,   7,   8, 0                                    , 0                                    ,  9 },
        {   0,   0,   0, 0                                    , 0                                    ,  0 },
        {   0,   0,   0, 0                                    , 0                                    ,  0 },
        {  13, 255, 255, 0                                    , 0                                    ,  4 },
        {  13,  20, 255, 7 | MASK0x0080_ACTION_REQUIRES_CHARGE, 0                                    , 16 },
        {  13,  23, 255, 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 0                                    , 17 },
        {  28,  41,  22, 2                                    , 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 14 },
        {  16,   2,  23, 0                                    , 4 | MASK0x0080_ACTION_REQUIRES_CHARGE, 17 },
        {   2,  25,  20, 2                                    , 6 | MASK0x0080_ACTION_REQUIRES_CHARGE, 16 },
        {  17,  41,  34, 3                                    , 5                                    , 16 },
        {  42,   9,  28, 0                                    , 2                                    ,  9 },
        {  13,  17,   2, 2                                    , 3                                    ,  4 },
        {  16,  17,  15, 1                                    , 5                                    ,  5 },
        {  28,  17,  25, 1                                    , 5                                    ,  4 },
        {   2,  25,  15, 5                                    , 6                                    ,  5 },
        {   9,   2,  29, 2                                    , 5                                    ,  4 },
        {  16,  29,  24, 2                                    , 4                                    ,  3 },
        {  13,  15,  19, 5                                    , 7                                    ,  4 },
        {  13,   2,  25, 0                                    , 5                                    ,  4 },
        {   2,  29,  19, 3                                    , 8                                    ,  4 },
        {  13,  30,  31, 2                                    , 4                                    ,  6 },
        {  13,  31,  25, 3                                    , 6                                    ,  6 },
        {  42,  30, 255, 0                                    , 0                                    ,  6 },
        {   0,   0,   0, 0                                    , 0                                    ,  0 },
        {  42,   9, 255, 0                                    , 0                                    ,  9 },
        {  32, 255, 255, 0                                    , 0                                    , 11 },
        {  37,  33,  36, 2 | MASK0x0080_ACTION_REQUIRES_CHARGE, 3                                    , 15 },
        {  37,  33,  34, 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 4 | MASK0x0080_ACTION_REQUIRES_CHARGE, 15 },
        {  17,  38,  21, 0 | MASK0x0080_ACTION_REQUIRES_CHARGE, 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 16 },
        {  13,  21,  34, 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 4 | MASK0x0080_ACTION_REQUIRES_CHARGE, 16 },
        {  36,  37,  41, 2                                    , 3                                    , 18 },
        {  13,  23,  39, 2 | MASK0x0080_ACTION_REQUIRES_CHARGE, 4 | MASK0x0080_ACTION_REQUIRES_CHARGE, 17 },
        {  13,  17,  40, 0                                    , 3 | MASK0x0080_ACTION_REQUIRES_CHARGE, 17 },
        {  17,  36,  38, 3                                    , 4 | MASK0x0080_ACTION_REQUIRES_CHARGE, 19 },
        {   4, 255, 255, 0                                    , 0                                    , 14 },
        {   5, 255, 255, 0                                    , 0                                    ,  0 },
        {  11, 255, 255, 0                                    , 0                                    , 14 },
        {  10, 255, 255, 0                                    , 0                                    ,  8 },
        {  42,   9, 255, 0                                    , 0                                    ,  9 },
        {   1,  12, 255, 2                                    , 0                                    ,  9 },
        {  42, 255, 255, 0                                    , 0                                    , 10 },
        {   6,  11, 255, 0 | MASK0x0080_ACTION_REQUIRES_CHARGE, 0                                    ,  3 } };
#ifdef X431_I34E
char G0490_ac_Graphic560_ActionNames[300] = "N\0BLOCK\0CHOP\0X\0BLOW HORN\0FLIP\0PUNCH\0KICK\0WAR CRY\0STAB\0CLIMB DOWN\0FREEZE LIFE\0HIT\0SWING\0STAB\0THRUST\0JAB\0PARRY\0HACK\0BERZERK\0FIREBALL\0DISPELL\0CONFUSE\0LIGHTNING\0DISRUPT\0MELEE\0X\0INVOKE\0SLASH\0CLEAVE\0BASH\0STUN\0SHOOT\0SPELLSHIELD\0FIRESHIELD\0FLUXCAGE\0HEAL\0CALM\0LIGHT\0WINDOW\0SPIT\0BRANDISH\0THROW\0FUSE";
#endif
#ifdef X736_I34M
char* G0490_ac_Graphic560_ActionNames;
#endif
unsigned char G0491_auc_Graphic560_ActionDisabledTicks[44] = {
        0,   /* N */
        6,   /* BLOCK */
        8,   /* CHOP */
        0,   /* X */
        6,   /* BLOW HORN */
        3,   /* FLIP */
        1,   /* PUNCH */
        5,   /* KICK */
        3,   /* WAR CRY */
        5,   /* STAB */
        35,  /* CLIMB DOWN */
        20,  /* FREEZE LIFE */
        4,   /* HIT */
        6,   /* SWING */
        10,  /* STAB */
        16,  /* THRUST */
        2,   /* JAB */
        18,  /* PARRY */
        8,   /* HACK */
        30,  /* BERZERK */
        42,  /* FIREBALL */
        31,  /* DISPELL */
        10,  /* CONFUSE */
        38,  /* LIGHTNING */
        9,   /* DISRUPT */
        20,  /* MELEE */
        10,  /* X */
        16,  /* INVOKE */
        4,   /* SLASH */
        12,  /* CLEAVE */
        20,  /* BASH */
        7,   /* STUN */
        14,  /* SHOOT */
        30,  /* SPELLSHIELD */
        35,  /* FIRESHIELD */
        2,   /* FLUXCAGE */
        19,  /* HEAL */
        9,   /* CALM */
        10,  /* LIGHT */
        15,  /* WINDOW */
        22,  /* SPIT */
        10,  /* BRANDISH */
        0,   /* THROW */
        2 }; /* FUSE */
unsigned char G0492_auc_Graphic560_ActionDamageFactor[44] = {
        0,   /* N */
        15,  /* BLOCK */
        48,  /* CHOP */
        0,   /* X */
        0,   /* BLOW HORN */
        0,   /* FLIP */
        32,  /* PUNCH */
        48,  /* KICK */
        0,   /* WAR CRY */
        48,  /* STAB */
        0,   /* CLIMB DOWN */
        0,   /* FREEZE LIFE */
        20,  /* HIT */
        16,  /* SWING */
        60,  /* STAB */
        66,  /* THRUST */
        8,   /* JAB */
        8,   /* PARRY */
        25,  /* HACK */
        96,  /* BERZERK */
        0,   /* FIREBALL */
        0,   /* DISPELL */
        0,   /* CONFUSE */
        0,   /* LIGHTNING */
        55,  /* DISRUPT */
        60,  /* MELEE */
        0,   /* X */
        0,   /* INVOKE */
        16,  /* SLASH */
        48,  /* CLEAVE */
        50,  /* BASH */
        16,  /* STUN */
        0,   /* SHOOT */
        0,   /* SPELLSHIELD */
        0,   /* FIRESHIELD */
        0,   /* FLUXCAGE */
        0,   /* HEAL */
        0,   /* CALM */
        0,   /* LIGHT */
        0,   /* WINDOW */
        0,   /* SPIT */
        0,   /* BRANDISH */
        0,   /* THROW */
        0 }; /* FUSE */
unsigned char G0493_auc_Graphic560_ActionHitProbability[44] = {
        0,   /* N */
        22,  /* BLOCK */
        48,  /* CHOP */
        0,   /* X */
        0,   /* BLOW HORN */
        0,   /* FLIP */
        38,  /* PUNCH */
        28,  /* KICK */
        0,   /* WAR CRY */
        30,  /* STAB */
        0,   /* CLIMB DOWN */
        0,   /* FREEZE LIFE */
        20,  /* HIT */
        32,  /* SWING */
        42,  /* STAB */
        57,  /* THRUST */
        70,  /* JAB */
        18,  /* PARRY */
        27,  /* HACK */
        46,  /* BERZERK */
        0,   /* FIREBALL */
        0,   /* DISPELL */
        0,   /* CONFUSE */
        0,   /* LIGHTNING */
        46,  /* DISRUPT */
        64,  /* MELEE */
        0,   /* X */
        0,   /* INVOKE */
        26,  /* SLASH */
        40,  /* CLEAVE */
        32,  /* BASH */
        50,  /* STUN */
        0,   /* SHOOT */
        0,   /* SPELLSHIELD */
        0,   /* FIRESHIELD */
        0,   /* FLUXCAGE */
        0,   /* HEAL */
        0,   /* CALM */
        0,   /* LIGHT */
        0,   /* WINDOW */
        0,   /* SPIT */
        0,   /* BRANDISH */
        0,   /* THROW */
        0 }; /* FUSE */
unsigned char G0494_auc_Graphic560_ActionStamina[44] = {
        0,   /* N */
        4,   /* BLOCK */
        10,  /* CHOP */
        0,   /* X */
        1,   /* BLOW HORN */
        0,   /* FLIP */
        1,   /* PUNCH */
        3,   /* KICK */
        1,   /* WAR CRY */
        3,   /* STAB */
        40,  /* CLIMB DOWN */
        3,   /* FREEZE LIFE */
        3,   /* HIT */
        2,   /* SWING */
        4,   /* STAB */
        17,  /* THRUST */
        3,   /* JAB */
        1,   /* PARRY */
        6,   /* HACK */
        40,  /* BERZERK */
        5,   /* FIREBALL */
        2,   /* DISPELL */
        2,   /* CONFUSE */
        4,   /* LIGHTNING */
        5,   /* DISRUPT */
        25,  /* MELEE */
        1,   /* X */
        2,   /* INVOKE */
        2,   /* SLASH */
        10,  /* CLEAVE */
        9,   /* BASH */
        2,   /* STUN */
        3,   /* SHOOT */
        1,   /* SPELLSHIELD */
        2,   /* FIRESHIELD */
        6,   /* FLUXCAGE */
        1,   /* HEAL */
        1,   /* CALM */
        3,   /* LIGHT */
        2,   /* WINDOW */
        3,   /* SPIT */
        2,   /* BRANDISH */
        0,   /* THROW */
        2 }; /* FUSE */
char G0495_ac_Graphic560_ActionDefense[44] = {
        0,    /* N */
        36,   /* BLOCK */
        0,    /* CHOP */
        0,    /* X */
        -4,   /* BLOW HORN */
        -10,  /* FLIP */
        -10,  /* PUNCH */
        -5,   /* KICK */
        4,    /* WAR CRY */
        -20,  /* STAB */
        -15,  /* CLIMB DOWN */
        -10,  /* FREEZE LIFE */
        16,   /* HIT */
        5,    /* SWING */
        -15,  /* STAB */
        -17,  /* THRUST */
        -5,   /* JAB */
        29,   /* PARRY */
        10,   /* HACK */
        -10,  /* BERZERK */
        -7,   /* FIREBALL */
        -7,   /* DISPELL */
        -7,   /* CONFUSE */
        -7,   /* LIGHTNING */
        -7,   /* DISRUPT */
        -5,   /* MELEE */
        -15,  /* X */
        -9,   /* INVOKE */
        4,    /* SLASH */
        0,    /* CLEAVE */
        0,    /* BASH */
        5,    /* STUN */
        -15,  /* SHOOT */
        -7,   /* SPELLSHIELD */
        -7,   /* FIRESHIELD */
        8,    /* FLUXCAGE */
        -20,  /* HEAL */
        -5,   /* CALM */
        0,    /* LIGHT */
        -15,  /* WINDOW */
        -7,   /* SPIT */
        -4,   /* BRANDISH */
        0,    /* THROW */
        8 };  /* FUSE */
unsigned char G0496_auc_Graphic560_ActionSkillIndex[44] = {
        0,   /* N */
        7,   /* BLOCK */
        6,   /* CHOP */
        0,   /* X */
        14,  /* BLOW HORN */
        12,  /* FLIP */
        9,   /* PUNCH */
        9,   /* KICK */
        7,   /* WAR CRY Atari ST Versions 1.0 1987-12-08 1987-12-11 1.1: 14 */
        9,   /* STAB */
        8,   /* CLIMB DOWN */
        14,  /* FREEZE LIFE */
        9,   /* HIT */
        4,   /* SWING */
        5,   /* STAB */
        5,   /* THRUST */
        5,   /* JAB */
        7,   /* PARRY */
        4,   /* HACK */
        4,   /* BERZERK */
        16,  /* FIREBALL */
        17,  /* DISPELL */
        14,  /* CONFUSE */
        17,  /* LIGHTNING */
        17,  /* DISRUPT */
        6,   /* MELEE */
        8,   /* X */
        3,   /* INVOKE */
        4,   /* SLASH */
        4,   /* CLEAVE */
        6,   /* BASH */
        6,   /* STUN */
        11,  /* SHOOT */
        15,  /* SPELLSHIELD */
        15,  /* FIRESHIELD */
        3,   /* FLUXCAGE */
        13,  /* HEAL */
        14,  /* CALM */
        17,  /* LIGHT */
        18,  /* WINDOW */
        16,  /* SPIT */
        14,  /* BRANDISH */
        10,  /* THROW */
        3 }; /* FUSE */
unsigned char G0497_auc_Graphic560_ActionExperienceGain[44] = {
        0,   /* N */
        8,   /* BLOCK */
        10,  /* CHOP */
        0,   /* X */
        1,   /* BLOW HORN */
        0,   /* FLIP */
        8,   /* PUNCH */
        13,  /* KICK */
        7,   /* WAR CRY */
        15,  /* STAB */
        15,  /* CLIMB DOWN */
        22,  /* FREEZE LIFE */
        10,  /* HIT */
        6,   /* SWING */
        12,  /* STAB */
        19,  /* THRUST */
        11,  /* JAB */
        17,  /* PARRY */
        9,   /* HACK */
        40,  /* BERZERK */
        35,  /* FIREBALL */
        25,  /* DISPELL */
        0,   /* CONFUSE */
        30,  /* LIGHTNING */
        10,  /* DISRUPT */
        24,  /* MELEE */
        0,   /* X */
        25,  /* INVOKE */
        9,   /* SLASH */
        12,  /* CLEAVE */
        11,  /* BASH */
        10,  /* STUN */
        20,  /* SHOOT Atari ST Versions 1.0 1987-12-08 1987-12-11: 9 */
        20,  /* SPELLSHIELD */
        20,  /* FIRESHIELD */
        12,  /* FLUXCAGE */
        5,   /* HEAL */
        1,   /* CALM */
        20,  /* LIGHT */
        30,  /* WINDOW */
        25,  /* SPIT */
        3,   /* BRANDISH */
        5,   /* THROW */
        1 }; /* FUSE */
int16_t G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE;
unsigned int16_t G0506_ui_ActingChampionOrdinal;
unsigned int16_t G0507_ui_ActionCount;
BOOLEAN G0508_B_RefreshActionArea;
BOOLEAN G0509_B_ActionAreaContainsIcons;
ACTION_LIST G0713_s_ActionList;
int16_t G0513_i_ActionDamage;
THING G0517_T_ActionTargetGroupThing;



#ifdef X736_I34M
void F0620_LoadActionNames(
void
)
{
        G0490_ac_Graphic560_ActionNames = (char*)M533_F0468_MEMORY_Allocate((long)(F0494_MEMORY_GetGraphicDecompressedByteCount(C699_GRAPHIC_ACTION_NAMES)), C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
        F0490_MEMORY_LoadDecompressAndExpandGraphic(MASK0x8000_NOT_EXPANDED | MASK0x4000_DO_NOT_COPY_DIMENSIONS | C699_GRAPHIC_ACTION_NAMES, G0490_ac_Graphic560_ActionNames);
}
#endif

void F0381_MENUS_PrintMessageAfterReplacements(
REGISTER char* P0756_pc_String FINAL_SEPARATOR
{
        REGISTER char* L1164_pc_Character;
        REGISTER char* L1165_pc_ReplacementString;
        char L1166_ac_OutputString[100];


        L1164_pc_Character = L1166_ac_OutputString;
        *L1164_pc_Character++ = '\n'; /* New line */
        do {
                if (*P0756_pc_String == '@') {
                        P0756_pc_String++;
                        if (*(L1164_pc_Character - 1) != '\n') { /* New line */
                                *L1164_pc_Character++ = ' ';
                        }
                        if (*P0756_pc_String == 'p') { /* '@p' in the source string is replaced by the champion name followed by a space */
                                        L1165_pc_ReplacementString = M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal)].Name;
                        }
                        *L1164_pc_Character = '\0';
                        M545_STRCAT(L1166_ac_OutputString, L1165_pc_ReplacementString); /* BUG0_01 Coding error without consequence. If '@' is present in string to print but not followed by 'p' then L1165_pc_ReplacementString is not initialized and a random string is printed. No consequence because this function is never with a string containing '@' */
                        L1164_pc_Character += M544_STRLEN(L1165_pc_ReplacementString);
                        *L1164_pc_Character++ = ' ';
                } else {
                        *L1164_pc_Character++ = *P0756_pc_String;
                }
        } while (*P0756_pc_String++);
        *L1164_pc_Character = '\0';
        if (L1166_ac_OutputString[1]) { /* If the string is not empty (the first character is a new line \n) */
                F0047_TEXT_MESSAGEAREA_PrintMessage(C04_COLOR_CYAN, L1166_ac_OutputString);
        }
}

STATICFUNCTION int16_t F0382_MENUS_GetActionObjectChargeCount(
void
)
{
        REGISTER JUNK* L1168_ps_Junk;
        REGISTER THING L1167_T_Thing;


        L1168_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1167_T_Thing = M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal)].Slots[C01_SLOT_ACTION_HAND]);
        switch (M012_TYPE(L1167_T_Thing)) {
                case C05_THING_TYPE_WEAPON:
                        return ((WEAPON*)L1168_ps_Junk)->ChargeCount;
                case C06_THING_TYPE_ARMOUR:
                        return ((ARMOUR*)L1168_ps_Junk)->ChargeCount;
                case C10_THING_TYPE_JUNK:
                        return L1168_ps_Junk->ChargeCount;
                default:
                        return 1;
        }
}

STATICFUNCTION void F0383_MENUS_SetActionList(
REGISTER ACTION_SET* P0757_ps_ActionSet FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L1169_ui_ActionListIndex;
        REGISTER unsigned int16_t L1170_ui_NextAvailableActionListIndex;
        REGISTER unsigned int16_t L1172_ui_MinimumSkillLevel;
        REGISTER unsigned int16_t L1171_ui_ActionIndex;


        G0713_s_ActionList.ActionIndices[0] = P0757_ps_ActionSet->ActionIndices[0];
        G0713_s_ActionList.MinimumSkillLevel[0] = 1;
        L1170_ui_NextAvailableActionListIndex = 1;
        for (L1169_ui_ActionListIndex = 1; L1169_ui_ActionListIndex < 3; L1169_ui_ActionListIndex++) {
                if ((L1171_ui_ActionIndex = P0757_ps_ActionSet->ActionIndices[L1169_ui_ActionListIndex]) == C0xFF_ACTION_NONE)
                        continue;
                if (M007_GET(L1172_ui_MinimumSkillLevel = P0757_ps_ActionSet->ActionProperties[L1169_ui_ActionListIndex - 1], MASK0x0080_ACTION_REQUIRES_CHARGE) && !F0382_MENUS_GetActionObjectChargeCount())
                        continue;
                M009_CLEAR(L1172_ui_MinimumSkillLevel, MASK0x0080_ACTION_REQUIRES_CHARGE);
                if (F0303_CHAMPION_GetSkillLevel(M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal), G0496_auc_Graphic560_ActionSkillIndex[L1171_ui_ActionIndex]) >= L1172_ui_MinimumSkillLevel) {
                        G0713_s_ActionList.ActionIndices[L1170_ui_NextAvailableActionListIndex] = L1171_ui_ActionIndex;
                        G0713_s_ActionList.MinimumSkillLevel[L1170_ui_NextAvailableActionListIndex] = L1172_ui_MinimumSkillLevel;
                        L1170_ui_NextAvailableActionListIndex++;
                }
        }
        G0507_ui_ActionCount = L1170_ui_NextAvailableActionListIndex;
        for (L1169_ui_ActionListIndex = L1170_ui_NextAvailableActionListIndex; L1169_ui_ActionListIndex < 3; L1169_ui_ActionListIndex++) {
                G0713_s_ActionList.ActionIndices[L1169_ui_ActionListIndex] = C0xFF_ACTION_NONE;
        }
}



void F0388_MENUS_ClearActingChampion(
void
)
{
        if (G0506_ui_ActingChampionOrdinal) {
                M008_SET(M516_CHAMPIONS[--G0506_ui_ActingChampionOrdinal].Attributes, MASK0x8000_ACTION_HAND);
                F0292_CHAMPION_DrawState(G0506_ui_ActingChampionOrdinal);
                G0506_ui_ActingChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);
                G0508_B_RefreshActionArea = C1_TRUE;
        }
}

void F0389_MENUS_ProcessCommands116To119_SetActingChampion(
unsigned int16_t P0761_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1190_ps_Champion;
        REGISTER ACTION_SET* L1191_ps_ActionSet;
        REGISTER unsigned int16_t L1188_ui_ActionSetIndex;
        REGISTER THING L1189_T_Thing;


        L1190_ps_Champion = &M516_CHAMPIONS[P0761_ui_ChampionIndex];
        if (M007_GET(L1190_ps_Champion->Attributes, MASK0x0008_DISABLE_ACTION) || !L1190_ps_Champion->CurrentHealth) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                return;
        }
        if ((L1189_T_Thing = L1190_ps_Champion->Slots[C01_SLOT_ACTION_HAND]) == C0xFFFF_THING_NONE) {
                L1188_ui_ActionSetIndex = 2; /* Actions Punck, Kick and War Cry */
        } else {
                if ((L1188_ui_ActionSetIndex = G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(L1189_T_Thing)].ActionSetIndex) == 0) {
                        return;
                }
        }
        L1191_ps_ActionSet = &G0489_as_Graphic560_ActionSets[L1188_ui_ActionSetIndex];
        G0506_ui_ActingChampionOrdinal = M000_INDEX_TO_ORDINAL(P0761_ui_ChampionIndex);
        G0713_s_ActionList.Useless = L1191_ps_ActionSet->Useless; /* BUG0_00 Useless code */
        F0383_MENUS_SetActionList(L1191_ps_ActionSet);
        G0509_B_ActionAreaContainsIcons = C0_FALSE;
        M008_SET(L1190_ps_Champion->Attributes, MASK0x8000_ACTION_HAND);
        F0292_CHAMPION_DrawState(P0761_ui_ChampionIndex);
        F0387_MENUS_DrawActionArea();
}

void F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived(
void
)
{
        REGISTER CHAMPION* L1195_ps_Champion;
        REGISTER int16_t L1192_i_ChampionIndex;


        if (!G0305_ui_PartyChampionCount) {
                return;
        }
        if (G0300_B_PartyIsResting || G0299_ui_CandidateChampionOrdinal) {
                if (G0506_ui_ActingChampionOrdinal) {
                        F0388_MENUS_ClearActingChampion();
                        goto T0390012;
                }
                else {
                        if (!G0299_ui_CandidateChampionOrdinal)
                                goto T0390012;
                }
        } else {
                L1195_ps_Champion = M516_CHAMPIONS;
                L1192_i_ChampionIndex = C00_CHAMPION_FIRST;
                do {
                        if ((L1192_i_ChampionIndex != G0411_i_LeaderIndex) && (M000_INDEX_TO_ORDINAL(L1192_i_ChampionIndex) != G0506_ui_ActingChampionOrdinal) && L1195_ps_Champion->MaximumDamageReceived && (L1195_ps_Champion->Direction != L1195_ps_Champion->DirectionMaximumDamageReceived)) {
                                L1195_ps_Champion->Direction = L1195_ps_Champion->DirectionMaximumDamageReceived;
                                M008_SET(L1195_ps_Champion->Attributes, MASK0x0400_ICON);
                                F0292_CHAMPION_DrawState(L1192_i_ChampionIndex);
                        }
                        L1195_ps_Champion->MaximumDamageReceived = 0;
                        L1195_ps_Champion++;
                } while (++L1192_i_ChampionIndex < G0305_ui_PartyChampionCount);
        }
        if (G0508_B_RefreshActionArea) {
                if (!G0506_ui_ActingChampionOrdinal) {
                        if (G0513_i_ActionDamage) {
                                F0385_MENUS_DrawActionDamage(G0513_i_ActionDamage);
                                G0513_i_ActionDamage = 0;
                        } else {
                                G0509_B_ActionAreaContainsIcons = C1_TRUE;
#ifdef PC_FIX_CODE_SIZE
        L1192_i_ChampionIndex++;
        L1192_i_ChampionIndex++;
        L1192_i_ChampionIndex++;
        L1192_i_ChampionIndex++;
        L1192_i_ChampionIndex++;
#endif
                                F0387_MENUS_DrawActionArea();
                        }
                } else {
                        G0509_B_ActionAreaContainsIcons = C0_FALSE;
                        M008_SET(L1195_ps_Champion->Attributes, MASK0x8000_ACTION_HAND);
                        F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal));
                        F0387_MENUS_DrawActionArea();
                }
        }
        T0390012: ;
}

BOOLEAN F0391_MENUS_DidClickTriggerAction(
int16_t P0762_i_ActionListIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1199_ps_Champion;
        REGISTER unsigned int16_t L1197_ui_ActionIndex;
        REGISTER BOOLEAN L1198_B_ClickTriggeredAction;
        REGISTER unsigned int16_t L1196_ui_ChampionIndex;


        if (!G0506_ui_ActingChampionOrdinal || (G0713_s_ActionList.ActionIndices[P0762_i_ActionListIndex] == C0xFF_ACTION_NONE)) { /* BUG0_01 Coding error without consequence. Out of bounds array index when called with P0762_i_ActionListIndex = -1. No consequence because this accesses G0713_s_ActionList.Useless which cannot have value 255 */
                return C0_FALSE;
        }
        L1199_ps_Champion = &M516_CHAMPIONS[L1196_ui_ChampionIndex = M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal)];
        if (P0762_i_ActionListIndex == -1) {
                L1198_B_ClickTriggeredAction = 0;
        } else {
                L1197_ui_ActionIndex = G0713_s_ActionList.ActionIndices[P0762_i_ActionListIndex];
                L1199_ps_Champion->ActionDefense += G0495_ac_Graphic560_ActionDefense[L1197_ui_ActionIndex]; /* BUG0_54 The defense modifier of an action is permanent.
                Each action has an associated defense modifier value and a number of ticks while the champion cannot perform another action because the action icon is grayed out. If an action has a non zero defense modifier and a zero value for the number of ticks then the defense modifier is applied but it is never removed. This causes no issue in the original games because there are no actions in this case but it may occur in a version where data is customized. This statement should only be executed if the value for the action in G0491_auc_Graphic560_ActionDisabledTicks is not 0 otherwise the action is not disabled at the end of F0407_MENUS_IsActionPerformed and thus not enabled later in F0253_TIMELINE_ProcessEvent11Part1_EnableChampionAction where the defense modifier is also removed */
                M008_SET(L1199_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                L1198_B_ClickTriggeredAction = F0407_MENUS_IsActionPerformed(L1196_ui_ChampionIndex, L1197_ui_ActionIndex);
                L1199_ps_Champion->ActionIndex = L1197_ui_ActionIndex;
        }
        F0388_MENUS_ClearActingChampion();
        return L1198_B_ClickTriggeredAction;
}


STATICFUNCTION BOOLEAN F0401_MENUS_IsGroupFrightenedByAction(
int16_t          P0769_i_ChampionIndex SEPARATOR
unsigned int16_t P0770_ui_ActionIndex  SEPARATOR
int16_t          P0771_i_MapX          SEPARATOR
int16_t          P0772_i_MapY          FINAL_SEPARATOR
{
        REGISTER GROUP* L1233_ps_Group;
        REGISTER CREATURE_INFO* L1234_ps_CreatureInfo;
        REGISTER ACTIVE_GROUP* L1235_ps_ActiveGroup;
        REGISTER unsigned int16_t L1229_ui_FrightAmount;
        REGISTER unsigned int16_t L1231_ui_Experience;
        REGISTER unsigned int16_t L1230_ui_FearResistance;
        REGISTER BOOLEAN L1232_B_IsGroupFrightenedByAction;


        L1232_B_IsGroupFrightenedByAction = C0_FALSE;
        if (G0517_T_ActionTargetGroupThing == C0xFFFE_THING_ENDOFLIST)
                goto T0401016;
        switch (P0770_ui_ActionIndex) {
                case C008_ACTION_WAR_CRY:
                        L1229_ui_FrightAmount = 3;
                        L1231_ui_Experience = 12; /* War Cry gives experience in priest skill C14_SKILL_INFLUENCE below. The War Cry action also has an experience gain of 7 defined in G0497_auc_Graphic560_ActionExperienceGain in the same skill (versions 1.1 and below) or in the fighter skill C07_SKILL_PARRY (versions 1.2 and above). In versions 1.2 and above, this is the only action that gives experience in two skills */
                        break;
                case C037_ACTION_CALM:
                        L1229_ui_FrightAmount = 7;
                        L1231_ui_Experience = 35;
                        break;
                case C041_ACTION_BRANDISH:
                        L1229_ui_FrightAmount = 6;
                        L1231_ui_Experience = 30;
                        break;
                case C004_ACTION_BLOW_HORN:
                        L1229_ui_FrightAmount = 6;
                        L1231_ui_Experience = 20;
                        break;
                case C022_ACTION_CONFUSE:
                        L1229_ui_FrightAmount = 12;
                        L1231_ui_Experience = 45;
        }
        L1229_ui_FrightAmount += F0303_CHAMPION_GetSkillLevel(P0769_i_ChampionIndex, C14_SKILL_INFLUENCE);
        L1233_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(G0517_T_ActionTargetGroupThing);
        L1234_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[L1233_ps_Group->Type];
        if (((L1230_ui_FearResistance = M057_FEAR_RESISTANCE(L1234_ps_CreatureInfo->Properties)) > M002_RANDOM(L1229_ui_FrightAmount)) || (L1230_ui_FearResistance == C15_IMMUNE_TO_FEAR)) {
                L1231_ui_Experience >>= 1;
        } else {
                L1235_ps_ActiveGroup = &G0375_ps_ActiveGroups[L1233_ps_Group->ActiveGroupIndex];
                if (L1233_ps_Group->Behavior == C6_BEHAVIOR_ATTACK) {
                        F0182_GROUP_StopAttacking(L1235_ps_ActiveGroup, P0771_i_MapX, P0772_i_MapY);
                        F0180_GROUP_StartWandering(P0771_i_MapX, P0772_i_MapY);
                }
                L1233_ps_Group->Behavior = C5_BEHAVIOR_FLEE;
                L1235_ps_ActiveGroup->DelayFleeingFromTarget = ((16 - L1230_ui_FearResistance) << 2) / L1234_ps_CreatureInfo->MovementTicks;
                L1232_B_IsGroupFrightenedByAction = C1_TRUE;
        }
        F0304_CHAMPION_AddSkillExperience(P0769_i_ChampionIndex, C14_SKILL_INFLUENCE, L1231_ui_Experience);
        T0401016:
        return L1232_B_IsGroupFrightenedByAction;
}

STATICFUNCTION BOOLEAN F0402_MENUS_IsMeleeActionPerformed(
unsigned int16_t   P0773_i_ChampionIndex SEPARATOR
REGISTER int16_t   P0775_i_ActionIndex   SEPARATOR
int16_t            P0776_i_TargetMapX    SEPARATOR
int16_t            P0777_i_TargetMapY    SEPARATOR
int16_t            P0778_i_SkillIndex    FINAL_SEPARATOR
{
        REGISTER CHAMPION* P0774_ps_Champion = &M516_CHAMPIONS[P0773_i_ChampionIndex];
        REGISTER unsigned int16_t L1236_ui_Multiple;
#define AL1236_ui_ChampionCell       L1236_ui_Multiple
#define AL1236_ui_ActionDamageFactor L1236_ui_Multiple
        REGISTER unsigned int16_t L1237_ui_Multiple;
#define AL1237_ui_Direction            L1237_ui_Multiple
#define AL1237_ui_CellDelta            L1237_ui_Multiple
#define AL1237_ui_ActionHitProbability L1237_ui_Multiple
        REGISTER int16_t L1238_i_CreatureOrdinal;


        F0064_SOUND_RequestPlay_CPSD(M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, G0306_i_PartyMapX, G0307_i_PartyMapY, C01_MODE_PLAY_IF_PRIORITIZED);
        if (G0517_T_ActionTargetGroupThing == C0xFFFE_THING_ENDOFLIST)
                goto T0402010;
        if (L1238_i_CreatureOrdinal = F0177_GROUP_GetMeleeTargetCreatureOrdinal(P0776_i_TargetMapX, P0777_i_TargetMapY, G0306_i_PartyMapX, G0307_i_PartyMapY, AL1236_ui_ChampionCell = P0774_ps_Champion->Cell)) {
                switch (M021_NORMALIZE(AL1236_ui_ChampionCell + 4 - P0774_ps_Champion->Direction)) {
                        case C02_VIEW_CELL_BACK_RIGHT: /* Champion is on the back right of the square and tries to attack a creature in the front right of its square */
                                AL1237_ui_CellDelta = 3;
                                goto T0402005;
                        case C03_VIEW_CELL_BACK_LEFT: /* Champion is on the back left of the square and tries to attack a creature in the front left of its square */
                                AL1237_ui_CellDelta = 1;
                                T0402005: /* Check if there is another champion in front */
                                if (F0285_CHAMPION_GetIndexInCell(M021_NORMALIZE(AL1236_ui_ChampionCell + AL1237_ui_CellDelta)) != CM1_CHAMPION_NONE) {
                                        G0513_i_ActionDamage = CM1_DAMAGE_CANT_REACH;
                                        goto T0402010;
                                }
                }
                if ((P0775_i_ActionIndex == C024_ACTION_DISRUPT) && !M007_GET(F0144_DUNGEON_GetCreatureAttributes(G0517_T_ActionTargetGroupThing), MASK0x0040_NON_MATERIAL))
                        goto T0402010;
                AL1237_ui_ActionHitProbability = G0493_auc_Graphic560_ActionHitProbability[P0775_i_ActionIndex];
                AL1236_ui_ActionDamageFactor = G0492_auc_Graphic560_ActionDamageFactor[P0775_i_ActionIndex];
                if ((F0033_OBJECT_GetIconIndex(P0774_ps_Champion->Slots[C01_SLOT_ACTION_HAND]) == C040_ICON_WEAPON_VORPAL_BLADE) || (P0775_i_ActionIndex == C024_ACTION_DISRUPT)) {
                        M008_SET(AL1237_ui_ActionHitProbability, MASK0x8000_HIT_NON_MATERIAL_CREATURES);
                }
                G0513_i_ActionDamage = F0231_GROUP_GetMeleeActionDamage(P0774_ps_Champion, P0773_i_ChampionIndex, (GROUP*)F0156_DUNGEON_GetThingData(G0517_T_ActionTargetGroupThing), M001_ORDINAL_TO_INDEX(L1238_i_CreatureOrdinal), P0776_i_TargetMapX, P0777_i_TargetMapY, AL1237_ui_ActionHitProbability, AL1236_ui_ActionDamageFactor, P0778_i_SkillIndex);
                return C1_TRUE;
        }
        T0402010:
        return C0_FALSE;
}

STATICFUNCTION BOOLEAN F0403_MENUS_IsPartySpellOrFireShieldSuccessful(
REGISTER CHAMPION*        P0779_ps_Champion   SEPARATOR
BOOLEAN                   P0780_B_SpellShield SEPARATOR
REGISTER unsigned int16_t P0781_ui_Ticks      SEPARATOR
BOOLEAN                   P0782_B_UseMana     FINAL_SEPARATOR
{
        REGISTER BOOLEAN L1239_B_IsPartySpellOrFireShieldSuccessful;
        EVENT L1240_s_Event;


        L1239_B_IsPartySpellOrFireShieldSuccessful = C1_TRUE;
        if (P0782_B_UseMana) {
                if (P0779_ps_Champion->CurrentMana == 0) {
                        return C0_FALSE;
                }
                else {
                        if (P0779_ps_Champion->CurrentMana < 4) {
                                P0781_ui_Ticks >>= 1;
                                P0779_ps_Champion->CurrentMana = 0;
                                L1239_B_IsPartySpellOrFireShieldSuccessful = C0_FALSE;
                        } else {
                                P0779_ps_Champion->CurrentMana -= 4;
                        }
                }
        }
        L1240_s_Event.B.Defense = P0781_ui_Ticks >> 5;
        if (P0780_B_SpellShield) {
                L1240_s_Event.A.A.Type = C77_EVENT_SPELLSHIELD;
                if (G0407_s_Party.SpellShieldDefense > 50) {
                        L1240_s_Event.B.Defense >>= 2;
                }
                G0407_s_Party.SpellShieldDefense += L1240_s_Event.B.Defense;
        } else {
                L1240_s_Event.A.A.Type = C78_EVENT_FIRESHIELD;
                if (G0407_s_Party.FireShieldDefense > 50) {
                        L1240_s_Event.B.Defense >>= 2;
                }
                G0407_s_Party.FireShieldDefense += L1240_s_Event.B.Defense;
        }
        L1240_s_Event.A.A.Priority = 0;
        M033_SET_MAP_AND_TIME(L1240_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + P0781_ui_Ticks);
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1240_s_Event);
        F0293_CHAMPION_DrawAllChampionStates(MASK0x1000_STATUS_BOX);
        return L1239_B_IsPartySpellOrFireShieldSuccessful;
}

STATICFUNCTION void F0404_MENUS_CreateEvent70_Light(
int16_t P0783_i_LightPower SEPARATOR
int16_t P0784_i_Ticks      FINAL_SEPARATOR
{
        EVENT L1241_s_Event;


        L1241_s_Event.A.A.Type = C70_EVENT_LIGHT;
        L1241_s_Event.B.LightPower = P0783_i_LightPower;
        M033_SET_MAP_AND_TIME(L1241_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + P0784_i_Ticks);
        L1241_s_Event.A.A.Priority = 0;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1241_s_Event);
        F0337_INVENTORY_SetDungeonViewPalette();
}

STATICFUNCTION void F0405_MENUS_DecrementCharges(
CHAMPION* P0785_ps_Champion FINAL_SEPARATOR
{
        REGISTER JUNK* L1243_ps_Junk;
        REGISTER THING L1242_T_Thing;


        L1243_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1242_T_Thing = P0785_ps_Champion->Slots[C01_SLOT_ACTION_HAND]);
        switch (M012_TYPE(L1242_T_Thing)) {
                case C05_THING_TYPE_WEAPON:
                        if (((WEAPON*)L1243_ps_Junk)->ChargeCount) {
#ifdef PC_FIX_CODE_SIZE
        L1242_T_Thing++;
        L1242_T_Thing++;
        L1242_T_Thing++;
        L1242_T_Thing++;
#endif
                                ((WEAPON*)L1243_ps_Junk)->ChargeCount--;
                        }
                        break;
                case C06_THING_TYPE_ARMOUR:
                        if (((ARMOUR*)L1243_ps_Junk)->ChargeCount) {
#ifdef PC_FIX_CODE_SIZE
        L1242_T_Thing++;
        L1242_T_Thing++;
        L1242_T_Thing++;
        L1242_T_Thing++;
#endif
                                ((ARMOUR*)L1243_ps_Junk)->ChargeCount--;
                        }
                        break;
                case C10_THING_TYPE_JUNK:
                        if (L1243_ps_Junk->ChargeCount) {
                                L1243_ps_Junk->ChargeCount--;
                        }
        }
        F0296_CHAMPION_DrawChangedObjectIcons();
}

STATICFUNCTION void F0406_MENUS_SetChampionDirectionToPartyDirection(
REGISTER CHAMPION* P0786_ps_Champion FINAL_SEPARATOR
{
        if (P0786_ps_Champion->Direction != G0308_i_PartyDirection) {
                P0786_ps_Champion->Direction = G0308_i_PartyDirection;
                M008_SET(P0786_ps_Champion->Attributes, MASK0x0400_ICON);
        }
}

STATICFUNCTION BOOLEAN F0407_MENUS_IsActionPerformed(
REGISTER unsigned int16_t P0787_ui_ChampionIndex SEPARATOR
int16_t                   P0788_i_ActionIndex    FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1247_ps_Champion;
        REGISTER WEAPON* L1248_ps_Weapon;
#define AL1244_ui_TargetSquare  L1244_ui_Multiple
#define AL1244_ui_ManaCost      L1244_ui_Multiple
        REGISTER WEAPON_INFO* L1256_ps_WeaponInfoActionHand;
        WEAPON_INFO* L1257_ps_WeaponInfoReadyHand;
        REGISTER int16_t L1246_i_Multiple;
#define AL1992_HealingAmount     L1245_i_Multiple
#define AL1246_i_RequiredManaAmount    L1246_i_Multiple
#define AL1246_i_ActionHandWeaponClass L1246_i_Multiple
#define AL1246_i_StepEnergy            L1246_i_Multiple
#define AL1246_i_HealingCapability     L1246_i_Multiple
#define AL1246_i_Ticks                 L1246_i_Multiple
        REGISTER int16_t L1245_i_Multiple;
#define AL1245_T_ExplosionThing  L1245_i_Multiple
#define AL1245_B_ActionPerformed L1245_i_Multiple
#define AL1250_i_KineticEnergy        L1250_i_Multiple
#define AL1250_i_ReadyHandWeaponClass L1250_i_Multiple
#define AL1250_T_Object               L1250_i_Multiple
#define AL1250_i_MissingHealth        L1250_i_Multiple
#define AL1250_i_HealingAmount        L1250_i_Multiple
        REGISTER int16_t L1250_i_Multiple;
        REGISTER int16_t L1251_i_MapX;
        int16_t L1252_i_MapY;
        REGISTER unsigned int16_t L1244_ui_Multiple;
        unsigned int16_t L1249_ui_ActionDisabledTicks;
        int16_t L1253_i_ActionStamina;
        int16_t L1254_i_ActionSkillIndex;
        int16_t L1255_i_ActionExperienceGain;
        EVENT L1258_s_Event;


        if (P0787_ui_ChampionIndex >= G0305_ui_PartyChampionCount) {
                return C0_FALSE;
        }
        L1247_ps_Champion = &M516_CHAMPIONS[P0787_ui_ChampionIndex];
        L1248_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(L1247_ps_Champion->Slots[C01_SLOT_ACTION_HAND]);
        if (!L1247_ps_Champion->CurrentHealth) {
                return C0_FALSE;
        }
        L1251_i_MapX = G0306_i_PartyMapX;
        L1252_i_MapY = G0307_i_PartyMapY;
        L1251_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[L1247_ps_Champion->Direction], L1252_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[L1247_ps_Champion->Direction];
        G0517_T_ActionTargetGroupThing = F0175_GROUP_GetThing(L1251_i_MapX, L1252_i_MapY);
        L1249_ui_ActionDisabledTicks = G0491_auc_Graphic560_ActionDisabledTicks[P0788_i_ActionIndex];
        L1254_i_ActionSkillIndex = G0496_auc_Graphic560_ActionSkillIndex[P0788_i_ActionIndex];
        L1253_i_ActionStamina = G0494_auc_Graphic560_ActionStamina[P0788_i_ActionIndex] + M005_RANDOM(2);
        L1255_i_ActionExperienceGain = G0497_auc_Graphic560_ActionExperienceGain[P0788_i_ActionIndex];
        AL1244_ui_TargetSquare = F0151_DUNGEON_GetSquare(L1251_i_MapX, L1252_i_MapY);
        AL1245_B_ActionPerformed = C1_TRUE;
        if (((L1254_i_ActionSkillIndex >= C16_SKILL_FIRE) && (L1254_i_ActionSkillIndex <= C19_SKILL_WATER)) || (L1254_i_ActionSkillIndex == C03_SKILL_WIZARD)) {
                AL1246_i_RequiredManaAmount = 7 - F0024_MAIN_GetMinimumValue(6, F0303_CHAMPION_GetSkillLevel(P0787_ui_ChampionIndex, L1254_i_ActionSkillIndex));
        }
        switch (P0788_i_ActionIndex) {
                case C023_ACTION_LIGHTNING:
                        AL1250_i_KineticEnergy = 180;
                        AL1245_T_ExplosionThing = C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT;
                        goto T0407014;
                case C021_ACTION_DISPELL:
                        AL1250_i_KineticEnergy = 150;
                        AL1245_T_ExplosionThing = C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL;
                        goto T0407014;
                case C020_ACTION_FIREBALL:
                        AL1250_i_KineticEnergy = 150;
                        goto T0407013;
                case C040_ACTION_SPIT:
                        AL1250_i_KineticEnergy = 250;
                        T0407013:
                        AL1245_T_ExplosionThing = C0xFF80_THING_EXPLOSION_FIREBALL;
                        T0407014:
                        F0406_MENUS_SetChampionDirectionToPartyDirection(L1247_ps_Champion);
                        if (L1247_ps_Champion->CurrentMana < AL1246_i_RequiredManaAmount) {
                                AL1250_i_KineticEnergy = F0025_MAIN_GetMaximumValue(2, L1247_ps_Champion->CurrentMana * AL1250_i_KineticEnergy / AL1246_i_RequiredManaAmount);
                                AL1246_i_RequiredManaAmount = L1247_ps_Champion->CurrentMana;
                        }
                        if (!(AL1245_B_ActionPerformed = F0327_CHAMPION_IsProjectileSpellCast(P0787_ui_ChampionIndex, AL1245_T_ExplosionThing, AL1250_i_KineticEnergy, AL1246_i_RequiredManaAmount))) {
                                L1255_i_ActionExperienceGain >>= 1;
                        }
                        F0405_MENUS_DecrementCharges(L1247_ps_Champion);
                        break;
                case C030_ACTION_BASH:
                case C018_ACTION_HACK:
                case C019_ACTION_BERZERK:
                case C007_ACTION_KICK:
                case C013_ACTION_SWING:
                case C002_ACTION_CHOP:
                        if ((M034_SQUARE_TYPE(AL1244_ui_TargetSquare) == C04_ELEMENT_DOOR) && (M036_DOOR_STATE(AL1244_ui_TargetSquare) == C4_DOOR_STATE_CLOSED)) {
                                F0064_SOUND_RequestPlay_CPSD(M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, G0306_i_PartyMapX, G0307_i_PartyMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                L1249_ui_ActionDisabledTicks = 6;
                                F0232_GROUP_IsDoorDestroyedByAttack(L1251_i_MapX, L1252_i_MapY, F0312_CHAMPION_GetStrength(P0787_ui_ChampionIndex, C01_SLOT_ACTION_HAND), C0_FALSE, 2);
                                F0064_SOUND_RequestPlay_CPSD(C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM, G0306_i_PartyMapX, G0307_i_PartyMapY, C02_MODE_PLAY_ONE_TICK_LATER);
                                break;
                        }
                case C024_ACTION_DISRUPT:
                case C016_ACTION_JAB:
                case C017_ACTION_PARRY:
                case C014_ACTION_STAB:
                case C009_ACTION_STAB:
                case C031_ACTION_STUN:
                case C015_ACTION_THRUST:
                case C025_ACTION_MELEE:
                case C028_ACTION_SLASH:
                case C029_ACTION_CLEAVE:
                case C006_ACTION_PUNCH:
                        if (!(AL1245_B_ActionPerformed = F0402_MENUS_IsMeleeActionPerformed(P0787_ui_ChampionIndex, P0788_i_ActionIndex, L1251_i_MapX, L1252_i_MapY, L1254_i_ActionSkillIndex))) {
                                L1255_i_ActionExperienceGain >>= 1;
#ifdef PC_FIX_CODE_SIZE
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
#endif
                                L1249_ui_ActionDisabledTicks >>= 1;
                        }
                        break;
                case C022_ACTION_CONFUSE:
                        F0405_MENUS_DecrementCharges(L1247_ps_Champion);
                case C008_ACTION_WAR_CRY:
                case C037_ACTION_CALM:
                case C041_ACTION_BRANDISH:
                case C004_ACTION_BLOW_HORN:
                        if (P0788_i_ActionIndex == C008_ACTION_WAR_CRY) {
                                F0064_SOUND_RequestPlay_CPSD(M619_SOUND_WAR_CRY, L1251_i_MapX, L1252_i_MapY, C00_MODE_PLAY_IMMEDIATELY);
                        }
                        if (P0788_i_ActionIndex == C004_ACTION_BLOW_HORN) {
                                F0064_SOUND_RequestPlay_CPSD(M620_SOUND_BLOW_HORN, L1251_i_MapX, L1252_i_MapY, C00_MODE_PLAY_IMMEDIATELY);
                        }
                        AL1245_B_ActionPerformed = F0401_MENUS_IsGroupFrightenedByAction(P0787_ui_ChampionIndex, P0788_i_ActionIndex, L1251_i_MapX, L1252_i_MapY);
                        break;
                case C032_ACTION_SHOOT:
                        if (M012_TYPE(L1247_ps_Champion->Slots[C00_SLOT_READY_HAND]) != C05_THING_TYPE_WEAPON)
                                goto T0407032;
                        L1256_ps_WeaponInfoActionHand = &G0238_as_Graphic559_WeaponInfo[L1248_ps_Weapon->Type];
                        L1257_ps_WeaponInfoReadyHand = F0158_DUNGEON_GetWeaponInfo(L1247_ps_Champion->Slots[C00_SLOT_READY_HAND]);
                        AL1246_i_ActionHandWeaponClass = L1256_ps_WeaponInfoActionHand->Class;
                        AL1250_i_ReadyHandWeaponClass = L1257_ps_WeaponInfoReadyHand->Class;
                        if ((AL1246_i_ActionHandWeaponClass >= C016_CLASS_FIRST_BOW) && (AL1246_i_ActionHandWeaponClass <= C031_CLASS_LAST_BOW)) {
                                if (AL1250_i_ReadyHandWeaponClass != C010_CLASS_BOW_AMMUNITION)
                                        goto T0407032;
                                AL1246_i_StepEnergy -= C016_CLASS_FIRST_BOW;
                        } else {
                                if ((AL1246_i_ActionHandWeaponClass >= C032_CLASS_FIRST_SLING) && (AL1246_i_ActionHandWeaponClass <= C047_CLASS_LAST_SLING)) {
                                        if (AL1250_i_ReadyHandWeaponClass != C011_CLASS_SLING_AMMUNITION) {
                                                T0407032:
                                                G0513_i_ActionDamage = CM2_DAMAGE_NO_AMMUNITION;
                                                L1255_i_ActionExperienceGain = 0;
                                                AL1245_B_ActionPerformed = C0_FALSE;
                                                break;
                                        }
                                        AL1246_i_StepEnergy -= C032_CLASS_FIRST_SLING;
                                }
                        }
                        F0406_MENUS_SetChampionDirectionToPartyDirection(L1247_ps_Champion);
                        AL1250_T_Object = F0300_CHAMPION_GetObjectRemovedFromSlot(P0787_ui_ChampionIndex, C00_SLOT_READY_HAND);
                        F0064_SOUND_RequestPlay_CPSD(M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT, G0306_i_PartyMapX, G0307_i_PartyMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                        F0326_CHAMPION_ShootProjectile(L1247_ps_Champion, AL1250_T_Object, L1256_ps_WeaponInfoActionHand->KineticEnergy + L1257_ps_WeaponInfoReadyHand->KineticEnergy, (M065_SHOOT_ATTACK(L1256_ps_WeaponInfoActionHand->Attributes) + F0303_CHAMPION_GetSkillLevel(P0787_ui_ChampionIndex, C11_SKILL_SHOOT)) << 1, AL1246_i_StepEnergy);
                        break;
                case C005_ACTION_FLIP:
                        if (M005_RANDOM(2)) {
#ifdef PC_FIX_CODE_SIZE
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
#endif
#ifdef X196_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                F0381_MENUS_PrintMessageAfterReplacements("IT COMES UP HEADS.");
#endif
#ifdef X736_I34M
#ifdef PC_FIX_CODE_SIZE
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
#endif
                                F0381_MENUS_PrintMessageAfterReplacements(F0758_TranslateLanguage(C69_IT_COMES_UP_HEADS));
#endif
                        } else {
#ifdef PC_FIX_CODE_SIZE
        L1246_i_Multiple++;
        L1246_i_Multiple++;
#endif
#ifdef X196_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                F0381_MENUS_PrintMessageAfterReplacements("IT COMES UP TAILS.");
#endif
#ifdef X736_I34M
                                F0381_MENUS_PrintMessageAfterReplacements(F0758_TranslateLanguage(C70_IT_COMES_UP_TAILS));
#endif
                        }
                        break;
                case C033_ACTION_SPELLSHIELD:
                case C034_ACTION_FIRESHIELD:
                        if (!F0403_MENUS_IsPartySpellOrFireShieldSuccessful(L1247_ps_Champion, P0788_i_ActionIndex == C033_ACTION_SPELLSHIELD, 280, C1_TRUE)) {
                                L1255_i_ActionExperienceGain >>= 2;
                                L1249_ui_ActionDisabledTicks >>= 1;
                        } else {
#ifdef PC_FIX_CODE_SIZE
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
        L1246_i_Multiple++;
#endif
                                F0405_MENUS_DecrementCharges(L1247_ps_Champion);
                        }
                        break;
                case C027_ACTION_INVOKE:
                        AL1250_i_KineticEnergy = M003_RANDOM(128) + 100;
                        switch (M002_RANDOM(6)) {
                                case 0:
                                        AL1245_T_ExplosionThing = C0xFF86_THING_EXPLOSION_POISON_BOLT;
                                        goto T0407014;
                                case 1:
                                        AL1245_T_ExplosionThing = C0xFF87_THING_EXPLOSION_POISON_CLOUD;
                                        goto T0407014;
                                case 2:
                                        AL1245_T_ExplosionThing = C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL;
                                        goto T0407014;
                                default:
                                        goto T0407013;
                        }
                case C035_ACTION_FLUXCAGE:
                        F0406_MENUS_SetChampionDirectionToPartyDirection(L1247_ps_Champion);
                        F0224_GROUP_FluxCageAction(L1251_i_MapX, L1252_i_MapY);
                        break;
                case C043_ACTION_FUSE:
                        F0406_MENUS_SetChampionDirectionToPartyDirection(L1247_ps_Champion);
                        L1251_i_MapX = G0306_i_PartyMapX;
                        L1252_i_MapY = G0307_i_PartyMapY;
                        L1251_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[G0308_i_PartyDirection], L1252_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[G0308_i_PartyDirection];
                        F0225_GROUP_FuseAction(L1251_i_MapX, L1252_i_MapY);
                        break;
                case C036_ACTION_HEAL:
                        if (((AL1250_i_MissingHealth = L1247_ps_Champion->MaximumHealth - L1247_ps_Champion->CurrentHealth) > 0) && L1247_ps_Champion->CurrentMana) {
                                AL1246_i_HealingCapability = F0024_MAIN_GetMinimumValue(10, F0303_CHAMPION_GetSkillLevel(P0787_ui_ChampionIndex, C13_SKILL_HEAL));
                                L1255_i_ActionExperienceGain = 2;
                                do {
                                        AL1992_HealingAmount = F0024_MAIN_GetMinimumValue(AL1250_i_MissingHealth, AL1246_i_HealingCapability);
                                        L1247_ps_Champion->CurrentHealth += AL1992_HealingAmount;
                                        L1255_i_ActionExperienceGain += 2;
                                } while (((L1247_ps_Champion->CurrentMana -= 2) > 0) && (AL1250_i_MissingHealth -= AL1992_HealingAmount));
                                if (L1247_ps_Champion->CurrentMana < 0) {
                                        L1247_ps_Champion->CurrentMana = 0;
                                }
                                M008_SET(L1247_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                                AL1245_B_ActionPerformed = C1_TRUE;
                        }
                        break;
                case C039_ACTION_WINDOW:
                        AL1246_i_Ticks = M002_RANDOM(F0303_CHAMPION_GetSkillLevel(P0787_ui_ChampionIndex, L1254_i_ActionSkillIndex) + 8) + 5;
                        L1258_s_Event.A.A.Priority = 0;
                        L1258_s_Event.A.A.Type = C73_EVENT_THIEVES_EYE;
                        M033_SET_MAP_AND_TIME(L1258_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + AL1246_i_Ticks);
                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1258_s_Event);
                        G0407_s_Party.Event73Count_ThievesEye++;
                        goto T0407076;
                case C010_ACTION_CLIMB_DOWN:
                        L1251_i_MapX = G0306_i_PartyMapX;
                        L1252_i_MapY = G0307_i_PartyMapY;
                        L1251_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[G0308_i_PartyDirection], L1252_i_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[G0308_i_PartyDirection];
                        if ((M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(L1251_i_MapX, L1252_i_MapY)) == C02_ELEMENT_PIT) && (F0175_GROUP_GetThing(L1251_i_MapX, L1252_i_MapY) == C0xFFFE_THING_ENDOFLIST)) {
/* BUG0_77 The party moves forward when using the rope in front of a closed pit. The engine does not check whether the pit is open before moving the party over the pit. This is not consistent with the behavior when using the rope in front of a corridor where nothing happens */
                                G0402_B_UseRopeToClimbDownPit = C1_TRUE;
                                F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1251_i_MapX, L1252_i_MapY);
                                G0402_B_UseRopeToClimbDownPit = C0_FALSE;
                        } else {
                                L1249_ui_ActionDisabledTicks = 0;
/* BUG0_79 Stamina is decreased and experience is gained even when the Climb Down action fails. If the Climb Down action is not possible because there is no pit in front of the party or there is a levitating creature over the pit, the engine does cancel the disabling of the champion action icon but it does not cancel the stamina decrease nor the experience gain. */
                        }
                        break;
                case C011_ACTION_FREEZE_LIFE:
                        if (L1248_ps_Weapon->Type == C42_JUNK_MAGICAL_BOX_BLUE) {
                                AL1246_i_Ticks = 30;
                                goto T0407071;
                        }
                        else {
                                if (L1248_ps_Weapon->Type == C43_JUNK_MAGICAL_BOX_GREEN) {
                                        AL1246_i_Ticks = 125;
                                        T0407071:
                                        F0300_CHAMPION_GetObjectRemovedFromSlot(P0787_ui_ChampionIndex, C01_SLOT_ACTION_HAND);
                                        L1248_ps_Weapon->Next = C0xFFFF_THING_NONE;
                                } else {
                                        AL1246_i_Ticks = 70;
                                        F0405_MENUS_DecrementCharges(L1247_ps_Champion);
                                }
                        }
                        G0407_s_Party.FreezeLifeTicks = F0024_MAIN_GetMinimumValue(200, G0407_s_Party.FreezeLifeTicks + AL1246_i_Ticks);
                        break;
                case C038_ACTION_LIGHT:
                        G0407_s_Party.MagicalLightAmount += G0039_ai_Graphic562_LightPowerToLightAmount[2];
                        F0404_MENUS_CreateEvent70_Light(-2, 2500);
                        T0407076:
                        F0405_MENUS_DecrementCharges(L1247_ps_Champion);
                        break;
                case C042_ACTION_THROW:
                        F0406_MENUS_SetChampionDirectionToPartyDirection(L1247_ps_Champion);
                        if (AL1245_B_ActionPerformed = F0328_CHAMPION_IsObjectThrown(P0787_ui_ChampionIndex, C01_SLOT_ACTION_HAND, (L1247_ps_Champion->Cell == M017_NEXT(G0308_i_PartyDirection)) || (L1247_ps_Champion->Cell == M018_OPPOSITE(G0308_i_PartyDirection)))) {
                                G0370_ps_Events[L1247_ps_Champion->EnableActionEventIndex].B.SlotOrdinal = M000_INDEX_TO_ORDINAL(C01_SLOT_ACTION_HAND);
                        }
                        break;
        }
        if (L1249_ui_ActionDisabledTicks) {
                F0330_CHAMPION_DisableAction(P0787_ui_ChampionIndex, L1249_ui_ActionDisabledTicks);
        }
        if (L1253_i_ActionStamina) {
                F0325_CHAMPION_DecrementStamina(P0787_ui_ChampionIndex, L1253_i_ActionStamina);
        }
        if (L1255_i_ActionExperienceGain) {
                F0304_CHAMPION_AddSkillExperience(P0787_ui_ChampionIndex, L1254_i_ActionSkillIndex, L1255_i_ActionExperienceGain);
        }
        F0292_CHAMPION_DrawState(P0787_ui_ChampionIndex);
        return AL1245_B_ActionPerformed;
}

int16_t F0408_MENUS_GetClickOnSpellCastResult(
void
)
{
        REGISTER CHAMPION* L1260_ps_Champion;
        REGISTER int16_t L1259_i_SpellCastResult;


        L1260_ps_Champion = &M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex];
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0363_COMMAND_HighlightBoxDisable();
        if ((L1259_i_SpellCastResult = F0412_MENUS_GetChampionSpellCastResult(G0514_i_MagicCasterChampionIndex)) != C3_SPELL_CAST_FAILURE_NEEDS_FLASK) {
                L1260_ps_Champion->Symbols[0] = '\0';
                F0397_MENUS_DrawAvailableSymbols(L1260_ps_Champion->SymbolStep = 0);
                F0398_MENUS_DrawChampionSymbols(L1260_ps_Champion);
        } else {
                L1259_i_SpellCastResult = C0_SPELL_CAST_FAILURE;
        }
        F0078_MOUSE_DisableScreenUpdate();
        return L1259_i_SpellCastResult;
}

STATICFUNCTION SPELL* F0409_MENUS_GetSpellFromSymbols(
REGISTER unsigned char* P0789_puc_Symbols FINAL_SEPARATOR
{
        REGISTER SPELL* L1263_ps_Spell;
        REGISTER int16_t L1262_i_Multiple;
#define AL1262_i_BitShiftCount L1262_i_Multiple
#define AL1262_i_SpellIndex    L1262_i_Multiple
        REGISTER long L1261_l_Symbols;


        if (*(P0789_puc_Symbols + 1)) {
                AL1262_i_BitShiftCount = 24;
                L1261_l_Symbols = 0;
                do {
                        L1261_l_Symbols |= (long)*P0789_puc_Symbols++ << AL1262_i_BitShiftCount;
                } while (*P0789_puc_Symbols && ((AL1262_i_BitShiftCount -= 8) >= 0));
                L1263_ps_Spell = G0487_as_Graphic560_Spells;
                AL1262_i_SpellIndex = M530_SPELL_COUNT;
                while (AL1262_i_SpellIndex--) {
                        if (L1263_ps_Spell->Symbols & 0xFF000000) { /* If byte 1 of spell is not 0 then the spell includes the power symbol */
                                if (L1261_l_Symbols == L1263_ps_Spell->Symbols) { /* Compare champion symbols, including power symbol, with spell (never used with actual spells) */
                                        return L1263_ps_Spell;
                                }
                        } else {
                                if ((L1261_l_Symbols & 0x00FFFFFF) == L1263_ps_Spell->Symbols) { /* Compare champion symbols, except power symbol, with spell */
                                        return L1263_ps_Spell;
                                }
                        }
                        L1263_ps_Spell++;
                }
        }
        return NULL;
}


STATICFUNCTION POTION* F0411_MENUS_GetEmptyFlaskInHand(
REGISTER CHAMPION* P0793_ps_Champion    SEPARATOR
REGISTER THING*    P0794_pT_PotionThing FINAL_SEPARATOR
{
        REGISTER THING L1265_T_Thing;


        REGISTER int16_t L1266_i_SlotIndex;


        for (L1266_i_SlotIndex = C02_SLOT_HEAD; --L1266_i_SlotIndex >= C00_SLOT_READY_HAND; ) {
                if (((L1265_T_Thing = P0793_ps_Champion->Slots[L1266_i_SlotIndex]) != C0xFFFF_THING_NONE) && (F0033_OBJECT_GetIconIndex(L1265_T_Thing) == C195_ICON_POTION_EMPTY_FLASK)) {
                        *P0794_pT_PotionThing = L1265_T_Thing;
                        return (POTION*)F0156_DUNGEON_GetThingData(L1265_T_Thing);
                }
        }
        return NULL;
}

int16_t F0412_MENUS_GetChampionSpellCastResult(
REGISTER unsigned int16_t P0795_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1270_ps_Champion;
        REGISTER SPELL* L1271_ps_Spell;
        REGISTER POTION* L1275_ps_Potion;
        JUNK* L1277_ps_Junk;
        REGISTER unsigned int16_t L1267_ui_Multiple;
#define AL1267_ui_SkillLevel L1267_ui_Multiple
#define AL1267_ui_LightPower L1267_ui_Multiple
#define AL1267_ui_SpellPower L1267_ui_Multiple
#define AL1267_ui_Ticks      L1267_ui_Multiple
#define AL1267_ui_SlotIndex  L1267_ui_Multiple
        REGISTER int16_t L1268_i_PowerSymbolOrdinal;
        REGISTER unsigned int16_t L1269_ui_Multiple;
#define AL1269_ui_RequiredSkillLevel L1269_ui_Multiple
#define AL1269_ui_EmptyFlaskWeight   L1269_ui_Multiple
#define AL1269_ui_Ticks              L1269_ui_Multiple
#define AL1269_ui_EventIndex         L1269_ui_Multiple
        REGISTER int16_t L1274_i_MissingSkillLevelCount;
        unsigned int16_t L1273_ui_Experience;
        int16_t L3014_i_SpellDisabledTicks;
        THING L1272_T_Object;
        EVENT L1276_s_Event;


        if (P0795_ui_ChampionIndex >= G0305_ui_PartyChampionCount) {
                return C0_SPELL_CAST_FAILURE;
        }
        L1270_ps_Champion = &M516_CHAMPIONS[P0795_ui_ChampionIndex];
        if (!(L1270_ps_Champion->CurrentHealth)) {
                return C0_SPELL_CAST_FAILURE;
        }
        if ((L1271_ps_Spell = F0409_MENUS_GetSpellFromSymbols(M774_CAST_PUC(L1270_ps_Champion->Symbols))) == NULL) {
                F0410_MENUS_PrintSpellFailureMessage(L1270_ps_Champion, C01_FAILURE_MEANINGLESS_SPELL, 0);
                return C0_SPELL_CAST_FAILURE;
        }
        L3014_i_SpellDisabledTicks = M069_SPELL_DISABLED_TICKS(L1271_ps_Spell);
        L1268_i_PowerSymbolOrdinal = L1270_ps_Champion->Symbols[0] - '_'; /* Values 1 to 6 */
        L1273_ui_Experience = M003_RANDOM(8) + ((AL1269_ui_RequiredSkillLevel = L1271_ps_Spell->BaseRequiredSkillLevel + L1268_i_PowerSymbolOrdinal) << 4) + ((M001_ORDINAL_TO_INDEX(L1268_i_PowerSymbolOrdinal) * L1271_ps_Spell->BaseRequiredSkillLevel) << 3) + (AL1269_ui_RequiredSkillLevel * AL1269_ui_RequiredSkillLevel);
        AL1267_ui_SkillLevel = F0303_CHAMPION_GetSkillLevel(P0795_ui_ChampionIndex, L1271_ps_Spell->SkillIndex);
        if (AL1267_ui_SkillLevel < AL1269_ui_RequiredSkillLevel) {
                L1274_i_MissingSkillLevelCount = AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel;
                while (L1274_i_MissingSkillLevelCount--) {
                        if (M003_RANDOM(128) > F0024_MAIN_GetMinimumValue(L1270_ps_Champion->Statistics[C3_STATISTIC_WISDOM][C1_CURRENT] + 15, 115)) {
                                F0304_CHAMPION_AddSkillExperience(P0795_ui_ChampionIndex, L1271_ps_Spell->SkillIndex, L1273_ui_Experience >> (AL1269_ui_RequiredSkillLevel - AL1267_ui_SkillLevel));
                                F0410_MENUS_PrintSpellFailureMessage(L1270_ps_Champion, C00_FAILURE_NEEDS_MORE_PRACTICE, L1271_ps_Spell->SkillIndex);
                                return C0_SPELL_CAST_FAILURE;
                        }
                }
        }
        switch (M067_SPELL_KIND(L1271_ps_Spell)) {
                case C1_SPELL_KIND_POTION:
                        if ((L1275_ps_Potion = F0411_MENUS_GetEmptyFlaskInHand(L1270_ps_Champion, &L1272_T_Object)) == NULL) {
                                F0410_MENUS_PrintSpellFailureMessage(L1270_ps_Champion, C10_FAILURE_NEEDS_FLASK_IN_HAND, 0);
                                return C3_SPELL_CAST_FAILURE_NEEDS_FLASK;
                        }
                        AL1269_ui_EmptyFlaskWeight = F0140_DUNGEON_GetObjectWeight(L1272_T_Object);
                        L1275_ps_Potion->Type = M068_SPELL_TYPE(L1271_ps_Spell);
                        L1275_ps_Potion->Power = M003_RANDOM(16) + (L1268_i_PowerSymbolOrdinal * 40);
                        L1270_ps_Champion->Load += F0140_DUNGEON_GetObjectWeight(L1272_T_Object) - AL1269_ui_EmptyFlaskWeight;
                        F0296_CHAMPION_DrawChangedObjectIcons();
                        if (G0423_i_InventoryChampionOrdinal == M000_INDEX_TO_ORDINAL(P0795_ui_ChampionIndex)) {
                                M008_SET(L1270_ps_Champion->Attributes, MASK0x0200_LOAD);
                                F0292_CHAMPION_DrawState(P0795_ui_ChampionIndex);
                        }
                        break;
                case C2_SPELL_KIND_PROJECTILE:
                        if (L1270_ps_Champion->Direction != G0308_i_PartyDirection) {
                                L1270_ps_Champion->Direction = G0308_i_PartyDirection;
                                M008_SET(L1270_ps_Champion->Attributes, MASK0x0400_ICON);
                                F0292_CHAMPION_DrawState(P0795_ui_ChampionIndex);
                        }
                        if (M068_SPELL_TYPE(L1271_ps_Spell) == C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR) {
                                AL1267_ui_SkillLevel <<= 1;
                        }
                        F0327_CHAMPION_IsProjectileSpellCast(P0795_ui_ChampionIndex, M068_SPELL_TYPE(L1271_ps_Spell) + C0xFF80_THING_FIRST_EXPLOSION, F0026_MAIN_GetBoundedValue(21, (L1268_i_PowerSymbolOrdinal + 2) * (4 + (AL1267_ui_SkillLevel << 1)), 255), 0);
                        break;
                case C3_SPELL_KIND_OTHER:
                        L1276_s_Event.A.A.Priority = 0;
                        AL1267_ui_SpellPower = (L1268_i_PowerSymbolOrdinal + 1) << 2;
                        switch (M068_SPELL_TYPE(L1271_ps_Spell)) {
                                case C0_SPELL_TYPE_OTHER_LIGHT:
                                        AL1269_ui_Ticks = 10000 + ((AL1267_ui_SpellPower - 8) << 9);
                                        AL1267_ui_LightPower >>= 1;
                                        AL1267_ui_LightPower--;
                                        goto T0412019;
                                case C5_SPELL_TYPE_OTHER_MAGIC_TORCH:
                                        AL1269_ui_Ticks = 2000 + ((AL1267_ui_SpellPower - 3) << 7);
                                        AL1267_ui_LightPower >>= 2;
                                        AL1267_ui_LightPower++;
                                        T0412019:
                                        G0407_s_Party.MagicalLightAmount += G0039_ai_Graphic562_LightPowerToLightAmount[AL1267_ui_LightPower];
                                        F0404_MENUS_CreateEvent70_Light(-AL1267_ui_LightPower, AL1269_ui_Ticks);
                                        break;
                                case C1_SPELL_TYPE_OTHER_DARKNESS:
                                        AL1267_ui_LightPower >>= 2;
                                        G0407_s_Party.MagicalLightAmount -= G0039_ai_Graphic562_LightPowerToLightAmount[AL1267_ui_LightPower];
                                        F0404_MENUS_CreateEvent70_Light(AL1267_ui_LightPower, 98);
                                        break;
                                case C2_SPELL_TYPE_OTHER_THIEVES_EYE:
                                        L1276_s_Event.A.A.Type = C73_EVENT_THIEVES_EYE;
                                        G0407_s_Party.Event73Count_ThievesEye++;
                                        AL1267_ui_SpellPower >>= 1;
                                        goto T0412032;
                                case C3_SPELL_TYPE_OTHER_INVISIBILITY:
                                        L1276_s_Event.A.A.Type = C71_EVENT_INVISIBILITY;
                                        if (!G0407_s_Party.Event71Count_Invisibility++) {
                                                if (G0423_i_InventoryChampionOrdinal) {
                                                        M008_SET(M516_CHAMPIONS[G0423_i_InventoryChampionOrdinal - 1].Attributes, MASK0x1000_STATUS_BOX);
                                                }
                                                F0293_CHAMPION_DrawAllChampionStates(MASK0x0400_ICON);
                                        }
                                        AL1267_ui_SpellPower <<= 3;
                                        goto T0412033;
                                case C4_SPELL_TYPE_OTHER_PARTY_SHIELD:
                                        L1276_s_Event.A.A.Type = C74_EVENT_PARTY_SHIELD;
                                        L1276_s_Event.B.Defense = AL1267_ui_SpellPower;
                                        if (G0407_s_Party.ShieldDefense > 50) {
                                                L1276_s_Event.B.Defense >>= 2;
                                        }
                                        G0407_s_Party.ShieldDefense += L1276_s_Event.B.Defense;
                                        F0293_CHAMPION_DrawAllChampionStates(MASK0x1000_STATUS_BOX);
                                        goto T0412032;
                                case C6_SPELL_TYPE_OTHER_FOOTPRINTS:
                                        L1276_s_Event.A.A.Type = C79_EVENT_FOOTPRINTS;
                                        G0407_s_Party.Event79Count_Footprints++;
                                        G0407_s_Party.FirstScentIndex = G0407_s_Party.ScentCount;
                                        if (L1268_i_PowerSymbolOrdinal < 3) {
                                                G0407_s_Party.LastScentIndex = G0407_s_Party.FirstScentIndex;
                                        } else {
                                                G0407_s_Party.LastScentIndex = 0;
                                        }
                                        T0412032:
                                        AL1267_ui_Ticks *= AL1267_ui_SpellPower;
                                        T0412033:
                                        M033_SET_MAP_AND_TIME(L1276_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + AL1267_ui_Ticks);
                                        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1276_s_Event);
                                        break;
                                case C7_SPELL_TYPE_OTHER_ZOKATHRA:
                                        if ((L1272_T_Object = F0166_DUNGEON_GetUnusedThing(C10_THING_TYPE_JUNK)) == C0xFFFF_THING_NONE)
                                                break;
                                        L1277_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1272_T_Object);
                                        L1277_ps_Junk->Type = C51_JUNK_ZOKATHRA;
                                        if (L1270_ps_Champion->Slots[C00_SLOT_READY_HAND] == C0xFFFF_THING_NONE) {
                                                AL1267_ui_SlotIndex = C00_SLOT_READY_HAND;
                                        } else {
                                                if (L1270_ps_Champion->Slots[C01_SLOT_ACTION_HAND] == C0xFFFF_THING_NONE) {
                                                        AL1267_ui_SlotIndex = C01_SLOT_ACTION_HAND;
                                                } else {
                                                        AL1267_ui_SlotIndex = 0xFFFF;
                                                }
                                        }
                                        if ((AL1267_ui_SlotIndex == C00_SLOT_READY_HAND) || (AL1267_ui_SlotIndex == C01_SLOT_ACTION_HAND)) {
                                                F0301_CHAMPION_AddObjectInSlot(P0795_ui_ChampionIndex, L1272_T_Object, AL1267_ui_SlotIndex);
#ifdef PC_FIX_CODE_SIZE
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
#endif
                                                F0292_CHAMPION_DrawState(P0795_ui_ChampionIndex);
                                        } else {
#ifdef PC_FIX_CODE_SIZE
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
        P0795_ui_ChampionIndex++;
#endif
                                                F0267_MOVE_GetMoveResult_CPSCE(L1272_T_Object, CM1_MAPX_NOT_ON_A_SQUARE, 0, G0306_i_PartyMapX, G0307_i_PartyMapY);
                                        }
                                        break;
                                case C8_SPELL_TYPE_OTHER_FIRESHIELD:
                                        F0403_MENUS_IsPartySpellOrFireShieldSuccessful(L1270_ps_Champion, C0_FALSE, (AL1267_ui_SpellPower * AL1267_ui_SpellPower) + 100, C0_FALSE);
                                        break;
                        }
        }
        F0304_CHAMPION_AddSkillExperience(P0795_ui_ChampionIndex, L1271_ps_Spell->SkillIndex, L1273_ui_Experience);
        F0330_CHAMPION_DisableAction(P0795_ui_ChampionIndex, L3014_i_SpellDisabledTicks);
        return C1_SPELL_CAST_SUCCESS;
}

#include "SYMBOL.C"
/* END MENU.C */

/* BEGIN PANEL.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
int16_t G0421_i_ObjectDescriptionTextX; /* static? */
int16_t G0422_i_ObjectDescriptionTextY; /* static? */
int16_t G0423_i_InventoryChampionOrdinal;
int16_t G2008_i_PanelContent = C00_PANEL_INVENTORY;
int16_t G0424_i_PanelContent = C00_PANEL_FOOD_WATER_POISONED;
THING G0425_aT_ChestSlots[8];
THING G0426_T_OpenChest = C0xFFFF_THING_NONE;
#ifdef X193_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
char* G0428_apc_SkillLevelNames[15] = {
        "NEOPHYTE",
        "NOVICE",
        "APPRENTICE",
        "JOURNEYMAN",
        "CRAFTSMAN",
        "ARTISAN",
        "ADEPT",
        "EXPERT",
        "` MASTER",
        "a MASTER",
        "b MASTER",
        "c MASTER",
        "d MASTER",
        "e MASTER",
        "ARCHMASTER" };
#endif
#ifdef X736_I34M
int16_t G2014_ai_SkillLevelNames[15] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 }; /* Text indices */
#endif




STATICFUNCTION void F0332_INVENTORY_DrawIconToViewport(
int16_t P0689_i_IconIndex SEPARATOR
int16_t P2283_i_ZoneIndex FINAL_SEPARATOR
{
        unsigned char* L1016_puc_Bitmap_Icon;
        int16_t L2825_ai_XYZ[4];


        L1016_puc_Bitmap_Icon = F0606_AllocateMemForGraphic(G2075_ObjectIconWidth, G2076_ObjectIconHeight, C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP);
        F0618_LoadZone(P2283_i_ZoneIndex, L2825_ai_XYZ);
        F0036_OBJECT_ExtractIconFromBitmap(P0689_i_IconIndex, M775_CAST_PL(L1016_puc_Bitmap_Icon));
        F0020_MAIN_BlitToViewport(L1016_puc_Bitmap_Icon, L2825_ai_XYZ, CM1_COLOR_NO_TRANSPARENCY);
        F0607_FreeMemForGraphic(L1016_puc_Bitmap_Icon);
}


STATICFUNCTION void F0335_INVENTORY_DrawPanel_ObjectDescriptionString(
REGISTER char* P0695_pc_String FINAL_SEPARATOR
{
        REGISTER char* L1030_pc_StringLine;
        REGISTER char* L1031_pc_String;
        REGISTER BOOLEAN L1029_B_SeveralLines;
        char L1032_ac_String[128];


        if (P0695_pc_String[0] == '\f') { /* Form feed */
                P0695_pc_String++;
                F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C556_ZONE_OBJECT_DESCRIPTION, &G0421_i_ObjectDescriptionTextX, &G0422_i_ObjectDescriptionTextY);
        }
        if (P0695_pc_String[0]) {
                M547_STRCPY(L1032_ac_String, P0695_pc_String);
                L1030_pc_StringLine = L1032_ac_String;
                L1029_B_SeveralLines = C0_FALSE;
                while (*L1030_pc_StringLine) {
                        if (M544_STRLEN(L1030_pc_StringLine) > 18) { /* If string is too long to fit on one line */
                                L1031_pc_String = &L1030_pc_StringLine[17];
                                while (*L1031_pc_String != ' ') { /* Go back to the last space character */
                                        L1031_pc_String--;
                                }
                                *L1031_pc_String = '\0'; /* and split the string there */
                                L1029_B_SeveralLines = C1_TRUE;
                        }
                        F0052_TEXT_PrintToViewport(G0421_i_ObjectDescriptionTextX, G0422_i_ObjectDescriptionTextY, C13_COLOR_LIGHTEST_GRAY, L1030_pc_StringLine);
                        G0422_i_ObjectDescriptionTextY += G2088_C7_TextLineHeight;
                        if (L1029_B_SeveralLines) {
                                L1029_B_SeveralLines = C0_FALSE;
                                L1030_pc_StringLine = ++L1031_pc_String;
                        } else {
                                *L1030_pc_StringLine = '\0';
                        }
                }
        }
}

STATICFUNCTION void F0336_INVENTORY_DrawPanel_BuildObjectAttributesString(
int16_t          P0696_i_PotentialAttributesMask SEPARATOR
int16_t          P0697_i_ActualAttributesMask    SEPARATOR
char**           P0698_ppc_AttributeStrings      SEPARATOR
REGISTER char*   P0699_pc_StringDestination      SEPARATOR
char*            P0700_pc_StringPrefix           SEPARATOR
char*            P0701_pc_StringSuffix           FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L1033_ui_StringIndex;
        REGISTER int16_t L1034_i_AttributeMask;
        REGISTER unsigned int16_t L1035_ui_IdenticalBitCount;


        L1035_ui_IdenticalBitCount = 0;
        L1034_i_AttributeMask = 1;
        for (L1033_ui_StringIndex = 0; L1033_ui_StringIndex < 16; L1033_ui_StringIndex++) {
                if (L1034_i_AttributeMask & P0696_i_PotentialAttributesMask & P0697_i_ActualAttributesMask) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE test */
                        L1035_ui_IdenticalBitCount++;
                }
                L1034_i_AttributeMask <<= 1;
        }
        if (L1035_ui_IdenticalBitCount == 0) {
                *P0699_pc_StringDestination = '\0';
                return;
        }
        M547_STRCPY(P0699_pc_StringDestination, P0700_pc_StringPrefix);
        L1034_i_AttributeMask = 1;
        for (L1033_ui_StringIndex = 0; L1033_ui_StringIndex < 16; L1033_ui_StringIndex++) {
                if (L1034_i_AttributeMask & P0696_i_PotentialAttributesMask & P0697_i_ActualAttributesMask) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE test */
                        M545_STRCAT(P0699_pc_StringDestination, P0698_ppc_AttributeStrings[L1033_ui_StringIndex]);
                        if (L1035_ui_IdenticalBitCount-- > 2) {
                                M545_STRCAT(P0699_pc_StringDestination, ", ");
                        } else {
                                if (L1035_ui_IdenticalBitCount == 1) {
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                        M545_STRCAT(P0699_pc_StringDestination, " AND ");
#endif
#ifdef X736_I34M
                                        M545_STRCAT(P0699_pc_StringDestination, F0758_TranslateLanguage(C30_TEXT_AND));
#endif
                                }
                        }
                }
                L1034_i_AttributeMask <<= 1;
        }
        M545_STRCAT(P0699_pc_StringDestination, P0701_pc_StringSuffix);
}


void F0337_INVENTORY_SetDungeonViewPalette(
void
)
{
        REGISTER int16_t* L1040_pi_Multiple;
#define AL1040_pi_TorchLightPower L1040_pi_Multiple
#define AL1040_pi_LightAmount     L1040_pi_Multiple
        REGISTER int16_t* L1041_pi_TorchLightPower;
        REGISTER CHAMPION* L1043_ps_Champion;
        WEAPON* L1042_ps_Weapon;
        REGISTER unsigned int16_t L1039_ui_Multiple;
#define AL1039_ui_SlotIndex    L1039_ui_Multiple
#define AL1039_ui_PaletteIndex L1039_ui_Multiple
#define AL1039_ui_Counter      L1039_ui_Multiple
        REGISTER int16_t L1036_i_TotalLightAmount;
        REGISTER unsigned int16_t L1037_ui_TorchLightAmountMultiplier;
        REGISTER int16_t L1038_i_Counter;
#define AL1044_T_Thing            L1044_ui_Multiple
#define AL1044_ui_TorchLightPower L1044_ui_Multiple
        REGISTER unsigned int16_t L1044_ui_Multiple;
        int16_t L1045_ai_TorchesLightPower[8];


        if (G0269_ps_CurrentMap->C.Difficulty == 0) {
                G0304_i_DungeonViewPaletteIndex = 0; /* Brightest color palette index */
        } else {
                /* Get torch light power from both hands of each champion in the party */
                L1038_i_Counter = 4; /* BUG0_01 Coding error without consequence. The hands of four champions are inspected even if there are less champions in the party. No consequence as the data in unused champions is set to 0 and F0032_OBJECT_GetType then returns -1 */
                L1043_ps_Champion = M516_CHAMPIONS;
                AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
                while (L1038_i_Counter--) {
                        AL1039_ui_SlotIndex = C01_SLOT_ACTION_HAND + 1;
                        while (AL1039_ui_SlotIndex--) {
                                if ((F0032_OBJECT_GetType(AL1044_T_Thing = L1043_ps_Champion->Slots[AL1039_ui_SlotIndex]) >= C004_ICON_WEAPON_TORCH_UNLIT) &&
                                    (F0032_OBJECT_GetType(AL1044_T_Thing = L1043_ps_Champion->Slots[AL1039_ui_SlotIndex]) <= C007_ICON_WEAPON_TORCH_LIT)) {
                                        L1042_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(AL1044_T_Thing);
                                        *AL1040_pi_TorchLightPower = L1042_ps_Weapon->ChargeCount;
                                } else {
                                        *AL1040_pi_TorchLightPower = 0;
                                }
                                AL1040_pi_TorchLightPower++;
                        }
                        L1043_ps_Champion++;
                }
                /* Sort torch light power values so that the four highest values are in the first four entries in the array L1045_ai_TorchesLightPower in decreasing order. The last four entries contain the smallest values but they are not sorted */
                AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
                AL1039_ui_Counter = 0;
                while (AL1039_ui_Counter != 4) {
                        L1038_i_Counter = 7 - AL1039_ui_Counter;
                        L1041_pi_TorchLightPower = &L1045_ai_TorchesLightPower[AL1039_ui_Counter + 1];
                        while (L1038_i_Counter--) {
                                if (*L1041_pi_TorchLightPower > *AL1040_pi_TorchLightPower) {
                                        AL1044_ui_TorchLightPower = *L1041_pi_TorchLightPower;
                                        *L1041_pi_TorchLightPower = *AL1040_pi_TorchLightPower;
                                        *AL1040_pi_TorchLightPower = AL1044_ui_TorchLightPower;
                                }
                                L1041_pi_TorchLightPower++;
                        }
                        AL1040_pi_TorchLightPower++;
                        AL1039_ui_Counter++;
                }
                /* Get total light amount provided by the four torches with the highest light power values and by the fifth torch in the array which may be any one of the four torches with the smallest ligh power values */
                L1037_ui_TorchLightAmountMultiplier = 6;
                AL1039_ui_Counter = 5;
                L1036_i_TotalLightAmount = 0;
                AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
                while (AL1039_ui_Counter--) {
                        if (*AL1040_pi_TorchLightPower) {
                                L1036_i_TotalLightAmount += (G0039_ai_Graphic562_LightPowerToLightAmount[*AL1040_pi_TorchLightPower] << L1037_ui_TorchLightAmountMultiplier) >> 6;
                                L1037_ui_TorchLightAmountMultiplier = F0025_MAIN_GetMaximumValue(0, L1037_ui_TorchLightAmountMultiplier - 1);
                        }
                        AL1040_pi_TorchLightPower++;
                }
                L1036_i_TotalLightAmount += G0407_s_Party.MagicalLightAmount;
                /* Select palette corresponding to the total light amount */
                AL1040_pi_LightAmount = G0040_ai_Graphic562_PaletteIndexToLightAmount;
                if (L1036_i_TotalLightAmount > 0) {
                        AL1039_ui_PaletteIndex = 0; /* Brightest color palette index */
                        while (*AL1040_pi_LightAmount++ > L1036_i_TotalLightAmount) {
                                AL1039_ui_PaletteIndex++;
                        }
                } else {
                        AL1039_ui_PaletteIndex = 5; /* Darkest color palette index */
                }
                G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex;
        }
        G0342_B_RefreshDungeonViewPaletteRequested = C1_TRUE;
}

void F0338_INVENTORY_DecreaseTorchesLightPower_CPSE(
void
)
{
        REGISTER CHAMPION* L1050_ps_Champion;
        REGISTER WEAPON* L1051_ps_Weapon;
        REGISTER int16_t L1047_i_SlotIndex;
        REGISTER int16_t L1046_i_ChampionCount;
        REGISTER BOOLEAN L1048_B_TorchChargeCountChanged;
        REGISTER int16_t L1049_i_IconIndex;


        L1048_B_TorchChargeCountChanged = C0_FALSE;
        L1046_i_ChampionCount = G0305_ui_PartyChampionCount;
        if (G0299_ui_CandidateChampionOrdinal) {
                L1046_i_ChampionCount--;
        }
        L1050_ps_Champion = M516_CHAMPIONS;
        while (L1046_i_ChampionCount--) {
                L1047_i_SlotIndex = C01_SLOT_ACTION_HAND + 1;
                while (L1047_i_SlotIndex--) {
                        L1049_i_IconIndex = F0033_OBJECT_GetIconIndex(L1050_ps_Champion->Slots[L1047_i_SlotIndex]);
                        if ((L1049_i_IconIndex >= C004_ICON_WEAPON_TORCH_UNLIT) && (L1049_i_IconIndex <= C007_ICON_WEAPON_TORCH_LIT)) {
                                L1051_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(L1050_ps_Champion->Slots[L1047_i_SlotIndex]);
                                if (L1051_ps_Weapon->ChargeCount) {
                                        if ((--L1051_ps_Weapon->ChargeCount) == 0) {
                                                L1051_ps_Weapon->DoNotDiscard = C0_FALSE;
                                        }
                                        L1048_B_TorchChargeCountChanged = C1_TRUE;
                                }
                        }
                }
                L1050_ps_Champion++;
        }
        if (L1048_B_TorchChargeCountChanged) {
                F0337_INVENTORY_SetDungeonViewPalette();
                F0296_CHAMPION_DrawChangedObjectIcons();
        }
}

STATICFUNCTION void F0339_INVENTORY_DrawPanel_ArrowOrEye(
BOOLEAN P0702_B_PressingEye FINAL_SEPARATOR
{
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(P0702_B_PressingEye ? C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION : C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT, C503_ZONE_ARROW_OR_EYE, C08_COLOR_RED);
}


STATICFUNCTION void F0340_INVENTORY_DrawPanel_ScrollTextLine(
int16_t        P2293_i_X       SEPARATOR
int16_t        P0703_i_Y       SEPARATOR
REGISTER char* P0704_pc_String FINAL_SEPARATOR
{
        int16_t L2839_i_Width;
        int16_t L2840_i_Height;
        REGISTER char* L1052_pc_Character;


        L1052_pc_Character = P0704_pc_String;
        while (*L1052_pc_Character) {
                if ((*L1052_pc_Character >= 'A') && (*L1052_pc_Character <= 'Z')) {
                        *L1052_pc_Character++ -= 64; /* Do not use default characters in font but rather special characters for scrolls */
                } else {
                        if (*L1052_pc_Character >= '{') { /* '{' is ASCII 123 */
                                *L1052_pc_Character++ -= 96;
                        } else
                                L1052_pc_Character++;
                }
        }
        if (F0645_GetStringPixelDimensions(P0704_pc_String, &L2839_i_Width, &L2840_i_Height)) {
                M720_F0644_PrintTextAlt(G0296_puc_Bitmap_Viewport, G2073_C224_ViewportPixelWidth, P2293_i_X - (L2839_i_Width >> 1), P0703_i_Y, C00_COLOR_BLACK, C15_COLOR_WHITE, P0704_pc_String, G2074_C136_ViewportHeight);
        }
}


STATICFUNCTION void F0341_INVENTORY_DrawPanel_Scroll(
SCROLL* P0705_ps_Scroll FINAL_SEPARATOR
{
        REGISTER char* L1055_pc_Character;
        REGISTER char* L1054_pc_Character;
        REGISTER int16_t L1053_i_Multiple;
#define AL1053_B_MultipleLines L1053_i_Multiple
#define AL1053_i_LineCount     L1053_i_Multiple
#define AL1053_i_Y             L1053_i_Multiple
#define AL1053_i_Counter       L1053_i_Multiple
        int16_t L2847_i_X;
        int16_t L2848_i_Y;
        char L1056_ac_StringFirstLine[200];


        G0424_i_PanelContent = M643_PANEL_SCROLL;
        F0168_DUNGEON_DecodeText(M774_CAST_PUC(L1055_pc_Character = L1056_ac_StringFirstLine), P0705_ps_Scroll->TextStringThingIndex, C2_TEXT_TYPE_SCROLL | MASK0x8000_DECODE_EVEN_IF_INVISIBLE);
        while (*L1055_pc_Character && (*L1055_pc_Character != '\n')) { /* New line */ /* BUG0_88 Text is displayed outside of the scroll bitmap. The engine does not limit the length of each line nor the number of lines. This is not an issue in the original dungeons where no scroll contains any line longer than 15 characters or more than 8 lines */
                L1055_pc_Character++;
        }
        if (*L1055_pc_Character == '\0') {
                AL1053_B_MultipleLines = C0_FALSE;
        } else {
                AL1053_B_MultipleLines = C1_TRUE;
        }
        *L1055_pc_Character = '\0'; /* Mark the end of the first line */
                F0658_BlitBitmapIndexToZoneIndexWithTransparency(C023_GRAPHIC_PANEL_OPEN_SCROLL, C101_ZONE_PANEL, C08_COLOR_RED);
                F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C560_ZONE_SCROLL_TEXT, &L2847_i_X, &L2848_i_Y);
                if (AL1053_B_MultipleLines == C0_FALSE) {
                        AL1053_i_LineCount = 1;
                } else {
                        L1055_pc_Character++;
                        L1054_pc_Character = (char*)L1055_pc_Character; /* First character of second line */
                        while (*L1054_pc_Character) { /* BUG0_47 Graphical glitch when you open a scroll. If there is a single line of text in a scroll (with no line feed character) then L1054_pc_Character points to undefined data. This may result in a graphical glitch and also corrupt other memory. This is not an issue in the original dungeons where all scrolls contain at least one line feed character */
                                if (*L1054_pc_Character == '\n') { /* New line */
                                        AL1053_i_LineCount++;
                                }
                                L1054_pc_Character++;
                        }
                        if (*(L1054_pc_Character - 1) != '\n') { /* New line */
                                AL1053_i_LineCount++; /* BUG0_88 Text is displayed outside of the scroll bitmap. The engine does not limit the length of each line nor the number of lines. This is not an issue in the original dungeons where no scroll contains any line longer than 15 characters or more than 8 lines */
                        } else {
                                if (*(L1054_pc_Character - 2) == '\n') { /* New line */
                                        AL1053_i_LineCount--;
                                }
                        }
                }
                L2848_i_Y -= (((G2088_C7_TextLineHeight * AL1053_i_LineCount) - G2086_C1_ - 1) >> 1) - G2083_C6_;
                F0340_INVENTORY_DrawPanel_ScrollTextLine(L2847_i_X, L2848_i_Y, L1056_ac_StringFirstLine);
                L1054_pc_Character = L1055_pc_Character;
                while (*L1054_pc_Character) {
                        L2848_i_Y += G2088_C7_TextLineHeight;
                        while (*L1055_pc_Character && (*L1055_pc_Character != '\n')) { /* New line */
                                L1055_pc_Character++;
                        }
                        if (!(*L1055_pc_Character)) {
                                L1055_pc_Character[1] = '\0';
                        }
                        *L1055_pc_Character++ = '\0';
                        F0340_INVENTORY_DrawPanel_ScrollTextLine(L2847_i_X, L2848_i_Y, L1054_pc_Character);
                        L1054_pc_Character = L1055_pc_Character;
                }
}

#include "CHEST.C"

void F0342_INVENTORY_DrawPanel_Object(
THING   P0706_T_Thing       SEPARATOR
BOOLEAN P0707_B_PressingEye FINAL_SEPARATOR
{
        REGISTER JUNK* L1062_ps_Junk;
        REGISTER char* L1061_pc_DescriptionString;
        REGISTER unsigned int16_t L1057_ui_Weight;
        REGISTER unsigned int16_t L1058_ui_IconIndex;
        REGISTER int16_t L1059_i_PotentialAttributesMask;
        REGISTER int16_t L1060_i_ActualAttributesMask;
        char L1063_ac_Unreferenced[48]; /* BUG0_00 Useless code */
        unsigned int16_t L1067_ui_ThingType;
        char* L1064_apc_AttributeStrings[16];
        char L1068_ac_String[40];
#ifdef X196_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
        static char* G0430_apc_DirectionNames[4] = { "NORTH", "EAST", "SOUTH", "WEST" };
#endif
#ifdef X736_I34M
        static int16_t G2013_ai_DirectionNames[4] = { 22, 24, 23, 25 }; /* Text indices */
#endif


                F0334_INVENTORY_CloseChest(); /* BUG0_48 The contents of a chest are reorganized when an object with a statistic modifier is placed or removed on a champion */
        L1062_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(P0706_T_Thing);
        F0335_INVENTORY_DrawPanel_ObjectDescriptionString("\f"); /* Form Feed */
        L1067_ui_ThingType = M012_TYPE(P0706_T_Thing);
        if (L1067_ui_ThingType == C07_THING_TYPE_SCROLL) {
                F0341_INVENTORY_DrawPanel_Scroll(M777_CAST_SCROLL(L1062_ps_Junk));
        } else {
                if (L1067_ui_ThingType == C09_THING_TYPE_CONTAINER) {
                        F0333_INVENTORY_OpenAndDrawChest(P0706_T_Thing, M778_CAST_CONTAINER(L1062_ps_Junk), P0707_B_PressingEye);
                } else {
                        G0424_i_PanelContent = C03_PANEL_OBJECT_DESCRIPTION;
                        L1058_ui_IconIndex = F0033_OBJECT_GetIconIndex(P0706_T_Thing);
                        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C020_GRAPHIC_PANEL_EMPTY, C101_ZONE_PANEL, C08_COLOR_RED);
                        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE, C504_ZONE_OBJECT_DESCRIPTION_CIRCLE, C12_COLOR_DARKEST_GRAY);
                        if (L1058_ui_IconIndex == C147_ICON_JUNK_CHAMPION_BONES) {
#ifdef X736_I34M
                                if (G2000_Language == C1_FRENCH) {
                                        M547_STRCPY(L1068_ac_String, G0352_apc_ObjectNames[L1058_ui_IconIndex]);
                                        M545_STRCAT(L1068_ac_String, " ");
                                        M545_STRCAT(L1068_ac_String, M516_CHAMPIONS[L1062_ps_Junk->ChargeCount].Name);
                                } else {
#endif
                                        M547_STRCPY(L1068_ac_String, M516_CHAMPIONS[L1062_ps_Junk->ChargeCount].Name);
                                        M545_STRCAT(L1068_ac_String, " ");
                                        M545_STRCAT(L1068_ac_String, M772_CAST_PC(G0352_apc_ObjectNames[L1058_ui_IconIndex]));
#ifdef X736_I34M
                                }
#endif
                                L1061_pc_DescriptionString = L1068_ac_String;
                        } else {
                                if ((L1067_ui_ThingType == C08_THING_TYPE_POTION) && (L1058_ui_IconIndex != C163_ICON_POTION_WATER_FLASK) && (F0303_CHAMPION_GetSkillLevel(M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal), C02_SKILL_PRIEST) > 1)) { /* BUG0_49 When examining an 'Empty Flask' the description string shows an undesired symbol. The code contains an exception not to draw the power for 'Water Flask' potions C195_ICON_POTION_EMPTY_FLASK but a similar exception is missing for 'Empty Flask' potions. These are processed like all other potions so that the power symbol is drawn: '_' for an 'Empty Flask' with Power / 40 = 0, or the power symbol of the last potion contained in the Flask: after drinking a potion, the potion type is set to Empty Flask but the potion power is not reset in F0349_INVENTORY_ProcessCommand70_ClickOnMouth */
                                        L1068_ac_String[0] = '_' + ((POTION*)L1062_ps_Junk)->Power / 40; /* Character '_' precedes the first Symbol in the font. Potions are always created with a minimum power of 40 in F0412_MENUS_GetChampionSpellCastResult */
                                        L1068_ac_String[1] = ' ';
                                        L1068_ac_String[2] = '\0';
                                        M545_STRCAT(L1068_ac_String, M772_CAST_PC(G0352_apc_ObjectNames[L1058_ui_IconIndex]));
                                        L1061_pc_DescriptionString = L1068_ac_String;
                                } else {
                                        L1061_pc_DescriptionString = (char*)G0352_apc_ObjectNames[L1058_ui_IconIndex];
                                }
                        }
                        F0648_PrintTextInViewportZone(C506_ZONE_OBJECT_DESCRIPTION, C13_COLOR_LIGHTEST_GRAY, L1061_pc_DescriptionString);
                        F0332_INVENTORY_DrawIconToViewport(L1058_ui_IconIndex, C505_ZONE_OBJECT_DESCRIPTION_ICON);
                                G0422_i_ObjectDescriptionTextY = 87;
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                        L1064_apc_AttributeStrings[0] = "CONSUMABLE";
                        L1064_apc_AttributeStrings[1] = "POISONED";
                        L1064_apc_AttributeStrings[2] = "BROKEN";
                        L1064_apc_AttributeStrings[3] = "CURSED";
#endif
#ifdef X736_I34M
                        L1064_apc_AttributeStrings[0] = F0758_TranslateLanguage(C26_CONSUMABLE);
                        L1064_apc_AttributeStrings[1] = F0758_TranslateLanguage(C27_POISONED);
                        L1064_apc_AttributeStrings[2] = F0758_TranslateLanguage(C28_BROKEN);
                        L1064_apc_AttributeStrings[3] = F0758_TranslateLanguage(C29_CURSED);
#endif
                        switch (L1067_ui_ThingType) {
                                case C05_THING_TYPE_WEAPON:
                                        L1059_i_PotentialAttributesMask = MASK0x0008_DESCRIPTION_CURSED | MASK0x0002_DESCRIPTION_POISONED | MASK0x0004_DESCRIPTION_BROKEN;
                                        L1060_i_ActualAttributesMask = (((WEAPON*)L1062_ps_Junk)->Cursed << 3) | (((WEAPON*)L1062_ps_Junk)->Poisoned << 1) | (((WEAPON*)L1062_ps_Junk)->Broken << 2);
                                        if ((L1058_ui_IconIndex >= C004_ICON_WEAPON_TORCH_UNLIT) && (L1058_ui_IconIndex <= C007_ICON_WEAPON_TORCH_LIT) && (((WEAPON*)L1062_ps_Junk)->ChargeCount == 0)) {
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                                F0335_INVENTORY_DrawPanel_ObjectDescriptionString("(BURNT OUT)");
#endif
#ifdef X736_I34M
                                                F0335_INVENTORY_DrawPanel_ObjectDescriptionString(F0758_TranslateLanguage(C31_BURNT_OUT));
#endif
                                        }
                                        break;
                                case C06_THING_TYPE_ARMOUR:
                                        L1059_i_PotentialAttributesMask = MASK0x0008_DESCRIPTION_CURSED | MASK0x0004_DESCRIPTION_BROKEN;
                                        L1060_i_ActualAttributesMask = (((ARMOUR*)L1062_ps_Junk)->Cursed << 3) | (((ARMOUR*)L1062_ps_Junk)->Broken << 2);
                                        break;
                                case C08_THING_TYPE_POTION:
                                        L1059_i_PotentialAttributesMask = MASK0x0001_DESCRIPTION_CONSUMABLE;
#ifdef PC_FIX_CODE_SIZE
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
#endif
                                        L1060_i_ActualAttributesMask = G0237_as_Graphic559_ObjectInfo[C002_OBJECT_INFO_INDEX_FIRST_POTION + ((POTION*)L1062_ps_Junk)->Type].AllowedSlots;
                                        break;
                                case C10_THING_TYPE_JUNK:
                                        if ((L1058_ui_IconIndex >= C000_ICON_JUNK_COMPASS_NORTH) && (L1058_ui_IconIndex <= C003_ICON_JUNK_COMPASS_WEST)) {
                                                L1059_i_PotentialAttributesMask = 0;
#ifdef X703_I34E
                                                M547_STRCPY(L1068_ac_String, "PARTY FACING ");
#endif
#ifdef X588_I34E
                                                M545_STRCAT(L1068_ac_String, G0430_apc_DirectionNames[L1058_ui_IconIndex]);
#endif
#ifdef X736_I34M
                                                M547_STRCPY(L1068_ac_String, F0758_TranslateLanguage(C21_PARTY_FACING));
                                                M545_STRCAT(L1068_ac_String, F0758_TranslateLanguage(G2013_ai_DirectionNames[L1058_ui_IconIndex]));
#endif
                                                F0335_INVENTORY_DrawPanel_ObjectDescriptionString(L1068_ac_String);
                                        } else {
/* BUGX_XX ? CSB FM-Towns does not display the state of Waterskin (Full / empty) */
                                                if ((L1058_ui_IconIndex >= C008_ICON_JUNK_WATER) && (L1058_ui_IconIndex <= C009_ICON_JUNK_WATERSKIN)) {
                                                        L1059_i_PotentialAttributesMask = 0;
                                                        switch (L1062_ps_Junk->ChargeCount) {
                                                                case 0:
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                                                        L1061_pc_DescriptionString = "(EMPTY)";
#endif
#ifdef X736_I34M
                                                                        L1061_pc_DescriptionString = F0758_TranslateLanguage(C32_EMPTY);
#endif
                                                                        break;
                                                                case 1:
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                                                        L1061_pc_DescriptionString = "(ALMOST EMPTY)";
#endif
#ifdef X736_I34M
                                                                        L1061_pc_DescriptionString = F0758_TranslateLanguage(C33_ALMOST_EMPTY);
#endif
                                                                        break;
                                                                case 2:
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                                                        L1061_pc_DescriptionString = "(ALMOST FULL)";
#endif
#ifdef X736_I34M
                                                                        L1061_pc_DescriptionString = F0758_TranslateLanguage(C34_ALMOST_FULL);
#endif
                                                                        break;
                                                                case 3:
#ifdef X194_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                                                                        L1061_pc_DescriptionString = "(FULL)";
#endif
#ifdef X736_I34M
                                                                        L1061_pc_DescriptionString = F0758_TranslateLanguage(C35_FULL);
#endif
                                                        }
#ifdef PC_FIX_CODE_SIZE
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
        L1058_ui_IconIndex++;
#endif
                                                        F0335_INVENTORY_DrawPanel_ObjectDescriptionString(L1061_pc_DescriptionString);
                                                } else {
                                                                L1059_i_PotentialAttributesMask = MASK0x0001_DESCRIPTION_CONSUMABLE;
                                                                L1060_i_ActualAttributesMask = G0237_as_Graphic559_ObjectInfo[C127_OBJECT_INFO_INDEX_FIRST_JUNK + L1062_ps_Junk->Type].AllowedSlots;
                                                        }
                                                }
                                        break;
                        }
                        if (L1059_i_PotentialAttributesMask) {
                                F0336_INVENTORY_DrawPanel_BuildObjectAttributesString(L1059_i_PotentialAttributesMask, L1060_i_ActualAttributesMask, L1064_apc_AttributeStrings, L1068_ac_String, "(", ")");
                                F0335_INVENTORY_DrawPanel_ObjectDescriptionString(L1068_ac_String);
                        }
#ifdef X196_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                        M547_STRCPY(L1068_ac_String, "WEIGHS ");
#endif
#ifdef X736_I34M
                        M547_STRCPY(L1068_ac_String, F0758_TranslateLanguage(C37_WEIGHS));
#endif
                        L1057_ui_Weight = F0140_DUNGEON_GetObjectWeight(P0706_T_Thing);
                        M545_STRCAT(L1068_ac_String, F0288_CHAMPION_GetStringFromInteger(L1057_ui_Weight / 10, C0_FALSE, 3));
#ifdef X193_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                        M545_STRCAT(L1068_ac_String, ".");
#endif
#ifdef X736_I34M
                        M545_STRCAT(L1068_ac_String, F0758_TranslateLanguage(C38_CHAMPION_LOAD_DECIMAL_SEPARATOR));
#endif
                        L1057_ui_Weight -= (L1057_ui_Weight / 10) * 10;
                        M545_STRCAT(L1068_ac_String, F0288_CHAMPION_GetStringFromInteger(L1057_ui_Weight, C0_FALSE, 1));
#ifdef X188_I34E /* CHANGE5_00_LOCALIZATION Translation to French language */
                        M545_STRCAT(L1068_ac_String, " KG.");
#endif
#ifdef X736_I34M
                        M545_STRCAT(L1068_ac_String, F0758_TranslateLanguage(C39_CHAMPION_LOAD_KILOGRAM));
#endif
                        F0335_INVENTORY_DrawPanel_ObjectDescriptionString(L1068_ac_String);
                }
        }
        F0339_INVENTORY_DrawPanel_ArrowOrEye(P0707_B_PressingEye);
}


STATICFUNCTION void F0344_INVENTORY_DrawPanel_FoodOrWaterBar(
REGISTER int16_t P0712_i_Amount    SEPARATOR
int16_t          P2295_i_ZoneIndex SEPARATOR
int16_t          P0714_i_Color     FINAL_SEPARATOR
{
        REGISTER int16_t L1070_i_Color;
        int16_t L2863_ai_XYZ[4];
        int16_t L2864_ai_XYZ[4];


        if (P0712_i_Amount < -512) {
                L1070_i_Color = C08_COLOR_RED;
        } else {
                if (P0712_i_Amount < 0) {
                        L1070_i_Color = C11_COLOR_YELLOW;
                } else {
                        L1070_i_Color = P0714_i_Color;
                }
        }
        P0712_i_Amount -= -1024;
        if (!F0637_GetProportionalZone(P2295_i_ZoneIndex, L2863_ai_XYZ, ((int)P0712_i_Amount * 10000L) / (unsigned int16_t)3072L, 10000))
                return;
        F0007_MAIN_CopyBytes((char*)L2863_ai_XYZ, (char*)L2864_ai_XYZ, 8L);
        L2864_ai_XYZ[0] += G2097_FoodOrWaterBarShadowOffset;
        L2864_ai_XYZ[1] += G2097_FoodOrWaterBarShadowOffset;
        M525_F0135_VIDEO_FillBox(G0296_puc_Bitmap_Viewport, L2864_ai_XYZ, C00_COLOR_BLACK, G2073_C224_ViewportPixelWidth, G2074_C136_ViewportHeight);
        M525_F0135_VIDEO_FillBox(G0296_puc_Bitmap_Viewport, L2863_ai_XYZ, L1070_i_Color, G2073_C224_ViewportPixelWidth, G2074_C136_ViewportHeight);
}

void F0345_INVENTORY_DrawPanel_FoodWaterPoisoned(
void
)
{
        REGISTER CHAMPION* L1074_ps_Champion;


        G0424_i_PanelContent = M565_PANEL_FOOD_WATER_POISONED;
        L1074_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)];
        F0334_INVENTORY_CloseChest();
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C020_GRAPHIC_PANEL_EMPTY, C101_ZONE_PANEL, C08_COLOR_RED);
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C030_GRAPHIC_FOOD_LABEL, C500_ZONE_FOOD, C12_COLOR_DARKEST_GRAY);
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C031_GRAPHIC_WATER_LABEL, C501_ZONE_WATER, C12_COLOR_DARKEST_GRAY);
        if (L1074_ps_Champion->PoisonEventCount) {
                F0658_BlitBitmapIndexToZoneIndexWithTransparency(C032_GRAPHIC_POISONED_LABEL, C502_ZONE_POISONED, C12_COLOR_DARKEST_GRAY);
        }
        F0344_INVENTORY_DrawPanel_FoodOrWaterBar(L1074_ps_Champion->Food, C103_ZONE_FOOD_BAR, C05_COLOR_LIGHT_BROWN);
        F0344_INVENTORY_DrawPanel_FoodOrWaterBar(L1074_ps_Champion->Water, C104_ZONE_FOOD_WATER, C14_COLOR_BLUE);
}

STATICFUNCTION void F0346_INVENTORY_DrawPanel_ResurrectReincarnate(
void
)
{
        G0424_i_PanelContent = G2008_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE;
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE, C101_ZONE_PANEL, C06_COLOR_DARK_GREEN);
}

void F0347_INVENTORY_DrawPanel(
void
)
{
        REGISTER THING L1075_T_Thing;


        F0334_INVENTORY_CloseChest();
        if (G0299_ui_CandidateChampionOrdinal) {
                F0346_INVENTORY_DrawPanel_ResurrectReincarnate();
                return;
        }
        L1075_T_Thing = M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)].Slots[C01_SLOT_ACTION_HAND];
        switch (M012_TYPE(L1075_T_Thing)) {
                case C09_THING_TYPE_CONTAINER:
                        G2008_i_PanelContent = M569_PANEL_CHEST;
                        break;
                case C07_THING_TYPE_SCROLL:
                        G2008_i_PanelContent = M643_PANEL_SCROLL;
                        break;
                default:
                        G2008_i_PanelContent = M565_PANEL_FOOD_WATER_POISONED;
                        L1075_T_Thing = C0xFFFF_THING_NONE;
        }
        if (L1075_T_Thing == C0xFFFF_THING_NONE) {
                F0345_INVENTORY_DrawPanel_FoodWaterPoisoned();
        } else {
                F0342_INVENTORY_DrawPanel_Object(L1075_T_Thing, C0_FALSE);
        }
}

STATICFUNCTION void F0348_INVENTORY_AdjustStatisticCurrentValue(
REGISTER CHAMPION*        P0715_ps_Champion       SEPARATOR
REGISTER unsigned int16_t P0716_ui_StatisticIndex SEPARATOR
REGISTER int16_t          P0717_i_ValueDelta      FINAL_SEPARATOR
{
        REGISTER int16_t L1077_i_Multiple;
#define AL1077_i_CurrentValue L1077_i_Multiple
#define AL1077_i_Delta        L1077_i_Multiple


        if (P0717_i_ValueDelta >= 0) {
                if ((AL1077_i_CurrentValue = P0715_ps_Champion->Statistics[P0716_ui_StatisticIndex][C1_CURRENT]) > 120) {
                        P0717_i_ValueDelta >>= 1;
                        if (AL1077_i_CurrentValue > 150) {
                                P0717_i_ValueDelta >>= 1;
                        }
                        P0717_i_ValueDelta++;
                }
                AL1077_i_Delta = F0024_MAIN_GetMinimumValue(P0717_i_ValueDelta, 170 - AL1077_i_CurrentValue);
        } else { /* BUG0_00 Useless code. The function is always called with P0717_i_ValueDelta having a positive value */
                AL1077_i_Delta = F0025_MAIN_GetMaximumValue(P0717_i_ValueDelta, P0715_ps_Champion->Statistics[P0716_ui_StatisticIndex][C2_MINIMUM] - P0715_ps_Champion->Statistics[P0716_ui_StatisticIndex][C1_CURRENT]);
        }
        P0715_ps_Champion->Statistics[P0716_ui_StatisticIndex][C1_CURRENT] += AL1077_i_Delta;
}

void F0349_INVENTORY_ProcessCommand70_ClickOnMouth(
void
)
{
        REGISTER CHAMPION* L1083_ps_Champion;
#define AL1085_ui_PotionPower         L1085_ui_Multiple
#define AL1085_ui_AdjustedPotionPower L1085_ui_Multiple
#define AL1085_ui_Counter             L1085_ui_Multiple
        REGISTER JUNK* L1082_ps_Junk;
        REGISTER unsigned int16_t L1085_ui_Multiple;
        REGISTER unsigned int16_t L1088_ui_Multiple;
        REGISTER unsigned int16_t L1086_ui_Counter;
        REGISTER THING L1078_T_Thing;
        REGISTER BOOLEAN L1081_B_RemoveObjectFromLeaderHand;
        REGISTER unsigned int16_t L1080_ui_ChampionIndex;
        REGISTER unsigned int16_t L1079_ui_IconIndex;
#define AL1088_ui_ThingType               L1088_ui_Multiple
#define AL1088_ui_Mana                    L1088_ui_Multiple
#define AL1088_ui_HealWoundIterationCount L1088_ui_Multiple
        unsigned int16_t L1087_ui_Wounds;
        unsigned int16_t L1089_ui_Weight;
        EVENT L1084_s_Event;


        if (G0415_ui_LeaderEmptyHanded) {
                if (G0424_i_PanelContent == M565_PANEL_FOOD_WATER_POISONED) {
                        return;
                }
                G0597_B_IgnoreMouseMovements = C1_TRUE;
                G0333_B_PressingMouth = C1_TRUE;
                F0706_GetMouseState(&G2010_, &G2011_, &G0588_i_MouseButtonsStatus);
                if (!M007_GET(G0588_i_MouseButtonsStatus, MASK0x0002_MOUSE_LEFT_BUTTON)) {
                        G0597_B_IgnoreMouseMovements = C0_FALSE;
                        T0349001: /* ANOMALY_FMT Label required to disable optimization */
                        G0333_B_PressingMouth = C0_FALSE;
                        T0349002: ; /* ANOMALY_FMT Label required to disable optimization */
                } else {
                        M522_MOUSE_HidePointer();
                        G0587_i_HideMousePointerRequestCount = 1;
                        F0345_INVENTORY_DrawPanel_FoodWaterPoisoned();
                        F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
                }
                return;
        }
        if (G0299_ui_CandidateChampionOrdinal) {
                return;
        }
        if (!M007_GET(G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(L1078_T_Thing = G4055_s_LeaderHandObject.Thing)].AllowedSlots, MASK0x0001_MOUTH)) {
                return;
        }
        L1079_ui_IconIndex = F0033_OBJECT_GetIconIndex(L1078_T_Thing);
        AL1088_ui_ThingType = M012_TYPE(L1078_T_Thing);
        L1089_ui_Weight = F0140_DUNGEON_GetObjectWeight(L1078_T_Thing);
        L1083_ps_Champion = &M516_CHAMPIONS[L1080_ui_ChampionIndex = M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)];
        L1082_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1078_T_Thing);
        if ((L1079_ui_IconIndex >= C008_ICON_JUNK_WATER) && (L1079_ui_IconIndex <= C009_ICON_JUNK_WATERSKIN)) {
                if (!(L1082_ps_Junk->ChargeCount))
                        return;
                L1083_ps_Champion->Water = F0024_MAIN_GetMinimumValue(L1083_ps_Champion->Water + 800, 2048);
                L1082_ps_Junk->ChargeCount--;
                L1081_B_RemoveObjectFromLeaderHand = C0_FALSE;
        } else {
                if (AL1088_ui_ThingType == C08_THING_TYPE_POTION) {
                        L1081_B_RemoveObjectFromLeaderHand = C0_FALSE;
                } else {
                        L1082_ps_Junk->Next = C0xFFFF_THING_NONE;
                        L1081_B_RemoveObjectFromLeaderHand = C1_TRUE;
                }
        }
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        if (L1081_B_RemoveObjectFromLeaderHand) {
                F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
        }
        if (AL1088_ui_ThingType == C08_THING_TYPE_POTION) {
                AL1085_ui_PotionPower = ((POTION*)L1082_ps_Junk)->Power;
                L1086_ui_Counter = ((511 - AL1085_ui_PotionPower) / (32 + (AL1085_ui_PotionPower + 1) / 8)) >> 1;
                AL1085_ui_AdjustedPotionPower = (AL1085_ui_PotionPower / 25) + 8; /* Value between 8 and 18 */
                switch (((POTION*)L1082_ps_Junk)->Type) {
                        case C06_POTION_ROS_POTION:
                                F0348_INVENTORY_AdjustStatisticCurrentValue(L1083_ps_Champion, C2_STATISTIC_DEXTERITY, AL1085_ui_AdjustedPotionPower);
                                break;
                        case C07_POTION_KU_POTION:
                                F0348_INVENTORY_AdjustStatisticCurrentValue(L1083_ps_Champion, C1_STATISTIC_STRENGTH, (((POTION*)L1082_ps_Junk)->Power / 35) + 5); /* Value between 5 and 12 */
                                break;
                        case C08_POTION_DANE_POTION:
                                F0348_INVENTORY_AdjustStatisticCurrentValue(L1083_ps_Champion, C3_STATISTIC_WISDOM, AL1085_ui_AdjustedPotionPower);
                                break;
                        case C09_POTION_NETA_POTION:
                                F0348_INVENTORY_AdjustStatisticCurrentValue(L1083_ps_Champion, C4_STATISTIC_VITALITY, AL1085_ui_AdjustedPotionPower);
                                break;
                        case C10_POTION_ANTIVENIN:
                                F0323_CHAMPION_Unpoison(L1080_ui_ChampionIndex);
                                break;
                        case C11_POTION_MON_POTION:
                                L1083_ps_Champion->CurrentStamina += F0024_MAIN_GetMinimumValue(L1083_ps_Champion->MaximumStamina - L1083_ps_Champion->CurrentStamina, L1083_ps_Champion->MaximumStamina / L1086_ui_Counter);
                                break;
                        case C12_POTION_YA_POTION:
                                AL1085_ui_AdjustedPotionPower += AL1085_ui_AdjustedPotionPower >> 1;
                                if (L1083_ps_Champion->ShieldDefense > 50) {
                                        AL1085_ui_AdjustedPotionPower >>= 2;
                                }
                                L1083_ps_Champion->ShieldDefense += AL1085_ui_AdjustedPotionPower;
                                L1084_s_Event.A.A.Type = C72_EVENT_CHAMPION_SHIELD;
                                M033_SET_MAP_AND_TIME(L1084_s_Event.Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + (AL1085_ui_AdjustedPotionPower * AL1085_ui_AdjustedPotionPower));
                                L1084_s_Event.A.A.Priority = L1080_ui_ChampionIndex;
                                L1084_s_Event.B.Defense = AL1085_ui_AdjustedPotionPower;
                                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L1084_s_Event);
                                M008_SET(L1083_ps_Champion->Attributes, MASK0x1000_STATUS_BOX);
                                break;
                        case C13_POTION_EE_POTION:
                                AL1088_ui_Mana = F0024_MAIN_GetMinimumValue(900, (L1083_ps_Champion->CurrentMana + AL1085_ui_AdjustedPotionPower) + (AL1085_ui_AdjustedPotionPower - 8));
                                if (AL1088_ui_Mana > L1083_ps_Champion->MaximumMana) {
                                        AL1088_ui_Mana -= (AL1088_ui_Mana - F0025_MAIN_GetMaximumValue(L1083_ps_Champion->CurrentMana, L1083_ps_Champion->MaximumMana)) >> 1;
                                }
                                L1083_ps_Champion->CurrentMana = AL1088_ui_Mana;
                                break;
                        case C14_POTION_VI_POTION:
                                AL1088_ui_HealWoundIterationCount = F0025_MAIN_GetMaximumValue(1, (((POTION*)L1082_ps_Junk)->Power / 42));
                                L1083_ps_Champion->CurrentHealth += L1083_ps_Champion->MaximumHealth / L1086_ui_Counter;
                                if (L1087_ui_Wounds = L1083_ps_Champion->Wounds) { /* If the champion is wounded */
                                        L1086_ui_Counter = 10;
                                        do {
                                                for (AL1085_ui_Counter = 0; AL1085_ui_Counter < AL1088_ui_HealWoundIterationCount; AL1085_ui_Counter++) {
                                                        L1083_ps_Champion->Wounds &= M006_RANDOM(65536);
                                                }
                                                AL1088_ui_HealWoundIterationCount = 1;
                                        } while ((L1087_ui_Wounds == L1083_ps_Champion->Wounds) && --L1086_ui_Counter); /* Loop until at least one wound is healed or there are no more heal iterations */
                                }
                                M008_SET(L1083_ps_Champion->Attributes, MASK0x0200_LOAD | MASK0x2000_WOUNDS);
                                break;
                        case C15_POTION_WATER_FLASK:
                                L1083_ps_Champion->Water = F0024_MAIN_GetMinimumValue(L1083_ps_Champion->Water + 1600, 2048);
                                break;
                }
                ((POTION*)L1082_ps_Junk)->Type = C20_POTION_EMPTY_FLASK;
        } else {
                if ((L1079_ui_IconIndex >= C168_ICON_JUNK_APPLE) && (L1079_ui_IconIndex < C176_ICON_JUNK_IRON_KEY)) {
                        L1083_ps_Champion->Food = F0024_MAIN_GetMinimumValue(L1083_ps_Champion->Food + G0242_ai_Graphic559_FoodAmounts[L1079_ui_IconIndex - C168_ICON_JUNK_APPLE], 2048);
                }
        }
        if (L1083_ps_Champion->CurrentStamina > L1083_ps_Champion->MaximumStamina) {
                L1083_ps_Champion->CurrentStamina = L1083_ps_Champion->MaximumStamina;
        }
        if (L1083_ps_Champion->CurrentHealth > L1083_ps_Champion->MaximumHealth) {
                L1083_ps_Champion->CurrentHealth = L1083_ps_Champion->MaximumHealth;
        }
        if (L1081_B_RemoveObjectFromLeaderHand) {
                for (L1086_ui_Counter = 5; --L1086_ui_Counter; ) { /* Animate mouth icon */
                        F0332_INVENTORY_DrawIconToViewport(C205_ICON_MOUTH_OPEN + !(L1086_ui_Counter & 0x0001), C545_ZONE_MOUTH);
                        F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
                        F0022_MAIN_Delay(8);
                }
        } else {
                F0296_CHAMPION_DrawChangedObjectIcons();
                M516_CHAMPIONS[G0411_i_LeaderIndex].Load += F0140_DUNGEON_GetObjectWeight(L1078_T_Thing) - L1089_ui_Weight;
                M008_SET(M516_CHAMPIONS[G0411_i_LeaderIndex].Attributes, MASK0x0200_LOAD);
        }
        F0064_SOUND_RequestPlay_CPSD(C08_SOUND_SWALLOW, G0306_i_PartyMapX, G0307_i_PartyMapY, C00_MODE_PLAY_IMMEDIATELY);
        M008_SET(L1083_ps_Champion->Attributes, MASK0x0100_STATISTICS);
        if (G0424_i_PanelContent == M565_PANEL_FOOD_WATER_POISONED) {
                M008_SET(L1083_ps_Champion->Attributes, MASK0x0800_PANEL);
        }
        F0292_CHAMPION_DrawState(L1080_ui_ChampionIndex);
        F0078_MOUSE_DisableScreenUpdate();
}

void F0350_INVENTORY_DrawStopPressingMouth(
void
)
{
        F0347_INVENTORY_DrawPanel();
        F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
        G0587_i_HideMousePointerRequestCount = 1;
        M523_MOUSE_ShowPointer();
}

void F0351_INVENTORY_DrawChampionSkillsAndStatistics(
void
)
{
        REGISTER CHAMPION* L1094_ps_Champion;
        REGISTER unsigned int16_t L1090_ui_Multiple;
#define AL1090_ui_SkillIndex     L1090_ui_Multiple
#define AL1090_ui_StatisticIndex L1090_ui_Multiple
        REGISTER int16_t L1092_i_Multiple;
#define AL1092_i_SkillLevel            L1092_i_Multiple
#define AL1092_i_StatisticCurrentValue L1092_i_Multiple
        REGISTER int16_t L1095_i_StatisticColor;
        REGISTER unsigned int16_t L1096_ui_StatisticMaximumValue;
        REGISTER unsigned int16_t L1093_ui_ChampionIndex;
        int16_t L2865_i_X;
        int16_t L2866_i_X2;
        int16_t L1091_i_Y;
        char L1097_ac_String[20];
#ifdef X250_I34E /* CHANGE3_08_OPTIMIZATION Trailing space characters removed from strings CHANGE4_00_LOCALIZATION Translation to German language */
        static char* G0431_apc_StatisticNames[7] = { "L", "STRENGTH", "DEXTERITY", "WISDOM", "VITALITY", "ANTI-MAGIC", "ANTI-FIRE" };
#endif
#ifdef X736_I34M
        static int16_t G2017_ai_StatisticNames[7] = { 41, 42, 43, 44, 45, 46, 47 }; /* Text indices */
#endif


        F0334_INVENTORY_CloseChest();
        G0424_i_PanelContent = C02_PANEL_SKILLS_AND_STATISTICS;
        L1094_ps_Champion = &M516_CHAMPIONS[L1093_ui_ChampionIndex = M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)];
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C020_GRAPHIC_PANEL_EMPTY, C101_ZONE_PANEL, C08_COLOR_RED);
        F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C557_ZONE_SKILL_VALUE, &L2865_i_X, &L1091_i_Y);
        for (AL1090_ui_SkillIndex = C00_SKILL_FIGHTER; AL1090_ui_SkillIndex <= C03_SKILL_WIZARD; AL1090_ui_SkillIndex++) {
                AL1092_i_SkillLevel = F0024_MAIN_GetMinimumValue(16, F0303_CHAMPION_GetSkillLevel(L1093_ui_ChampionIndex, AL1090_ui_SkillIndex | MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE));
                if (AL1092_i_SkillLevel == 1)
                        continue;
#ifdef X193_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                M547_STRCPY(L1097_ac_String, G0428_apc_SkillLevelNames[AL1092_i_SkillLevel - 2]);
                M545_STRCAT(L1097_ac_String, " ");
                M545_STRCAT(L1097_ac_String, G0417_apc_BaseSkillNames[AL1090_ui_SkillIndex]);
#endif
#ifdef X736_I34M
                if (G2000_Language == C0_ENGLISH) {
                        M547_STRCPY(L1097_ac_String, F0758_TranslateLanguage(G2014_ai_SkillLevelNames[AL1092_i_SkillLevel - 2]));
                        M545_STRCAT(L1097_ac_String, " ");
                        M545_STRCAT(L1097_ac_String, F0758_TranslateLanguage(G2015_ai_BaseSkillNames[AL1090_ui_SkillIndex]));
                } else {
                        M547_STRCPY(L1097_ac_String, F0758_TranslateLanguage(G2015_ai_BaseSkillNames[AL1090_ui_SkillIndex]));
                        M545_STRCAT(L1097_ac_String, " ");
                        M545_STRCAT(L1097_ac_String, F0758_TranslateLanguage(G2014_ai_SkillLevelNames[AL1092_i_SkillLevel - 2]));
                }
#endif
                F0052_TEXT_PrintToViewport(L2865_i_X, L1091_i_Y, (G2016_ai_SkillRecentlyUpgraded[L1093_ui_ChampionIndex][AL1090_ui_SkillIndex]) ? C07_COLOR_LIGHT_GREEN : C13_COLOR_LIGHTEST_GRAY, L1097_ac_String);
                L1091_i_Y += G2088_C7_TextLineHeight;
        }
        F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(C559_ZONE_STATISTIC_VALUE, &L2866_i_X2, &L1091_i_Y);
        for (AL1090_ui_StatisticIndex = C1_STATISTIC_STRENGTH; AL1090_ui_StatisticIndex <= C6_STATISTIC_ANTIFIRE; AL1090_ui_StatisticIndex++) {
#ifdef X520_I34E
                F0052_TEXT_PrintToViewport(L2865_i_X, L1091_i_Y, C13_COLOR_LIGHTEST_GRAY, G0431_apc_StatisticNames[AL1090_ui_StatisticIndex]);
#endif
#ifdef X736_I34M
                F0052_TEXT_PrintToViewport(L2865_i_X, L1091_i_Y, C13_COLOR_LIGHTEST_GRAY, F0758_TranslateLanguage(G2017_ai_StatisticNames[AL1090_ui_StatisticIndex]));
#endif
                AL1092_i_StatisticCurrentValue = L1094_ps_Champion->Statistics[AL1090_ui_StatisticIndex][C1_CURRENT];
                L1096_ui_StatisticMaximumValue = L1094_ps_Champion->Statistics[AL1090_ui_StatisticIndex][C0_MAXIMUM];
                if (AL1092_i_StatisticCurrentValue < L1096_ui_StatisticMaximumValue) {
                        L1095_i_StatisticColor = C08_COLOR_RED;
                } else {
                        if (AL1092_i_StatisticCurrentValue > L1096_ui_StatisticMaximumValue) {
                                L1095_i_StatisticColor = C07_COLOR_LIGHT_GREEN;
                        } else {
                                L1095_i_StatisticColor = C13_COLOR_LIGHTEST_GRAY;
                        }
                }
                F0052_TEXT_PrintToViewport(L2866_i_X2, L1091_i_Y, L1095_i_StatisticColor, F0288_CHAMPION_GetStringFromInteger(AL1092_i_StatisticCurrentValue, C1_TRUE, 3));
                M547_STRCPY(L1097_ac_String, "/");
                M545_STRCAT(L1097_ac_String, F0288_CHAMPION_GetStringFromInteger(L1096_ui_StatisticMaximumValue, C1_TRUE, 3));
                F0052_TEXT_PrintToViewport(L2866_i_X2  + (G2087_C6_TextCharacterWidth * 3), L1091_i_Y, C13_COLOR_LIGHTEST_GRAY, L1097_ac_String);
                L1091_i_Y += G2088_C7_TextLineHeight;
        }
}

void F0352_INVENTORY_ProcessCommand71_ClickOnEye(
void
)
{
        REGISTER int16_t L1098_i_Unreferenced; /* BUG0_00 Useless code */
        REGISTER int16_t L1099_i_Unreferenced; /* BUG0_00 Useless code */


        G0597_B_IgnoreMouseMovements = C1_TRUE;
        G0331_B_PressingEye = C1_TRUE;
        F0706_GetMouseState(&G2010_, &G2011_, &G0588_i_MouseButtonsStatus);
        if (!M007_GET(G0588_i_MouseButtonsStatus, MASK0x0002_MOUSE_LEFT_BUTTON)) {
                G0597_B_IgnoreMouseMovements = C0_FALSE;
                G0331_B_PressingEye = C0_FALSE;
                return;
        }
        F0357_COMMAND_DiscardAllInput();
        M522_MOUSE_HidePointer();
        F0022_MAIN_Delay(8);
        F0332_INVENTORY_DrawIconToViewport(C203_ICON_EYE_LOOKING, C546_ZONE_EYE);
        if (G0415_ui_LeaderEmptyHanded) {
                F0351_INVENTORY_DrawChampionSkillsAndStatistics();
        } else {
                F0035_OBJECT_ClearLeaderHandObjectName();
                F0342_INVENTORY_DrawPanel_Object(G4055_s_LeaderHandObject.Thing, C1_TRUE);
        }
        F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
}

void F0353_INVENTORY_DrawStopPressingEye(
void
)
{
        REGISTER int16_t L2867_i_;
        REGISTER THING L1100_T_LeaderHandObject;


        F0332_INVENTORY_DrawIconToViewport(C202_ICON_EYE_NOT_LOOKING, C546_ZONE_EYE);
        F0347_INVENTORY_DrawPanel();
        F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
        if ((L1100_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing) == C0xFFFF_THING_NONE) {
                for (L2867_i_ = 0; L2867_i_ <= 3; G2016_ai_SkillRecentlyUpgraded[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)][L2867_i_++] = C0_FALSE);
        } else {
                F0034_OBJECT_DrawLeaderHandObjectName(L1100_T_LeaderHandObject);
        }
        M523_MOUSE_ShowPointer();
}

void F0354_INVENTORY_DrawStatusBoxPortrait(
int16_t P0718_i_ChampionIndex FINAL_SEPARATOR
{
        int16_t L2868_ai_XYZ[4];


        F0638_GetZone(C175_ZONE_FIRST_CHAMPION_STATUS_BOX + P0718_i_ChampionIndex, L2868_ai_XYZ);
        F0132_VIDEO_Blit(M516_CHAMPIONS[P0718_i_ChampionIndex].Portrait, G0348_Bitmap_Screen, L2868_ai_XYZ, 0, 0, G2078_C32_PortraitWidth, G2071_C320_ScreenPixelWidth, CM1_COLOR_NO_TRANSPARENCY, MASK0x0000_NO_FLIP);
        if (G0407_s_Party.Event71Count_Invisibility) {
                F0136_VIDEO_HatchScreenBox(C175_ZONE_FIRST_CHAMPION_STATUS_BOX + P0718_i_ChampionIndex, C12_COLOR_DARKEST_GRAY);
        }
}

void F0355_INVENTORY_Toggle_CPSE(
REGISTER int16_t P0719_i_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1103_ps_Champion;
        REGISTER unsigned int16_t L1102_ui_Multiple;
#define AL1102_ui_InventoryChampionOrdinal L1102_ui_Multiple
#define AL1102_ui_SlotIndex                L1102_ui_Multiple
        REGISTER BOOLEAN L2869_B_;


        L2869_B_ = C0_FALSE;
        if (G2152_B_PressingClosedImaginaryFakeWall)
                return;
        if ((P0719_i_ChampionIndex < C04_CHAMPION_CLOSE_INVENTORY) && !M516_CHAMPIONS[P0719_i_ChampionIndex].CurrentHealth) {
                return;
        }
        if ((P0719_i_ChampionIndex != C05_CHAMPION_SPECIAL_INVENTORY) && (G0333_B_PressingMouth || G0331_B_PressingEye)) {
                return;
        }
        G0321_B_StopWaitingForPlayerInput = C1_TRUE;
        if (P0719_i_ChampionIndex != C05_CHAMPION_SPECIAL_INVENTORY) {
                G0424_i_PanelContent = C00_PANEL_INVENTORY;
        }
        AL1102_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal;
        if (M000_INDEX_TO_ORDINAL(P0719_i_ChampionIndex) == AL1102_ui_InventoryChampionOrdinal) {
                P0719_i_ChampionIndex = C04_CHAMPION_CLOSE_INVENTORY;
        }
        if (!AL1102_ui_InventoryChampionOrdinal || (P0719_i_ChampionIndex == C04_CHAMPION_CLOSE_INVENTORY)) {
                L2869_B_ = C1_TRUE;
                F0077_MOUSE_EnableScreenUpdate_CPSE();
        }
        G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;
        if (AL1102_ui_InventoryChampionOrdinal && (P0719_i_ChampionIndex != C05_CHAMPION_SPECIAL_INVENTORY)) {
                G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);
                F0334_INVENTORY_CloseChest();
                L1103_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(AL1102_ui_InventoryChampionOrdinal)];
                if (L1103_ps_Champion->CurrentHealth && !G0299_ui_CandidateChampionOrdinal) {
                        M008_SET(L1103_ps_Champion->Attributes, MASK0x1000_STATUS_BOX);
                        F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(AL1102_ui_InventoryChampionOrdinal));
                }
                if (G0300_B_PartyIsResting) {
                        if (!L2869_B_) {
                                return;
                        }
                        F0700_TriggerImmediateMouseEvent();
                        F0078_MOUSE_DisableScreenUpdate();
                        return;
                }
                if (P0719_i_ChampionIndex == C04_CHAMPION_CLOSE_INVENTORY) {
                        G0326_B_RefreshMousePointerInMainLoop = C1_TRUE;
                        F0395_MENUS_DrawMovementArrows();
                        if (L2869_B_) {
                                F0700_TriggerImmediateMouseEvent();
                                F0078_MOUSE_DisableScreenUpdate();
                        }
                        G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;
                        G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;
                        F0357_COMMAND_DiscardAllInput();
                        F0098_DUNGEONVIEW_DrawFloorAndCeiling();
                        return;
                }
        }
        if (P0719_i_ChampionIndex == C05_CHAMPION_SPECIAL_INVENTORY) {
                P0719_i_ChampionIndex = M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal);
        } else {
                G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(P0719_i_ChampionIndex);
        }
        if (!AL1102_ui_InventoryChampionOrdinal) {
                F0136_VIDEO_HatchScreenBox(C009_ZONE_MOVEMENT_ARROWS, C00_COLOR_BLACK);
        }
        L1103_ps_Champion = &M516_CHAMPIONS[P0719_i_ChampionIndex];
        F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY, G0296_puc_Bitmap_Viewport);
        if (G0299_ui_CandidateChampionOrdinal) {
                F0734_ClearZoneInInventory(C562_ZONE_SAVE_GAME_ICON);
                F0734_ClearZoneInInventory(C564_ZONE_REST_ICON);
                F0734_ClearZoneInInventory(C566_ZONE_CLOSE_INVENTORY_ICON);
        }
        if (!G2106_B_ShowMusicState) {
                F0734_ClearZoneInInventory(C568_ZONE_TOGGLE_MUSIC_ICON); /* Probable BUGX_XX in P20JB? Should be zone 570! */
        }
#ifdef X561_I34E
        F0648_PrintTextInViewportZone(C547_ZONE_HEALTH_LABEL, C13_COLOR_LIGHTEST_GRAY, "HEALTH");
        F0648_PrintTextInViewportZone(C548_ZONE_STAMINA_LABEL, C13_COLOR_LIGHTEST_GRAY, "STAMINA");
        F0648_PrintTextInViewportZone(C549_ZONE_MANA_LABEL, C13_COLOR_LIGHTEST_GRAY, "MANA");
#endif
#ifdef X736_I34M
        F0648_PrintTextInViewportZone(C547_ZONE_HEALTH_LABEL, C13_COLOR_LIGHTEST_GRAY, F0758_TranslateLanguage(C48_HEALTH));
        F0648_PrintTextInViewportZone(C548_ZONE_STAMINA_LABEL, C13_COLOR_LIGHTEST_GRAY, F0758_TranslateLanguage(C49_STAMINA));
        F0648_PrintTextInViewportZone(C549_ZONE_MANA_LABEL, C13_COLOR_LIGHTEST_GRAY, F0758_TranslateLanguage(C50_MANA));
#endif
        for (AL1102_ui_SlotIndex = C00_SLOT_READY_HAND; AL1102_ui_SlotIndex < C30_SLOT_CHEST_1; AL1102_ui_SlotIndex++) {
                F0291_CHAMPION_DrawSlot(P0719_i_ChampionIndex, AL1102_ui_SlotIndex);
        }
        M008_SET(L1103_ps_Champion->Attributes, MASK0x4000_VIEWPORT | MASK0x1000_STATUS_BOX | MASK0x0800_PANEL | MASK0x0400_ICON | MASK0x0200_LOAD | MASK0x0100_STATISTICS | MASK0x0080_NAME_TITLE);
        F0292_CHAMPION_DrawState(P0719_i_ChampionIndex);
        if (L2869_B_) {
                G0598_B_MousePointerBitmapUpdated = C1_TRUE;
                F0700_TriggerImmediateMouseEvent();
                F0078_MOUSE_DisableScreenUpdate();
        }
        G0442_ps_SecondaryMouseInput = G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory;
        G0444_ps_SecondaryKeyboardInput = NULL;
        F0357_COMMAND_DiscardAllInput();
}
/* END PANEL.C */

/* BEGIN CHAMDRAW.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#ifdef X520_I34E
char* G0417_apc_BaseSkillNames[4] = { "FIGHTER", "NINJA", "PRIEST", "WIZARD" };
#endif
#ifdef X736_I34M
int16_t G2015_ai_BaseSkillNames[4] = { 0, 1, 3, 2 };
#endif
BOOLEAN G2149_;
BOOLEAN G2016_ai_SkillRecentlyUpgraded[4][4];
unsigned char G2362_auc_PaletteChanges_Invisibility[16] = { 0, 1, 2, 0, 4, 0, 6, 7, 8, 9, 0, 11, 12, 13, 14, 15 };


void F0621_ClearChampionIconBox(
int16_t P2057_i_ChampionIndex FINAL_SEPARATOR
{
        F0733_FillZoneByIndex(P2057_i_ChampionIndex + C113_ZONE_CHAMPION_ICON_TOP_LEFT, C00_COLOR_BLACK);
}

unsigned char* F0622_PrepareChampionIconBitmap(
unsigned char* P2058_pc_ChampionIconBitmap SEPARATOR
int16_t        P2059_i_ChampionIndex       FINAL_SEPARATOR
{


        M100_PIXEL_WIDTH(P2058_pc_ChampionIconBitmap) = G2080_C19_ChampionIconWidth;
        M101_PIXEL_HEIGHT(P2058_pc_ChampionIconBitmap) = G2081_C14_ChampionIconHeight;
        F0134_VIDEO_FillBitmap(P2058_pc_ChampionIconBitmap, G0407_s_Party.Event71Count_Invisibility ? C01_COLOR_DARK_GRAY : G0046_auc_Graphic562_ChampionColor[P2059_i_ChampionIndex], G2080_C19_ChampionIconWidth, G2081_C14_ChampionIconHeight);
        F0654_Call_F0132_VIDEO_Blit(F0489_MEMORY_GetNativeBitmapOrGraphic(C028_GRAPHIC_CHAMPION_ICONS), P2058_pc_ChampionIconBitmap, F0627_GetTemporaryZoneInitializedFromDimensions(G2080_C19_ChampionIconWidth, G2081_C14_ChampionIconHeight), M026_CHAMPION_ICON_INDEX(M516_CHAMPIONS[P2059_i_ChampionIndex].Direction, G0308_i_PartyDirection) * G2080_C19_ChampionIconWidth, 0, C12_COLOR_DARKEST_GRAY);
        if (G0407_s_Party.Event71Count_Invisibility) {
                F0662_ApplyPaletteChanges(M772_CAST_PC(P2058_pc_ChampionIconBitmap), G2362_auc_PaletteChanges_Invisibility);
        }
        return P2058_pc_ChampionIconBitmap;
}

STATICFUNCTION void F0287_CHAMPION_DrawBarGraphs(
int16_t P0605_i_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0845_ps_Multiple;
#define AL0845_ps_Champion       L0845_ps_Multiple
#define AL0845_pi_BarGraphHeight L0845_ps_Multiple
        REGISTER int16_t L2251_i_Counter;
        REGISTER int16_t L2252_i_BoxIndex;
        int16_t L2254_i_Bars[3][2];
        int16_t L2004_ai_XYZBlankBar[4];
        int16_t L2005_ai_XYZColoredBar[4];


        AL0845_ps_Champion = &M516_CHAMPIONS[P0605_i_ChampionIndex];
        L2254_i_Bars[0][0] = AL0845_ps_Champion->CurrentHealth;
        L2254_i_Bars[0][1] = AL0845_ps_Champion->MaximumHealth;
        L2254_i_Bars[1][0] = AL0845_ps_Champion->CurrentStamina;
        L2254_i_Bars[1][1] = AL0845_ps_Champion->MaximumStamina;
        L2254_i_Bars[2][0] = AL0845_ps_Champion->CurrentMana;
        L2254_i_Bars[2][1] = AL0845_ps_Champion->MaximumMana;
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        L2252_i_BoxIndex = P0605_i_ChampionIndex + C195_ZONE_FIRST_BAR_GRAPH;
        for (L2251_i_Counter = 0; L2251_i_Counter < 3; L2251_i_Counter++, L2252_i_BoxIndex += 4) {
                F0638_GetZone(L2252_i_BoxIndex, L2005_ai_XYZColoredBar);
                if (L2254_i_Bars[L2251_i_Counter][0] < L2254_i_Bars[L2251_i_Counter][1]) {
                        F0007_MAIN_CopyBytes((char*)L2005_ai_XYZColoredBar, (char*)L2004_ai_XYZBlankBar, (long)sizeof(L2004_ai_XYZBlankBar));
                        if (L2254_i_Bars[L2251_i_Counter][0] != 0) {
                                M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar) -= F0025_MAIN_GetMaximumValue(1, (int16_t)(((long)M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar) * (long)L2254_i_Bars[L2251_i_Counter][0]) / (long)L2254_i_Bars[L2251_i_Counter][1]));
                        }
                        if (M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar) > 0) {
                                F0732_FillScreenArea(L2004_ai_XYZBlankBar, C12_COLOR_DARKEST_GRAY);
                                M706_ZONE_TOP(L2005_ai_XYZColoredBar) = M706_ZONE_TOP(L2004_ai_XYZBlankBar) + M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar);
                                M709_ZONE_HEIGHT(L2005_ai_XYZColoredBar) -= M709_ZONE_HEIGHT(L2004_ai_XYZBlankBar);
                        }
                }
                if ((L2254_i_Bars[L2251_i_Counter][0] != 0) && (M709_ZONE_HEIGHT(L2005_ai_XYZColoredBar) > 0)) {
                        F0732_FillScreenArea(L2005_ai_XYZColoredBar, G0046_auc_Graphic562_ChampionColor[P0605_i_ChampionIndex]);
                }
        }
        F0078_MOUSE_DisableScreenUpdate();
}

char* F0288_CHAMPION_GetStringFromInteger(
REGISTER unsigned int16_t P0606_ui_Integer               SEPARATOR
REGISTER BOOLEAN          P0607_B_Padding                SEPARATOR
unsigned int16_t          P0608_ui_PaddingCharacterCount FINAL_SEPARATOR
{
        REGISTER char* L0850_pc_Character;
        REGISTER int16_t L0849_i_Digit;
        static char G0419_ac_IntegerToStringConversionBuffer[6];


        if (P0607_B_Padding) {
                F0009_MAIN_WriteSpacedBytes(G0419_ac_IntegerToStringConversionBuffer, 4, ' ', sizeof(char));
        }
        L0850_pc_Character = &G0419_ac_IntegerToStringConversionBuffer[4];
        *L0850_pc_Character = '\0';
        if (!P0606_ui_Integer) {
                *(--L0850_pc_Character) = '0';
        } else {
                while (L0849_i_Digit = P0606_ui_Integer) {
                        *(--L0850_pc_Character) = '0' + L0849_i_Digit - ((P0606_ui_Integer /= 10) * 10);
                }
        }
        if (P0607_B_Padding) {
                return &G0419_ac_IntegerToStringConversionBuffer[4 - P0608_ui_PaddingCharacterCount];
        }
        else {
                return L0850_pc_Character;
        }
}

STATICFUNCTION void F0289_CHAMPION_DrawHealthOrStaminaOrManaValue(
int16_t P0609_i_ZoneIndex    SEPARATOR
int16_t P0610_i_CurrentValue SEPARATOR
int16_t P0611_i_MaximumValue FINAL_SEPARATOR
{
        char ValueString[8];


        M547_STRCPY(ValueString, F0288_CHAMPION_GetStringFromInteger(P0610_i_CurrentValue, C1_TRUE, 3));
        M545_STRCAT(ValueString, "/");
        M545_STRCAT(ValueString, F0288_CHAMPION_GetStringFromInteger(P0611_i_MaximumValue, C1_TRUE, 3));
        F0648_PrintTextInViewportZone(P0609_i_ZoneIndex, C13_COLOR_LIGHTEST_GRAY, ValueString);
}

STATICFUNCTION void F0290_CHAMPION_DrawHealthStaminaManaValues(
REGISTER CHAMPION* P0612_ps_Champion FINAL_SEPARATOR
{
        F0289_CHAMPION_DrawHealthOrStaminaOrManaValue(C550_ZONE_HEALTH_VALUE, P0612_ps_Champion->CurrentHealth, P0612_ps_Champion->MaximumHealth);
        F0289_CHAMPION_DrawHealthOrStaminaOrManaValue(C551_ZONE_MANA_VALUE, P0612_ps_Champion->CurrentStamina / 10, P0612_ps_Champion->MaximumStamina / 10);
        F0289_CHAMPION_DrawHealthOrStaminaOrManaValue(C552_ZONE_STAMINA_VALUE, P0612_ps_Champion->CurrentMana, P0612_ps_Champion->MaximumMana);
}

void F0291_CHAMPION_DrawSlot(
unsigned int16_t          P0613_ui_ChampionIndex SEPARATOR
REGISTER unsigned int16_t P0614_ui_SlotIndex     FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0854_ps_Champion;
        REGISTER unsigned char* L0860_puc_Bitmap;
        REGISTER int16_t L0853_i_IconIndex;
        REGISTER int16_t L0859_i_NativeBitmapIndex = -1;
        REGISTER BOOLEAN L0852_B_IsInventoryChampion;
        REGISTER THING L0851_T_Thing;
        unsigned int16_t L0856_ui_SlotBoxIndex;
        int16_t L0861_i_DestinationByteWidth;
        int16_t L2255_ai_XYZ[4];


        L0854_ps_Champion = &M516_CHAMPIONS[P0613_ui_ChampionIndex];
        L0852_B_IsInventoryChampion = (G0423_i_InventoryChampionOrdinal == M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex));
        if (!L0852_B_IsInventoryChampion) { /* If drawing a slot for a champion other than the champion whose inventory is open */
                if ((P0614_ui_SlotIndex > C01_SLOT_ACTION_HAND) || (G0299_ui_CandidateChampionOrdinal == M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex))) {
                        return;
                }
                L0856_ui_SlotBoxIndex = (P0613_ui_ChampionIndex << 1) + P0614_ui_SlotIndex;
        } else {
                L0856_ui_SlotBoxIndex = C08_SLOT_BOX_INVENTORY_FIRST_SLOT + P0614_ui_SlotIndex;
        }
        if (P0614_ui_SlotIndex >= C30_SLOT_CHEST_1) {
                L0851_T_Thing = G0425_aT_ChestSlots[P0614_ui_SlotIndex - C30_SLOT_CHEST_1];
        } else {
                L0851_T_Thing = L0854_ps_Champion->Slots[P0614_ui_SlotIndex];
        }
        F0619_GetSlotBoxBorderCoordinates(G0030_as_Graphic562_SlotBoxes[L0856_ui_SlotBoxIndex].ZoneIndex, L2255_ai_XYZ);
        if (L0852_B_IsInventoryChampion) {
                L0860_puc_Bitmap = G0296_puc_Bitmap_Viewport;
                L0861_i_DestinationByteWidth = G2073_C224_ViewportPixelWidth;
        } else {
                L0860_puc_Bitmap = G0348_Bitmap_Screen;
                L0861_i_DestinationByteWidth = G2071_C320_ScreenPixelWidth;
                F0077_MOUSE_EnableScreenUpdate_CPSE();
        }
        if (L0851_T_Thing == C0xFFFF_THING_NONE) {
                if (P0614_ui_SlotIndex <= C05_SLOT_FEET) {
                        L0853_i_IconIndex = C212_ICON_READY_HAND + (P0614_ui_SlotIndex << 1);
                        if (M007_GET(L0854_ps_Champion->Wounds, 1 << P0614_ui_SlotIndex)) {
                                L0853_i_IconIndex++;
                                L0859_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;
                        } else {
#ifdef PC_FIX_CODE_SIZE
        L0853_i_IconIndex++;
        L0853_i_IconIndex++;
        L0853_i_IconIndex++;
#endif
                                L0859_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;
                        }
                } else {
                        if ((P0614_ui_SlotIndex >= C10_SLOT_NECK) && (P0614_ui_SlotIndex <= C13_SLOT_BACKPACK_LINE1_1)) {
                                L0853_i_IconIndex = C208_ICON_NECK + (P0614_ui_SlotIndex - C10_SLOT_NECK);
                        } else {
                                L0853_i_IconIndex = C204_ICON_EMPTY_BOX;
                        }
                }
        } else {
                L0853_i_IconIndex = F0033_OBJECT_GetIconIndex(L0851_T_Thing); /* BUG0_35 The closed chest icon is drawn when the chest is opened. If you open the inventory of a champion and place a chest in the action hand, the closed chest icon is drawn instead of the open chest icon. If you place a Chest in the action hand before opening the inventory (in the champion status box on top of the screen) then when you open the inventory the open chest icon is drawn correctly in the action hand (by F0333_INVENTORY_OpenAndDrawChest). Code is missing to change the icon when a chest is drawn in the action hand while in the inventory */
                if (L0852_B_IsInventoryChampion && (P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND) && ((L0853_i_IconIndex == C144_ICON_CONTAINER_CHEST_CLOSED)
                   )) {
                        L0853_i_IconIndex++;
                }
                if (P0614_ui_SlotIndex <= C05_SLOT_FEET) {
                                if (M007_GET(L0854_ps_Champion->Wounds, 1 << P0614_ui_SlotIndex)) {
                                        L0859_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;
                                } else {
                                        L0859_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;
                                }
                }
        }
        if ((P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND) && (M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex) == G0506_ui_ActingChampionOrdinal)) {
                L0859_i_NativeBitmapIndex = C035_GRAPHIC_SLOT_BOX_ACTING_HAND;
        }
        if (L0859_i_NativeBitmapIndex != -1) {
                L0854_ps_Champion = (CHAMPION*)F0489_MEMORY_GetNativeBitmapOrGraphic(L0859_i_NativeBitmapIndex);
                F0132_VIDEO_Blit((unsigned char*)L0854_ps_Champion, L0860_puc_Bitmap, L2255_ai_XYZ, 0, 0, M100_PIXEL_WIDTH((int16_t*)L0854_ps_Champion), L0861_i_DestinationByteWidth, C12_COLOR_DARKEST_GRAY, MASK0x0000_NO_FLIP);
        }
        F0038_OBJECT_DrawIconInSlotBox(L0856_ui_SlotBoxIndex, L0853_i_IconIndex);
        if (!L0852_B_IsInventoryChampion) {
                F0078_MOUSE_DisableScreenUpdate();
        }
}

void F0623_DrawDamageToChampion_F0320_sub(
unsigned int16_t P2060_ui_ChampionIndex SEPARATOR
int16_t P2061_i_Damage                  FINAL_SEPARATOR
{
        REGISTER int16_t L2257_i_ZoneIndex;
        REGISTER int16_t L2258_i_GraphicIndex;


        if (M000_INDEX_TO_ORDINAL(P2060_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal) {
                L2258_i_GraphicIndex = C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG;
                L2257_i_ZoneIndex = C179_ZONE_FIRST_DAMAGE_TO_CHAMPION_BIG;
        } else {
                L2258_i_GraphicIndex = C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL;
                L2257_i_ZoneIndex = C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL;
        }
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0660_(L2258_i_GraphicIndex, L2257_i_ZoneIndex += P2060_ui_ChampionIndex, C10_COLOR_FLESH);
        F0650_PrintCenteredTextToScreenZone(L2257_i_ZoneIndex, C15_COLOR_WHITE, C08_COLOR_RED, F0288_CHAMPION_GetStringFromInteger(P2061_i_Damage, C0_FALSE, 3));
        F0292_CHAMPION_DrawState(P2060_ui_ChampionIndex);
        F0078_MOUSE_DisableScreenUpdate();
}

void F0292_CHAMPION_DrawState(
REGISTER unsigned int16_t P0615_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0865_ps_Champion;
        REGISTER char* L0866_pc_ChampionName;
        REGISTER int16_t L0864_i_Multiple;
#define AL0864_i_BorderCount       L0864_i_Multiple
#define AL0864_i_ColorIndex        L0864_i_Multiple
#define AL0864_i_Load              L0864_i_Multiple
#define AL0864_i_ChampionIconIndex L0864_i_Multiple
#define AL0864_i_StatisticIndex    L0864_i_Multiple
#define AL0864_i_SlotIndex         L0864_i_Multiple
        REGISTER unsigned int16_t L0862_ui_ChampionAttributes;
        REGISTER int16_t L0870_i_Multiple;
#define AL0870_i_NativeBitmapIndex L0870_i_Multiple
#define AL0870_i_Color             L0870_i_Multiple
        REGISTER BOOLEAN L0863_B_IsInventoryChampion;
        BOOLEAN L2259_B_MousePointerHidden;
        char L0867_c_ChampionTitleFirstCharacter;
        int16_t L2260_ai_XYZ[4];
        int16_t L0872_ai_NativeBitmapIndices[3];
        char L2261_ac_String[28];


        L2259_B_MousePointerHidden = C0_FALSE;
        L0865_ps_Champion = &M516_CHAMPIONS[P0615_ui_ChampionIndex];
        L0862_ui_ChampionAttributes = L0865_ps_Champion->Attributes;
        if (!M007_GET(L0862_ui_ChampionAttributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND))
                return;
        L0863_B_IsInventoryChampion = (M000_INDEX_TO_ORDINAL(P0615_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal);
        if (L0863_B_IsInventoryChampion && G0297_B_DrawFloorAndCeilingRequested) {
                F0355_INVENTORY_Toggle_CPSE(C05_CHAMPION_SPECIAL_INVENTORY);
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x1000_STATUS_BOX)) {
                F0638_GetZone(P0615_ui_ChampionIndex + C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS, L2260_ai_XYZ);
                L2259_B_MousePointerHidden = C1_TRUE;
                F0077_MOUSE_EnableScreenUpdate_CPSE();
                if (L0865_ps_Champion->CurrentHealth) {
                        F0732_FillScreenArea(L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY);
                        F0008_MAIN_ClearBytes(M772_CAST_PC(L0872_ai_NativeBitmapIndices), M543_BYTE_COUNT_INT(sizeof(L0872_ai_NativeBitmapIndices)));
                        AL0864_i_BorderCount = 0;
                        if (G0407_s_Party.FireShieldDefense > 0) {
                                L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = C038_GRAPHIC_BORDER_PARTY_FIRESHIELD;
                        }
                        if (G0407_s_Party.SpellShieldDefense > 0) {
                                L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD;
                        }
                        if ((G0407_s_Party.ShieldDefense > 0) || L0865_ps_Champion->ShieldDefense) {
                                L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount++] = C037_GRAPHIC_BORDER_PARTY_SHIELD;
                        }
                        while (AL0864_i_BorderCount--) {
                                F0659_(L0872_ai_NativeBitmapIndices[AL0864_i_BorderCount], L2260_ai_XYZ, C10_COLOR_FLESH);
                        }
                        if (L0863_B_IsInventoryChampion) {
                                F0354_INVENTORY_DrawStatusBoxPortrait(P0615_ui_ChampionIndex);
                                M008_SET(L0862_ui_ChampionAttributes, MASK0x0100_STATISTICS);
                        } else {
                                M008_SET(L0862_ui_ChampionAttributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x2000_WOUNDS | MASK0x8000_ACTION_HAND);
                        }
                } else {
                        F0659_(C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION, L2260_ai_XYZ, CM1_COLOR_NO_TRANSPARENCY);
                        F0650_PrintCenteredTextToScreenZone(P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME, C13_COLOR_LIGHTEST_GRAY, C01_COLOR_DARK_GRAY, L0865_ps_Champion->Name);
                        F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex);
                }
        }
        if (!(L0865_ps_Champion->CurrentHealth))
                goto T0292042;
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0080_NAME_TITLE)) {
                AL0864_i_ColorIndex = (P0615_ui_ChampionIndex == G0411_i_LeaderIndex) ? C11_COLOR_YELLOW : C09_COLOR_GOLD;
                if (L0863_B_IsInventoryChampion) {
                        M547_STRCPY(L2261_ac_String, L0866_pc_ChampionName = L0865_ps_Champion->Name);
                        L0867_c_ChampionTitleFirstCharacter = L0865_ps_Champion->Title[0];
                        if ((L0867_c_ChampionTitleFirstCharacter != ',') && (L0867_c_ChampionTitleFirstCharacter != ';') && (L0867_c_ChampionTitleFirstCharacter != '-')) {
                                M545_STRCAT(L2261_ac_String, " ");
                        }
                        M545_STRCAT(L2261_ac_String, L0865_ps_Champion->Title);
                        F0648_PrintTextInViewportZone(C553_ZONE_CHAMPION_NAME_AND_TITLE, AL0864_i_ColorIndex, L2261_ac_String);
                        M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
                } else {
                        if (!L2259_B_MousePointerHidden) {
                                L2259_B_MousePointerHidden = C1_TRUE;
                                F0077_MOUSE_EnableScreenUpdate_CPSE();
                        }
                        F0733_FillZoneByIndex(P0615_ui_ChampionIndex + C159_ZONE_CHAMPION_0_STATUS_BOX_NAME, C01_COLOR_DARK_GRAY);
                        F0650_PrintCenteredTextToScreenZone(P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME, AL0864_i_ColorIndex, C01_COLOR_DARK_GRAY, L0865_ps_Champion->Name);
                }
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0100_STATISTICS)) {
                if (!L2259_B_MousePointerHidden) {
                        L2259_B_MousePointerHidden = C1_TRUE;
                        F0077_MOUSE_EnableScreenUpdate_CPSE();
                }
                F0287_CHAMPION_DrawBarGraphs(P0615_ui_ChampionIndex);
                if (L0863_B_IsInventoryChampion) {
                        F0290_CHAMPION_DrawHealthStaminaManaValues(L0865_ps_Champion);
                        if ((L0865_ps_Champion->Food < 0) || (L0865_ps_Champion->Water < 0) || (L0865_ps_Champion->PoisonEventCount)) {
                                AL0870_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;
                        } else {
                                AL0870_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;
                        }
                        F0619_GetSlotBoxBorderCoordinates(C545_ZONE_MOUTH, L2260_ai_XYZ);
                        F0657_BlitBitmapIndexToViewportZoneWithTransparency(AL0870_i_NativeBitmapIndex, L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY);
                        AL0870_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;
                        for (AL0864_i_StatisticIndex = C1_STATISTIC_STRENGTH; AL0864_i_StatisticIndex <= C6_STATISTIC_ANTIFIRE; AL0864_i_StatisticIndex++) {
                                if ((L0865_ps_Champion->Statistics[AL0864_i_StatisticIndex][C1_CURRENT] < L0865_ps_Champion->Statistics[AL0864_i_StatisticIndex][C0_MAXIMUM])) {
                                        AL0870_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;
                                        break;
                                }
                        }
                        F0619_GetSlotBoxBorderCoordinates(C546_ZONE_EYE, L2260_ai_XYZ);
                        F0657_BlitBitmapIndexToViewportZoneWithTransparency(AL0870_i_NativeBitmapIndex, L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY);
                        M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
                }
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x2000_WOUNDS)) {
                if (L0863_B_IsInventoryChampion) {
                        M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
                }
                else {
                        if (!L2259_B_MousePointerHidden) {
                                L2259_B_MousePointerHidden = C1_TRUE;
                                F0077_MOUSE_EnableScreenUpdate_CPSE();
                        }
                }
                for (AL0864_i_SlotIndex = L0863_B_IsInventoryChampion ? C05_SLOT_FEET : C01_SLOT_ACTION_HAND; AL0864_i_SlotIndex >= C00_SLOT_READY_HAND; AL0864_i_SlotIndex--) {
                        F0291_CHAMPION_DrawSlot(P0615_ui_ChampionIndex, AL0864_i_SlotIndex);
                }
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0200_LOAD) && L0863_B_IsInventoryChampion) {
                if (L0865_ps_Champion->Load > (AL0864_i_Load = F0309_CHAMPION_GetMaximumLoad(L0865_ps_Champion))) {
                        AL0870_i_Color = C08_COLOR_RED;
                } else {
                        if (((long)L0865_ps_Champion->Load << 3) > ((long)AL0864_i_Load * 5)) {
                                AL0870_i_Color = C11_COLOR_YELLOW;
                        } else {
                                AL0870_i_Color = C13_COLOR_LIGHTEST_GRAY;
                        }
                }
#ifdef X561_I34E
                F0648_PrintTextInViewportZone(C554_ZONE_CHAMPION_LOAD_LABEL, AL0870_i_Color, "LOAD ");
#endif
#ifdef X736_I34M
                F0648_PrintTextInViewportZone(C554_ZONE_CHAMPION_LOAD_LABEL, AL0870_i_Color, F0758_TranslateLanguage(C36_CHAMPION_LOAD_LABEL));
#endif
                AL0864_i_Load = L0865_ps_Champion->Load / 10;
                M547_STRCPY(G0353_ac_StringBuildBuffer, F0288_CHAMPION_GetStringFromInteger(AL0864_i_Load, C1_TRUE, 3));
#ifdef X193_I34E /* CHANGE4_00_LOCALIZATION Translation to German language */
                M545_STRCAT(G0353_ac_StringBuildBuffer, ".");
#endif
#ifdef X736_I34M
                M545_STRCAT(G0353_ac_StringBuildBuffer, F0758_TranslateLanguage(C38_CHAMPION_LOAD_DECIMAL_SEPARATOR));
#endif
                AL0864_i_Load = L0865_ps_Champion->Load - (AL0864_i_Load * 10);
                M545_STRCAT(G0353_ac_StringBuildBuffer, F0288_CHAMPION_GetStringFromInteger(AL0864_i_Load, C0_FALSE, 1));
                M545_STRCAT(G0353_ac_StringBuildBuffer, "/");
                AL0864_i_Load = (F0309_CHAMPION_GetMaximumLoad(L0865_ps_Champion) + 5) / 10;
                M545_STRCAT(G0353_ac_StringBuildBuffer, F0288_CHAMPION_GetStringFromInteger(AL0864_i_Load, C1_TRUE, 3));
#ifdef X188_I34E /* CHANGE5_00_LOCALIZATION Translation to French language */
                M545_STRCAT(G0353_ac_StringBuildBuffer, " KG");
#endif
#ifdef X736_I34M
                M545_STRCAT(G0353_ac_StringBuildBuffer, F0758_TranslateLanguage(C40_CHAMPION_LOAD_KILOGRAM));
#endif
                F0648_PrintTextInViewportZone(C555_ZONE_CHAMPION_LOAD_VALUE, AL0870_i_Color, G0353_ac_StringBuildBuffer);
                M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
        }
        AL0864_i_ChampionIconIndex = M026_CHAMPION_ICON_INDEX(L0865_ps_Champion->Cell, G0308_i_PartyDirection);
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0400_ICON) && (G0599_ui_UseChampionIconOrdinalAsMousePointerBitmap != M000_INDEX_TO_ORDINAL(AL0864_i_ChampionIconIndex))) {
                if (!L2259_B_MousePointerHidden) {
                        L2259_B_MousePointerHidden = C1_TRUE;
                        F0077_MOUSE_EnableScreenUpdate_CPSE();
                }
                L0866_pc_ChampionName = M772_CAST_PC(F0606_AllocateMemForGraphic(G2080_C19_ChampionIconWidth, G2081_C14_ChampionIconHeight, C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP));
                F0622_PrepareChampionIconBitmap((unsigned char*)L0866_pc_ChampionName, P0615_ui_ChampionIndex);
                F0621_ClearChampionIconBox(AL0864_i_ChampionIconIndex);
                F0021_MAIN_BlitToScreen(M774_CAST_PUC(L0866_pc_ChampionName), AL0864_i_ChampionIconIndex + C113_ZONE_CHAMPION_ICON_TOP_LEFT, CM1_COLOR_NO_TRANSPARENCY);
                F0607_FreeMemForGraphic(M774_CAST_PUC(L0866_pc_ChampionName));
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0800_PANEL) && L0863_B_IsInventoryChampion) {
                if (G0333_B_PressingMouth) {
                        F0345_INVENTORY_DrawPanel_FoodWaterPoisoned();
                } else {
                        if (G0331_B_PressingEye) {
                                if (G0415_ui_LeaderEmptyHanded) {
                                        F0351_INVENTORY_DrawChampionSkillsAndStatistics();
                                }
                        } else {
                                F0347_INVENTORY_DrawPanel();
                        }
                }
                M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x8000_ACTION_HAND)) {
                if (!L2259_B_MousePointerHidden) {
                        L2259_B_MousePointerHidden = C1_TRUE;
                        F0077_MOUSE_EnableScreenUpdate_CPSE();
                }
                F0291_CHAMPION_DrawSlot(P0615_ui_ChampionIndex, C01_SLOT_ACTION_HAND);
                F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex);
                if (L0863_B_IsInventoryChampion) {
                        M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);
                }
        }
        if (M007_GET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT)) {
                if (G2106_B_ShowMusicState) {
                        if (G2024_B_PendingMusicOn) {
                                F0658_BlitBitmapIndexToZoneIndexWithTransparency(M713_NEGGRAPHIC_MUSIC_ICON, M701_ZONE_TOGGLE_MUSIC_ICON, CM1_COLOR_NO_TRANSPARENCY);
                        } else {
                                F0734_ClearZoneInInventory(M701_ZONE_TOGGLE_MUSIC_ICON);
                        }
                }
                F0097_DUNGEONVIEW_DrawViewport(C0_VIEWPORT_NOT_DUNGEON_VIEW);
        }
        T0292042:
        M009_CLEAR(L0865_ps_Champion->Attributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND);
        if (L2259_B_MousePointerHidden)
                F0078_MOUSE_DisableScreenUpdate();
}

void F0293_CHAMPION_DrawAllChampionStates(
unsigned int16_t P2062_ui_ FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0873_ui_ChampionIndex;


        for (L0873_ui_ChampionIndex = C00_CHAMPION_FIRST; L0873_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0873_ui_ChampionIndex++) {
                M516_CHAMPIONS[L0873_ui_ChampionIndex].Attributes |= P2062_ui_;
                F0292_CHAMPION_DrawState(L0873_ui_ChampionIndex);
        }
        G2149_ = C0_FALSE;
}


BOOLEAN G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen;

STATICFUNCTION BOOLEAN F0295_CHAMPION_HasObjectIconInSlotBoxChanged(
int16_t          P0619_i_SlotBoxIndex SEPARATOR
THING            P0620_T_Thing        FINAL_SEPARATOR
{
        REGISTER int16_t L0880_i_CurrentIconIndex;
        REGISTER int16_t L0881_i_NewIconIndex;


        L0880_i_CurrentIconIndex = F0039_OBJECT_GetIconIndexInSlotBox(P0619_i_SlotBoxIndex);
        if (((L0880_i_CurrentIconIndex < C032_ICON_WEAPON_DAGGER) && (L0880_i_CurrentIconIndex >= C000_ICON_JUNK_COMPASS_NORTH)) ||
            ((L0880_i_CurrentIconIndex >= C148_ICON_POTION_MA_POTION_MON_POTION) && (L0880_i_CurrentIconIndex <= C163_ICON_POTION_WATER_FLASK)) ||
            (L0880_i_CurrentIconIndex == C195_ICON_POTION_EMPTY_FLASK)) {
                L0881_i_NewIconIndex = F0033_OBJECT_GetIconIndex(P0620_T_Thing);
                if (L0881_i_NewIconIndex != L0880_i_CurrentIconIndex) {
                        if ((P0619_i_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT) && !G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen) {
                                G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen = C1_TRUE;
                                F0077_MOUSE_EnableScreenUpdate_CPSE();
                        }
                        F0038_OBJECT_DrawIconInSlotBox(P0619_i_SlotBoxIndex, L0881_i_NewIconIndex);
                        return C1_TRUE;
                }
        }
        return C0_FALSE;
}

/* This function draws visible object icons that have changed (in the leader hand, champion hands, champion inventory slots and open chest slots) */
void F0296_CHAMPION_DrawChangedObjectIcons(
void
)
{
        REGISTER THING* L0886_pT_Thing;
        REGISTER CHAMPION* L0887_ps_Champion;
        REGISTER unsigned int16_t L0882_ui_Counter;
#define AL0882_ui_SlotBoxIndex L0882_ui_Counter
#define AL0882_ui_SlotIndex    L0882_ui_Counter
        REGISTER unsigned int16_t L0883_ui_InventoryChampionOrdinal;
        REGISTER int16_t L0884_i_Multiple;
#define AL0884_i_LeaderHandObjectIconIndex L0884_i_Multiple
#define AL0884_B_DrawViewport              L0884_i_Multiple
        REGISTER int16_t L0885_i_ChampionIndex;
        REGISTER int16_t L0888_i_IconIndex;
        unsigned int16_t L0889_ui_ObjectIconChanged;


        L0883_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal;
        if (G0299_ui_CandidateChampionOrdinal && !L0883_ui_InventoryChampionOrdinal) {
                return;
        }
        G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen = C0_FALSE;
        if ((((AL0884_i_LeaderHandObjectIconIndex = G4055_s_LeaderHandObject.IconIndex) < C032_ICON_WEAPON_DAGGER) && ((AL0884_i_LeaderHandObjectIconIndex = G4055_s_LeaderHandObject.IconIndex) >= C000_ICON_JUNK_COMPASS_NORTH)) || (((AL0884_i_LeaderHandObjectIconIndex = G4055_s_LeaderHandObject.IconIndex) >= C148_ICON_POTION_MA_POTION_MON_POTION) && ((AL0884_i_LeaderHandObjectIconIndex = G4055_s_LeaderHandObject.IconIndex) <= C163_ICON_POTION_WATER_FLASK)) || ((AL0884_i_LeaderHandObjectIconIndex = G4055_s_LeaderHandObject.IconIndex) == C195_ICON_POTION_EMPTY_FLASK)) {
                L0888_i_IconIndex = F0033_OBJECT_GetIconIndex(G4055_s_LeaderHandObject.Thing);
                if (L0888_i_IconIndex != AL0884_i_LeaderHandObjectIconIndex) {
                        G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen = C1_TRUE;
                        F0077_MOUSE_EnableScreenUpdate_CPSE();
                        F0036_OBJECT_ExtractIconFromBitmap(L0888_i_IconIndex, M775_CAST_PL(G4055_s_LeaderHandObject.BitmapIconForMousePointer));
                        F0068_MOUSE_SetPointerToObject(G4055_s_LeaderHandObject.BitmapIconForMousePointer);
                        G4055_s_LeaderHandObject.IconIndex = L0888_i_IconIndex;
                        F0034_OBJECT_DrawLeaderHandObjectName(G4055_s_LeaderHandObject.Thing);
                }
        }
        for (AL0882_ui_SlotBoxIndex = 0; AL0882_ui_SlotBoxIndex < (G0305_ui_PartyChampionCount << 1); AL0882_ui_SlotBoxIndex++) {
                L0885_i_ChampionIndex = AL0882_ui_SlotBoxIndex >> 1;
                if (L0883_ui_InventoryChampionOrdinal == M000_INDEX_TO_ORDINAL(L0885_i_ChampionIndex))
                        continue;
                if ((F0295_CHAMPION_HasObjectIconInSlotBoxChanged(AL0882_ui_SlotBoxIndex, M516_CHAMPIONS[L0885_i_ChampionIndex].Slots[M070_HAND_SLOT_INDEX(AL0882_ui_SlotBoxIndex)])) && (M070_HAND_SLOT_INDEX(AL0882_ui_SlotBoxIndex) == C01_SLOT_ACTION_HAND)) {
                        F0386_MENUS_DrawActionIcon(L0885_i_ChampionIndex);
                }
        }
        if (L0883_ui_InventoryChampionOrdinal) {
                L0887_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)];
                L0886_pT_Thing = &L0887_ps_Champion->Slots[AL0882_ui_SlotIndex = C00_SLOT_READY_HAND];
                for (AL0884_B_DrawViewport = C0_FALSE; AL0882_ui_SlotIndex < C30_SLOT_CHEST_1; AL0882_ui_SlotIndex++, L0886_pT_Thing++) {
                        AL0884_B_DrawViewport |= (L0889_ui_ObjectIconChanged = F0295_CHAMPION_HasObjectIconInSlotBoxChanged(AL0882_ui_SlotIndex + C08_SLOT_BOX_INVENTORY_FIRST_SLOT, *L0886_pT_Thing));
                        if (L0889_ui_ObjectIconChanged && (AL0882_ui_SlotIndex == C01_SLOT_ACTION_HAND)) {
                                F0386_MENUS_DrawActionIcon(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal));
                        }
                }
                if (G2008_i_PanelContent == M569_PANEL_CHEST) {
                        L0886_pT_Thing = G0425_aT_ChestSlots;
                        for (AL0882_ui_SlotIndex = 0; AL0882_ui_SlotIndex < 8; AL0882_ui_SlotIndex++, L0886_pT_Thing++) {
                                AL0884_B_DrawViewport |= F0295_CHAMPION_HasObjectIconInSlotBoxChanged(AL0882_ui_SlotIndex + C38_SLOT_BOX_CHEST_FIRST_SLOT, *L0886_pT_Thing);
                        }
                }
                if (AL0884_B_DrawViewport) {
                        M008_SET(L0887_ps_Champion->Attributes, MASK0x4000_VIEWPORT);
                        F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal));
                }
        }
        if (G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen) {
                F0078_MOUSE_DisableScreenUpdate();
        }
}

/* END CHAMDRAW.C */

/* BEGIN ACTIDRAW.C */
unsigned char G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 };


char* F0384_MENUS_GetActionName(
REGISTER unsigned char P0758_uc_ActionIndex FINAL_SEPARATOR
{
        REGISTER char* L1173_pc_Character;


        if (P0758_uc_ActionIndex == C0xFF_ACTION_NONE) {
                return "";
        }
        L1173_pc_Character = G0490_ac_Graphic560_ActionNames;
        while (P0758_uc_ActionIndex--) {
                while (*L1173_pc_Character++);
        }
        return L1173_pc_Character;
}

void F0385_MENUS_DrawActionDamage(
REGISTER int16_t P0759_i_Damage FINAL_SEPARATOR
{
        REGISTER unsigned char* L1177_puc_Bitmap;
        REGISTER unsigned int16_t L1174_ui_Multiple;
#define AL1174_ui_DerivedBitmapIndex L1174_ui_Multiple
#define AL1174_ui_CharacterIndex     L1174_ui_Multiple
        REGISTER int16_t L1176_i_Multiple;
#define AL1176_i_X          L1176_i_Multiple
#define AL1176_i_PixelWidth L1176_i_Multiple
        REGISTER int16_t L3017_i_Height;
        char L1179_ac_String[6];


        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0733_FillZoneByIndex(C011_ZONE_ACTION_AREA, C00_COLOR_BLACK);
        if (P0759_i_Damage < 0) {
#ifdef X561_I34E
                F0650_PrintCenteredTextToScreenZone(C075_ZONE_ACTION_RESULT, C04_COLOR_CYAN, C00_COLOR_BLACK, (P0759_i_Damage == CM1_DAMAGE_CANT_REACH) ? "CAN'T REACH" : "NEED AMMO");
#endif
#ifdef X736_I34M
                F0650_PrintCenteredTextToScreenZone(C075_ZONE_ACTION_RESULT, C04_COLOR_CYAN, C00_COLOR_BLACK, (P0759_i_Damage == CM1_DAMAGE_CANT_REACH) ? F0758_TranslateLanguage(C71_CANT_REACH) : F0758_TranslateLanguage(C72_NEED_AMMO));
#endif
        } else {
                if (P0759_i_Damage > 40) {
                        L1177_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(C014_GRAPHIC_DAMAGE_TO_CREATURE);
                } else {
                        if (P0759_i_Damage > 15) {
                                AL1174_ui_DerivedBitmapIndex = C002_DERIVED_BITMAP_DAMAGE_TO_CREATURE_MEDIUM;
                                AL1176_i_PixelWidth = G2093_DamageToCreatureMediumWidth;
                                L3017_i_Height = G2094_DamageToCreatureMediumHeight;
                        } else {
                                AL1174_ui_DerivedBitmapIndex = C003_DERIVED_BITMAP_DAMAGE_TO_CREATURE_SMALL;
                                AL1176_i_PixelWidth = G2095_DamageToCreatureSmallWidth;
                                L3017_i_Height = G2096_DamageToCreatureSmallHeight;
                        }
                        L1177_puc_Bitmap = F0661_GetShrinkedBitmap(C014_GRAPHIC_DAMAGE_TO_CREATURE, AL1174_ui_DerivedBitmapIndex, AL1176_i_PixelWidth, L3017_i_Height, NULL);
                }
                F0021_MAIN_BlitToScreen(L1177_puc_Bitmap, C075_ZONE_ACTION_RESULT, CM1_COLOR_NO_TRANSPARENCY);
                /* Convert damage value to string */
                AL1174_ui_CharacterIndex = 5;
                L1179_ac_String[5] = '\0';
                do {
                        L1179_ac_String[--AL1174_ui_CharacterIndex] = '0' + (P0759_i_Damage % 10);
                } while (P0759_i_Damage /= 10);
                F0650_PrintCenteredTextToScreenZone(C075_ZONE_ACTION_RESULT, C04_COLOR_CYAN, C00_COLOR_BLACK, &L1179_ac_String[AL1174_ui_CharacterIndex]);
        }
        F0078_MOUSE_DisableScreenUpdate();
}

void F0386_MENUS_DrawActionIcon(
unsigned int16_t P0760_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER unsigned char* L1182_puc_Bitmap_Icon;
        REGISTER CHAMPION* L1183_ps_Champion;
        REGISTER THING L1181_ui_Multiple;
#define AL1181_T_Thing      L1181_ui_Multiple
#define AL1181_ui_IconIndex L1181_ui_Multiple


        if (!G0509_B_ActionAreaContainsIcons) {
                return;
        }
        L1183_ps_Champion = &M516_CHAMPIONS[P0760_ui_ChampionIndex];
        if (!L1183_ps_Champion->CurrentHealth) {
                F0733_FillZoneByIndex(P0760_ui_ChampionIndex + C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, C00_COLOR_BLACK);
                return;
        }
        L1182_puc_Bitmap_Icon = F0606_AllocateMemForGraphic(G2075_ObjectIconWidth, G2076_ObjectIconHeight, C0_ALLOCATION_TEMPORARY_ON_TOP_OF_HEAP);
        if ((AL1181_T_Thing = L1183_ps_Champion->Slots[C01_SLOT_ACTION_HAND]) == C0xFFFF_THING_NONE) {
                AL1181_ui_IconIndex = C201_ICON_ACTION_ICON_EMPTY_HAND;
        } else {
                if (G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(AL1181_T_Thing)].ActionSetIndex) {
                        AL1181_ui_IconIndex = F0033_OBJECT_GetIconIndex(AL1181_T_Thing);
                } else {
                        F0134_VIDEO_FillBitmap(M772_CAST_PC(L1182_puc_Bitmap_Icon), C04_COLOR_CYAN, M100_PIXEL_WIDTH(L1182_puc_Bitmap_Icon) = G2075_ObjectIconWidth, M101_PIXEL_HEIGHT(L1182_puc_Bitmap_Icon) = G2076_ObjectIconHeight);
                        goto T0386006;
                }
        }
        F0036_OBJECT_ExtractIconFromBitmap(AL1181_ui_IconIndex, M775_CAST_PL(L1182_puc_Bitmap_Icon));
        F0662_ApplyPaletteChanges(M772_CAST_PC(L1182_puc_Bitmap_Icon), G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon);
        T0386006:
        F0733_FillZoneByIndex(P0760_ui_ChampionIndex + C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, C04_COLOR_CYAN);
        F0021_MAIN_BlitToScreen(L1182_puc_Bitmap_Icon, P0760_ui_ChampionIndex + C093_ZONE_ACTION_AREA_CHAMPION_0_ACTION_ICON, CM1_COLOR_NO_TRANSPARENCY);
        if (M007_GET(L1183_ps_Champion->Attributes, MASK0x0008_DISABLE_ACTION) || G0299_ui_CandidateChampionOrdinal || G0300_B_PartyIsResting) {
                F0136_VIDEO_HatchScreenBox(P0760_ui_ChampionIndex + C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, C00_COLOR_BLACK);
        }
        F0607_FreeMemForGraphic(L1182_puc_Bitmap_Icon);
}

void F0387_MENUS_DrawActionArea(
void
)
{
        REGISTER unsigned int16_t L1186_ui_Multiple;
#define AL1186_ui_ChampionIndex   L1186_ui_Multiple
#define AL1186_ui_ActionListIndex L1186_ui_Multiple
#define AL1186_ui_ZoneIndex       L1186_ui_Multiple


        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0733_FillZoneByIndex(C011_ZONE_ACTION_AREA, C00_COLOR_BLACK);
        if (G0509_B_ActionAreaContainsIcons) {
                for (AL1186_ui_ChampionIndex = C00_CHAMPION_FIRST; AL1186_ui_ChampionIndex < G0305_ui_PartyChampionCount; F0386_MENUS_DrawActionIcon(AL1186_ui_ChampionIndex++));
        } else {
                if (G0506_ui_ActingChampionOrdinal) {
                        AL1186_ui_ZoneIndex = C011_ZONE_ACTION_AREA;
                        if (G0713_s_ActionList.ActionIndices[2] == C0xFF_ACTION_NONE) {
                                AL1186_ui_ZoneIndex = C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU;
                        }
                        if (G0713_s_ActionList.ActionIndices[1] == C0xFF_ACTION_NONE) {
                                AL1186_ui_ZoneIndex = C079_ZONE_ACTION_AREA_ONE_ACTION_MENU;
                        }
                        F0660_(C010_GRAPHIC_MENU_ACTION_AREA, AL1186_ui_ZoneIndex, CM1_COLOR_NO_TRANSPARENCY);
                        M722_F0768_TEXT_PrintToZoneWithTrailingSpaces(G0348_Bitmap_Screen, G2071_C320_ScreenPixelWidth, 80, C00_COLOR_BLACK, C04_COLOR_CYAN, M516_CHAMPIONS[M001_ORDINAL_TO_INDEX((unsigned int16_t)G0506_ui_ActingChampionOrdinal)].Name, C007_CHAMPION_NAME_MAXIMUM_LENGTH, G2072_C200_ScreenPixelHeight);
                        for (AL1186_ui_ActionListIndex = 0; AL1186_ui_ActionListIndex < 3; AL1186_ui_ActionListIndex++) {
                                M722_F0768_TEXT_PrintToZoneWithTrailingSpaces(G0348_Bitmap_Screen, G2071_C320_ScreenPixelWidth, 85 + AL1186_ui_ActionListIndex, C04_COLOR_CYAN, C00_COLOR_BLACK, F0384_MENUS_GetActionName(G0713_s_ActionList.ActionIndices[AL1186_ui_ActionListIndex]), C012_ACTION_NAME_MAXIMUM_LENGTH, G2072_C200_ScreenPixelHeight);
                        }
                }
        }
        F0078_MOUSE_DisableScreenUpdate();
        G0508_B_RefreshActionArea = C0_FALSE;
}
/* END ACTIDRAW.C */

/* BEGIN CASTER.C */
void F0394_MENUS_SetMagicCasterAndDrawSpellArea(
REGISTER int16_t P0765_i_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1213_ps_Champion;

        if ((P0765_i_ChampionIndex == G0514_i_MagicCasterChampionIndex) || ((P0765_i_ChampionIndex != CM1_CHAMPION_NONE) && !M516_CHAMPIONS[P0765_i_ChampionIndex].CurrentHealth)) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                return;
        }
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        G0514_i_MagicCasterChampionIndex = P0765_i_ChampionIndex;
        if (P0765_i_ChampionIndex == CM1_CHAMPION_NONE) {
                F0733_FillZoneByIndex(C013_ZONE_SPELL_AREA, C00_COLOR_BLACK);
        } else {
                L1213_ps_Champion = &M516_CHAMPIONS[P0765_i_ChampionIndex];
                F0660_(C009_GRAPHIC_MENU_SPELL_AREA_LINES, C013_ZONE_SPELL_AREA, CM1_COLOR_NO_TRANSPARENCY);
                F0393_MENUS_DrawSpellAreaControls(P0765_i_ChampionIndex);
                F0397_MENUS_DrawAvailableSymbols(L1213_ps_Champion->SymbolStep);
                F0398_MENUS_DrawChampionSymbols(L1213_ps_Champion);
        }
        F0078_MOUSE_DisableScreenUpdate();
}
/* END CASTER.C */

/* BEGIN CHEST.C */
void F0333_INVENTORY_OpenAndDrawChest(
THING               P0692_T_Thing       SEPARATOR
CONTAINER*          P0693_ps_Container  SEPARATOR
BOOLEAN             P0694_B_PressingEye FINAL_SEPARATOR
{
        REGISTER int16_t L1017_i_ChestSlotIndex;
        REGISTER THING L1018_T_Thing;
        REGISTER int16_t L1019_i_ThingCount;


        G0424_i_PanelContent = M569_PANEL_CHEST;
        if (G0426_T_OpenChest == P0692_T_Thing)
                return;
                F0334_INVENTORY_CloseChest();
        G0426_T_OpenChest = P0692_T_Thing;
        if (!P0694_B_PressingEye) {
                F0038_OBJECT_DrawIconInSlotBox(C09_SLOT_BOX_INVENTORY_ACTION_HAND, C145_ICON_CONTAINER_CHEST_OPEN);
        }
        F0658_BlitBitmapIndexToZoneIndexWithTransparency(C025_GRAPHIC_PANEL_OPEN_CHEST, C101_ZONE_PANEL, C08_COLOR_RED);
        L1017_i_ChestSlotIndex = 0;
        L1018_T_Thing = P0693_ps_Container->Slot;
        L1019_i_ThingCount = 0;
        while (L1018_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if (++L1019_i_ThingCount > 8) {
                        break;
                }
                F0038_OBJECT_DrawIconInSlotBox(L1017_i_ChestSlotIndex + C38_SLOT_BOX_CHEST_FIRST_SLOT, F0033_OBJECT_GetIconIndex(L1018_T_Thing));
                G0425_aT_ChestSlots[L1017_i_ChestSlotIndex++] = L1018_T_Thing;
                L1018_T_Thing = F0159_DUNGEON_GetNextThing(L1018_T_Thing);
        }
        while (L1017_i_ChestSlotIndex < 8) {
                F0038_OBJECT_DrawIconInSlotBox(L1017_i_ChestSlotIndex + C38_SLOT_BOX_CHEST_FIRST_SLOT, C0xFFFF_ICON_NONE);
                G0425_aT_ChestSlots[L1017_i_ChestSlotIndex++] = C0xFFFF_THING_NONE;
        }
}

void F0334_INVENTORY_CloseChest(
void
)
{
        REGISTER CONTAINER* L1027_ps_Container;
        REGISTER GENERIC* L1028_ps_Generic;
        REGISTER int16_t L1025_i_ChestSlotIndex;
        REGISTER THING L1023_T_Thing;
        REGISTER THING L1024_T_PreviousThing;
        REGISTER BOOLEAN L1026_B_ProcessFirstChestSlot;


        L1026_B_ProcessFirstChestSlot = C1_TRUE;
        if (G0426_T_OpenChest == C0xFFFF_THING_NONE)
                return;
        L1027_ps_Container = (CONTAINER*)F0156_DUNGEON_GetThingData(G0426_T_OpenChest);
        G0426_T_OpenChest = C0xFFFF_THING_NONE;
        L1027_ps_Container->Slot = C0xFFFE_THING_ENDOFLIST;
        for (L1025_i_ChestSlotIndex = 0; L1025_i_ChestSlotIndex < 8; L1025_i_ChestSlotIndex++) {
                if ((L1023_T_Thing = G0425_aT_ChestSlots[L1025_i_ChestSlotIndex]) != C0xFFFF_THING_NONE) {
                        G0425_aT_ChestSlots[L1025_i_ChestSlotIndex] = C0xFFFF_THING_NONE;
                        if (L1026_B_ProcessFirstChestSlot) {
                                L1026_B_ProcessFirstChestSlot = C0_FALSE;
                                L1028_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L1023_T_Thing);
                                L1028_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;
                                L1027_ps_Container->Slot = L1024_T_PreviousThing = L1023_T_Thing;
                        } else {
                                F0163_DUNGEON_LinkThingToList(L1023_T_Thing, L1024_T_PreviousThing, CM1_MAPX_NOT_ON_A_SQUARE, 0);
                                L1024_T_PreviousThing = L1023_T_Thing;
                        }
                }
        }
}
/* END CHEST.C */

/* BEGIN CLIKCHAM.C */
STATICFUNCTION void F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox(
unsigned int16_t P0736_ui_ChampionIndex SEPARATOR
int16_t          P0737_i_X              SEPARATOR
int16_t          P0738_i_Y              FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L1126_ui_Command;


        if (M000_INDEX_TO_ORDINAL(P0736_ui_ChampionIndex) == G0423_i_InventoryChampionOrdinal) {
                F0368_COMMAND_SetLeader(P0736_ui_ChampionIndex);
        } else {
                L1126_ui_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0455_as_Graphic561_MouseInput_ChampionNamesHands, P0737_i_X, P0738_i_Y, MASK0x0002_MOUSE_LEFT_BUTTON);
                if ((L1126_ui_Command >= C016_COMMAND_SET_LEADER_CHAMPION_0) && (L1126_ui_Command <= C019_COMMAND_SET_LEADER_CHAMPION_3)) {
                        F0368_COMMAND_SetLeader(L1126_ui_Command - C016_COMMAND_SET_LEADER_CHAMPION_0);
                } else {
                        if ((L1126_ui_Command >= C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND) && (L1126_ui_Command <= C027_COMMAND_CLICK_ON_SLOT_BOX_07_CHAMPION_3_STATUS_BOX_ACTION_HAND)) {
                                F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(L1126_ui_Command - C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND);
                        }
                }
        }
}

void F0368_COMMAND_SetLeader(
REGISTER int16_t P0739_i_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1129_ps_Champion;
        REGISTER unsigned int16_t L1127_ui_LeaderIndex;


        if ((P0739_i_ChampionIndex == G0411_i_LeaderIndex) || ((P0739_i_ChampionIndex != CM1_CHAMPION_NONE) && !M516_CHAMPIONS[P0739_i_ChampionIndex].CurrentHealth)) {
                return;
        }
        if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                M008_SET(M516_CHAMPIONS[L1127_ui_LeaderIndex = G0411_i_LeaderIndex].Attributes, MASK0x0200_LOAD | MASK0x0080_NAME_TITLE);
                M516_CHAMPIONS[L1127_ui_LeaderIndex].Load -= F0140_DUNGEON_GetObjectWeight(G4055_s_LeaderHandObject.Thing);
                G0411_i_LeaderIndex = CM1_CHAMPION_NONE;
                F0292_CHAMPION_DrawState(L1127_ui_LeaderIndex);
        }
        if (P0739_i_ChampionIndex == CM1_CHAMPION_NONE) {
                return;
        }
        L1129_ps_Champion = &M516_CHAMPIONS[G0411_i_LeaderIndex = P0739_i_ChampionIndex];
        L1129_ps_Champion->Direction = G0308_i_PartyDirection;
        M516_CHAMPIONS[P0739_i_ChampionIndex].Load += F0140_DUNGEON_GetObjectWeight(G4055_s_LeaderHandObject.Thing);
        if (M000_INDEX_TO_ORDINAL(P0739_i_ChampionIndex) != G0299_ui_CandidateChampionOrdinal) {
                M008_SET(L1129_ps_Champion->Attributes, MASK0x0400_ICON | MASK0x0200_LOAD | MASK0x0080_NAME_TITLE);
                F0292_CHAMPION_DrawState(P0739_i_ChampionIndex);
        }
}
/* END CLIKCHAM.C */

/* BEGIN SYMBOL.C */
void F0399_MENUS_AddChampionSymbol(
int16_t P0768_i_SymbolIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L1225_ps_Champion;
        REGISTER unsigned int16_t L1222_ui_SymbolStep;
        REGISTER unsigned int16_t L1223_ui_ManaCost;


        L1225_ps_Champion = &M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex];
        L1222_ui_SymbolStep = L1225_ps_Champion->SymbolStep;
        L1223_ui_ManaCost = G0485_aauc_Graphic560_SymbolBaseManaCost[L1222_ui_SymbolStep][P0768_i_SymbolIndex];
        if (L1222_ui_SymbolStep)
                L1223_ui_ManaCost = (L1223_ui_ManaCost * G0486_auc_Graphic560_SymbolManaCostMultiplier[L1225_ps_Champion->Symbols[0] - 96]) >> 3;
        if (L1223_ui_ManaCost <= L1225_ps_Champion->CurrentMana) {
                L1225_ps_Champion->CurrentMana -= L1223_ui_ManaCost;
                M008_SET(L1225_ps_Champion->Attributes, MASK0x0100_STATISTICS);
                L1225_ps_Champion->Symbols[L1222_ui_SymbolStep] = 96 + (L1222_ui_SymbolStep * 6) + P0768_i_SymbolIndex;
                L1225_ps_Champion->Symbols[L1222_ui_SymbolStep + 1] = '\0';
                L1225_ps_Champion->SymbolStep = L1222_ui_SymbolStep = (L1222_ui_SymbolStep + 1) % 4; /* Also works for _A20ED_A20E_A20F_A20G_A21E_A22E_A22G, maybe also others ? */
                F0077_MOUSE_EnableScreenUpdate_CPSE();
                F0397_MENUS_DrawAvailableSymbols(L1222_ui_SymbolStep);
                F0398_MENUS_DrawChampionSymbols(L1225_ps_Champion);
                F0292_CHAMPION_DrawState(G0514_i_MagicCasterChampionIndex);
                F0078_MOUSE_DisableScreenUpdate();
        }
}

void F0400_MENUS_DeleteChampionSymbol(
void
)
{
        REGISTER CHAMPION* L1228_ps_Champion;
        REGISTER unsigned int16_t L1226_ui_SymbolStep;


        L1228_ps_Champion = &M516_CHAMPIONS[G0514_i_MagicCasterChampionIndex];
        if (!M544_STRLEN(M772_CAST_PC(L1228_ps_Champion->Symbols))) { /* ANOMALY_ST_COMPARISON_WITH_ZERO */
                return;
        }
        L1228_ps_Champion->SymbolStep = L1226_ui_SymbolStep = M108_PREVIOUS(L1228_ps_Champion->SymbolStep);
        L1228_ps_Champion->Symbols[L1226_ui_SymbolStep] = '\0';
        F0077_MOUSE_EnableScreenUpdate_CPSE();
        F0397_MENUS_DrawAvailableSymbols(L1226_ui_SymbolStep);
        F0398_MENUS_DrawChampionSymbols(L1228_ps_Champion);
        F0078_MOUSE_DisableScreenUpdate();
}
/* END SYMBOL.C */

/* BEGIN CHAMPRST.C */
void F0278_CHAMPION_ResetDataToStartGame(
void
)
{
        REGISTER CHAMPION* L0788_ps_Champion;
        REGISTER int16_t L0785_i_ChampionIndex;
        REGISTER THING L0787_T_Thing;


        if (!G0298_B_NewGame) {
                if ((L0787_T_Thing = G4055_s_LeaderHandObject.Thing) == C0xFFFF_THING_NONE) {
                        G0415_ui_LeaderEmptyHanded = C1_TRUE;
                        G4055_s_LeaderHandObject.IconIndex = C0xFFFF_ICON_NONE;
                        F0069_MOUSE_SetPointer();
                } else {
                        L0785_i_ChampionIndex = G0411_i_LeaderIndex;
                        G0411_i_LeaderIndex = CM1_CHAMPION_NONE;
                        F0297_CHAMPION_PutObjectInLeaderHand(L0787_T_Thing, C1_TRUE); /* This call will add the weight of the leader hand object to the Load of the leader a first time */
                        G0411_i_LeaderIndex = L0785_i_ChampionIndex;
                }
                L0788_ps_Champion = M516_CHAMPIONS;
                for (L0785_i_ChampionIndex = C00_CHAMPION_FIRST; L0785_i_ChampionIndex < G0305_ui_PartyChampionCount; L0785_i_ChampionIndex++, L0788_ps_Champion++) {
                        M009_CLEAR(L0788_ps_Champion->Attributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND);
                }
                F0293_CHAMPION_DrawAllChampionStates(MASK0x8000_ACTION_HAND | MASK0x1000_STATUS_BOX | MASK0x0400_ICON);
                if ((L0785_i_ChampionIndex = G0411_i_LeaderIndex) != CM1_CHAMPION_NONE) {
                        G0411_i_LeaderIndex = CM1_CHAMPION_NONE;
                        F0368_COMMAND_SetLeader(L0785_i_ChampionIndex); /* BUG0_32 The load of a champion with no objects in his inventory is not 0. If you save the game with an object in the leader hand then when you resume or restart the game the weight of the object in the leader hand is permanently added to the load of the leader. When the game is saved, the weight of the leader hand object is substracted from the load of the leader. But when the game is loaded, the weight of the leader hand object is added twice to the load of the leader: a first time by the call to F0297_CHAMPION_PutObjectInLeaderHand above and a second time by this call to F0368_COMMAND_SetLeader */
                }
                if ((L0785_i_ChampionIndex = G0514_i_MagicCasterChampionIndex) != CM1_CHAMPION_NONE) {
                        G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE;
                        F0394_MENUS_SetMagicCasterAndDrawSpellArea(L0785_i_ChampionIndex);
                }
                return;
        }
        G4055_s_LeaderHandObject.Thing = C0xFFFF_THING_NONE;
        G4055_s_LeaderHandObject.IconIndex = C0xFFFF_ICON_NONE;
        G0415_ui_LeaderEmptyHanded = C1_TRUE;
}
/* END CHAMPRST.C */

/* BEGIN NEWMAP.C */
extern void F0742_MUSIC_SetTrack(
int P2947_i_MapIndex
);

void F0003_MAIN_ProcessNewPartyMap_CPSE(
int16_t P0000_i_MapIndex FINAL_SEPARATOR
{


        F0742_MUSIC_SetTrack(P0000_i_MapIndex);
        if (!G2136_B_EvaluatingMemoryRequirements) {
                F0194_GROUP_RemoveAllActiveGroups(); /* BUG0_63 The behavior of groups and the directions of individual creatures in groups are not restored when a saved game is loaded. Each group on the same map as the party has additional data that other groups do not have. This additional data is stored as part of saved games and loaded when the game is restarted. However, after loading a saved game the function to initialize this data is called. This overwrites the data and thus the state of groups when the game was saved is ignored */
        }
        F0174_DUNGEON_SetCurrentMapAndPartyMap(P0000_i_MapIndex);
        F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF();
        if (!G2136_B_EvaluatingMemoryRequirements) {
                F0195_GROUP_AddAllActiveGroups(); /* BUG0_63 */
        }
        F0337_INVENTORY_SetDungeonViewPalette();
}
/* END NEWMAP.C */

/* BEGIN PORTRAIT.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif

#if (EXETYPE == C06_CEDT)
#endif
/* END PORTRAIT.C */

/* BEGIN BMPSIZE.C */
/* Returns the amount of memory required to store a scaled bitmap */
int16_t F0459_START_GetScaledBitmapByteCount(
int16_t          P0867_i_ByteWidth SEPARATOR
int16_t          P0868_i_Height    SEPARATOR
REGISTER int16_t P0869_i_Scale     FINAL_SEPARATOR
{
        return M103_BITMAP_BYTE_COUNT(M078_SCALED_DIMENSION(P0867_i_ByteWidth, P0869_i_Scale), M078_SCALED_DIMENSION(P0868_i_Height, P0869_i_Scale));
}
/* END BMPSIZE.C */

/* BEGIN COORD.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
/* Types declared in TOWNSGLB.H for FM-Towns for global variable ordering */
typedef struct {
        unsigned int16_t RecordType;
        int16_t ParentRecordIndex;
        int16_t Data1;
        int16_t Data2;
} LAYOUT_RECORD;

typedef struct layout_range {
        struct layout_range* NextRange;
        int16_t FirstIndex;
        int16_t LastIndex;
        LAYOUT_RECORD Records[];
} LAYOUT_RANGE;

typedef struct {
        unsigned char s3m1; /* Type. 1: s3u contains a graphic index, 2: s3u contains a pointer to a bitmap */
        char s3m2;
        int16_t X;
        int16_t Y;
        int16_t Width; /* If 0, get the width of the bitmap */
        int16_t Height; /* If 0, get the height of the bitmap */
        union {
                int16_t GraphicIndex;
                unsigned char* Bitmap;
        } s3u;
} STRUCT3;


extern unsigned char* F0789_AllocateLayoutRange();

STRUCT3 G2002_NegativeBitmaps[45] = {
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  0 = 65535 -  -1 Floor */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  1 = 65535 -  -2 Ceiling */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  2 = 65535 -  -3 Wall D3L2 */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  3 = 65535 -  -4 Wall D3R2 */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  4 = 65535 -  -5 Wall D3C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  5 = 65535 -  -6 Wall D3L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  6 = 65535 -  -7 Wall D3R */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  7 = 65535 -  -8 Wall D2L2 */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  8 = 65535 -  -9 Wall D2R2 */
        {  2, 0,   0,   0,  0,  0, NULL },                      /*  9 = 65535 - -10 Wall D2C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 10 = 65535 - -11 Wall D2L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 11 = 65535 - -12 Wall D2R */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 12 = 65535 - -13 Wall D1C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 13 = 65535 - -14 Wall D1L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 14 = 65535 - -15 Wall D1R */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 15 = 65535 - -16 Wall D0L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 16 = 65535 - -17 Wall D0R */
        {  1, 0,  50,   0,  7,  0, NULL },                      /* 17 = 65535 - -18 Stairs Up D3L2 */
        {  1, 0,   6,   0,  7,  0, NULL },                      /* 18 = 65535 - -19 Stairs Up D3R2 */
        {  1, 0,  62,   0,  7,  0, NULL },                      /* 19 = 65535 - -20 Stairs Down D3L2 */
        {  1, 0,   6,   0,  7,  0, NULL },                      /* 20 = 65535 - -21 Stairs Down D3R2 */
        {  2, 0,   7,   0,  4,  0, NULL },                      /* 21 = 65535 - -22 Door Frame Right D3L */
        {  2, 0,   6,   0,  4,  0, NULL },                      /* 22 = 65535 - -23 Door Frame Left D3R */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 23 = 65535 - -24 Door Frame Left D3L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 24 = 65535 - -25 Door Frame Left D3C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 25 = 65535 - -26 Door Frame Left D2C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 26 = 65535 - -27 Door Frame Left D1C */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 27 = 65535 - -28 */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 28 = 65535 - -29 Door Frame Front D0C*/
        {  2, 0,  16,   0, 60,  0, NULL },                      /* 29 = 65535 - -30 Door Frame Top D2L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 30 = 65535 - -31 Door Frame Top D2LCR*/
        {  2, 0,  16,   0, 60,  0, NULL },                      /* 31 = 65535 - -32 Door Frame Top D2R */
        {  2, 0,  16,   0, 32,  0, NULL },                      /* 32 = 65535 - -33 Door Frame Top D1L */
        {  2, 0,   0,   0,  0,  0, NULL },                      /* 33 = 65535 - -34 Door Frame Top D1LCR*/
        {  2, 0,  16,   0, 32,  0, NULL },                      /* 34 = 65535 - -35 Door Frame Top D1R */
        {  2, 0,  90,  28,  4, 26, NULL },                      /* 35 = 65535 - -36 */
        {  1, 0,  32,   0, 32,  0, C041_GRAPHIC_HOLE_IN_WALL }, /* 36 = 65535 - -37 Hole in wall */
        {  2, 0,  64,  19, 96, 95, NULL },                      /* 37 = 65535 - -38 Thieves Eye visible area */
        {  2, 0,   0,  14,  0, 75, NULL },                      /* 38 = 65535 - -39 Dialog Patch 1 choice */
        {  2, 0, 102,  52, 21, 37, NULL },                      /* 39 = 65535 - -40 Dialog Patch 2 choices */
        {  2, 0, 102,  99, 21, 36, NULL },                      /* 40 = 65535 - -41 Dialog Patch 4 choices */
        {  1, 0, 168,   3,  9,  9, C017_GRAPHIC_INVENTORY },    /* 41 = 65535 - -42 Music icon */
        {  2, 0,   0, 137,  0, 16, NULL },                      /* 42 = 65535 - -43 */
        {  2, 0,   0,   0,  0, 80, NULL },                      /* 43 = 65535 - -44 */
        {  2, 0,   0,  80,  0, 57, NULL } };                    /* 44 = 65535 - -45 */
int16_t G2067_i_ViewportScreenX = 0;
int16_t G2068_i_ViewportScreenY = 33;
unsigned int16_t G2069_LargestCacheableNativeBitmapByteCount = 1000;
unsigned int16_t G2183_CreditsBitmapByteCount = 32000;
int16_t G2071_C320_ScreenPixelWidth = 320;
int16_t G2072_C200_ScreenPixelHeight = 200;
unsigned int16_t G2070_ViewportBitmapByteCount = 15232; /* Bitmap size of viewport 224x136 (4bpp) */
int16_t G2073_C224_ViewportPixelWidth = 224;
int16_t G2074_C136_ViewportHeight = 136;
unsigned int16_t G2142_ViewportBlackAreaByteCount;
int16_t G2140_i_LargestCacheableNativeBitmapWidth;
int16_t G2141_i_LargestCacheableNativeBitmapHeight;
int16_t G2184_ActionIconByteCount = M075_BITMAP_BYTE_COUNT(16, 16);
int16_t G2075_ObjectIconWidth = 16;
int16_t G2076_ObjectIconHeight = 16;
int16_t G2185_Unreferenced = 2;
int16_t G2186_Unreferenced = 2;
int16_t G2077_C1_TextMargin = 1;
int16_t G2078_C32_PortraitWidth = 32;
int16_t G2079_C29_PortraitHeight = 29;
int16_t G2080_C19_ChampionIconWidth = 19;
int16_t G2081_C14_ChampionIconHeight = 14;
int16_t G2082_ = 5;
int16_t G2083_C6_ = 6;
int16_t G2084_C1_ = 1;
int16_t G2085_ = 1;
int16_t G2086_C1_ = 1;
int16_t G2087_C6_TextCharacterWidth = 6;
int16_t G2088_C7_TextLineHeight = 7;
int16_t G2089_C8_InscriptionCharacterWidth = 8;
int16_t G2090_C8_InscriptionLineHeight = 8;
unsigned int16_t G2091_MessageAreaLineByteCount = 1120; /* = M075_BITMAP_BYTE_COUNT(320, 7) */
int16_t G2092_MessageAreaWidth = 320;
int16_t G2187_MessageAreaHeight_Unreferenced = 7;
int16_t G2093_DamageToCreatureMediumWidth = 64;
int16_t G2094_DamageToCreatureMediumHeight = 37;
int16_t G2095_DamageToCreatureSmallWidth = 42;
int16_t G2096_DamageToCreatureSmallHeight = 37;
int16_t G2097_FoodOrWaterBarShadowOffset = 2;
int16_t G2347_ = 14;
int16_t G2348_ = 13;
int16_t G2349_ = 4;
int16_t G3043_ = 3;
int16_t G3044_ = 3;
int16_t G3045_ = 2;
int16_t G3046_ = 2;
int16_t G2001_TemporaryZoneIndex;
int16_t G2063_aai_XYZ_Temporary[8][4];
LAYOUT_RANGE* G2174_ps_LayoutData = NULL;


int16_t* F0787_GetZoneInitializedFromCoordinates(
REGISTER int16_t* P2100_pi_XYZ   SEPARATOR
int16_t           P2101_i_X      SEPARATOR
int16_t           P2102_i_Y      SEPARATOR
int16_t           P2103_i_Width  SEPARATOR
int16_t           P2104_i_Height FINAL_SEPARATOR
{
        P2100_pi_XYZ[0] = P2101_i_X;
        P2100_pi_XYZ[1] = P2102_i_Y;
        P2100_pi_XYZ[2] = P2103_i_Width;
        P2100_pi_XYZ[3] = P2104_i_Height;
        return P2100_pi_XYZ;
}

int16_t* F0625_GetZoneInitializedFromDimensions(
REGISTER int16_t* P2105_pi_XYZ   SEPARATOR
int16_t           P2106_i_Width  SEPARATOR
int16_t           P2107_i_Height FINAL_SEPARATOR
{
        M704_ZONE_LEFT(P2105_pi_XYZ) = 0;
        M706_ZONE_TOP(P2105_pi_XYZ) = 0;
        M708_ZONE_WIDTH(P2105_pi_XYZ) = P2106_i_Width;
        M709_ZONE_HEIGHT(P2105_pi_XYZ) = P2107_i_Height;
        return P2105_pi_XYZ;
}

int16_t* F0626_GetTemporaryZoneInitializedFromCoordinates(
int16_t P2108_i_X      SEPARATOR
int16_t P2109_i_Y      SEPARATOR
int16_t P2110_i_Width  SEPARATOR
int16_t P2111_i_Height FINAL_SEPARATOR
{
        REGISTER int16_t* L2292_pi_XYZ;


        L2292_pi_XYZ = G2063_aai_XYZ_Temporary[G2001_TemporaryZoneIndex];
        if (++G2001_TemporaryZoneIndex >= 8) {
                G2001_TemporaryZoneIndex = 0;
        }
        M704_ZONE_LEFT(L2292_pi_XYZ) = P2108_i_X;
        M706_ZONE_TOP(L2292_pi_XYZ) = P2109_i_Y;
        M708_ZONE_WIDTH(L2292_pi_XYZ) = P2110_i_Width;
        M709_ZONE_HEIGHT(L2292_pi_XYZ) = P2111_i_Height;
        return L2292_pi_XYZ;
}

int16_t* F0627_GetTemporaryZoneInitializedFromDimensions(
int16_t P2112_i_PixelWidth  SEPARATOR
int16_t P2113_i_PixelHeight FINAL_SEPARATOR
{
        return F0626_GetTemporaryZoneInitializedFromCoordinates(0, 0, P2112_i_PixelWidth, P2113_i_PixelHeight);
}

int16_t* F0628_AddZoneMargin(
REGISTER int16_t* P2114_pi_XYZ         SEPARATOR
int16_t           P2115_i_WidthMargin  SEPARATOR
int16_t           P2116_i_HeightMargin FINAL_SEPARATOR
{
        M704_ZONE_LEFT(P2114_pi_XYZ) -= P2115_i_WidthMargin;
        M706_ZONE_TOP(P2114_pi_XYZ) -= P2116_i_HeightMargin;
        M708_ZONE_WIDTH(P2114_pi_XYZ) += P2115_i_WidthMargin << 1;
        M709_ZONE_HEIGHT(P2114_pi_XYZ) += P2116_i_HeightMargin << 1;
        return P2114_pi_XYZ;
}

BOOLEAN F0798_COMMAND_IsPointInZone(
REGISTER int16_t* P0749_pi_XYZ SEPARATOR
int16_t           P0750_i_X    SEPARATOR
int16_t           P0751_i_Y    FINAL_SEPARATOR
{
        return ((P0750_i_X >= M704_ZONE_LEFT(P0749_pi_XYZ)) && (P0750_i_X <= M705_ZONE_RIGHT(P0749_pi_XYZ)) && (P0751_i_Y >= M706_ZONE_TOP(P0749_pi_XYZ)) && (P0751_i_Y <= M707_ZONE_BOTTOM(P0749_pi_XYZ)));
}

BOOLEAN F0629_IsPointInZoneIndex(
int16_t          P2117_i_ZoneIndex SEPARATOR
REGISTER int16_t P2118_i_X         SEPARATOR
REGISTER int16_t P2119_i_Y         FINAL_SEPARATOR
{
        int16_t L2293_ai_XYZ[4];


        if (!F0638_GetZone(P2117_i_ZoneIndex, L2293_ai_XYZ)) {
                return C0_FALSE;
        } else{
                return F0798_COMMAND_IsPointInZone(L2293_ai_XYZ, P2118_i_X, P2119_i_Y);
        }
}

unsigned char* F0630_InitBitmapStruct2(
int16_t           P2120_i_GraphicIndex SEPARATOR
REGISTER STRUCT2* P2121_ps_            FINAL_SEPARATOR
{
        REGISTER STRUCT3* L2294_ps_;
        REGISTER unsigned char* L2295_puc_Bitmap;
        REGISTER int16_t L2296_i_PixelCount;


        if (P2120_i_GraphicIndex < 0) {
                P2120_i_GraphicIndex = -1 - P2120_i_GraphicIndex;
                L2294_ps_ = &G2002_NegativeBitmaps[P2120_i_GraphicIndex];
                P2121_ps_->X = L2294_ps_->X;
                P2121_ps_->Y = L2294_ps_->Y;
                switch (L2294_ps_->s3m1) {
                        case 1:
                                L2295_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(L2294_ps_->s3u.GraphicIndex);
                                break;
                        case 2:
                                L2295_puc_Bitmap = L2294_ps_->s3u.Bitmap;
                }
                if ((L2296_i_PixelCount = L2294_ps_->Width) != 0) {
                        P2121_ps_->Width = L2296_i_PixelCount;
                } else {
                        P2121_ps_->Width = M100_PIXEL_WIDTH(L2295_puc_Bitmap);
                }
                if ((L2296_i_PixelCount = L2294_ps_->Height) != 0) {
                        P2121_ps_->Height = L2296_i_PixelCount;
                } else {
#ifdef PC_FIX_CODE_SIZE
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
        L2296_i_PixelCount++;
#endif
                        P2121_ps_->Height = M101_PIXEL_HEIGHT(L2295_puc_Bitmap);
                }
        } else {
                P2121_ps_->X = 0;
                P2121_ps_->Y = 0;
                L2295_puc_Bitmap = F0489_MEMORY_GetNativeBitmapOrGraphic(P2120_i_GraphicIndex);
                P2121_ps_->Width = M100_PIXEL_WIDTH(L2295_puc_Bitmap);
                P2121_ps_->Height = M101_PIXEL_HEIGHT(L2295_puc_Bitmap);
        }
        return P2121_ps_->s2m1 = L2295_puc_Bitmap;
}

unsigned char* F0631_GetBitmapPointer(
int16_t P2122_i_GraphicIndex FINAL_SEPARATOR
{
        STRUCT2 L2297_s_Struct2;


        return F0630_InitBitmapStruct2(P2122_i_GraphicIndex, &L2297_s_Struct2);
}

void F0632_COORD_SetNegativeBitmapPointer(
register int16_t P2123_i_Index    SEPARATOR
unsigned char*   P2124_puc_Bitmap FINAL_SEPARATOR
{
        if (P2123_i_Index < 0) {
                P2123_i_Index = -1 - P2123_i_Index;
                if (G2002_NegativeBitmaps[P2123_i_Index].s3m1 == 2) {
                        G2002_NegativeBitmaps[P2123_i_Index].s3u.Bitmap = P2124_puc_Bitmap;
                }
        }
}

void F0633_COORD_SetNegativeBitmapIndex(
REGISTER int16_t P2125_i_Index        SEPARATOR
int16_t          P2126_i_GraphicIndex FINAL_SEPARATOR
{
        if (P2125_i_Index < 0) {
                P2125_i_Index = -1 - P2125_i_Index;
                if (G2002_NegativeBitmaps[P2125_i_Index].s3m1 == 1) {
                        G2002_NegativeBitmaps[P2125_i_Index].s3u.GraphicIndex = P2126_i_GraphicIndex;
                }
        }
}

STATICFUNCTION LAYOUT_RECORD* F0634_GetLayoutRecord(
REGISTER LAYOUT_RANGE* P2127_ps_LayoutRange SEPARATOR
REGISTER int16_t P2128_i_LayoutRecordIndex  FINAL_SEPARATOR
{
        if (P2128_i_LayoutRecordIndex) {
                while (P2127_ps_LayoutRange != NULL) {
                        if ((P2127_ps_LayoutRange->FirstIndex <= P2128_i_LayoutRecordIndex) && (P2127_ps_LayoutRange->LastIndex >= P2128_i_LayoutRecordIndex)) {
                                return &P2127_ps_LayoutRange->Records[P2128_i_LayoutRecordIndex - P2127_ps_LayoutRange->FirstIndex];
                        } else {
                                P2127_ps_LayoutRange = P2127_ps_LayoutRange->NextRange;
                        }
                }
        }
        return NULL;
}

int16_t* F0635_(
unsigned char*    P2129_puc_Bitmap  SEPARATOR
REGISTER int16_t* P2130_pi_XYZ      SEPARATOR
int16_t           P2131_i_ZoneIndex SEPARATOR
int16_t*          P2132_pi_X        SEPARATOR
int16_t*          P2133_pi_Y        FINAL_SEPARATOR
{
        REGISTER LAYOUT_RECORD* L2298_ps_LayoutRecord;
        REGISTER LAYOUT_RECORD* L2299_ps_LayoutRecord2;
        REGISTER int16_t L2300_i_;
        REGISTER int16_t L2301_;
        REGISTER int16_t L2302_i_;
        REGISTER int16_t L2303_i_;
        REGISTER int16_t L2304_i_RecordType;
        BOOLEAN L2305_B_;
        int16_t L2306_i_;
        int16_t L2307_ai_XYZ[4];


        if (P2131_i_ZoneIndex == CM1_UNKNOWN) {
                return NULL;
        }
        if ((L2300_i_ = M007_GET(P2131_i_ZoneIndex, MASK0x8000_SHIFT_OBJECTS_AND_CREATURES)) != 0) {
                M009_CLEAR(P2131_i_ZoneIndex, MASK0x8000_SHIFT_OBJECTS_AND_CREATURES);
        }
        if ((L2298_ps_LayoutRecord = F0634_GetLayoutRecord(G2174_ps_LayoutData, P2131_i_ZoneIndex)) == NULL) {
                return NULL;
        }
                F0625_GetZoneInitializedFromDimensions(L2307_ai_XYZ, 20000, 20000);
                if ((L2304_i_RecordType = L2298_ps_LayoutRecord->RecordType) <= 8) {
                        L2302_i_ = L2298_ps_LayoutRecord->Data1;
                        L2303_i_ = L2298_ps_LayoutRecord->Data2;
                } else {
                        if (L2304_i_RecordType == 9) {
                                return NULL;
                        }
                        L2304_i_RecordType -= 10;
                        L2302_i_ = 0;
                        L2303_i_ = 0;
                }
                if (L2300_i_) {
                        L2302_i_ += *P2132_pi_X;
                        L2303_i_ += *P2133_pi_Y;
                        *P2132_pi_X = 0;
                        *P2133_pi_Y = 0;
                }
                L2305_B_ = C0_FALSE;
                while (L2298_ps_LayoutRecord->ParentRecordIndex) {
                        if ((L2298_ps_LayoutRecord->RecordType >= 10) && (L2298_ps_LayoutRecord->RecordType <= 18)) {
                                if ((L2299_ps_LayoutRecord2 = F0634_GetLayoutRecord(G2174_ps_LayoutData, L2298_ps_LayoutRecord->ParentRecordIndex)) == NULL) {
                                        break;
                                }
                                L2300_i_ = L2299_ps_LayoutRecord2->Data1;
                                L2301_ = L2299_ps_LayoutRecord2->Data2;
                                L2306_i_ = L2299_ps_LayoutRecord2->RecordType;
                                if ((L2299_ps_LayoutRecord2 = F0634_GetLayoutRecord(G2174_ps_LayoutData, L2299_ps_LayoutRecord2->ParentRecordIndex)) == NULL) {
                                        break;
                                }
                                switch (L2306_i_) {
                                        default:
                                                return NULL;
                                        case 0:
                                                L2301_ -= ((L2299_ps_LayoutRecord2->Data2 + 1) >> 1);
                                        case 5:
                                                L2300_i_ -= ((L2299_ps_LayoutRecord2->Data1 + 1) >> 1);
                                                break;
                                        case 3:
                                                L2301_ -= L2299_ps_LayoutRecord2->Data2 - 1;
                                        case 2:
                                                L2300_i_ -= L2299_ps_LayoutRecord2->Data1 - 1;
                                                break;
                                        case 6:
                                                L2300_i_ -= L2299_ps_LayoutRecord2->Data1 - 1;
                                        case 8:
                                                L2301_ -= (L2299_ps_LayoutRecord2->Data2 + 1) >> 1;
                                                break;
                                        case 7:
                                                L2300_i_ -= (L2299_ps_LayoutRecord2->Data1 + 1) >> 1;
                                        case 4:
                                                L2301_ -= L2299_ps_LayoutRecord2->Data2 - 1;
                                        case 1:
                                                break;
                                }
                                if ((M704_ZONE_LEFT(L2307_ai_XYZ) += L2300_i_) < L2300_i_) {
                                        M704_ZONE_LEFT(L2307_ai_XYZ) = L2300_i_;
                                }
                                if (M705_ZONE_RIGHT(L2307_ai_XYZ) >= L2299_ps_LayoutRecord2->Data1 + L2300_i_) {
                                        M708_ZONE_WIDTH(L2307_ai_XYZ) = L2299_ps_LayoutRecord2->Data1 - M704_ZONE_LEFT(L2307_ai_XYZ) + L2300_i_;
                                }
                                if ((M706_ZONE_TOP(L2307_ai_XYZ) += L2301_) < L2301_) {
                                        M706_ZONE_TOP(L2307_ai_XYZ) = L2301_;
                                }
                                if (M707_ZONE_BOTTOM(L2307_ai_XYZ) >= L2299_ps_LayoutRecord2->Data2 + L2301_) {
                                        M709_ZONE_HEIGHT(L2307_ai_XYZ) = L2299_ps_LayoutRecord2->Data2 - M706_ZONE_TOP(L2307_ai_XYZ) + L2301_;
                                }
                                switch (L2298_ps_LayoutRecord->RecordType) {
                                        default:
                                                return NULL;
                                        case 10:
                                                L2301_ += (L2299_ps_LayoutRecord2->Data2 + 1) >> 1;
                                        case 15:
                                                L2300_i_ += (L2299_ps_LayoutRecord2->Data1 + 1) >> 1;
                                                break;
                                        case 13:
                                                L2301_ += L2299_ps_LayoutRecord2->Data2 - 1;
                                        case 12:
                                                L2300_i_ += L2299_ps_LayoutRecord2->Data1 - 1;
                                                break;
                                        case 16:
                                                L2300_i_ += L2299_ps_LayoutRecord2->Data1 - 1;
                                        case 18:
                                                L2301_ += (L2299_ps_LayoutRecord2->Data2 + 1) >> 1;
                                                break;
                                        case 17:
                                                L2300_i_ += (L2299_ps_LayoutRecord2->Data1 + 1) >> 1;
                                        case 14:
                                                L2301_ += L2299_ps_LayoutRecord2->Data2 - 1;
                                        case 11:
                                                break;
                                }
                                L2302_i_ += L2300_i_ + L2298_ps_LayoutRecord->Data1;
                                L2303_i_ += L2301_ + L2298_ps_LayoutRecord->Data2;
                                L2298_ps_LayoutRecord = L2299_ps_LayoutRecord2;
                        } else {
                                if ((L2299_ps_LayoutRecord2 = F0634_GetLayoutRecord(G2174_ps_LayoutData, L2298_ps_LayoutRecord->ParentRecordIndex)) == NULL) {
                                        break;
                                }
                                L2300_i_ = L2299_ps_LayoutRecord2->Data1;
                                L2301_ = L2299_ps_LayoutRecord2->Data2;
                                if (L2299_ps_LayoutRecord2->RecordType == 1) {
                                        L2302_i_ += L2300_i_;
                                        L2303_i_ += L2301_;
                                        M704_ZONE_LEFT(L2307_ai_XYZ) += L2300_i_;
                                        M706_ZONE_TOP(L2307_ai_XYZ) += L2301_;
                                } else {
                                        if (L2299_ps_LayoutRecord2->RecordType == 9) {
                                                switch (L2298_ps_LayoutRecord->RecordType) {
                                                        case 0:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - ((L2300_i_ + 1) >> 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - ((L2301_ + 1) >> 1);
                                                                break;
                                                        case 1:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1;
                                                                L2301_ = L2298_ps_LayoutRecord->Data2;
                                                                break;
                                                        case 2:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - (L2300_i_ - 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2;
                                                                break;
                                                        case 3:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - (L2300_i_ - 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - (L2301_ - 1);
                                                                break;
                                                        case 4:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1;
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - (L2301_ - 1);
                                                                break;
                                                        case 5:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - ((L2300_i_ + 1) >> 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2;
                                                                break;
                                                        case 6:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - (L2300_i_ - 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - ((L2301_ + 1) >> 1);
                                                                break;
                                                        case 7:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1 - ((L2300_i_ + 1) >> 1);
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - (L2301_ - 1);
                                                                break;
                                                        case 8:
                                                                L2300_i_ = L2298_ps_LayoutRecord->Data1;
                                                                L2301_ = L2298_ps_LayoutRecord->Data2 - ((L2301_ + 1) >> 1);

                                                }
                                                if (L2305_B_) {
                                                        L2305_B_ = C0_FALSE;
                                                        L2302_i_ += L2300_i_;
                                                        L2303_i_ += L2301_;
                                                        M704_ZONE_LEFT(L2307_ai_XYZ) += L2300_i_;
                                                        M706_ZONE_TOP(L2307_ai_XYZ) += L2301_;
                                                }
                                                if (M704_ZONE_LEFT(L2307_ai_XYZ) < L2300_i_) {
                                                        M704_ZONE_LEFT(L2307_ai_XYZ) = L2300_i_;
                                                }
                                                if (M705_ZONE_RIGHT(L2307_ai_XYZ) >= L2299_ps_LayoutRecord2->Data1 + L2300_i_) {
                                                        M708_ZONE_WIDTH(L2307_ai_XYZ) = L2299_ps_LayoutRecord2->Data1 - M704_ZONE_LEFT(L2307_ai_XYZ) + L2300_i_;
                                                }
                                                if (M706_ZONE_TOP(L2307_ai_XYZ) < L2301_) {
                                                        M706_ZONE_TOP(L2307_ai_XYZ) = L2301_;
                                                }
                                                if (M707_ZONE_BOTTOM(L2307_ai_XYZ) >= L2299_ps_LayoutRecord2->Data2 + L2301_) {
                                                        M709_ZONE_HEIGHT(L2307_ai_XYZ) = L2299_ps_LayoutRecord2->Data2 - M706_ZONE_TOP(L2307_ai_XYZ) + L2301_;
                                                }
                                        } else {
                                                if (L2299_ps_LayoutRecord2->RecordType <= 8) {
                                                        L2305_B_ = C1_TRUE;
                                                }
                                        }
                                }
                                L2298_ps_LayoutRecord = L2299_ps_LayoutRecord2;
                        }
                }
                if ((L2300_i_ = *P2132_pi_X) == 0) {
                        L2300_i_ = M100_PIXEL_WIDTH(P2129_puc_Bitmap);
                }
                if ((L2301_ = *P2133_pi_Y) == 0) {
                        L2301_ = M101_PIXEL_HEIGHT(P2129_puc_Bitmap);
                }
                switch (L2304_i_RecordType) {
                        default:
                                return NULL;
                        case 0:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - ((L2300_i_ + 1) >> 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - ((L2301_ + 1) >> 1);
                                break;
                        case 1:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_;
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_;
                                break;
                        case 2:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - (L2300_i_ - 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_;
                                break;
                        case 3:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - (L2300_i_ - 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - (L2301_ - 1);
                                break;
                        case 4:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_;
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - (L2301_ - 1);
                                break;
                        case 5:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - ((L2300_i_ + 1) >> 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_;
                                break;
                        case 6:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - (L2300_i_ - 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - ((L2301_ + 1) >> 1);
                                break;
                        case 7:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_ - ((L2300_i_ + 1) >> 1);
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - (L2301_ - 1);
                                break;
                        case 8:
                                M704_ZONE_LEFT(P2130_pi_XYZ) = L2302_i_;
                                M706_ZONE_TOP(P2130_pi_XYZ) = L2303_i_ - ((L2301_ + 1) >> 1);
                }
        L2302_i_ = M704_ZONE_LEFT(L2307_ai_XYZ) - M704_ZONE_LEFT(P2130_pi_XYZ);
        L2303_i_ = M706_ZONE_TOP(L2307_ai_XYZ) - M706_ZONE_TOP(P2130_pi_XYZ);
        if (L2302_i_ <= 0) {
                *P2132_pi_X = 0;
                M708_ZONE_WIDTH(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue(L2300_i_, M708_ZONE_WIDTH(L2307_ai_XYZ) + L2302_i_);
        } else {
                *P2132_pi_X = L2302_i_;
                M704_ZONE_LEFT(P2130_pi_XYZ) = M704_ZONE_LEFT(L2307_ai_XYZ);
                M708_ZONE_WIDTH(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue(L2300_i_ - L2302_i_, M708_ZONE_WIDTH(L2307_ai_XYZ));
        }
        if (L2303_i_ <= 0) {
                *P2133_pi_Y = 0;
                M709_ZONE_HEIGHT(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue(L2301_, M709_ZONE_HEIGHT(L2307_ai_XYZ) + L2303_i_);
        } else {
                *P2133_pi_Y = L2303_i_;
                M706_ZONE_TOP(P2130_pi_XYZ) = M706_ZONE_TOP(L2307_ai_XYZ);
                M709_ZONE_HEIGHT(P2130_pi_XYZ) = F0024_MAIN_GetMinimumValue(L2301_ - L2303_i_, M709_ZONE_HEIGHT(L2307_ai_XYZ));
        }
        if ((M708_ZONE_WIDTH(P2130_pi_XYZ) <= 0) || (M709_ZONE_HEIGHT(P2130_pi_XYZ) <= 0)) {
                return NULL;
        }
        return P2130_pi_XYZ;
}

int16_t* F0788_(
unsigned char*    P2134_puc_Bitmap  SEPARATOR
int16_t*          P2135_pi_XYZ      SEPARATOR
int16_t           P2136_i_ZoneIndex FINAL_SEPARATOR
{
        int16_t L2308_i_X;
        int16_t L2309_i_Y;


        L2308_i_X = 0;
        L2309_i_Y = 0;
        return F0635_(P2134_puc_Bitmap, P2135_pi_XYZ, P2136_i_ZoneIndex, &L2308_i_X, &L2309_i_Y);
}

void F0636_GetZoneTopLeftCoordinatesWith1PixelMargin(
int16_t  P2137_i_ZoneIndex SEPARATOR
int16_t* P2138_pi_X        SEPARATOR
int16_t* P2139_pi_Y        FINAL_SEPARATOR
{
        int16_t L2310_i_X;
        int16_t L2311_i_Y;
        int16_t L2312_ai_XYZ[4];


        L2310_i_X = 1;
        L2311_i_Y = 1;
        F0635_(NULL, L2312_ai_XYZ, P2137_i_ZoneIndex, &L2310_i_X, &L2311_i_Y);
        *P2138_pi_X = M704_ZONE_LEFT(L2312_ai_XYZ);
        *P2139_pi_Y = M706_ZONE_TOP(L2312_ai_XYZ);
}

int16_t* F0637_GetProportionalZone(
int16_t          P2140_i_ZoneIndex SEPARATOR
int16_t*         P2141_pi_XYZ      SEPARATOR
unsigned int16_t P2142_ui_A        SEPARATOR
unsigned int16_t P2143_ui_B        FINAL_SEPARATOR
{
        REGISTER LAYOUT_RECORD* L2313_ps_LayoutRecord;
        int16_t L2314_i_X;
        int16_t L2315_i_Y;


        if ((L2313_ps_LayoutRecord = F0634_GetLayoutRecord(G2174_ps_LayoutData, P2140_i_ZoneIndex)) == NULL) {
                goto loc_23A1D;
        }
        if (L2313_ps_LayoutRecord->ParentRecordIndex == 0) {
                goto loc_23A1D;
        }
        if ((L2313_ps_LayoutRecord = F0634_GetLayoutRecord(G2174_ps_LayoutData, L2313_ps_LayoutRecord->ParentRecordIndex)) == NULL) {
                goto loc_23A1D;
        }
        if (L2313_ps_LayoutRecord->RecordType == 9) {
                if (P2142_ui_A == 10000) {
                        L2314_i_X = L2313_ps_LayoutRecord->Data1;
                } else {
                        L2314_i_X = ((long)L2313_ps_LayoutRecord->Data1 * P2142_ui_A) / 10000L;
                }
                if (P2143_ui_B == 10000) {
                        L2315_i_Y = L2313_ps_LayoutRecord->Data2;
                } else {
                        L2315_i_Y = ((long)L2313_ps_LayoutRecord->Data2 * P2143_ui_B) / 10000L;
                }
                if ((L2314_i_X > 0) && (L2315_i_Y > 0)) {
                        return F0635_(NULL, P2141_pi_XYZ, P2140_i_ZoneIndex, &L2314_i_X, &L2315_i_Y);
                }
        }
        loc_23A1D:
        return NULL;
}

int16_t* F0638_GetZone(
int16_t  P2144_i_ZoneIndex SEPARATOR
int16_t* P2145_pi_XYZ      FINAL_SEPARATOR
{
        return F0637_GetProportionalZone(P2144_i_ZoneIndex, P2145_pi_XYZ, 10000, 10000);
}

void F0639_LoadLayoutRanges(
REGISTER unsigned int16_t* P2146_pui_Buffer                  SEPARATOR
REGISTER unsigned long     P2147_ul_ByteCount                SEPARATOR
REGISTER LAYOUT_RANGE*     P2148_ps_LayoutData               SEPARATOR
unsigned char*           (*P2149_pf_Callback)(unsigned long) FINAL_SEPARATOR
{
        REGISTER LAYOUT_RANGE* L2316_ps_LayoutRange;
        unsigned int16_t* L2317_pui_;
        REGISTER int16_t L2318_i_RangeFirstIndex;
        REGISTER int16_t L2319_i_RangeLastIndex;
        REGISTER int16_t L2320_i_RangeCounter;
        REGISTER unsigned long L2321_ul_RangeByteCount;


        if (*P2146_pui_Buffer++ != 0xFC0D) {
                return;
        }
        L2320_i_RangeCounter = *P2146_pui_Buffer++; /* Number of index ranges in item 696 */
        L2317_pui_ = P2146_pui_Buffer;
        P2146_pui_Buffer += L2320_i_RangeCounter * 2;
        while (P2148_ps_LayoutData->NextRange) {
                P2148_ps_LayoutData = P2148_ps_LayoutData->NextRange;
        }
        while (--L2320_i_RangeCounter >= 0) { /* Loop for each index range */
                L2318_i_RangeFirstIndex = *L2317_pui_++; /* First index in range */
                L2319_i_RangeLastIndex = *L2317_pui_++; /* Last index in range */
                L2321_ul_RangeByteCount = (L2319_i_RangeLastIndex - L2318_i_RangeFirstIndex + 1) * sizeof(LAYOUT_RECORD);
                L2316_ps_LayoutRange = (LAYOUT_RANGE*)(*P2149_pf_Callback)(L2321_ul_RangeByteCount + 8);
                P2148_ps_LayoutData->NextRange = L2316_ps_LayoutRange;
                P2148_ps_LayoutData = L2316_ps_LayoutRange;
                L2316_ps_LayoutRange->NextRange = NULL;
                L2316_ps_LayoutRange->FirstIndex = L2318_i_RangeFirstIndex;
                L2316_ps_LayoutRange->LastIndex = L2319_i_RangeLastIndex;
                F0007_MAIN_CopyBytes(M772_CAST_PC(P2146_pui_Buffer), (char*)L2316_ps_LayoutRange->Records, L2321_ul_RangeByteCount);
                P2146_pui_Buffer = M776_CAST_PUI(((unsigned char*)P2146_pui_Buffer) + L2321_ul_RangeByteCount);
        }
}

unsigned char* F0789_AllocateLayoutRange(
unsigned long P2150_puc_ByteCount FINAL_SEPARATOR
{
        return M533_F0468_MEMORY_Allocate(P2150_puc_ByteCount, C1_ALLOCATION_PERMANENT, MASK0x0400_MEMREQ);
}

void F0640_LoadLayoutData(
int16_t P2151_i_GraphicIndex FINAL_SEPARATOR
{
        REGISTER unsigned char* L2322_puc_Buffer;
        REGISTER unsigned long L2323_ul_ByteCount;


        L2322_puc_Buffer = M533_F0468_MEMORY_Allocate(L2323_ul_ByteCount = F0494_MEMORY_GetGraphicDecompressedByteCount(P2151_i_GraphicIndex), C2_ALLOCATION_TEMPORARY_ON_BOTTOM_OF_HEAP, MASK0x0400_MEMREQ);
        F0490_MEMORY_LoadDecompressAndExpandGraphic(P2151_i_GraphicIndex | MASK0x8000_NOT_EXPANDED | MASK0x4000_DO_NOT_COPY_DIMENSIONS, L2322_puc_Buffer);
        F0639_LoadLayoutRanges(M776_CAST_PUI(L2322_puc_Buffer), L2323_ul_ByteCount, &G2174_ps_LayoutData, F0789_AllocateLayoutRange);
        F0470_MEMORY_FreeAtHeapBottom(L2323_ul_ByteCount);
}

void F0641_InitializeLayout(
void
)
{
        F0640_LoadLayoutData(C696_GRAPHIC_LAYOUT);
}
/* END COORD.C */

/* BEGIN MOVESENS.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
unsigned int16_t G0397_i_MoveResultMapX;
unsigned int16_t G0398_i_MoveResultMapY;
unsigned int16_t G0399_ui_MoveResultMapIndex;
unsigned int16_t G0400_i_MoveResultDirection;
unsigned int16_t G0401_ui_MoveResultCell;
BOOLEAN G0402_B_UseRopeToClimbDownPit;
int16_t G0403_i_SensorRotationEffect = CM1_EFFECT_NONE;
int16_t G0404_i_SensorRotationEffectMapX;
int16_t G0405_i_SensorRotationEffectMapY;
int16_t G0406_i_SensorRotationEffectCell;


STATICFUNCTION int16_t F0262_MOVE_GetTeleporterRotatedGroupResult(
REGISTER TELEPORTER* P0541_ps_Teleporter SEPARATOR
THING                P0542_T_GroupThing  SEPARATOR
unsigned int16_t     P0543_ui_MapIndex   FINAL_SEPARATOR
{
        REGISTER GROUP* L0686_ps_Group;
        REGISTER unsigned int16_t L0683_ui_Rotation;
        REGISTER unsigned int16_t L0685_ui_UpdatedGroupDirections;
        REGISTER unsigned int16_t L0684_ui_GroupDirections;
        REGISTER unsigned int16_t L0687_ui_UpdatedGroupCells;
        REGISTER int16_t L0688_i_CreatureIndex;
        int16_t L0692_i_RelativeRotation;
        BOOLEAN L0689_B_AbsoluteRotation;
        unsigned int16_t L0690_ui_GroupCells;
        int16_t L0691_i_CreatureSize;


        L0686_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0542_T_GroupThing);
        L0683_ui_Rotation = P0541_ps_Teleporter->Rotation;
        L0684_ui_GroupDirections = F0147_DUNGEON_GetGroupDirections(L0686_ps_Group, P0543_ui_MapIndex);
        if (L0689_B_AbsoluteRotation = P0541_ps_Teleporter->AbsoluteRotation) {
                L0685_ui_UpdatedGroupDirections = L0683_ui_Rotation;
        } else {
                L0685_ui_UpdatedGroupDirections = M021_NORMALIZE(L0684_ui_GroupDirections + L0683_ui_Rotation);
        }
        if ((L0687_ui_UpdatedGroupCells = F0145_DUNGEON_GetGroupCells(L0686_ps_Group, P0543_ui_MapIndex)) != C0xFF_SINGLE_CENTERED_CREATURE) {
                L0690_ui_GroupCells = L0687_ui_UpdatedGroupCells;
                L0691_i_CreatureSize = M007_GET(G0243_as_Graphic559_CreatureInfo[L0686_ps_Group->Type].Attributes, MASK0x0003_SIZE);
                L0692_i_RelativeRotation = M021_NORMALIZE(4 + L0685_ui_UpdatedGroupDirections - L0684_ui_GroupDirections);
                for (L0688_i_CreatureIndex = 0; L0688_i_CreatureIndex <= L0686_ps_Group->Count; L0688_i_CreatureIndex++) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                        L0685_ui_UpdatedGroupDirections = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0685_ui_UpdatedGroupDirections, L0688_i_CreatureIndex, L0689_B_AbsoluteRotation ? L0683_ui_Rotation : M021_NORMALIZE(L0684_ui_GroupDirections + L0683_ui_Rotation));
                        if ((L0691_i_CreatureSize == C0_SIZE_QUARTER_SQUARE) && (L0692_i_RelativeRotation = !L0689_B_AbsoluteRotation)) {
                                L0692_i_RelativeRotation = L0683_ui_Rotation;
                        }
                        if (L0692_i_RelativeRotation) {
                                L0687_ui_UpdatedGroupCells = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(L0687_ui_UpdatedGroupCells, L0688_i_CreatureIndex, M021_NORMALIZE(L0690_ui_GroupCells + L0692_i_RelativeRotation));
                        }
                        L0684_ui_GroupDirections >>= 2;
                        L0690_ui_GroupCells >>= 2;
                }
        }
        F0148_DUNGEON_SetGroupDirections(L0686_ps_Group, L0685_ui_UpdatedGroupDirections, P0543_ui_MapIndex);
        F0146_DUNGEON_SetGroupCells(L0686_ps_Group, L0687_ui_UpdatedGroupCells, P0543_ui_MapIndex);
        if ((P0543_ui_MapIndex == G0309_i_PartyMapIndex) && (L0686_ps_Group->Behavior == C6_BEHAVIOR_ATTACK)) {
                return L0686_ps_Group->ActiveGroupIndex + 2;
        }
        else {
                return 1;
        }
}

STATICFUNCTION THING F0263_MOVE_GetTeleporterRotatedProjectileThing(
TELEPORTER*    P0544_ps_Teleporter     SEPARATOR
REGISTER THING P0545_T_ProjectileThing FINAL_SEPARATOR
{
        REGISTER int16_t L0693_i_UpdatedDirection;
        REGISTER int16_t L0694_i_Rotation;


        L0693_i_UpdatedDirection = G0400_i_MoveResultDirection;
        L0694_i_Rotation = P0544_ps_Teleporter->Rotation;
        if (P0544_ps_Teleporter->AbsoluteRotation) {
                L0693_i_UpdatedDirection = L0694_i_Rotation;
        } else {
                L0693_i_UpdatedDirection = M021_NORMALIZE(L0693_i_UpdatedDirection + L0694_i_Rotation);
                P0545_T_ProjectileThing = M015_THING_WITH_NEW_CELL(P0545_T_ProjectileThing, M021_NORMALIZE(M011_CELL(P0545_T_ProjectileThing) + L0694_i_Rotation));
        }
        G0400_i_MoveResultDirection = L0693_i_UpdatedDirection;
        return P0545_T_ProjectileThing;
}

BOOLEAN F0264_MOVE_IsLevitating(
REGISTER THING P0546_T_Thing FINAL_SEPARATOR
{
        REGISTER int16_t L0695_i_ThingType;


        if ((L0695_i_ThingType = M012_TYPE(P0546_T_Thing)) == C04_THING_TYPE_GROUP) {
                return M007_GET(F0144_DUNGEON_GetCreatureAttributes(P0546_T_Thing), MASK0x0020_LEVITATION);
        }
        else {
                if ((L0695_i_ThingType == C14_THING_TYPE_PROJECTILE) || (L0695_i_ThingType == C15_THING_TYPE_EXPLOSION)) {
                        return C1_TRUE;
                }
                else {
                        return C0_FALSE;
                }
        }
}

STATICFUNCTION void F0265_MOVE_CreateEvent60To61_MoveGroup(
THING          P0547_T_GroupThing SEPARATOR
int16_t        P0548_i_MapX       SEPARATOR
int16_t        P0549_i_MapY       SEPARATOR
int16_t        P0550_i_MapIndex   SEPARATOR
BOOLEAN        P0551_B_Audible    FINAL_SEPARATOR
{
        EVENT L0696_s_Event;


        M033_SET_MAP_AND_TIME(L0696_s_Event.Map_Time, P0550_i_MapIndex, G0313_ul_GameTime + 5);
        L0696_s_Event.A.A.Type = P0551_B_Audible ? C61_EVENT_MOVE_GROUP_AUDIBLE : C60_EVENT_MOVE_GROUP_SILENT;
        L0696_s_Event.A.A.Priority = 0;
        L0696_s_Event.B.Location.MapX = P0548_i_MapX;
        L0696_s_Event.B.Location.MapY = P0549_i_MapY;
        L0696_s_Event.C.Slot = P0547_T_GroupThing;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0696_s_Event);
}

/* This function processes projectile impacts when moving the party or a group. It return C1_TRUE only if a group was killed by a projectile impact */
STATICFUNCTION BOOLEAN F0266_MOVE_IsKilledByProjectileImpact(
REGISTER int16_t P0552_i_SourceMapX      SEPARATOR
int16_t          P0553_i_SourceMapY      SEPARATOR
int16_t          P0554_i_DestinationMapX SEPARATOR
int16_t          P0555_i_DestinationMapY SEPARATOR
THING            P0556_T_Thing           FINAL_SEPARATOR /* Must be the party or a group */
{
        REGISTER GROUP* L0701_ps_Group;
        REGISTER unsigned int16_t L0699_ui_Multiple;
#define AL0699_ui_Cell                      L0699_ui_Multiple
#define AL0699_ui_PrimaryDirection          L0699_ui_Multiple
#define AL0699_ui_ChampionOrCreatureOrdinal L0699_ui_Multiple
        REGISTER int16_t L0700_i_Multiple;
#define AL0700_B_CreatureAlive      L0700_i_Multiple
#define AL0700_i_Distance           L0700_i_Multiple
#define AL0700_i_SecondaryDirection L0700_i_Multiple
        REGISTER THING L0697_T_Thing;
        REGISTER BOOLEAN L0703_B_CheckDestinationSquareProjectileImpacts;
        int16_t L0702_i_ImpactType;
        unsigned int16_t L0704_ui_ProjectileMapX;
        unsigned int16_t L0705_ui_ProjectileMapY;
        unsigned char L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[4]; /* This array is used only when moving between two adjacent squares and is used to test projectile impacts when the party or group is in the 'intermediary' step between the two squares. Without this test, in the example below no impact would be detected. In this example, the party moves from the source square on the left (which contains a single champion at cell 2) to the destination square on the right (which contains a single projectile at cell 3).
        Party:      Projectiles on target square:   Incorrect result without the test for the intermediary step (the champion would have passed through the projectile without impact):
        00    ->    00                         00
        01          P0                         P1 */
        unsigned char L0707_auc_ChampionOrCreatureOrdinalInCell[4]; /* This array has an entry for each cell on the source square, containing the ordinal of the champion or creature (0 if there is no champion or creature at this cell) */


        L0703_B_CheckDestinationSquareProjectileImpacts = C0_FALSE;
        F0008_MAIN_ClearBytes(M772_CAST_PC(L0707_auc_ChampionOrCreatureOrdinalInCell), M543_BYTE_COUNT_INT(sizeof(L0707_auc_ChampionOrCreatureOrdinalInCell)));
        if (P0556_T_Thing == C0xFFFF_THING_PARTY) {
                L0702_i_ImpactType = CM2_ELEMENT_CHAMPION;
                for (AL0699_ui_Cell = C00_CELL_NORTHWEST; AL0699_ui_Cell < C03_CELL_SOUTHWEST + 1; AL0699_ui_Cell++) {
                        if (F0285_CHAMPION_GetIndexInCell(AL0699_ui_Cell) >= 0) {
                                L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_Cell] = M000_INDEX_TO_ORDINAL(AL0699_ui_Cell);
                        }
                }
        } else {
                L0702_i_ImpactType = CM1_ELEMENT_CREATURE;
                L0701_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(P0556_T_Thing);
                AL0700_B_CreatureAlive = C0_FALSE;
                for (AL0699_ui_Cell = C00_CELL_NORTHWEST; AL0699_ui_Cell < C03_CELL_SOUTHWEST + 1; AL0699_ui_Cell++) {
                        AL0700_B_CreatureAlive |= L0701_ps_Group->Health[AL0699_ui_Cell];
                        if (F0176_GROUP_GetCreatureOrdinalInCell(L0701_ps_Group, AL0699_ui_Cell)) {
                                L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_Cell] = M000_INDEX_TO_ORDINAL(AL0699_ui_Cell);
                        }
                }
                if (!AL0700_B_CreatureAlive) {
                        return C0_FALSE;
                }
        }
        if ((P0554_i_DestinationMapX >= 0) && (((((AL0700_i_Distance = P0552_i_SourceMapX - P0554_i_DestinationMapX) < 0) ? -AL0700_i_Distance : AL0700_i_Distance) + (((AL0700_i_Distance = P0553_i_SourceMapY - P0555_i_DestinationMapY) < 0) ? -AL0700_i_Distance : AL0700_i_Distance)) == 1)) { /* If source and destination squares are adjacent (if party or group is not being teleported) */
                AL0699_ui_PrimaryDirection = F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0552_i_SourceMapX, P0553_i_SourceMapY, P0554_i_DestinationMapX, P0555_i_DestinationMapY);
                AL0700_i_SecondaryDirection = M017_NEXT(AL0699_ui_PrimaryDirection);
                F0008_MAIN_ClearBytes(M772_CAST_PC(L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell), M543_BYTE_COUNT_INT(sizeof(L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell)));
                if (L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[M019_PREVIOUS(AL0699_ui_PrimaryDirection)] = L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection]) {
                        L0703_B_CheckDestinationSquareProjectileImpacts = C1_TRUE;
                }
                if (L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[M017_NEXT(AL0700_i_SecondaryDirection)] = L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection]) {
                        L0703_B_CheckDestinationSquareProjectileImpacts = C1_TRUE;
                }
                if (!L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection]) {
                        L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection] = L0707_auc_ChampionOrCreatureOrdinalInCell[M019_PREVIOUS(AL0699_ui_PrimaryDirection)];
                }
                if (!L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection]) {
                        L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection] = L0707_auc_ChampionOrCreatureOrdinalInCell[M017_NEXT(AL0700_i_SecondaryDirection)];
                }
        }
        L0704_ui_ProjectileMapX = P0552_i_SourceMapX; /* Check impacts with projectiles on the source square */
        L0705_ui_ProjectileMapY = P0553_i_SourceMapY;
        T0266017_CheckProjectileImpacts:
        L0697_T_Thing = F0161_DUNGEON_GetSquareFirstThing(L0704_ui_ProjectileMapX, L0705_ui_ProjectileMapY);
        while (L0697_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if ((M012_TYPE(L0697_T_Thing) == C14_THING_TYPE_PROJECTILE) &&
                    (G0370_ps_Events[(((PROJECTILE*)G0284_apuc_ThingData[C14_THING_TYPE_PROJECTILE])[M013_INDEX(L0697_T_Thing)]).EventIndex].A.A.Type != C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS) && (AL0699_ui_ChampionOrCreatureOrdinal = L0707_auc_ChampionOrCreatureOrdinalInCell[M011_CELL(L0697_T_Thing)]) &&
                    F0217_PROJECTILE_HasImpactOccured(L0702_i_ImpactType, P0552_i_SourceMapX, P0553_i_SourceMapY, M001_ORDINAL_TO_INDEX(AL0699_ui_ChampionOrCreatureOrdinal), L0697_T_Thing)) {
                        F0214_PROJECTILE_DeleteEvent(L0697_T_Thing);
                        if (G0364_i_CreatureDamageOutcome == C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP) {
                                return C1_TRUE;
                        }
                        goto T0266017_CheckProjectileImpacts;
                }
                L0697_T_Thing = F0159_DUNGEON_GetNextThing(L0697_T_Thing);
        }
        if (L0703_B_CheckDestinationSquareProjectileImpacts) {
                P0552_i_SourceMapX |= (((L0704_ui_ProjectileMapX = P0554_i_DestinationMapX) + 1) << 8); /* Check impacts with projectiles on the destination square */
                P0553_i_SourceMapY |= ((L0705_ui_ProjectileMapY = P0555_i_DestinationMapY) << 8);
                F0007_MAIN_CopyBytes(M772_CAST_PC(L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell), M772_CAST_PC(L0707_auc_ChampionOrCreatureOrdinalInCell), M543_BYTE_COUNT_INT(sizeof(L0707_auc_ChampionOrCreatureOrdinalInCell)));
                L0703_B_CheckDestinationSquareProjectileImpacts = C0_FALSE;
                goto T0266017_CheckProjectileImpacts;
        }
        return C0_FALSE;
}

/* Returns 0 (C0_FALSE) if the thing was moved successfully. Returns non 0 (C1_TRUE or > 1) if the specified thing is a group and the ACTIVE_GROUP has been refreshed or removed which occurs when the group was blocked by the party (the group is then removed from the dungeon and an event is created to move the group back in the dungeon later), killed during movement or teleported, or entered or leaved the party map */
BOOLEAN F0267_MOVE_GetMoveResult_CPSCE(
REGISTER THING   P0557_T_Thing           SEPARATOR
int16_t          P0558_i_SourceMapX      SEPARATOR /* If P0558_i_SourceMapX is negative (CM1_MAPX_NOT_ON_A_SQUARE or CM2_MAPX_PROJECTILE_ASSOCIATED_OBJECT) then place the thing on the destination square. If placing a projectile associated object (CM2_MAPX_PROJECTILE_ASSOCIATED_OBJECT) on a teleporter square with an absolute rotation then ignore the rotation */
int16_t          P0559_i_SourceMapY      SEPARATOR
REGISTER int16_t P0560_i_DestinationMapX SEPARATOR /* If P0560_i_DestinationMapX is negative, then remove the thing from the source square */
REGISTER int16_t P0561_i_DestinationMapY FINAL_SEPARATOR
{
        REGISTER TELEPORTER* L0712_ps_Teleporter;
        REGISTER CHAMPION* L0711_ps_Champion;
        REGISTER unsigned int16_t L0715_ui_MapIndexDestination;
        REGISTER int16_t L0708_i_Multiple;
#define AL0708_i_DestinationSquare L0708_i_Multiple
#define AL0708_i_ScentIndex        L0708_i_Multiple
#define AL0708_i_ActiveGroupIndex  L0708_i_Multiple
#define AL0708_i_Checksum          L0708_i_Multiple
        unsigned int16_t L0714_ui_MapIndexSource;
        REGISTER int16_t L0710_i_ThingType;
        unsigned int16_t L0727_ui_Multiple;
#define AL0727_ui_ThingCell L0727_ui_Multiple
#define AL0727_ui_Outcome   L0727_ui_Multiple
#define AL0727_ui_Backup    L0727_ui_Multiple
        unsigned int16_t L0716_ui_Direction;
        REGISTER int16_t L0709_i_Multiple;
#define AL0709_i_DestinationSquareType L0709_i_Multiple
#define AL0709_i_ChampionIndex         L0709_i_Multiple
        int16_t L0718_i_RequiredTeleporterScope;
        int16_t L0719_i_TraversedPitCount;
        unsigned int16_t L0717_ui_ThingCell;
        unsigned int16_t L0728_i_ChainedMoveCount;
        unsigned int16_t L0720_ui_MoveGroupResult;
        BOOLEAN L0713_B_ThingLevitates;
        BOOLEAN L0721_B_GroupOnPartyMap;
        BOOLEAN L0722_B_FallKilledGroup;
        BOOLEAN L0724_B_DestinationIsTeleporterTarget;
        BOOLEAN L0725_B_PartySquare;
        BOOLEAN L0726_B_Audible;
        BOOLEAN L0723_B_DrawDungeonViewWhileFalling;
        int16_t L3022_i_X;
        int16_t L3023_i_Y;


        L0710_i_ThingType = CM1_THING_TYPE_PARTY;
        L0719_i_TraversedPitCount = 0;
        L0720_ui_MoveGroupResult = 0;
        L0713_B_ThingLevitates = C0_FALSE;
        L0721_B_GroupOnPartyMap = C0_FALSE;
        L0722_B_FallKilledGroup = C0_FALSE;
        L0724_B_DestinationIsTeleporterTarget = C0_FALSE;
        L0725_B_PartySquare = C0_FALSE;
        L0726_B_Audible = C0_FALSE;
        L0723_B_DrawDungeonViewWhileFalling = C0_FALSE;
        if (P0557_T_Thing != C0xFFFF_THING_PARTY) {
                L0710_i_ThingType = M012_TYPE(P0557_T_Thing);
                L0717_ui_ThingCell = M011_CELL(P0557_T_Thing);
                L0713_B_ThingLevitates = F0264_MOVE_IsLevitating(P0557_T_Thing);
        }
        /* If moving the party or a creature on the party map from a dungeon square then check for a projectile impact */
        if ((P0558_i_SourceMapX >= 0) && ((P0557_T_Thing == C0xFFFF_THING_PARTY) || ((L0710_i_ThingType == C04_THING_TYPE_GROUP) && (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex)))) {
                if (F0266_MOVE_IsKilledByProjectileImpact(P0558_i_SourceMapX, P0559_i_SourceMapY, P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing)) {
                        return C1_TRUE; /* The specified group thing cannot be moved because it was killed by a projectile impact */
                }
        }
        if (P0560_i_DestinationMapX >= 0) {
                L0714_ui_MapIndexSource = L0715_ui_MapIndexDestination = G0272_i_CurrentMapIndex;
                L0721_B_GroupOnPartyMap = (L0714_ui_MapIndexSource == G0309_i_PartyMapIndex) && (P0558_i_SourceMapX >= 0);
                if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                        G0306_i_PartyMapX = P0560_i_DestinationMapX;
                        G0307_i_PartyMapY = P0561_i_DestinationMapY;
                        L0718_i_RequiredTeleporterScope = MASK0x0002_SCOPE_OBJECTS_OR_PARTY;
                        L0723_B_DrawDungeonViewWhileFalling = !G0423_i_InventoryChampionOrdinal && !G0300_B_PartyIsResting;
                        L0716_ui_Direction = G0308_i_PartyDirection;
                } else {
                        if (L0710_i_ThingType == C04_THING_TYPE_GROUP) {
                                L0718_i_RequiredTeleporterScope = MASK0x0001_SCOPE_CREATURES;
                        } else {
                                L0718_i_RequiredTeleporterScope = (MASK0x0001_SCOPE_CREATURES | MASK0x0002_SCOPE_OBJECTS_OR_PARTY);
                        }
                }
                if (L0710_i_ThingType == C14_THING_TYPE_PROJECTILE) {
                        L0712_ps_Teleporter = (TELEPORTER*)F0156_DUNGEON_GetThingData(P0557_T_Thing);
                        G0400_i_MoveResultDirection = (G0370_ps_Events[((PROJECTILE*)L0712_ps_Teleporter)->EventIndex]).C.Projectile.Direction;
                }
                for (L0728_i_ChainedMoveCount = 100; --L0728_i_ChainedMoveCount; ) { /* No more than 100 chained moves at once (in a chain of teleporters and pits for example) */
                        AL0708_i_DestinationSquare = G0271_ppuc_CurrentMapData[P0560_i_DestinationMapX][P0561_i_DestinationMapY];
                        if ((AL0709_i_DestinationSquareType = M034_SQUARE_TYPE(AL0708_i_DestinationSquare)) == C05_ELEMENT_TELEPORTER) {
                                if (!M007_GET(AL0708_i_DestinationSquare, MASK0x0008_TELEPORTER_OPEN))
                                        break;
                                L0712_ps_Teleporter = (TELEPORTER*)F0157_DUNGEON_GetSquareFirstThingData(P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                if ((L0712_ps_Teleporter->Scope == MASK0x0001_SCOPE_CREATURES) && (L0710_i_ThingType != C04_THING_TYPE_GROUP))
                                        break;
                                if ((L0718_i_RequiredTeleporterScope != (MASK0x0001_SCOPE_CREATURES | MASK0x0002_SCOPE_OBJECTS_OR_PARTY)) && !M007_GET(L0712_ps_Teleporter->Scope, L0718_i_RequiredTeleporterScope)) /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE test */
                                        break;
                                L0724_B_DestinationIsTeleporterTarget = (P0560_i_DestinationMapX == L0712_ps_Teleporter->TargetMapX) && (P0561_i_DestinationMapY == L0712_ps_Teleporter->TargetMapY) && (L0715_ui_MapIndexDestination == L0712_ps_Teleporter->TargetMapIndex);
                                P0560_i_DestinationMapX = L0712_ps_Teleporter->TargetMapX;
                                P0561_i_DestinationMapY = L0712_ps_Teleporter->TargetMapY;
                                L0726_B_Audible = L0712_ps_Teleporter->Audible;
                                F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination = L0712_ps_Teleporter->TargetMapIndex);
                                if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                                        G0306_i_PartyMapX = P0560_i_DestinationMapX;
                                        G0307_i_PartyMapY = P0561_i_DestinationMapY;
                                        if (L0712_ps_Teleporter->Audible) {
                                                F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, G0306_i_PartyMapX, G0307_i_PartyMapY, C00_MODE_PLAY_IMMEDIATELY);
                                        }
                                        L0723_B_DrawDungeonViewWhileFalling &= G2040_auc_MemoryFlags[C0_MEM_];
                                        if (L0712_ps_Teleporter->AbsoluteRotation) {
                                                F0284_CHAMPION_SetPartyDirection(L0712_ps_Teleporter->Rotation);
                                        } else {
#ifdef PC_FIX_CODE_SIZE
        P0557_T_Thing++;
        P0557_T_Thing++;
        P0557_T_Thing++;
        P0557_T_Thing++;
        P0557_T_Thing++;
        P0557_T_Thing++;
        P0557_T_Thing++;
#endif
                                                F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + L0712_ps_Teleporter->Rotation));
                                        }
                                } else {
                                        if (L0710_i_ThingType == C04_THING_TYPE_GROUP) {
                                                if (L0712_ps_Teleporter->Audible) {
                                                        F0064_SOUND_RequestPlay_CPSD(M560_SOUND_BUZZ, P0560_i_DestinationMapX, P0561_i_DestinationMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                                }
                                                L0720_ui_MoveGroupResult = F0262_MOVE_GetTeleporterRotatedGroupResult(L0712_ps_Teleporter, P0557_T_Thing, L0714_ui_MapIndexSource);
                                        } else {
                                                if (L0710_i_ThingType == C14_THING_TYPE_PROJECTILE) {
                                                        P0557_T_Thing = F0263_MOVE_GetTeleporterRotatedProjectileThing(L0712_ps_Teleporter, P0557_T_Thing);
                                                } else {
                                                        if (!(L0712_ps_Teleporter->AbsoluteRotation) && (P0558_i_SourceMapX != CM2_MAPX_PROJECTILE_ASSOCIATED_OBJECT)) {
                                                                P0557_T_Thing = M015_THING_WITH_NEW_CELL(P0557_T_Thing, M021_NORMALIZE(M011_CELL(P0557_T_Thing) + L0712_ps_Teleporter->Rotation));
                                                        }
                                                }
                                        }
                                }
                                if (L0724_B_DestinationIsTeleporterTarget)
                                        break;
                        } else {
                                if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates && M007_GET(AL0708_i_DestinationSquare, MASK0x0008_PIT_OPEN) && !M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY)) {
                                        if (L0723_B_DrawDungeonViewWhileFalling) {
                                                if (!G0402_B_UseRopeToClimbDownPit) {
                                                        L0723_B_DrawDungeonViewWhileFalling = G2040_auc_MemoryFlags[C0_MEM_];
                                                        if (L0719_i_TraversedPitCount) {
                                                                F0174_DUNGEON_SetCurrentMapAndPartyMap(L0715_ui_MapIndexDestination);
                                                                F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF();
                                                        }
                                                        F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY); /* BUG0_28 When falling through multiple pits the dungeon view is updated to show each traversed map but the graphics used for creatures, wall and floor ornaments may not be correct. The dungeon view is drawn for each map by using the graphics loaded for the source map. Therefore the graphics for creatures, wall and floor ornaments may not look like what they should */
                                                        /* BUG0_71 Some timings are too short on fast computers. When the party falls in a series of pits, the dungeon view is refreshed too quickly because the execution speed is not limited */
                                                        /* BUG0_01 While drawing creatures the engine will read invalid ACTIVE_GROUP data in G0375_ps_ActiveGroups because the data is for the creatures on the source map and not the map being drawn. The only consequence is that creatures may be drawn with incorrect bitmaps and/or directions */
                                                }
                                                L0719_i_TraversedPitCount++;
                                        }
                                        L3022_i_X = P0560_i_DestinationMapX;
                                        L3023_i_Y = P0561_i_DestinationMapY;
                                        L0715_ui_MapIndexDestination = F0154_DUNGEON_GetLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1, &L3022_i_X, &L3023_i_Y);
                                        P0560_i_DestinationMapX = L3022_i_X;
                                        P0561_i_DestinationMapY = L3023_i_Y;
                                        F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                                        if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                                                G0306_i_PartyMapX = P0560_i_DestinationMapX;
                                                G0307_i_PartyMapY = P0561_i_DestinationMapY;
                                                if (G0305_ui_PartyChampionCount > 0) {
                                                        if (G0402_B_UseRopeToClimbDownPit) {
                                                                L0723_B_DrawDungeonViewWhileFalling = G2040_auc_MemoryFlags[C0_MEM_] && !G0423_i_InventoryChampionOrdinal && !G0300_B_PartyIsResting;
                                                                L0711_ps_Champion = M516_CHAMPIONS;
                                                                for (AL0709_i_ChampionIndex = C00_CHAMPION_FIRST; AL0709_i_ChampionIndex < G0305_ui_PartyChampionCount; L0711_ps_Champion++, AL0709_i_ChampionIndex++) {
                                                                        if (L0711_ps_Champion->CurrentHealth) {
                                                                                F0325_CHAMPION_DecrementStamina(AL0709_i_ChampionIndex, ((L0711_ps_Champion->Load * 25) / F0309_CHAMPION_GetMaximumLoad(L0711_ps_Champion)) + 1);
                                                                        }
                                                                }
                                                        } else {
                                                                if (F0324_CHAMPION_DamageAll_GetDamagedChampionCount(20, MASK0x0010_WOUND_LEGS | MASK0x0020_WOUND_FEET, C2_ATTACK_SELF)) {
                                                                        F0064_SOUND_RequestPlay_CPSD(M561_SOUND_SCREAM, G0306_i_PartyMapX, G0307_i_PartyMapY, C00_MODE_PLAY_IMMEDIATELY);
                                                                }
                                                        }
                                                }
                                                G0402_B_UseRopeToClimbDownPit = C0_FALSE;
                                        } else {
                                                if (L0710_i_ThingType == C04_THING_TYPE_GROUP) {
                                                        F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                                                        AL0727_ui_Outcome = F0191_GROUP_GetDamageAllCreaturesOutcome((GROUP*)F0156_DUNGEON_GetThingData(P0557_T_Thing), P0558_i_SourceMapX, P0559_i_SourceMapY, 20, C0_FALSE);
                                                        F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                                                        if (L0722_B_FallKilledGroup = (AL0727_ui_Outcome == C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP))
                                                                break;
                                                        else {
                                                                if (AL0727_ui_Outcome == C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP) {
                                                                        F0187_GROUP_DropMovingCreatureFixedPossessions(P0557_T_Thing, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                                                }
                                                        }
                                                }
                                        }
                                } else {
                                        if ((AL0709_i_DestinationSquareType == C03_ELEMENT_STAIRS) && (P0557_T_Thing != C0xFFFF_THING_PARTY) && (L0710_i_ThingType != C14_THING_TYPE_PROJECTILE)) {
                                                if (!M007_GET(AL0708_i_DestinationSquare, MASK0x0004_STAIRS_UP)) {
                                                        L3022_i_X = P0560_i_DestinationMapX;
                                                        L3023_i_Y = P0561_i_DestinationMapY;
                                                        L0715_ui_MapIndexDestination = F0154_DUNGEON_GetLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1, &L3022_i_X, &L3023_i_Y);
                                                        P0560_i_DestinationMapX = L3022_i_X;
                                                        P0561_i_DestinationMapY = L3023_i_Y;
                                                        F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                                                }
                                                L0716_ui_Direction = F0155_DUNGEON_GetStairsExitDirection(P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                                P0560_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[L0716_ui_Direction], P0561_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[L0716_ui_Direction];
                                                L0716_ui_Direction = M018_OPPOSITE(L0716_ui_Direction);
                                                AL0727_ui_ThingCell = M011_CELL(P0557_T_Thing);
                                                AL0727_ui_ThingCell = M021_NORMALIZE((((AL0727_ui_ThingCell - L0716_ui_Direction + 1) & 0x0002) >> 1) + L0716_ui_Direction);
                                                P0557_T_Thing = M015_THING_WITH_NEW_CELL(P0557_T_Thing, AL0727_ui_ThingCell);
                                        } else
                                                break;
                                }
                        }
                }
                if ((L0710_i_ThingType == C04_THING_TYPE_GROUP) && (L0722_B_FallKilledGroup || !F0139_DUNGEON_IsCreatureAllowedOnMap(P0557_T_Thing, L0715_ui_MapIndexDestination))) {
                        F0187_GROUP_DropMovingCreatureFixedPossessions(P0557_T_Thing, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                        F0188_GROUP_DropGroupPossessions(P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing, C02_MODE_PLAY_ONE_TICK_LATER);
                        F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                        if (P0558_i_SourceMapX >= 0) {
                                F0189_GROUP_Delete(P0558_i_SourceMapX, P0559_i_SourceMapY);
                        }
                        return C1_TRUE; /* The specified group thing cannot be moved because it was killed by a fall or because it is not allowed on the destination map */
                }
                G0397_i_MoveResultMapX = P0560_i_DestinationMapX;
                G0398_i_MoveResultMapY = P0561_i_DestinationMapY;
                G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;
                G0401_ui_MoveResultCell = M011_CELL(P0557_T_Thing);
                L0725_B_PartySquare = (L0715_ui_MapIndexDestination == L0714_ui_MapIndexSource) && (P0560_i_DestinationMapX == P0558_i_SourceMapX) && (P0561_i_DestinationMapY == P0559_i_SourceMapY);
                if (L0725_B_PartySquare) {
                        if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                                if (G0308_i_PartyDirection == L0716_ui_Direction) {
                                        return C0_FALSE;
                                }
                        } else {
                                if ((G0401_ui_MoveResultCell == L0717_ui_ThingCell) && (L0710_i_ThingType != C14_THING_TYPE_PROJECTILE)) {
                                        return C0_FALSE;
                                }
                        }
                } else {
                        if ((P0557_T_Thing == C0xFFFF_THING_PARTY) && G0305_ui_PartyChampionCount) {
                                AL0727_ui_Backup = AL0708_i_DestinationSquare;
                                AL0708_i_ScentIndex = G0407_s_Party.ScentCount;
                                while (AL0708_i_ScentIndex >= 24) {
                                        F0316_CHAMPION_DeleteScent(0);
                                        AL0708_i_ScentIndex--;
                                }
                                if (AL0708_i_ScentIndex) {
                                        F0317_CHAMPION_AddScentStrength(P0558_i_SourceMapX, P0559_i_SourceMapY, (int16_t)(G0313_ul_GameTime - G0362_l_LastPartyMovementTime));
                                }
                                G0362_l_LastPartyMovementTime = G0313_ul_GameTime;
                                G0407_s_Party.ScentCount++;
                                if (G0407_s_Party.Event79Count_Footprints) {
                                        G0407_s_Party.LastScentIndex = G0407_s_Party.ScentCount;
                                }
                                G0407_s_Party.Scents[AL0708_i_ScentIndex].Location.MapX = P0560_i_DestinationMapX;
                                G0407_s_Party.Scents[AL0708_i_ScentIndex].Location.MapY = P0561_i_DestinationMapY;
                                G0407_s_Party.Scents[AL0708_i_ScentIndex].Location.MapIndex = L0715_ui_MapIndexDestination;
                                G0407_s_Party.ScentStrengths[AL0708_i_ScentIndex] = 0;
                                F0317_CHAMPION_AddScentStrength(P0560_i_DestinationMapX, P0561_i_DestinationMapY, MASK0x8000_MERGE_CYCLES | 24);
                                AL0708_i_DestinationSquare = AL0727_ui_Backup;
                        }
                        if (L0715_ui_MapIndexDestination != L0714_ui_MapIndexSource) {
                                if ((P0557_T_Thing == C0xFFFF_THING_PARTY) && (G0309_i_PartyMapIndex != L0714_ui_MapIndexSource)) {
                                        F0174_DUNGEON_SetCurrentMapAndPartyMap(L0714_ui_MapIndexSource);
                                } else {
                                        F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                                }
                        }
                }
        }
        if (P0558_i_SourceMapX >= 0) {
                if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                        F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);
                } else {
                        if (L0713_B_ThingLevitates) {
                                F0164_DUNGEON_UnlinkThingFromList(P0557_T_Thing, 0, P0558_i_SourceMapX, P0559_i_SourceMapY);
                        } else {
                                F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, P0557_T_Thing, (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (P0558_i_SourceMapX == G0306_i_PartyMapX) && (P0559_i_SourceMapY == G0307_i_PartyMapY), C0_FALSE);
                        }
                }
        }
        if (P0560_i_DestinationMapX >= 0) {
                if (P0557_T_Thing == C0xFFFF_THING_PARTY) {
                        F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                        if ((P0557_T_Thing = F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)) != C0xFFFE_THING_ENDOFLIST) { /* Delete group if party moves onto its square */
                                F0188_GROUP_DropGroupPossessions(G0306_i_PartyMapX, G0307_i_PartyMapY, P0557_T_Thing, C01_MODE_PLAY_IF_PRIORITIZED);
                                F0189_GROUP_Delete(G0306_i_PartyMapX, G0307_i_PartyMapY);
                        }
                        if (L0715_ui_MapIndexDestination == L0714_ui_MapIndexSource) {
                                F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);
                        } else {
                                F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                                G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination;
                        }
                } else {
                        if (L0710_i_ThingType == C04_THING_TYPE_GROUP) {
                                F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                                L0712_ps_Teleporter = (TELEPORTER*)F0156_DUNGEON_GetThingData(P0557_T_Thing);
                                AL0708_i_ActiveGroupIndex = ((GROUP*)L0712_ps_Teleporter)->ActiveGroupIndex;
                                if (((L0715_ui_MapIndexDestination == G0309_i_PartyMapIndex) && (P0560_i_DestinationMapX == G0306_i_PartyMapX) && (P0561_i_DestinationMapY == G0307_i_PartyMapY)) || (F0175_GROUP_GetThing(P0560_i_DestinationMapX, P0561_i_DestinationMapY) != C0xFFFE_THING_ENDOFLIST)) { /* If a group tries to move to the party square or over another group then create an event to move the group later */
                                        F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                                        if (P0558_i_SourceMapX >= 0) {
                                                F0181_GROUP_DeleteEvents(P0558_i_SourceMapX, P0559_i_SourceMapY);
                                        }
                                        if (L0721_B_GroupOnPartyMap) {
                                                F0184_GROUP_RemoveActiveGroup(AL0708_i_ActiveGroupIndex);
                                        }
                                        F0265_MOVE_CreateEvent60To61_MoveGroup(P0557_T_Thing, P0560_i_DestinationMapX, P0561_i_DestinationMapY, L0715_ui_MapIndexDestination, L0726_B_Audible);
                                        return C1_TRUE; /* The specified group thing cannot be moved because the party or another group is on the destination square */
                                }
                                F0064_SOUND_RequestPlay_CPSD(F0514_MOVE_GetSound(((GROUP*)G0284_apuc_ThingData[C04_THING_TYPE_GROUP])[M013_INDEX(P0557_T_Thing)].Type), P0560_i_DestinationMapX, P0561_i_DestinationMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                if (L0721_B_GroupOnPartyMap && (L0715_ui_MapIndexDestination != G0309_i_PartyMapIndex)) { /* If the group leaves the party map */
                                        F0184_GROUP_RemoveActiveGroup(AL0708_i_ActiveGroupIndex);
                                        L0720_ui_MoveGroupResult = C1_TRUE;
                                } else {
                                        if ((L0715_ui_MapIndexDestination == G0309_i_PartyMapIndex) && (!L0721_B_GroupOnPartyMap)) { /* If the group arrives on the party map */
                                                F0183_GROUP_AddActiveGroup(P0557_T_Thing, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                                L0720_ui_MoveGroupResult = C1_TRUE;
                                        }
                                }
                                if (L0713_B_ThingLevitates) {
                                        F0163_DUNGEON_LinkThingToList(P0557_T_Thing, 0, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                } else {
                                        F0276_SENSOR_ProcessThingAdditionOrRemoval(P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing, C0_FALSE, C1_TRUE);
                                }
                                if (L0720_ui_MoveGroupResult || (P0558_i_SourceMapX < 0)) { /* If group moved from one map to another or if it was just placed on a square */
                                        F0180_GROUP_StartWandering(P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                }
                                F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                                if (P0558_i_SourceMapX >= 0) {
                                        if (L0720_ui_MoveGroupResult > C1_TRUE) { /* If the group behavior was C6_BEHAVIOR_ATTACK before being teleported from and to the party map */
                                                F0182_GROUP_StopAttacking(&G0375_ps_ActiveGroups[L0720_ui_MoveGroupResult - 2], P0558_i_SourceMapX, P0559_i_SourceMapY);
                                        } else {
                                                if (L0720_ui_MoveGroupResult) { /* If the group was teleported or leaved the party map or entered the party map */
                                                        F0181_GROUP_DeleteEvents(P0558_i_SourceMapX, P0559_i_SourceMapY);
                                                }
                                        }
                                }
                                return L0720_ui_MoveGroupResult;
                        }
                        else {
                                F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination);
                                if (L0710_i_ThingType == C14_THING_TYPE_PROJECTILE) { /* BUG0_29 An explosion can trigger a floor sensor. Explosions do not trigger floor sensors on the square where they are created. However, if an explosion is moved by a teleporter (or by falling into a pit, see BUG0_26) after it was created, it can trigger floor sensors on the destination square. This is because explosions are not considered as levitating in the code, while projectiles are. The condition here should be (L0713_B_ThingLevitates) so that explosions would not start sensor processing on their destination square as they should be Levitating. This would work if F0264_MOVE_IsLevitating returned C1_TRUE for explosions (see BUG0_26) */
                                        F0163_DUNGEON_LinkThingToList(P0557_T_Thing, 0, P0560_i_DestinationMapX, P0561_i_DestinationMapY);
                                } else {
                                        F0276_SENSOR_ProcessThingAdditionOrRemoval(P0560_i_DestinationMapX, P0561_i_DestinationMapY, P0557_T_Thing, (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (P0560_i_DestinationMapX == G0306_i_PartyMapX) && (P0561_i_DestinationMapY == G0307_i_PartyMapY), C1_TRUE);
                                }
                                F0173_DUNGEON_SetCurrentMap(L0714_ui_MapIndexSource);
                        }
                }
        }
        return C0_FALSE;
}


int16_t F0514_MOVE_GetSound(
int16_t P2099_i_Type FINAL_SEPARATOR
{
        REGISTER int16_t L3025_i_SoundOrdinal;


        if (G0300_B_PartyIsResting) {
                return -1;
        }
        if ((L3025_i_SoundOrdinal = G0243_as_Graphic559_CreatureInfo[P2099_i_Type].AttackSoundOrdinal) != 0) {
                return G2003_aauc_CreatureSounds[M001_ORDINAL_TO_INDEX(L3025_i_SoundOrdinal)][C1_MOVEMENT_SOUND];
        } else {
                return -1;
        }
}

void F0268_SENSOR_AddEvent(
unsigned char P0562_uc_Type   SEPARATOR
unsigned char P0563_uc_MapX   SEPARATOR
unsigned char P0564_uc_MapY   SEPARATOR
unsigned char P0565_uc_Cell   SEPARATOR
unsigned char P0566_uc_Effect SEPARATOR
long          P0567_l_Time    FINAL_SEPARATOR
{
        EVENT L0729_s_Event;


        M033_SET_MAP_AND_TIME(L0729_s_Event.Map_Time, G0272_i_CurrentMapIndex, P0567_l_Time);
        L0729_s_Event.A.A.Type = P0562_uc_Type;
        switch (P0566_uc_Effect) {
                case C01_EFFECT_CLEAR:
                        L0729_s_Event.A.A.Priority = 3;
                        break;
                case C02_EFFECT_TOGGLE:
                        L0729_s_Event.A.A.Priority = 2;
                        break;
                case C00_EFFECT_SET:
                        L0729_s_Event.A.A.Priority = 1;
        }
        L0729_s_Event.B.Location.MapX = P0563_uc_MapX;
        L0729_s_Event.B.Location.MapY = P0564_uc_MapY;
        L0729_s_Event.C.A.Cell = P0565_uc_Cell;
        L0729_s_Event.C.A.Effect = P0566_uc_Effect;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0729_s_Event);
}

STATICFUNCTION void F0269_SENSOR_AddSkillExperience(
int16_t                   P0568_i_SkillIndex  SEPARATOR
REGISTER unsigned int16_t P0569_ui_Experience SEPARATOR
BOOLEAN                   P0570_B_LeaderOnly  FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0731_ps_Champion;
        REGISTER unsigned int16_t L0730_ui_ChampionIndex;


        if (P0570_B_LeaderOnly) {
                if (G0411_i_LeaderIndex != CM1_CHAMPION_NONE) {
                        F0304_CHAMPION_AddSkillExperience(G0411_i_LeaderIndex, P0568_i_SkillIndex, P0569_ui_Experience);
                }
        } else {
                P0569_ui_Experience /= G0305_ui_PartyChampionCount;
                L0731_ps_Champion = M516_CHAMPIONS;
                for (L0730_ui_ChampionIndex = C00_CHAMPION_FIRST; L0730_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0731_ps_Champion++, L0730_ui_ChampionIndex++) {
                        if (L0731_ps_Champion->CurrentHealth) {
                                F0304_CHAMPION_AddSkillExperience(L0730_ui_ChampionIndex, P0568_i_SkillIndex, P0569_ui_Experience);
                        }
                }
        }
}

/* While processing a list of sensors, this function may be called several times. The rotation effect of the last processed sensor is stored in global variables. When processing of the list of sensors is complete, a call to F0271_SENSOR_ProcessRotationEffect is made to perform the actual effect (either nothing or a rotation of the list of sensors). This ensures that only one rotation can occur when processing sensors on a given square */
STATICFUNCTION void F0270_SENSOR_TriggerLocalEffect(
int16_t P0571_i_SensorLocalEffect     SEPARATOR
int16_t P0572_i_SensorLocalEffectX    SEPARATOR
int16_t P0573_i_SensorLocalEffectY    SEPARATOR
int16_t P0574_i_SensorLocalEffectCell FINAL_SEPARATOR
{
        if (P0571_i_SensorLocalEffect == C10_EFFECT_ADD_300XP_STEAL_SKILL) {
                F0269_SENSOR_AddSkillExperience(C08_SKILL_STEAL, 300, P0574_i_SensorLocalEffectCell != CM1_CELL_ANY);
                return;
        }
        G0403_i_SensorRotationEffect = P0571_i_SensorLocalEffect;
        G0404_i_SensorRotationEffectMapX = P0572_i_SensorLocalEffectX;
        G0405_i_SensorRotationEffectMapY = P0573_i_SensorLocalEffectY;
        G0406_i_SensorRotationEffectCell = P0574_i_SensorLocalEffectCell;
}

void F0271_SENSOR_ProcessRotationEffect(
void
)
{
        REGISTER SENSOR* L0735_ps_LastSensor;
        REGISTER THING L0733_T_LastSensorThing;
        REGISTER THING L0732_T_FirstSensorThing;
        REGISTER SENSOR* L0734_ps_FirstSensor;


        if (G0403_i_SensorRotationEffect == CM1_EFFECT_NONE)
                return;
        switch (G0403_i_SensorRotationEffect) {
                case C01_EFFECT_CLEAR:
                case C02_EFFECT_TOGGLE:
                        L0732_T_FirstSensorThing = F0161_DUNGEON_GetSquareFirstThing(G0404_i_SensorRotationEffectMapX, G0405_i_SensorRotationEffectMapY);
                        while (!((M012_TYPE(L0732_T_FirstSensorThing) == C03_THING_TYPE_SENSOR) && ((G0406_i_SensorRotationEffectCell == CM1_CELL_ANY) || (M011_CELL(L0732_T_FirstSensorThing) == G0406_i_SensorRotationEffectCell)))) {
                                L0732_T_FirstSensorThing = F0159_DUNGEON_GetNextThing(L0732_T_FirstSensorThing);
                        }
                        L0734_ps_FirstSensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0732_T_FirstSensorThing);
                        L0733_T_LastSensorThing = L0734_ps_FirstSensor->Remote.Next;
                        while ((L0733_T_LastSensorThing != C0xFFFE_THING_ENDOFLIST) && !((M012_TYPE(L0733_T_LastSensorThing) == C03_THING_TYPE_SENSOR) && ((G0406_i_SensorRotationEffectCell == CM1_CELL_ANY) || (M011_CELL(L0733_T_LastSensorThing) == G0406_i_SensorRotationEffectCell)))) {
                                L0733_T_LastSensorThing = F0159_DUNGEON_GetNextThing(L0733_T_LastSensorThing);
                        }
                        if (L0733_T_LastSensorThing == C0xFFFE_THING_ENDOFLIST)
                                break;
                        F0164_DUNGEON_UnlinkThingFromList(L0732_T_FirstSensorThing, 0, G0404_i_SensorRotationEffectMapX, G0405_i_SensorRotationEffectMapY);
                        L0735_ps_LastSensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0733_T_LastSensorThing);
                        L0733_T_LastSensorThing = F0159_DUNGEON_GetNextThing(L0733_T_LastSensorThing);
                        while (((L0733_T_LastSensorThing != C0xFFFE_THING_ENDOFLIST) && (M012_TYPE(L0733_T_LastSensorThing) == C03_THING_TYPE_SENSOR))) {
                                if ((G0406_i_SensorRotationEffectCell == CM1_CELL_ANY) || (M011_CELL(L0733_T_LastSensorThing) == G0406_i_SensorRotationEffectCell)) {
                                        L0735_ps_LastSensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0733_T_LastSensorThing);
                                }
                                L0733_T_LastSensorThing = F0159_DUNGEON_GetNextThing(L0733_T_LastSensorThing);
                        }
                        L0734_ps_FirstSensor->Remote.Next = L0735_ps_LastSensor->Remote.Next;
                        L0735_ps_LastSensor->Remote.Next = L0732_T_FirstSensorThing;
                        break;
        }
        G0403_i_SensorRotationEffect = CM1_EFFECT_NONE;
}

void F0272_SENSOR_TriggerEffect(
REGISTER SENSOR* P0575_ps_Sensor SEPARATOR
int16_t          P0576_i_Effect  SEPARATOR
int16_t          P0577_i_MapX    SEPARATOR
int16_t          P0578_i_MapY    SEPARATOR
int16_t          P0579_i_Cell    FINAL_SEPARATOR
{
        REGISTER int16_t L0736_i_TargetMapX;
        REGISTER int16_t L0737_i_TargetMapY;
        REGISTER unsigned int16_t L0739_ui_SquareType;
        REGISTER long L0738_l_Time;
        REGISTER unsigned int16_t L0740_ui_TargetCell;


        if (P0575_ps_Sensor->Remote.OnceOnly) {
                M044_SET_TYPE_DISABLED(P0575_ps_Sensor);
        }
        L0738_l_Time = G0313_ul_GameTime + P0575_ps_Sensor->Remote.Value;
        if (P0575_ps_Sensor->Remote.LocalEffect) {
                F0270_SENSOR_TriggerLocalEffect(M049_LOCAL_EFFECT(P0575_ps_Sensor), P0577_i_MapX, P0578_i_MapY, P0579_i_Cell);
        } else {
                L0736_i_TargetMapX = P0575_ps_Sensor->Remote.TargetMapX;
                L0737_i_TargetMapY = P0575_ps_Sensor->Remote.TargetMapY;
                L0739_ui_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[L0736_i_TargetMapX][L0737_i_TargetMapY]);
                if (L0739_ui_SquareType == C00_ELEMENT_WALL) {
                        L0740_ui_TargetCell = P0575_ps_Sensor->Remote.TargetCell;
                } else {
                        L0740_ui_TargetCell = C00_CELL_NORTHWEST;
                }
                F0268_SENSOR_AddEvent(G0059_auc_Graphic562_SquareTypeToEventType[L0739_ui_SquareType], L0736_i_TargetMapX, L0737_i_TargetMapY, L0740_ui_TargetCell, P0576_i_Effect, L0738_l_Time);
        }
}

THING F0273_SENSOR_GetObjectOfTypeInCell(
int16_t                   P0580_i_MapX        SEPARATOR
int16_t                   P0581_i_MapY        SEPARATOR
REGISTER int16_t          P0582_i_Cell        SEPARATOR
REGISTER unsigned int16_t P0583_ui_ObjectType FINAL_SEPARATOR
{
        REGISTER THING L0741_T_Thing;


        L0741_T_Thing = F0162_DUNGEON_GetSquareFirstObject(P0580_i_MapX, P0581_i_MapY);
        while (L0741_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if (F0032_OBJECT_GetType(L0741_T_Thing) == P0583_ui_ObjectType) {
                        if ((P0582_i_Cell == CM1_CELL_ANY) || (M011_CELL(L0741_T_Thing) == P0582_i_Cell)) {
                                return L0741_T_Thing;
                        }
                }
                L0741_T_Thing = F0159_DUNGEON_GetNextThing(L0741_T_Thing);
        }
        return C0xFFFF_THING_NONE;
}

STATICFUNCTION BOOLEAN F0274_SENSOR_IsObjectInPartyPossession(
int16_t P0584_i_ObjectType FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0745_ps_Champion;
        REGISTER THING* L0746_pT_Thing;
        REGISTER CONTAINER* L0749_ps_Container;
        REGISTER THING L0744_T_Thing;
        REGISTER unsigned int16_t L0743_ui_SlotIndex;
        REGISTER unsigned int16_t L0742_ui_ChampionIndex;
        REGISTER BOOLEAN L0748_B_LeaderHandObjectProcessed;
        REGISTER int16_t L0747_i_ObjectType;


        L0748_B_LeaderHandObjectProcessed = C0_FALSE;
        for (L0742_ui_ChampionIndex = C00_CHAMPION_FIRST, L0745_ps_Champion = M516_CHAMPIONS; L0742_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0742_ui_ChampionIndex++, L0745_ps_Champion++) {
                if (L0745_ps_Champion->CurrentHealth) {
                        L0746_pT_Thing = L0745_ps_Champion->Slots;
                        for (L0743_ui_SlotIndex = C00_SLOT_READY_HAND; (L0743_ui_SlotIndex < C30_SLOT_CHEST_1) && !L0748_B_LeaderHandObjectProcessed; L0743_ui_SlotIndex++) {
                                L0744_T_Thing = *L0746_pT_Thing++;
                                T0274003:
                                if ((L0747_i_ObjectType = F0032_OBJECT_GetType(L0744_T_Thing)) == P0584_i_ObjectType) {
                                        return C1_TRUE;
                                }
                                else {
                                        if (L0747_i_ObjectType == C144_ICON_CONTAINER_CHEST_CLOSED) {
                                                L0749_ps_Container = (CONTAINER*)F0156_DUNGEON_GetThingData(L0744_T_Thing);
                                                L0744_T_Thing = L0749_ps_Container->Slot;
                                                while (L0744_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                                                        if (F0032_OBJECT_GetType(L0744_T_Thing) == P0584_i_ObjectType) {
                                                                return C1_TRUE;
                                                        }
                                                        L0744_T_Thing = F0159_DUNGEON_GetNextThing(L0744_T_Thing);
                                                }
                                        }
                                }
                        }
                }
        }
        if (!L0748_B_LeaderHandObjectProcessed) {
                L0748_B_LeaderHandObjectProcessed = C1_TRUE;
                L0744_T_Thing = G4055_s_LeaderHandObject.Thing;
                goto T0274003;
        }
        return C0_FALSE;
}

BOOLEAN F0275_SENSOR_IsTriggeredByClickOnWall(
REGISTER int16_t P0585_i_MapX  SEPARATOR
int16_t          P0586_i_MapY  SEPARATOR
unsigned int16_t P0587_ui_Cell FINAL_SEPARATOR
{
        REGISTER SENSOR* L0755_ps_Sensor;
        REGISTER GENERIC* L0754_ps_Generic;
        REGISTER THING L0750_T_ThingBeingProcessed;
        REGISTER THING L0761_T_LeaderHandObject;
        REGISTER unsigned int16_t L0752_ui_Cell;
        REGISTER BOOLEAN L0753_B_DoNotTriggerSensor;
        unsigned int16_t L0757_ui_SensorType;
        unsigned int16_t L0758_ui_SensorData;
        int16_t L0756_i_SensorEffect;
        REGISTER unsigned int16_t L0751_ui_ThingType;
        BOOLEAN L0759_B_AtLeastOneSensorWasTriggered;
        THING L0763_T_LastProcessedThing;
        THING L0762_T_ThingOnSquare;
        THING L0764_T_SquareFirstThing;
        int16_t L0760_ai_SensorCountToProcessPerCell[4];


        L0759_B_AtLeastOneSensorWasTriggered = C0_FALSE;
        L0761_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing;
        F0008_MAIN_ClearBytes(M772_CAST_PC(L0760_ai_SensorCountToProcessPerCell), (long)sizeof(L0760_ai_SensorCountToProcessPerCell));
        L0764_T_SquareFirstThing = L0750_T_ThingBeingProcessed = F0161_DUNGEON_GetSquareFirstThing(P0585_i_MapX, P0586_i_MapY);
        while (L0750_T_ThingBeingProcessed != C0xFFFE_THING_ENDOFLIST) {
                if ((L0751_ui_ThingType = M012_TYPE(L0750_T_ThingBeingProcessed)) == C03_THING_TYPE_SENSOR) {
                        L0760_ai_SensorCountToProcessPerCell[M011_CELL(L0750_T_ThingBeingProcessed)]++;
                } else {
                        if (L0751_ui_ThingType >= C04_THING_TYPE_GROUP)
                                break;
                }
                L0750_T_ThingBeingProcessed = F0159_DUNGEON_GetNextThing(L0750_T_ThingBeingProcessed);
        }
        L0763_T_LastProcessedThing = L0750_T_ThingBeingProcessed = L0764_T_SquareFirstThing;
        while (L0750_T_ThingBeingProcessed != C0xFFFE_THING_ENDOFLIST) {
                if ((L0751_ui_ThingType = M012_TYPE(L0750_T_ThingBeingProcessed)) == C03_THING_TYPE_SENSOR) {
                        L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell = M011_CELL(L0750_T_ThingBeingProcessed)]--;
                        L0755_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0750_T_ThingBeingProcessed);
                        if ((L0757_ui_SensorType = M039_TYPE(L0755_ps_Sensor)) == C000_SENSOR_DISABLED)
                                goto T0275058_ProceedToNextThing;
                        if ((G0411_i_LeaderIndex == CM1_CHAMPION_NONE) && (L0757_ui_SensorType != C127_SENSOR_WALL_CHAMPION_PORTRAIT))
                                goto T0275058_ProceedToNextThing;
                        if (L0752_ui_Cell != P0587_ui_Cell)
                                goto T0275058_ProceedToNextThing;
                        L0758_ui_SensorData = M040_DATA(L0755_ps_Sensor);
                        L0756_i_SensorEffect = L0755_ps_Sensor->Remote.Effect;
                        switch (L0757_ui_SensorType) {
                                case C001_SENSOR_WALL_ORNAMENT_CLICK:
                                        L0753_B_DoNotTriggerSensor = C0_FALSE;
                                        if (L0755_ps_Sensor->Remote.Effect == C03_EFFECT_HOLD) {
                                                goto T0275058_ProceedToNextThing;
                                        }
                                        break;
                                case C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT:
                                        L0753_B_DoNotTriggerSensor = (G0415_ui_LeaderEmptyHanded != L0755_ps_Sensor->Remote.RevertEffect);
                                        break;
                                case C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR:
                                case C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS:
                                        if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
                                                goto T0275058_ProceedToNextThing;
                                case C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT:
                                case C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED:
                                        L0753_B_DoNotTriggerSensor = ((L0758_ui_SensorData == F0032_OBJECT_GetType(L0761_T_LeaderHandObject)) == L0755_ps_Sensor->Remote.RevertEffect);
                                        if (!L0753_B_DoNotTriggerSensor && (L0757_ui_SensorType == C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR)) {
                                                if (L0763_T_LastProcessedThing == L0750_T_ThingBeingProcessed) /* If the sensor is the only one of its type on the cell */
                                                        break;
                                                L0754_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0763_T_LastProcessedThing);
                                                ((SENSOR*)L0754_ps_Generic)->Remote.Next = L0755_ps_Sensor->Remote.Next;
                                                L0755_ps_Sensor->Remote.Next = C0xFFFF_THING_NONE;
                                                L0750_T_ThingBeingProcessed = L0763_T_LastProcessedThing;
                                        }
                                        if (!L0753_B_DoNotTriggerSensor && (L0757_ui_SensorType == C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS)) {
#ifdef PC_FIX_CODE_SIZE
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
        L0750_T_ThingBeingProcessed++;
#endif
                                                F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
                                        }
                                        break;
                                case C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE_SENSORS:
                                        if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
                                                goto T0275058_ProceedToNextThing;
                                        L0753_B_DoNotTriggerSensor = !G0415_ui_LeaderEmptyHanded;
                                        if (!L0753_B_DoNotTriggerSensor) {
                                                F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
                                        }
                                        break;
                                case C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE_SENSORS:
                                        if (G0415_ui_LeaderEmptyHanded) {
                                                if ((L0761_T_LeaderHandObject = F0273_SENSOR_GetObjectOfTypeInCell(P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell, L0758_ui_SensorData)) == C0xFFFF_THING_NONE)
                                                        goto T0275058_ProceedToNextThing;
                                                F0164_DUNGEON_UnlinkThingFromList(L0761_T_LeaderHandObject, 0, P0585_i_MapX, P0586_i_MapY);
                                                F0297_CHAMPION_PutObjectInLeaderHand(L0761_T_LeaderHandObject, C1_TRUE);
                                        } else {
                                                if ((F0032_OBJECT_GetType(L0761_T_LeaderHandObject) != L0758_ui_SensorData) || (F0273_SENSOR_GetObjectOfTypeInCell(P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell, L0758_ui_SensorData) != C0xFFFF_THING_NONE))
                                                        goto T0275058_ProceedToNextThing;
                                                F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
                                                F0163_DUNGEON_LinkThingToList(M015_THING_WITH_NEW_CELL(L0761_T_LeaderHandObject, L0752_ui_Cell), 0, P0585_i_MapX, P0586_i_MapY);
                                                L0761_T_LeaderHandObject = C0xFFFF_THING_NONE;
                                        }
                                        F0270_SENSOR_TriggerLocalEffect(C02_EFFECT_TOGGLE, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
                                        L0753_B_DoNotTriggerSensor = (L0756_i_SensorEffect == C03_EFFECT_HOLD) && !G0415_ui_LeaderEmptyHanded;
                                        break;
                                case C016_SENSOR_WALL_OBJECT_EXCHANGER:
                                        if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
                                                goto T0275058_ProceedToNextThing;
                                        L0762_T_ThingOnSquare = F0162_DUNGEON_GetSquareFirstObject(P0585_i_MapX, P0586_i_MapY);
                                        if ((F0032_OBJECT_GetType(L0761_T_LeaderHandObject) != L0758_ui_SensorData) || (L0762_T_ThingOnSquare == C0xFFFF_THING_NONE))
                                                goto T0275058_ProceedToNextThing;
                                        F0164_DUNGEON_UnlinkThingFromList(L0762_T_ThingOnSquare, 0, P0585_i_MapX, P0586_i_MapY);
                                        F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
                                        F0163_DUNGEON_LinkThingToList(M015_THING_WITH_NEW_CELL(L0761_T_LeaderHandObject, L0752_ui_Cell), 0, P0585_i_MapX, P0586_i_MapY);
                                        F0297_CHAMPION_PutObjectInLeaderHand(L0762_T_ThingOnSquare, C1_TRUE);
                                        L0753_B_DoNotTriggerSensor = C0_FALSE;
                                        break;
                                case C127_SENSOR_WALL_CHAMPION_PORTRAIT:
                                        F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData);
                                        goto T0275058_ProceedToNextThing;
                                default:
                                        goto T0275058_ProceedToNextThing;
                        }
                        if (L0756_i_SensorEffect == C03_EFFECT_HOLD) {
                                L0756_i_SensorEffect = L0753_B_DoNotTriggerSensor ? C01_EFFECT_CLEAR : C00_EFFECT_SET;
                                L0753_B_DoNotTriggerSensor = C0_FALSE;
                        }
                        if (!L0753_B_DoNotTriggerSensor) {
                                L0759_B_AtLeastOneSensorWasTriggered = C1_TRUE;
                                if (L0755_ps_Sensor->Remote.Audible) {
                                        F0064_SOUND_RequestPlay_CPSD(C01_SOUND_SWITCH, G0306_i_PartyMapX, G0307_i_PartyMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                                }
                                if (!G0415_ui_LeaderEmptyHanded && ((L0757_ui_SensorType == C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED) || (L0757_ui_SensorType == C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS) || (L0757_ui_SensorType == C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR))) {
                                        L0754_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0761_T_LeaderHandObject);
                                        L0754_ps_Generic->Next = C0xFFFF_THING_NONE;
                                        F0298_CHAMPION_GetObjectRemovedFromLeaderHand();
                                        L0761_T_LeaderHandObject = C0xFFFF_THING_NONE;
                                } else {
                                        if (G0415_ui_LeaderEmptyHanded &&
                                           (L0757_ui_SensorType == C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE_SENSORS) &&
                                           ((L0761_T_LeaderHandObject = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(L0758_ui_SensorData)) != C0xFFFF_THING_NONE)) {
                                                F0297_CHAMPION_PutObjectInLeaderHand(L0761_T_LeaderHandObject, C1_TRUE);
                                        }
                                }
                                F0272_SENSOR_TriggerEffect(L0755_ps_Sensor, L0756_i_SensorEffect, P0585_i_MapX, P0586_i_MapY, L0752_ui_Cell);
                        }
                        goto T0275058_ProceedToNextThing;
                }
                if (L0751_ui_ThingType >= C04_THING_TYPE_GROUP)
                        break;
                T0275058_ProceedToNextThing:
                L0763_T_LastProcessedThing = L0750_T_ThingBeingProcessed;
                L0750_T_ThingBeingProcessed = F0159_DUNGEON_GetNextThing(L0750_T_ThingBeingProcessed);
        }
        F0271_SENSOR_ProcessRotationEffect();
        return L0759_B_AtLeastOneSensorWasTriggered;
}

void F0276_SENSOR_ProcessThingAdditionOrRemoval(
unsigned int16_t P0588_ui_MapX       SEPARATOR
unsigned int16_t P0589_ui_MapY       SEPARATOR
REGISTER THING   P0590_T_Thing       SEPARATOR
BOOLEAN          P0591_B_PartySquare SEPARATOR
BOOLEAN          P0592_B_AddThing    FINAL_SEPARATOR
{
        REGISTER SENSOR* L0769_ps_Sensor;
        REGISTER THING L0766_T_Thing;
        REGISTER int16_t L0767_i_ThingType;
        REGISTER BOOLEAN L0768_B_TriggerSensor;
        REGISTER unsigned int16_t L0771_ui_ThingType;
        int16_t L0779_i_SensorData;
        int16_t L0770_ui_SensorTriggeredCell;
        int16_t L0774_i_ObjectType;
        BOOLEAN L0772_B_SquareContainsObject;
        BOOLEAN L0773_B_SquareContainsGroup;
        BOOLEAN L0775_B_SquareContainsThingOfSameType;
        BOOLEAN L0776_B_SquareContainsThingOfDifferentType;
        int16_t L0778_i_Effect;
        unsigned int16_t L0777_ui_Square;


        if (P0590_T_Thing != C0xFFFF_THING_PARTY) {
                L0767_i_ThingType = M012_TYPE(P0590_T_Thing);
                L0774_i_ObjectType = F0032_OBJECT_GetType(P0590_T_Thing);
        } else {
                L0767_i_ThingType = CM1_THING_TYPE_PARTY;
                L0774_i_ObjectType = C0xFFFF_ICON_NONE;
        }
        if ((!P0592_B_AddThing) && (L0767_i_ThingType != CM1_THING_TYPE_PARTY)) {
                F0164_DUNGEON_UnlinkThingFromList(P0590_T_Thing, 0, P0588_ui_MapX, P0589_ui_MapY);
        }
        if (M034_SQUARE_TYPE(L0777_ui_Square = G0271_ppuc_CurrentMapData[P0588_ui_MapX][P0589_ui_MapY]) == C00_ELEMENT_WALL) {
                L0770_ui_SensorTriggeredCell = M011_CELL(P0590_T_Thing);
        } else {
                L0770_ui_SensorTriggeredCell = CM1_CELL_ANY;
        }
        L0772_B_SquareContainsObject = L0773_B_SquareContainsGroup = L0775_B_SquareContainsThingOfSameType = L0776_B_SquareContainsThingOfDifferentType = C0_FALSE;
        L0766_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0588_ui_MapX, P0589_ui_MapY);
        if (L0770_ui_SensorTriggeredCell == CM1_CELL_ANY) {
                while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                        if ((L0771_ui_ThingType = M012_TYPE(L0766_T_Thing)) == C04_THING_TYPE_GROUP) {
                                L0773_B_SquareContainsGroup = C1_TRUE;
                        } else {
                                if ((L0771_ui_ThingType == C02_THING_TYPE_TEXTSTRING) && (L0767_i_ThingType == CM1_THING_TYPE_PARTY) && P0592_B_AddThing && !P0591_B_PartySquare) {
                                        F0168_DUNGEON_DecodeText(M774_CAST_PUC(G0353_ac_StringBuildBuffer), L0766_T_Thing, C1_TEXT_TYPE_MESSAGE);
                                        F0047_TEXT_MESSAGEAREA_PrintMessage(C15_COLOR_WHITE, G0353_ac_StringBuildBuffer);
                                } else {
                                        if ((L0771_ui_ThingType > C04_THING_TYPE_GROUP) && (L0771_ui_ThingType < C14_THING_TYPE_PROJECTILE)) {
                                                L0772_B_SquareContainsObject = C1_TRUE;
                                                L0775_B_SquareContainsThingOfSameType |= (F0032_OBJECT_GetType(L0766_T_Thing) == L0774_i_ObjectType);
                                                L0776_B_SquareContainsThingOfDifferentType |= (F0032_OBJECT_GetType(L0766_T_Thing) != L0774_i_ObjectType);
                                        }
                                }
                        }
                        L0766_T_Thing = F0159_DUNGEON_GetNextThing(L0766_T_Thing);
                }
        } else {
                while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                        if ((L0770_ui_SensorTriggeredCell == M011_CELL(L0766_T_Thing)) && (M012_TYPE(L0766_T_Thing) > C04_THING_TYPE_GROUP)) {
                                L0772_B_SquareContainsObject = C1_TRUE;
                                L0775_B_SquareContainsThingOfSameType |= (F0032_OBJECT_GetType(L0766_T_Thing) == L0774_i_ObjectType);
                                L0776_B_SquareContainsThingOfDifferentType |= (F0032_OBJECT_GetType(L0766_T_Thing) != L0774_i_ObjectType);
                        }
                        L0766_T_Thing = F0159_DUNGEON_GetNextThing(L0766_T_Thing);
                }
        }
        if (P0592_B_AddThing && (L0767_i_ThingType != CM1_THING_TYPE_PARTY)) {
                F0163_DUNGEON_LinkThingToList(P0590_T_Thing, 0, P0588_ui_MapX, P0589_ui_MapY);
        }
        L0766_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0588_ui_MapX, P0589_ui_MapY);
        while (L0766_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if ((L0771_ui_ThingType = M012_TYPE(L0766_T_Thing)) == C03_THING_TYPE_SENSOR) {
                        L0769_ps_Sensor = (SENSOR*)F0156_DUNGEON_GetThingData(L0766_T_Thing);
                        if (M039_TYPE(L0769_ps_Sensor) == C000_SENSOR_DISABLED)
                                goto T0276079;
                        L0779_i_SensorData = M040_DATA(L0769_ps_Sensor);
                        L0768_B_TriggerSensor = P0592_B_AddThing;
                        if (L0770_ui_SensorTriggeredCell == CM1_CELL_ANY) {
                                switch (M039_TYPE(L0769_ps_Sensor)) {
                                        case C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT:
                                                if (P0591_B_PartySquare || L0772_B_SquareContainsObject || L0773_B_SquareContainsGroup) /* BUG0_30 A floor sensor is not triggered when you put an object on the floor if a levitating creature is present on the same square. The condition to determine if the sensor should be triggered checks if there is a creature on the square but does not check whether the creature is levitating. While it is normal not to trigger the sensor if there is a non levitating creature on the square (because it was already triggered by the creature itself), a levitating creature should not prevent triggering the sensor with an object. */
                                                        goto T0276079;
                                                break;
                                        case C002_SENSOR_FLOOR_THERON_PARTY_CREATURE:
                                                if ((L0767_i_ThingType > C04_THING_TYPE_GROUP) || P0591_B_PartySquare || L0773_B_SquareContainsGroup)
                                                        goto T0276079;
                                                break;
                                        case C003_SENSOR_FLOOR_PARTY:
                                                if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || (G0305_ui_PartyChampionCount == 0))
                                                        goto T0276079;
                                                if (L0779_i_SensorData == 0) {
                                                        if (P0591_B_PartySquare)
                                                                goto T0276079;
                                                } else {
                                                        if (!P0592_B_AddThing) {
                                                                L0768_B_TriggerSensor = C0_FALSE;
                                                        } else {
#ifdef PC_FIX_CODE_SIZE
        L0766_T_Thing++;
#endif
                                                                L0768_B_TriggerSensor = (L0779_i_SensorData == M000_INDEX_TO_ORDINAL(G0308_i_PartyDirection));
                                                        }
                                                }
                                                break;
                                        case C004_SENSOR_FLOOR_OBJECT:
                                                if ((L0779_i_SensorData != F0032_OBJECT_GetType(P0590_T_Thing)) || L0775_B_SquareContainsThingOfSameType)
                                                        goto T0276079;
                                                break;
                                        case C005_SENSOR_FLOOR_PARTY_ON_STAIRS:
                                                if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || (M034_SQUARE_TYPE(L0777_ui_Square) != C03_ELEMENT_STAIRS))
                                                        goto T0276079;
                                                break;
                                        case C006_SENSOR_FLOOR_GROUP_GENERATOR:
                                                goto T0276079;
                                        case C007_SENSOR_FLOOR_CREATURE:
                                                if ((L0767_i_ThingType > C04_THING_TYPE_GROUP) || (L0767_i_ThingType == CM1_THING_TYPE_PARTY) || L0773_B_SquareContainsGroup)
                                                        goto T0276079;
                                                break;
                                        case C008_SENSOR_FLOOR_PARTY_POSSESSION:
                                                if (L0767_i_ThingType != CM1_THING_TYPE_PARTY)
                                                        goto T0276079;
                                                L0768_B_TriggerSensor = F0274_SENSOR_IsObjectInPartyPossession(L0779_i_SensorData);
                                                break;
                                        case C009_SENSOR_FLOOR_VERSION_CHECKER:
                                                if ((L0767_i_ThingType != CM1_THING_TYPE_PARTY) || !P0592_B_AddThing || P0591_B_PartySquare)
                                                        goto T0276079;
#ifdef PC_FIX_CODE_SIZE
        L0766_T_Thing++;
        L0766_T_Thing++;
#endif
                                                L0768_B_TriggerSensor = (L0779_i_SensorData <= 34);
                                                break;
                                        default:
                                                goto T0276079;
                                }
                        } else {
                                if (L0770_ui_SensorTriggeredCell != M011_CELL(L0766_T_Thing))
                                        goto T0276079;
                                switch (M039_TYPE(L0769_ps_Sensor)) {
                                        case C001_SENSOR_WALL_ORNAMENT_CLICK:
                                                if (L0772_B_SquareContainsObject)
                                                        goto T0276079;
                                                break;
                                        case C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT:
                                                if (L0775_B_SquareContainsThingOfSameType || (M040_DATA(L0769_ps_Sensor) != F0032_OBJECT_GetType(P0590_T_Thing)))
                                                        goto T0276079;
                                                break;
                                        case C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT:
                                                if (L0776_B_SquareContainsThingOfDifferentType || (M040_DATA(L0769_ps_Sensor) == F0032_OBJECT_GetType(P0590_T_Thing)))
                                                        goto T0276079;
                                                break;
                                        default:
                                                goto T0276079;
                                }
                        }
                        L0768_B_TriggerSensor ^= L0769_ps_Sensor->Remote.RevertEffect;
                        if ((L0778_i_Effect = L0769_ps_Sensor->Remote.Effect) == C03_EFFECT_HOLD) {
                                L0778_i_Effect = L0768_B_TriggerSensor ? C00_EFFECT_SET : C01_EFFECT_CLEAR;
                        } else {
                                if (!L0768_B_TriggerSensor)
                                        goto T0276079;
                        }
                        if (L0769_ps_Sensor->Remote.Audible) {
                                F0064_SOUND_RequestPlay_CPSD(C01_SOUND_SWITCH, P0588_ui_MapX, P0589_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                        }
                        F0272_SENSOR_TriggerEffect(L0769_ps_Sensor, L0778_i_Effect, P0588_ui_MapX, P0589_ui_MapY, CM1_CELL_ANY);
                        goto T0276079;
                }
                if (L0771_ui_ThingType >= C04_THING_TYPE_GROUP)
                        break;
                T0276079:
                L0766_T_Thing = F0159_DUNGEON_GetNextThing(L0766_T_Thing);
        }
        F0271_SENSOR_ProcessRotationEffect();
}
/* END MOVESENS.C */

/* BEGIN PROJEXPL.C */
#ifndef COMPILE_H
#include "COMPILE.H"
#endif
long G0361_l_LastCreatureAttackTime = -200;
long G0362_l_LastPartyMovementTime;
int16_t G0363_i_SecondaryDirectionToOrFromParty;
int16_t G0364_i_CreatureDamageOutcome;
BOOLEAN G0365_B_CreateLauncherProjectile;
int16_t G0366_i_ProjectilePoisonAttack;
int16_t G0367_i_ProjectileAttackType;



void F0212_PROJECTILE_Create(
REGISTER THING P0433_T_Thing          SEPARATOR
int16_t        P0434_i_MapX           SEPARATOR
int16_t        P0435_i_MapY           SEPARATOR
int16_t        P0436_i_Cell           SEPARATOR
int16_t        P0437_i_Direction      SEPARATOR
unsigned char  P0438_uc_KineticEnergy SEPARATOR
unsigned char  P0439_uc_Attack        SEPARATOR
unsigned char  P0440_uc_StepEnergy    FINAL_SEPARATOR
{
        register PROJECTILE* L0467_ps_Projectile;
        register THING L0466_T_ProjectileThing;
        EVENT L0468_s_Event;


        if ((L0466_T_ProjectileThing = F0166_DUNGEON_GetUnusedThing(C14_THING_TYPE_PROJECTILE)) == C0xFFFF_THING_NONE) { /* BUG0_16 If the game cannot create a projectile thing because it has run out of such things (60 maximum) then the object being thrown/shot/launched is orphaned. If the game has run out of projectile things it will try to remove a projectile from elsewhere in the dungeon, except in an area of 11x11 squares centered around the party (to make sure the player cannot actually see the thing disappear on screen) */
                if (M012_TYPE(P0433_T_Thing) != C15_THING_TYPE_EXPLOSION) {
                        F0267_MOVE_GetMoveResult_CPSCE(M015_THING_WITH_NEW_CELL(P0433_T_Thing, P0436_i_Cell), CM1_MAPX_NOT_ON_A_SQUARE, 0, P0434_i_MapX, P0435_i_MapY);
                }
                return;
        }
        L0466_T_ProjectileThing = M015_THING_WITH_NEW_CELL(L0466_T_ProjectileThing, P0436_i_Cell);
        L0467_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(L0466_T_ProjectileThing);
        L0467_ps_Projectile->Slot = P0433_T_Thing;
        L0467_ps_Projectile->KineticEnergy = F0024_MAIN_GetMinimumValue(P0438_uc_KineticEnergy, 255);
        L0467_ps_Projectile->Attack = P0439_uc_Attack;
        F0163_DUNGEON_LinkThingToList(L0466_T_ProjectileThing, 0, P0434_i_MapX, P0435_i_MapY); /* Projectiles are added on the square and not 'moved' onto the square. In the case of a projectile launcher sensor, this means that the new projectile traverses the square in front of the launcher without any trouble: there is no impact if it is a wall, the projectile direction is not changed if it is a teleporter. Impacts with creatures and champions are still processed */
        M033_SET_MAP_AND_TIME(L0468_s_Event.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime + 1);
        if (G0365_B_CreateLauncherProjectile) {
                L0468_s_Event.A.A.Type = C49_EVENT_MOVE_PROJECTILE; /* Launcher projectiles can impact immediately */
        } else {
                L0468_s_Event.A.A.Type = C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS; /* Projectiles created by champions or creatures ignore impacts on their first movement */
        }
        L0468_s_Event.A.A.Priority = 0;
        L0468_s_Event.B.Slot = L0466_T_ProjectileThing;
        L0468_s_Event.C.Projectile.MapX = P0434_i_MapX;
        L0468_s_Event.C.Projectile.MapY = P0435_i_MapY;
        L0468_s_Event.C.Projectile.StepEnergy = P0440_uc_StepEnergy;
        L0468_s_Event.C.Projectile.Direction = P0437_i_Direction;
        L0467_ps_Projectile->EventIndex = F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0468_s_Event);
}

void F0213_EXPLOSION_Create(
REGISTER THING            P0441_T_ExplosionThing SEPARATOR
REGISTER unsigned int16_t P0442_ui_Attack        SEPARATOR
REGISTER unsigned int16_t P0443_ui_MapXCombo     SEPARATOR
REGISTER unsigned int16_t P0444_ui_MapYCombo     SEPARATOR
unsigned int16_t          P0445_ui_Cell          FINAL_SEPARATOR
#define AP0443_ui_ProjectileMapX P0443_ui_MapXCombo
#define AP0444_ui_ProjectileMapY P0444_ui_MapYCombo
{
        REGISTER EXPLOSION* L0470_ps_Explosion;
        REGISTER CREATURE_INFO* L0471_ps_CreatureInfo;
        REGISTER GROUP* L0472_ps_Group;
        REGISTER THING L0473_T_Thing;
        int16_t L0474_i_ProjectileTargetMapX;
        int16_t L0475_i_ProjectileTargetMapY;
        REGISTER int16_t L0469_i_CreatureFireResistance;
        EVENT L0476_s_Event;


        if ((L0473_T_Thing = F0166_DUNGEON_GetUnusedThing(C15_THING_TYPE_EXPLOSION)) == C0xFFFF_THING_NONE) {
                return;
        }
        L0470_ps_Explosion = &((EXPLOSION*)G0284_apuc_ThingData[C15_THING_TYPE_EXPLOSION])[M013_INDEX(L0473_T_Thing)];
        if (P0443_ui_MapXCombo <= 255) {
                L0474_i_ProjectileTargetMapX = P0443_ui_MapXCombo;
                L0475_i_ProjectileTargetMapY = P0444_ui_MapYCombo;
        } else {
                L0474_i_ProjectileTargetMapX = P0443_ui_MapXCombo & 0x00FF;
                L0475_i_ProjectileTargetMapY = P0444_ui_MapYCombo & 0x00FF;
                AP0443_ui_ProjectileMapX >>= 8;
                AP0443_ui_ProjectileMapX--;
                AP0444_ui_ProjectileMapY >>= 8;
        }
        if (P0445_ui_Cell == C0xFF_SINGLE_CENTERED_CREATURE) {
                L0470_ps_Explosion->Centered = C1_TRUE;
        } else {
                L0470_ps_Explosion->Centered = C0_FALSE;
                L0473_T_Thing = M015_THING_WITH_NEW_CELL(L0473_T_Thing, P0445_ui_Cell);
        }
        L0470_ps_Explosion->Type = P0441_T_ExplosionThing - C0xFF80_THING_FIRST_EXPLOSION;
        L0470_ps_Explosion->Attack = P0442_ui_Attack;
        if (P0441_T_ExplosionThing < C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL) {
                F0064_SOUND_RequestPlay_CPSD((P0442_ui_Attack > 80) ? C05_SOUND_STRONG_EXPLOSION : M541_SOUND_WEAK_EXPLOSION, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, C01_MODE_PLAY_IF_PRIORITIZED);
        } else {
                if (P0441_T_ExplosionThing != C0xFFA8_THING_EXPLOSION_SMOKE) {
                        F0064_SOUND_RequestPlay_CPSD(M542_SOUND_SPELL, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, C01_MODE_PLAY_IF_PRIORITIZED);
                }
        }
        F0163_DUNGEON_LinkThingToList(L0473_T_Thing, 0, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY);
        M033_SET_MAP_AND_TIME(L0476_s_Event.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime + ((P0441_T_ExplosionThing == C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1) ? 5 : 1));
        L0476_s_Event.A.A.Type = C25_EVENT_EXPLOSION;
        L0476_s_Event.A.A.Priority = 0;
        L0476_s_Event.C.Slot = L0473_T_Thing;
        L0476_s_Event.B.Location.MapX = AP0443_ui_ProjectileMapX;
        L0476_s_Event.B.Location.MapY = AP0444_ui_ProjectileMapY;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0476_s_Event);
        if ((P0441_T_ExplosionThing == C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT) || (P0441_T_ExplosionThing == C0xFF80_THING_EXPLOSION_FIREBALL)) {
                AP0443_ui_ProjectileMapX = L0474_i_ProjectileTargetMapX;
                AP0444_ui_ProjectileMapY = L0475_i_ProjectileTargetMapY;
                P0442_ui_Attack = (P0442_ui_Attack >> 1) + 1;
                P0442_ui_Attack += M002_RANDOM(P0442_ui_Attack) + 1;
                if ((P0441_T_ExplosionThing == C0xFF80_THING_EXPLOSION_FIREBALL) || (P0442_ui_Attack >>= 1)) {
                        if ((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (AP0443_ui_ProjectileMapX == G0306_i_PartyMapX) && (AP0444_ui_ProjectileMapY == G0307_i_PartyMapY)) {
                                F0324_CHAMPION_DamageAll_GetDamagedChampionCount(P0442_ui_Attack, MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND | MASK0x0004_WOUND_HEAD | MASK0x0008_WOUND_TORSO | MASK0x0010_WOUND_LEGS | MASK0x0020_WOUND_FEET, C1_ATTACK_FIRE);
                        } else {
                                if ((L0473_T_Thing = F0175_GROUP_GetThing(AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY)) != C0xFFFE_THING_ENDOFLIST) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                        L0472_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0473_T_Thing);
                                        L0471_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[L0472_ps_Group->Type];
                                        if ((L0469_i_CreatureFireResistance = M060_FIRE_RESISTANCE(L0471_ps_CreatureInfo->Resistances)) != C15_IMMUNE_TO_FIRE) {
                                                if (M007_GET(L0471_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL)) {
                                                        P0442_ui_Attack >>= 2;
                                                }
                                                if ((P0442_ui_Attack -= M002_RANDOM((L0469_i_CreatureFireResistance << 1) + 1)) > 0) {
                                                        G0364_i_CreatureDamageOutcome = F0191_GROUP_GetDamageAllCreaturesOutcome(L0472_ps_Group, AP0443_ui_ProjectileMapX, AP0444_ui_ProjectileMapY, P0442_ui_Attack, C1_TRUE);
                                                }
                                        }
                                }
                        }
                }
        }
}

void F0214_PROJECTILE_DeleteEvent(
THING P0446_T_Thing FINAL_SEPARATOR
{
        PROJECTILE* L0477_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(P0446_T_Thing);


        F0237_TIMELINE_DeleteEvent(L0477_ps_Projectile->EventIndex);
}

void F0215_PROJECTILE_Delete(
THING           P0447_T_ProjectileThing SEPARATOR
REGISTER THING* P0448_pT_GroupSlot      SEPARATOR /* When not NULL, the projectile associated thing is linked to the list of group possessions. This is used for creatures that keep some weapons thrown at them (Daggers, Arrow, Slayer, Poison Dart and Throwing Star) */
int16_t         P0449_i_MapX            SEPARATOR /* If P0448_pT_GroupSlot is NULL then the projectile associated thing is moved to the specified square */
int16_t         P0450_i_MapY            FINAL_SEPARATOR
{
        REGISTER PROJECTILE* L0480_ps_Projectile;
        REGISTER GENERIC* L0481_ps_Generic;
        REGISTER THING L0479_T_Thing;
        REGISTER THING L0478_T_PreviousThing;


        L0480_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(P0447_T_ProjectileThing);
        if (M012_TYPE(L0479_T_Thing = L0480_ps_Projectile->Slot) != C15_THING_TYPE_EXPLOSION) {
                if (P0448_pT_GroupSlot != NULL) {
                        if ((L0478_T_PreviousThing = *P0448_pT_GroupSlot) == C0xFFFE_THING_ENDOFLIST) {
                                L0481_ps_Generic = (GENERIC*)F0156_DUNGEON_GetThingData(L0479_T_Thing);
                                L0481_ps_Generic->Next = C0xFFFE_THING_ENDOFLIST;
                                *P0448_pT_GroupSlot = L0479_T_Thing;
                        } else {
                                F0163_DUNGEON_LinkThingToList(L0479_T_Thing, L0478_T_PreviousThing, CM1_MAPX_NOT_ON_A_SQUARE, 0);
                        }
                } else {
                        F0267_MOVE_GetMoveResult_CPSCE(M014_TYPE_AND_INDEX(L0479_T_Thing) | M007_GET(P0447_T_ProjectileThing, MASK0xC000_THING_CELL), CM2_MAPX_PROJECTILE_ASSOCIATED_OBJECT, 0, P0449_i_MapX, P0450_i_MapY);
                }
        }
        L0480_ps_Projectile->Next = C0xFFFF_THING_NONE;
}

STATICFUNCTION unsigned int16_t F0216_PROJECTILE_GetImpactAttack(
PROJECTILE*          P0451_ps_Projectile SEPARATOR
REGISTER THING       P0452_T_Thing       FINAL_SEPARATOR
{
        REGISTER WEAPON_INFO* L0485_ps_WeaponInfo;
        REGISTER unsigned int16_t L0483_ui_Multiple;
#define AL0483_ui_ThingType L0483_ui_Multiple
#define AL0483_ui_Attack    L0483_ui_Multiple
        REGISTER unsigned int16_t L0484_ui_KineticEnergy;


        G0366_i_ProjectilePoisonAttack = 0;
        G0367_i_ProjectileAttackType = C3_ATTACK_BLUNT;
        L0484_ui_KineticEnergy = P0451_ps_Projectile->KineticEnergy;
        if ((AL0483_ui_ThingType = M012_TYPE(P0452_T_Thing)) != C15_THING_TYPE_EXPLOSION) {
                if (AL0483_ui_ThingType == C05_THING_TYPE_WEAPON) {
                        L0485_ps_WeaponInfo = F0158_DUNGEON_GetWeaponInfo(P0452_T_Thing);
                        AL0483_ui_Attack = L0485_ps_WeaponInfo->KineticEnergy;
                        G0367_i_ProjectileAttackType = C3_ATTACK_BLUNT;
                } else {
                        AL0483_ui_Attack = M004_RANDOM(4);
                }
                AL0483_ui_Attack += F0140_DUNGEON_GetObjectWeight(P0452_T_Thing);
        } else {
                if (P0452_T_Thing == C0xFF81_THING_EXPLOSION_SLIME) {
                        AL0483_ui_Attack = M003_RANDOM(16);
                        G0366_i_ProjectilePoisonAttack = AL0483_ui_Attack + 10;
                        AL0483_ui_Attack += M003_RANDOM(32);
                } else {
                        if (P0452_T_Thing >= C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL) {
                                G0367_i_ProjectileAttackType = C5_ATTACK_MAGIC;
                                if (P0452_T_Thing == C0xFF86_THING_EXPLOSION_POISON_BOLT) {
                                        G0366_i_ProjectilePoisonAttack = L0484_ui_KineticEnergy;
                                        return (L0484_ui_KineticEnergy >> 4) + 1;
                                }
                                return 0;
                        }
                                G0367_i_ProjectileAttackType = C1_ATTACK_FIRE;
                                AL0483_ui_Attack = M003_RANDOM(16) + M003_RANDOM(16) + 10;
                                if (P0452_T_Thing == C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT) {
                                        G0367_i_ProjectileAttackType = C7_ATTACK_LIGHTNING;
                                        AL0483_ui_Attack <<= 4;
                                        AL0483_ui_Attack += L0484_ui_KineticEnergy;
                        }
                }
        }
        AL0483_ui_Attack = ((AL0483_ui_Attack + L0484_ui_KineticEnergy) >> 4) + 1;
        AL0483_ui_Attack += M002_RANDOM((AL0483_ui_Attack >> 1) + 1) + M004_RANDOM(4);
        AL0483_ui_Attack = F0025_MAIN_GetMaximumValue(AL0483_ui_Attack >> 1, AL0483_ui_Attack - (32 - (P0451_ps_Projectile->Attack >> 3)));
        return AL0483_ui_Attack;
}

BOOLEAN F0217_PROJECTILE_HasImpactOccured(
int16_t          P0453_i_ImpactType      SEPARATOR
REGISTER int16_t P0454_i_MapXCombo       SEPARATOR
REGISTER int16_t P0455_i_MapYCombo       SEPARATOR
int16_t          P0456_i_Cell            SEPARATOR
int16_t          P0457_i_ProjectileThing FINAL_SEPARATOR
#define AP0454_i_ProjectileTargetMapX P0454_i_MapXCombo
#define AP0455_i_ProjectileTargetMapY P0455_i_MapYCombo
#define AP0456_i_ChampionIndex P0456_i_Cell
{
        REGISTER GROUP* L0491_ps_Group;
        REGISTER PROJECTILE* L0490_ps_Projectile;
        REGISTER CREATURE_INFO* L0493_ps_CreatureInfo;
        THING* L0497_pT_GroupSlot;
        REGISTER THING L0486_T_ProjectileAssociatedThing;
        REGISTER int16_t L0487_i_Multiple;
#define AL0487_i_DoorState  L0487_i_Multiple
#define AL0487_i_IconIndex  L0487_i_Multiple
#define AL0487_i_Outcome    L0487_i_Multiple
#define AL0487_i_WeaponType L0487_i_Multiple
        REGISTER unsigned int16_t L0507_ui_Multiple;
#define AL0507_ui_ExplosionAttack L0507_ui_Multiple
#define AL0507_ui_SoundIndex      L0507_ui_Multiple
        REGISTER unsigned int16_t L0488_ui_Attack;
        POTION* L0492_ps_Potion;
        DOOR* L0494_ps_Door;
        WEAPON* L0495_ps_Weapon;
        unsigned int16_t* L0496_pui_CreatureHealth;
        THING L0498_T_ExplosionThing;
        int16_t L0499_i_ProjectileMapX;
        int16_t L0500_i_ProjectileMapY;
        int16_t L0501_i_MapXCombo;
        int16_t L0502_i_MapYCombo;
        BOOLEAN L0505_B_CreateExplosionOnImpact;
        REGISTER unsigned int16_t L0489_ui_ChampionAttack;
        int16_t L0508_i_PotionPower;
        BOOLEAN L0509_B_RemovePotion;
        int16_t L0510_i_ProjectileAssociatedThingType;
        unsigned int16_t L0511_ui_CreatureType;
        unsigned int16_t L0512_ui_CreatureIndex;
        EVENT L0504_s_Unreferenced; /* BUG0_00 Useless code */


        L0490_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(P0457_i_ProjectileThing);
        L0501_i_MapXCombo = P0454_i_MapXCombo;
        L0502_i_MapYCombo = P0455_i_MapYCombo;
        L0509_B_RemovePotion = C0_FALSE;
        G0364_i_CreatureDamageOutcome = C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
        if ((L0510_i_ProjectileAssociatedThingType = M012_TYPE(L0486_T_ProjectileAssociatedThing = L0490_ps_Projectile->Slot)) == C08_THING_TYPE_POTION) {
                L0491_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0486_T_ProjectileAssociatedThing);
                switch (((POTION*)L0491_ps_Group)->Type) {
                        case C03_POTION_VEN_POTION:
                                L0498_T_ExplosionThing = C0xFF87_THING_EXPLOSION_POISON_CLOUD;
                                goto T0217004;
                        case C19_POTION_FUL_BOMB:
                                L0498_T_ExplosionThing = C0xFF80_THING_EXPLOSION_FIREBALL;
                                T0217004:
                                L0509_B_RemovePotion = C1_TRUE;
                                L0508_i_PotionPower = ((POTION*)L0491_ps_Group)->Power;
                                L0492_ps_Potion = (POTION*)L0491_ps_Group;
                                break;
                }
        }
        L0505_B_CreateExplosionOnImpact = (L0510_i_ProjectileAssociatedThingType == C15_THING_TYPE_EXPLOSION) && (L0486_T_ProjectileAssociatedThing != C0xFF81_THING_EXPLOSION_SLIME) && (L0486_T_ProjectileAssociatedThing != C0xFF86_THING_EXPLOSION_POISON_BOLT);
        L0497_pT_GroupSlot = NULL;
        L0489_ui_ChampionAttack = 0;
        if (P0454_i_MapXCombo <= 255) {
                L0499_i_ProjectileMapX = P0454_i_MapXCombo;
                L0500_i_ProjectileMapY = P0455_i_MapYCombo;
        } else {
                L0499_i_ProjectileMapX = (P0454_i_MapXCombo >> 8) - 1;
                L0500_i_ProjectileMapY = (P0455_i_MapYCombo >> 8);
                AP0454_i_ProjectileTargetMapX &= 0x00FF;
                AP0455_i_ProjectileTargetMapY &= 0x00FF;
        }
        switch (P0453_i_ImpactType) {
                case C04_ELEMENT_DOOR:
                        AL0487_i_DoorState = M036_DOOR_STATE(G0271_ppuc_CurrentMapData[AP0454_i_ProjectileTargetMapX][AP0455_i_ProjectileTargetMapY]);
                        L0494_ps_Door = (DOOR*)F0157_DUNGEON_GetSquareFirstThingData(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY);
                        if ((AL0487_i_DoorState != C5_DOOR_STATE_DESTROYED) && (L0486_T_ProjectileAssociatedThing == C0xFF84_THING_EXPLOSION_OPEN_DOOR)) {
                                if (L0494_ps_Door->Button) {
                                        F0268_SENSOR_AddEvent(C10_EVENT_DOOR, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, 0, C02_EFFECT_TOGGLE, G0313_ul_GameTime + 1);
                                }
                                break;
                        }
                        if ((AL0487_i_DoorState == C5_DOOR_STATE_DESTROYED) ||
                            (AL0487_i_DoorState <= C1_DOOR_STATE_CLOSED_ONE_FOURTH) ||
                            (M007_GET(G0275_as_CurrentMapDoorInfo[L0494_ps_Door->Type].Attributes, MASK0x0002_PROJECTILES_CAN_PASS_THROUGH) &&
                             ((L0510_i_ProjectileAssociatedThingType == C15_THING_TYPE_EXPLOSION) ?
                                (L0486_T_ProjectileAssociatedThing >= C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL) :
                                ((L0490_ps_Projectile->Attack > M003_RANDOM(128)) &&
                                 M007_GET(G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(L0486_T_ProjectileAssociatedThing)].AllowedSlots, MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS)
                                 && ((L0510_i_ProjectileAssociatedThingType != C10_THING_TYPE_JUNK) ||
                                  ((AL0487_i_IconIndex = F0033_OBJECT_GetIconIndex(L0486_T_ProjectileAssociatedThing)) < 0) ||
                                  (!((AL0487_i_IconIndex >= C176_ICON_JUNK_IRON_KEY) && (AL0487_i_IconIndex <= C191_ICON_JUNK_MASTER_KEY))))
                                 )))) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                return C0_FALSE;
                        }
                        L0488_ui_Attack = F0216_PROJECTILE_GetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) + 1;
                        F0232_GROUP_IsDoorDestroyedByAttack(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_ui_Attack + M002_RANDOM(L0488_ui_Attack), C0_FALSE, 0);
                        break;
                case CM2_ELEMENT_CHAMPION:
                        if ((AP0456_i_ChampionIndex = F0285_CHAMPION_GetIndexInCell(P0456_i_Cell)) < 0) {
                                return C0_FALSE;
                        }
                        L0489_ui_ChampionAttack = L0488_ui_Attack = F0216_PROJECTILE_GetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing);
                        break;
                case CM1_ELEMENT_CREATURE:
                        L0491_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(F0175_GROUP_GetThing(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY));
                        if (!(L0512_ui_CreatureIndex = F0176_GROUP_GetCreatureOrdinalInCell(L0491_ps_Group, P0456_i_Cell))) {
                                return C0_FALSE;
                        }
                        L0512_ui_CreatureIndex--;
                        L0493_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[L0511_ui_CreatureType = L0491_ps_Group->Type];
                        if ((L0486_T_ProjectileAssociatedThing == C0xFF80_THING_EXPLOSION_FIREBALL) && (L0511_ui_CreatureType == C11_CREATURE_BLACK_FLAME)) {
                                L0496_pui_CreatureHealth = &L0491_ps_Group->Health[L0512_ui_CreatureIndex];
                                *L0496_pui_CreatureHealth = F0024_MAIN_GetMinimumValue(1000, *L0496_pui_CreatureHealth + F0216_PROJECTILE_GetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing));
                                goto T0217044;
                        }
                        if (M007_GET(L0493_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL) && (L0486_T_ProjectileAssociatedThing != C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL)) {
                                return C0_FALSE;
                        }
                        if (L0488_ui_Attack = (unsigned int16_t)((long)F0216_PROJECTILE_GetImpactAttack(L0490_ps_Projectile, L0486_T_ProjectileAssociatedThing) << 6) / L0493_ps_CreatureInfo->Defense) {
                                if ((AL0487_i_Outcome = F0190_GROUP_GetDamageCreatureOutcome(L0491_ps_Group, L0512_ui_CreatureIndex, AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, L0488_ui_Attack + F0192_GROUP_GetResistanceAdjustedPoisonAttack(L0511_ui_CreatureType, G0366_i_ProjectilePoisonAttack), C1_TRUE)) != C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP) {
                                        F0209_GROUP_ProcessEvents29to41(AP0454_i_ProjectileTargetMapX, AP0455_i_ProjectileTargetMapY, CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE, 0);
                                }
                                G0364_i_CreatureDamageOutcome = AL0487_i_Outcome;
                                if (!L0505_B_CreateExplosionOnImpact &&
                                    (AL0487_i_Outcome == C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP) &&
                                    (L0510_i_ProjectileAssociatedThingType == C05_THING_TYPE_WEAPON) &&
                                    M007_GET(L0493_ps_CreatureInfo->Attributes, MASK0x0400_KEEP_THROWN_SHARP_WEAPONS)) {
                                        L0495_ps_Weapon = (WEAPON*)F0156_DUNGEON_GetThingData(L0486_T_ProjectileAssociatedThing);
                                        AL0487_i_WeaponType = L0495_ps_Weapon->Type;
                                        if ((AL0487_i_WeaponType == C08_WEAPON_DAGGER) || (AL0487_i_WeaponType == C27_WEAPON_ARROW) || (AL0487_i_WeaponType == C28_WEAPON_SLAYER) || (AL0487_i_WeaponType == C31_WEAPON_POISON_DART) || (AL0487_i_WeaponType == C32_WEAPON_THROWING_STAR)) {
                                                L0497_pT_GroupSlot = &L0491_ps_Group->Slot;
                                        }
                                }
                        }
        }
        if (L0489_ui_ChampionAttack && F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(AP0456_i_ChampionIndex, L0488_ui_Attack, MASK0x0004_WOUND_HEAD | MASK0x0008_WOUND_TORSO, G0367_i_ProjectileAttackType) && G0366_i_ProjectilePoisonAttack && M005_RANDOM(2)) {
                F0322_CHAMPION_Poison(AP0456_i_ChampionIndex, G0366_i_ProjectilePoisonAttack);
        }
        if (L0505_B_CreateExplosionOnImpact || L0509_B_RemovePotion
            || (L0486_T_ProjectileAssociatedThing == C0xFF86_THING_EXPLOSION_POISON_BOLT)
           ) {
                if (L0509_B_RemovePotion) {
                        L0486_T_ProjectileAssociatedThing = L0498_T_ExplosionThing;
                        AL0507_ui_ExplosionAttack = L0508_i_PotionPower;
                } else {
                        AL0507_ui_ExplosionAttack = L0490_ps_Projectile->KineticEnergy;
                        if (L0486_T_ProjectileAssociatedThing == C0xFF86_THING_EXPLOSION_POISON_BOLT) {
                                if (!(AL0507_ui_ExplosionAttack >>= 2))
                                        goto T0217044;
                        } else {
                                if ((L0486_T_ProjectileAssociatedThing == C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT) && !(AL0507_ui_ExplosionAttack >>= 1))
                                        goto T0217044;
                        }
                }
                F0213_EXPLOSION_Create(L0486_T_ProjectileAssociatedThing, AL0507_ui_ExplosionAttack, L0501_i_MapXCombo, L0502_i_MapYCombo, (L0486_T_ProjectileAssociatedThing == C0xFF87_THING_EXPLOSION_POISON_CLOUD) ? C0xFF_SINGLE_CENTERED_CREATURE : P0456_i_Cell);
        } else {
                if (M012_TYPE(L0486_T_ProjectileAssociatedThing) == C05_THING_TYPE_WEAPON) {
                        AL0507_ui_SoundIndex = C00_SOUND_METALLIC_THUD;
                } else {
                                AL0507_ui_SoundIndex = C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM;
                }
                F0064_SOUND_RequestPlay_CPSD(AL0507_ui_SoundIndex, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY, C01_MODE_PLAY_IF_PRIORITIZED);
        }
        T0217044:
        if (L0509_B_RemovePotion) {
                L0492_ps_Potion->Next = C0xFFFF_THING_NONE;
                L0490_ps_Projectile->Slot = L0498_T_ExplosionThing;
        }
        F0164_DUNGEON_UnlinkThingFromList(P0457_i_ProjectileThing, 0, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
        F0215_PROJECTILE_Delete(P0457_i_ProjectileThing, L0497_pT_GroupSlot, L0499_i_ProjectileMapX, L0500_i_ProjectileMapY);
        return C1_TRUE;
}

int16_t F0218_PROJECTILE_GetImpactCount(
int16_t          P0458_i_ImpactType SEPARATOR
int16_t          P0459_i_MapX       SEPARATOR
int16_t          P0460_i_MapY       SEPARATOR
REGISTER int16_t P0461_i_Cell       FINAL_SEPARATOR
{
        REGISTER THING L0513_T_Thing;
        REGISTER int16_t L0514_i_ImpactCount;


        L0514_i_ImpactCount = 0;
        G0364_i_CreatureDamageOutcome = C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
        T0218001:
        L0513_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0459_i_MapX, P0460_i_MapY);
        while (L0513_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if ((M012_TYPE(L0513_T_Thing) == C14_THING_TYPE_PROJECTILE) &&
                    (M011_CELL(L0513_T_Thing) == P0461_i_Cell) &&
                    F0217_PROJECTILE_HasImpactOccured(P0458_i_ImpactType, P0459_i_MapX, P0460_i_MapY, P0461_i_Cell, L0513_T_Thing)) {
                        F0214_PROJECTILE_DeleteEvent(L0513_T_Thing);
                        L0514_i_ImpactCount++;
                        if ((P0458_i_ImpactType == CM1_ELEMENT_CREATURE) && (G0364_i_CreatureDamageOutcome == C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP))
                                break;
                        goto T0218001;
                }
                L0513_T_Thing = F0159_DUNGEON_GetNextThing(L0513_T_Thing);
        }
        return L0514_i_ImpactCount;
}

void F0219_PROJECTILE_ProcessEvents48To49_Projectile(
EVENT* P0462_ps_Event FINAL_SEPARATOR
{
        REGISTER EVENT* L0519_ps_Event;
        REGISTER PROJECTILE* L0520_ps_Projectile;
        REGISTER THING L0515_T_ProjectileThingNewCell;
        REGISTER int16_t L0523_i_DestinationMapX;
        REGISTER int16_t L0524_i_DestinationMapY;
        REGISTER int16_t L0518_i_Cell;
        REGISTER unsigned int16_t L0516_ui_Multiple;
#define AL0516_ui_StepEnergy L0516_ui_Multiple
#define AL0516_ui_Square     L0516_ui_Multiple
        THING L0521_T_ProjectileThing;
        REGISTER unsigned int16_t L0517_ui_ProjectileDirection;
        BOOLEAN L0522_B_ProjectileMovesToOtherSquare;
        int16_t L0525_i_SourceMapX;
        int16_t L0526_i_SourceMapY;
        EVENT L0527_s_Event;


        L0527_s_Event = *P0462_ps_Event;
        L0519_ps_Event = &L0527_s_Event;
        L0520_ps_Projectile = (PROJECTILE*)F0156_DUNGEON_GetThingData(L0521_T_ProjectileThing = L0515_T_ProjectileThingNewCell = L0519_ps_Event->B.Slot);
        L0523_i_DestinationMapX = L0519_ps_Event->C.Projectile.MapX;
        L0524_i_DestinationMapY = L0519_ps_Event->C.Projectile.MapY;
        if (L0519_ps_Event->A.A.Type == C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS) {
                L0519_ps_Event->A.A.Type = C49_EVENT_MOVE_PROJECTILE;
        } else {
                L0518_i_Cell = M011_CELL(L0515_T_ProjectileThingNewCell);
                if ((G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0523_i_DestinationMapX == G0306_i_PartyMapX) && (L0524_i_DestinationMapY == G0307_i_PartyMapY) && F0217_PROJECTILE_HasImpactOccured(CM2_ELEMENT_CHAMPION, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_i_Cell, L0515_T_ProjectileThingNewCell)) {
                        return;
                }
                if ((F0175_GROUP_GetThing(L0523_i_DestinationMapX, L0524_i_DestinationMapY) != C0xFFFE_THING_ENDOFLIST) && F0217_PROJECTILE_HasImpactOccured(CM1_ELEMENT_CREATURE, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_i_Cell, L0521_T_ProjectileThing)) {
                        return;
                }
                if (L0520_ps_Projectile->KineticEnergy <= (AL0516_ui_StepEnergy = L0519_ps_Event->C.Projectile.StepEnergy)) {
                        F0164_DUNGEON_UnlinkThingFromList(L0515_T_ProjectileThingNewCell = L0521_T_ProjectileThing, 0, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
                        F0215_PROJECTILE_Delete(L0515_T_ProjectileThingNewCell, NULL, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
                        return;
                }
                else {
                        L0520_ps_Projectile->KineticEnergy -= AL0516_ui_StepEnergy;
                }
                if (L0520_ps_Projectile->Attack < AL0516_ui_StepEnergy) {
                        L0520_ps_Projectile->Attack = 0;
                } else {
                        L0520_ps_Projectile->Attack -= AL0516_ui_StepEnergy;
                }
        }
        if (L0522_B_ProjectileMovesToOtherSquare = ((L0517_ui_ProjectileDirection = L0519_ps_Event->C.Projectile.Direction) == (L0518_i_Cell = M011_CELL(L0515_T_ProjectileThingNewCell = L0519_ps_Event->B.Slot))) || (M017_NEXT(L0517_ui_ProjectileDirection) == L0518_i_Cell)) {
                        L0525_i_SourceMapX = L0523_i_DestinationMapX;
                        L0526_i_SourceMapY = L0524_i_DestinationMapY;
                        L0523_i_DestinationMapX += G0233_ai_Graphic559_DirectionToStepEastCount[L0517_ui_ProjectileDirection], L0524_i_DestinationMapY += G0234_ai_Graphic559_DirectionToStepNorthCount[L0517_ui_ProjectileDirection];
                        if ((M034_SQUARE_TYPE(AL0516_ui_Square = F0151_DUNGEON_GetSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY)) == C00_ELEMENT_WALL) ||
                            ((M034_SQUARE_TYPE(AL0516_ui_Square) == C06_ELEMENT_FAKEWALL) && !M007_GET(AL0516_ui_Square, (MASK0x0001_FAKEWALL_IMAGINARY | MASK0x0004_FAKEWALL_OPEN))) ||
                            ((M034_SQUARE_TYPE(AL0516_ui_Square) == C03_ELEMENT_STAIRS)  && (M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[L0525_i_SourceMapX][L0526_i_SourceMapY]) == C03_ELEMENT_STAIRS))) {
                                if (F0217_PROJECTILE_HasImpactOccured(M034_SQUARE_TYPE(AL0516_ui_Square), L0525_i_SourceMapX, L0526_i_SourceMapY, L0518_i_Cell, L0515_T_ProjectileThingNewCell)) {
                                        return;
                                }
                        }
        }
        if ((L0517_ui_ProjectileDirection & 0x0001) == (L0518_i_Cell & 0x0001)) {
                L0518_i_Cell--;
        } else {
                L0518_i_Cell++;
        }
        L0515_T_ProjectileThingNewCell = M015_THING_WITH_NEW_CELL(L0515_T_ProjectileThingNewCell, L0518_i_Cell &= 0x0003);
        if (L0522_B_ProjectileMovesToOtherSquare) {
                F0267_MOVE_GetMoveResult_CPSCE(L0515_T_ProjectileThingNewCell, L0525_i_SourceMapX, L0526_i_SourceMapY, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
                L0519_ps_Event->C.Projectile.MapX = G0397_i_MoveResultMapX;
                L0519_ps_Event->C.Projectile.MapY = G0398_i_MoveResultMapY;
                L0519_ps_Event->C.Projectile.Direction = G0400_i_MoveResultDirection;
                L0515_T_ProjectileThingNewCell = M015_THING_WITH_NEW_CELL(L0515_T_ProjectileThingNewCell, G0401_ui_MoveResultCell);
                M031_SET_MAP(L0519_ps_Event->Map_Time, G0399_ui_MoveResultMapIndex);
        } else {
                if ((M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(L0523_i_DestinationMapX, L0524_i_DestinationMapY)) == C04_ELEMENT_DOOR) && F0217_PROJECTILE_HasImpactOccured(C04_ELEMENT_DOOR, L0523_i_DestinationMapX, L0524_i_DestinationMapY, L0518_i_Cell, L0521_T_ProjectileThing)) {
                        return;
                }
                else {
                        F0164_DUNGEON_UnlinkThingFromList(L0515_T_ProjectileThingNewCell, 0, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
                        F0163_DUNGEON_LinkThingToList(L0515_T_ProjectileThingNewCell, 0, L0523_i_DestinationMapX, L0524_i_DestinationMapY);
                }
        }
        L0519_ps_Event->Map_Time += (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) ? 1 : 3;
        L0519_ps_Event->B.Slot = L0515_T_ProjectileThingNewCell;
        L0520_ps_Projectile->EventIndex = F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(L0519_ps_Event);
}

void F0220_EXPLOSION_ProcessEvent25_Explosion(
REGISTER EVENT* P0463_ps_Event FINAL_SEPARATOR
{
        REGISTER EXPLOSION* L0532_ps_Explosion;
        REGISTER GROUP* L0533_ps_Group;
        CREATURE_INFO* L0534_ps_CreatureInfo;
        REGISTER int16_t L0530_i_Attack;
        REGISTER unsigned int16_t L0528_ui_MapX;
        REGISTER unsigned int16_t L0529_ui_MapY;
        REGISTER unsigned int16_t L0537_ui_Multiple;
#define AL0537_ui_CreatureType                L0537_ui_Multiple
#define AL0537_ui_NonMaterialAdditionalAttack L0537_ui_Multiple
        REGISTER int16_t L0531_i_Multiple;
#define AL0531_i_SquareType    L0531_i_Multiple
#define AL0531_i_CreatureCount L0531_i_Multiple
        THING L0535_T_GroupThing;
        THING L0536_T_ExplosionThing;
        BOOLEAN L0538_B_ExplosionOnPartySquare;
        EVENT L0539_s_Event;


        L0528_ui_MapX = P0463_ps_Event->B.Location.MapX;
        L0529_ui_MapY = P0463_ps_Event->B.Location.MapY;
        L0532_ps_Explosion = &((EXPLOSION*)G0284_apuc_ThingData[C15_THING_TYPE_EXPLOSION])[M013_INDEX(P0463_ps_Event->C.Slot)];
        AL0531_i_SquareType = M034_SQUARE_TYPE(G0271_ppuc_CurrentMapData[L0528_ui_MapX][L0529_ui_MapY]);
        L0538_B_ExplosionOnPartySquare = (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex) && (L0528_ui_MapX == G0306_i_PartyMapX) && (L0529_ui_MapY == G0307_i_PartyMapY);
        if ((L0535_T_GroupThing = F0175_GROUP_GetThing(L0528_ui_MapX, L0529_ui_MapY)) != C0xFFFE_THING_ENDOFLIST) {
                L0533_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0535_T_GroupThing);
                L0534_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[AL0537_ui_CreatureType = L0533_ps_Group->Type];
        }
        if ((L0536_T_ExplosionThing = C0xFF80_THING_FIRST_EXPLOSION + L0532_ps_Explosion->Type) == C0xFF87_THING_EXPLOSION_POISON_CLOUD) {
                L0530_i_Attack = F0025_MAIN_GetMaximumValue(1, F0024_MAIN_GetMinimumValue(L0532_ps_Explosion->Attack >> 5, 4) + M005_RANDOM(2)); /* Value between 1 and 5 */
        } else {
                L0530_i_Attack = (L0532_ps_Explosion->Attack >> 1) + 1;
                L0530_i_Attack += M002_RANDOM(L0530_i_Attack) + 1;
        }
        switch (L0536_T_ExplosionThing) {
                case C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT:
                        if (!(L0530_i_Attack >>= 1))
                                break;
                case C0xFF80_THING_EXPLOSION_FIREBALL:
                        if (AL0531_i_SquareType == C04_ELEMENT_DOOR) {
                                F0232_GROUP_IsDoorDestroyedByAttack(L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, C1_TRUE, 0);
                        }
                        break;
                case C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL:
                        if ((L0535_T_GroupThing != C0xFFFE_THING_ENDOFLIST) && M007_GET(L0534_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL)) {
                                if ((AL0537_ui_CreatureType == C19_CREATURE_MATERIALIZER_ZYTAZ) && (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex)) { /* ANOMALY_ASSEMBLY_COMPILATION_DIFFERENCE jmp */
                                        L0530_i_Attack -= (AL0537_ui_NonMaterialAdditionalAttack = L0530_i_Attack >> 3);
                                        AL0537_ui_NonMaterialAdditionalAttack <<= 1;
                                        AL0537_ui_NonMaterialAdditionalAttack++;
                                        AL0531_i_CreatureCount = L0533_ps_Group->Count;
                                        do {
                                                if (M007_GET(G0375_ps_ActiveGroups[L0533_ps_Group->ActiveGroupIndex].Aspect[AL0531_i_CreatureCount], MASK0x0080_IS_ATTACKING)) { /* Materializer / Zytaz can only be damaged while they are attacking */
                                                        F0190_GROUP_GetDamageCreatureOutcome(L0533_ps_Group, AL0531_i_CreatureCount, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack + M002_RANDOM(AL0537_ui_NonMaterialAdditionalAttack) + M004_RANDOM(4), C1_TRUE);
                                                }
                                        } while (--AL0531_i_CreatureCount >= 0);
                                } else {
                                        F0191_GROUP_GetDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, C1_TRUE);
                                }
                        }
                        break;
                case C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1:
                        L0532_ps_Explosion->Type++;
                        F0064_SOUND_RequestPlay_CPSD(C05_SOUND_STRONG_EXPLOSION, L0528_ui_MapX, L0529_ui_MapY, C01_MODE_PLAY_IF_PRIORITIZED);
                        goto T0220026;
                case C0xFFA8_THING_EXPLOSION_SMOKE:
                        if (L0532_ps_Explosion->Attack > 55) {
                                L0532_ps_Explosion->Attack -= 40;
                                goto T0220026;
                        }
                        break;
                case C0xFF87_THING_EXPLOSION_POISON_CLOUD:
                        if (L0538_B_ExplosionOnPartySquare) {
                                F0324_CHAMPION_DamageAll_GetDamagedChampionCount(L0530_i_Attack, MASK0x0000_WOUND_NONE, C0_ATTACK_NORMAL);
                        } else {
                                if ((L0535_T_GroupThing != C0xFFFE_THING_ENDOFLIST) && (L0530_i_Attack = F0192_GROUP_GetResistanceAdjustedPoisonAttack(AL0537_ui_CreatureType, L0530_i_Attack)) && (F0191_GROUP_GetDamageAllCreaturesOutcome(L0533_ps_Group, L0528_ui_MapX, L0529_ui_MapY, L0530_i_Attack, C1_TRUE) != C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP) && (L0530_i_Attack > 2)) {
                                        F0209_GROUP_ProcessEvents29to41(L0528_ui_MapX, L0529_ui_MapY, CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE, 0);
                                }
                        }
                        if (L0532_ps_Explosion->Attack >= 6) {
                                L0532_ps_Explosion->Attack -= 3;
                                T0220026:
                                L0539_s_Event = *P0463_ps_Event;
                                L0539_s_Event.Map_Time++;
                                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0539_s_Event);
                                return;
                        }
        }
        F0164_DUNGEON_UnlinkThingFromList(P0463_ps_Event->C.Slot, 0, L0528_ui_MapX, L0529_ui_MapY);
        L0532_ps_Explosion->Next = C0xFFFF_THING_NONE;
}

STATICFUNCTION BOOLEAN F0221_GROUP_IsFluxcageOnSquare(
int16_t P0464_i_MapX SEPARATOR
int16_t P0465_i_MapY FINAL_SEPARATOR
{
        REGISTER THING L0540_T_Thing;
        REGISTER int16_t L0541_i_SquareType;


        if (((L0541_i_SquareType = M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(P0464_i_MapX, P0465_i_MapY))) == C00_ELEMENT_WALL) || (L0541_i_SquareType == C03_ELEMENT_STAIRS)) {
                return C0_FALSE;
        }
        L0540_T_Thing = F0161_DUNGEON_GetSquareFirstThing(P0464_i_MapX, P0465_i_MapY);
        while (L0540_T_Thing != C0xFFFE_THING_ENDOFLIST) {
                if ((M012_TYPE(L0540_T_Thing) == C15_THING_TYPE_EXPLOSION) && (((EXPLOSION*)G0284_apuc_ThingData[C15_THING_TYPE_EXPLOSION])[M013_INDEX(L0540_T_Thing)].Type == C050_EXPLOSION_FLUXCAGE)) {
                        return C1_TRUE;
                }
                L0540_T_Thing = F0159_DUNGEON_GetNextThing(L0540_T_Thing);
        }
        return C0_FALSE;
}

STATICFUNCTION unsigned int16_t F0222_GROUP_IsLordChaosOnSquare(
int16_t P0466_i_MapX SEPARATOR
int16_t P0467_i_MapY FINAL_SEPARATOR
{
        REGISTER GROUP* L0543_ps_Group;
        REGISTER THING L0542_T_Thing;


        if ((L0542_T_Thing = F0175_GROUP_GetThing(P0466_i_MapX, P0467_i_MapY)) == C0xFFFE_THING_ENDOFLIST) {
                return 0;
        }
        L0543_ps_Group = (GROUP*)F0156_DUNGEON_GetThingData(L0542_T_Thing);
        if (L0543_ps_Group->Type == C23_CREATURE_LORD_CHAOS) {
                return L0542_T_Thing;
        }
        else {
                return 0;
        }
}


void F0224_GROUP_FluxCageAction(
REGISTER int16_t P0470_i_MapX SEPARATOR
REGISTER int16_t P0471_i_MapY FINAL_SEPARATOR
{
        REGISTER THING L0545_T_Thing;
        REGISTER int16_t L0546_i_Multiple;
#define AL0546_i_SquareType    L0546_i_Multiple
#define AL0546_i_FluxcageCount L0546_i_Multiple
        EVENT L0547_s_Event;


        if (((AL0546_i_SquareType = M034_SQUARE_TYPE(F0151_DUNGEON_GetSquare(P0470_i_MapX, P0471_i_MapY))) == C00_ELEMENT_WALL) || (AL0546_i_SquareType == C03_ELEMENT_STAIRS)) {
                return;
        }
        if ((L0545_T_Thing = F0166_DUNGEON_GetUnusedThing(C15_THING_TYPE_EXPLOSION)) == C0xFFFF_THING_NONE) {
                return;
        }
        F0163_DUNGEON_LinkThingToList(L0545_T_Thing, 0, P0470_i_MapX, P0471_i_MapY);
        (((EXPLOSION*)G0284_apuc_ThingData[C15_THING_TYPE_EXPLOSION])[M013_INDEX(L0545_T_Thing)]).Type = C050_EXPLOSION_FLUXCAGE;
        M033_SET_MAP_AND_TIME(L0547_s_Event.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime + 100);
        L0547_s_Event.A.A.Type = C24_EVENT_REMOVE_FLUXCAGE;
        L0547_s_Event.A.A.Priority = 0;
        L0547_s_Event.C.Slot = L0545_T_Thing;
        L0547_s_Event.B.Location.MapX = P0470_i_MapX;
        L0547_s_Event.B.Location.MapY = P0471_i_MapY;
        F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0547_s_Event);
        if (F0222_GROUP_IsLordChaosOnSquare(P0470_i_MapX, P0471_i_MapY - 1)) {
                P0471_i_MapY--;
                AL0546_i_FluxcageCount = F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX + 1, P0471_i_MapY);
                goto T0224005;
        }
        else {
                if (F0222_GROUP_IsLordChaosOnSquare(P0470_i_MapX - 1, P0471_i_MapY)) {
                        P0470_i_MapX--;
                        AL0546_i_FluxcageCount = F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX, P0471_i_MapY + 1);
                        T0224005:
                        AL0546_i_FluxcageCount += F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX, P0471_i_MapY - 1) + F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX - 1, P0471_i_MapY);
                } else {
                        if (F0222_GROUP_IsLordChaosOnSquare(P0470_i_MapX + 1, P0471_i_MapY)) {
                                P0470_i_MapX++;
                                AL0546_i_FluxcageCount = F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX, P0471_i_MapY - 1);
                                goto T0224008;
                        }
                        else {
                                if (F0222_GROUP_IsLordChaosOnSquare(P0470_i_MapX, P0471_i_MapY + 1)) {
                                        P0471_i_MapY++;
                                        AL0546_i_FluxcageCount = F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX - 1, P0471_i_MapY);
                                        T0224008:
                                        AL0546_i_FluxcageCount += F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX, P0471_i_MapY + 1) + F0221_GROUP_IsFluxcageOnSquare(P0470_i_MapX + 1, P0471_i_MapY);
                                } else {
                                        AL0546_i_FluxcageCount = 0;
                                }
                        }
                }
        }
        if (AL0546_i_FluxcageCount == 2) {
                F0209_GROUP_ProcessEvents29to41(P0470_i_MapX, P0471_i_MapY, CM3_EVENT_CREATE_REACTION_EVENT_29_DANGER_ON_SQUARE, 0);
        }
}

void F0225_GROUP_FuseAction(
REGISTER unsigned int16_t P0472_ui_MapX SEPARATOR
REGISTER unsigned int16_t P0473_ui_MapY FINAL_SEPARATOR
{
        REGISTER int16_t L0548_i_MapX;
        REGISTER int16_t L0549_i_MapY;
        unsigned int16_t L0552_ui_FluxcageIndex;
        THING L0555_T_LordChaosThing;
        unsigned int16_t L0551_ui_FluxcageCount;
        unsigned int16_t L0553_ui_Counter;
        BOOLEAN L0554_aB_Fluxcages[4];


        if ((P0472_ui_MapX >= 0) && (P0472_ui_MapX < G0273_i_CurrentMapWidth) && (P0473_ui_MapY >= 0) && (P0473_ui_MapY < G0274_i_CurrentMapHeight)) {
                F0213_EXPLOSION_Create(C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL, 255, P0472_ui_MapX, P0473_ui_MapY, C0xFF_SINGLE_CENTERED_CREATURE); /* BUG0_17 The game crashes after the Fuse action is performed while looking at a wall on a map boundary. An explosion thing is created on the square in front of the party but there is no check to ensure the square coordinates are in the map bounds. This corrupts a memory location and leads to a game crash */
                if (L0555_T_LordChaosThing = F0222_GROUP_IsLordChaosOnSquare(P0472_ui_MapX, P0473_ui_MapY)) {
                        L0551_ui_FluxcageCount = (L0554_aB_Fluxcages[0] = F0221_GROUP_IsFluxcageOnSquare(P0472_ui_MapX - 1, P0473_ui_MapY)) +
                                                 (L0554_aB_Fluxcages[1] = F0221_GROUP_IsFluxcageOnSquare(P0472_ui_MapX + 1, P0473_ui_MapY)) +
                                                 (L0554_aB_Fluxcages[2] = F0221_GROUP_IsFluxcageOnSquare(P0472_ui_MapX, P0473_ui_MapY - 1)) +
                                                 (L0554_aB_Fluxcages[3] = F0221_GROUP_IsFluxcageOnSquare(P0472_ui_MapX, P0473_ui_MapY + 1));
                        while (L0551_ui_FluxcageCount++ < 4) {
                                L0548_i_MapX = P0472_ui_MapX;
                                L0549_i_MapY = P0473_ui_MapY;
                                L0552_ui_FluxcageIndex = M004_RANDOM(4);
                                for (L0553_ui_Counter = 5; --L0553_ui_Counter; L0552_ui_FluxcageIndex = M017_NEXT(L0552_ui_FluxcageIndex)) {
                                        if (!L0554_aB_Fluxcages[L0552_ui_FluxcageIndex]) {
                                                L0554_aB_Fluxcages[L0552_ui_FluxcageIndex] = C1_TRUE;
                                                switch (L0552_ui_FluxcageIndex) {
                                                        case 0:
                                                                L0548_i_MapX--;
                                                                break;
                                                        case 1:
                                                                L0548_i_MapX++;
                                                                break;
                                                        case 2:
                                                                L0549_i_MapY--;
                                                                break;
                                                        case 3:
                                                                L0549_i_MapY++;
                                                }
                                                break;
                                        }
                                }
                                if (F0223_GROUP_IsLordChaosAllowed(L0548_i_MapX, L0549_i_MapY)) {
                                        if (!F0267_MOVE_GetMoveResult_CPSCE(L0555_T_LordChaosThing, P0472_ui_MapX, P0473_ui_MapY, L0548_i_MapX, L0549_i_MapY)) {
                                                F0180_GROUP_StartWandering(L0548_i_MapX, L0549_i_MapY);
                                        }
                                        return;
                                }
                        }
                        F0446_STARTEND_FuseSequence();
                }
        }
}

int16_t F0226_GROUP_GetDistanceBetweenSquares(
REGISTER int16_t P0474_i_SourceMapX      SEPARATOR
REGISTER int16_t P0475_i_SourceMapY      SEPARATOR
int16_t          P0476_i_DestinationMapX SEPARATOR
int16_t          P0477_i_DestinationMapY FINAL_SEPARATOR
{
        return ((((P0474_i_SourceMapX -= P0476_i_DestinationMapX) < 0) ? -P0474_i_SourceMapX : P0474_i_SourceMapX) +
                (((P0475_i_SourceMapY -= P0477_i_DestinationMapY) < 0) ? -P0475_i_SourceMapY : P0475_i_SourceMapY));
}

BOOLEAN F0227_GROUP_IsDestinationVisibleFromSource(
unsigned int16_t P0478_ui_Direction      SEPARATOR
REGISTER int16_t P0479_i_SourceMapX      SEPARATOR
REGISTER int16_t P0480_i_SourceMapY      SEPARATOR
REGISTER int16_t P0481_i_DestinationMapX SEPARATOR
REGISTER int16_t P0482_i_DestinationMapY FINAL_SEPARATOR
{
        int16_t L1637_i_Temp;


        switch (P0478_ui_Direction) { /* If direction is not 'West' then swap variables so that the same test as for west can be applied */
                case C2_DIRECTION_SOUTH:
                        L1637_i_Temp = P0479_i_SourceMapX;
                        P0479_i_SourceMapX = P0482_i_DestinationMapY;
                        P0482_i_DestinationMapY = L1637_i_Temp;
                        L1637_i_Temp = P0481_i_DestinationMapX;
                        P0481_i_DestinationMapX = P0480_i_SourceMapY;
                        P0480_i_SourceMapY = L1637_i_Temp;
                        break;
                case C1_DIRECTION_EAST:
                        L1637_i_Temp = P0479_i_SourceMapX;
                        P0479_i_SourceMapX = P0481_i_DestinationMapX;
                        P0481_i_DestinationMapX = L1637_i_Temp;
                        L1637_i_Temp = P0482_i_DestinationMapY;
                        P0482_i_DestinationMapY = P0480_i_SourceMapY;
                        P0480_i_SourceMapY = L1637_i_Temp;
                        break;
                case C0_DIRECTION_NORTH:
                        L1637_i_Temp = P0479_i_SourceMapX;
                        P0479_i_SourceMapX = P0480_i_SourceMapY;
                        P0480_i_SourceMapY = L1637_i_Temp;
                        L1637_i_Temp = P0481_i_DestinationMapX;
                        P0481_i_DestinationMapX = P0482_i_DestinationMapY;
                        P0482_i_DestinationMapY = L1637_i_Temp;
        }
        return ((P0479_i_SourceMapX -= (P0481_i_DestinationMapX - 1)) > 0) && ((((P0480_i_SourceMapY -= P0482_i_DestinationMapY) < 0) ? -P0480_i_SourceMapY : P0480_i_SourceMapY) <= P0479_i_SourceMapX);
}

int16_t F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(
REGISTER int16_t P0483_i_SourceMapX      SEPARATOR
REGISTER int16_t P0484_i_SourceMapY      SEPARATOR
REGISTER int16_t P0485_i_DestinationMapX SEPARATOR
int16_t          P0486_i_DestinationMapY FINAL_SEPARATOR
#define AP0483_i_PrimaryDirection P0483_i_SourceMapX
{
        REGISTER int16_t L0556_i_Direction;


        if (P0483_i_SourceMapX == P0485_i_DestinationMapX) {
                G0363_i_SecondaryDirectionToOrFromParty = (M006_RANDOM(65536) & 0x0002) + 1; /* Resulting direction may be 1 or 3 (East or West) */
                if (P0484_i_SourceMapY > P0486_i_DestinationMapY) {
                        return C0_DIRECTION_NORTH;
                }
                else {
                        return C2_DIRECTION_SOUTH;
                }
        }
        if (P0484_i_SourceMapY == P0486_i_DestinationMapY) {
                G0363_i_SecondaryDirectionToOrFromParty = (M006_RANDOM(65536) & 0x0002) + 0; /* Resulting direction may be 0 or 2 (North or South) */
                if (P0483_i_SourceMapX > P0485_i_DestinationMapX) {
                        return C3_DIRECTION_WEST;
                }
                else {
                        return C1_DIRECTION_EAST;
                }
        }
        L0556_i_Direction = C0_DIRECTION_NORTH;
        for (;;) { /*_Infinite loop_*/
                if (F0227_GROUP_IsDestinationVisibleFromSource(L0556_i_Direction, P0483_i_SourceMapX, P0484_i_SourceMapY, P0485_i_DestinationMapX, P0486_i_DestinationMapY)) {
                        if (!F0227_GROUP_IsDestinationVisibleFromSource(G0363_i_SecondaryDirectionToOrFromParty = M017_NEXT(L0556_i_Direction), P0483_i_SourceMapX, P0484_i_SourceMapY, P0485_i_DestinationMapX, P0486_i_DestinationMapY)) {
                                if ((L0556_i_Direction != C0_DIRECTION_NORTH) || !F0227_GROUP_IsDestinationVisibleFromSource(G0363_i_SecondaryDirectionToOrFromParty = M019_PREVIOUS(L0556_i_Direction), P0483_i_SourceMapX, P0484_i_SourceMapY, P0485_i_DestinationMapX, P0486_i_DestinationMapY)) {
                                        G0363_i_SecondaryDirectionToOrFromParty = M017_NEXT((M006_RANDOM(65536) & 0x0002) + L0556_i_Direction);
                                        return L0556_i_Direction;
                                }
                        }
                        if (M005_RANDOM(2)) {
                                AP0483_i_PrimaryDirection = G0363_i_SecondaryDirectionToOrFromParty;
                                G0363_i_SecondaryDirectionToOrFromParty = L0556_i_Direction;
                                return AP0483_i_PrimaryDirection;
                        }
                        else {
                                return L0556_i_Direction;
                        }
                }
                L0556_i_Direction++;
        }
}

void F0229_GROUP_SetOrderedCellsToAttack(
char*            P0487_pc_OrderedCellsToAttack SEPARATOR
int16_t          P0488_i_TargetMapX            SEPARATOR
int16_t          P0489_i_TargetMapY            SEPARATOR
int16_t          P0490_i_AttackerMapX          SEPARATOR
int16_t          P0491_i_AttackerMapY          SEPARATOR
unsigned int16_t P0492_ui_CellSource           FINAL_SEPARATOR
{
        REGISTER unsigned int16_t L0557_ui_OrderedCellsToAttackIndex;


        if (!((L0557_ui_OrderedCellsToAttackIndex = F0228_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource(P0488_i_TargetMapX, P0489_i_TargetMapY, P0490_i_AttackerMapX, P0491_i_AttackerMapY) << 1) & 0x0002)) {
                P0492_ui_CellSource++;
        }
        L0557_ui_OrderedCellsToAttackIndex += (P0492_ui_CellSource >> 1) & 0x0001;
        F0007_MAIN_CopyBytes(G0023_aac_Graphic562_OrderedCellsToAttack[L0557_ui_OrderedCellsToAttackIndex], P0487_pc_OrderedCellsToAttack, M543_BYTE_COUNT_INT(4));
}

int16_t F0230_GROUP_GetChampionDamage(
REGISTER GROUP*           P0493_ps_Group         SEPARATOR
unsigned int16_t          P0494_ui_ChampionIndex FINAL_SEPARATOR
{
        REGISTER CHAMPION* L0562_ps_Champion;
        REGISTER int16_t L0558_i_Multiple;
#define AL0558_i_Attack L0558_i_Multiple
#define AL0558_i_Damage L0558_i_Multiple
        REGISTER unsigned int16_t L0559_ui_Multiple;
#define AL0559_ui_WoundTest          L0559_ui_Multiple
#define AL0559_ui_PoisonAttack       L0559_ui_Multiple
#define AL0559_ui_CreatureDifficulty L0559_ui_Multiple
        REGISTER unsigned int16_t L0561_ui_Multiple;
#define AL0561_ui_WoundProbabilityIndex L0561_ui_Multiple
#define AL0561_ui_AllowedWound          L0561_ui_Multiple
        REGISTER unsigned int16_t L0560_ui_WoundProbabilities;
        int16_t L0563_i_DoubledMapDifficulty;
        CREATURE_INFO L0564_s_CreatureInfo;


        L0562_ps_Champion = &M516_CHAMPIONS[P0494_ui_ChampionIndex];
        if (P0494_ui_ChampionIndex >= G0305_ui_PartyChampionCount) {
                return 0;
        }
        if (!L0562_ps_Champion->CurrentHealth) {
                return 0;
        }
        if (G0300_B_PartyIsResting) {
                F0314_CHAMPION_WakeUp();
        }
        L0563_i_DoubledMapDifficulty = G0269_ps_CurrentMap->C.Difficulty << 1;
        L0564_s_CreatureInfo = G0243_as_Graphic559_CreatureInfo[P0493_ps_Group->Type];
        F0304_CHAMPION_AddSkillExperience(P0494_ui_ChampionIndex, C07_SKILL_PARRY, M058_EXPERIENCE(L0564_s_CreatureInfo.Properties));
        if (G0407_s_Party.Event71Count_Invisibility && !M007_GET(L0564_s_CreatureInfo.Attributes, MASK0x0800_SEE_INVISIBLE)) {
                AL0559_ui_CreatureDifficulty = 16;
        } else {
                if (M007_GET(L0564_s_CreatureInfo.Attributes, MASK0x1000_NIGHT_VISION)) {
                        AL0559_ui_CreatureDifficulty = 0;
                } else {
                        AL0559_ui_CreatureDifficulty = G0304_i_DungeonViewPaletteIndex << 1;
                }
        }
        if (G0300_B_PartyIsResting || (L0564_s_CreatureInfo.Dexterity == 255) || (((F0311_CHAMPION_GetDexterity(L0562_ps_Champion) < (M003_RANDOM(32) + L0564_s_CreatureInfo.Dexterity + L0563_i_DoubledMapDifficulty + AL0559_ui_CreatureDifficulty - 16)) || !M004_RANDOM(4)) && !F0308_CHAMPION_IsLucky(L0562_ps_Champion, 60))) {
                if ((AL0559_ui_WoundTest = M006_RANDOM(65536)) & 0x0070) {
                        AL0559_ui_WoundTest &= 0x000F;
                        L0560_ui_WoundProbabilities = L0564_s_CreatureInfo.WoundProbabilities;
                        for (AL0561_ui_WoundProbabilityIndex = 0; AL0559_ui_WoundTest > (L0560_ui_WoundProbabilities & 0x000F); L0560_ui_WoundProbabilities >>= 4) {
                                AL0561_ui_WoundProbabilityIndex++;
                        }
                        AL0561_ui_AllowedWound = G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask[AL0561_ui_WoundProbabilityIndex];
                } else {
                        AL0561_ui_AllowedWound = M007_GET(AL0559_ui_WoundTest, MASK0x0001_WOUND_READY_HAND);
                }
                if ((AL0558_i_Attack = (M003_RANDOM(16) + L0564_s_CreatureInfo.Attack + L0563_i_DoubledMapDifficulty) - (F0303_CHAMPION_GetSkillLevel(P0494_ui_ChampionIndex, C07_SKILL_PARRY) << 1)) <= 1) {
                        if (M005_RANDOM(2)) {
                                goto T0230014;
                        }
                        AL0558_i_Attack = M004_RANDOM(4) + 2;
                }
                AL0558_i_Attack >>= 1;
                AL0558_i_Attack += M002_RANDOM(AL0558_i_Attack) + M004_RANDOM(4);
                AL0558_i_Attack += M002_RANDOM(AL0558_i_Attack);
                AL0558_i_Attack >>= 2;
                AL0558_i_Attack += M004_RANDOM(4) + 1;
                if (M005_RANDOM(2)) {
                        AL0558_i_Attack -= M002_RANDOM((AL0558_i_Attack >> 1) + 1) - 1;
                }
                if (AL0558_i_Damage = F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(P0494_ui_ChampionIndex, AL0558_i_Attack, AL0561_ui_AllowedWound, L0564_s_CreatureInfo.AttackType)) {
                        F0064_SOUND_RequestPlay_CPSD(C09_SOUND_CHAMPION_0_DAMAGED + P0494_ui_ChampionIndex, G0306_i_PartyMapX, G0307_i_PartyMapY, C02_MODE_PLAY_ONE_TICK_LATER);
                        if ((AL0559_ui_PoisonAttack = L0564_s_CreatureInfo.PoisonAttack) && M005_RANDOM(2) && ((AL0559_ui_PoisonAttack = F0307_CHAMPION_GetStatisticAdjustedAttack(L0562_ps_Champion, C4_STATISTIC_VITALITY, AL0559_ui_PoisonAttack)) >= 0)) {
                                F0322_CHAMPION_Poison(P0494_ui_ChampionIndex, AL0559_ui_PoisonAttack);
                        }
                        return AL0558_i_Damage;
                }
        }
        T0230014:
        return 0;
}

int16_t F0231_GROUP_GetMeleeActionDamage(
REGISTER CHAMPION* P0495_ps_Champion             SEPARATOR
unsigned int16_t   P0496_i_ChampionIndex         SEPARATOR
GROUP*             P0497_ps_Group                SEPARATOR
int16_t            P0498_i_CreatureIndex         SEPARATOR
int16_t            P0499_i_MapX                  SEPARATOR
int16_t            P0500_i_MapY                  SEPARATOR
unsigned int16_t   P0501_ui_ActionHitProbability SEPARATOR
unsigned int16_t   P0502_ui_ActionDamageFactor   SEPARATOR
int16_t            P0503_i_SkillIndex            FINAL_SEPARATOR
{
        REGISTER CREATURE_INFO* L0572_ps_CreatureInfo;
        REGISTER int16_t L0568_i_Defense;
        REGISTER int16_t L0571_i_ActionHandObjectIconIndex;
        REGISTER int16_t L0565_i_Damage;
        REGISTER int16_t L0566_i_Damage;
        REGISTER int16_t L0567_i_DoubledMapDifficulty;
        BOOLEAN L0570_B_ActionHitsNonMaterialCreatures;
        int16_t L0569_i_Outcome;


        if (P0496_i_ChampionIndex >= G0305_ui_PartyChampionCount) {
                return 0;
        }
        if (!P0495_ps_Champion->CurrentHealth) {
                return 0;
        }
        L0567_i_DoubledMapDifficulty = G0269_ps_CurrentMap->C.Difficulty << 1;
        L0572_ps_CreatureInfo = &G0243_as_Graphic559_CreatureInfo[P0497_ps_Group->Type];
        if (L0572_ps_CreatureInfo->Dexterity != 255) {
                L0571_i_ActionHandObjectIconIndex = F0033_OBJECT_GetIconIndex(P0495_ps_Champion->Slots[C01_SLOT_ACTION_HAND]);
                if (L0570_B_ActionHitsNonMaterialCreatures = M007_GET(P0501_ui_ActionHitProbability, MASK0x8000_HIT_NON_MATERIAL_CREATURES)) {
                        M009_CLEAR(P0501_ui_ActionHitProbability, MASK0x8000_HIT_NON_MATERIAL_CREATURES);
                }
                if ((!M007_GET(L0572_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL) || L0570_B_ActionHitsNonMaterialCreatures) &&
                    ((F0311_CHAMPION_GetDexterity(P0495_ps_Champion) > (M003_RANDOM(32) + L0572_ps_CreatureInfo->Dexterity + L0567_i_DoubledMapDifficulty + (G0304_i_DungeonViewPaletteIndex << 1) + -16)) ||
                     (!M004_RANDOM(4)) ||
                     (F0308_CHAMPION_IsLucky(P0495_ps_Champion, 75 - P0501_ui_ActionHitProbability)))) {
                        if (!(L0565_i_Damage = F0312_CHAMPION_GetStrength(P0496_i_ChampionIndex, C01_SLOT_ACTION_HAND))) {
                                goto T0231009; /* BUG0_81: A weak champion may inflict high damage. If a champion's strength (computed value between 0 and 100) is 0 then a modifier may be added to the damage value without being initialized first and may thus contain a large value. This may happen for a champion with a low strength statistic (like Tiggy), a wounded action hand holding a heavy weapon (like a Stone Club) and a very low stamina. If F0312_CHAMPION_GetStrength returns 0 then L0566_i_Damage may be used without being initialized */
                        }
                        L0565_i_Damage += M002_RANDOM((L0565_i_Damage >> 1) + 1);
                        L0565_i_Damage = ((long)L0565_i_Damage * (long)P0502_ui_ActionDamageFactor) >> 5;
                        L0568_i_Defense = M003_RANDOM(32) + L0572_ps_CreatureInfo->Defense + L0567_i_DoubledMapDifficulty;
                        if (L0571_i_ActionHandObjectIconIndex == C039_ICON_WEAPON_DIAMOND_EDGE) {
                                L0568_i_Defense -= L0568_i_Defense >> 2;
                        } else {
                                if (L0571_i_ActionHandObjectIconIndex == C043_ICON_WEAPON_HARDCLEAVE_EXECUTIONER) {
                                        L0568_i_Defense -= L0568_i_Defense >> 3;
                                }
                        }
                        if ((L0566_i_Damage = L0565_i_Damage = M003_RANDOM(32) + L0565_i_Damage - L0568_i_Defense) <= 1) {
                                T0231009:
                                if (!(L0565_i_Damage = M004_RANDOM(4))) {
                                        goto T0231015;
                                }
                                else {
                                        L0565_i_Damage++;
                                        if (((L0566_i_Damage += M003_RANDOM(16)) > 0) || (M005_RANDOM(2))) {
                                                L0565_i_Damage += M004_RANDOM(4);
                                                if (!M004_RANDOM(4)) {
                                                        L0565_i_Damage += F0025_MAIN_GetMaximumValue(0, L0566_i_Damage + M003_RANDOM(16));
                                                }
                                        }
                                }
                        }
                        L0565_i_Damage >>= 1;
                        L0565_i_Damage += M002_RANDOM(L0565_i_Damage) + M004_RANDOM(4);
                        L0565_i_Damage += M002_RANDOM(L0565_i_Damage);
                        L0565_i_Damage >>= 2;
                        L0565_i_Damage += M004_RANDOM(4) + 1;
                        if ((L0571_i_ActionHandObjectIconIndex == C040_ICON_WEAPON_VORPAL_BLADE) && !M007_GET(L0572_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL) && !(L0565_i_Damage >>= 1))
                                goto T0231015;
                        if (M003_RANDOM(64) < F0303_CHAMPION_GetSkillLevel(P0496_i_ChampionIndex, P0503_i_SkillIndex)) {
                                L0565_i_Damage += L0565_i_Damage + 10;
                        }
                        L0569_i_Outcome = F0190_GROUP_GetDamageCreatureOutcome(P0497_ps_Group, P0498_i_CreatureIndex, P0499_i_MapX, P0500_i_MapY, L0565_i_Damage, C1_TRUE);
                        F0304_CHAMPION_AddSkillExperience(P0496_i_ChampionIndex, P0503_i_SkillIndex, (L0565_i_Damage * M058_EXPERIENCE(L0572_ps_CreatureInfo->Properties) >> 4) + 3);
                        F0325_CHAMPION_DecrementStamina(P0496_i_ChampionIndex, M004_RANDOM(4) + 4);
                        goto T0231016;
                }
        }
        T0231015:
        L0565_i_Damage = 0;
        L0569_i_Outcome = C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP;
        F0325_CHAMPION_DecrementStamina(P0496_i_ChampionIndex, M005_RANDOM(2) + 2);
        T0231016:
        F0292_CHAMPION_DrawState(P0496_i_ChampionIndex);
        if (L0569_i_Outcome != C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP) {
                F0209_GROUP_ProcessEvents29to41(P0499_i_MapX, P0500_i_MapY, CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT, 0);
        }
        return L0565_i_Damage;
}

BOOLEAN F0232_GROUP_IsDoorDestroyedByAttack(
unsigned int16_t P0504_ui_MapX       SEPARATOR
unsigned int16_t P0505_ui_MapY       SEPARATOR
int16_t          P0506_i_Attack      SEPARATOR
BOOLEAN          P0507_B_MagicAttack SEPARATOR
unsigned int16_t P0508_Ticks         FINAL_SEPARATOR
{
        REGISTER DOOR* L0573_ps_Door;
        REGISTER unsigned char* L0574_puc_Square;
        EVENT L0575_s_Event;


        L0573_ps_Door = (DOOR*)F0157_DUNGEON_GetSquareFirstThingData(P0504_ui_MapX, P0505_ui_MapY);
        if ((P0507_B_MagicAttack && !L0573_ps_Door->MagicDestructible) || (!P0507_B_MagicAttack && !L0573_ps_Door->MeleeDestructible)) {
                return C0_FALSE;
        }
        if (P0506_i_Attack >= G0275_as_CurrentMapDoorInfo[L0573_ps_Door->Type].Defense) {
                L0574_puc_Square = &G0271_ppuc_CurrentMapData[P0504_ui_MapX][P0505_ui_MapY];
                if (M036_DOOR_STATE(*L0574_puc_Square) == C4_DOOR_STATE_CLOSED) {
                        if (P0508_Ticks) {
                                M033_SET_MAP_AND_TIME(L0575_s_Event.Map_Time, G0272_i_CurrentMapIndex, G0313_ul_GameTime + P0508_Ticks);
                                L0575_s_Event.A.A.Type = C02_EVENT_DOOR_DESTRUCTION;
                                L0575_s_Event.A.A.Priority = 0;
                                L0575_s_Event.B.Location.MapX = P0504_ui_MapX;
                                L0575_s_Event.B.Location.MapY = P0505_ui_MapY;
                                F0238_TIMELINE_AddEvent_GetEventIndex_CPSE(&L0575_s_Event);
                        } else {
                                M037_SET_DOOR_STATE(*L0574_puc_Square, C5_DOOR_STATE_DESTROYED);
                        }
                        return C1_TRUE;
                }
        }
        return C0_FALSE;
}
/* END PROJEXPL.C */
