/* Actual M11 native IIGS startup; source ownership and lifecycle, not visual parity. */
#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "dm1_v1_dungeon_light_admission_f0337_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "render_sdl_m11.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(c,m) do { if (!(c)) { fprintf(stderr,"FAIL: %s\n",m); goto cleanup; } } while (0)

static int check_scan(const char *root, int version, int missing_program) {
    static const char *const ids[]={"iigs20-en","iigs21-en"};
    static const char *const roles[]={"graphics","dungeon","program","startup"};
    static const char *const names[]={"GRAPHICS.GAME","DUNGEON.GAME","DUNGEON.MASTER","SYSTEM/START"};
    const char *hashes[]={"1045a36952e64eb2bc7b4c7b2965b112","e10959de5ce74e48a17e2cc84d1c0b92",
        version==20 ? "51b9d22c7418651ee49ad77f37c80a3c" : "eccc49e7b29ee233cfb10615e8ec86f3",
        "3bcae225346ac24fb1975aa6d93f0fa6"};
    M12_AssetStatus *status=calloc(1,sizeof(*status));
    char expected[ASSET_PATH_MAX], prepared[ASSET_PATH_MAX]; int result=0;
    REQUIRE(status,"bounded public scanner snapshot");
    for (unsigned api=0u; api<2u; ++api) {
        int index=M12_AssetStatus_FindVersionIndex("dm1",ids[version==21]);
        int other=M12_AssetStatus_FindVersionIndex("dm1",ids[version==20]);
        const M12_AssetVersionStatus *selected, *neighbor;
        memset(status,0,sizeof(*status));
        if (api) M12_AssetStatus_ScanGame(status,root,"dm1");
        else M12_AssetStatus_Scan(status,root);
        REQUIRE(index>=0 && other>=0,"both native editions exist in public catalogue");
        selected=M12_AssetStatus_GetVersion(status,"dm1",(size_t)index);
        neighbor=M12_AssetStatus_GetVersion(status,"dm1",(size_t)other);
        REQUIRE(selected && neighbor && !neighbor->matched,"selected raw PO cannot borrow neighboring native edition");
        if (missing_program) {
            int program_absent=1;
            REQUIRE(!status->dm1Available && !selected->matched,
                "both public scan APIs reject incomplete selected raw disk without default-root fallback");
            for (size_t i=0u; i<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++i) {
                const M12_AssetRequiredFileStatus *file=M12_AssetStatus_GetRequiredFile(status,"dm1",i);
                if (file && file->roleId && strcmp(file->roleId,"program")==0) program_absent=!file->matched;
            }
            REQUIRE(program_absent,"required-file report never claims a missing authentic program is present");
            REQUIRE(!M12_AssetStatus_PrepareDM1RuntimeVersion(status,ids[version==21],prepared,sizeof(prepared)),
                "incomplete scan cannot prepare a native runtime locator");
            continue;
        }
        REQUIRE(status->dm1Available && selected->matched &&
            snprintf(expected,sizeof(expected),"%s::GRAPHICS.GAME",root)<(int)sizeof(expected) &&
            strcmp(selected->matchedPath,expected)==0 &&
            strcmp(selected->matchedMd5,hashes[0])==0 &&
            M12_AssetStatus_PrepareDM1RuntimeVersion(status,ids[version==21],prepared,sizeof(prepared)) &&
            strcmp(prepared,expected)==0,"both public scan APIs and preparation retain exact selected native graphics locator");
        REQUIRE(M12_AssetStatus_GetRequiredFileCount(status,"dm1")==4u,"native scanner publishes all four required roles");
        for (unsigned role=0u; role<4u; ++role) {
            const M12_AssetRequiredFileStatus *matched=NULL;
            for (size_t i=0u; i<4u; ++i) {
                const M12_AssetRequiredFileStatus *file=M12_AssetStatus_GetRequiredFile(status,"dm1",i);
                if (file && file->roleId && strcmp(file->roleId,roles[role])==0) matched=file;
            }
            REQUIRE(matched && matched->matched &&
                snprintf(expected,sizeof(expected),"%s::%s",root,names[role])<(int)sizeof(expected) &&
                strcmp(matched->matchedPath,expected)==0 && strcmp(matched->sourcePath,expected)==0 &&
                strcmp(matched->matchedHash,hashes[role])==0,
                "native required-file report preserves four exact same-owner source paths and authenticated hashes");
        }
    }
    result=1;
cleanup:
    free(status); return result;
}
static int check_archive_scan(const char *archive) {
    M12_AssetStatus *status=calloc(1,sizeof(*status));
    char locators[2][ASSET_PATH_MAX]; int result=0;
    REQUIRE(status,"bounded archive scanner snapshot");
    for (unsigned api=0u; api<2u; ++api) {
        memset(status,0,sizeof(*status));
        if (api) M12_AssetStatus_ScanGame(status,archive,"dm1"); else M12_AssetStatus_Scan(status,archive);
        for (unsigned edition=0u; edition<2u; ++edition) {
            const char *id=edition ? "iigs21-en" : "iigs20-en";
            int index=M12_AssetStatus_FindVersionIndex("dm1",id);
            const M12_AssetVersionStatus *row=index<0 ? NULL : M12_AssetStatus_GetVersion(status,"dm1",(size_t)index);
            char owner[ASSET_PATH_MAX], expected[ASSET_PATH_MAX];
            REQUIRE(snprintf(owner,sizeof(owner),"%s::Dungeon Master for Apple IIGS 2.%u (English) Original (Not working).po",archive,edition)<(int)sizeof(owner) &&
                snprintf(expected,sizeof(expected),"%s::GRAPHICS.GAME",owner)<(int)sizeof(expected) &&
                row && row->matched && strcmp(row->matchedPath,expected)==0 &&
                M12_AssetStatus_PrepareDM1RuntimeVersion(status,id,locators[edition],sizeof(locators[edition])) &&
                strcmp(locators[edition],expected)==0,
                "archive scan admits both exact original edition owners and preserves each through preparation");
            REQUIRE(snprintf(expected,sizeof(expected),"%s::DUNGEON.MASTER",owner)<(int)sizeof(expected) &&
                asset_file_matches_md5(expected,edition ? "eccc49e7b29ee233cfb10615e8ec86f3" :
                    "51b9d22c7418651ee49ad77f37c80a3c"),
                "each native archive locator has its own authentic edition-specific program");
        }
        REQUIRE(strcmp(locators[0],locators[1])!=0,"native archive editions retain distinct original program owners");
    }
    result=1;
cleanup:
    free(status); return result;
}

static int check_elija_equipment(const M11_GameViewState *view, int committed, int candidate) {
    static const unsigned short chain[]={0x4c12u,0x5807u,0x6800u,0x5808u,0x5809u};
    static const unsigned slots[]={CHAMPION_SLOT_FEET,CHAMPION_SLOT_BACKPACK_1,CHAMPION_SLOT_TORSO,CHAMPION_SLOT_LEGS};
    const struct DungeonThings_Compat *things=view->world.things;
    unsigned short thing=F0511_DUNGEON_GetSquareFirstThing_Compat(view->world.dungeon,things,0,8,7);
    int result=0;
    for (unsigned i=0u; i<(committed ? 1u : 5u); ++i) {
        unsigned type,index; const uint8_t *raw;
        REQUIRE(thing==chain[i],"ELIJA original mirror chain retains exact source Thing ownership");
        type=THING_GET_TYPE(thing); index=THING_GET_INDEX(thing);
        REQUIRE(type<16u && index<(unsigned)things->thingCounts[type] && things->rawThingData[type],"bounded original mirror chain");
        raw=things->rawThingData[type]+index*s_thingDataByteCount[type];
        thing=(unsigned short)((unsigned)raw[0]|((unsigned)raw[1]<<8));
    }
    REQUIRE(thing==THING_ENDOFLIST,"ELIJA wall chain contains exactly its original remaining owners");
    REQUIRE(things->sensors[18].sensorType==(committed ? 0u : 127u),
        "only confirmed resurrection disables original C127 sensor");
    if (candidate || committed) for (unsigned slot=0u; slot<CHAMPION_SLOT_COUNT; ++slot) {
        unsigned short expected=THING_NONE;
        for (unsigned i=0u; i<4u; ++i) if (slot==slots[i]) expected=chain[i+1u];
        REQUIRE(view->world.party.champions[0].inventory[slot]==expected,
            "ELIJA exact original sandals, blue box and robe pieces occupy native-selected host slots");
    }
    result=1;
cleanup:
    return result;
}

/* Independent original bitstream/TEXTSTRING oracle: C127 at(8,7), cell1,
 * ordinal0; party-square(9,7) textThing6/word112. The route follows only
 * authentic map-zero corridor squares, without relocating the party. */
