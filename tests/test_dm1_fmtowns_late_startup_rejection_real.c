#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "asset_status_m12.h"
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
    REQUIRE(FSP_JoinPath(duplicate,sizeof(duplicate),root,"zz-complete") &&
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
