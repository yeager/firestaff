#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "asset_status_m12.h"
#include "asset_find_by_hash.h"
#include "dm1_v1_fmtowns_startup.h"
#include "fs_portable_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;
#define REQUIRE(c,m) do { if (!(c)) { fprintf(stderr,"FAIL: %s\n",m); goto cleanup; } } while (0)

#ifndef _WIN32
static int menu_reports_missing_companion(M12_StartupMenuState* menu,
                                           const char* selectedGraphics,
                                           const M12_StartupMenuInitOptions* options,
                                           int architecture, const char* missing) {
    M12_StartupMenu_Destroy(menu);
    M12_StartupMenu_InitWithOptions(menu,selectedGraphics,"dm1",options);
    menu->view=M12_MENU_VIEW_MAIN;
    menu->selectedIndex=0;
    menu->gameOptions[0].architectureIndex=architecture;
    M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
    return menu->messageIsMissingGameData && !menu->launchRequested &&
        menu->messageLine2 && strstr(menu->messageLine2,missing) &&
        !strstr(menu->messageLine2,"GRAPHICS.DAT");
}
#endif

#ifndef _WIN32
static int write_original_bytes(const char* path, const unsigned char* bytes, size_t size) {
    FILE* file=fopen(path,"wb");
    int ok;
    if (!file) return 0;
    ok=fwrite(bytes,1,size,file)==size;
    if (fclose(file)!=0) ok=0;
    return ok;
}
/* Stage the unchanged original disk, rather than rebuilding an ADF from
 * members: explicit source ownership must survive both inventory APIs. */
static int direct_adf_preserves_selected_owner(M12_AssetStatus* status,
    M11_GameViewState* game, const char* root, const char* originalArchive,
    const char* neighborArchive, const char* originalVideo) {
    char dir[FSP_PATH_MAX]={0}, selected[FSP_PATH_MAX]={0};
    char neighbor[FSP_PATH_MAX]={0}, duplicate[FSP_PATH_MAX]={0};
    char unsupported[FSP_PATH_MAX]={0}, physical[FSP_PATH_MAX], member[ASSET_PATH_MAX];
    unsigned char* bytes=NULL;
    size_t size=0;
    int result=1;
    const char* graphicsMd5="7f9458e4a3972d06e649a6fa85a7f34b";
    M11_GameLaunchSpec spec;
    M11_BootProbeReceipt receipt;
    REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,"direct-adf-owner.adf") && FSP_CreateDirectory(dir),
        "create isolated direct ADF owner fixture");
    REQUIRE(snprintf(member,sizeof(member),"%s::Dungeon Master Amiga 3.6.adf",originalArchive)<(int)sizeof(member) &&
        asset_read_path_alloc(member,&bytes,&size) && size==901120U &&
        asset_memory_matches_md5(bytes,size,"3ff177c27abe1a15ba36946669b2e512"),
        "authenticate complete unchanged original Amiga 3.6 disk");
    REQUIRE(FSP_JoinPath(selected,sizeof(selected),dir,"selected.ADF") &&
        write_original_bytes(selected,bytes,size),"stage unchanged selected original ADF");
    free(bytes); bytes=NULL;
    REQUIRE(FSP_JoinPath(neighbor,sizeof(neighbor),dir,"Dungeon-Master_Amiga_EN_Version-20.zip") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),neighborArchive) && symlink(physical,neighbor)==0,
        "stage complete original different-edition neighbor");
    REQUIRE(FSP_JoinPath(duplicate,sizeof(duplicate),dir,"000-neighbor.adf") &&
        asset_read_path_alloc(selected,&bytes,&size) &&
        asset_memory_matches_md5(bytes,size,"3ff177c27abe1a15ba36946669b2e512") &&
        write_original_bytes(duplicate,bytes,size),
        "stage independent unchanged same-edition disk before selected image");
    free(bytes); bytes=NULL;
    REQUIRE(FSP_JoinPath(unsupported,sizeof(unsupported),dir,"unsupported.ADF") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),originalVideo) && symlink(physical,unsupported)==0,
        "stage unchanged original video payload with unsupported disk extension");
    for (int mode=0; mode<2; ++mode) {
        const M12_AssetVersionStatus* owner=NULL;
        int graphicsFound=0, worldFound=0;
        size_t prefix=strlen(selected);
        if (mode) M12_AssetStatus_Scan(status,selected);
        else M12_AssetStatus_ScanGame(status,selected,"dm1");
        for (size_t version=0; version<M12_AssetStatus_GetVersionCount("dm1"); ++version) {
            const M12_AssetVersionStatus* candidate=M12_AssetStatus_GetVersion(status,"dm1",version);
            if (candidate && candidate->matched) {
                REQUIRE(strcmp(candidate->matchedMd5,graphicsMd5)==0 &&
                    strncmp(candidate->matchedPath,selected,prefix)==0 &&
                    strncmp(candidate->matchedPath+prefix,"::",2)==0,
                    "every admitted edition belongs to explicitly selected ADF");
                owner=candidate;
            }
        }
        REQUIRE(owner && M12_AssetStatus_GameAvailable(status,"dm1"),
            "selected original ADF stays available beside complete same/different-edition neighbors");
        for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
            const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
            const char* path=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
            if (strcmp(required->roleId,"graphics")==0)
                graphicsFound=required->matched && strcmp(required->matchedHash,graphicsMd5)==0 &&
                    strncmp(path,selected,prefix)==0 && strncmp(path+prefix,"::",2)==0;
            if (strcmp(required->roleId,"dungeon")==0)
                worldFound=required->matched && strcmp(required->matchedHash,"30028fb6a301ecb20127ef0b3af32b05")==0 &&
                    strncmp(path,selected,prefix)==0 && strncmp(path+prefix,"::",2)==0;
        }
        REQUIRE(graphicsFound && worldFound,"required-file receipts retain selected disk provenance");
        memset(&spec,0,sizeof(spec));
        spec.title="DUNGEON MASTER"; spec.gameId=spec.sourceId="dm1";
        spec.dataDir=selected; spec.verifiedAssetPath=owner->matchedPath; spec.verifiedAssetMd5=graphicsMd5;
        spec.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
        spec.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
        spec.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        REQUIRE(M11_GameView_Start(game,&spec) && game->active && game->assetLoader.dm1Amiga36 &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "explicit selected ADF starts original Amiga 3.6 world");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        for (int language=0; language<2; ++language) {
            const char* hash=language==0 ? "c0e1bf1b9c5578681879d624bb28e663" : "59e332cf47852d5c7c88a9538f0adcea";
            spec.languageIndex=language==0 ? 2 : 3;
            REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
                M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded &&
                asset_file_matches_md5(game->dungeonPath,hash) &&
                strncmp(game->dungeonPath,selected,prefix)==0,
                "direct original ADF starts its chosen French/German world");
            M11_GameView_Shutdown(game); M11_GameView_Init(game);
        }
        if (mode) M12_AssetStatus_Scan(status,unsupported);
        else M12_AssetStatus_ScanGame(status,unsupported,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
            "unsupported selected disk cannot borrow complete neighboring originals");
    }
    for (int mode=0; mode<2; ++mode) {
        if (mode) M12_AssetStatus_Scan(status,dir);
        else M12_AssetStatus_ScanGame(status,dir,"dm1");
        REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1"),
            "directory with ADF suffix retains ordinary collection discovery");
    }
    REQUIRE(asset_file_matches_md5(selected,"3ff177c27abe1a15ba36946669b2e512"),
        "original selected disk remains unchanged after scans and runtime starts");
    result=0;