static int check_recruitment(M11_GameViewState *view, uint8_t *frame) {
    static const uint8_t route[][2]={
        {1,4},{1,5},{1,6},{1,7},{2,7},{3,7},{4,7},{4,6},{4,5},{4,4},{4,3},
        {4,2},{5,2},{6,2},{6,3},{7,3},{8,3},{8,4},{9,4},{9,5},{9,6},{9,7}};
    static const unsigned short attributes[]={42,40,42,36,53,40};
    static const unsigned long experience[]={1000,0,3250,0,
        250,250,500,0,0,0,0,0,500,250,2000,500,0,0,0,0};
    struct ChampionState_Compat *champion;
    int result=0;
    REQUIRE(view->world.party.mapX==1 && view->world.party.mapY==3 &&
        !view->world.party.championCount,"native recruitment starts at authentic empty HoC entry");
    for (size_t i=0u; i<sizeof(route)/sizeof(route[0]); ++i) {
        int dx=(int)route[i][0]-view->world.party.mapX;
        int dy=(int)route[i][1]-view->world.party.mapY;
        int direction=dx>0 ? 1 : dx<0 ? 3 : dy>0 ? 2 : 0;
        REQUIRE(abs(dx)+abs(dy)==1,"source-pinned HoC route contains adjacent squares");
        for (unsigned turns=0u; view->world.party.direction!=direction && turns<4u; ++turns)
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        REQUIRE(view->world.party.mapX==route[i][0] && view->world.party.mapY==route[i][1],
            "ordinary native footsteps reach each authentic source corridor square");
    }
    for (unsigned turns=0u; view->world.party.direction!=3 && turns<4u; ++turns)
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
    REQUIRE(M11_GameView_GetFrontMirrorOrdinal(view)==0,
        "actual source C127 wall resolves ELIJA portrait ordinal zero");
    REQUIRE(check_elija_equipment(view,0,0),"original equipment is linked to native mirror before selection");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACTION);
    REQUIRE(view->candidateMirrorPanelActive && view->candidateMirrorOrdinal==0 &&
        view->world.party.championCount==1,"native action selects original mirror candidate");
    REQUIRE(check_elija_equipment(view,0,1),"tentative candidate references original equipment without unlinking it");
    { unsigned draws=view->dm1IigsDrawCount; int panel=0,portraits=0;
      M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
      for (unsigned i=0u; i<M11_ASSET_CACHE_SLOTS; ++i) if (view->assetLoader.cache[i].loaded) {
          if (view->assetLoader.cache[i].graphicIndex==40u) panel=1;
          if (view->assetLoader.cache[i].graphicIndex==26u) portraits=1;
      }
      REQUIRE(view->dm1IigsDrawCount==draws+1u && panel && portraits && view->candidateMirrorPanelActive,
          "actual native modal Draw consumes original C040 panel and C026 portrait atlas"); }
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_BACK);
    REQUIRE(view->active && view->dm1IigsActive && !view->candidateMirrorPanelActive &&
        view->world.party.championCount==0,"native candidate cancel removes tentative champion without exiting game");
    REQUIRE(check_elija_equipment(view,0,0),"cancel retains all four original wall possessions");
    REQUIRE(M11_GameView_HandlePointerButton(view,110,80,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        view->candidateMirrorPanelActive && view->candidateMirrorOrdinal==0 &&
        check_elija_equipment(view,0,1),"native source portrait hitbox selects original candidate without unlinking possessions");
    { unsigned draws=view->dm1IigsDrawCount;
      M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
      REQUIRE(view->dm1IigsDrawCount==draws+1u && view->candidateMirrorPanelActive,
          "pointer-selected candidate presents through actual native modal Draw"); }
    REQUIRE(M11_GameView_HandlePointerButton(view,110,150,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        !view->candidateMirrorPanelActive && !view->world.party.championCount && view->active &&
        check_elija_equipment(view,0,0),"native source cancel hitbox preserves original mirror and all possessions");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACTION);
    REQUIRE(view->candidateMirrorPanelActive && view->candidateMirrorOrdinal==0,
        "canceled authentic mirror remains available for selection");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACCEPT);
    champion=&view->world.party.champions[0];
    REQUIRE(!view->candidateMirrorPanelActive && view->world.party.championCount==1 &&
        champion->present && champion->portraitIndex==0 && champion->sex=='M' &&
        memcmp(champion->name,"ELIJA",5u)==0 && memcmp(champion->title,"LION OF YAITOPYA",sizeof("LION OF YAITOPYA")-1u)==0 &&
        champion->hp.current==60u && champion->hp.maximum==60u &&
        champion->stamina.current==580u && champion->stamina.maximum==580u &&
        champion->mana.current==22u && champion->mana.maximum==22u &&
        memcmp(champion->attributes,attributes,sizeof(attributes))==0 && champion->portraitBitmapValid,
        "native accept resurrects original ELIJA identity, vitals, statistics and source portrait");
    { static const uint8_t native_statistics[]={50,42,40,42,36,53,40};
      const struct ChampionLifecycleState_Compat *life=&view->world.lifecycle.champions[0];
      REQUIRE(life->maxHealth==60u && life->maxStamina==580u && life->maxMana==22u,
          "native lifecycle maximum vitals decode original four-nibble hex values");
      for (unsigned stat=0u; stat<7u; ++stat)
          REQUIRE(life->statistics[stat][LIFECYCLE_STAT_MAXIMUM]==native_statistics[stat] &&
              life->statistics[stat][LIFECYCLE_STAT_CURRENT]==native_statistics[stat] &&
              life->statistics[stat][LIFECYCLE_STAT_MINIMUM]==(stat==LIFECYCLE_STAT_LUCK ? 10u : 30u),
              "all seven original native lifecycle statistics retain hex values and source Luck minimum"); }
    for (unsigned skill=0u; skill<4u; ++skill)
        REQUIRE(champion->skillExperience[skill]==experience[skill],"original four base-class sums survive resurrection");
    for (unsigned skill=0u; skill<20u; ++skill) {
        unsigned long effective=experience[skill]; int level=1;
        REQUIRE(view->world.lifecycle.champions[0].skills20[skill].experience==(int32_t)experience[skill] &&
            !view->world.lifecycle.champions[0].skills20[skill].temporaryExperience,
            "all twenty native skills reach actual lifecycle authority without temporary XP");
        /* CHAMPION.C F0303: hidden skills average base+hidden, then
         * repeatedly halve at500; original ELIJA equipment grants no bonus. */
        if (skill>=4u) effective=(effective+experience[(skill-4u)/4u])/2u;
        while (effective>=500u) { effective>>=1; ++level; }
        REQUIRE(M11_GameView_GetSkillLevel(view,0,(int)skill)==level,
            "public skill level reads native lifecycle base/hidden experience using source formula");
    }
    REQUIRE(check_elija_equipment(view,1,0) && M11_GameView_GetFrontMirrorOrdinal(view)<0,
        "accept unlinks exactly four original possessions and makes the source mirror unavailable");
    { unsigned draws=view->dm1IigsDrawCount;
      M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
      REQUIRE(view->dm1IigsDrawCount==draws+1u && !view->candidateMirrorPanelActive &&
          view->world.party.championCount==1,"accepted original champion presents through actual native runtime Draw"); }
    result=1;
cleanup:
    return result;
}

/* Additional original mirrors exercise F0299/F0301 with genuine possessions.
 * CHANI Moonstone changes maximum mana, BORIS Rabbit's Foot changes Luck,
 * ZED's original unlit torch becomes lit even when selection is canceled. */
static int check_equipped_heroes(M11_GameViewState *view, uint8_t *frame) {
    static const uint8_t routes[3][6][2]={
        {{9,6},{9,5},{10,5}},
        {{10,4},{11,4},{12,4},{13,4},{13,3},{14,3}},
        {{14,4},{14,5},{14,6}}};
    static const unsigned lengths[]={3,6,3}, ordinals[]={4,5,7}, directions[]={2,0,2};
    static const uint8_t walls[][2]={{10,6},{14,2},{14,7}};
    static const unsigned short chains[][5]={
        {0x0c16,0x1812,0x1813,0x1459,0x1874},
        {0x8c1d,0x9826,0x9827,0xa802,0x9828},
        {0x0c1e,0x1829,0x182a,0x182b,0x2803}};
    static const char *const names[]={"ZED","CHANI","BORIS"};
    static const uint8_t statistics[][7]={{58,40,40,40,50,40,40},{57,37,47,57,37,47,37},{25,35,45,55,40,45,40}};
    static const unsigned base_mana[]={10,17,28}, maximum_mana[]={10,20,28};
    int result=0;
    for (unsigned hero=0u; hero<3u; ++hero) {
        int slot=(int)hero+1;
        for (unsigned step=0u; step<lengths[hero]; ++step) {
            int dx=(int)routes[hero][step][0]-view->world.party.mapX;
            int dy=(int)routes[hero][step][1]-view->world.party.mapY;
            int direction=dx>0 ? 1 : dx<0 ? 3 : dy>0 ? 2 : 0;
            REQUIRE(abs(dx)+abs(dy)==1,"additional original hero route has adjacent source squares");
            for (unsigned turn=0u; view->world.party.direction!=direction && turn<4u; ++turn)
                (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
            for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
            REQUIRE(view->world.party.mapX==routes[hero][step][0] && view->world.party.mapY==routes[hero][step][1],
                "native footsteps reach additional authentic equipment-bearing mirrors");
        }
        for (unsigned turn=0u; view->world.party.direction!=(int)directions[hero] && turn<4u; ++turn)
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        REQUIRE(M11_GameView_GetFrontMirrorOrdinal(view)==(int)ordinals[hero],"native wall matches independently decoded hero ordinal");
        if (!hero) REQUIRE(!view->world.things->weapons[89].lit,"original ZED torch starts unlit");
        for (unsigned attempt=0u; attempt<2u; ++attempt) {
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACTION);
            REQUIRE(view->candidateMirrorPanelActive && view->candidateMirrorPartyIndex==slot &&
                view->world.party.championCount==slot+1 &&
                memcmp(view->world.party.champions[slot].name,names[hero],strlen(names[hero]))==0,
                "actual native mirror selects source-owned equipment-bearing champion");
            REQUIRE(view->world.party.champions[slot].mana.current==base_mana[hero] &&
                view->world.party.champions[slot].mana.maximum==maximum_mana[hero] &&
                view->world.lifecycle.champions[slot].maxMana==maximum_mana[hero],
                "native Moonstone changes maximum mana only and binds lifecycle before confirmation");
            for (unsigned stat=0u; stat<7u; ++stat) {
                unsigned bonus=hero==2u && stat==0u ? 10u : 0u;
                REQUIRE(view->world.lifecycle.champions[slot].statistics[stat][LIFECYCLE_STAT_CURRENT]==statistics[hero][stat]+bonus &&
                    view->world.lifecycle.champions[slot].statistics[stat][LIFECYCLE_STAT_MAXIMUM]==statistics[hero][stat]+bonus &&
                    view->world.lifecycle.champions[slot].statistics[stat][LIFECYCLE_STAT_MINIMUM]==(stat ? 30u : 10u)+bonus,
                    "native Rabbit's Foot modifies all three Luck rows without changing other source statistics");
            }
            { unsigned short thing=F0511_DUNGEON_GetSquareFirstThing_Compat(view->world.dungeon,view->world.things,0,walls[hero][0],walls[hero][1]);
              for (unsigned i=0u; i<5u; ++i) {
                  const uint8_t *raw;
                  REQUIRE(thing==chains[hero][i],"tentative modifiers preserve exact original wall chain");
                  raw=view->world.things->rawThingData[THING_GET_TYPE(thing)]+THING_GET_INDEX(thing)*s_thingDataByteCount[THING_GET_TYPE(thing)];
                  thing=(unsigned short)(raw[0]|((unsigned)raw[1]<<8));
              }
              REQUIRE(thing==THING_ENDOFLIST,"source mirror contains exactly original five linked Things"); }
            if (!hero) REQUIRE(view->world.things->weapons[89].lit &&
                view->world.party.champions[slot].inventory[CHAMPION_SLOT_ACTION_HAND]==0x1459u,
                "native F0301 lights original ZED torch in action hand before confirmation");
            { uint32_t before=view->world.gameTick;
              (void)M11_GameView_AdvanceIdleTick(view);
              M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
              REQUIRE(view->world.gameTick==before,"candidate modal pauses source world advancement"); }
            if (!attempt) {
                (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_BACK);
                REQUIRE(!view->candidateMirrorPanelActive && view->world.party.championCount==slot &&
                    !view->world.lifecycle.champions[slot].maxMana,
                    "cancel removes tentative champion and modified lifecycle row");
                if (!hero) REQUIRE(view->world.things->weapons[89].lit,
                    "original cancellation retains F0301 torch activation side effect");
            } else (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACCEPT);
        }
        REQUIRE(!view->candidateMirrorPanelActive && view->world.party.championCount==slot+1 &&
            view->world.party.champions[slot].mana.maximum==maximum_mana[hero],
            "resurrection commits genuine native equipment modifiers once");
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        REQUIRE(view->world.party.champions[slot].mana.maximum==maximum_mana[hero] &&
            view->world.lifecycle.champions[slot].maxMana==maximum_mana[hero] &&
            view->world.lifecycle.champions[slot].statistics[0][LIFECYCLE_STAT_MAXIMUM]==statistics[hero][0]+(hero==2u ? 10u : 0u) &&
            view->world.lifecycle.champions[slot].statistics[0][LIFECYCLE_STAT_MINIMUM]==(hero==2u ? 20u : 10u),
            "native idle lifecycle synchronization preserves committed maximum mana and Luck modifiers");
    }
    result=1;
cleanup:
    return result;
}

