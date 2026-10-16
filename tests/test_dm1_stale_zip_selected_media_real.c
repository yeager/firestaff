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
#include "firestaff_l10n.h"
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
/* A20 TITLE.C F0437 requires its original SWSH beside the selected world. */
static int check_amiga_virtual_startup_companion(const char* root, const char* original) {
    static const char* const names[2]={"GRAPHICS.DAT","DUNGEON.DAT"};
    static const char* const nativeNames[2]={"graphics.dat","Dungeon.dat"};
    static const char* const hashes[2]={"6a2f135b53c2220f0251fa103e2a6e7e",
        "30028fb6a301ecb20127ef0b3af32b05"};
    unsigned char* bytes[2]={NULL,NULL}; size_t sizes[2]={0,0};
    char archive[FSP_PATH_MAX]={0}, member[ASSET_PATH_MAX];
    char dir[FSP_PATH_MAX]={0}, renamed[FSP_PATH_MAX]={0}, physical[FSP_PATH_MAX];
    M12_AssetStatus* status=NULL;
    M11_GameViewState* game=NULL;
    M12_StartupMenuState* menu=NULL;
    M12_StartupMenuInitOptions options;
    int result=1, initialized=0, menuInitialized=0;
    if (!original || !FSP_FileExists(original)) {
        puts("SKIP: authentic Amiga A20 archive unavailable for missing SWSH case");
        return 1;
    }
    for (int i=0; i<2; ++i) {
        REQUIRE(snprintf(member,sizeof(member),
            "%s::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf::%s",
            original,nativeNames[i])<(int)sizeof(member) &&
            asset_read_path_alloc(member,&bytes[i],&sizes[i]) &&
            asset_memory_matches_md5(bytes[i],sizes[i],hashes[i]),
            "read unchanged authentic A20 startup owner members");
    }
    REQUIRE(FSP_JoinPath(archive,sizeof(archive),root,"a20-without-swoosh.zip") &&
        write_original_pair(archive,names,bytes,sizes),
        "frame authentic A20 graphics and world without required SWSH");
    status=SDL_calloc(1,sizeof(*status));
    menu=SDL_calloc(1,sizeof(*menu));
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    REQUIRE(status && menu,"allocate A20 incomplete startup scan and menu");
    for (int mode=0; mode<2; ++mode) {
        if (mode) M12_AssetStatus_Scan(status,archive);
        else M12_AssetStatus_ScanGame(status,archive,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
            "original A20 graphics and dungeon cannot launch without their required SWSH");
        {
            int graphicsFound=0, worldFound=0, startupMissing=0;
            for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
                const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
                const char* path=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                char expected[ASSET_PATH_MAX];
                if (strcmp(required->roleId,"graphics")==0) {
                    REQUIRE(snprintf(expected,sizeof(expected),"%s::GRAPHICS.DAT",archive)<(int)sizeof(expected),
                        "construct incomplete archive's authenticated graphics receipt");
                    graphicsFound=required->matched && strcmp(path,expected)==0 &&
                        strcmp(required->matchedHash,hashes[0])==0;
                }
                if (strcmp(required->roleId,"dungeon")==0) {
                    REQUIRE(snprintf(expected,sizeof(expected),"%s::DUNGEON.DAT",archive)<(int)sizeof(expected),
                        "construct incomplete archive's authenticated world receipt");
                    worldFound=required->matched && strcmp(path,expected)==0 &&
                        strcmp(required->matchedHash,hashes[1])==0;
                }
                if (required->label && strcmp(required->label,"swoosh")==0)
                    startupMissing=required->required && !required->matched;
            }
            if (!graphicsFound || !worldFound || !startupMissing) {
                fprintf(stderr,"archive-report scan=%d graphics=%d world=%d startup=%d\n",
                    mode,graphicsFound,worldFound,startupMissing);
                for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
                    const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
                    fprintf(stderr,"archive-report role=%s label=%s matched=%d path=%s source=%s hash=%s\n",
                        required->roleId,required->label ? required->label : "",required->matched,
                        required->matchedPath,required->sourcePath,required->matchedHash);
                }
            }
            REQUIRE(graphicsFound && worldFound && startupMissing,
                "explicit original archive reports missing SWSH separately from its own verified members");
        }
        if (menuInitialized) M12_StartupMenu_Destroy(menu);
        M12_StartupMenu_InitWithOptions(menu,archive,"dm1",&options); menuInitialized=1;
        menu->view=M12_MENU_VIEW_MAIN; menu->selectedIndex=0;
        menu->gameOptions[0].architectureIndex=M12_ARCH_AMIGA;
        M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
        M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
        REQUIRE(menu->messageIsMissingGameData && !menu->launchRequested && menu->messageLine2 &&
            strstr(menu->messageLine2,"swoosh") && !strstr(menu->messageLine2,"GRAPHICS.DAT") &&
            !strstr(menu->messageLine2,"DUNGEON.DAT"),
            "ordinary menu input identifies only missing SWSH for selected original archive");
    }
    REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,"a20-renamed") && FSP_CreateDirectory(dir) &&
        FSP_JoinPath(renamed,sizeof(renamed),dir,"original.zip") &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),original) && symlink(physical,renamed)==0,
        "link unchanged complete original A20 under an arbitrary archive filename");
    game=SDL_calloc(1,sizeof(*game));
    REQUIRE(game,"allocate renamed A20 runtime");
    M11_GameView_Init(game); initialized=1;
    for (int mode=0; mode<4; ++mode) {
        const char* selected=(mode&2) ? dir : renamed;
        const M12_AssetVersionStatus* owner;
        M11_GameLaunchSpec spec;
        M11_BootProbeReceipt receipt;
        if (mode&1) M12_AssetStatus_Scan(status,selected);
        else M12_AssetStatus_ScanGame(status,selected,"dm1");
        owner=M12_AssetStatus_GetFirstMatchedVersion(status,"dm1");
        REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1") && owner &&
            strcmp(owner->matchedMd5,hashes[0])==0 &&
            strncmp(owner->matchedPath,renamed,strlen(renamed))==0 &&
            strncmp(owner->matchedPath+strlen(renamed),"::",2)==0,
            "renamed nested original A20 is discovered by file and directory scans");
        if (mode==0) {
            char collected[8][ASSET_PATH_MAX];
            int collectedCount=asset_collect_paths_by_md5(renamed,hashes[0],collected,8U,0);
            int selectedFound=0;
            REQUIRE(collectedCount>0,"bounded collector finds original nested A20 graphics");
            /* This preservation set also contains an alternate disk with
             * byte-identical graphics; distinct original member paths survive. */
            for (int entry=0; entry<collectedCount; ++entry) {
                REQUIRE(strncmp(collected[entry],renamed,strlen(renamed))==0 &&
                    strncmp(collected[entry]+strlen(renamed),"::",2)==0 &&
                    asset_file_matches_md5(collected[entry],hashes[0]),
                    "nested collector retains exact original owner and authenticated member bytes");
                if (strcmp(collected[entry],owner->matchedPath)==0) selectedFound=1;
            }
            REQUIRE(selectedFound,"bounded collector preserves renamed nested A20 container provenance");
        }
        memset(&spec,0,sizeof(spec));
        spec.title="DUNGEON MASTER"; spec.gameId=spec.sourceId="dm1";
        spec.dataDir=selected; spec.verifiedAssetPath=owner->matchedPath;
        spec.verifiedAssetMd5=hashes[0]; spec.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
        spec.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
        spec.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
        REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "renamed nested A20 handoff loads its original world");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
    }
    result=0;