cleanup:
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    free(bytes);
    if (unsupported[0]) unlink(unsupported);
    if (duplicate[0]) unlink(duplicate);
    if (neighbor[0]) unlink(neighbor);
    if (selected[0]) unlink(selected);
    if (dir[0]) rmdir(dir);
    return result==0;
}
static int amiga36_language_menu_worlds(M11_GameViewState* game,
    const char* dir, const char* originalArchive) {
    static const char* const names[]={"DUNGEONF.DAT","DUNGEONG.DAT"};
    static const char* const hashes[]={"c0e1bf1b9c5578681879d624bb28e663",
        "59e332cf47852d5c7c88a9538f0adcea"};
    static const size_t sizes[]={25414U,25380U};
    char files[2][FSP_PATH_MAX]={{0}}, member[ASSET_PATH_MAX], english[FSP_PATH_MAX];
    char graphics[FSP_PATH_MAX];
    unsigned char* bytes[2]={NULL,NULL};
    unsigned char* standard=NULL;
    size_t size[2]={0,0}, standardSize=0;
    M12_StartupMenuState* menu=NULL;
    M12_StartupMenuInitOptions options;
    M11_BootProbeReceipt receipt;
    int result=1, initialized=0;
    REQUIRE(FSP_JoinPath(graphics,sizeof(graphics),dir,"GRAPHICS.DAT") &&
        FSP_JoinPath(english,sizeof(english),dir,"DUNGEON.DAT") &&
        asset_read_path_alloc(english,&standard,&standardSize) && standardSize==25006U &&
        asset_memory_matches_md5(standard,standardSize,"30028fb6a301ecb20127ef0b3af32b05"),
        "retain authenticated original English A36 dungeon for wrong-language cases");
    for (int language=0; language<2; ++language) {
        REQUIRE(asset_find_by_md5(originalArchive,hashes[language],member,sizeof(member),0) &&
            asset_read_path_alloc(member,&bytes[language],&size[language]) && size[language]==sizes[language] &&
            asset_memory_matches_md5(bytes[language],size[language],hashes[language]) &&
            FSP_JoinPath(files[language],sizeof(files[language]),dir,names[language]) &&
            write_original_bytes(files[language],bytes[language],size[language]),
            "stage unchanged authenticated compressed A36 French/German originals");
    }
    menu=SDL_calloc(1,sizeof(*menu));
    REQUIRE(menu,"allocate A36 language launcher");
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    for (int source=0; source<2; ++source) {
        if (initialized) M12_StartupMenu_Destroy(menu);
        M12_StartupMenu_InitWithOptions(menu,source ? originalArchive : dir,"dm1",&options);
        initialized=1;
        menu->selectedIndex=menu->activatedIndex=0; menu->launchRequested=1;
        menu->gameOptions[0].architectureIndex=M12_ARCH_AMIGA;
        menu->gameOptions[0].versionIndex=M12_AssetStatus_FindVersionIndex("dm1","amiga36-multi");
        menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
        menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
        for (int language=0; language<2; ++language) {
            menu->gameOptions[0].languageIndex=language==0 ? 2 : 3;
            REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                game->assetLoader.dm1Amiga36 && M11_GameView_GetBootProbeReceipt(game,&receipt) &&
                receipt.levelLoaded && asset_file_matches_md5(game->dungeonPath,hashes[language]) &&
                (source ? strncmp(game->dungeonPath,originalArchive,strlen(originalArchive))==0 :
                    strcmp(game->dungeonPath,files[language])==0),
                "loose/ZIP A36 menu starts the exact chosen original language world");
            M11_GameView_Shutdown(game); M11_GameView_Init(game);
            if (!source) {
                M11_GameLaunchSpec direct;
                memset(&direct,0,sizeof(direct));
                direct.title="DUNGEON MASTER"; direct.gameId=direct.sourceId="dm1";
                direct.dataDir=dir; direct.verifiedAssetPath=graphics;
                direct.verifiedAssetMd5="7f9458e4a3972d06e649a6fa85a7f34b";
                direct.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
                direct.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
                direct.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
                direct.languageIndex=language==0 ? 2 : 3;
                REQUIRE(write_original_bytes(files[language],standard,standardSize),
                    "replace only staged language world with unchanged English original");
                REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
                    "A36 chosen-language identity rejects wrong original under expected filename");
                REQUIRE(!M11_GameView_Start(game,&direct) && !game->active,
                    "direct runtime independently rejects wrong chosen-language original");
                REQUIRE(unlink(files[language])==0,"remove only staged A36 language world");
                REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
                    "missing A36 language world cannot borrow English or source archive");
                REQUIRE(!M11_GameView_Start(game,&direct) && !game->active,
                    "direct runtime independently rejects missing chosen-language original");
                menu->view=M12_MENU_VIEW_MAIN; menu->selectedIndex=0; menu->launchRequested=0;
                for (int input=0; input<3 && !menu->messageIsMissingGameData; ++input)
                    M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
                REQUIRE(menu->messageIsMissingGameData && !menu->launchRequested &&
                    menu->messageLine2 && strstr(menu->messageLine2,names[language]) &&
                    !strstr(menu->messageLine2,"GRAPHICS.DAT"),
                    "A36 ordinary menu reports the missing selected-language original");
                M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_BACK);
                REQUIRE(menu->view==M12_MENU_VIEW_GAME_OPTIONS &&
                    menu->gameOptions[0].languageIndex==direct.languageIndex,
                    "A36 missing-language dialog returns with chosen language intact");
                menu->launchRequested=1;
                REQUIRE(write_original_bytes(files[language],bytes[language],size[language]),
                    "restore unchanged original A36 chosen-language world");
                REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                    M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
                    "restored original A36 language world recovers runtime startup");
                M11_GameView_Shutdown(game); M11_GameView_Init(game);
            }
        }
    }
    result=0;