static int check_original_scroll_opening(M11_GameViewState *view) {
    static const uint8_t original[]={0xfe,0xff,0x21,0x04};
    struct DungeonThings_Compat *things=view->world.things;
    uint8_t *raw=NULL; int result=0;
    REQUIRE(things && things->scrollCount==35 && things->rawThingData[THING_TYPE_SCROLL],
        "original native world carries its 35 source scrolls");
    raw=things->rawThingData[THING_TYPE_SCROLL];
    REQUIRE(memcmp(raw,original,4u)==0 && M11_GameView_GetObjectIconIndexForThing(view,0x1c00u)==31,
        "authentic scroll zero starts closed with original text Thing33");
    /* Original F0301 clears the six Closed bits of this actual native
     * record; no replacement payload or relocated game object is created. */
    raw[3]&=3u;
    REQUIRE(raw[2]==0x21u && raw[3]==0u && ((unsigned)raw[2]|((unsigned)raw[3]<<8))==33u &&
        M11_GameView_GetObjectIconIndexForThing(view,0x1c00u)==30,
        "source-defined opening of original scroll preserves text33 and changes icon31 to30");
    result=1;
cleanup:
    if (raw) memcpy(raw,original,4u);
    return result;
}

static int check_original_hand_light(const M11_GameViewState *view) {
    const unsigned short hands[8]={0x1459,0x1812,THING_NONE,THING_NONE,THING_NONE,THING_NONE,THING_NONE,THING_NONE};
    DM1_V1_DungeonLightReceiptF0337Pc34 receipt; int result=0;
    REQUIRE(view->world.things->weapons[89].type==2u && !view->world.things->weapons[89].lit &&
        view->world.things->weapons[89].chargeCount==15u && view->world.things->armours[18].type==32u,
        "pure light admission uses actual original unlit charged torch and nonweapon armour records");
    REQUIRE(dm1_v1_dungeon_light_admit_f0337_pc34(view->world.things,hands,&receipt) && receipt.valid &&
        receipt.torchLightPower[0]==15 && receipt.torchLightPower[1]==0,
        "source F0337 counts held torch charge without Lit gate and accepts authentic nonweapon hand as zero");
    result=1;
cleanup:
    return result;
}
static int check_native_palette(M11_GameViewState *view, uint8_t *frame, int level_one) {
    static const int power_amount[]={0,5,12,24,33,40,46,51,59,68,76,82,89,94,97,100};
    static const int thresholds[]={99,75,50,25,1,0};
    struct DungeonViewLight_Compat light; uint8_t palette[256][3];
    int expected=0, result=0; unsigned draws=view->dm1IigsDrawCount;
    REQUIRE(F0890b_ORCH_ComputeDungeonViewLight_Compat(&view->world,&light),
        "actual original native world admits complete F0337 light receipt");
    if (level_one) {
        unsigned charge=view->world.things->weapons[89].chargeCount;
        int total=power_amount[charge]+view->world.magic.magicalLightAmount;
        if (total<=0) expected=5; else while (thresholds[expected]>total) ++expected;
        REQUIRE(!light.forcedBrightMap && light.totalLightAmount==total &&
            light.litTorchCount==(charge ? 1 : 0),"native level-one lighting uses live original torch charge and magical light");
    } else REQUIRE(light.forcedBrightMap,"source difficulty-zero HoC forces brightest palette");
    REQUIRE(light.paletteIndex==expected,"native light receipt matches independent original power/threshold tables");
    M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
    REQUIRE(view->dm1IigsDrawCount==draws+1u && M11_Render_CopyIndexedPaletteRgb6(palette),
        "actual native Draw publishes admitted light-dependent indexed palette");
    for (unsigned i=0u; i<32u; ++i) {
        uint16_t rgb=i<16u ? view->dm1IigsInstallation.metadata.interface_palette[i] :
            view->dm1IigsInstallation.metadata.dungeon_palettes[expected][i-16u];
        for (unsigned channel=0u; channel<3u; ++channel) {
            unsigned nibble=(rgb>>(8u-4u*channel))&15u;
            REQUIRE(palette[i][channel]==(uint8_t)((nibble<<2)|(nibble>>2)),
                "renderer retains native interface colors and selected original dungeon RGB4 palette row");
        }
    }
    result=1;
cleanup:
    return result;
}
static int check_first_descent(M11_GameViewState *view, uint8_t *frame) {
    static const uint8_t route[][2]={{15,6},{15,7},{15,8},{15,9},{14,9},{13,9},{12,9},
        {11,9},{10,9},{9,9},{8,9},{7,9},{6,9},{5,9},{4,9},{4,10},{4,11},{4,12},{4,13},{4,14},{4,15},{3,15}};
    int result=0;
    REQUIRE(view->world.party.mapIndex==0 && view->world.party.mapX==14 && view->world.party.mapY==6 &&
        view->world.party.championCount==4 && view->world.dungeon->maps[0].difficulty==0 &&
        view->world.dungeon->maps[1].difficulty==1,"four original heroes begin native first descent from BORIS");
    REQUIRE(check_native_palette(view,frame,0),"actual HoC presentation uses native forced-bright palette");
    for (unsigned step=0u; step<sizeof(route)/sizeof(route[0]); ++step) {
        int dx=(int)route[step][0]-view->world.party.mapX;
        int dy=(int)route[step][1]-view->world.party.mapY;
        int direction=dx>0 ? 1 : dx<0 ? 3 : dy>0 ? 2 : 0;
        REQUIRE(abs(dx)+abs(dy)==1,"original stair route uses adjacent source squares");
        for (unsigned turn=0u; view->world.party.direction!=direction && turn<4u; ++turn)
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        if (step==13u) {
            DM1_V1_IigsDocSnapshot sound;
            unsigned expected=(view->dm1IigsAudioMetadata.distance_volume[11][12]/15u)*15u;
            REQUIRE((view->world.dungeon->tiles[0].squareData[5*19+9]&7u)==0u,
                "ordinary party pressure sensor at six-nine opens original exit door before passage");
            REQUIRE(view->world.party.mapX==6 && view->world.party.mapY==9 && view->world.party.direction==3 &&
                M11_Audio_GetDm1IigsSnapshot(&view->audioState,&sound) && sound.latest_pair[2]>=0 &&
                sound.left[sound.latest_pair[2]]==expected && sound.right[sound.latest_pair[2]]==expected &&
                !view->audioState.playedMarkerCount,
                "actual original exit-door rattle retains target five-nine and native front-one-square stereo attenuation");
        }
        if (step==12u) {
            DM1_V1_IigsDocSnapshot before,after;
            int audible=0;
            REQUIRE(F0720_SENSOR_CountAudiblePartyPass_Compat(view->world.dungeon,
                view->world.things,0,6,9,SENSOR_EVENT_WALK_ON,0,&view->world.party,
                THING_NONE,&audible) && audible==1 &&
                M11_Audio_GetDm1IigsSnapshot(&view->audioState,&before),
                "original exit pressure sensor is one audible party-enter trigger");
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
            REQUIRE(M11_Audio_GetDm1IigsSnapshot(&view->audioState,&after) &&
                after.triggers==before.triggers+1u && after.latest_pair[1]>=0 &&
                after.left[after.latest_pair[1]]==255u &&
                after.right[after.latest_pair[1]]==255u &&
                !view->audioState.playedMarkerCount,
                "original audible floor sensor requests local prioritized DOC C01 on entry");
        } else (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        if (step+1u<sizeof(route)/sizeof(route[0]))
            REQUIRE(view->world.party.mapIndex==0 && view->world.party.mapX==route[step][0] &&
                view->world.party.mapY==route[step][1],"ordinary native footsteps pass authentic source door toward exit stairs");
    }
    if (view->world.party.mapIndex==0 && view->world.party.mapX==3 && view->world.party.mapY==15)
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_USE_STAIRS);
    REQUIRE(view->world.party.mapIndex==1 && view->world.partyMapIndex==1 &&
        view->world.party.mapX==3 && view->world.party.mapY==1 && view->world.party.direction==0 &&
        view->world.iigsCreatureProfileBound,"native stairs preserve original offset-mapped level-one arrival and north facing");
    { static const int original_groups[][4]={
        {9,10,1,19},
        {8,10,3,18},
        {170,6,4,13},
        {154,6,5,24},
        {169,10,6,2},
        {7,6,8,24},
        {2,6,11,5},
        {0,6,13,4},
        {160,6,16,16},
        {153,6,20,29},
        {1,6,22,5},
        {5,10,24,13},
        {3,10,27,2},
        {6,10,27,20},
        {4,6,31,9}};
      REQUIRE(view->world.newPartyMapIndex==-1 && view->world.creatureAICount==15,
          "native level transition consumes pending map change and activates all fifteen original map-one groups");
      for (unsigned group=0u; group<15u; ++group) {
          const struct CreatureAIState_Compat *active=NULL;
          for (int i=0; i<view->world.creatureAICount; ++i)
              if (view->world.creatureAI[i].reserved0==original_groups[group][0]) {
                  REQUIRE(!active,"each original native group has one active owner");
                  active=&view->world.creatureAI[i];
              }
          REQUIRE(active && active->groupMapIndex==1 && active->creatureType==original_groups[group][1] &&
              active->groupMapX==original_groups[group][2] && active->groupMapY==original_groups[group][3] &&
              view->world.things->groups[active->reserved0].creatureType==active->creatureType &&
              F0890e_ORCH_GetCreatureProfileForWorld_Compat(&view->world,active->creatureType)->baseHealth==
                  view->dm1IigsInstallation.metadata.creature_info[active->creatureType].base_health,
              "active group identity, position and behavior owner match independent native map-one C04 oracle");
      }
    }
    /* A fresh charge15 torch selects row0, which would hide a renderer
     * hardcoded to brightest. Advance the actual world, bounded by one
     * native512-tick fuel interval, until source fuel crosses that boundary. */
    { unsigned initial=view->world.things->weapons[89].chargeCount;
      for (unsigned ticks=0u; view->world.things->weapons[89].chargeCount==15u && ticks<520u; ++ticks)
          (void)M11_GameView_AdvanceIdleTick(view);
      REQUIRE(view->world.things->weapons[89].chargeCount>0u &&
          view->world.things->weapons[89].chargeCount<15u &&
          (initial<15u || view->world.things->weapons[89].chargeCount==14u),
          "ordinary native ticks cross original torch fuel boundary without replacing game state");
      { struct DungeonViewLight_Compat dimmed;
        REQUIRE(F0890b_ORCH_ComputeDungeonViewLight_Compat(&view->world,&dimmed) && dimmed.paletteIndex>0,
            "authentic level-one torch decay selects a nonbright palette and exposes fixed-row-zero rendering"); } }
    REQUIRE(check_native_palette(view,frame,1),"actual level-one native Draw selects source light-dependent palette");
    result=1;
cleanup:
    return result;
}