cleanup:
    if (menuInitialized) M12_StartupMenu_Destroy(menu);
    SDL_free(menu);
    if (initialized) M11_GameView_Shutdown(game);
    SDL_free(game); SDL_free(status); free(bytes[0]); free(bytes[1]);
    if (renamed[0]) unlink(renamed);
    if (dir[0]) rmdir(dir);
    if (archive[0]) unlink(archive);
    return result==0;
}
static int check_demo_duplicate_startup_owner(const char* root) {
    const char* original=getenv("FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE");
    static const char* const names[2]={"graphics.dat","DemoDun.dat"};
    static const char* const hashes[2]={"491ca939f9abb33ceeb26619b841fe91",
        "78848e1a2d3d5a11e5954deb8c7b772b"};
    unsigned char* bytes[2]={NULL,NULL}; size_t sizes[2]={0,0};
    char dir[FSP_PATH_MAX]={0}, incomplete[FSP_PATH_MAX]={0}, ready[FSP_PATH_MAX]={0};
    char member[ASSET_PATH_MAX], physical[FSP_PATH_MAX];
    M12_AssetStatus* status=NULL;
    M11_GameViewState* game=NULL;
    int result=1, initialized=0;
    if (!original || !FSP_FileExists(original)) {
        puts("SKIP: authentic Amiga demo unavailable for late duplicate owner case");
        return 1;
    }
    for (int i=0; i<2; ++i) {
        REQUIRE(asset_find_by_md5(original,hashes[i],member,sizeof(member),0) &&
            asset_read_path_alloc(member,&bytes[i],&sizes[i]) &&
            asset_memory_matches_md5(bytes[i],sizes[i],hashes[i]),
            "read unchanged original demo owner members");
    }
    REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,"demo-duplicates") && FSP_CreateDirectory(dir) &&
        FSP_JoinPath(incomplete,sizeof(incomplete),dir,"dm-amiga-demo-en.zip") &&
        FSP_JoinPath(ready,sizeof(ready),dir,"ready-demo.zip") &&
        write_original_pair(incomplete,names,bytes,sizes) &&
        FSP_ResolvePhysicalPath(physical,sizeof(physical),original) && symlink(physical,ready)==0,
        "stage incomplete canonical demo beside complete original duplicate");
    status=SDL_calloc(1,sizeof(*status));
    game=SDL_calloc(1,sizeof(*game));
    REQUIRE(status && game,"allocate demo duplicate scan and runtime");
    M11_GameView_Init(game); initialized=1;
    for (int mode=0; mode<2; ++mode) {
        const M12_AssetVersionStatus* owner;
        if (mode) M12_AssetStatus_Scan(status,dir);
        else M12_AssetStatus_ScanGame(status,dir,"dm1");
        owner=M12_AssetStatus_GetFirstMatchedVersion(status,"dm1");
        REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1") && owner &&
            strncmp(owner->matchedPath,ready,strlen(ready))==0 &&
            strncmp(owner->matchedPath+strlen(ready),"::",2)==0,
            "late canonical admission cannot replace complete demo with missing-SWSH duplicate");
        {
            M11_GameLaunchSpec spec;
            M11_BootProbeReceipt receipt;
            memset(&spec,0,sizeof(spec));
            spec.title="DUNGEON MASTER"; spec.gameId=spec.sourceId="dm1";
            spec.dataDir=ready; spec.verifiedAssetPath=owner->matchedPath;
            spec.verifiedAssetMd5=hashes[0]; spec.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
            spec.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
            spec.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
            REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
                M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
                "surviving complete demo owner loads its actual original dungeon");
            M11_GameView_Shutdown(game); M11_GameView_Init(game);
        }
        if (mode) M12_AssetStatus_Scan(status,incomplete);
        else M12_AssetStatus_ScanGame(status,incomplete,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
            "explicit missing-SWSH demo cannot substitute its complete neighbor");
    }
    result=0;
