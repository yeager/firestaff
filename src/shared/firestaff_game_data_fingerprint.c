#include "firestaff_game_data_fingerprint.h"
#include <string.h>
#include <ctype.h>

#define G_DM1  FIRESTAFF_GAME_DM1
#define G_CSB  FIRESTAFF_GAME_CSB
#define G_DM2  FIRESTAFF_GAME_DM2
#define P_AMI  FIRESTAFF_PLATFORM_AMIGA
#define P_ST   FIRESTAFF_PLATFORM_ATARI_ST
#define P_PC   FIRESTAFF_PLATFORM_PC
#define P_FMT  FIRESTAFF_PLATFORM_FM_TOWNS
#define P_98   FIRESTAFF_PLATFORM_PC98
#define P_X68  FIRESTAFF_PLATFORM_X68000
#define P_IIGS FIRESTAFF_PLATFORM_APPLE_IIGS
#define P_MAC  FIRESTAFF_PLATFORM_MACINTOSH
#define P_SCD  FIRESTAFF_PLATFORM_SEGA_CD
#define P_PSV  FIRESTAFF_PLATFORM_IBM_PSV
#define P_9821 FIRESTAFF_PLATFORM_PC9821
#define L_EN   FIRESTAFF_LANG_ENGLISH
#define L_FR   FIRESTAFF_LANG_FRENCH
#define L_DE   FIRESTAFF_LANG_GERMAN
#define L_JP   FIRESTAFF_LANG_JAPANESE
#define L_ML   FIRESTAFF_LANG_MULTILANGUAGE
#define F_GFX  FIRESTAFF_FILE_GRAPHICS_DAT
#define F_SONG FIRESTAFF_FILE_SONG_DAT
#define F_HCSB FIRESTAFF_FILE_HCSB_DAT
#define F_CED  FIRESTAFF_FILE_CEDTLS_DAT
#define F_ANIM FIRESTAFF_FILE_ANIMATE_DAT
#define F_SNG  FIRESTAFF_FILE_ENTER_SNG
#define F_AMG  FIRESTAFF_FILE_UTILITY_AMG
#define F_DMCO FIRESTAFF_FILE_DMCOORD_DAT
#define F_DEMO FIRESTAFF_FILE_DEMOIIGS_DAT
#define F_GAME FIRESTAFF_FILE_GRAPHICS_GAME
#define F_DNG  FIRESTAFF_FILE_DUNGEON_DAT
#define F_TITL FIRESTAFF_FILE_TITL_DAT
#define F_ENDA FIRESTAFF_FILE_ENDA_DAT
#define F_SWSH FIRESTAFF_FILE_SWSH_FTL