/* Route from the authentic IIGS stair arrival to the closest active map-one
 * creature. BFS uses the production movement planner for collision rules;
 * replay uses ordinary M11 inputs and never writes party coordinates. */
static int check_iigs_creature_combat(M11_GameViewState *view) {
    enum { MAX_NODES=32*32*4, MAX_ROUTE=MAX_NODES };
    static const int moves[6]={MOVE_FORWARD,MOVE_RIGHT,MOVE_BACKWARD,
        MOVE_LEFT,MOVE_TURN_RIGHT,MOVE_TURN_LEFT};
    static const M12_MenuInput inputs[6]={M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_RIGHT,M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT,M12_MENU_INPUT_TURN_RIGHT,
        M12_MENU_INPUT_TURN_LEFT};
    int queue[MAX_NODES],previous[MAX_NODES],via[MAX_NODES],route[MAX_ROUTE];
    const struct DungeonMapDesc_Compat *map;
    int width,height,head=0,tail=0,goal=-1,routeCount=0,node,targetGroup=-1;
    uint32_t attackBefore;
    uint32_t attackAfterFirst;
    unsigned hpBefore=0u;
    int firstAttackSeen=0;
    int repeatedAttacks=0;
    if (!view || view->world.party.mapIndex!=1 ||
        !view->world.dungeon || !view->world.things) return 0;
    map=&view->world.dungeon->maps[1]; width=map->width; height=map->height;
    if (width>32 || height>32 || width<=0 || height<=0) return 0;
    /* The authentic stair arrival (3,1) is behind the first-level door.
     * Walk the original corridor to (6,0), face its source button door at
     * (6,1), and open it through the ordinary viewport pointer route. */
    {
        static const M12_MenuInput opening_route[]={M12_MENU_INPUT_UP,
            M12_MENU_INPUT_TURN_RIGHT,M12_MENU_INPUT_UP,M12_MENU_INPUT_UP,
            M12_MENU_INPUT_UP,M12_MENU_INPUT_TURN_RIGHT};
        unsigned short door=F0511_DUNGEON_GetSquareFirstThing_Compat(
            view->world.dungeon,view->world.things,1,6,1);
        int vx,vy,vw,vh;
        if (view->world.party.mapX!=3 || view->world.party.mapY!=1 ||
            view->world.party.direction!=0 || THING_GET_TYPE(door)!=THING_TYPE_DOOR ||
            THING_GET_INDEX(door)>=view->world.things->doorCount ||
            !view->world.things->doors[THING_GET_INDEX(door)].button) {
            fprintf(stderr,"FAIL: original IIGS group door/button route is unavailable\n");
            return 0;
        }
        for (unsigned step=0;step<sizeof(opening_route)/sizeof(opening_route[0]);++step) {
            (void)M11_GameView_HandleInput(view,opening_route[step]);
            for (unsigned tick=0;tick<8u;++tick)
                (void)M11_GameView_AdvanceIdleTick(view);
        }
        if (!M11_GameView_GetViewportRect(&vx,&vy,&vw,&vh)) return 0;
        if (view->world.party.mapX!=6 || view->world.party.mapY!=0 ||
            view->world.party.direction!=2 ||
            M11_GameView_HandlePointerButton(view,vx+168,vy+48,
                DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_IGNORED) {
            fprintf(stderr,"FAIL: natural IIGS route did not reach/click original group door\n");
            return 0;
        }
        for (unsigned tick=0;tick<16u;++tick)
            (void)M11_GameView_AdvanceIdleTick(view);
        if ((view->world.dungeon->tiles[1].squareData[6*height+1]&7u)!=0u &&
            (view->world.dungeon->tiles[1].squareData[6*height+1]&7u)!=1u &&
            (view->world.dungeon->tiles[1].squareData[6*height+1]&7u)!=5u) {
            fprintf(stderr,"FAIL: authentic IIGS button click did not open group door\n");
            return 0;
        }
    }
#define IIGS_NODE(x,y,d) ((((y)*width+(x))<<2)|((d)&3))
    for (node=0;node<MAX_NODES;++node) { previous[node]=-2; via[node]=-1; }
    node=IIGS_NODE(view->world.party.mapX,view->world.party.mapY,
                    view->world.party.direction);
    previous[node]=-1; queue[tail++]=node;
    while (head<tail && goal<0) {
        struct PartyState_Compat party=view->world.party;
        int x,y,d;
        node=queue[head++]; d=node&3; x=(node>>2)%width; y=(node>>2)/width;
        for (int ai=0;ai<view->world.creatureAICount;++ai) {
            const struct CreatureAIState_Compat *active=&view->world.creatureAI[ai];
            if (active->groupMapIndex==1 &&
                abs(active->groupMapX-x)+abs(active->groupMapY-y)==1) {
                goal=node; targetGroup=active->reserved0; break;
            }
        }
        if (goal>=0) break;
        party.mapIndex=1; party.mapX=x; party.mapY=y; party.direction=d;
        for (int mi=0;mi<6;++mi) {
            struct MovementResult_Compat result;
            int next;
            if (!F0702_MOVEMENT_TryMove_Compat(view->world.dungeon,&party,
                                                moves[mi],&result) ||
                result.newMapIndex!=1 || result.newMapX<0 || result.newMapY<0 ||
                result.newMapX>=width || result.newMapY>=height) continue;
            next=IIGS_NODE(result.newMapX,result.newMapY,result.newDirection);
            if (previous[next]!=-2 || tail>=MAX_NODES) continue;
            previous[next]=node; via[next]=mi; queue[tail++]=next;
        }
    }
    if (goal<0) {
        fprintf(stderr,"IIGS route: no adjacent group start=%d,%d active=%d map=%dx%d reached=%d\n",
            view->world.party.mapX,view->world.party.mapY,
            view->world.creatureAICount,width,height,tail);
        for (int yy=0;yy<4;++yy) for (int xx=2;xx<7;++xx) {
            unsigned char sq=view->world.dungeon->tiles[1].squareData[xx*height+yy];
            fprintf(stderr," tile(%d,%d)=%02x",xx,yy,sq);
            if (xx==6) fputc('\n',stderr);
        }
        for (int ai=0;ai<view->world.creatureAICount;++ai) {
            const struct CreatureAIState_Compat *active=&view->world.creatureAI[ai];
            fprintf(stderr," group=%d map=%d pos=%d,%d state=%d\n",
                active->reserved0,active->groupMapIndex,active->groupMapX,
                active->groupMapY,active->stateKind);
        }
        return 0;
    }
    while (previous[goal]>=0 && routeCount<MAX_ROUTE) {
        route[routeCount++]=via[goal]; goal=previous[goal];
    }
    attackBefore=view->world.lifecycle.lastCreatureAttackTime;
    for (unsigned c=0;c<view->world.party.championCount;++c)
        hpBefore+=view->world.party.champions[c].hp.current;
    while (routeCount>0) {
        struct PartyState_Compat before=view->world.party;
        int ticks,action=route[--routeCount];
        (void)M11_GameView_HandleInput(view,inputs[action]);
        for (ticks=0;ticks<8 && before.mapX==view->world.party.mapX &&
             before.mapY==view->world.party.mapY &&
             before.direction==view->world.party.direction;++ticks)
            (void)M11_GameView_AdvanceIdleTick(view);
        if (before.mapX==view->world.party.mapX &&
            before.mapY==view->world.party.mapY &&
            before.direction==view->world.party.direction) {
            fprintf(stderr,"IIGS route: rejected action=%d pose=%d,%d,d%d\n",
                action,view->world.party.mapX,view->world.party.mapY,
                view->world.party.direction);
            return 0;
        }
    }
    for (unsigned ticks=0;ticks<8192u;++ticks) {
        unsigned hpAfter=0u;
        (void)M11_GameView_AdvanceIdleTick(view);
        for (unsigned c=0;c<view->world.party.championCount;++c)
            hpAfter+=view->world.party.champions[c].hp.current;
        if (view->world.lifecycle.lastCreatureAttackTime!=attackBefore &&
            hpAfter<hpBefore && targetGroup>=0 &&
            targetGroup<view->world.things->groupCount &&
            view->world.things->groups[targetGroup].behavior==DM1_BEHAVIOR_ATTACK) {
            firstAttackSeen=1;
            break;
        }
    }
    if (firstAttackSeen && targetGroup>=0 &&
        targetGroup<view->world.things->groupCount &&
        view->world.things->groups[targetGroup].health[0]!=0) {
        /* After proving real native damage, protect the party so the
         * recurrence probe measures the original group timeline instead of
         * ending early through champion death. */
        view->world.magic.partyShieldDefense=100u;
        attackAfterFirst=view->world.lifecycle.lastCreatureAttackTime;
        for (unsigned tick=0u;tick<1500u;++tick) {
            (void)M11_GameView_AdvanceIdleTick(view);
            for (int emissionIndex=0;
                 emissionIndex<view->lastTickResult.emissionCount;
                 ++emissionIndex) {
                const struct TickEmission_Compat *emission=
                    &view->lastTickResult.emissions[emissionIndex];
                if (emission->kind==EMIT_CREATURE_ATTACK &&
                    emission->payload[0]==targetGroup) ++repeatedAttacks;
            }
            if (view->world.lifecycle.lastCreatureAttackTime>
                    attackAfterFirst && repeatedAttacks>0 &&
                view->world.things->groups[targetGroup].health[0]!=0)
                return 1;
        }
        fprintf(stderr,"FAIL: authentic IIGS group %d did not attack again in 21 seconds (attacks=%d tick=%u attack=%u->%u)\n",
            targetGroup,repeatedAttacks,(unsigned)view->world.gameTick,
            (unsigned)attackAfterFirst,
            (unsigned)view->world.lifecycle.lastCreatureAttackTime);
        return 0;
    }
    fprintf(stderr,"FAIL: authentic IIGS group %d no damage after route pose=%d,%d,d%d tick=%u attack=%u->%u hp=%u->",
        targetGroup,view->world.party.mapX,view->world.party.mapY,
        view->world.party.direction,(unsigned)view->world.gameTick,
        (unsigned)attackBefore,(unsigned)view->world.lifecycle.lastCreatureAttackTime,hpBefore);
    for (unsigned c=0;c<view->world.party.championCount;++c)
        fprintf(stderr,"%s%u",c?",":"",view->world.party.champions[c].hp.current);
    fputc('\n',stderr);
    return 0;
#undef IIGS_NODE
}

static int original_thing_owners(const M11_GameViewState *view, unsigned short identity) {
    const struct GameWorld_Compat *world=&view->world; int owners=0;
    for (int c=0; c<4; ++c) for (int slot=0; slot<CHAMPION_SLOT_COUNT; ++slot)
        if (world->party.champions[c].inventory[slot]!=THING_NONE &&
            (world->party.champions[c].inventory[slot]&0x3fffu)==(identity&0x3fffu)) ++owners;
    if (view->leaderHandObjectPresent && (view->leaderHandThing&0x3fffu)==(identity&0x3fffu)) ++owners;
    for (int map=0; map<world->dungeon->header.mapCount; ++map)
        for (int x=0; x<world->dungeon->maps[map].width; ++x)
            for (int y=0; y<world->dungeon->maps[map].height; ++y) {
                unsigned short thing=F0511_DUNGEON_GetSquareFirstThing_Compat(world->dungeon,world->things,map,x,y);
                unsigned links=0u;
                while (thing!=THING_NONE && thing!=THING_ENDOFLIST) {
                    unsigned type=THING_GET_TYPE(thing), index=THING_GET_INDEX(thing); const uint8_t *raw;
                    if (++links>1024u || type>=16u || !s_thingDataByteCount[type] ||
                        index>=(unsigned)world->things->thingCounts[type] || !world->things->rawThingData[type]) return -1;
                    if ((thing&0x3fffu)==(identity&0x3fffu)) ++owners;
                    raw=world->things->rawThingData[type]+index*s_thingDataByteCount[type];
                    thing=(unsigned short)(raw[0]|((unsigned)raw[1]<<8));
                }
            }
    return owners;
}
static int check_native_inventory(M11_GameViewState *view, uint8_t *frame) {
    static const M12_MenuInput keys[]={M12_MENU_INPUT_CHAMPION_2_INVENTORY,
        M12_MENU_INPUT_CHAMPION_3_INVENTORY,M12_MENU_INPUT_CHAMPION_4_INVENTORY};
    static const unsigned short things[]={0x1459,0xa802,0x2803};
    static const unsigned slots[]={CHAMPION_SLOT_ACTION_HAND,CHAMPION_SLOT_NECK,CHAMPION_SLOT_BACKPACK_1};
    static const int points[][2]={{70,94},{14,74},{74,74}}, weights[]={11,2,1};
    int leader=view->world.party.activeChampionIndex, result=0;
    REQUIRE(leader>=0 && leader<4 && !view->leaderHandObjectPresent,"original four-hero party has a leader and empty mouse hand");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_1_INVENTORY);
    REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==1 &&
        view->world.party.activeChampionIndex==leader,"native first champion inventory opens without leader reassignment");
    for (unsigned item=0u; item<3u; ++item) {
        int champion=(int)item+1; unsigned short loads[4], head=view->world.party.champions[champion].inventory[CHAMPION_SLOT_HEAD];
        struct ChampionLifecycleState_Compat before=view->world.lifecycle.champions[champion];
        unsigned charge=view->world.things->weapons[89].chargeCount;
        unsigned current_mana=view->world.party.champions[champion].mana.current;
        for (int c=0; c<4; ++c) loads[c]=view->world.party.champions[c].load;
        REQUIRE(view->world.party.champions[champion].inventory[slots[item]]==things[item] &&
            original_thing_owners(view,things[item])==1,"original equipment has exactly one source-owned location before inventory input");
        (void)M11_GameView_HandleInput(view,keys[item]);
        REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==champion+1 &&
            view->world.party.activeChampionIndex==leader,"native champion inventory key opens or switches owner without changing leader");
        { unsigned draws=view->dm1IigsDrawCount;
          M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
          REQUIRE(view->dm1IigsDrawCount==draws+1u,"actual native inventory draws through GS source route"); }
        REQUIRE(M11_GameView_HandlePointerButton(view,points[item][0],points[item][1],DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
            view->leaderHandObjectPresent && view->leaderHandThing==things[item] &&
            view->world.party.champions[champion].inventory[slots[item]]==THING_NONE &&
            original_thing_owners(view,things[item])==1,"native source slot click transfers exact original Thing to unique mouse-hand owner");
        REQUIRE(M11_GameView_HandlePointerMove(view,110,120)==M11_GAME_INPUT_REDRAW &&
            view->pointerPositionKnown && view->pointerX==110 && view->pointerY==120,
            "actual mouse-held native item movement publishes known source pointer coordinates");
        { uint8_t native_frame[M11_FB_WIDTH*M11_FB_HEIGHT]; unsigned draws=view->dm1IigsDrawCount;
          M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
          REQUIRE(view->dm1IigsDrawCount==draws+1u,"held original item is drawn by native inventory source renderer");
          memcpy(native_frame,frame,sizeof(native_frame));
          M11_GameView_DrawLeaderHandCursor(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
          REQUIRE(memcmp(native_frame,frame,sizeof(native_frame))==0,
              "generic PC cursor renderer cannot overdraw native GS mouse-held source icon"); }

        for (int c=0; c<4; ++c)
            REQUIRE(view->world.party.champions[c].load==(unsigned short)((int)loads[c]+(c==leader ? weights[item] : 0)-(c==champion ? weights[item] : 0)),
                "native object removal transfers original source weight from inventory owner to existing leader");
        if (item==0u) {
            const uint8_t *raw=view->world.things->rawThingData[THING_TYPE_WEAPON]+89u*4u;
            struct DungeonViewLight_Compat light;
            REQUIRE(!view->world.things->weapons[89].lit && !(raw[3]&0x80u) &&
                view->world.things->weapons[89].chargeCount==charge &&
                F0890b_ORCH_ComputeDungeonViewLight_Compat(&view->world,&light) && light.paletteIndex==5,
                "real torch removal clears typed/raw Lit without consuming fuel and removes held light");
            (void)M11_GameView_HandlePointerButton(view,42,66,DM1_V1_MOUSE_MASK_LEFT_PC34);
            REQUIRE(view->leaderHandObjectPresent && view->leaderHandThing==things[item] &&
                view->world.party.champions[champion].inventory[CHAMPION_SLOT_HEAD]==head &&
                view->world.party.champions[champion].inventory[slots[item]]==THING_NONE &&
                original_thing_owners(view,things[item])==1,"native AllowedSlots rejects original torch in occupied head slot without ownership mutation");
        } else if (item==1u) REQUIRE(view->world.party.champions[champion].mana.maximum==17u &&
            view->world.lifecycle.champions[champion].maxMana==17u,"actual Moonstone removal reverses native maximum-mana bonus");
        else for (unsigned row=0u; row<3u; ++row)
            REQUIRE(view->world.lifecycle.champions[champion].statistics[LIFECYCLE_STAT_LUCK][row]==
                (uint8_t)(before.statistics[LIFECYCLE_STAT_LUCK][row]-10u),"actual Rabbit's Foot pickup reverses all three native Luck modifier rows");
        REQUIRE(M11_GameView_HandlePointerButton(view,points[item][0],points[item][1],DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
            !view->leaderHandObjectPresent && view->world.party.champions[champion].inventory[slots[item]]==things[item] &&
            original_thing_owners(view,things[item])==1,"native source slot drop re-equips original Thing without duplication");
        for (int c=0; c<4; ++c) REQUIRE(view->world.party.champions[c].load==loads[c],"source item re-equipment restores every champion load");
        REQUIRE(view->world.party.champions[champion].mana.current==current_mana,
            "native remove/re-equip clicks preserve actual current mana while only maximum modifiers change");
        if (!item) {
            const uint8_t *raw=view->world.things->rawThingData[THING_TYPE_WEAPON]+89u*4u;
            REQUIRE(view->world.things->weapons[89].lit && (raw[3]&0x80u) &&
                view->world.things->weapons[89].chargeCount==charge,"actual native torch re-equipment restores typed/raw activation without fuel mutation");
        } else if (item==1u) REQUIRE(view->world.party.champions[champion].mana.maximum==20u &&
            view->world.lifecycle.champions[champion].maxMana==20u,"actual Moonstone re-equipment restores native maximum mana");
        else for (unsigned row=0u; row<3u; ++row)
            REQUIRE(view->world.lifecycle.champions[champion].statistics[LIFECYCLE_STAT_LUCK][row]==before.statistics[LIFECYCLE_STAT_LUCK][row],
                "actual Rabbit's Foot re-equipment restores each original modified Luck row");
        REQUIRE(M11_GameView_HandlePointerButton(view,214,40,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
            !view->inventoryPanelActive && !view->leaderHandObjectPresent && view->world.party.activeChampionIndex==leader,
            "native source close box returns to dungeon without changing leader or item ownership");
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        REQUIRE(original_thing_owners(view,things[item])==1 &&
            view->world.party.champions[2].mana.maximum==20u && view->world.lifecycle.champions[2].maxMana==20u &&
            view->world.lifecycle.champions[3].statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MAXIMUM]==35u,
            "ordinary native idle ticks preserve re-equipped source item ownership and committed modifiers");
    }
    { unsigned short loads[4];
      unsigned current_mana=view->world.party.champions[2].mana.current;
      uint8_t luck[3];
      for (int c=0; c<4; ++c) loads[c]=view->world.party.champions[c].load;
      memcpy(luck,view->world.lifecycle.champions[3].statistics[LIFECYCLE_STAT_LUCK],sizeof(luck));
      (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_3_INVENTORY);
      REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==3 &&
          M11_GameView_HandlePointerButton(view,14,74,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
          view->leaderHandObjectPresent && view->leaderHandThing==0xa802u &&
          view->world.party.champions[2].inventory[CHAMPION_SLOT_NECK]==THING_NONE &&
          view->world.party.champions[2].mana.maximum==17u && view->world.lifecycle.champions[2].maxMana==17u,
          "cross-champion swap begins with original CHANI Moonstone removal and maximum-mana inverse");
      (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_4_INVENTORY);
      REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==4 &&
          view->world.party.activeChampionIndex==leader && view->leaderHandThing==0xa802u,
          "native inventory owner switch retains existing leader and original mouse-held Moonstone");
      for (unsigned swap=0u; swap<2u; ++swap) {
          unsigned short cursor=swap ? 0xa802u : 0x2803u, backpack=swap ? 0x2803u : 0xa802u;
          REQUIRE(M11_GameView_HandlePointerButton(view,74,74,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
              view->leaderHandObjectPresent && view->leaderHandThing==cursor &&
              view->leaderHandIconIndex==M11_GameView_GetObjectIconIndexForThing(view,cursor) &&
              view->world.party.champions[3].inventory[CHAMPION_SLOT_BACKPACK_1]==backpack &&
              original_thing_owners(view,0xa802u)==1 && original_thing_owners(view,0x2803u)==1 &&
              view->world.party.champions[2].mana.maximum==17u && view->world.lifecycle.champions[2].maxMana==17u,
              "occupied native backpack click atomically swaps two authentic source Things without Moonstone neck bonus");
          for (unsigned row=0u; row<3u; ++row)
              REQUIRE(view->world.lifecycle.champions[3].statistics[LIFECYCLE_STAT_LUCK][row]==
                  (uint8_t)(luck[row]-(swap ? 0u : 10u)),
                  "native occupied swap removes and restores Rabbit's Foot Luck in source order");
          for (int c=0; c<4; ++c) {
              int delta=(c==2 ? -2 : 0)+(c==3 && !swap ? 1 : 0)+(c==leader ? (swap ? 2 : 1) : 0);
              REQUIRE(view->world.party.champions[c].load==(unsigned short)((int)loads[c]+delta),
                  "cross-champion occupied swap accounts original item weights in owner and leader mouse hand");
          }
          { unsigned draws=view->dm1IigsDrawCount;
            M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
            REQUIRE(view->dm1IigsDrawCount==draws+1u && view->dm1InventoryChampionOrdinal==4,
                "occupied native swap redraw retains actual inventory owner and native asset route"); }
      }
      (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_3_INVENTORY);
      REQUIRE(view->dm1InventoryChampionOrdinal==3 && view->world.party.activeChampionIndex==leader &&
          M11_GameView_HandlePointerButton(view,14,74,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
          !view->leaderHandObjectPresent && view->world.party.champions[2].inventory[CHAMPION_SLOT_NECK]==0xa802u &&
          view->world.party.champions[2].mana.maximum==20u && view->world.lifecycle.champions[2].maxMana==20u &&
          view->world.party.champions[2].mana.current==current_mana &&
          original_thing_owners(view,0xa802u)==1 && original_thing_owners(view,0x2803u)==1,
          "returning original Moonstone to CHANI neck restores maximum mana and empties unique mouse hand");
      for (int c=0; c<4; ++c) REQUIRE(view->world.party.champions[c].load==loads[c],
          "completed original occupied-swap round trip restores every champion load");
      REQUIRE(M11_GameView_HandlePointerButton(view,214,40,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
          !view->inventoryPanelActive,"native cross-champion inventory sequence closes through source close box");
    }
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_INVENTORY_TOGGLE);
    REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==leader+1,
        "native inventory toggle opens original leader inventory");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_INVENTORY_TOGGLE);
    REQUIRE(!view->inventoryPanelActive,"native repeated leader inventory toggle closes panel");
    result=1;
cleanup:
    return result;
}