cleanup:
    if (initialized) M11_GameView_Shutdown(game);
    SDL_free(game); SDL_free(status); free(bytes[0]); free(bytes[1]);
    if (ready[0]) unlink(ready);
    if (incomplete[0]) unlink(incomplete);
    if (dir[0]) rmdir(dir);
    return result==0;
}
static int check_atari_virtual_dungeon_compatibility(const char* root, const char* original,
    const char* pcOriginal, const unsigned char* pcDungeon, size_t pcDungeonSize) {
    static const char* const hashes[]={"9ce2eaf7a9e78620e3f17594437caffa",
        "050fb2cfded1b502ec2c53956b94c5bd"};
    static const size_t expectedSizes[]={271918U,33442U};
    static const char* const names[]={"GRAPHICS.DAT","DUNGEON.DAT"};
    unsigned char* bytes[2]={NULL,NULL};
    size_t sizes[2]={0,0};
    char dir[FSP_PATH_MAX]={0}, archive[FSP_PATH_MAX]={0}, pcAlias[FSP_PATH_MAX]={0};
    char nativeAlias[FSP_PATH_MAX]={0}, completeArchive[FSP_PATH_MAX]={0};
    const char* amigaOriginal=getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    char member[ASSET_PATH_MAX], graphics[ASSET_PATH_MAX], dungeon[ASSET_PATH_MAX];
    char pcGraphics[ASSET_PATH_MAX], pcDungeonPath[ASSET_PATH_MAX], physical[FSP_PATH_MAX];
    M12_AssetStatus* status=NULL;
    M11_GameViewState* game=NULL;
    M11_GameLaunchSpec spec;
    M11_BootProbeReceipt receipt;
    int result=1, initialized=0;
    REQUIRE(check_demo_duplicate_startup_owner(root),
        "complete demo survives late canonical duplicate admission");
    REQUIRE(check_amiga_virtual_startup_companion(root,amigaOriginal),
        "virtual A20 retains required original startup companion gate");
    if (!original || !FSP_FileExists(original)) {
        puts("SKIP: authentic Atari archive unavailable for virtual dungeon compatibility case");
        return 1;
    }
    REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,"atari-virtual") && FSP_CreateDirectory(dir) &&
        /* Exercise the preservation-name parent-scan branch as well as
         * hash-based native identity; the filename must not select a sibling. */
        FSP_JoinPath(archive,sizeof(archive),dir,"selected-Dungeon-Master_Amiga_EN_Version-20.zip") &&
        snprintf(graphics,sizeof(graphics),"%s::GRAPHICS.DAT",archive)<(int)sizeof(graphics) &&
        snprintf(dungeon,sizeof(dungeon),"%s::DUNGEON.DAT",archive)<(int)sizeof(dungeon),
        "construct isolated native ZIP owner");
    for (int i=0; i<2; ++i) {
        REQUIRE(asset_find_by_md5(original,hashes[i],member,sizeof(member),0) &&
            asset_read_path_alloc(member,&bytes[i],&sizes[i]) && sizes[i]==expectedSizes[i] &&
            asset_memory_matches_md5(bytes[i],sizes[i],hashes[i]),
            "read unchanged authentic Atari ZIP payload");
    }
    REQUIRE(asset_memory_matches_md5(pcDungeon,pcDungeonSize,"766450c940651fc021c92fe5d0d0b3a6"),
        "authenticate incompatible original PC ZIP payload");
    status=SDL_calloc(1,sizeof(*status)); game=SDL_calloc(1,sizeof(*game));
    REQUIRE(status && game,"allocate native ZIP scan and runtime");
    M11_GameView_Init(game); initialized=1;
    memset(&spec,0,sizeof(spec));
    spec.title="DUNGEON MASTER"; spec.gameId=spec.sourceId="dm1";
    spec.dataDir=archive; spec.verifiedAssetPath=graphics; spec.verifiedAssetMd5=hashes[0];
    spec.sourceKind=M11_GAME_SOURCE_BUILTIN_CATALOG;
    spec.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
    spec.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
    for (int phase=0; phase<3; ++phase) {
        unsigned char* payloads[2]={bytes[0],phase==1 ? (unsigned char*)pcDungeon : bytes[1]};
        size_t payloadSizes[2]={sizes[0],phase==1 ? pcDungeonSize : sizes[1]};
        REQUIRE(write_original_pair(archive,names,payloads,payloadSizes),
            "frame unchanged original bytes in selected native ZIP");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        if (phase==1) REQUIRE(!M11_GameView_Start(game,&spec) && !game->active,
            "virtual native runtime rejects stale original graphics with PC dungeon bytes");
        else REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "original native ZIP pair starts before replacement and after restoration");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        for (int mode=0; mode<2; ++mode) {
            const M12_AssetVersionStatus* owner;
            int graphicsReported=0, dungeonReported=0;
            if (mode) M12_AssetStatus_Scan(status,archive);
            else M12_AssetStatus_ScanGame(status,archive,"dm1");
            if (phase==1) {
                REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
                    "both scan APIs reject selected native ZIP with incompatible PC original dungeon");
                {
                    int ownGraphics=0, missingWorld=0;
                    for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++role) {
                        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",role);
                        const char* path=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                        if (strcmp(required->roleId,"graphics")==0)
                            ownGraphics=required->matched && strcmp(path,graphics)==0 &&
                                strcmp(required->matchedHash,hashes[0])==0;
                        if (strcmp(required->roleId,"dungeon")==0) missingWorld=!required->matched;
                    }
                    REQUIRE(ownGraphics && missingWorld,
                        "selected native archive reports its own graphics and rejects incompatible world receipt");
                }
                continue;
            }
            owner=M12_AssetStatus_GetFirstMatchedVersion(status,"dm1");
            REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1") && owner &&
                strcmp(owner->versionId,"st12-en")==0 && strcmp(owner->matchedPath,graphics)==0,
                "both scan APIs retain original native ZIP graphics owner");
            for (size_t i=0; i<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++i) {
                const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",i);
                const char* reported=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                if (strcmp(required->roleId,"graphics")==0)
                    graphicsReported=required->matched && strcmp(reported,graphics)==0 &&
                        strcmp(required->matchedHash,hashes[0])==0;
                if (strcmp(required->roleId,"dungeon")==0)
                    dungeonReported=required->matched && strcmp(reported,dungeon)==0 &&
                        strcmp(required->matchedHash,hashes[1])==0;
            }
            REQUIRE(graphicsReported && dungeonReported,"original native ZIP reports its exact own members");
        }
        if (phase==1) {
            REQUIRE(FSP_JoinPath(pcAlias,sizeof(pcAlias),dir,"pc-original.zip") &&
                FSP_ResolvePhysicalPath(physical,sizeof(physical),pcOriginal) && symlink(physical,pcAlias)==0,
                "link authentic PC ZIP beside incompatible native ZIP");
            for (int role=0; role<2; ++role) {
                const char* separator;
                const char* hash=role==0 ? "fa6b1aa29e191418713bf2cda93d962e" :
                    "766450c940651fc021c92fe5d0d0b3a6";
                char* expected=role==0 ? pcGraphics : pcDungeonPath;
                REQUIRE(asset_find_by_md5(pcOriginal,hash,member,sizeof(member),0) &&
                    (separator=strstr(member,"::"))!=NULL &&
                    snprintf(expected,ASSET_PATH_MAX,"%s%s",pcAlias,separator)<ASSET_PATH_MAX,
                    "locate exact original PC ZIP companion paths");
            }
            for (int mode=0; mode<2; ++mode) {
                const M12_AssetVersionStatus* owner;
                const M12_AssetVersionStatus* native;
                int nativeIndex=M12_AssetStatus_FindVersionIndex("dm1","st12-en");
                int graphicsReported=0, dungeonReported=0;
                if (mode) M12_AssetStatus_Scan(status,dir);
                else M12_AssetStatus_ScanGame(status,dir,"dm1");
                owner=M12_AssetStatus_GetFirstMatchedVersion(status,"dm1");
                native=nativeIndex>=0 ? M12_AssetStatus_GetVersion(status,"dm1",(size_t)nativeIndex) : NULL;
                REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1") && owner &&
                    strcmp(owner->versionId,"pc34-en")==0 && strcmp(owner->matchedPath,pcGraphics)==0 &&
                    native && !native->matched,"mixed directory preserves PC and rejects incompatible native edition");
                for (size_t i=0; i<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++i) {
                    const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",i);
                    const char* reported=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                    if (strcmp(required->roleId,"graphics")==0)
                        graphicsReported=required->matched && strcmp(reported,pcGraphics)==0 &&
                            strcmp(required->matchedHash,"fa6b1aa29e191418713bf2cda93d962e")==0;
                    if (strcmp(required->roleId,"dungeon")==0)
                        dungeonReported=required->matched && strcmp(reported,pcDungeonPath)==0 &&
                            strcmp(required->matchedHash,"766450c940651fc021c92fe5d0d0b3a6")==0;
                }
                REQUIRE(graphicsReported && dungeonReported,"mixed directory reports surviving PC ZIP's exact original members");
                if (mode) M12_AssetStatus_Scan(status,archive);
                else M12_AssetStatus_ScanGame(status,archive,"dm1");
                REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
                    "explicit incompatible native ZIP cannot substitute complete neighboring PC ZIP");
            }
            if (amigaOriginal && FSP_FileExists(amigaOriginal)) {
                REQUIRE(FSP_JoinPath(nativeAlias,sizeof(nativeAlias),dir,"Dungeon-Master_Amiga_EN_Version-20.zip") &&
                    FSP_ResolvePhysicalPath(physical,sizeof(physical),amigaOriginal) && symlink(physical,nativeAlias)==0,
                    "link canonical authentic Amiga preservation archive beside explicit incompatible Atari ZIP");
                for (int mode=0; mode<2; ++mode) {
                    if (mode) M12_AssetStatus_Scan(status,archive);
                    else M12_AssetStatus_ScanGame(status,archive,"dm1");
                    REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
                        "explicit incompatible Atari ZIP cannot substitute canonical neighboring Amiga preservation archive");
                }
                REQUIRE(unlink(nativeAlias)==0,"remove only task-owned Amiga preservation link");
                nativeAlias[0]='\0';
            } else puts("SKIP: authentic Amiga archive unavailable for explicit native ZIP neighbor case");
            REQUIRE(unlink(pcAlias)==0,"remove only task-owned PC ZIP link");
            pcAlias[0]='\0';
        }
    }
    /* Directory discovery must continue beyond an incompatible first copy.
     * Both containers retain unchanged, independently authenticated originals. */
    REQUIRE(FSP_JoinPath(completeArchive,sizeof(completeArchive),dir,"zz-complete-original-Dungeon-Master_Atari-ST_EN_Version-12-copy.zip") &&
        write_original_pair(completeArchive,names,bytes,sizes),"frame complete original duplicate ZIP");
    {
        unsigned char* incompatible[2]={bytes[0],(unsigned char*)pcDungeon};
        size_t incompatibleSizes[2]={sizes[0],pcDungeonSize};
        REQUIRE(write_original_pair(archive,names,incompatible,incompatibleSizes),
            "replace first duplicate's dungeon with incompatible original");
    }
    REQUIRE(asset_find_by_md5(dir,hashes[0],member,sizeof(member),32),
        "locate first duplicate using production hash discovery");
    if (strcmp(member,graphics)!=0) {
        unsigned char* incompatible[2]={bytes[0],(unsigned char*)pcDungeon};
        size_t incompatibleSizes[2]={sizes[0],pcDungeonSize};
        char swap[FSP_PATH_MAX];
        REQUIRE(strncmp(member,completeArchive,strlen(completeArchive))==0,
            "first discovery belongs to one of the two isolated duplicates");
        REQUIRE(write_original_pair(archive,names,bytes,sizes) &&
            write_original_pair(completeArchive,names,incompatible,incompatibleSizes),
            "place incompatible dungeon in actual first-discovered duplicate");
        memcpy(swap,archive,sizeof(swap));
        memcpy(archive,completeArchive,sizeof(archive));
        memcpy(completeArchive,swap,sizeof(completeArchive));
    }
    REQUIRE(asset_find_by_md5(dir,hashes[0],member,sizeof(member),32) &&
        strncmp(member,archive,strlen(archive))==0 &&
        strncmp(member+strlen(archive),"::",2)==0,
        "confirm incompatible duplicate remains first after unchanged graphics rewrite");
    {
        char paths[2][ASSET_PATH_MAX];
        REQUIRE(asset_collect_paths_by_md5(dir,hashes[0],paths,2U,32)==2 &&
            strcmp(paths[0],paths[1])<0,
            "bounded virtual collector returns both distinct original graphics in sorted order");
        REQUIRE(asset_collect_paths_by_md5(dir,hashes[0],paths,1U,32)==-2 && !paths[0][0],
            "bounded virtual collector reports overflow without partial publication");
        {
            const char* const duplicateNames[2]={"first/GRAPHICS.DAT","second/GRAPHICS.DAT"};
            unsigned char* duplicateBytes[2]={bytes[0],bytes[0]};
            size_t duplicateSizes[2]={sizes[0],sizes[0]};
            REQUIRE(FSP_JoinPath(nativeAlias,sizeof(nativeAlias),dir,"collector-only.zip") &&
                write_original_pair(nativeAlias,duplicateNames,duplicateBytes,duplicateSizes),
                "frame two unchanged original graphics members in one ZIP");
            REQUIRE(asset_collect_paths_by_md5(nativeAlias,hashes[0],paths,2U,0)==2 &&
                strcmp(paths[0],paths[1])<0,
                "virtual collector retains distinct same-hash members within a selected archive");
            REQUIRE(unlink(nativeAlias)==0,"remove only task-owned collector ZIP");
            nativeAlias[0]='\0';
        }
    }

    for (int mode=0; mode<2; ++mode) {
        const M12_AssetVersionStatus* owner;
        if (mode) M12_AssetStatus_Scan(status,dir);
        else M12_AssetStatus_ScanGame(status,dir,"dm1");
        owner=M12_AssetStatus_GetFirstMatchedVersion(status,"dm1");
        REQUIRE(M12_AssetStatus_GameAvailable(status,"dm1") && owner &&
            strncmp(owner->matchedPath,completeArchive,strlen(completeArchive))==0 &&
            strncmp(owner->matchedPath+strlen(completeArchive),"::",2)==0,
            "directory scan selects complete native ZIP beyond incompatible duplicate");
        {
            int graphicsReported=0, dungeonReported=0;
            char expectedDungeon[ASSET_PATH_MAX];
            REQUIRE(snprintf(expectedDungeon,sizeof(expectedDungeon),"%s::DUNGEON.DAT",completeArchive)<
                (int)sizeof(expectedDungeon),"construct complete duplicate's dungeon receipt");
            for (size_t i=0; i<M12_AssetStatus_GetRequiredFileCount(status,"dm1"); ++i) {
                const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(status,"dm1",i);
                const char* reported=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                if (strcmp(required->roleId,"graphics")==0)
                    graphicsReported=required->matched && strcmp(reported,owner->matchedPath)==0 &&
                        strcmp(required->matchedHash,hashes[0])==0;
                if (strcmp(required->roleId,"dungeon")==0)
                    dungeonReported=required->matched && strcmp(reported,expectedDungeon)==0 &&
                        strcmp(required->matchedHash,hashes[1])==0;
            }
            REQUIRE(graphicsReported && dungeonReported,
                "directory duplicate selection reports both original members from complete owner");
        }
        spec.dataDir=completeArchive; spec.verifiedAssetPath=owner->matchedPath;
        REQUIRE(M11_GameView_Start(game,&spec) && game->active &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "directory-selected complete duplicate starts actual native world");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        if (mode) M12_AssetStatus_Scan(status,archive);
        else M12_AssetStatus_ScanGame(status,archive,"dm1");
        REQUIRE(!M12_AssetStatus_GameAvailable(status,"dm1"),
            "explicit incompatible duplicate remains terminal beside complete same-edition ZIP");
    }
    result=0;
