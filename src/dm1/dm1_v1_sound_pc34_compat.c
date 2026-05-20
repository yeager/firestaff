/*
 * DM1 V1 Sound & Music System — ReDMCSB SOUND.C / MUSIC.C source-locked.
 *
 * Source anchors:
 *   SOUND.C  F0064_SOUND_RequestPlay_CPSD (lines ~600-700 MEDIA042/MEDIA167)
 *            F0065_SOUND_PlayPendingSound_CPSD (MEDIA167)
 *            F0505_SOUND_GetVolume (MEDIA371, directional 25x25 table)
 *   MUSIC.C  F0742_MUSIC_SetTrack, F0743_MUSIC_Update (MEDIA488)
 *            G2039_ai_MapIndexToMusicTrack (I34E: {2,14,9,6,11,6,5,12,15,16,17,10,19,3})
 *   DEFS.H   SOUND_DATA, CM1..C34 sound constants, play modes
 *   DATA.C   G0060_as_Graphic562_Sounds table (I34E)
 *
 * The 25x25 directional volume lookup table is from ReDMCSB SOUND.C
 * MEDIA425 (Amiga variant), used for I34E PC with 64-max L/R.
 */
#include "dm1_v1_sound_pc34_compat.h"
#include <string.h>

/* ── ReDMCSB G1028_aauc_DistanceToSoundVolume[25][25] (MEDIA425) ── */
static const uint8_t kVolumeTable[25][25] = {
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4,  5,  5,  5,  5,  5,  5,  5, 5, 4, 4, 4, 4, 4 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 5,  6,  6,  6,  6,  5,  5,  5, 5, 5, 5, 4, 4, 4 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  6,  6,  6,  6,  6,  6,  5, 5, 5, 5, 5, 4, 4 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  7,  7,  7,  7,  6,  6,  6, 6, 5, 5, 5, 5, 4 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  8,  8,  7,  7,  7,  7,  6, 6, 6, 5, 5, 5, 4 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 4, 6,  9,  9,  8,  8,  8,  7,  7, 6, 6, 6, 5, 5, 5 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 6, 10, 10, 10,  9,  8,  8,  7, 7, 6, 6, 5, 5, 5 },
    { 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 7, 12, 12, 11, 10,  9,  9,  8, 7, 7, 6, 6, 5, 5 },
    { 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 7, 15, 14, 13, 12, 11,  9,  8, 8, 7, 6, 6, 5, 5 },
    { 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 8, 20, 19, 16, 14, 12, 10,  9, 8, 7, 7, 6, 6, 5 },
    { 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 8, 29, 26, 21, 16, 13, 11, 10, 8, 7, 7, 6, 6, 5 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4, 8, 58, 41, 26, 19, 14, 12, 10, 9, 8, 7, 6, 6, 5 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 6, 64, 58, 29, 20, 15, 12, 10, 9, 8, 7, 6, 6, 5 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 41, 29, 19, 13, 10,  8,  7, 6, 6, 5, 5, 4, 4 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 21, 19, 15, 12, 10,  8,  7, 6, 5, 5, 4, 4, 4 },
    { 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 14, 13, 12, 10,  9,  7,  7, 6, 5, 5, 4, 4, 4 },
    { 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 5, 11, 10, 10,  9,  8,  7,  6, 6, 5, 5, 4, 4, 4 },
    { 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 5,  9,  8,  8,  7,  7,  6,  6, 5, 5, 4, 4, 4, 4 },
    { 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 5,  7,  7,  7,  7,  6,  6,  5, 5, 5, 4, 4, 4, 4 },
    { 0, 1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 4,  6,  6,  6,  6,  6,  5,  5, 5, 4, 4, 4, 4, 3 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,  6,  6,  5,  5,  5,  5,  5, 4, 4, 4, 4, 3, 3 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,  5,  5,  5,  5,  5,  4,  4, 4, 4, 4, 3, 3, 3 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3,  5,  5,  4,  4,  4,  4,  4, 4, 4, 3, 3, 3, 3 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3,  4,  4,  4,  4,  4,  4,  4, 4, 3, 3, 3, 3, 3 },
    { 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3,  4,  4,  4,  4,  4,  4,  4, 3, 3, 3, 3, 3, 3 }
};

/* ReDMCSB DATA.C G0060_as_Graphic562_Sounds for I34E.
 * Fields: graphicIndex, period, priority, loudDistance, softDistance */