#define MD5(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
    {0x##a,0x##b,0x##c,0x##d,0x##e,0x##f,0x##g,0x##h, \
     0x##i,0x##j,0x##k,0x##l,0x##m,0x##n,0x##o,0x##p}

const FirestaffGameDataFingerprint firestaff_fingerprint_table[FIRESTAFF_FINGERPRINT_COUNT] = {
    /* CSB Amiga */
    {MD5(61,FB,FD,56,88,7C,94,AD,C2,68,88,A9,49,1C,66,11), G_CSB, P_AMI, L_ML, F_GFX, "3.1/3.3", "CSB Amiga 3.1 and 3.3 Multilanguage GRAPHICS.DAT"},
    {MD5(29,1E,1B,C6,80,3E,3D,C4,B9,74,C6,01,17,CA,5D,68), G_CSB, P_AMI, L_EN, F_GFX, "3.5", "CSB Amiga 3.5 English GRAPHICS.DAT"},
    {MD5(CE,FA,DD,FD,F5,65,1D,F2,C9,1F,61,B5,61,1A,83,62), G_CSB, P_AMI, L_ML, F_GFX, "3.5", "CSB Amiga 3.5 Multilanguage GRAPHICS.DAT"},
    {MD5(21,19,7B,1D,49,94,FD,83,5C,40,3D,5A,33,DC,AC,2B), G_CSB, P_AMI, L_EN, F_GFX, "X.X/3.1", "CSB Amiga X.X and 3.1 English GRAPHICS.DAT"},
    /* CSB Amiga Utility Disk */
    {MD5(BD,85,38,65,35,69,7D,F6,2B,DB,AE,73,74,0F,E4,35), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk DRAGON.AMG"},
    {MD5(4D,14,D2,F1,27,52,65,3B,CB,1D,44,64,84,0A,72,30), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk English NAKED.AMG"},
    {MD5(70,8E,11,3C,86,9A,B9,22,63,3E,88,5A,A7,2A,3C,77), G_CSB, P_AMI, L_EN, F_HCSB, "R1/RX/ST2.0", "CSB Amiga Utility Disk English Release 1 Release X and Atari ST 2.0 EN HCSB.DAT"},
    {MD5(74,96,B3,B8,B9,FF,6E,23,68,EA,C9,A1,6B,E8,23,0B), G_CSB, P_AMI, L_EN, F_HCSB, "R2/R3", "CSB Amiga Utility Disk English Release 2 and 3 HCSB.DAT"},
    {MD5(8A,EF,81,65,97,5A,36,A4,26,AA,2E,A3,98,23,C1,49), G_CSB, P_AMI, L_EN, F_CED, "R2/R3", "CSB Amiga Utility Disk English Release 2 Release 3 CEDTLS.DAT"},
    {MD5(37,0A,3C,46,AE,CE,E0,4B,DC,6E,AE,73,77,12,08,C0), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk EXPLOS1.AMG"},
    {MD5(DC,22,B3,DA,E4,C1,C7,99,E7,E2,1F,66,F2,91,AA,9C), G_CSB, P_AMI, L_FR, F_AMG, NULL, "CSB Amiga Utility Disk French and German NAKED.AMG"},
    {MD5(B3,67,D5,83,74,A7,99,DE,88,BC,1A,24,C6,32,07,71), G_CSB, P_AMI, L_FR, F_CED, NULL, "CSB Amiga Utility Disk French CEDTLS.DAT"},
    {MD5(BB,F3,AD,A2,DA,97,22,57,7F,EE,A4,FA,21,3B,32,F1), G_CSB, P_AMI, L_FR, F_HCSB, NULL, "CSB Amiga Utility Disk French Release HCSB.DAT"},
    {MD5(A3,CF,F5,2E,F8,E4,D8,58,53,28,21,81,21,9C,DE,63), G_CSB, P_AMI, L_DE, F_CED, NULL, "CSB Amiga Utility Disk German CEDTLS.DAT"},
    {MD5(9E,0D,A6,C5,A5,69,85,9C,61,91,20,1D,CC,6E,6A,AE), G_CSB, P_AMI, L_DE, F_HCSB, NULL, "CSB Amiga Utility Disk German Release HCSB.DAT"},
    {MD5(59,CE,0C,A0,34,65,65,95,B0,E0,A7,A9,B1,A7,13,16), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk MAGEXPLO.AMG"},
    {MD5(CC,A4,12,90,E6,12,1F,BB,45,1E,E5,52,83,CE,4A,BB), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk SWIPE.AMG"},
    {MD5(70,52,8F,71,43,BD,0A,3C,A7,BF,D3,98,87,22,A8,A8), G_CSB, P_AMI, L_EN, F_AMG, NULL, "CSB Amiga Utility Disk TELE2.AMG"},
    /* CSB Amiga Game Disk */
    {MD5(66,95,D2,AC,EB,CE,49,F9,5D,B1,D8,F3,A5,C7,33,DE), G_CSB, P_AMI, L_FR, F_DNG, "3.3", "CSB Amiga 3.3 French Dungeon.DAT"},
    {MD5(5B,59,0E,A3,A6,F5,EE,D5,13,B5,67,8B,01,46,8E,E4), G_CSB, P_AMI, L_FR, F_TITL, "3.3", "CSB Amiga 3.3 French TITL.DAT"},
    {MD5(9F,2B,73,FF,73,AD,00,32,81,0D,79,02,1C,90,0C,A9), G_CSB, P_AMI, L_FR, F_ENDA, "3.3", "CSB Amiga 3.3 French ENDA.DAT"},
    {MD5(FF,38,72,BA,AE,D8,EE,4E,83,EE,3C,06,84,B2,EE,EC), G_CSB, P_AMI, L_FR, F_SWSH, "3.3", "CSB Amiga 3.3 French SWSH.FTL"},
    /* CSB Atari ST */
    {MD5(EB,F6,A5,7A,F3,F2,77,82,E3,58,C0,49,0B,FD,2F,2E), G_CSB, P_ST, L_EN, F_GFX, "2.0/2.1", "CSB Atari ST 2.0 and 2.1 English GRAPHICS.DAT"},
    {MD5(9F,8F,EB,26,9C,95,9C,9F,E7,22,AC,08,F9,9D,9C,35), G_CSB, P_ST, L_EN, F_ANIM, NULL, "CSB Atari ST Utility Disk English ANIMATE.DAT"},
    /* CSB FM Towns */
    {MD5(40,5B,75,70,38,EE,A3,C2,63,E6,0F,24,08,54,D6,DE), G_CSB, P_FMT, L_EN, F_GFX, NULL, "CSB FM-Towns English GRAPHICS.DAT"},
    {MD5(76,1D,6F,C5,88,B3,1A,EA,AA,9C,AF,37,25,E1,11,B9), G_CSB, P_FMT, L_JP, F_GFX, NULL, "CSB FM-Towns Japanese GRAPHICS.DAT"},
    /* CSB PC-98 */
    {MD5(61,F5,1D,7F,FB,E0,A8,CC,D6,A4,9C,2F,EC,32,95,FC), G_CSB, P_98, L_JP, F_SNG, NULL, "CSB PC-98 ENTER.SNG"},
    {MD5(76,1D,6F,C5,88,B3,1A,EA,AA,9C,AF,37,25,E1,11,B9), G_CSB, P_98, L_JP, F_GFX, NULL, "CSB PC-98 Japanese GRAPHICS.DAT"},
    /* CSB X68000 */
    {MD5(EE,1E,C8,A6,3E,0D,41,D4,5D,2E,07,3B,FC,DF,5D,F7), G_CSB, P_X68, L_JP, F_SNG, NULL, "CSB X68000 Japanese ENTER.SNG"},
    {MD5(8F,E5,9D,4F,2A,F5,B5,7A,4C,B1,44,47,C0,11,D3,F1), G_CSB, P_X68, L_JP, F_GFX, NULL, "CSB X68000 Japanese GRAPHICS.DAT"},
    /* DM1 Amiga */
    {MD5(06,79,E3,9D,A9,DC,C2,E8,55,CB,33,C6,C6,4D,DC,B5), G_DM1, P_AMI, L_DE, F_GFX, "2.0/2.2", "DM Amiga 2.0 and 2.2 German GRAPHICS.DAT"},
    {MD5(6A,2F,13,5B,53,C2,22,0F,02,51,FA,10,3E,2A,6E,7E), G_DM1, P_AMI, L_EN, F_GFX, "2.0", "DM Amiga 2.0 English GRAPHICS.DAT"},
    {MD5(DD,37,39,54,B3,FB,12,7D,B7,38,79,46,13,1E,A3,22), G_DM1, P_AMI, L_FR, F_GFX, "2.0", "DM Amiga 2.0 French GRAPHICS.DAT"},
    {MD5(B3,59,31,B5,5D,B6,49,A1,BD,2D,41,5B,61,B2,98,01), G_DM1, P_AMI, L_EN, F_GFX, "2.1/2.2", "DM Amiga 2.1 and 2.2 English GRAPHICS.DAT"},
    {MD5(7F,94,58,E4,A3,97,2D,06,E6,49,A6,FA,85,A7,F3,4B), G_DM1, P_AMI, L_ML, F_GFX, "3.6", "DM Amiga 3.6 Multilanguage GRAPHICS.DAT"},
    {MD5(49,1C,A9,39,F9,AB,B3,3C,EE,B2,66,19,B8,41,FE,91), G_DM1, P_AMI, L_EN, F_GFX, "Demo", "DM Amiga Demo English GRAPHICS.DAT"},
    /* DM1 Apple IIGS */
    {MD5(12,ED,EF,86,58,07,96,97,AA,E1,C2,DC,B1,6D,5F,67), G_DM1, P_IIGS, L_EN, F_DEMO, NULL, "DM Apple IIGS English DEMOIIGS.DAT"},
    {MD5(10,45,A3,69,52,E6,4E,B2,BC,7B,4C,7B,29,65,B1,12), G_DM1, P_IIGS, L_EN, F_GAME, NULL, "DM Apple IIGS English GRAPHICS.GAME"},
    /* DM1 Atari ST */
    {MD5(B3,CF,D8,4E,44,CD,F0,7C,E2,EE,BA,47,E8,7F,77,2B), G_DM1, P_ST, L_EN, F_GFX, "1.0-1208", "DM Atari ST 1.0 (1987-12-08) English GRAPHICS.DAT"},
    {MD5(7E,EE,39,69,93,74,5E,8A,F2,12,F4,4D,75,FF,6C,1A), G_DM1, P_ST, L_EN, F_GFX, "1.0-1211", "DM Atari ST 1.0 (1987-12-11) English GRAPHICS.DAT"},
    {MD5(50,95,A1,36,92,70,22,35,D2,E7,4F,6B,2B,13,67,A9), G_DM1, P_ST, L_EN, F_GFX, "1.1", "DM Atari ST 1.1 English GRAPHICS.DAT"},
    {MD5(9C,E2,EA,F7,A9,E7,86,20,E3,F1,75,94,43,7C,AF,FA), G_DM1, P_ST, L_EN, F_GFX, "1.2", "DM Atari ST 1.2 English GRAPHICS.DAT"},
    {MD5(2B,DC,5F,43,1F,84,C0,EC,E7,38,F5,4D,BD,78,7C,3B), G_DM1, P_ST, L_DE, F_GFX, "1.2", "DM Atari ST 1.2 German GRAPHICS.DAT"},
    {MD5(0D,7A,F4,4D,D1,4F,38,34,64,28,8A,BD,CE,C7,6A,FC), G_DM1, P_ST, L_FR, F_GFX, "1.3", "DM Atari ST 1.3 French GRAPHICS.DAT"},
    /* DM1 FM Towns */
    {MD5(C1,0C,51,2F,63,46,1E,BE,79,B5,AC,36,51,15,B6,1B), G_DM1, P_FMT, L_EN, F_GFX, NULL, "DM FM-Towns English GRAPHICS.DAT"},
    {MD5(ED,F4,7D,7D,A5,DE,81,84,60,4D,6D,80,47,7E,F0,1F), G_DM1, P_FMT, L_JP, F_GFX, NULL, "DM FM-Towns Japanese GRAPHICS.DAT"},
    /* DM1 PC */
    {MD5(FA,6B,1A,A2,9E,19,14,18,71,3B,F2,CD,A9,3D,96,2E), G_DM1, P_PC, L_EN, F_GFX, "3.4", "DM PC 3.4 English GRAPHICS.DAT"},
    {MD5(F9,34,D9,7E,43,E1,BA,6E,51,59,83,9A,CB,CD,06,11), G_DM1, P_PC, L_ML, F_GFX, "3.4", "DM PC 3.4 Multilanguage GRAPHICS.DAT"},
    {MD5(C2,0E,5B,8F,75,6E,36,0A,63,15,95,CC,92,60,F6,2D), G_DM1, P_PC, L_EN, F_SONG, NULL, "DM PC SONG.DAT"},
    /* DM1 PC-98 */
    {MD5(EA,EC,21,31,54,15,73,65,8D,A9,9C,13,86,5C,2E,67), G_DM1, P_98, L_JP, F_GFX, "2.0a", "DM PC-98 2.0a Japanese GRAPHICS.DAT"},
    {MD5(3E,3B,9B,18,00,C6,7B,4C,E8,50,E0,87,81,3C,32,5D), G_DM1, P_98, L_JP, F_GFX, "2.0b", "DM PC-98 2.0b Japanese GRAPHICS.DAT"},
    /* DM1 X68000 */
    {MD5(FE,08,A9,7C,64,76,66,14,B7,11,64,FA,06,ED,A5,45), G_DM1, P_X68, L_JP, F_GFX, NULL, "DM X68000 Japanese GRAPHICS.DAT"},
    /* DM2 Amiga */
    {MD5(1C,94,0E,A9,57,03,EA,EA,0E,CD,F8,4D,17,E9,54,B9), G_DM2, P_AMI, L_EN, F_GFX, NULL, "DMII Amiga GRAPHICS.DAT"},
    /* DM2 FM Towns */
    {MD5(02,7F,F3,B8,DD,C2,C4,C4,CD,DA,7A,DA,0B,0B,C4,6C), G_DM2, P_FMT, L_JP, F_GFX, NULL, "DMII FM-Towns Japanese GRAPHICS.DAT"},
    /* DM2 IBM PSV */
    {MD5(F9,26,A7,55,4B,DF,B5,85,21,05,17,9E,67,B8,A2,64), G_DM2, P_PSV, L_JP, F_GFX, NULL, "DMII IBM PSV Japanese GRAPHICS.DAT"},
    /* DM2 Macintosh */
    {MD5(4B,F2,8B,3D,84,E6,79,9D,76,86,C6,AA,F9,6C,BF,23), G_DM2, P_MAC, L_EN, F_GFX, "Demo", "DMII Macintosh English Demo GRAPHICS.DAT"},
    {MD5(5C,AB,25,F6,B9,75,95,7E,AE,4A,20,31,74,E7,F2,A6), G_DM2, P_MAC, L_EN, F_GFX, NULL, "DMII Macintosh English GRAPHICS.DAT"},
    {MD5(28,3D,54,56,C4,F6,76,60,94,89,E2,00,21,96,05,BB), G_DM2, P_MAC, L_JP, F_GFX, NULL, "DMII Macintosh Japanese GRAPHICS.DAT"},
    /* DM2 PC */
    {MD5(5D,C5,D1,5A,E4,A3,EE,85,75,7B,3D,36,22,ED,22,21), G_DM2, P_PC, L_EN, F_DMCO, "0.9beta", "DMII PC 0.9 Beta English DMCOORD.DAT"},
    {MD5(0A,63,E2,2C,D8,3F,E3,C9,0A,AC,FF,DA,5C,0F,06,2C), G_DM2, P_PC, L_EN, F_GFX, "0.9beta", "DMII PC 0.9 Beta English GRAPHICS.DAT"},
    {MD5(BD,2D,31,6E,B7,7C,6D,6D,21,7B,FB,76,BD,0D,7E,41), G_DM2, P_PC, L_EN, F_GFX, "Demo-19950112", "DMII PC English Demo 19950112 GRAPHICS.DAT"},
    {MD5(43,CF,7E,85,79,E8,3E,9F,1F,A9,B4,11,69,58,42,FD), G_DM2, P_PC, L_EN, F_GFX, "Demo-19950509", "DMII PC English Demo 19950509 GRAPHICS.DAT"},
    {MD5(9A,6A,A7,06,AE,9B,ED,5D,DD,68,FF,D7,30,52,44,76), G_DM2, P_PC, L_EN, F_GFX, "Demo-19950713", "DMII PC English Demo 19950713 GRAPHICS.DAT"},
    {MD5(25,24,7E,DE,4D,AB,B6,A7,1E,5D,AB,DF,BC,D5,90,7D), G_DM2, P_PC, L_EN, F_GFX, NULL, "DMII PC English GRAPHICS.DAT"},
    {MD5(B4,D7,33,57,6E,A6,0C,41,73,7F,79,F2,12,FA,F5,28), G_DM2, P_PC, L_FR, F_GFX, NULL, "DMII PC French GRAPHICS.DAT"},
    {MD5(E5,2A,B5,E0,17,15,04,2B,16,A4,DC,FF,02,05,2E,5D), G_DM2, P_PC, L_DE, F_GFX, NULL, "DMII PC German and EnglishJewelCase GRAPHICS.DAT"},
    /* DM2 PC-9801 */
    {MD5(A0,27,71,95,09,9B,2A,CE,51,D4,E0,85,F7,EE,F8,35), G_DM2, P_98, L_JP, F_GFX, "Demo", "DMII PC-9801 Japanese Demo GRAPHICS.DAT"},
    {MD5(A6,69,AD,F2,A6,FF,88,7E,0D,45,1D,93,C8,46,F5,7F), G_DM2, P_98, L_JP, F_GFX, NULL, "DMII PC-9801 Japanese GRAPHICS.DAT"},
    /* DM2 PC-9821 */
    {MD5(A3,10,23,DB,49,D5,D8,5E,46,9C,93,23,67,18,12,C7), G_DM2, P_9821, L_JP, F_GFX, "R1", "DMII PC-9821 Japanese Release 1 GRAPHICS.DAT"},
    {MD5(A8,0C,55,5A,85,8E,F7,77,0E,1D,7F,3D,2E,37,FE,C3), G_DM2, P_9821, L_JP, F_GFX, "R2", "DMII PC-9821 Japanese Release 2 GRAPHICS.DAT"},
    /* DM2 Sega CD */
    {MD5(DB,CE,D1,3A,38,D3,03,6F,42,B9,79,71,75,B7,EC,88), G_DM2, P_SCD, L_EN, F_GFX, NULL, "DMII Sega CD English GRAPHICS.DAT"},
    {MD5(A6,54,BA,19,E9,A6,91,9F,46,81,8E,CD,23,D7,EA,9D), G_DM2, P_SCD, L_JP, F_GFX, NULL, "DMII Sega CD Japanese GRAPHICS.DAT"},
};

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

FirestaffGameDataClassifyResult firestaff_game_data_classify_md5(
    const uint8_t md5[16])
{
    FirestaffGameDataClassifyResult result;
    int i;

    memset(&result, 0, sizeof(result));
    if (!md5) return result;

    for (i = 0; i < FIRESTAFF_FINGERPRINT_COUNT; i++) {
        if (memcmp(md5, firestaff_fingerprint_table[i].md5, 16) == 0) {
            result.valid = 1;
            result.entry = &firestaff_fingerprint_table[i];
            return result;
        }
    }
    return result;
}

FirestaffGameDataClassifyResult firestaff_game_data_classify_hex(
    const char *hex32)
{
    uint8_t md5[16];
    int i;

    if (!hex32) {
        FirestaffGameDataClassifyResult r;
        memset(&r, 0, sizeof(r));
        return r;
    }
    for (i = 0; i < 16; i++) {
        int hi = hex_nibble(hex32[i * 2]);
        int lo = hex_nibble(hex32[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            FirestaffGameDataClassifyResult r;
            memset(&r, 0, sizeof(r));
            return r;
        }
        md5[i] = (uint8_t)((hi << 4) | lo);
    }
    return firestaff_game_data_classify_md5(md5);
}

const char *firestaff_game_name(FirestaffGame game) {
    switch (game) {
    case FIRESTAFF_GAME_DM1: return "Dungeon Master";
    case FIRESTAFF_GAME_CSB: return "Chaos Strikes Back";
    case FIRESTAFF_GAME_DM2: return "Dungeon Master II";
    default: return "Unknown";
    }
}

const char *firestaff_platform_name(FirestaffPlatform platform) {
    switch (platform) {
    case FIRESTAFF_PLATFORM_AMIGA:     return "Amiga";
    case FIRESTAFF_PLATFORM_ATARI_ST:  return "Atari ST";
    case FIRESTAFF_PLATFORM_PC:        return "PC";
    case FIRESTAFF_PLATFORM_FM_TOWNS:  return "FM Towns";
    case FIRESTAFF_PLATFORM_PC98:      return "PC-98";
    case FIRESTAFF_PLATFORM_X68000:    return "X68000";
    case FIRESTAFF_PLATFORM_APPLE_IIGS: return "Apple IIGS";
    case FIRESTAFF_PLATFORM_MACINTOSH: return "Macintosh";
    case FIRESTAFF_PLATFORM_SEGA_CD:   return "Sega CD";
    case FIRESTAFF_PLATFORM_IBM_PSV:   return "IBM PS/V";
    case FIRESTAFF_PLATFORM_PC9821:    return "PC-9821";
    default: return "Unknown";
    }
}

const char *firestaff_language_name(FirestaffLanguage language) {
    switch (language) {
    case FIRESTAFF_LANG_ENGLISH:       return "English";
    case FIRESTAFF_LANG_FRENCH:        return "French";
    case FIRESTAFF_LANG_GERMAN:        return "German";
    case FIRESTAFF_LANG_JAPANESE:      return "Japanese";
    case FIRESTAFF_LANG_MULTILANGUAGE: return "Multilanguage";
    default: return "Unknown";
    }
}