cleanup:
    if (initialized) M11_GameView_Shutdown(game);
    SDL_free(game); SDL_free(status); free(bytes[0]); free(bytes[1]);
    if (completeArchive[0]) unlink(completeArchive);
    if (nativeAlias[0]) unlink(nativeAlias);
    if (pcAlias[0]) unlink(pcAlias);
    if (archive[0]) unlink(archive);
    if (dir[0]) rmdir(dir);
    return result==0;
}
static int check_missing_pc_language_feedback(const char* selected, int language,
    const char* filename, int automatic) {
    M12_StartupMenuState* menu=SDL_calloc(1,sizeof(*menu));
    M12_StartupMenuInitOptions options;
    int ok;
    if (!menu) return 0;
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    options.hasLanguageOverride=1; options.languageOverride=language;
    M12_StartupMenu_InitWithOptions(menu,selected,"dm1",&options);
    menu->view=M12_MENU_VIEW_MAIN; menu->selectedIndex=0;
    menu->gameOptions[0].architectureIndex=automatic ? M12_ARCH_AUTO : M12_ARCH_PC;
    menu->gameOptions[0].versionIndex=M12_AssetStatus_FindVersionIndex("dm1",
        automatic ? "pc34-en" : "pc34-multi");
    menu->gameOptions[0].languageIndex=language;
    menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
    for (int input=0; input<3 && !menu->messageIsMissingGameData; ++input)
        M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
    ok=menu->messageIsMissingGameData && !menu->launchRequested && menu->messageLine2 &&
        strstr(menu->messageLine2,filename) && !strstr(menu->messageLine2,"GRAPHICS.DAT") &&
        !strstr(menu->messageLine2,"DUNGEON.DAT");
    M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_BACK);
    ok=ok && !menu->launchRequested && !menu->messageIsMissingGameData &&
        menu->view==M12_MENU_VIEW_GAME_OPTIONS && menu->activatedIndex==0 &&
        menu->gameOptions[0].languageIndex==language;
    M12_StartupMenu_Destroy(menu); SDL_free(menu);
    return ok;
}
static int check_multilingual_loose_selection(const char* root, const char* original, const char* multi,
    const unsigned char* graphicsBytes, size_t graphicsSize,
    const unsigned char* dungeonBytes, size_t dungeonSize) {
    char dir[FSP_PATH_MAX]={0}, selected[FSP_PATH_MAX]={0};
    char dungeon[FSP_PATH_MAX]={0}, english[FSP_PATH_MAX]={0}, renamed[FSP_PATH_MAX]={0};
    char foreignPath[ASSET_PATH_MAX], multiAlias[FSP_PATH_MAX]={0};
    char languageWorlds[2][FSP_PATH_MAX]={{0}};
    unsigned char* foreignDungeon=NULL; size_t foreignSize=0;
    M12_StartupMenuState* menu=NULL;
    M12_StartupMenuState* languageMenu=NULL;
    M11_GameViewState* game=NULL;
    M12_StartupMenuInitOptions options;
    M11_BootProbeReceipt receipt;
    const M12_AssetVersionStatus* version;
    const M12_AssetVersionStatus* first;
    int index, englishIndex, scanMode, initialized=0, result=1;
    REQUIRE(FSP_JoinPath(dir,sizeof(dir),root,"loose-multilingual") && FSP_CreateDirectory(dir) &&
        FSP_JoinPath(selected,sizeof(selected),dir,"selected-multi.payload") &&
        FSP_JoinPath(dungeon,sizeof(dungeon),dir,"DUNGEON.DAT") &&
        FSP_JoinPath(renamed,sizeof(renamed),dir,"original-world.payload") &&
        FSP_JoinPath(english,sizeof(english),dir,"english-original.zip"),"construct authentic multilingual owner");
    /* The original EUDATA standard dungeon is byte-identical to PC34 English
     * (766450c940651fc021c92fe5d0d0b3a6). Only task-owned framing is changed. */
    REQUIRE(asset_find_by_md5(multi,"766450c940651fc021c92fe5d0d0b3a6",foreignPath,sizeof(foreignPath),0) &&
        asset_read_path_alloc(foreignPath,&foreignDungeon,&foreignSize) &&
        foreignSize==dungeonSize && memcmp(foreignDungeon,dungeonBytes,dungeonSize)==0,
        "original multilingual standard dungeon is byte-identical to English source bytes");
    free(foreignDungeon); foreignDungeon=NULL; foreignSize=0;
    REQUIRE(write_original_bytes(selected,graphicsBytes,graphicsSize) &&
        write_original_bytes(dungeon,dungeonBytes,dungeonSize) && symlink(original,english)==0,
        "stage unchanged multilingual graphics/dungeon and original swoosh companion");
    menu=SDL_calloc(1,sizeof(*menu)); game=SDL_calloc(1,sizeof(*game));
    REQUIRE(menu && game,"allocate multilingual launcher and runtime");
    memset(&options,0,sizeof(options)); options.skipScreenshotGalleryScan=1;
    options.hasLanguageOverride=1; options.languageOverride=0;
    M12_StartupMenu_InitWithOptions(menu,selected,"dm1",&options);
    index=M12_AssetStatus_FindVersionIndex("dm1","pc34-multi");
    englishIndex=M12_AssetStatus_FindVersionIndex("dm1","pc34-en");
    REQUIRE(index>=0 && englishIndex>=0,"find both original PC catalogue editions");
    for (scanMode=0; scanMode<2; ++scanMode) {
        int reportedDungeon=0;
        if (scanMode) M12_AssetStatus_Scan(&menu->assetStatus,selected);
        version=M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index);
        first=M12_AssetStatus_GetFirstMatchedVersion(&menu->assetStatus,"dm1");
        REQUIRE(version && version->matched && strcmp(version->matchedPath,selected)==0 &&
            first && strcmp(first->versionId,"pc34-multi")==0 &&
            !M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)englishIndex)->matched,
            "explicit multilingual graphics remain terminal beside complete English archive");
        for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(&menu->assetStatus,"dm1"); ++role) {
            const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(&menu->assetStatus,"dm1",role);
            if (strcmp(required->roleId,"dungeon")==0)
                reportedDungeon=required->matched && strcmp(required->matchedPath,dungeon)==0;
        }
        REQUIRE(reportedDungeon,"both scan APIs report selected multilingual dungeon provenance");
    }
    REQUIRE(FSP_JoinPath(multiAlias,sizeof(multiAlias),dir,"multi-original.zip") && symlink(multi,multiAlias)==0,
        "stage unchanged original multilingual archive for report check");
    M12_AssetStatus_ScanGame(&menu->assetStatus,multiAlias,"dm1");
    REQUIRE(M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1"),"original multilingual archive remains available");
    {
        int reportedDungeon=0;
        for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(&menu->assetStatus,"dm1"); ++role) {
            const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(&menu->assetStatus,"dm1",role);
            if (strcmp(required->roleId,"dungeon")==0)
                reportedDungeon=required->matched && strncmp(required->matchedPath,multiAlias,strlen(multiAlias))==0;
        }
        REQUIRE(reportedDungeon,"multilingual archive reports its own dungeon rather than English companion");
    }
    REQUIRE(unlink(multiAlias)==0,"remove only task multilingual archive link");
    multiAlias[0]='\0';
    M12_AssetStatus_ScanGame(&menu->assetStatus,selected,"dm1");
    menu->selectedIndex=0; menu->activatedIndex=0; menu->launchRequested=1;
    menu->gameOptions[0].architectureIndex=M12_ARCH_PC;
    menu->gameOptions[0].versionIndex=index;
    menu->gameOptions[0].presentationModeIndex=M12_PRESENTATION_V1_ORIGINAL;
    menu->settings.rendererBackendIndex=M12_RENDERER_BACKEND_SOFTWARE;
    M11_GameView_Init(game); initialized=1;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) &&
        M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded && game->active &&
        strcmp(receipt.bootAssetMd5,"f934d97e43e1ba6e5159839acbcd0611")==0,
        "explicit original multilingual owner starts its authenticated world");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    {
        static const char* const names[]={"DUNGEONF.DAT","DUNGEONG.DAT"};
        static const char* const hashes[]={"82c7802122e9cfb2dc117bc9d197f457",
            "b9487db563ff9e89605c77ee44d17d14"};
        static const size_t sizes[]={33687U,33705U};
        REQUIRE(FSP_JoinPath(multiAlias,sizeof(multiAlias),dir,"multi-original.zip") &&
            symlink(multi,multiAlias)==0,"stage unchanged multilingual archive for language handoff");
        for (int language=0; language<2; ++language) {
            M12_AssetStatusScanOptions chosenLanguageScan;
            memset(&chosenLanguageScan,0,sizeof(chosenLanguageScan));
            chosenLanguageScan.dm1DungeonLanguage=language+1;
            REQUIRE(FSP_JoinPath(languageWorlds[language],sizeof(languageWorlds[language]),dir,names[language]) &&
                asset_find_by_md5(multi,hashes[language],foreignPath,sizeof(foreignPath),0) &&
                asset_read_path_alloc(foreignPath,&foreignDungeon,&foreignSize) &&
                foreignSize==sizes[language] && asset_memory_matches_md5(foreignDungeon,foreignSize,hashes[language]) &&
                write_original_bytes(languageWorlds[language],foreignDungeon,foreignSize),
                "stage authenticated original French/German dungeon beside multilingual graphics");
            {
                struct GameWorld_Compat strictWorld;
                memset(&strictWorld,0,sizeof(strictWorld));
                REQUIRE(!F0882_WORLD_InitFromDungeonDatBuffer_Compat(foreignDungeon,
                    (int)foreignSize,0xF1A5U,&strictWorld) && !strictWorld.dungeon,
                    "generic save-tail reader still rejects authentic checksumless language media");
                REQUIRE(!F0882M_WORLD_InitFromPcMultilangBodyBuffer_Compat(foreignDungeon,
                    (int)foreignSize-1,0xF1A5U,&strictWorld) && !strictWorld.dungeon,
                    "original-language body reader rejects truncated original bytes");
            }
            free(foreignDungeon); foreignDungeon=NULL;
            for (int virtualOwner=0; virtualOwner<2; ++virtualOwner) {
                const char* owner=virtualOwner ? multiAlias : selected;
                menu->gameOptions[0].languageIndex=language==0 ? 2 : 3;
                M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,owner,"dm1",&chosenLanguageScan);
                REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                    M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
                    "menu French/German option starts the selected original multilingual world");
                REQUIRE(asset_file_matches_md5(game->dungeonPath,hashes[language]) &&
                    (virtualOwner ? strncmp(game->dungeonPath,multiAlias,strlen(multiAlias))==0 :
                        strcmp(game->dungeonPath,languageWorlds[language])==0),
                    "multilingual menu handoff consumes selected language dungeon from its own loose/ZIP owner");
                M11_GameView_Shutdown(game); M11_GameView_Init(game);
            }
            /* A language-only installation has no English world in its own
             * loose owner. Keep the original English ZIP solely as the real
             * startup companion; it must not lend a dungeon to this owner. */
            {
                M12_AssetStatusScanOptions languageScan;
                REQUIRE(unlink(dungeon)==0 && unlink(multiAlias)==0,
                    "remove only staged English world and multilingual archive alias");
                for (int fullScan=0; fullScan<2; ++fullScan) {
                    if (fullScan) M12_AssetStatus_Scan(&menu->assetStatus,selected);
                    else M12_AssetStatus_ScanGame(&menu->assetStatus,selected,"dm1");
                    REQUIRE(!M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1") &&
                        !M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)englishIndex)->matched,
                        "default English scan cannot borrow a sibling world for language-only selected graphics");
                }
                memset(&languageScan,0,sizeof(languageScan));
                languageScan.dm1DungeonLanguage=language+1;
                for (int fullScan=0; fullScan<2; ++fullScan) {
                    int reportedDungeon=0;
                    if (fullScan) M12_AssetStatus_ScanWithOptions(&menu->assetStatus,selected,&languageScan);
                    else M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&languageScan);
                    version=M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index);
                    REQUIRE(M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1") &&
                        menu->assetStatus.dm1DungeonLanguage==language+1 && version && version->matched &&
                        strcmp(version->matchedPath,selected)==0 &&
                        !M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)englishIndex)->matched,
                        "language-aware scan admits exact multilingual graphics without a standard English world");
                    for (size_t role=0; role<M12_AssetStatus_GetRequiredFileCount(&menu->assetStatus,"dm1"); ++role) {
                        const M12_AssetRequiredFileStatus* required=M12_AssetStatus_GetRequiredFile(&menu->assetStatus,"dm1",role);
                        const char* source=required->sourcePath[0] ? required->sourcePath : required->matchedPath;
                        if (strcmp(required->roleId,"dungeon")==0)
                            reportedDungeon=required->matched && strcmp(source,languageWorlds[language])==0 &&
                                strcmp(required->matchedHash,hashes[language])==0;
                    }
                    REQUIRE(reportedDungeon,
                        "language-aware scan reports the exact original selected-language world");
                }
                if (language==0) {
                    M12_StartupMenuInitOptions languageOptions;
                    memset(&languageOptions,0,sizeof(languageOptions));
                    languageOptions.skipScreenshotGalleryScan=1;
                    languageOptions.hasLanguageOverride=1;
                    languageOptions.languageOverride=2;
                    languageMenu=SDL_calloc(1,sizeof(*languageMenu));
                    REQUIRE(languageMenu,"allocate initial French-only menu policy check");
                    M12_StartupMenu_InitWithOptions(languageMenu,selected,"dm1",&languageOptions);
                    version=M12_AssetStatus_GetVersion(&languageMenu->assetStatus,"dm1",(size_t)index);
                    REQUIRE(languageMenu->gameOptions[0].languageIndex==2 &&
                        languageMenu->assetStatus.dm1DungeonLanguage==1 &&
                        M12_AssetStatus_GameAvailable(&languageMenu->assetStatus,"dm1") &&
                        version && version->matched && strcmp(version->matchedPath,selected)==0,
                        "initial language override applies before first scan of French-only original installation");
                    M12_StartupMenu_Destroy(languageMenu); SDL_free(languageMenu); languageMenu=NULL;
                }
                menu->view=M12_MENU_VIEW_GAME_OPTIONS; menu->selectedIndex=0; menu->activatedIndex=0;
                menu->gameCardFlowStage=2; menu->gameOptSelectedRow=M12_GAME_OPT_ROW_COUNT;
                menu->launchRequested=0; menu->messageIsMissingGameData=0;
                menu->gameOptions[0].architectureIndex=M12_ARCH_PC;
                menu->gameOptions[0].versionIndex=index;
                menu->gameOptions[0].languageIndex=language==0 ? 2 : 3;
                M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
                REQUIRE(menu->launchRequested && M12_StartupMenu_GetLaunchIntent(menu).valid &&
                    M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                    M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded &&
                    strcmp(receipt.bootAssetMd5,"f934d97e43e1ba6e5159839acbcd0611")==0 &&
                    strcmp(game->dungeonPath,languageWorlds[language])==0 &&
                    asset_file_matches_md5(game->dungeonPath,hashes[language]),
                    "ordinary menu Launch input starts original French/German-only installation");
                M11_GameView_Shutdown(game); M11_GameView_Init(game);
                REQUIRE(asset_read_path_alloc(languageWorlds[language],&foreignDungeon,&foreignSize) &&
                    unlink(languageWorlds[language])==0,
                    "remove only selected-language world while retaining original English startup ZIP");
                M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&languageScan);
                REQUIRE(!M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1") &&
                    !M12_StartupMenu_GetLaunchIntent(menu).valid &&
                    !M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
                    "language-only installation cannot borrow English or another language when its world is absent");
                menu->launchRequested=0; menu->view=M12_MENU_VIEW_GAME_OPTIONS;
                menu->gameOptSelectedRow=M12_GAME_OPT_ROW_COUNT;
                M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
                REQUIRE(!menu->launchRequested,
                    "ordinary menu Launch input blocks missing chosen language without sibling substitution");
                REQUIRE(write_original_bytes(languageWorlds[language],foreignDungeon,foreignSize) &&
                    write_original_bytes(dungeon,dungeonBytes,dungeonSize) && symlink(multi,multiAlias)==0,
                    "restore unchanged original language world, standard world and task archive alias");
                free(foreignDungeon); foreignDungeon=NULL;
                M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&chosenLanguageScan);
                menu->launchRequested=1;
            }
            /* A previously scanned French/German selection must not fall
             * back to English when only its task-owned dungeon disappears. */
            REQUIRE(asset_read_path_alloc(languageWorlds[language],&foreignDungeon,&foreignSize) &&
                write_original_bytes(languageWorlds[language],dungeonBytes,dungeonSize),
                "replace only staged language file with unchanged different original English bytes");
            M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&chosenLanguageScan);
            REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
                "selected-language hash gate rejects a different original under the expected filename");
            REQUIRE(check_missing_pc_language_feedback(selected,language==0 ? 2 : 3,names[language],0) &&
                check_missing_pc_language_feedback(selected,language==0 ? 2 : 3,names[language],1),
                "menu reports the exact invalid chosen-language file before requesting launch");
            REQUIRE(unlink(languageWorlds[language])==0,"remove only staged selected-language dungeon");
            M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&chosenLanguageScan);
            REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
                "missing chosen-language dungeon rejects without English or archive substitution");
            REQUIRE(!M12_StartupMenu_GetLaunchIntent(menu).valid &&
                check_missing_pc_language_feedback(selected,language==0 ? 2 : 3,names[language],0) &&
                check_missing_pc_language_feedback(selected,language==0 ? 2 : 3,names[language],1),
                "missing chosen-language file blocks both direct intent and ordinary menu input");
            REQUIRE(write_original_bytes(languageWorlds[language],foreignDungeon,foreignSize),
                "restore unchanged selected-language original dungeon");
            free(foreignDungeon); foreignDungeon=NULL;
            REQUIRE(rename(languageWorlds[language],renamed)==0,
                "rename only staged original chosen-language dungeon");
            M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&chosenLanguageScan);
            REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                strcmp(game->dungeonPath,renamed)==0 && asset_file_matches_md5(game->dungeonPath,hashes[language]),
                "renamed chosen-language original resolves by hash within selected loose owner only");
            M11_GameView_Shutdown(game); M11_GameView_Init(game);
            REQUIRE(rename(renamed,languageWorlds[language])==0,
                "restore staged original chosen-language filename");
        }
        {
            char scanRoot[M12_ASSET_DATA_DIR_CAPACITY];
            uint64_t deadline;
            snprintf(scanRoot,sizeof(scanRoot),"%s",menu->selectedDataDir[0] ?
                menu->selectedDataDir : M12_AssetStatus_GetDataDir(&menu->assetStatus));
            menu->view=M12_MENU_VIEW_GAME_OPTIONS; menu->selectedIndex=0; menu->activatedIndex=0;
            menu->gameCardFlowStage=2; menu->gameOptSelectedRow=M12_GAME_OPT_ROW_LANGUAGE;
            menu->gameOptions[0].architectureIndex=M12_ARCH_PC;
            menu->gameOptions[0].versionIndex=index;
            menu->gameOptions[0].languageIndex=3;
            menu->launchRequested=0; menu->messageIsMissingGameData=0;
            M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_LEFT);
            REQUIRE(menu->gameOptions[0].languageIndex==2 && menu->dataDirScanActive &&
                menu->dataDirScanJob && !menu->launchRequested &&
                !M12_StartupMenu_GetLaunchIntent(menu).valid,
                "ordinary German-to-French option input blocks launch while original-media scan is pending");
            /* A language request can become stale before worker publication.
             * The old French result must never replace the German snapshot. */
            menu->gameOptions[0].languageIndex=3;
            deadline=SDL_GetTicks()+30000U;
            while (menu->dataDirScanJob && SDL_GetTicks()<deadline) {
                M12_StartupMenu_Update(menu);
                REQUIRE(menu->assetStatus.dm1DungeonLanguage!=1,
                    "stale French worker cannot publish after the requested language changed");
                if (menu->dataDirScanJob) SDL_Delay(1);
            }
            REQUIRE(!menu->dataDirScanJob && !menu->dataDirScanActive &&
                menu->assetStatus.dm1DungeonLanguage==2 &&
                menu->view==M12_MENU_VIEW_GAME_OPTIONS,
                "replacement worker completes the latest requested German language");
            menu->gameOptSelectedRow=M12_GAME_OPT_ROW_LANGUAGE;
            M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_LEFT);
            REQUIRE(menu->gameOptions[0].languageIndex==2 && menu->dataDirScanJob,
                "ordinary French request can restart after a stale worker was rejected");
            deadline=SDL_GetTicks()+30000U;
            while (menu->dataDirScanJob && SDL_GetTicks()<deadline) {
                M12_StartupMenu_Update(menu);
                if (menu->dataDirScanJob) SDL_Delay(1);
            }
            version=M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index);
            REQUIRE(!menu->dataDirScanJob && !menu->dataDirScanActive &&
                !menu->dataDirScanCancelled && !menu->dm1LanguageRescanPending &&
                menu->assetStatus.dm1DungeonLanguage==1 &&
                menu->view==M12_MENU_VIEW_GAME_OPTIONS &&
                strcmp(menu->selectedDataDir,scanRoot)==0 &&
                strcmp(M12_AssetStatus_GetDataDir(&menu->assetStatus),scanRoot)==0 &&
                version && version->matched && strcmp(version->matchedPath,selected)==0,
                "completed asynchronous French scan restores options and exact selected original owner/root");
            menu->gameOptSelectedRow=M12_GAME_OPT_ROW_COUNT;
            M12_StartupMenu_HandleInput(menu,M12_MENU_INPUT_ACCEPT);
            REQUIRE(menu->launchRequested && M12_StartupMenu_GetLaunchIntent(menu).valid &&
                M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
                M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded &&
                strcmp(game->dungeonPath,languageWorlds[0])==0 &&
                asset_file_matches_md5(game->dungeonPath,hashes[0]),
                "completed ordinary language-change scan launches exact original French world");
            M11_GameView_Shutdown(game); M11_GameView_Init(game);
        }
        REQUIRE(unlink(multiAlias)==0,"remove only task language archive link");
        multiAlias[0]='\0';
        menu->gameOptions[0].languageIndex=1;
        M12_AssetStatus_ScanGame(&menu->assetStatus,selected,"dm1");
        REQUIRE(M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
            strcmp(game->dungeonPath,dungeon)==0 &&
            asset_file_matches_md5(game->dungeonPath,"766450c940651fc021c92fe5d0d0b3a6"),
            "unsupported original language retains the English dungeon");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        {
            M12_AssetStatusScanOptions autoScan;
            M12_LaunchIntent autoIntent;
            FS_Language systemLanguage=fs_l10n_detect_system_language();
            memset(&autoScan,0,sizeof(autoScan));
            autoScan.dm1DungeonLanguage=systemLanguage==FS_LANG_FR ? 1 :
                systemLanguage==FS_LANG_DE ? 2 : 0;
            menu->gameOptions[0].languageIndex=(M12_StartupMenu_GetLanguageCount()-1);
            M12_AssetStatus_ScanGameWithOptions(&menu->assetStatus,selected,"dm1",&autoScan);
            autoIntent=M12_StartupMenu_GetLaunchIntent(menu);
            REQUIRE(autoIntent.valid && autoIntent.options.languageIndex!=(M12_StartupMenu_GetLanguageCount()-1) &&
                autoIntent.launcherOptions.languageIndex==autoIntent.options.languageIndex,
                "AUTO resolves the same original-world language before runtime intent export");
        }
        menu->gameOptions[0].languageIndex=0;
        M12_AssetStatus_ScanGame(&menu->assetStatus,selected,"dm1");
    }
    REQUIRE(asset_find_by_md5(multi,"82c7802122e9cfb2dc117bc9d197f457",foreignPath,sizeof(foreignPath),0) &&
        asset_read_path_alloc(foreignPath,&foreignDungeon,&foreignSize) &&
        write_original_bytes(dungeon,foreignDungeon,foreignSize),
        "replace default dungeon with unchanged original French dungeon bytes");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
        "stale standard-dungeon receipt rejects consumed different original dungeon");
    {
        M11_GameLaunchSpec custom;
        memset(&custom,0,sizeof(custom));
        custom.title="DUNGEON MASTER"; custom.gameId=custom.sourceId="dm1";
        custom.dataDir=dir; custom.dungeonPath=dungeon;
        custom.verifiedAssetPath=selected;
        custom.verifiedAssetMd5="f934d97e43e1ba6e5159839acbcd0611";
        custom.sourceKind=M11_GAME_SOURCE_DIRECT_DUNGEON;
        custom.presentationMode=M12_PRESENTATION_V1_ORIGINAL;
        custom.rendererBackend=M12_RENDERER_BACKEND_SOFTWARE;
        REQUIRE(M11_GameView_Start(game,&custom) && game->active &&
            M11_GameView_GetBootProbeReceipt(game,&receipt) && receipt.levelLoaded,
            "explicit direct dungeon path retains authentic nonstandard-dungeon support");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        REQUIRE(FSP_JoinPath(multiAlias,sizeof(multiAlias),dir,"multi-original.zip") &&
            symlink(multi,multiAlias)==0 &&
            snprintf(foreignPath,sizeof(foreignPath),"%s::GRAPHICS.DAT",multiAlias)<(int)sizeof(foreignPath),
            "stage authenticated virtual graphics for explicit dungeon path");
        custom.dataDir=foreignPath;
        custom.verifiedAssetPath=foreignPath;
        REQUIRE(M11_GameView_Start(game,&custom) && game->active &&
            strcmp(game->dungeonPath,dungeon)==0,
            "virtual PC graphics preserve explicitly supplied original dungeon path");
        M11_GameView_Shutdown(game); M11_GameView_Init(game);
        REQUIRE(unlink(multiAlias)==0,"remove only task virtual graphics archive link");
        multiAlias[0]='\0';
    }
    REQUIRE(write_original_bytes(dungeon,dungeonBytes,dungeonSize) && rename(dungeon,renamed)==0,
        "rename unchanged original standard dungeon within selected owner");
    M12_AssetStatus_ScanGame(&menu->assetStatus,selected,"dm1");
    REQUIRE(M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1") &&
        M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index)->matched &&
        M11_GameView_OpenSelectedMenuEntry(game,menu) && game->active &&
        strcmp(game->dungeonPath,renamed)==0,
        "renamed original multilingual dungeon retains exact same-owner startup");
    M11_GameView_Shutdown(game); M11_GameView_Init(game);
    REQUIRE(unlink(renamed)==0,"remove only selected multilingual dungeon");
    REQUIRE(!M11_GameView_OpenSelectedMenuEntry(game,menu) && !game->active,
        "stale multilingual owner cannot borrow English archive dungeon");
    M12_StartupMenu_Destroy(menu);
    M12_StartupMenu_InitWithOptions(menu,selected,"dm1",&options);
    REQUIRE(!M12_AssetStatus_GameAvailable(&menu->assetStatus,"dm1") &&
        !M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)index)->matched &&
        !M12_AssetStatus_GetVersion(&menu->assetStatus,"dm1",(size_t)englishIndex)->matched,
        "fresh incomplete multilingual file rejects without English substitution");
    result=0;
cleanup:
    if(initialized) M11_GameView_Shutdown(game);
    if(languageMenu) M12_StartupMenu_Destroy(languageMenu);
    SDL_free(languageMenu);
    if(menu) M12_StartupMenu_Destroy(menu);
    SDL_free(game); SDL_free(menu); free(foreignDungeon);
    for (int language=0; language<2; ++language)
        if(languageWorlds[language][0]) unlink(languageWorlds[language]);
    if(renamed[0]) unlink(renamed);
    if(multiAlias[0]) unlink(multiAlias);
    if(selected[0]) unlink(selected);
    if(dungeon[0]) unlink(dungeon);
    if(english[0]) unlink(english);
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
    const char* atari=getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
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
    REQUIRE(check_multilingual_loose_selection(root,original,multi,bytes[0],sizes[0],bytes[1],sizes[1]),
        "multilingual loose selection retains its exact original owner");
    REQUIRE(check_atari_virtual_dungeon_compatibility(root,atari,original,bytes[1],sizes[1]),
        "selected native ZIP retains compatible original dungeon ownership");
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