static const DM1_SoundData kDefaultSoundData[DM1_SND_COUNT] = {
    /* 0  C00_SOUND_METALLIC_THUD */           { 671, 112, 80, 3, 7 },
    /* 1  C01_SOUND_SWITCH */                  { 672, 112, 90, 4, 8 },
    /* 2  C02_SOUND_DOOR_RATTLE */             { 673, 112, 85, 4, 9 },
    /* 3  C03_SOUND_DOOR_RATTLE_ENTRANCE */    { 673, 112, 85, 4, 9 },
    /* 4  C04_SOUND_WOODEN_THUD */             { 674, 112, 70, 3, 7 },
    /* 5  C05_SOUND_STRONG_EXPLOSION */        { 675, 112,100, 5,10 },
    /* 6  M541_SOUND_WEAK_EXPLOSION */         { 675, 112, 60, 3, 8 },
    /* 7  M561_SOUND_SCREAM */                 { 677, 112, 95, 5,10 },
    /* 8  C08_SOUND_SWALLOW */                 { 678, 112, 50, 1, 3 },
    /* 9  C09_SOUND_CHAMPION_0_DAMAGED */      { 679, 112, 85, 1, 3 },
    /*10  C10_SOUND_CHAMPION_1_DAMAGED */      { 680, 112, 85, 1, 3 },
    /*11  C11_SOUND_CHAMPION_2_DAMAGED */      { 681, 112, 85, 1, 3 },
    /*12  C12_SOUND_CHAMPION_3_DAMAGED */      { 682, 112, 85, 1, 3 },
    /*13  M563_SOUND_COMBAT */                 { 684, 112, 90, 4, 8 },
    /*14  M560_SOUND_BUZZ */                   { 685, 112, 75, 3, 7 },
    /*15  M562_SOUND_PARTY_DAMAGED */          { 687, 112, 85, 1, 4 },
    /*16  M542_SOUND_SPELL */                  { 683, 112, 80, 4, 9 },
    /*17  M619_SOUND_WAR_CRY */                { 707, 112, 70, 1, 3 },
    /*18  M620_SOUND_BLOW_HORN */              { 704, 112, 70, 4, 8 },
    /*19  ATTACK_SCREAMER_OITU */              { 690, 112, 80, 4, 8 },
    /*20  ATTACK_SCORPION */                   { 691, 112, 80, 4, 8 },
    /*21  ATTACK_WORM */                       { 692, 112, 80, 4, 8 },
    /*22  ATTACK_GIGGLER */                    { 693, 112, 80, 4, 8 },
    /*23  ATTACK_PAIN_RAT */                   { 688, 112, 80, 4, 8 },
    /*24  ATTACK_ROCK */                       { 708, 112, 80, 4, 8 },
    /*25  ATTACK_MUMMY_GHOST */                { 689, 112, 80, 4, 8 },
    /*26  ATTACK_WATER_ELEMENTAL */            { 709, 112, 80, 4, 8 },
    /*27  ATTACK_COUATL */                     { 710, 112, 80, 4, 8 },
    /*28  MOVE_ANIMATED_ARMOUR */              { 701, 112, 40, 3, 7 },
    /*29  MOVE_COUATL_WASP */                  { 702, 112, 40, 3, 7 },
    /*30  MOVE_MUMMY_TROLIN */                 { 703, 112, 40, 3, 7 },
    /*31  MOVE_SCREAMER_ROCK */                { 705, 112, 40, 3, 7 },
    /*32  MOVE_SLIME_WATER */                  { 706, 112, 40, 3, 7 },
    /*33  MOVE_RED_DRAGON */                   { 711, 112, 40, 3, 7 },
    /*34  MOVE_SKELETON */                     { 712, 112, 40, 3, 7 },
};

/* ReDMCSB MUSIC.C G2039_ai_MapIndexToMusicTrack (I34E) */
static const int16_t kMapToMusicTrack[DM1_MUSIC_MAP_COUNT] = {
    2, 14, 9, 6, 11, 6, 5, 12, 15, 16, 17, 10, 19, 3
};

