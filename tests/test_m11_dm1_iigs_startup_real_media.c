/* Actual M11 native IIGS startup; source ownership and lifecycle, not visual parity. */
#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
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
        if (snprintf(owner,sizeof(owner),"%s::Dungeon Master for Apple IIGS 2.%d (English) Original (Not working).po",argv[3],i)>=(int)sizeof(owner) ||
            !check_session(argv[3],version,owner)) return 1;
    }
    if (!check_archive_scan(argv[3]) || !check_rejections(argv[1],argv[4]) ||
        !asset_file_matches_md5(argv[1],"15e3f954f408612fd2f15a474a182f9f") ||
        !asset_file_matches_md5(argv[2],"d38f00e5ad8516553c5cba3214b29bef")) return 1;
    puts("PASS: authentic IIGS M11 native startup, ownership, input and restart lifecycle; visual/audio parity unverified");
    return 0;
}