static int check_linflas_capacity(const char *root, int version) {
    static const uint8_t route[][2]={{1,4},{1,5},{1,6},{1,7},{2,7},{3,7},{4,7},{4,6},{4,5},{4,4},
        {4,3},{4,2},{5,2},{6,2},{6,3},{7,3},{8,3},{8,4},{9,4},{9,5},{9,6},{9,7},{9,8},{9,9},{10,9},{11,9},{12,9}};
    M11_GameViewState *view=calloc(1,sizeof(*view));
    struct ChampionState_Compat *linflas; unsigned short load; int result=0;
    REQUIRE(view,"bounded original LINFLAS startup state"); M11_GameView_Init(view);
    REQUIRE(M11_GameView_StartIigs(view,root,version) && view->dm1IigsActive && !view->world.party.championCount,
        "separate authentic native entry leaves room for original LINFLAS recruitment");
    {
        DM1_V1_IigsDocSnapshot before, after;
        REQUIRE(M11_Audio_GetDm1IigsSnapshot(&view->audioState,&before) &&
            !view->audioState.originalSnd3Available && !view->audioState.originalSongAvailable &&
            !view->audioState.playedMarkerCount,"native startup owns DOC bank without PC audio or procedural playback");
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        for (unsigned tick=0u;tick<8u;++tick) (void)M11_GameView_AdvanceIdleTick(view);
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        REQUIRE(view->lastDm1V1MovementPipelineResult.core.movementBlocked &&
            view->world.party.mapX==1 && view->world.party.mapY==3 &&
            M11_Audio_GetDm1IigsSnapshot(&view->audioState,&after) && after.triggers==before.triggers &&
            !view->audioState.playedMarkerCount,
            "actual original empty-party HoC wall collision stays silent without a fabricated metallic cue");
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_LEFT);
    }
    for (unsigned step=0u; step<sizeof(route)/sizeof(route[0]); ++step) {
        int dx=(int)route[step][0]-view->world.party.mapX, dy=(int)route[step][1]-view->world.party.mapY;
        int direction=dx>0 ? 1 : dx<0 ? 3 : dy>0 ? 2 : 0;
        REQUIRE(abs(dx)+abs(dy)==1,"original LINFLAS route follows adjacent native corridor squares");
        for (unsigned turn=0u; view->world.party.direction!=direction && turn<4u; ++turn)
            (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        REQUIRE(view->world.party.mapX==route[step][0] && view->world.party.mapY==route[step][1],
            "ordinary native footsteps reach each authentic LINFLAS approach square");
    }
    for (unsigned turn=0u; view->world.party.direction!=0 && turn<4u; ++turn)
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
    REQUIRE(M11_GameView_GetFrontMirrorOrdinal(view)==12,"source mirror twelve identifies original LINFLAS");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACTION);
    linflas=&view->world.party.champions[0];
    REQUIRE(view->candidateMirrorPanelActive && view->candidateMirrorOrdinal==12 &&
        linflas->present && memcmp(linflas->name,"LINFLAS",7u)==0 && linflas->hp.maximum==65u &&
        linflas->stamina.current==500u && linflas->stamina.maximum==500u && linflas->mana.maximum==12u &&
        linflas->attributes[CHAMPION_ATTR_STRENGTH]==45u &&
        linflas->inventory[CHAMPION_SLOT_FEET]==0x9822u && linflas->maxLoad==490u,
        "authentic LINFLAS candidate computes F0309 capacity with original Elven Boots");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACCEPT);
    REQUIRE(!view->candidateMirrorPanelActive && view->world.party.championCount==1 && linflas->maxLoad==490u,
        "native original LINFLAS resurrection retains source boot-adjusted carrying capacity");
    load=linflas->load;
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_1_INVENTORY);
    REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==1 &&
        M11_GameView_HandlePointerButton(view,42,126,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        view->leaderHandObjectPresent && view->leaderHandThing==0x9822u &&
        linflas->inventory[CHAMPION_SLOT_FEET]==THING_NONE && linflas->maxLoad==460u && linflas->load==load &&
        linflas->attributes[CHAMPION_ATTR_STRENGTH]==45u && linflas->stamina.current==500u &&
        original_thing_owners(view,0x9822u)==1,
        "native original boots pickup removes F0309 bonus while leader-held object retains weight and unique ownership");
    REQUIRE(M11_GameView_HandlePointerButton(view,42,126,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        !view->leaderHandObjectPresent && linflas->inventory[CHAMPION_SLOT_FEET]==0x9822u &&
        linflas->maxLoad==490u && linflas->load==load && original_thing_owners(view,0x9822u)==1,
        "native same-slot Elven Boots re-equipment restores original carrying-capacity bonus once");
    REQUIRE(M11_GameView_HandlePointerButton(view,214,40,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        !view->inventoryPanelActive,"original LINFLAS inventory closes through native source box");
    for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
    REQUIRE(linflas->maxLoad==490u && linflas->inventory[CHAMPION_SLOT_FEET]==0x9822u,
        "ordinary native idle synchronization preserves source Elven Boots capacity");
    result=1;
cleanup:
    if (view) M11_GameView_Shutdown(view);
    free(view); return result;
}

static int check_original_food(const char *root, int version) {
    M11_GameViewState *view=calloc(1,sizeof(*view));
    uint8_t *frame=malloc((size_t)M11_FB_WIDTH*M11_FB_HEIGHT);
    int result=0, leader; unsigned short leader_load; int target_food, leader_food; uint32_t world_tick;
    REQUIRE(view && frame,"bounded original native food session"); M11_GameView_Init(view);
    REQUIRE(M11_GameView_StartIigs(view,root,version) && check_recruitment(view,frame),
        "fresh authentic ELIJA leader reaches original neighboring SYRA food mirror");
    leader=view->world.party.activeChampionIndex;
    REQUIRE(leader==0,"original ELIJA remains leader for separate food recipient");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_1_INVENTORY);
    REQUIRE(view->inventoryPanelActive &&
        M11_GameView_HandlePointerButton(view,74,74,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        view->leaderHandThing==0x6800u && view->leaderHandObjectPresent,"original ELIJA Blue Box is held for native nonfood rejection");
    { int food=view->world.party.champions[0].food; unsigned short load=view->world.party.champions[0].load;
      uint32_t tick=view->world.gameTick;
      (void)M11_GameView_HandlePointerButton(view,64,54,DM1_V1_MOUSE_MASK_LEFT_PC34);
      REQUIRE(!view->v1FoodCommandPending && view->leaderHandObjectPresent && view->leaderHandThing==0x6800u &&
          view->world.party.champions[0].food==food && view->world.party.champions[0].load==load &&
          view->world.gameTick==tick && original_thing_owners(view,0x6800u)==1,
          "native mouth rejects authentic Blue Box without state, ownership or clock mutation"); }
    (void)M11_GameView_HandlePointerButton(view,74,74,DM1_V1_MOUSE_MASK_LEFT_PC34);
    REQUIRE(!view->leaderHandObjectPresent,"original nonfood returns to its source inventory slot");
    (void)M11_GameView_HandlePointerButton(view,214,40,DM1_V1_MOUSE_MASK_LEFT_PC34);
    for (unsigned turn=0u; view->world.party.direction!=2 && turn<4u; ++turn)
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
    for (int y=8; y<=9; ++y) {
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        REQUIRE(view->world.party.mapX==9 && view->world.party.mapY==y,"ordinary native movement reaches original SYRA mirror");
    }
    REQUIRE(M11_GameView_GetFrontMirrorOrdinal(view)==2,"source wall resolves original SYRA food owner");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACTION);
    REQUIRE(view->candidateMirrorPanelActive && view->candidateMirrorOrdinal==2,"native action selects original SYRA");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_ACCEPT);
    REQUIRE(view->world.party.championCount==2 && view->world.party.activeChampionIndex==leader &&
        memcmp(view->world.party.champions[1].name,"SYRA",4u)==0 &&
        view->world.party.champions[1].inventory[CHAMPION_SLOT_BACKPACK_1]==0x28b4u,
        "original SYRA owns authentic Apple while original ELIJA remains leader");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_2_INVENTORY);
    REQUIRE(view->inventoryPanelActive && view->dm1InventoryChampionOrdinal==2 &&
        M11_GameView_HandlePointerButton(view,74,74,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        view->leaderHandObjectPresent && view->leaderHandThing==0x28b4u && original_thing_owners(view,0x28b4u)==1,
        "native SYRA backpack source click transfers original Apple to separate leader mouse hand");
    target_food=view->world.party.champions[1].food; leader_food=view->world.party.champions[leader].food;
    leader_load=view->world.party.champions[leader].load; world_tick=view->world.gameTick;
    REQUIRE(M11_GameView_HandlePointerButton(view,64,54,DM1_V1_MOUSE_MASK_LEFT_PC34)==M11_GAME_INPUT_REDRAW &&
        !view->leaderHandObjectPresent && original_thing_owners(view,0x28b4u)==0 &&
        view->world.things->rawThingData[THING_TYPE_JUNK][180u*4u]==0xffu &&
        view->world.things->rawThingData[THING_TYPE_JUNK][180u*4u+1u]==0xffu &&
        view->world.things->junks[180].next==THING_NONE &&
        view->world.party.champions[1].food==(target_food+500>2048 ? 2048 : target_food+500) &&
        view->world.lifecycle.champions[1].food==view->world.party.champions[1].food &&
        view->world.party.champions[leader].food==leader_food &&
        view->world.party.champions[leader].load==leader_load-4u && view->world.gameTick==world_tick,
        "native source mouth consumes original Apple for selected recipient with exact food, free record and separate leader weight");
    { static const int icons[]={206,205,206,205};
      DM1_V1_IigsDocSnapshot audio_before, audio_after;
      REQUIRE(M11_Audio_GetDm1IigsSnapshot(&view->audioState,&audio_before),"native food begins with source DOC ownership");
      unsigned completion=view->v1FoodCompletionCount, edges=0u;
      int x=view->world.party.mapX, y=view->world.party.mapY;
      REQUIRE(view->v1FoodCommandPending && view->v1FoodAwaitingPresentation &&
          !view->v1FoodPaletteWaitPending && view->v1MouthAnimationFrameIndex==0 &&
          view->v1MouthVisualIconIndex==206 && view->v1MouthAnimationDelayRemaining==8,
          "native first mouth frame awaits presentation immediately without PC palette-wait edge");
      (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_CHAMPION_1_INVENTORY);
      (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
      (void)M11_GameView_AdvanceIdleTick(view);
      REQUIRE(view->dm1InventoryChampionOrdinal==2 && view->world.party.mapX==x && view->world.party.mapY==y &&
          view->world.gameTick==world_tick && view->v1FoodCommandPending,
          "pending native consumption blocks inventory switch, movement and world idle advancement");
      for (unsigned index=0u; index<4u; ++index) {
          unsigned draws=view->dm1IigsDrawCount; uint64_t serial=view->v1FoodPresentationSerial;
          REQUIRE(view->v1FoodAwaitingPresentation && !view->v1FoodPaletteWaitPending &&
              view->v1MouthAnimationFrameIndex==(int)index && view->v1MouthVisualIconIndex==icons[index] &&
              view->v1MouthAnimationDelayRemaining==8,"native consumption publishes each original alternating mouth frame");
          M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
          REQUIRE(view->dm1IigsDrawCount==draws+1u,"actual native Draw presents source mouth animation frame");
          (void)M11_GameView_AdvanceFoodSourceVblank(view);
          REQUIRE(view->v1FoodAwaitingPresentation && view->v1MouthAnimationDelayRemaining==8 &&
              view->world.gameTick==world_tick,"unacknowledged mouth frame cannot spend native delay edges");
          REQUIRE(M11_GameView_AcknowledgeFoodPresentation(view,serial),"native mouth frame presentation is explicitly acknowledged");
          for (unsigned edge=1u; edge<=8u; ++edge) {
              if (version==21 && edge==8u) {
                  REQUIRE(view->v1FoodVblankHzNumerator==60u && view->v1FoodVblankHzDenominator==1u,
                      "native host food clock explicitly selects nominal NTSC 60 Hz profile");
                  (void)M11_GameView_AdvanceFoodClockMs(view,17u);
              } else (void)M11_GameView_AdvanceFoodSourceVblank(view);
              ++edges;
              if (edge<8u) REQUIRE(view->v1FoodCommandPending && !view->v1FoodAwaitingPresentation &&
                  view->v1MouthAnimationFrameIndex==(int)index && view->v1MouthAnimationDelayRemaining==(int)(8u-edge),
                  "each native mouth frame retains exactly eight source delay edges");
          }
      }
      REQUIRE(M11_Audio_GetDm1IigsSnapshot(&view->audioState,&audio_after) &&
          audio_after.triggers==audio_before.triggers+1u && audio_after.sound_index[audio_before.next_pair]==8 &&
          !view->audioState.playedMarkerCount,"native food completion requests original swallow once after final source wait");
      REQUIRE(edges==32u && !view->v1FoodCommandPending && !view->v1FoodAwaitingPresentation &&
          view->v1FoodCompletionCount==completion+1u && view->world.gameTick==world_tick &&
          view->world.party.champions[1].food==(target_food+500>2048 ? 2048 : target_food+500) &&
          view->world.party.champions[leader].food==leader_food,
          "native food completes after four times eight edges with one completion and no world tick or recipient drift");
    }
    result=1;
cleanup:
    if (view) M11_GameView_Shutdown(view);
    free(frame); free(view); return result;
}

static int check_native_wall_bump(M11_GameViewState *view) {
    DM1_V1_IigsDocSnapshot audio_before,audio_after;
    struct GameWorld_Compat *staged=calloc(1,sizeof(*staged));
    DM1IigsWallBumpResult_Compat receipt;
    unsigned hp, pending; uint32_t expected_seed; int cloned=0, result=0;
    REQUIRE(view->world.party.mapIndex==0 && view->world.party.mapX==9 && view->world.party.mapY==7 &&
        view->world.party.direction==3 && view->world.party.championCount==1 &&
        memcmp(view->world.party.champions[0].name,"ELIJA",5u)==0,
        "native living bump starts at original ELIJA mirror approach without relocating champion");
    for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
    hp=view->world.party.champions[0].hp.current;
    REQUIRE(!view->world.pendingChampionCombat[0].damageApplied &&
        M11_Audio_GetDm1IigsSnapshot(&view->audioState,&audio_before),"native living bump starts without staged damage and with actual DOC owner");
    REQUIRE(staged,"bounded copy of actual source ELIJA world for native staging boundary");
    cloned=F0880b_WORLD_Clone_Compat(&view->world,staged);
    REQUIRE(cloned && !view->world.party.champions[0].wounds,
        "original unwounded ELIJA source state supplies independent native bump contract");
    expected_seed=view->world.masterRng.seed;
    /* Native CHAMPION.C F0286/F0228: one draw per axial selector,
     * including second selector even when it finds the same sole champion.
     * F0321/F0313: torso then legs each draw RANDOM((36>>3)+1),
     * then RANDOM128 for wounds; attack1 cannot cross its >=10 threshold.
     * No existing wounds add RANDOM4 and no duplicate second F0321 runs. */
    for (unsigned draw=0u; draw<5u; ++draw)
        expected_seed=expected_seed*UINT32_C(0xBB40E62D)+UINT32_C(11);
    REQUIRE(F0321G_WORLD_StageIigsWallBump_Compat(staged,8,7,1u,2u,&receipt) &&
        receipt.championIndex[0]==0 && receipt.championIndex[1]==0 &&
        receipt.damage[0]>0 && !receipt.damage[1] && !receipt.woundMask[0] && !receipt.woundMask[1] &&
        receipt.totalDamage==receipt.damage[0] && receipt.damagePositive && receipt.rngCallCount==5 &&
        staged->masterRng.seed==expected_seed && staged->party.champions[0].hp.current==hp &&
        staged->pendingChampionCombat[0].damageApplied==receipt.totalDamage,
        "native WORLD operation preserves original first-hit/second-selector RNG ordering and stages without applying HP");
    (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
    pending=(unsigned)view->world.pendingChampionCombat[0].damageApplied;
    REQUIRE(view->lastDm1V1MovementPipelineResult.core.movementBlocked &&
        view->lastDm1V1MovementPipelineResult.core.blockedByWallOrDoorDamageRequested &&
        view->lastDm1V1MovementPipelineResult.core.blockedByWallOrDoorDamageAttack==1 &&
        view->lastDm1V1MovementPipelineResult.core.blockedByWallOrDoorDamageAttackTypeSelf==2 &&
        view->lastDm1V1MovementPipelineResult.core.blockedByWallOrDoorDamageAllowedWounds==0x18u &&
        view->world.party.mapIndex==0 && view->world.party.mapX==9 && view->world.party.mapY==7 &&
        view->world.party.direction==3,"ordinary native forward input blocks at original mirror wall and requests source self attack");
    REQUIRE(pending==(unsigned)receipt.totalDamage && view->world.masterRng.seed==expected_seed &&
        pending>0u && pending<hp && view->world.party.champions[0].hp.current==hp &&
        view->world.pendingChampionCombatTargetReceipt[0]!=0,
        "native wall bump stages original F0321 damage without subtracting champion HP before F0320");
    REQUIRE(M11_Audio_GetDm1IigsSnapshot(&view->audioState,&audio_after) &&
        audio_after.triggers==audio_before.triggers+1u && audio_after.latest_pair[18]>=0 &&
        audio_after.left[audio_after.latest_pair[18]]==225u && audio_after.right[audio_after.latest_pair[18]]==225u &&
        !view->audioState.playedMarkerCount,
        "positive native wall damage plays original DOC18 at blocked wall eight-seven with front-one-square attenuation");
    (void)M11_GameView_AdvanceIdleTick(view);
    REQUIRE(view->world.party.champions[0].hp.current==hp-pending &&
        !view->world.pendingChampionCombat[0].damageApplied && !view->world.pendingChampionCombatTargetReceipt[0] &&
        view->world.party.mapX==9 && view->world.party.mapY==7,
        "next ordinary native tick applies staged F0320 damage once and clears pending owner");
    result=1;
cleanup:
    if (cloned) F0883_WORLD_Free_Compat(staged);
    free(staged); return result;
}

static int check_session(const char *root, int version, const char *owner) {
    M11_GameViewState *view=calloc(1,sizeof(*view));
    uint8_t *frame=malloc((size_t)M11_FB_WIDTH*M11_FB_HEIGHT);
    char expected[ASSET_PATH_MAX]; int result=0;
    REQUIRE(view && frame,"bounded M11 native startup state");
    M11_GameView_Init(view);
    for (unsigned restart=0u; restart<2u; ++restart) {
        REQUIRE(M11_GameView_StartIigs(view,root,version),"actual M11 admits authenticated same-owner native IIGS bundle");
        REQUIRE(view->active && view->dm1IigsActive && view->assetsAvailable &&
            view->sourceKind==M11_GAME_SOURCE_BUILTIN_CATALOG && strcmp(view->sourceId,"dm1")==0 &&
            view->assetLoader.iigsDm1 && !view->assetLoader.legacyDm1 &&
            view->originalFontAvailable && view->originalFont.loaded && view->originalFont.iigsNative &&
            view->world.iigsCreatureProfileBound && view->world.dungeon && view->world.things &&
            view->world.party.mapIndex==0 && view->world.party.mapX==1 && view->world.party.mapY==3 &&
            view->world.party.direction==2 && view->world.party.championCount==0 &&
            (int)view->dm1IigsInstallation.metadata.program_version==version,
            "M11 binds native graphics/font/world/profile without PC or Towns identity");
        REQUIRE(snprintf(expected,sizeof(expected),"%s::DUNGEON.GAME",owner)<(int)sizeof(expected) &&
            strcmp(view->dungeonPath,expected)==0 &&
            strcmp(view->dm1IigsInstallation.dungeon_path,expected)==0,
            "actual M11 world retains exact selected original disk owner");
        REQUIRE(snprintf(expected,sizeof(expected),"%s::DUNGEON.MASTER",owner)<(int)sizeof(expected) &&
            strcmp(view->dm1IigsInstallation.program_path,expected)==0 &&
            asset_file_matches_md5(expected,version==20 ? "51b9d22c7418651ee49ad77f37c80a3c" :
                "eccc49e7b29ee233cfb10615e8ec86f3"),"startup retains exact original program owner");
        REQUIRE(snprintf(expected,sizeof(expected),"%s::SYSTEM/START",owner)<(int)sizeof(expected) &&
            strcmp(view->dm1IigsInstallation.start_path,expected)==0 &&
            asset_file_matches_md5(expected,"3bcae225346ac24fb1975aa6d93f0fa6"),"startup retains exact original SYSTEM/START owner");
        REQUIRE(snprintf(expected,sizeof(expected),"%s::GRAPHICS.GAME",owner)<(int)sizeof(expected) &&
            strcmp(view->assetLoader.graphicsDatPath,expected)==0 &&
            asset_memory_matches_md5(view->assetLoader.iigsData,(size_t)view->assetLoader.iigsDataSize,
                "1045a36952e64eb2bc7b4c7b2965b112") &&
            asset_memory_matches_md5(view->originalFont.bitmap,sizeof(view->originalFont.bitmap),
                "c2e4a7671e51524d85e25430be5ba818"),
            "M11 resident graphics and native font retain unchanged original bytes");
        { const M11_AssetSlot *slot=M11_AssetLoader_Load(&view->assetLoader,21u);
          REQUIRE(slot && slot->width==16u && slot->height==17u &&
              !M11_AssetLoader_Load(&view->assetLoader,12u),"actual startup selects native raster dispatch including index21"); }
        REQUIRE(check_original_hand_light(view),"original native hand light admission semantics");
        REQUIRE(check_original_scroll_opening(view),"actual original scroll source-defined opening icon");
        for (int i=0; i<27; ++i) {
            struct DM1CreatureInfo_Compat row;
            REQUIRE(F0890d_ORCH_GetCreatureInfoForWorld_Compat(&view->world,i,&row) &&
                row.attributes==view->dm1IigsInstallation.metadata.creature_info[i].attributes &&
                row.graphicInfo==view->dm1IigsInstallation.metadata.creature_info[i].graphic_info &&
                row.movementTicks==view->dm1IigsInstallation.metadata.creature_info[i].movement_ticks,
                "actual M11 world uses source-owned native creature profiles");
        }
        { M11_BootProbeReceipt receipt;
          memset(&receipt,0xa5,sizeof(receipt));
          REQUIRE(M11_GameView_GetBootProbeReceipt(view,&receipt) && receipt.levelLoaded &&
              strcmp(receipt.startupPhase,"dm1-iigs-runtime")==0 &&
              strcmp(receipt.startupAnimation,"none")==0 && !receipt.startupActive &&
              !receipt.startupAnimationActive && !receipt.startupTitleFrame &&
              !receipt.startupTitleFrameMax && !receipt.startupTitleReady,
              "native boot receipt reports actual first world without invented PC title animation"); }
        memset(frame,0xa5,(size_t)M11_FB_WIDTH*M11_FB_HEIGHT);
        { unsigned draws=view->dm1IigsDrawCount;
          M11_GameView_Draw(view,frame,M11_FB_WIDTH,M11_FB_HEIGHT);
          REQUIRE(view->dm1IigsDrawCount==draws+1u,
              "actual Draw dispatch publishes native IIGS frame instead of PC renderer fallback"); }
        /* Rotation is possible with the original empty HoC party. A turn
         * and its inverse exercise normal key dispatch without inventing a champion. */
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_RIGHT);
        REQUIRE(view->world.party.direction==3,"ordinary M11 right-turn input reaches native world");
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_TURN_LEFT);
        REQUIRE(view->world.party.direction==2,"ordinary inverse turn restores original native facing");
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        REQUIRE(view->active && view->dm1IigsActive && view->world.iigsCreatureProfileBound,
            "ordinary idle updates retain native session ownership");
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_UP);
        REQUIRE(view->world.party.mapIndex==0 && view->world.party.mapX==1 &&
            view->world.party.mapY==4 && view->world.party.direction==2,
            "ordinary forward input walks from authentic HoC entry to native square one-four");
        for (unsigned tick=0u; tick<8u; ++tick) (void)M11_GameView_AdvanceIdleTick(view);
        (void)M11_GameView_HandleInput(view,M12_MENU_INPUT_DOWN);
        REQUIRE(view->world.party.mapIndex==0 && view->world.party.mapX==1 &&
            view->world.party.mapY==3 && view->world.party.direction==2,
            "ordinary backward input returns to authentic HoC entry after native movement cooldown");
        REQUIRE(check_recruitment(view,frame),"authentic native HoC champion recruitment through ordinary input");
        REQUIRE(check_native_wall_bump(view),"authentic native wall collision damage and source sound");
        REQUIRE(check_equipped_heroes(view,frame),"authentic native equipment modifier and activation lifecycle");
        REQUIRE(check_first_descent(view,frame),"actual native first descent and functional lighting");
        REQUIRE(check_iigs_creature_combat(view),"authentic IIGS monster approaches and damages the naturally routed party");
        REQUIRE(check_native_inventory(view,frame),"actual native inventory source transactions and modifier inverse");
        M11_GameView_Shutdown(view); M11_GameView_Shutdown(view);
        REQUIRE(!view->active && !view->dm1IigsActive && !view->world.iigsCreatureProfileBound &&
            !view->assetLoader.iigsDm1,"repeated shutdown clears native runtime resources and profile");
    }
    result=1;