cleanup:
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    if (initialized) M12_StartupMenu_Destroy(menu);
    SDL_free(menu); free(standard);
    for (int language=0; language<2; ++language) {
        free(bytes[language]);
        if (files[language][0]) unlink(files[language]);
    }
    return result==0;
}
static int native_report_has_graphics_without_world(const M12_AssetStatus* status,
    const char* graphics, const char* graphicsMd5) {
    int graphicsReported=0, dungeonMissing=0;
    for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
        const char* path=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
        if (strcmp(required->roleId,"graphics")==0)
            graphicsReported=required->matched && strcmp(path,graphics)==0 &&
                strcmp(required->matchedHash,graphicsMd5)==0;
        if (strcmp(required->roleId,"dungeon")==0) dungeonMissing=!required->matched;
    }
    return graphicsReported && dungeonMissing;
}
static int native_rejects_pc_towns_dungeons(M12_AssetStatus* status, M11_GameViewState* game,
    const char* dir, const char* graphics, const char* dungeon,
    const char* pcOriginal, const char* townsOriginal, M12_StartupMenuState* menu,
    const M12_StartupMenuInitOptions* options, int architecture,
    const char* startupName, const char* startupMd5) {
    static const char* const foreignHashes[]={
        "766450c940651fc021c92fe5d0d0b3a6", "3dc0a932d0e0adfe59878f07c51700c5",
        "fe098f70ce83cfe3f2333565093daf35"};
    unsigned char *own=NULL, *foreign=NULL;
    size_t ownSize=0, foreignSize=0;
    char member[ASSET_PATH_MAX], graphicsMd5[33];
    M11_GameLaunchSpec spec;
    M11_BootProbeReceipt receipt;
    int result=1, replaced=0;
    REQUIRE(asset_read_path_alloc(dungeon,&own,&ownSize) &&
        asset_file_md5_hex(graphics,graphicsMd5),"retain unchanged native original dungeon bytes");
    memset(&spec,0,sizeof(spec));
    spec.title="DUNGEON MASTER"; spec.gameId=spec.sourceId="dm1";
    spec.dataDir=dir; spec.verifiedAssetPath=graphics; spec.verifiedAssetMd5=graphicsMd5;
    spec.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
    spec.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
    spec.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
        M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
        "original native companion loads before incompatible replacement");
    if (startupName) {
        char swoosh[ASSET_PATH_MAX];
        unsigned char* startup=NULL; size_t startupSize=0;
        int restored=0;
        REQUIRE(FSP_JoinPath(swoosh,sizeof(swoosh),dir,startupName) &&
            asset_read_path_alloc(swoosh,&startup,&startupSize) &&
            asset_memory_matches_md5(startup,startupSize,startupMd5),
            "retain authenticated original native startup companion before incomplete-owner diagnostic");
        REQUIRE(unlink(swoosh)==0,"remove only staged original native startup companion");
        /* Restore before assertions so failure cannot leave staged original
         * startup material behind or affect later source-owner checks. */
        for (int mode=0; mode<2; ++mode) {
            int graphicsFound=0, worldFound=0, startupMissing=0, blocked, menuCorrect;
            if (mode) M12_AssetStatus_Scan(status,graphics);
            else M12_AssetStatus_ScanGame(status,graphics,"dm1");
            blocked=!M12_AssetStatus_GameAvailable(status,"dm1");
            for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
                const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
                const char* path=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                if (strcmp(required->roleId,"graphics")==0)
                    graphicsFound=required->matched && strcmp(path,graphics)==0;
                if (strcmp(required->roleId,"dungeon")==0)
                    worldFound=required->matched && strcmp(path,dungeon)==0;
                if (required->label && strcmp(required->label,startupName)==0)
                    startupMissing=required->required && !required->matched;
            }
            menuCorrect=menu_reports_missing_companion(menu,graphics,options,architecture,startupName) &&
                !strstr(menu->messageLine2,"DUNGEON.DAT");
            if (!blocked || !graphicsFound || !worldFound || !startupMissing || !menuCorrect) {
                restored=write_original_bytes(swoosh,startup,startupSize);
                free(startup);
                REQUIRE(restored,"restore staged native startup companion after failed diagnostic assertion");
                REQUIRE(0,"missing native startup companion is reported separately from verified graphics and world");
            }
        }
        restored=write_original_bytes(swoosh,startup,startupSize);
        free(startup);
        REQUIRE(restored,"restore unchanged staged original native startup companion");
    }
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    replaced=1;
    REQUIRE(unlink(dungeon)==0,"remove only staged native dungeon for exact missing-role feedback");
    for (int mode=0; mode<2; ++mode) {
        if (mode) M12_AssetStatus_Scan(status,graphics);
        else M12_AssetStatus_ScanGame(status,graphics,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),"missing native dungeon blocks selected owner");
        REQUIRE(native_report_has_graphics_without_world(status,graphics,graphicsMd5),
            "selected original native graphics remains verified when only its dungeon is missing");
        REQUIRE(menu_reports_missing_companion(menu,graphics,options,architecture,"DUNGEON.DAT"),
            "native menu reports missing dungeon without claiming verified graphics are missing");
    }
    REQUIRE(!M11_GameView_Start(game,&spec) && !game->active,
        "verified graphics report does not permit runtime startup without its own dungeon");
    REQUIRE(write_original_bytes(dungeon,own,ownSize),"restore staged native dungeon after missing-role feedback");
    replaced=0;
    for (int original=0; original<3; ++original) {
        if (original==0) {
            if (!pcOriginal || !FSP_FileExists(pcOriginal)) {
                puts("SKIP: authentic PC original unavailable for native compatibility case");
                continue;
            }
            REQUIRE(asset_find_by_md5(pcOriginal,foreignHashes[original],member,sizeof(member),0),
                "locate authentic incompatible PC dungeon");
        } else REQUIRE(FSP_JoinPath(member,sizeof(member),townsOriginal,
                original==1 ? "DATA/DUNGEON.DAT" : "JDATA/DUNGEON.DAT"),
                "locate authentic incompatible Towns dungeon");
        REQUIRE(asset_read_path_alloc(member,&foreign,&foreignSize) &&
            asset_memory_matches_md5(foreign,foreignSize,foreignHashes[original]),
            "authenticate unchanged incompatible PC or Towns original dungeon bytes");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        replaced=1;
        REQUIRE(write_original_bytes(dungeon,foreign,foreignSize),
            "stage unchanged PC or Towns dungeon beside native graphics");
        for (int mode=0; mode<2; ++mode) {
            if (mode) M12_AssetStatus_Scan(status,graphics);
            else M12_AssetStatus_ScanGame(status,graphics,"dm1");
            if (M12_AssetStatus_GameAvailable(status,"dm1"))
                fprintf(stderr,"incompatible original admitted: graphics=%s dungeon=%s scan=%d\n",
                    graphicsMd5,foreignHashes[original],mode);
            REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
                "both scan APIs reject PC or Towns dungeon paired with native graphics");
            REQUIRE(native_report_has_graphics_without_world(status,graphics,graphicsMd5),
                "incompatible original dungeon is not reported as a verified native world");
            REQUIRE(menu_reports_missing_companion(menu,graphics,options,architecture,"DUNGEON.DAT"),
                "native menu identifies incompatible world instead of generic verified-data failure");
        }
        REQUIRE(!M11_GameView_Start(game,&spec) && !game->active,
            "native runtime rejects replaced PC or Towns dungeon with stale original graphics identity");
        REQUIRE(write_original_bytes(dungeon,own,ownSize),"restore task-owned native dungeon bytes");
        replaced=0;
        for (int mode=0; mode<2; ++mode) {
            if (mode) M12_AssetStatus_Scan(status,graphics);
            else M12_AssetStatus_ScanGame(status,graphics,"dm1");
            REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1"),"both scan APIs recover restored native original");
        }
        REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "restored original native dungeon remains launchable");
        free(foreign); foreign=NULL;
    }
    result=0;