void DM1_Sound_Init(DM1_SoundSystem* sys) {
    if (!sys) return;
    memset(sys, 0, sizeof(*sys));
    memcpy(sys->soundData, kDefaultSoundData, sizeof(kDefaultSoundData));
    sys->pending.pendingSoundIndex = DM1_PENDING_NONE;
    sys->pending.pendingSoundVolume = 0;
    sys->masterVolume = 255;
    sys->sfxVolume = 255;
    sys->musicVolume = 128;
    sys->lastPlayedSoundIndex = DM1_SND_NONE;
    memcpy(sys->music.mapToTrack, kMapToMusicTrack, sizeof(kMapToMusicTrack));
    sys->music.currentMapIndex = DM1_MUSIC_TRACK_NONE;
    sys->music.pendingTrack = DM1_MUSIC_TRACK_NONE;
    sys->music.playingTrack = DM1_MUSIC_TRACK_NONE;
    sys->music.countdownBeforeStart = -1;
    sys->music.musicOn = 1;
    sys->music.previousMusicOn = sys->music.musicOn;
}

/*
 * ReDMCSB F0505_SOUND_GetVolume — directional distance-based L/R volume.
 * Transforms (sourceMapX,sourceMapY) relative to party position+direction
 * into the 25x25 volume lookup table.  Returns 0 if out of audible range.
 */
int DM1_Sound_GetVolume(const DM1_SoundSystem* sys,
                        int16_t sourceMapX, int16_t sourceMapY,
                        DM1_SoundVolume* outVolume) {
    int rightCol, line, leftCol;
    if (!sys || !outVolume) return 0;

    /* Transform to party-relative coordinates per direction */
    switch (sys->partyDirection) {
        case 0: /* North */
            rightCol = sourceMapX - sys->partyMapX;
            line     = sourceMapY - sys->partyMapY;
            break;
        case 1: /* East */
            rightCol = sourceMapY - sys->partyMapY;
            line     = -(sourceMapX - sys->partyMapX);
            break;
        case 2: /* South */
            rightCol = -(sourceMapX - sys->partyMapX);
            line     = -(sourceMapY - sys->partyMapY);
            break;
        case 3: /* West */
            rightCol = -(sourceMapY - sys->partyMapY);
            line     = sourceMapX - sys->partyMapX;
            break;
        default:
            return 0;
    }

    /* Range check: audible within 12 squares */
    if (rightCol < -12 || rightCol > 12) return 0;
    if (line < -12 || line > 12) return 0;

    leftCol = -rightCol + 12;
    rightCol += 12;
    line += 12;

    outVolume->right = kVolumeTable[line][rightCol];
    outVolume->left  = kVolumeTable[line][leftCol];
    return 1;
}

/*
 * ReDMCSB F0064_SOUND_RequestPlay_CPSD (MEDIA167/I34E variant)
 */
void DM1_Sound_RequestPlay(DM1_SoundSystem* sys,
                           int16_t soundIndex,
                           int16_t mapX, int16_t mapY,
                           int mode) {
    const DM1_SoundData* snd;
    int distance, volume;

    if (!sys || soundIndex < 0 || soundIndex >= DM1_SND_COUNT) return;
    if (mode == DM1_MODE_DO_NOT_PLAY) return;

    sys->totalSoundRequests++;
    snd = &sys->soundData[soundIndex];

    /* Distance-based volume (simplified Atari ST model) */
    distance = (mapX - sys->partyMapX);
    if (distance < 0) distance = -distance;
    distance += (mapY - sys->partyMapY < 0) ?
                -(mapY - sys->partyMapY) : (mapY - sys->partyMapY);

    if (distance > snd->softDistance) return;

    if (distance < snd->loudDistance) {
        volume = 3; /* loud */
    } else {
        volume = 3 - (distance - snd->loudDistance);
        if (volume < 0) volume = 0;
    }

    if (mode == DM1_MODE_PLAY_IMMEDIATELY) {
        /* Play immediately — corresponds to ReDMCSB C00_MODE_PLAY_IMMEDIATELY */
        sys->lastPlayedSoundIndex = soundIndex;
        sys->totalSoundsPlayed++;
        return;
    }

    /* Mode 1 (PLAY_IF_PRIORITIZED) or mode >=2 (deferred):
     * Set as pending if louder or higher priority than current pending.
     * ReDMCSB: G0583_i_PendingSoundIndex / G0584_PendingSoundVolume */
    if (sys->pending.pendingSoundIndex == DM1_PENDING_NONE ||
        volume > sys->pending.pendingSoundVolume ||
        (volume == sys->pending.pendingSoundVolume &&
         snd->priority > sys->soundData[sys->pending.pendingSoundIndex].priority)) {
        sys->pending.pendingSoundIndex = soundIndex;
        sys->pending.pendingSoundVolume = (int16_t)volume;
    }
}