cleanup:
    if (view) M11_GameView_Shutdown(view);
    free(frame); free(view); return result;
}
static uint8_t *root_entry(uint8_t *disk, const char *name) {
    for (unsigned slot=1u; slot<13u; ++slot) {
        uint8_t *entry=disk+1028u+39u*slot;
        if ((entry[0]&15u)==strlen(name) && memcmp(entry+1u,name,strlen(name))==0) return entry;
    }
    return NULL;
}
static int check_rejections(const char *original, const char *scratch) {
    M11_GameViewState *view=calloc(1,sizeof(*view));
    uint8_t *disk=NULL; size_t size=0u; char path[ASSET_PATH_MAX]; FILE *file=NULL;
    int result=0;
    REQUIRE(view,"bounded rejected session state"); M11_GameView_Init(view);
    REQUIRE(!M11_GameView_StartIigs(view,original,21) && !view->active && !view->dm1IigsActive,
        "explicit G20 selected owner cannot borrow G21 from a valid neighboring disk");
    REQUIRE(asset_read_path_alloc(original,&disk,&size) && size==819200u &&
        asset_memory_matches_md5(disk,size,"15e3f954f408612fd2f15a474a182f9f"),"negative starts from authentic original disk");
    { uint8_t *entry=root_entry(disk,"DUNGEON.MASTER");
      REQUIRE(entry,"locate original program directory entry"); entry[1]='X'; }
    REQUIRE(snprintf(path,sizeof(path),"%s/missing-program.po",scratch)<(int)sizeof(path),"bounded negative disk path");
    file=fopen(path,"wb"); REQUIRE(file && fwrite(disk,1,size,file)==size,"write private original-derived missing-program disk");
    { int closed=fclose(file); file=NULL; REQUIRE(!closed,"close private original-derived disk"); }
    REQUIRE(!M11_GameView_StartIigs(view,path,20) && !view->active && !view->dm1IigsActive &&
        !view->assetsAvailable && !view->world.iigsCreatureProfileBound,
        "explicit incomplete owner rejects without borrowing authentic neighboring graphics/program/world");
    REQUIRE(check_scan(path,20,1),"incomplete owner rejects through both public scanner APIs");
    result=1;
cleanup:
    if (file) fclose(file);
    if (view) { M11_GameView_Shutdown(view); M11_GameView_Shutdown(view); }
    free(disk); free(view); return result;
}
int main(int argc, char **argv) {
    char owner[ASSET_PATH_MAX];
    if (argc!=5) { fprintf(stderr,"usage: %s <G20.po> <G21.po> <software.7z> <private-directory>\n",argv[0]); return 2; }
    for (int i=0; i<2; ++i) {
        int version=20+i;
        if (!check_scan(argv[1+i],version,0) || !check_session(argv[1+i],version,argv[1+i])) return 1;
        if (!check_linflas_capacity(argv[1+i],version)) return 1;
        if (!check_original_food(argv[1+i],version)) return 1;
        if (snprintf(owner,sizeof(owner),"%s::Dungeon Master for Apple IIGS 2.%d (English) Original (Not working).po",argv[3],i)>=(int)sizeof(owner) ||
            !check_session(argv[3],version,owner)) return 1;
    }
    if (!check_archive_scan(argv[3]) || !check_rejections(argv[1],argv[4]) ||
        !asset_file_matches_md5(argv[1],"15e3f954f408612fd2f15a474a182f9f") ||
        !asset_file_matches_md5(argv[2],"d38f00e5ad8516553c5cba3214b29bef")) return 1;
    puts("PASS: authentic IIGS M11 native startup, HoC monster combat, ownership, input and restart lifecycle; visual/audio parity unverified");
    return 0;
}