cleanup:
    if (replaced && own) (void)write_original_bytes(dungeon,own,ownSize);
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    free(own); free(foreign);
    return result==0;
}
#endif

int main(void) {
#ifdef _WIN32
    puts("SKIP: native FM Towns source staging requires symlinks");
    return 77;
#else
    /* Link only source files consumed by the native startup receipt and the
     * game. DATA itself is a real task-owned directory, so canonicalizing the
     * selected directory cannot escape to the original installation. */
    static const char* const files[]={
        "AUTOEXEC.BAT", "EDM.EXP", "TMENU.EXP", "TMENU.ICN", "TMENU.INF",
        "DATA/GRAPHICS.DAT", "DATA/DUNGEON.DAT",
        "JDM.EXP", "JDATA/GRAPHICS.DAT", "JDATA/DUNGEON.DAT"
    };
    const char* original=getenv("FIRESTAFF_DM1_FMTOWNS_LOOSE_ROOT");
    const char* scratch=getenv("TMPDIR");
    const char* pcOriginal=getenv("FIRESTAFF_DM1_PC_ARCHIVE");
    const char* atariOriginal=getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
    const char* amigaOriginal=getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    const char* amiga36Original=getenv("FIRESTAFF_DM1_AMIGA_V36_ARCHIVE");
    char nativeDir[FSP_PATH_MAX]={0}, nativeFiles[3][FSP_PATH_MAX]={{0}};
    unsigned char* nativeBytes=NULL;
    size_t nativeSize=0;
    FILE* nativeOutput=NULL;
    char root[FSP_PATH_MAX]={0}, data[FSP_PATH_MAX]={0};
    char config[FSP_PATH_MAX]={0}, menuProgram[FSP_PATH_MAX]={0};
    char japaneseData[FSP_PATH_MAX]={0}, englishProgram[FSP_PATH_MAX]={0}, pcAlias[FSP_PATH_MAX]={0};
    char source[FSP_PATH_MAX], destination[FSP_PATH_MAX], physical[FSP_PATH_MAX];
    char duplicate[FSP_PATH_MAX]={0}, duplicateData[FSP_PATH_MAX]={0};
    size_t duplicateLinked=0;
    M12_StartupMenuState* menu=NULL;
    M11_GameViewState* game=NULL;
    M12_AssetStatus* fresh=NULL;
    M12_StartupMenuInitOptions options;
    M12_LaunchIntent intent;
    M11_BootProbeReceipt receipt;
    const M12_AssetVersionStatus* version;
    size_t i, linked=0;
    int created=0, initialized=0, index, japaneseIndex, result=1;
    if (!original || !FSP_DirExists(original)) {
        puts("SKIP: FIRESTAFF_DM1_FMTOWNS_LOOSE_ROOT authentic installation required");
        return 77;
    }
    for (i=0; i<sizeof(files)/sizeof(files[0]); ++i) {
        if (!FSP_JoinPath(source,sizeof(source),original,files[i]) || !FSP_FileExists(source)) {
            puts("SKIP: authentic FM Towns native startup files are incomplete");
            return 77;
        }
    }
    if (!scratch || !scratch[0]) scratch="build";
    REQUIRE(FSP_CreateDirectoryRecursive(scratch) &&
        snprintf(root,sizeof(root),"%s/dm1-towns-late-rejection-XXXXXX",scratch)<(int)sizeof(root) &&
        mkdtemp(root),"create isolated task directory");
    created=1;
    REQUIRE(FSP_ResolvePhysicalPath(physical,sizeof(physical),root),"resolve task directory");
    snprintf(root,sizeof(root),"%s",physical);
    REQUIRE(FSP_JoinPath(data,sizeof(data),root,"DATA") && FSP_CreateDirectory(data) &&
        FSP_JoinPath(japaneseData,sizeof(japaneseData),root,"JDATA") && FSP_CreateDirectory(japaneseData) &&
        FSP_JoinPath(englishProgram,sizeof(englishProgram),root,"EDM.EXP") &&
        FSP_JoinPath(config,sizeof(config),root,"menu.toml") &&
        FSP_JoinPath(menuProgram,sizeof(menuProgram),root,"TMENU.EXP"),"construct isolated paths");
    for (i=0; i<sizeof(files)/sizeof(files[0]); ++i) {
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,files[i]) &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
            FSP_JoinPath(destination,sizeof(destination),root,files[i]) &&
            symlink(physical,destination)==0,"link unchanged original source file");
        ++linked;
    }
    REQUIRE(SDL_setenv_unsafe("FIRESTAFF_CONFIG_PATH",config,1)==0 &&
        SDL_setenv_unsafe("SDL_VIDEODRIVER","dummy",1)==0 &&
        SDL_setenv_unsafe("SDL_AUDIODRIVER","dummy",1)==0 && SDL_Init(0),
        "isolate host configuration and initialize SDL");
    menu=SDL_calloc(1,sizeof(*menu)); game=SDL_calloc(1,sizeof(*game));
    REQUIRE(menu && game,"allocate real launcher and runtime");
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    M12_StartupMenu_InitWithOptions(menu,root,"dm1",&options);
    index=M12_AssetStatus_FindVersionIndex("dm1","fmtowns-en");
    version=index>=0 ? M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index) : NULL;
    REQUIRE(version && version->matched && strncmp(version->matchedPath,data,strlen(data))==0,
        "real scan admits staged authentic FM Towns English graphics");
    menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
    menu->gameOptions[0].architectureIndex=M12_ARCH_FM_TOWNS;
    menu->gameOptions[0].versionIndex=index;
    menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
    menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
    intent=M12_StartupMenu_GetLaunchIntent(menu);
    REQUIRE(intent.valid && intent.versionId && strcmp(intent.versionId,"fmtowns-en")==0,
        "authenticated native edition has valid selected launch intent");
    M11_GameView_Init(game); initialized=1;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu),"complete authentic native startup opens");
    REQUIRE(M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded && game->active &&
        game->dm1FmtownsStartupReceiptValid &&
        dm1_v1_fmtowns_startup_receipt_has_native_owners(&game->dm1FmtownsStartupReceipt) &&
        strcmp(game->dm1FmtownsStartupReceipt.game_program_name,"EDM.EXP")==0,
        "positive startup binds original EDM and TMENU with loaded world");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    japaneseIndex=M12_AssetStatus_FindVersionIndex("dm1","fmtowns-ja");
    REQUIRE(japaneseIndex>=0,"Japanese native edition exists");
    menu->gameOptions[0].versionIndex=japaneseIndex;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) &&
        M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded && game->active &&
        game->dm1FmtownsStartupReceiptValid &&
        strcmp(game->dm1FmtownsStartupReceipt.game_program_name,"JDM.EXP")==0,
        "complete authentic Japanese loose owner starts native world");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    menu->gameOptions[0].versionIndex=index;
    /* Remove only the task-owned link after scanning. Source graphics and
     * dungeon remain valid, so failure occurs at native startup admission. */
    REQUIRE(unlink(menuProgram)==0,"remove staged TMENU link without modifying original");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu),"missing native menu rejects stale selected startup");
    REQUIRE(!game->active,"native startup rejection leaves no running partial world");
    /* A later inventory must reject the incomplete native package before
     * offering it to the player, not merely fail its eventual handoff. */
    fresh=SDL_calloc(1,sizeof(*fresh));
    REQUIRE(fresh,"allocate independent fresh inventory");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index);
    REQUIRE(version && !version->matched && !M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "fresh scan does not advertise incomplete native FM Towns startup");
    japaneseIndex=M12_AssetStatus_FindVersionIndex("dm1","fmtowns-ja");
    version=japaneseIndex>=0 ? M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex) : NULL;
    REQUIRE(version && !version->matched,"missing common TMENU rejects Japanese native startup too");
    {
        int nativeMissing=0;
        for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
            const M12_AssetRequiredFileStatus* required=
                M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
            if (required && required->roleId && strcmp(required->roleId,"native-launcher")==0) {
                nativeMissing=required->required && !required->matched && required->label &&
                    strstr(required->label,"TMENU.EXP")!=NULL;
            }
        }
        REQUIRE(nativeMissing,"incomplete native package reports missing launcher filenames");
    }
    if (pcOriginal && FSP_FileExists(pcOriginal)) {
        int pcIndex=M12_AssetStatus_FindVersionIndex("dm1","pc34-en");
        REQUIRE(FSP_JoinPath(pcAlias,sizeof(pcAlias),root,"pc-original.zip") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),pcOriginal) &&
            symlink(physical,pcAlias)==0,"stage authentic PC sibling beside incomplete native Towns");
        M12_AssetStatus_ScanGame(fresh,root,"dm1");
        version=pcIndex>=0 ? M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)pcIndex) : NULL;
        REQUIRE(version && version->matched && M12_AssetStatus_GameAvailable(fresh,"dm1"),
            "incomplete Towns launcher does not hide authenticated PC sibling");
        REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched &&
            !M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex)->matched,
            "PC availability does not restore rejected native Towns editions");
        REQUIRE(unlink(pcAlias)==0,"remove only task-owned PC archive link");
        pcAlias[0]='\0';
    } else {
        puts("SKIP: authentic PC archive unavailable for mixed-platform preservation case");
    }
    REQUIRE(FSP_JoinPath(source,sizeof(source),original,"TMENU.EXP") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
        symlink(physical,menuProgram)==0,"restore only task-owned original menu link");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index);
    REQUIRE(version && version->matched && M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "fresh scan admits restored authentic native FM Towns startup");
    version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex);
    REQUIRE(version && version->matched,"restored common menu admits Japanese startup too");
    for (int sibling=0; sibling<4; ++sibling) {
        const char* siblingOriginal=sibling==0 ? pcOriginal : sibling==1 ? atariOriginal : sibling==2 ? amigaOriginal : amiga36Original;
        char japaneseProgram[FSP_PATH_MAX];
        int phase;
        size_t nativeCount=sibling==1 ? 2U : 3U;
        if (!siblingOriginal || !FSP_FileExists(siblingOriginal)) {
            printf("SKIP: authentic %s archive unavailable for mixed-owner report case\n",
                sibling==0 ? "PC" : sibling==1 ? "Atari ST" : sibling==2 ? "Amiga 2.0" : "Amiga 3.6");
            continue;
        }
        if (sibling!=0) {
            static const char* const nativeHashes[3][3]={
                {"9ce2eaf7a9e78620e3f17594437caffa", "050fb2cfded1b502ec2c53956b94c5bd", NULL},
                {"6a2f135b53c2220f0251fa103e2a6e7e", "30028fb6a301ecb20127ef0b3af32b05",
                 "a0ffbcc7ae8cecac03128ddb32887ef4"},
                {"7f9458e4a3972d06e649a6fa85a7f34b", "30028fb6a301ecb20127ef0b3af32b05",
                 "d86f045085481448f6ae3db8d3d640d2"}};
            /* Zero-size expectations still require the exact complete-file
             * MD5 below and the existing bounded original-media reader. */
            static const size_t nativeSizes[3][3]={{271918U,33442U,0U},{411960U,25006U,78480U},{0U,25006U,0U}};
            static const char* const names[]={"GRAPHICS.DAT","DUNGEON.DAT","swoosh"};
            const char* const* hashes=nativeHashes[sibling-1];
            const size_t* sizes=nativeSizes[sibling-1];
            REQUIRE(FSP_JoinPath(nativeDir,sizeof(nativeDir),root,sibling==1 ? "st12-original" : sibling==2 ? "a20-original" : "a36-original") &&
                FSP_CreateDirectory(nativeDir),"create isolated original native owner");
            for (i=0; i<nativeCount; ++i) {
                if (sibling==2) {
                    static const char* const amigaMembers[]={"graphics.dat","Dungeon.dat","swoosh"};
                    /* The original A20 package is ZIP -> ZIP -> ADF. Use
                     * its native bounded member reader; the generic hash
                     * walker is tested separately for renamed archives. */
                    REQUIRE(snprintf(source,sizeof(source),
                        "%s::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf::%s",
                        siblingOriginal,amigaMembers[i])<(int)sizeof(source),
                        "locate original nested Amiga member");
                } else REQUIRE(asset_find_by_md5(siblingOriginal,hashes[i],source,sizeof(source),0),
                    "locate original native member by identity");
                if (!asset_read_path_alloc(source,&nativeBytes,&nativeSize) ||
                    (sizes[i] && nativeSize!=sizes[i]) || !asset_memory_matches_md5(nativeBytes,nativeSize,hashes[i])) {
                    fprintf(stderr,"original member rejected: sibling=%d member=%zu path=%s bytes=%zu expected=%zu\n",
                        sibling,i,source,nativeSize,sizes[i]);
                    REQUIRE(0,"read bounded authentic native original member");
                }
                REQUIRE(FSP_JoinPath(nativeFiles[i],sizeof(nativeFiles[i]),nativeDir,sibling==3 && i==2 ? "VDEO.FTL" : names[i]) &&
                    (nativeOutput=fopen(nativeFiles[i],"wb"))!=NULL,
                    "create task-owned native member copy");
                REQUIRE(fwrite(nativeBytes,1,nativeSize,nativeOutput)==nativeSize,
                    "stage unchanged original native member bytes");
                {
                    int closeResult=fclose(nativeOutput);
                    nativeOutput=NULL;
                    REQUIRE(closeResult==0,"close original native member copy");
                }
                free(nativeBytes); nativeBytes=NULL;
                REQUIRE(asset_file_matches_md5(nativeFiles[i],hashes[i]),
                    "authenticate unchanged staged native member bytes");
            }
            REQUIRE(native_rejects_pc_towns_dungeons(fresh,game,nativeDir,nativeFiles[0],nativeFiles[1],
                pcOriginal,original,menu,&options,sibling==1 ? M12_ARCH_ATARI_ST : M12_ARCH_AMIGA,
                sibling==1 ? NULL : sibling==2 ? "swoosh" : "VDEO.FTL",
                sibling==1 ? NULL : hashes[2]),
                "native original companion compatibility and recovery");
            if (sibling==3)
                REQUIRE(amiga36_language_menu_worlds(game,nativeDir,siblingOriginal),
                    "A36 original language ownership through loose/archive menu startup");
            if (sibling==3 && amigaOriginal && FSP_FileExists(amigaOriginal))
                REQUIRE(direct_adf_preserves_selected_owner(fresh,game,root,siblingOriginal,
                    amigaOriginal,nativeFiles[2]),"explicit ADF source ownership and runtime recovery");
        } else REQUIRE(FSP_JoinPath(pcAlias,sizeof(pcAlias),root,"pc-original.zip") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),pcOriginal) &&
            symlink(physical,pcAlias)==0,"stage original PC archive for mixed-owner report");
        REQUIRE(FSP_JoinPath(japaneseProgram,sizeof(japaneseProgram),root,"JDM.EXP"),
            "locate task-owned Japanese launcher link");
        for (phase=0; phase<3; ++phase) {
            const M12_AssetVersionStatus* owner;
            int graphicsReported=0, dungeonReported=0, scanMode;
            if (phase==1) REQUIRE(unlink(englishProgram)==0,"reject English owner for report fallback");
            if (phase==2) REQUIRE(unlink(japaneseProgram)==0,"reject Japanese owner for sibling report fallback");
            for (scanMode=0; scanMode<2; ++scanMode) {
                graphicsReported=dungeonReported=0;
                if (scanMode==0) M12_AssetStatus_ScanGame(fresh,root,"dm1");
                else M12_AssetStatus_Scan(fresh,root);
                owner=M12_AssetStatus_GetFirstMatchedVersion(fresh,"dm1");
                REQUIRE(M12_AssetStatus_GameAvailable(fresh,"dm1") && owner &&
                    strcmp(owner->versionId,phase==0 ? "fmtowns-en" :
                    phase==1 ? "fmtowns-ja" : sibling==0 ? "pc34-en" : sibling==1 ? "st12-en" : sibling==2 ? "amiga20-en" : "amiga36-multi")==0,"report follows first surviving original edition");
                for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
                    const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
                    const char* reported=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                    if (strcmp(required->roleId,"graphics")==0)
                        graphicsReported=required->matched && strcmp(reported,owner->matchedPath)==0 &&
                            strcmp(required->matchedHash,owner->matchedMd5)==0;
                    if (strcmp(required->roleId,"dungeon")==0) {
                        if (phase<2) {
                            REQUIRE(FSP_JoinPath(destination,sizeof(destination),phase==0 ? data : japaneseData,
                                "DUNGEON.DAT"),"locate expected reported Towns dungeon");
                            dungeonReported=required->matched && strcmp(reported,destination)==0 &&
                                strcmp(required->matchedHash,phase==0 ? "3dc0a932d0e0adfe59878f07c51700c5" :
                                    "fe098f70ce83cfe3f2333565093daf35")==0;
                        } else if (sibling==0) {
                            dungeonReported=required->matched && strncmp(reported,pcAlias,strlen(pcAlias))==0 &&
                                strcmp(required->matchedHash,"766450c940651fc021c92fe5d0d0b3a6")==0;
                        } else {
                            dungeonReported=required->matched && strcmp(reported,nativeFiles[1])==0 &&
                                strcmp(required->matchedHash,sibling==1 ? "050fb2cfded1b502ec2c53956b94c5bd" :
                                    "30028fb6a301ecb20127ef0b3af32b05")==0;
                        }
                    }
                }
                if (!graphicsReported || !dungeonReported) {
                    fprintf(stderr,"report phase=%d scan=%d owner=%s graphics=%d dungeon=%d\n",
                        phase,scanMode,owner->versionId,graphicsReported,dungeonReported);
                    for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
                        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
                        fprintf(stderr,"report role=%s matched=%d path=%s source=%s md5=%s\n",
                            required->roleId,required->matched,required->matchedPath,required->sourcePath,
                            required->matchedHash);
                    }
                }
                REQUIRE(graphicsReported && dungeonReported,"required-file report retains surviving owner's original provenance");
            }
        }
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,"EDM.EXP") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) && symlink(physical,englishProgram)==0 &&
            FSP_JoinPath(source,sizeof(source),original,"JDM.EXP") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) && symlink(physical,japaneseProgram)==0 &&
            (sibling!=0 || unlink(pcAlias)==0),"restore only staged source links after report cases");
        pcAlias[0]='\0';
        if (sibling!=0) {
            for (i=0; i<nativeCount; ++i) {
                REQUIRE(unlink(nativeFiles[i])==0,"remove only task-owned native member copy");
                nativeFiles[i][0]='\0';
            }
            REQUIRE(rmdir(nativeDir)==0,"remove isolated native owner directory");
            nativeDir[0]='\0';
        }
    }
    REQUIRE(unlink(englishProgram)==0,"remove only staged English program link");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched &&
        M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex)->matched &&
        M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "missing EDM rejects English while preserving original Japanese startup");
    for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
        REQUIRE(!required || !required->roleId || strcmp(required->roleId,"native-launcher")!=0 ||
            !required->required || required->matched,
            "ready Japanese edition is not blocked by English native launcher requirement");
    }
    REQUIRE(FSP_JoinPath(source,sizeof(source),original,"EDM.EXP") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),source) && symlink(physical,englishProgram)==0,
        "restore staged original English program link");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    REQUIRE(M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched,
        "restoring EDM admits English native startup again");
    REQUIRE(FSP_JoinPath(destination,sizeof(destination),data,"DUNGEON.DAT") &&
        unlink(destination)==0,"remove only staged English dungeon");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched &&
        M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex)->matched &&
        M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "Japanese sibling dungeon does not admit English owner missing its dungeon");
    {
        char japaneseDungeon[FSP_PATH_MAX];
        REQUIRE(FSP_JoinPath(japaneseDungeon,sizeof(japaneseDungeon),japaneseData,"DUNGEON.DAT") &&
            unlink(japaneseDungeon)==0,"remove second language dungeon task link");
        M12_AssetStatus_ScanGame(fresh,root,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(fresh,"dm1"),
            "complete native programs cannot admit owners with both dungeons missing");
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,"JDATA/DUNGEON.DAT") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
            symlink(physical,japaneseDungeon)==0,"restore original Japanese dungeon link");
    }
    REQUIRE(FSP_JoinPath(source,sizeof(source),original,"JDATA/DUNGEON.DAT") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
        symlink(physical,destination)==0,"stage authentic Japanese dungeon under English owner");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched,
        "wrong authentic language dungeon does not admit English owner");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
        "stale English menu rejects consumed wrong-language dungeon bytes");
    REQUIRE(unlink(destination)==0 &&
        FSP_JoinPath(source,sizeof(source),original,"DATA/DUNGEON.DAT") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
        symlink(physical,destination)==0,"restore unchanged English dungeon link");
    /* A complete second installation must survive an earlier same-edition
     * graphics/dungeon pair whose native launcher is incomplete. */
    REQUIRE(FSP_JoinPath(duplicate,sizeof(duplicate),data,"zz-complete") &&
        FSP_CreateDirectory(duplicate) &&
        FSP_JoinPath(duplicateData,sizeof(duplicateData),duplicate,"DATA") &&
        FSP_CreateDirectory(duplicateData),"create second native owner directories");
    for (i=0; i<7; ++i) {
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,files[i]) &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
            FSP_JoinPath(destination,sizeof(destination),duplicate,files[i]) &&
            symlink(physical,destination)==0,"stage complete second original owner");
        ++duplicateLinked;
    }
    {
        char selectedGraphics[FSP_PATH_MAX];
        REQUIRE(FSP_JoinPath(selectedGraphics,sizeof(selectedGraphics),data,"GRAPHICS.DAT"),
            "locate explicitly selected native graphics");
        M12_AssetStatus_ScanGame(fresh,selectedGraphics,"dm1");
        version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index);
        REQUIRE(version && version->matched && strcmp(version->matchedPath,selectedGraphics)==0,
            "explicit complete native graphics retain exact owner beside duplicate");
        M12_StartupMenu_Destroy(menu);
        M12_StartupMenu_InitWithOptions(menu,selectedGraphics,"dm1",&options);
        menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
        menu->gameOptions[0].architectureIndex=M12_ARCH_FM_TOWNS;
        menu->gameOptions[0].versionIndex=index;
        menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
        menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
        REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded && game->active,
            "explicit complete native graphics start their selected original world");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        REQUIRE(FSP_JoinPath(destination,sizeof(destination),data,"DUNGEON.DAT") &&
            unlink(destination)==0,"remove explicit owner's dungeon");
        REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
            "stale explicit native owner cannot fall back to complete duplicate");
        M12_AssetStatus_ScanGame(fresh,selectedGraphics,"dm1");
        REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched &&
            !M12_AssetStatus_GameAvailable(fresh,"dm1"),
            "explicit incomplete native graphics cannot select complete duplicate");
        {
            int scanMode, wrongLanguage;
            for (wrongLanguage=0; wrongLanguage<2; ++wrongLanguage) {
                if (wrongLanguage) REQUIRE(FSP_JoinPath(source,sizeof(source),original,"JDATA/DUNGEON.DAT") &&
                    FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
                    symlink(physical,destination)==0,"stage wrong original language dungeon for explicit diagnostics");
                for (scanMode=0; scanMode<2; ++scanMode) {
                    int graphicsFound=0, dungeonMissing=0;
                    if (scanMode==0) M12_AssetStatus_ScanGame(fresh,selectedGraphics,"dm1");
                    else M12_AssetStatus_Scan(fresh,selectedGraphics);
                    REQUIRE(!M12_AssetStatus_GameAvailable(fresh,"dm1") &&
                        !M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched,
                        "explicit missing-dungeon diagnostics never admit another source");
                    for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
                        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
                        if (strcmp(required->roleId,"graphics")==0)
                            graphicsFound=required->matched && strcmp(required->matchedPath,selectedGraphics)==0;
                        if (strcmp(required->roleId,"dungeon")==0)
                            dungeonMissing=required->required && !required->matched;
                    }
                    REQUIRE(graphicsFound && dungeonMissing,
                        "explicit incomplete source reports found graphics and missing dungeon");
                }
                REQUIRE(menu_reports_missing_companion(menu,selectedGraphics,&options,M12_ARCH_FM_TOWNS,"DUNGEON.DAT"),
                    "normal menu input reports missing dungeon without claiming graphics are missing");
            }
            REQUIRE(unlink(destination)==0,"remove only staged wrong-language dungeon link");
        }
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,"DATA/DUNGEON.DAT") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
            symlink(physical,destination)==0,"restore explicit owner's original dungeon");
        REQUIRE(unlink(menuProgram)==0,"remove explicit owner's native menu program");
        {
            int scanMode;
            for (scanMode=0; scanMode<2; ++scanMode) {
                int graphicsFound=0, dungeonFound=0, launcherMissing=0;
                if (scanMode==0) M12_AssetStatus_ScanGame(fresh,selectedGraphics,"dm1");
                else M12_AssetStatus_Scan(fresh,selectedGraphics);
                REQUIRE(!M12_AssetStatus_GameAvailable(fresh,"dm1") &&
                    !M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched,
                    "explicit missing-program diagnostics never admit duplicate installation");
                for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
                    const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
                    if (strcmp(required->roleId,"graphics")==0)
                        graphicsFound=required->matched && strcmp(required->matchedPath,selectedGraphics)==0;
                    if (strcmp(required->roleId,"dungeon")==0)
                        dungeonFound=required->matched && strcmp(required->matchedPath,destination)==0;
                    if (strcmp(required->roleId,"native-launcher")==0)
                        launcherMissing=required->required && !required->matched &&
                            required->label && strstr(required->label,"TMENU.EXP");
                }
                REQUIRE(graphicsFound && dungeonFound && launcherMissing,
                    "explicit missing-program source reports graphics/dungeon found and native menu missing");
            }
        }
        REQUIRE(menu_reports_missing_companion(menu,selectedGraphics,&options,M12_ARCH_FM_TOWNS,"TMENU.EXP"),
            "normal menu input identifies missing native menu program");
        REQUIRE(FSP_JoinPath(source,sizeof(source),original,"TMENU.EXP") &&
            FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
            symlink(physical,menuProgram)==0,"restore explicit owner's native menu program");
    }
    REQUIRE(FSP_JoinPath(destination,sizeof(destination),data,"DUNGEON.DAT") &&
        unlink(destination)==0,"remove first owner's dungeon beside complete duplicate");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index);
    REQUIRE(version && version->matched &&
        strncmp(version->matchedPath,duplicateData,strlen(duplicateData))==0,
        "complete duplicate wins over launcher-ready owner missing its dungeon");
    REQUIRE(FSP_JoinPath(source,sizeof(source),original,"DATA/DUNGEON.DAT") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),source) &&
        symlink(physical,destination)==0,"restore first owner's original dungeon");
    REQUIRE(unlink(menuProgram)==0,"make first native owner incomplete");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    version=M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index);
    REQUIRE(version && version->matched &&
        strncmp(version->matchedPath,duplicateData,strlen(duplicateData))==0 &&
        M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "complete duplicate native owner survives earlier incomplete Towns installation");
    REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)japaneseIndex)->matched,
        "English duplicate does not substitute for incomplete Japanese owner");
    M12_StartupMenu_Destroy(menu);
    M12_StartupMenu_InitWithOptions(menu,root,"dm1",&options);
    menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
    menu->gameOptions[0].architectureIndex=M12_ARCH_FM_TOWNS;
    menu->gameOptions[0].versionIndex=index;
    menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
    menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) &&
        M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded && game->active &&
        game->dm1FmtownsStartupReceiptValid &&
        strcmp(game->dm1FmtownsStartupReceipt.game_program_name,"EDM.EXP")==0,
        "complete duplicate selected through menu starts authentic native world");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    REQUIRE(FSP_JoinPath(destination,sizeof(destination),duplicate,"TMENU.EXP") &&
        unlink(destination)==0,"remove selected duplicate task menu link after scan");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
        "stale duplicate owner rejects without falling back or retaining active world");
    M12_AssetStatus_ScanGame(fresh,root,"dm1");
    REQUIRE(!M12_AssetStatus_GetVersion(fresh,"dm1",(size_t)index)->matched &&
        !M12_AssetStatus_GameAvailable(fresh,"dm1"),
        "all incomplete duplicate native owners remain unavailable");
    {
        int nativeMissing=0;
        for (i=0; i<M12_AssetStatus_GetRequiredFileCount(fresh,"dm1"); ++i) {
            const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(fresh,"dm1",i);
            if (required && required->roleId && strcmp(required->roleId,"native-launcher")==0)
                nativeMissing=required->required && !required->matched;
        }
        REQUIRE(nativeMissing,"all incomplete duplicates retain missing launcher feedback");
    }
    result=0;
    puts("PASS: native Towns admission preserves language ownership and playable sibling editions");
cleanup:
    if (nativeOutput) fclose(nativeOutput);
    free(nativeBytes);
    for (i=0; i<3; ++i) if (nativeFiles[i][0]) unlink(nativeFiles[i]);
    if (nativeDir[0]) rmdir(nativeDir);
    if (initialized) M11_GameView_Shutdown(game);
    if (menu) M12_StartupMenu_Destroy(menu);
    SDL_free(game); SDL_free(menu); SDL_free(fresh);
    for (i=0; i<linked; ++i) {
        if (FSP_JoinPath(destination,sizeof(destination),root,files[i])) unlink(destination);
    }
    for (i=0; i<duplicateLinked; ++i) {
        if (FSP_JoinPath(destination,sizeof(destination),duplicate,files[i])) unlink(destination);
    }
    if (duplicateData[0]) rmdir(duplicateData);
    if (duplicate[0]) rmdir(duplicate);
    if (pcAlias[0]) unlink(pcAlias);
    if (config[0]) unlink(config);
    if (data[0]) rmdir(data);
    if (japaneseData[0]) rmdir(japaneseData);
    if (created) rmdir(root);
    SDL_Quit();
    return result;
#endif
}