/*
 * ReDMCSB F0065_SOUND_PlayPendingSound_CPSD (MEDIA167)
 * Called once per game tick to flush the pending sound.
 */
void DM1_Sound_PlayPending(DM1_SoundSystem* sys) {
    if (!sys) return;
    if (sys->pending.pendingSoundIndex != DM1_PENDING_NONE) {
        sys->lastPlayedSoundIndex = sys->pending.pendingSoundIndex;
        sys->totalSoundsPlayed++;
        sys->totalPendingFlushes++;
        sys->pending.pendingSoundIndex = DM1_PENDING_NONE;
        sys->pending.pendingSoundVolume = 0;
    }
}

void DM1_Sound_SetPartyPosition(DM1_SoundSystem* sys,
                                int16_t mapX, int16_t mapY,
                                int16_t direction, int16_t mapIndex) {
    if (!sys) return;
    sys->partyMapX = mapX;
    sys->partyMapY = mapY;
    sys->partyDirection = direction;
    sys->partyMapIndex = mapIndex;
}

int DM1_Music_SetOn(DM1_SoundSystem* sys, int musicOn) {
    if (!sys) return 0;
    sys->music.musicOn = musicOn ? 1 : 0;
    return sys->music.musicOn;
}

int DM1_Music_Toggle(DM1_SoundSystem* sys) {
    if (!sys) return 0;
    return DM1_Music_SetOn(sys, !sys->music.musicOn);
}

int DM1_Music_IsOn(const DM1_SoundSystem* sys) {
    return (sys && sys->music.musicOn) ? 1 : 0;
}

/*
 * ReDMCSB COMMAND.C:2374-2379 flips G2024_B_PendingMusicOn for C141.
 * MUSIC.C:657-668 (I34E) only starts map music while that pending state is
 * true; MUSIC.C:659-661 pauses and invalidates the current map when it flips
 * false. Firestaff keeps the same runtime state boundary here.
 */
void DM1_Music_SetTrack(DM1_SoundSystem* sys, int16_t mapIndex) {
    if (!sys) return;
    if (!sys->music.musicOn) {
        return;
    }
    if (sys->music.currentMapIndex != mapIndex) {
        if (mapIndex >= 0 && mapIndex < DM1_MUSIC_MAP_COUNT) {
            sys->music.currentMapIndex = mapIndex;
            sys->music.pendingTrack = sys->music.mapToTrack[mapIndex];
            sys->music.countdownBeforeStart = 100;
        }
    }
}

/*
 * ReDMCSB F0743_MUSIC_Update (MEDIA488)
 * Called each game tick.
 */
void DM1_Music_Update(DM1_SoundSystem* sys) {
    if (!sys) return;
    if (sys->music.previousMusicOn != sys->music.musicOn) {
        sys->music.previousMusicOn = sys->music.musicOn;
        if (!sys->music.musicOn) {
            DM1_Music_Stop(sys);
            sys->music.currentMapIndex = DM1_MUSIC_TRACK_NONE;
            sys->music.pendingTrack = DM1_MUSIC_TRACK_NONE;
            return;
        }
    }
    if (!sys->music.musicOn) {
        return;
    }
    if (sys->music.countdownBeforeStart == 0) {
        if (sys->music.pendingTrack != sys->music.playingTrack) {
            sys->music.playingTrack = sys->music.pendingTrack;
            /* Actual playback delegated to audio_sdl_m11 */
        }
        sys->music.countdownBeforeStart = -1;
    } else if (sys->music.countdownBeforeStart > 0) {
        sys->music.countdownBeforeStart--;
    }
}

void DM1_Music_Stop(DM1_SoundSystem* sys) {
    if (!sys) return;
    sys->music.playingTrack = DM1_MUSIC_TRACK_NONE;
    sys->music.countdownBeforeStart = -1;
}

int DM1_Sound_GetLastPlayedIndex(const DM1_SoundSystem* sys) {
    return sys ? sys->lastPlayedSoundIndex : DM1_SND_NONE;
}

const DM1_SoundData* DM1_Sound_GetSoundData(const DM1_SoundSystem* sys, int16_t soundIndex) {
    if (!sys || soundIndex < 0 || soundIndex >= DM1_SND_COUNT) return NULL;
    return &sys->soundData[soundIndex];
}

