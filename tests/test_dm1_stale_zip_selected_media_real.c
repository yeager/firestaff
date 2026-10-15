#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;
#ifndef _WIN32
#include <unistd.h>
/* Only ZIP framing is generated. Every stored game byte comes unchanged from
 * the two supplied original archives; no hash overrides are used. */
static void le16(unsigned char* p, unsigned v) { p[0]=(unsigned char)v; p[1]=(unsigned char)(v>>8); }
static void le32(unsigned char* p, uint32_t v) { le16(p,v); le16(p+2,v>>16); }
static uint32_t crc(const unsigned char* p, size_t n) {
    uint32_t c=~0u; size_t i; unsigned b;
    for(i=0;i<n;i++) { c^=p[i]; for(b=0;b<8;b++) c=(c>>1)^((0u-(c&1u))&0xedb88320u); }
    return ~c;
}
static int write_original_pair(const char* path, const char* const names[2],
                               unsigned char* const bytes[2], const size_t sizes[2]) {
    FILE* f=fopen(path,"wb"); uint32_t offsets[2], checksums[2], central; int i, ok=1;
    if(!f) return 0;
    for(i=0;i<2&&ok;i++) {
        unsigned char h[30]={0}; size_t nameSize=strlen(names[i]); long offset=ftell(f);
        if(offset<0||sizes[i]>UINT32_MAX||nameSize>65535u) { ok=0; break; }
        offsets[i]=(uint32_t)offset; checksums[i]=crc(bytes[i],sizes[i]);
        le32(h,0x04034b50u); le16(h+4,20); le32(h+14,checksums[i]);
        le32(h+18,(uint32_t)sizes[i]); le32(h+22,(uint32_t)sizes[i]); le16(h+26,(unsigned)nameSize);
        ok=fwrite(h,1,sizeof(h),f)==sizeof(h)&&fwrite(names[i],1,nameSize,f)==nameSize&&
            fwrite(bytes[i],1,sizes[i],f)==sizes[i];
    }
    central=(uint32_t)ftell(f);
    for(i=0;i<2&&ok;i++) {
        unsigned char h[46]={0}; size_t nameSize=strlen(names[i]);
        le32(h,0x02014b50u); le16(h+4,20); le16(h+6,20); le32(h+16,checksums[i]);
        le32(h+20,(uint32_t)sizes[i]); le32(h+24,(uint32_t)sizes[i]);
        le16(h+28,(unsigned)nameSize); le32(h+42,offsets[i]);
        ok=fwrite(h,1,sizeof(h),f)==sizeof(h)&&fwrite(names[i],1,nameSize,f)==nameSize;
    }
    if(ok) {
        unsigned char h[22]={0}; long end=ftell(f);
        if(end<0) ok=0;
        else { le32(h,0x06054b50u); le16(h+8,2); le16(h+10,2);
            le32(h+12,(uint32_t)end-central); le32(h+16,central);
            ok=fwrite(h,1,sizeof(h),f)==sizeof(h); }
    }
    if(fclose(f)) ok=0;
    return ok;
}
#endif
#define REQUIRE(c,m) do { if(!(c)) { fprintf(stderr,"FAIL: %s\n",m); goto cleanup; } } while(0)
#ifndef _WIN32
static int write_original_bytes(const char* path, const unsigned char* bytes, size_t size) {
    FILE* file=fopen(path,"wb"); int ok;
    if(!file) return 0;
    ok=fwrite(bytes,1,size,file)==size;
    if(fclose(file)) ok=0;
    return ok;
}
static int check_loose_selected_media(const char* root, const char* original,
    const unsigned char* multilingual, size_t multilingualSize,
    const unsigned char* dungeon, size_t dungeonSize) {
    char dir[FSP_PATH_MAX]={0}, graphics[FSP_PATH_MAX]={0};
    char selected[FSP_PATH_MAX]={0}, dungeonPath[FSP_PATH_MAX]={0};
    char originalGraphics[ASSET_PATH_MAX];
    unsigned char* english=NULL; size_t englishSize=0;
    M12_StartupMenuState* menu=NULL; M11_GameViewState* game=NULL;
    M12_StartupMenuInitOptions options; M12_LaunchIntent intent;
    M11_BootProbeReceipt receipt; int index, mode, initialized=0, result=1;
    REQUIRE(asset_find_by_md5(original,"fa6b1aa29e191418713bf2cda93d962e",
        originalGraphics,sizeof(originalGraphics),0)&&
        asset_read_path_alloc(originalGraphics,&english,&englishSize),
        "read unchanged authentic English graphics for loose staging");
    menu=SDL_calloc(1,sizeof(*menu)); game=SDL_calloc(1,sizeof(*game));
    REQUIRE(menu&&game,"allocate isolated loose-media launcher");
    for(mode=0;mode<2;mode++) {
        REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,mode?"loose-renamed":"loose-canonical")&&
            FSP_CreateDirectory(dir)&&FSP_JoinPath(graphics,sizeof(graphics),dir,"GRAPHICS.DAT")&&
            FSP_JoinPath(dungeonPath,sizeof(dungeonPath),dir,"DUNGEON.DAT")&&
            FSP_JoinPath(selected,sizeof(selected),dir,mode?"selected-original.payload":"GRAPHICS.DAT"),
            "construct task-owned loose original-media paths");
        REQUIRE(write_original_bytes(selected,english,englishSize)&&
            write_original_bytes(dungeonPath,dungeon,dungeonSize)&&
            (!mode||write_original_bytes(graphics,multilingual,multilingualSize)),
            "stage unmodified original loose graphics and dungeon bytes");
        memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
        M12_StartupMenu_InitWithOptions(menu,dir,"dm1",&options);
        index=M12_AssetStatus_FindVersionIndex("dm1","pc34-en");
        {
            const M12_AssetVersionStatus* version=index>=0?
                M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index):NULL;
            REQUIRE(version&&version->matched&&strcmp(version->matchedPath,selected)==0,
                "scan binds exact selected loose English graphics pathname");
        }
        menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
        menu->gameOptions[0].architectureIndex=M12_ARCH_PC;
        menu->gameOptions[0].versionIndex=index;
        menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
        menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
        intent=M12_StartupMenu_GetLaunchIntent(menu);
        REQUIRE(intent.valid&&intent.versionId&&strcmp(intent.versionId,"pc34-en")==0,
            "loose English source has a valid selected launch intent");
        M11_GameView_Init(game); initialized=1;
        REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu)&&
            M11_GameView_GetBootProbeReceipt(game,&receipt)&&receipt.levelLoaded&&
            strcmp(receipt.bootAssetMd5,"fa6b1aa29e191418713bf2cda93d962e")==0,
            "unchanged exact loose English source launches with authenticated receipt");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        if(mode) {
            REQUIRE(unlink(selected)==0&&asset_file_matches_md5(graphics,
                "f934d97e43e1ba6e5159839acbcd0611"),
                "remove renamed selected source while authentic multilingual sibling remains");
        } else {
            REQUIRE(write_original_bytes(selected,multilingual,multilingualSize)&&
                asset_file_matches_md5(selected,"f934d97e43e1ba6e5159839acbcd0611"),
                "replace loose selected source with unchanged authentic multilingual graphics");
        }
        REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu),
            "stale loose English selection rejects replacement or canonical sibling fallback");
        REQUIRE(!game->active,"rejected loose graphics cannot leave a running partial world");
        M11_GameView_Shutdown(game); initialized=0;
        M12_StartupMenu_Destroy(menu); memset(menu,0,sizeof(*menu));
        unlink(selected); unlink(graphics); unlink(dungeonPath); rmdir(dir);
        selected[0]=graphics[0]=dungeonPath[0]=dir[0]='\0';
    }
    result=0;
