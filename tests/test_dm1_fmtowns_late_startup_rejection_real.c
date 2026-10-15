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
        "DATA/GRAPHICS.DAT", "DATA/DUNGEON.DAT"
    };
    const char* original=getenv("FIRESTAFF_DM1_FMTOWNS_LOOSE_ROOT");
    const char* scratch=getenv("TMPDIR");
    char root[FSP_PATH_MAX]={0}, data[FSP_PATH_MAX]={0};
    char config[FSP_PATH_MAX]={0}, menuProgram[FSP_PATH_MAX]={0};
    char source[FSP_PATH_MAX], destination[FSP_PATH_MAX], physical[FSP_PATH_MAX];
    M12_StartupMenuState* menu=NULL;
    M11_GameViewState* game=NULL;
    M12_StartupMenuInitOptions options;
    M12_LaunchIntent intent;
    M11_BootProbeReceipt receipt;
    const M12_AssetVersionStatus* version;
    size_t i, linked=0;
    int created=0, initialized=0, index, result=1;
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
    M12_StartupMenu_InitWithOptions(menu,data,"dm1",&options);
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
    /* Remove only the task-owned link after scanning. Source graphics and
     * dungeon remain valid, so failure occurs at native startup admission. */
    REQUIRE(unlink(menuProgram)==0,"remove staged TMENU link without modifying original");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu),"missing native menu rejects stale selected startup");
    REQUIRE(!game->active,"native startup rejection leaves no running partial world");
    result=0;
    puts("PASS: complete native Towns startup opens; missing TMENU rejects and clears the world");
cleanup:
    if (initialized) M11_GameView_Shutdown(game);
    if (menu) M12_StartupMenu_Destroy(menu);
    SDL_free(game); SDL_free(menu);
    for (i=0; i<linked; ++i) {
        if (FSP_JoinPath(destination,sizeof(destination),root,files[i])) unlink(destination);
    }
    if (config[0]) unlink(config);
    if (data[0]) rmdir(data);
    if (created) rmdir(root);
    SDL_Quit();
    return result;
#endif
}