const char* DM1_Sound_Name(int16_t soundIndex) {
    switch (soundIndex) {
        case DM1_SND_METALLIC_THUD:       return "Metallic Thud";
        case DM1_SND_SWITCH:              return "Switch";
        case DM1_SND_DOOR_RATTLE:         return "Door Rattle";
        case DM1_SND_DOOR_RATTLE_ENTRANCE:return "Door Rattle (Entrance)";
        case DM1_SND_WOODEN_THUD:         return "Wooden Thud";
        case DM1_SND_STRONG_EXPLOSION:    return "Strong Explosion";
        case DM1_SND_WEAK_EXPLOSION:      return "Weak Explosion";
        case DM1_SND_SCREAM:              return "Scream";
        case DM1_SND_SWALLOW:             return "Swallow";
        case DM1_SND_CHAMPION_0_DAMAGED:  return "Champion 0 Damaged";
        case DM1_SND_CHAMPION_1_DAMAGED:  return "Champion 1 Damaged";
        case DM1_SND_CHAMPION_2_DAMAGED:  return "Champion 2 Damaged";
        case DM1_SND_CHAMPION_3_DAMAGED:  return "Champion 3 Damaged";
        case DM1_SND_COMBAT:              return "Combat";
        case DM1_SND_BUZZ:                return "Buzz (Teleporter)";
        case DM1_SND_PARTY_DAMAGED:       return "Party Damaged";
        case DM1_SND_SPELL:               return "Spell";
        case DM1_SND_WAR_CRY:            return "War Cry";
        case DM1_SND_BLOW_HORN:           return "Blow Horn";
        case DM1_SND_ATTACK_SCREAMER_OITU:return "Attack: Screamer/Oitu";
        case DM1_SND_ATTACK_SCORPION:     return "Attack: Scorpion";
        case DM1_SND_ATTACK_WORM:         return "Attack: Worm";
        case DM1_SND_ATTACK_GIGGLER:      return "Attack: Giggler";
        case DM1_SND_ATTACK_PAIN_RAT:     return "Attack: Pain Rat";
        case DM1_SND_ATTACK_ROCK:         return "Attack: Rockpile";
        case DM1_SND_ATTACK_MUMMY_GHOST:  return "Attack: Mummy/Ghost";
        case DM1_SND_ATTACK_WATER_ELEMENTAL: return "Attack: Water Elemental";
        case DM1_SND_ATTACK_COUATL:       return "Attack: Couatl";
        case DM1_SND_MOVE_ANIMATED_ARMOUR:return "Move: Animated Armour";
        case DM1_SND_MOVE_COUATL_WASP:    return "Move: Couatl/Wasp";
        case DM1_SND_MOVE_MUMMY_TROLIN:   return "Move: Mummy/Trolin";
        case DM1_SND_MOVE_SCREAMER_ROCK:  return "Move: Screamer/Rock";
        case DM1_SND_MOVE_SLIME_WATER:    return "Move: Slime/Water";
        case DM1_SND_MOVE_RED_DRAGON:     return "Move: Red Dragon";
        case DM1_SND_MOVE_SKELETON:       return "Move: Skeleton";
        default:                           return "Unknown";
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — SOUND.C remaining function citations
 *
 *   SOUND.C:1144 F0061_SOUND_S
 *   SOUND.C:767 F0062_SOUND_I
 *   SOUND.C:1318 F0063_SOUND_K
 *   SOUND.C:181 F0501_SOUND_D
 *   SOUND.C:227 F0502_SOUND_C
 *   SOUND.C:1423 F0503_SOUND_L
 *   SOUND.C:556 F0504_SOUND_R
 *   SOUND.C:1397 F0799_SOUND_D
 *   SOUND.C:425 F0945_I
 *   SOUND.C:496 F0946_R
 *   SOUND.C:6 F1785_SOUND_R
 *   SOUND.C:1383 F1788_P
 *   SOUND.C:1388 F1789_U
 *   SOUND.C:1392 F1790_U
 *   SOUND.C:945 F2198_CPSX
 *   SOUND.C:948 F2199_CPSX
 *   SOUND.C:946 F2200_CPSX
 *   SOUND.C:947 F2201_CPSX
 *   SOUND.C:949 F2202_CPSX
 *   SOUND.C:945 F2207_CPSX
 * ══════════════════════════════════════════════════════════════════════ */