cleanup:
    if(initialized) M11_GameView_Shutdown(game);
    if(menu) M12_StartupMenu_Destroy(menu);
    SDL_free(menu); SDL_free(game); free(english);
    if(selected[0]) unlink(selected);
    if(graphics[0]) unlink(graphics);
    if(dungeonPath[0]) unlink(dungeonPath);
    if(dir[0]) rmdir(dir);
    return result==0;
}
#endif
int main(void) {
#ifdef _WIN32
    puts("SKIP: original-media ZIP replacement requires symlink staging"); return 77;
#else
    const char* original=getenv("FIRESTAFF_DM1_PC_ARCHIVE");
    const char* multi=getenv("FIRESTAFF_DM1_PC_MULTI_ARCHIVE");
    const char* scratchParent=getenv("TMPDIR");
    char root[FSP_PATH_MAX]={0}, alias[FSP_PATH_MAX]={0}, replacement[FSP_PATH_MAX]={0};
    char configPath[FSP_PATH_MAX]={0}, virtualDungeon[ASSET_PATH_MAX], multiGraphics[ASSET_PATH_MAX];
    char namesStorage[2][ASSET_PATH_MAX]; const char* names[2]={namesStorage[0],namesStorage[1]};
    unsigned char* bytes[2]={NULL,NULL}; size_t sizes[2]={0,0};
    M12_StartupMenuState* menu=NULL; M12_StartupMenuState* deferredMenu=NULL;
    M11_GameViewState* game=NULL;
    M12_StartupMenuInitOptions options; M12_LaunchIntent intent; M11_BootProbeReceipt receipt;
    M12_StartupLaunchGate gate;
    const M12_AssetVersionStatus* version; const char* member; const char* slash;
    int index, result=1, gameInitialized=0, created=0;
    if(!original||!multi||!FSP_FileExists(original)||!FSP_FileExists(multi)) {
        puts("SKIP: FIRESTAFF_DM1_PC_ARCHIVE and FIRESTAFF_DM1_PC_MULTI_ARCHIVE required"); return 77;
    }
    if(!scratchParent||!scratchParent[0]) scratchParent="build";
    REQUIRE(FSP_CreateDirectoryRecursive(scratchParent) &&
        snprintf(root,sizeof(root),"%s/dm1-stale-zip-XXXXXX",scratchParent)<(int)sizeof(root) &&
        mkdtemp(root),"create isolated scratch");
    created=1;
    {
        char physical[FSP_PATH_MAX];
        REQUIRE(FSP_ResolvePhysicalPath(physical,sizeof(physical),root),"resolve scratch path");
        snprintf(root,sizeof(root),"%s",physical);
    }
    REQUIRE(SDL_Init(0),"initialize host support");
    REQUIRE(FSP_JoinPath(alias,sizeof(alias),root,"selected.zip")&&
        FSP_JoinPath(replacement,sizeof(replacement),root,"replacement.zip")&&
        FSP_JoinPath(configPath,sizeof(configPath),root,"menu.toml"),"construct staging paths");
    REQUIRE(SDL_setenv_unsafe("FIRESTAFF_CONFIG_PATH",configPath,1)==0&&
        SDL_setenv_unsafe("SDL_VIDEODRIVER","dummy",1)==0&&
        SDL_setenv_unsafe("SDL_AUDIODRIVER","dummy",1)==0,"isolate configuration and host drivers");
    REQUIRE(symlink(original,alias)==0,"link authentic English archive");
    menu=SDL_calloc(1,sizeof(*menu)); game=SDL_calloc(1,sizeof(*game));
    REQUIRE(menu&&game,"allocate launcher and game");
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    M12_StartupMenu_InitWithOptions(menu,alias,"dm1",&options);
    index=M12_AssetStatus_FindVersionIndex("dm1","pc34-en");
    version=index>=0?M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index):NULL;
    REQUIRE(version&&version->matched&&strstr(version->matchedPath,"selected.zip::"),"scan authentic English ZIP owner");
    member=strstr(version->matchedPath,"::")+2;
    REQUIRE(!strstr(member,"::")&&strlen(member)<sizeof(namesStorage[0]),"ordinary selected ZIP member");
    snprintf(namesStorage[0],sizeof(namesStorage[0]),"%s",member);
    slash=strrchr(member,'/');
    REQUIRE(snprintf(namesStorage[1],sizeof(namesStorage[1]),"%.*sDUNGEON.DAT",
        slash?(int)(slash-member+1):0,member)<(int)sizeof(namesStorage[1]),"same-directory dungeon member");
    REQUIRE(snprintf(virtualDungeon,sizeof(virtualDungeon),"%s::%s",original,names[1])<(int)sizeof(virtualDungeon)&&
        asset_read_path_alloc(virtualDungeon,&bytes[1],&sizes[1]),"read authentic original dungeon bytes");
    REQUIRE(asset_find_by_md5(multi,"f934d97e43e1ba6e5159839acbcd0611",multiGraphics,sizeof(multiGraphics),0)&&
        asset_read_path_alloc(multiGraphics,&bytes[0],&sizes[0]),"read authentic multilingual graphics bytes");
    REQUIRE(write_original_pair(replacement,names,bytes,sizes),"frame unchanged original bytes in matching ZIP layout");
    menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
    menu->gameOptions[0].architectureIndex=M12_ARCH_PC; menu->gameOptions[0].versionIndex=index;
    menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
    menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
    /* Persist an edition using genuine scanned catalogue identities. Deferred
     * startup must wait for evidence before repairing this saved choice. */
    {
        M12_Config saved;
        M12_StartupMenuInitOptions deferredOptions=options;
        int multilingual=M12_AssetStatus_FindVersionIndex("dm1","pc34-multi");
        REQUIRE(multilingual>=0,"find saved multilingual edition");
        deferredMenu=SDL_calloc(1,sizeof(*deferredMenu));
        REQUIRE(deferredMenu,"allocate deferred launcher");
        deferredOptions.skipAssetScan=1;
        M12_Config_Load(&saved,NULL);
        saved.gameArchitectureIndex[0]=M12_ARCH_PC;
        saved.gameVersionIndex[0]=multilingual;
        REQUIRE(M12_Config_Save(&saved),"persist explicit multilingual selection");
        M12_StartupMenu_InitWithOptions(deferredMenu,root,"dm1",&deferredOptions);
        REQUIRE(deferredMenu->deferredScanPending&&
            deferredMenu->gameOptions[0].versionIndex==multilingual,
            "deferred initialization preserves saved edition before inventory exists");
        M12_StartupMenu_RunDeferredScan(deferredMenu,NULL,NULL);
        REQUIRE(!deferredMenu->deferredScanPending&&
            deferredMenu->gameOptions[0].versionIndex==multilingual,
            "completed mixed-original scan preserves installed saved multilingual edition");
        {
            const M12_AssetVersionStatus* admitted=M12_AssetStatus_GetVersion(
                &deferredMenu->assetStatus,"dm1",(size_t)multilingual);
            REQUIRE(admitted&&admitted->matched,"mixed inventory authenticates saved multilingual edition");
        }
        M12_StartupMenu_Destroy(deferredMenu);
        memset(deferredMenu,0,sizeof(*deferredMenu));
        /* The same saved choice is unavailable when scoped to English only. */
        M12_Config_Load(&saved,NULL);
        saved.gameArchitectureIndex[0]=M12_ARCH_PC;
        saved.gameVersionIndex[0]=multilingual;
        REQUIRE(M12_Config_Save(&saved),"persist missing saved edition for deferred recovery");
        M12_StartupMenu_InitWithOptions(deferredMenu,alias,"dm1",&deferredOptions);
        REQUIRE(deferredMenu->deferredScanPending&&
            deferredMenu->gameOptions[0].versionIndex==multilingual,
            "English-only deferred initialization still waits for scan evidence");
        M12_StartupMenu_RunDeferredScan(deferredMenu,NULL,NULL);
        REQUIRE(!deferredMenu->deferredScanPending&&
            deferredMenu->gameOptions[0].versionIndex==index,
            "completed English-only scan repairs unavailable saved edition");
        M12_StartupMenu_Destroy(deferredMenu);
        memset(deferredMenu,0,sizeof(*deferredMenu));
        M12_Config_Load(&saved,NULL);
        saved.gameArchitectureIndex[0]=M12_ARCH_PC;
        saved.gameVersionIndex[0]=multilingual;
        REQUIRE(M12_Config_Save(&saved),"persist missing saved edition for synchronous recovery");
        M12_StartupMenu_InitWithOptions(deferredMenu,alias,"dm1",&options);
        REQUIRE(deferredMenu->gameOptions[0].versionIndex==index,
            "ordinary English-only startup repairs unavailable saved edition");
        M12_StartupMenu_Destroy(deferredMenu);
        SDL_free(deferredMenu); deferredMenu=NULL;
        M12_StartupMenu_SaveConfig(menu);
    }
    /* Exercise the actual Custom version-row handler, rather than fabricating
     * an admission row. This selected root has only English original media. */
    menu->launchRequested=0;
    menu->view=M12_MENU_VIEW_GAME_OPTIONS;
    menu->gameCardFlowStage=2;
    menu->gameOptSelectedRow=M12_GAME_OPT_ROW_VERSION;
    menu->languageExplicit=1;
    menu->settings.languageIndex=0;
    M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_RIGHT);
    {
        int unavailable=M12_AssetStatus_FindVersionIndex("dm1","pc34-multi");
        int stalePlatform=M12_AssetStatus_FindVersionIndex("dm1","fmtowns-en");
        const M12_AssetVersionStatus* unavailableVersion=unavailable>=0?
            M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)unavailable):NULL;
        REQUIRE(unavailableVersion&&!unavailableVersion->matched&&
            menu->gameOptions[0].versionIndex==unavailable&&
            menu->gameOptions[0].architectureIndex==M12_ARCH_PC,
            "version-row input explicitly selects unavailable multilingual edition");
        REQUIRE(M12_StartupMenu_GetLaunchGate(menu,0,&gate)&&gate.dataReady&&
            !gate.versionReady&&!gate.canLaunch&&gate.autoSelectedVersionIndex<0,
            "explicit unavailable edition is blocked without substitution");
        intent=M12_StartupMenu_GetLaunchIntent(menu);
        REQUIRE(!intent.valid&&intent.versionId&&strcmp(intent.versionId,"pc34-multi")==0,
            "invalid launch intent preserves explicitly requested edition");
        menu->gameOptSelectedRow=M12_GAME_OPT_ROW_COUNT;
        M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
        REQUIRE(!menu->launchRequested&&menu->gameOptions[0].versionIndex==unavailable,
            "Launch action preserves unavailable edition and does not request runtime");
        REQUIRE(menu->messageLine1&&strcmp(menu->messageLine1,"SELECTED VERSION NOT FOUND")==0,
            "Launch action explains unavailable selected edition");
        /* AUTO remains a discovery policy; a mismatched saved row likewise
         * remains recoverable inside the explicitly requested platform. */
        menu->gameOptions[0].architectureIndex=M12_ARCH_AUTO;
        REQUIRE(M12_StartupMenu_GetLaunchGate(menu,0,&gate)&&gate.canLaunch&&
            gate.autoSelectedVersionIndex==index,"AUTO recovers an authenticated English edition");
        intent=M12_StartupMenu_GetLaunchIntent(menu);
        REQUIRE(intent.valid&&intent.options.versionIndex==index,"AUTO launch intent follows recovery");
        REQUIRE(stalePlatform>=0,"find stale different-platform catalogue row");
        menu->gameOptions[0].architectureIndex=M12_ARCH_PC;
        menu->gameOptions[0].versionIndex=stalePlatform;
        REQUIRE(M12_StartupMenu_GetLaunchGate(menu,0,&gate)&&gate.canLaunch&&
            gate.autoSelectedVersionIndex==index,"wrong-platform row recovers within requested PC platform");
        intent=M12_StartupMenu_GetLaunchIntent(menu);
        REQUIRE(intent.valid&&intent.options.versionIndex==index,
            "wrong-platform launch intent follows authenticated PC recovery");
        /* Return through the same version-row input to the installed edition. */
        menu->view=M12_MENU_VIEW_GAME_OPTIONS;
        menu->gameCardFlowStage=2;
        menu->gameOptSelectedRow=M12_GAME_OPT_ROW_VERSION;
        menu->gameOptions[0].versionIndex=unavailable;
        M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_LEFT);
        REQUIRE(menu->gameOptions[0].versionIndex==index,
            "version-row input returns to authenticated English edition");
    }
    menu->launchRequested=1;
    intent=M12_StartupMenu_GetLaunchIntent(menu);
    REQUIRE(intent.valid&&intent.versionId&&strcmp(intent.versionId,"pc34-en")==0,"English selected launch intent");
    M11_GameView_Init(game); gameInitialized=1;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu),"unchanged authentic selected archive launches");
    REQUIRE(M11_GameView_GetBootProbeReceipt(game,&receipt)&&receipt.levelLoaded&&
        strcmp(receipt.bootAssetMd5,"fa6b1aa29e191418713bf2cda93d962e")==0,"positive launch has authenticated English receipt");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    REQUIRE(unlink(alias)==0&&symlink(replacement,alias)==0,"replace selected locator with different authentic graphics");
    REQUIRE(asset_file_matches_md5(version->matchedPath,
        "f934d97e43e1ba6e5159839acbcd0611"),"replacement retains authentic multilingual graphics");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu),"stale English receipt rejects consumed multilingual graphics");
    REQUIRE(!game->active,"rejected virtual graphics cannot leave a running partial world");
    REQUIRE(check_loose_selected_media(root,original,bytes[0],sizes[0],bytes[1],sizes[1]),
        "loose selected original media rejects stale ownership");
    result=0; puts("PASS: original ZIP and loose media reject stale selected graphics and sibling fallback");
cleanup:
    if(gameInitialized) M11_GameView_Shutdown(game);
    if(menu) M12_StartupMenu_Destroy(menu);
    if(deferredMenu) M12_StartupMenu_Destroy(deferredMenu);
    SDL_free(deferredMenu);
    SDL_free(game); SDL_free(menu); free(bytes[0]); free(bytes[1]);
    if(alias[0]) unlink(alias);
    if(replacement[0]) unlink(replacement);
    if(configPath[0]) unlink(configPath);
    if(created) rmdir(root);
    SDL_Quit(); return result;
#endif
}
