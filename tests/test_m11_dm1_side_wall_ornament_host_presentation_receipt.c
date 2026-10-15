#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "render_sdl_m11.h"
#include "asset_status_m12.h"
#include "memory_combat_pc34_compat.h"
#include "vga_palette_pc34_compat.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_original_legacy_materials(M11_GameViewState* state, const char* edition);

/* Independent MEDIA009 F0115 original-M558 oracle. These are isolated
 * material parameters; the second check below walks the untouched world. */
static int check_original_native_objects(M11_GameViewState* state, const char* edition)
{
    static const int objects[][2] = {{10,1},{5,0},{8,0},{9,0},{10,51}};
    static const unsigned char shifts[16][2] = {
        {2,5},{0,6},{5,7},{3,0},{7,1},{1,2},{6,3},{3,3},
        {5,5},{2,6},{7,7},{1,0},{3,1},{6,2},{1,3},{5,3}};
    static const signed char values[3][8] = {
        {0,1,2,3,0,-3,-2,-1},{0,1,1,2,0,-2,-1,-1},{0,1,1,1,0,-1,-1,-1}};
    unsigned char raw[4720], actual[64000], expected[64000];
    size_t length = 0;
    int cases = 0;
    if (state->assetLoader.atariStDm1) {
        DM1_V1_AtariStGraphicsDat dat;
        if (!dm1_v1_atari_st_graphics_open(state->assetLoader.atariStData,
                (size_t)state->assetLoader.atariStDataSize,&dat)) return 0;
        int count = dm1_v1_atari_st_graphics_read(&dat,558,raw,sizeof(raw));
        if (count != (int)sizeof(raw)) return 0;
        length = (size_t)count;
    } else if (!dm1_v1_legacy_graphics_read_raw(state->assetLoader.legacyData,
        (size_t)state->assetLoader.legacyDataSize,1,558,raw,sizeof(raw),&length)) return 0;
    if (length != sizeof(raw)) return 0;
    for (unsigned object=0; object<sizeof(objects)/sizeof(objects[0]); ++object)
    for (int square=0; square<10; ++square)
    for (int cell=0; cell<5; ++cell)
    for (int parity=0; parity<2; ++parity) {
        const int aspectIndex=dm1_item_aspect_index(objects[object][0],objects[object][1]);
        const unsigned char* aspect=raw+1642+aspectIndex*6;
        const unsigned char* xy=raw+1156+((aspect[5]*10+square)*5+cell)*2;
        const int forward=square==9?0:3-square/3;
        const int side=square%3==1?-1:square%3==2?1:0;
        const int alcove=cell==4;
        const int alt=alcove && square%3==0 && (aspect[4]&16);
        const int graphic=360+aspect[0]+(alt!=0);
        const int pile=(alcove?2:0)+parity*8;
        const M11_AssetSlot* slot=M11_AssetLoader_Load(&state->assetLoader,(unsigned)graphic);
        int width=aspect[2],height=aspect[3],set=0,padding=0;
        int flip=(aspect[4]&1) && !alt &&
            (square%3==2 || (square%3==0 && (cell==1 || cell==2)));
        const unsigned char* palette=NULL;
        if (!slot || !slot->pixels || slot->width!=width*2 || slot->height!=height) return 0;
        if (!(square==9 || (square>=6 && cell>=2))) {
            int scale=square>=6 || (square>=3 && cell>=2)?20:16;
            width=(width*scale+scale/2)/32;
            height=(height*scale+scale/2)/32;
            set=scale==20?1:2;
            palette=raw+(scale==20?1534:1518);
            if (flip) padding=(7-((width-1)&7))*2;
        }
        memset(actual,17,sizeof(actual));
        memset(expected,17,sizeof(expected));
        if (xy[1]) {
            int center=xy[0]+values[set][shifts[pile][0]];
            int bottom=xy[1]+values[set][shifts[pile][1]];
            int top=bottom-height+1,left=center-width+1,right=center+width;
            if (left<0) left=0;
            if (right>223) right=223;
            if (bottom>135) bottom=135;
            int sourceX=left?padding:width-center-1;
            unsigned xs=((unsigned)slot->width*1024u/(unsigned)(width*2))*64u;
            unsigned ys=((unsigned)slot->height*1024u/(unsigned)height)*64u;
            for (int y=top;y<=bottom;++y) for(int x=left;x<=right;++x) {
                int sx=sourceX+x-left;
                if(flip)sx=((width+7)&~7)*2-1-sx;
                if(sx<0||sx>=width*2||y<0)continue;
                unsigned u=(xs/2+32767+(unsigned)sx*xs)>>16;
                unsigned v=(ys/2+32767+(unsigned)(y-top)*ys)>>16;
                if(u>=slot->width||v>=slot->height)return 0;
                unsigned char color=slot->pixels[v*slot->width+u];
                if(palette)color=palette[color&15]/10;
                if(color!=10)expected[(33+y)*320+x]=color;
            }
        }
        (void)M11_GameView_ProbeDrawDm1Media009Object(state,actual,320,200,
            objects[object][0],objects[object][1],alcove?2:cell,pile,forward,side,alcove);
        if(memcmp(actual,expected,sizeof(actual))) {
            fprintf(stderr,"FAIL: original %s F0115 object%d square%d cell%d parity%d graphic%d\n",
                edition,object,square,cell,parity,graphic);return 0;
        }
        ++cases;
    }
    printf("PASS: original %s F0115 %d full-raster object cases, native M558 positions/palettes/flip/C10\n",edition,cases);
    return 1;
}

static int check_original_native_item_walk(const M11_GameLaunchSpec* spec, const char* edition)
{
    static const char path[]="SSSSEEENNNNNEESEESESSSSSWWWWWSSSSSS";
    M11_GameViewState* state=calloc(1,sizeof(*state));
    unsigned char framebuffer[64000];
    M11_Dm1FloorItemHostPresentationReceipt receipt={0};
    int ok=0;
    if(!state)return 0;
    M11_GameView_Init(state);
    if(!M11_GameView_Start(state,spec)||state->world.party.mapIndex!=0||
       state->world.party.mapX!=1||state->world.party.mapY!=3||state->world.party.direction!=2)goto done;
    for(unsigned step=0;step<sizeof(path)-1;++step) {
        int direction=path[step]=='N'?0:path[step]=='E'?1:path[step]=='S'?2:3;
        int x=state->world.party.mapX,y=state->world.party.mapY;
        for(int turn=0;turn<4&&state->world.party.direction!=direction;++turn)
            M11_GameView_HandleInput(state,M12_MENU_INPUT_RIGHT);
        for(int attempt=0;attempt<100&&state->world.party.mapX==x&&state->world.party.mapY==y;++attempt) {
            M11_GameView_HandleInput(state,M12_MENU_INPUT_UP);
            for(int wait=0;wait<5;++wait)M11_GameView_HandleInput(state,M12_MENU_INPUT_NONE);
        }
        if(state->world.party.mapIndex!=0||
           (state->world.party.mapX==x&&state->world.party.mapY==y))goto done;
    }
    for(int turn=0;turn<4&&state->world.party.direction!=3;++turn)
        M11_GameView_HandleInput(state,M12_MENU_INPUT_RIGHT);
    if(state->world.party.mapX!=4||state->world.party.mapY!=15||state->world.party.direction!=3)goto done;
    M11_GameView_Draw(state,framebuffer,320,200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if(!receipt.valid||!receipt.floorItemLane||receipt.graphicsId!=367||
       receipt.assetWidth!=48||receipt.assetHeight!=13||receipt.transparentColor!=10||
       receipt.usesF0791Blit||!receipt.destinationPixelsChanged||
       receipt.destinationW>48||receipt.destinationH>13)goto done;
    printf("PASS: original %s actual 35-step Hall walk to (4,15,W), F0115 object367 48x13, no creature505\n",edition);
    ok=1;
done:
    if(!ok)fprintf(stderr,"FAIL: original %s actual native object walk at %d,%d,%d\n",edition,
        state->world.party.mapX,state->world.party.mapY,state->world.party.direction);
    if(!ok)fprintf(stderr,"native object receipt valid%d floor%d graphic%d asset%dx%d destination%d,%d %dx%d changed%d F0791%d\n",
        receipt.valid,receipt.floorItemLane,receipt.graphicsId,receipt.assetWidth,receipt.assetHeight,
        receipt.destinationX,receipt.destinationY,receipt.destinationW,receipt.destinationH,
        receipt.destinationPixelsChanged,receipt.usesF0791Blit);
    M11_GameView_Shutdown(state);free(state);return ok;
}

/* Reuse one public M11 state across real native owners. Start must
 * invalidate metadata even if malloc later reuses an address. */
static int check_original_native_object_rebind(M11_GameViewState* state,
                                                const M11_GameLaunchSpec* amiga)
{
    const char* atari=getenv("FIRESTAFF_DM1_ATARI_ARROWS_ARCHIVE");
    M12_AssetStatus* scan;
    M11_GameLaunchSpec spec={0};
    const M12_AssetVersionStatus* version;
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    int index,ok=0;
    if(!atari||!atari[0])return 1;
    scan=calloc(1,sizeof(*scan));
    if(!scan)return 0;
    M12_AssetStatus_ScanGame(scan,atari,"dm1");
    index=M12_AssetStatus_FindVersionIndex("dm1","st12-en");
    version=index<0?NULL:M12_AssetStatus_GetVersion(scan,"dm1",(size_t)index);
    if(!version||!version->matched||strcmp(version->matchedMd5,"9ce2eaf7a9e78620e3f17594437caffa")||
       !M12_AssetStatus_PrepareDM1RuntimeVersion(scan,"st12-en",runtime,sizeof(runtime)))goto done;
    spec.title="Dungeon Master";spec.gameId=spec.sourceId="dm1";
    spec.dataDir=runtime;spec.verifiedAssetPath=version->matchedPath;
    spec.verifiedAssetMd5=version->matchedMd5;
    for(int route=0;route<2;++route) {
        if(!M11_GameView_Start(state,route?amiga:&spec)||
           state->dm1NativeObjectMetadataValid||state->dm1NativeObjectMetadataSource||
           !check_original_native_objects(state,route?"A20E rebound":"S12E rebound")||
           state->dm1NativeObjectPalettes[21]!=50)goto done;
    }
    puts("PASS: original same-state A20E -> S12E -> A20E metadata invalidation and original native D2 palette");
    ok=1;
done:
    if(!ok)fputs("FAIL: original native object owner rebind\n",stderr);
    free(scan);return ok;
}

/* Real original Hall entrance door: two accepted turns reach its view from
 * the original start. Its square N/S axis differs from Door.Vertical; no
 * tile, door record, party position or map is replaced by this regression. */
static int check_original_door_orientation(const M11_GameLaunchSpec* spec,
                                           const char* edition)
{
    M11_GameViewState* state=calloc(1,sizeof(*state));
    unsigned char framebuffer[64000];
    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
    unsigned short thing;
    int square, index, ok=0;
    if(!state)return 0;
    M11_GameView_Init(state);
    if(!M11_GameView_Start(state,spec)||!state->world.dungeon||
       state->world.party.mapIndex!=0||state->world.party.mapX!=1||
       state->world.party.mapY!=3||state->world.party.direction!=2)goto done;
    square=state->world.dungeon->tiles[0].squareData[
        state->world.dungeon->maps[0].height+2];
    thing=F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon,state->world.things,0,1,2);
    index=thing&1023;
    if((square>>5)!=DUNGEON_ELEMENT_DOOR||((thing>>10)&15)!=0||
       index>=state->world.things->doorCount||
       ((square&8)!=0)==(state->world.things->doors[index].vertical!=0))goto done;
    if(M11_GameView_HandleInput(state,M12_MENU_INPUT_LEFT)!=M11_GAME_INPUT_REDRAW||
       M11_GameView_HandleInput(state,M12_MENU_INPUT_LEFT)!=M11_GAME_INPUT_REDRAW||
       state->world.party.mapX!=1||state->world.party.mapY!=3||
       state->world.party.direction!=0)goto done;
    M11_GameView_Draw(state,framebuffer,320,200);
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
    if(!receipt.valid||!receipt.planReady||!receipt.sourcePlanDispatched||
       receipt.doorMaterialCallbackStepCount!=1||receipt.doorFrameCallbackStepCount!=1)goto done;
    printf("PASS: original %s actual Hall door square-axis%d animation-axis%d reached by two turns, F0111/frame callbacks=1\n",
        edition,(square&8)!=0,state->world.things->doors[index].vertical!=0);
    ok=1;
done:
    if(!ok)fprintf(stderr,"FAIL: original %s actual door orientation route\n",edition);
    M11_GameView_Shutdown(state);free(state);return ok;
}

/* Original HoC world and material bytes, through the full production Draw.
 * Advance once using accepted input to change source wall parity. */
static int check_original_pc34_d0_walls(const M11_GameLaunchSpec* spec)
{
    M11_GameViewState* state = calloc(1, sizeof(*state));
    unsigned char framebuffer[64000];
    int ok = 0;
    if (!state) return 0;
    M11_GameView_Init(state);
    if (!M11_GameView_Start(state, spec) || !state->world.dungeon) goto done;
    for (int pose = 0; pose < 2; ++pose) {
        M11_Dm1F0128PerSquareSchedulerReceipt receipt;
        const int px = state->world.party.mapX;
        const int py = state->world.party.mapY;
        const int parity = (px + py + state->world.party.direction) & 1;
        const int height = state->world.dungeon->maps[0].height;
        int walls = 0;
        if (px != 1 || py != 3 + pose || state->world.party.direction != 2) goto done;
        M11_GameView_Draw(state, framebuffer, 320, 200);
        M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
        {
            uint8_t rgb8[256][3];
            const int level = M11_GameView_GetDungeonPaletteIndex(state);
            if (level < 0 || level > 5 || !M11_Render_CopyIndexedPaletteRgb8(rgb8)) goto done;
            for (int color = 0; color < 16; ++color) for (int channel = 0; channel < 3; ++channel) {
                const unsigned int dac = G9010_auc_VgaPaletteAll_Compat[level][color][channel] >> 2;
                if (rgb8[color + 16][channel] != (unsigned char)((dac << 2) | (dac >> 4))) goto done;
            }
        }
        for (int right = 0; right < 2; ++right) {
            const int mx = right ? px - 1 : px + 1;
            const unsigned char square = state->world.dungeon->tiles[0].squareData[mx * height + py];
            const unsigned int graphic = parity ? (right ? 94u : 93u) : (right ? 93u : 94u);
            const M11_AssetSlot* slot;
            const int left = right ? 191 : 0;
            if ((square >> 5) != DUNGEON_ELEMENT_WALL) continue;
            ++walls;
            slot = M11_AssetLoader_Load(&state->assetLoader, graphic);
            if (!slot || slot->width != 33 || slot->height != 136) goto done;
            /* I34E VIDEODRV.C F8161 adds16 to C007 viewport indices;
             * assert the source-owned bank as well as all material nibbles. */
            for (int y = 0; y < 136; ++y) for (int x = 0; x < 33; ++x) {
                const unsigned char pixel = slot->pixels[y * 33 + (parity ? 32 - x : x)];
                if (pixel != 10 && framebuffer[(33 + y) * 320 + left + x] != (unsigned char)(pixel + 16)) {
                    fprintf(stderr, "PC34 D0 parity%d side%d pixel%d,%d got%u expected%u callbacks%d mismatch\n", parity, right, x, y, framebuffer[(33 + y) * 320 + left + x], pixel, receipt.d0WallMaterialCallbackStepCount);
                    goto done;
                }
            }
        }
        for (int right = 0; right < 2; ++right) {
            const int mx = right ? px - 1 : px + 1;
            const int square = state->world.dungeon->tiles[0].squareData[mx * height + py + 2];
            const unsigned int graphic = parity ? (right ? 101u : 100u) : (right ? 100u : 101u);
            const M11_AssetSlot* slot = M11_AssetLoader_Load(&state->assetLoader, graphic);
            if ((square >> 5) != DUNGEON_ELEMENT_WALL) continue;
            if (!slot || slot->width != 78 || slot->height != 74) goto done;
            /* D1L/R cover the adjacent60columns; these18columns expose
             * C710/C711's full native width and authored y19 anchor. */
            for (int y = 0; y < 74; ++y) for (int x = 0; x < 18; ++x) {
                const int dx = right ? 146 + x : 60 + x;
                const int localX = right ? x : 60 + x;
                const int sx = parity ? 77 - localX : localX;
                const unsigned char pixel = slot->pixels[y * 78 + sx];
                if (pixel != 10 && framebuffer[(52 + y) * 320 + dx] != (unsigned char)(pixel + 16)) {
                    fprintf(stderr, "PC34 route%d D2 side%d parity%d pixel%d,%d mismatch\n", state->sourceKind, right, parity, dx, 52 + y);
                    goto done;
                }
            }
        }
        if (!walls || receipt.d0WallMaterialCallbackStepCount != walls) goto done;
        printf("PASS: original PC34 HoC(%d,%d,S) parity%d D0 wall callbacks%d, complete native C716/C717 and exposed C710/C711 pixels route%d\n", px, py, parity, walls, state->sourceKind);
        if (!pose && M11_GameView_HandleInput(state, M12_MENU_INPUT_UP) != M11_GAME_INPUT_REDRAW) goto done;
    }
    ok = 1;
done:
    if (!ok) fprintf(stderr, "FAIL: original PC34 D0 wall production route\n");
    M11_GameView_Shutdown(state);
    free(state);
    return ok;
}

static const char* pc34_graphics_path(void)
{
    static char path[2048];
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    if (!root || !root[0]) return NULL;
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    return path;
}

static int check_original_d2_pixels(M11_GameViewState* state)
{
    unsigned char framebuffer[320 * 200];
    int right, flip;
    DM1_ViewportLaneVisibilityReceiptPc34 visibility =
        dm1_viewport_3d_lane_visibility_from_cells_pc34(NULL, NULL, NULL, NULL, NULL);
    for (right = 0; right < 2; ++right) {
        for (flip = 0; flip < 2; ++flip) {
            DM1_ViewportSideWallHostReceiptPc34 receipt;
            const M11_AssetSlot* source;
            int x, y, opaque = 0, transparent = 0;
            const int left = right ? 146 : 0;
            if (!dm1_viewport_3d_build_side_wall_host_receipt_pc34(
                    right ? DM1_VIEW_SQUARE_D2R : DM1_VIEW_SQUARE_D2L,
                    0, flip != 0, true, false, 3, &visibility, &receipt)) return 0;
            source = M11_AssetLoader_Load(&state->assetLoader,
                                          (unsigned int)receipt.material.graphic_index);
            if (!source || source->width != 78 || source->height != 74) return 0;
            memset(framebuffer, 0xe7, sizeof(framebuffer));
            if (!M11_GameView_ProbeDrawDm1SideWallHostReceipt(
                    state, &receipt, framebuffer, 320, 200)) return 0;
            /* Independent original C696 oracle: C710 left0/y19, C711
             * right223/y19, native78x74. Check the entire destination,
             * including all four edges and C10 holes, against decoded
             * original pixels. The old75x71 crop cannot satisfy this. */
            for (y = 0; y < 200; ++y) {
                for (x = 0; x < 320; ++x) {
                    unsigned char expected = 0xe7;
                    if (x >= left && x < left + 78 && y >= 33 + 19 && y < 33 + 19 + 74) {
                        const int sx = flip ? 77 - (x - left) : x - left;
                        unsigned char pixel = source->pixels[(y - 33 - 19) * 78 + sx];
                        if (pixel != 10) { expected = pixel; ++opaque; }
                        else ++transparent;
                    }
                    if (framebuffer[y * 320 + x] != expected) {
                        fprintf(stderr, "D2%s flip%d pixel%d,%d got%u expected%u\n",
                            right ? "R" : "L", flip, x, y,
                            framebuffer[y * 320 + x], expected);
                        return 0;
                    }
                }
            }
            if (!opaque || !transparent) return 0;
            printf("PASS: original D2%s flip=%d rect=(%d,19,78,74) opaque=%d C10holes=%d\n",
                right ? "R" : "L", flip, left, opaque, transparent);
        }
    }
    return 1;
}

/* The archive is original A20E media; production startup supplies both the
 * dungeon map/floor-set and its admitted graphics loader. No fixture map or
 * reconstructed raster is used. */
static int check_original_amiga_background(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_AMIGA_BACKGROUND_ARCHIVE");
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    const M11_AssetSlot *floor, *ceiling;
    int parity, x, y, ok = 0, index;
    M12_AssetStatus* scan;
    const M12_AssetVersionStatus* version;
    M11_GameLaunchSpec spec = {0};
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    if (!archive || !archive[0]) return 1;
    scan = calloc(1, sizeof(*scan));
    if (!scan) return 0;
    M11_GameView_Init(&state);
    M12_AssetStatus_ScanGame(scan, archive, "dm1");
    index = M12_AssetStatus_FindVersionIndex("dm1", "amiga20-en");
    version = index < 0 ? NULL : M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
    if (!version || !version->matched ||
        strcmp(version->matchedMd5, "6a2f135b53c2220f0251fa103e2a6e7e") ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(scan, "amiga20-en", runtime, sizeof(runtime))) goto done;
    spec.title = "Dungeon Master";
    spec.gameId = spec.sourceId = "dm1";
    spec.dataDir = runtime;
    spec.verifiedAssetPath = version->matchedPath;
    spec.verifiedAssetMd5 = version->matchedMd5;
    if (!M11_GameView_Start(&state, &spec) || !state.assetsAvailable ||
        !state.assetLoader.legacyDm1 || !state.assetLoader.legacyBigEndian ||
        !state.world.dungeon || !state.world.dungeon->maps ||
        state.world.dungeon->maps[state.world.party.mapIndex].floorSet != 0 ||
        !asset_file_matches_md5(state.assetLoader.graphicsDatPath,
                               "6a2f135b53c2220f0251fa103e2a6e7e")) goto done;
    if (!check_original_door_orientation(&spec,"A20E") ||
        !check_original_native_objects(&state,"A20E") ||
        !check_original_native_item_walk(&spec,"A20E")) goto done;
    floor = M11_AssetLoader_Load(&state.assetLoader, 75u);
    ceiling = M11_AssetLoader_Load(&state.assetLoader, 76u);
    if (!floor || !ceiling || floor->width != 224 || floor->height != 70 ||
        ceiling->width != 224 || ceiling->height != 29) goto done;
    /* The original78/79 are door frames, not an alternate floor pair.
     * Their real dimensions cannot satisfy the complete75/76 raster oracle. */
    for (x = 78; x <= 79; ++x) {
        const M11_AssetSlot* wrong = M11_AssetLoader_Load(&state.assetLoader, (unsigned)x);
        if (!wrong || !wrong->pixels ||
            (wrong->width == 224 && (wrong->height == 70 || wrong->height == 29))) goto done;
    }
    for (parity = 0; parity < 2; ++parity) {
        state.world.party.mapX = parity;
        state.world.party.mapY = state.world.party.direction = 0;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_ProbeDrawDm1ViewportBackground(&state, framebuffer, 320, 200);
        for (y = 0; y < 200; ++y) for (x = 0; x < 320; ++x) {
            unsigned char expected = 0;
            if (x < 224 && y >= 33 && y < 33 + 29) {
                const int sx = parity ? x : 223 - x;
                expected = ceiling->pixels[(y - 33) * 224 + sx];
            } else if (x < 224 && y >= 33 + 66 && y < 33 + 136) {
                const int sx = parity ? 223 - x : x;
                expected = floor->pixels[(y - 33 - 66) * 224 + sx];
            }
            if (framebuffer[y * 320 + x] != expected) {
                fprintf(stderr, "Amiga F0098 parity%d pixel%d,%d got%u expected%u\n",
                    parity, x, y, framebuffer[y * 320 + x], expected);
                goto done;
            }
        }
        printf("PASS: original A20E F0098 parity%d floor75=224x70 ceiling76=224x29\n", parity);
    }
    if (!check_original_legacy_materials(&state,"A20E") ||
        !check_original_native_object_rebind(&state,&spec) ||
        !asset_file_matches_md5(state.assetLoader.graphicsDatPath,
                               "6a2f135b53c2220f0251fa103e2a6e7e")) goto done;
    ok = 1;
done:
    M11_GameView_Shutdown(&state);
    free(scan);
    if (!ok) fprintf(stderr, "original A20E background oracle failed\n");
    return ok;
}

/* Original MEDIA009 C013, not a generated arrow raster. Optional so the
 * ordinary PC-only test keeps its existing corpus requirement. */
static unsigned int source_pixel_hash(const M11_AssetSlot* slot)
{
    unsigned int hash=2166136261u;
    for (size_t i=0; i<(size_t)slot->width*slot->height; ++i) hash=(hash^slot->pixels[i])*16777619u;
    return hash;
}

/* Independent MEDIA009 DUNVIEW.C:526-717 original frame oracle. */
static const int oracle_g0110[8] = {0,79,25,70,40,46,0,0};
static const int oracle_g0111[8] = {64,159,25,70,48,46,0,0};
static const int oracle_g0112[8] = {149,223,25,70,40,46,5,0};
static const int oracle_g0113[8] = {0,63,22,83,32,62,0,0};
static const int oracle_g0114[8] = {64,159,22,83,48,62,0,0};
static const int oracle_g0115[8] = {160,223,22,83,32,62,0,0};
static const int oracle_g0116[8] = {0,31,9,108,16,100,0,0};
static const int oracle_g0117[8] = {32,191,9,108,80,100,0,0};
static const int oracle_g0118[8] = {192,223,9,108,16,100,0,0};
static const int oracle_g0119[8] = {0,31,58,101,16,44,0,0};
static const int oracle_g0120[8] = {192,223,58,101,16,44,0,0};
static const int oracle_g0121[8] = {0,79,28,68,40,41,0,0};
static const int oracle_g0122[8] = {64,159,28,70,48,43,0,0};
static const int oracle_g0123[8] = {149,223,28,68,40,41,5,0};
static const int oracle_g0124[8] = {0,63,24,85,32,62,0,0};
static const int oracle_g0125[8] = {64,159,24,85,48,62,0,0};
static const int oracle_g0126[8] = {160,223,24,85,32,62,0,0};
static const int oracle_g0127[8] = {0,31,18,108,16,91,0,0};
static const int oracle_g0128[8] = {32,191,18,108,80,91,0,0};
static const int oracle_g0129[8] = {192,223,18,108,16,91,0,0};
static const int oracle_g0130[8] = {0,31,76,135,16,60,0,0};
static const int oracle_g0131[8] = {192,223,76,135,16,60,0,0};
static const int oracle_g0132[8] = {60,75,57,61,8,5,0,0};
static const int oracle_g0133[8] = {148,163,57,61,8,5,0,0};
static const int oracle_g0134[8] = {32,63,57,99,16,43,0,0};
static const int oracle_g0135[8] = {160,191,57,99,16,43,0,0};
static const int oracle_g0136[8] = {32,63,60,98,16,39,0,0};
static const int oracle_g0137[8] = {160,191,60,98,16,39,0,0};
static const int oracle_g0138[8] = {0,15,73,85,8,13,0,0};
static const int oracle_g0139[8] = {208,223,73,85,8,13,0,0};
static const int oracle_g0164[8] = {0,31,28,70,16,43,0,0};
static const int oracle_g0165[8] = {192,223,28,70,16,43,0,0};
static const int oracle_g0166[8] = {64,95,27,70,16,44,0,0};
static const int oracle_g0167[8] = {128,159,27,70,16,44,0,0};
static const int oracle_g0168[8] = {48,95,22,86,24,65,0,0};
static const int oracle_g0169[8] = {128,175,22,86,24,65,0,0};
static const int oracle_g0170[8] = {43,74,14,107,16,94,0,0};
static const int oracle_g0171[8] = {149,180,14,107,16,94,0,0};
static const int oracle_g0172[8] = {96,127,0,122,16,123,0,0};
static const int oracle_g0173[8] = {0,59,22,24,48,3,16,0};
static const int oracle_g0174[8] = {64,159,22,24,48,3,0,0};
static const int oracle_g0175[8] = {164,223,22,24,48,3,16,0};
static const int oracle_g0176[8] = {0,31,14,17,64,4,16,0};
static const int oracle_g0177[8] = {48,175,14,17,64,4,0,0};
static const int oracle_g0178[8] = {192,223,14,17,64,4,16,0};
static const int oracle_g0179[80] = {24,71,28,67,24,41,0,0,24,71,28,38,24,41,0,30,24,71,28,48,24,41,0,20,24,71,28,58,24,41,0,10,24,29,28,67,24,41,18,0,24,35,28,67,24,41,12,0,24,41,28,67,24,41,6,0,66,71,28,67,24,41,24,0,60,71,28,67,24,41,24,0,54,71,28,67,24,41,24,0};
static const int oracle_g0180[80] = {88,135,28,67,24,41,0,0,88,135,28,38,24,41,0,30,88,135,28,48,24,41,0,20,88,135,28,58,24,41,0,10,88,93,28,67,24,41,18,0,88,99,28,67,24,41,12,0,88,105,28,67,24,41,6,0,130,135,28,67,24,41,24,0,124,135,28,67,24,41,24,0,118,135,28,67,24,41,24,0};
static const int oracle_g0181[80] = {150,197,28,67,24,41,0,0,150,197,28,38,24,41,0,30,150,197,28,48,24,41,0,20,150,197,28,58,24,41,0,10,150,153,28,67,24,41,18,0,150,161,28,67,24,41,12,0,150,167,28,67,24,41,6,0,192,197,28,67,24,41,24,0,186,197,28,67,24,41,24,0,180,197,28,67,24,41,24,0};
static const int oracle_g0182[80] = {0,63,24,82,32,61,0,0,0,63,24,39,32,61,0,45,0,63,24,54,32,61,0,30,0,63,24,69,32,61,0,15,0,7,24,82,32,61,24,0,0,15,24,82,32,61,16,0,0,23,24,82,32,61,8,0,56,63,24,82,32,61,32,0,48,63,24,82,32,61,32,0,40,63,24,82,32,61,32,0};
static const int oracle_g0183[80] = {80,143,24,82,32,61,0,0,80,143,24,39,32,61,0,45,80,143,24,54,32,61,0,30,80,143,24,69,32,61,0,15,80,87,24,82,32,61,24,0,80,95,24,82,32,61,16,0,80,103,24,82,32,61,8,0,136,143,24,82,32,61,32,0,128,143,24,82,32,61,32,0,120,143,24,82,32,61,32,0};
static const int oracle_g0184[80] = {160,223,24,82,32,61,0,0,160,223,24,39,32,61,0,45,160,223,24,54,32,61,0,30,160,223,24,69,32,61,0,15,160,167,24,82,32,61,24,0,160,175,24,82,32,61,16,0,160,183,24,82,32,61,8,0,216,223,24,82,32,61,32,0,208,223,24,82,32,61,32,0,200,223,24,82,32,61,32,0};
static const int oracle_g0185[80] = {0,31,17,102,48,88,64,0,0,31,17,38,48,88,64,66,0,31,17,60,48,88,64,44,0,31,17,82,48,88,64,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,31,17,102,48,88,48,0,8,31,17,102,48,88,48,0,0,31,17,102,48,88,52,0};
static const int oracle_g0186[80] = {64,159,17,102,48,88,0,0,64,159,17,38,48,88,0,66,64,159,17,60,48,88,0,44,64,159,17,82,48,88,0,22,64,75,17,102,48,88,36,0,64,87,17,102,48,88,24,0,64,99,17,102,48,88,12,0,148,159,17,102,48,88,48,0,136,159,17,102,48,88,48,0,124,159,17,102,48,88,48,0};
static const int oracle_g0187[80] = {192,223,17,102,48,88,0,0,192,223,17,38,48,88,0,66,192,223,17,60,48,88,0,44,192,223,17,82,48,88,0,22,192,203,17,102,48,88,36,0,192,215,17,102,48,88,24,0,192,223,17,102,48,88,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static void oracle_native_frame(unsigned char* output,
    const unsigned char* pixels,int width,int height,const int* frame,int flip)
{
    if (!frame[4]) return;
    for(int y=frame[2];y<=frame[3];++y) for(int x=frame[0];x<=frame[1];++x) {
        int sx=frame[6]+x-frame[0],sy=frame[7]+y-frame[2];
        if(flip&1)sx=width-1-sx;
        if(flip&2)sy=height-1-sy;
        if(sx>=0&&sx<width&&sy>=0&&sy<height&&x<224&&y<136&&pixels[sy*width+sx]!=10)
            output[(y+33)*320+x]=pixels[sy*width+sx];
    }
}
static int oracle_native_record(M11_GameViewState* state,unsigned char* output,
    int graphic,const int* frame,int flip)
{
    const M11_AssetSlot* source=M11_AssetLoader_Load(&state->assetLoader,(unsigned)graphic);
    if(!source||!source->pixels||source->width!=frame[4]*2||source->height!=frame[5])return 0;
    oracle_native_frame(output,source->pixels,source->width,source->height,frame,flip);
    return 1;
}
static int oracle_native_mask(M11_GameViewState* state,unsigned char* pixels,
    int width,int height,int graphic,int view)
{
    static const int palette[2][16]={
        {0,12,1,3,4,3,0,6,3,9,10,11,0,2,0,13},
        {0,1,2,3,4,3,6,7,5,9,10,11,12,13,14,15}};
    const M11_AssetSlot* source=M11_AssetLoader_Load(&state->assetLoader,(unsigned)graphic);
    if(!source||!source->pixels||source->width!=96||source->height!=88)return 0;
    /* G0207 set1 covers the entire native door. F0129 samples source-cell
     * centres with a truncated 10-bit increment before the C9 blit. */
    unsigned xr=((96u<<10)/(unsigned)width)<<6,yr=((88u<<10)/(unsigned)height)<<6;
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){
        unsigned sx=(xr/2+32767u+(unsigned)x*xr)>>16;
        unsigned sy=(yr/2+32767u+(unsigned)y*yr)>>16;
        unsigned char pixel=source->pixels[sy*96+sx];
        if(view<2)pixel=(unsigned char)palette[view][pixel&15];
        if(pixel!=9)pixels[y*width+x]=pixel;
    }
    return 1;
}

static int check_original_legacy_materials(M11_GameViewState* state,const char* edition)
{
    static const int* front[2][11]={
        {oracle_g0110,oracle_g0111,oracle_g0112,oracle_g0113,oracle_g0114,oracle_g0115,
         oracle_g0116,oracle_g0117,oracle_g0118,oracle_g0119,oracle_g0120},
        {oracle_g0121,oracle_g0122,oracle_g0123,oracle_g0124,oracle_g0125,oracle_g0126,
         oracle_g0127,oracle_g0128,oracle_g0129,oracle_g0130,oracle_g0131}};
    static const int* lateral[8]={oracle_g0132,oracle_g0133,oracle_g0134,oracle_g0135,
        oracle_g0136,oracle_g0137,oracle_g0138,oracle_g0139};
    static const int* doorFrames[3][3]={
        {oracle_g0185,oracle_g0186,oracle_g0187},
        {oracle_g0182,oracle_g0183,oracle_g0184},
        {oracle_g0179,oracle_g0180,oracle_g0181}};
    static const int* left[3]={oracle_g0170,oracle_g0168,oracle_g0166};
    static const int* right[3]={oracle_g0171,oracle_g0169,oracle_g0167};
    unsigned char output[64000],expected[64000],pixels[96*88];
    unsigned int hashes[43];
    M11_Dm1LegacyMaterialProbe input={0};
    int mapIndex=state->world.party.mapIndex,checks=0;
    struct RngState_Compat savedRng=state->world.masterRng;
    const int wallSet=state->world.dungeon->maps[mapIndex].wallSet;
    if(wallSet!=0)return 0;
    for(int record=77;record<120;++record){
        const M11_AssetSlot* source=M11_AssetLoader_Load(&state->assetLoader,(unsigned)record);
        if(!source||!source->pixels)return 0;
        hashes[record-77]=source_pixel_hash(source);
    }
    for(int down=0;down<2;++down)for(int facing=0;facing<2;++facing)
    for(int forward=0;forward<=3;++forward)for(int side=-1;side<=1;++side){
        int member,index; const int* frame;
        if((!forward&&!side)||(!facing&&(forward==3||!side)))continue;
        input.kind=0;input.forward=forward;input.side=side;
        input.square=0x60|(down?0:4)|
            (((state->world.party.direction&1)==0)==facing?8:0);
        if(facing){
            index=forward?(3-forward)*3+side+1:(side<0?9:10);
            member=(forward?(3-forward)*2+(side==0):6)+down*7;
            frame=front[down][index];
        }else{
            index=forward==2?(side>0):forward==1?2+down*2+(side>0):6+(side>0);
            member=forward==2?14:forward==1?15+down:17;
            frame=lateral[index];
        }
        memset(output,0xe7,sizeof(output));memset(expected,0xe7,sizeof(expected));
        if(!oracle_native_record(state,expected,90+member,frame,side>0)||
           !M11_GameView_ProbeDrawDm1LegacyMaterial(state,&input,output,320,200)||
           memcmp(output,expected,sizeof(output)))goto failed;
        ++checks;
    }
    input.kind=1;
    input.square=0x80|((state->world.party.direction&1)?8:0);
    for(int forward=0;forward<=3;++forward)for(int side=-1;side<=1;++side){
        if(!forward&&side)continue;
        input.forward=forward;input.side=side;
        memset(output,0xe7,sizeof(output));memset(expected,0xe7,sizeof(expected));
        if(!forward){if(!oracle_native_record(state,expected,77,oracle_g0172,0))goto failed;}
        else if(!side){
            if(!oracle_native_record(state,expected,77+forward,left[forward-1],0)||
               !oracle_native_record(state,expected,77+forward,right[forward-1],1))goto failed;
            if(forward<3&&!oracle_native_record(state,expected,forward==1?82:83,
                forward==1?oracle_g0177:oracle_g0174,0))goto failed;
        }else if(forward==3){
            if(!oracle_native_record(state,expected,81,side<0?oracle_g0164:oracle_g0165,side>0))goto failed;
        }else if(!oracle_native_record(state,expected,forward==1?82:83,
            forward==1?(side<0?oracle_g0176:oracle_g0178):(side<0?oracle_g0173:oracle_g0175),0))goto failed;
        if(!M11_GameView_ProbeDrawDm1LegacyMaterial(state,&input,output,320,200)||
           memcmp(output,expected,sizeof(output)))goto failed;
        ++checks;
    }
    {
        int direction=state->world.party.direction;
        input.forward=input.side=0;
        for(int axis=0;axis<2;++axis)for(int facing=0;facing<4;++facing){
            int sourceSide=axis==(facing&1);
            state->world.party.direction=facing;input.square=0x80|(axis?8:0);
            memset(output,0xe7,sizeof(output));memset(expected,0xe7,sizeof(expected));
            if(sourceSide&&!oracle_native_record(state,expected,77,oracle_g0172,0))goto failed;
            int drawn=M11_GameView_ProbeDrawDm1LegacyMaterial(state,&input,output,320,200);
            if((drawn!=0)!=sourceSide||memcmp(output,expected,sizeof(output)))goto failed;
            ++checks;
        }
        state->world.party.direction=direction;
    }
    input.kind=2;
    for(int set=0;set<4;++set){
        int found=0;
        for(int map=0;map<state->world.dungeon->header.mapCount&&!found;++map){
            const struct DungeonMapDesc_Compat* desc=&state->world.dungeon->maps[map];
            for(int type=0;type<2;++type)if((type?desc->doorSet1:desc->doorSet0)==set){
                state->world.party.mapIndex=map;input.doorType=type;found=1;break;
            }
        }
        if(!found){fprintf(stderr,"missing authentic map DoorSet%d\n",set);goto failed;}
        for(int forward=1;forward<=3;++forward)for(int side=-1;side<=1;++side)
        for(int vertical=0;vertical<2;++vertical)for(int doorState=0;doorState<=5;++doorState){
            const int* table=doorFrames[forward-1][side+1];
            const M11_AssetSlot* source=M11_AssetLoader_Load(&state->assetLoader,(unsigned)(108+set*3+3-forward));
            struct RngState_Compat expectedRng=state->world.masterRng;
            int flip=0,first=0,last=0;
            input.forward=forward;input.side=side;input.doorVertical=vertical;input.doorState=doorState;
            if(!source||source->width!=table[4]*2||source->height!=table[5])goto failed;
            memset(output,0xe7,sizeof(output));memset(expected,0xe7,sizeof(expected));
            memcpy(pixels,source->pixels,(size_t)source->width*source->height);
            if(doorState){
                if(set==3){
                    flip=F0732_COMBAT_RngRandom_Compat(&expectedRng,2)!=0;
                    flip|=(F0732_COMBAT_RngRandom_Compat(&expectedRng,2)!=0)*2;
                    for(int y=0;y<source->height;++y)for(int x=0;x<source->width;++x)
                        pixels[y*source->width+x]=source->pixels[
                            ((flip&2)?source->height-1-y:y)*source->width+
                            ((flip&1)?source->width-1-x:x)];
                }
                if(doorState==5&&!oracle_native_mask(state,pixels,source->width,source->height,301,3-forward))goto failed;
                if(doorState<4){first=doorState+(vertical?0:3);last=vertical?first:first+3;}
                for(int row=first;row<=last;row+=3)
                    oracle_native_frame(expected,pixels,source->width,source->height,table+row*8,0);
            }
            if(!M11_GameView_ProbeDrawDm1LegacyMaterial(state,&input,output,320,200)||
               memcmp(output,expected,sizeof(output))||
               memcmp(&expectedRng,&state->world.masterRng,sizeof(expectedRng)))goto failed;
            ++checks;
        }
    }
    state->world.party.mapIndex=mapIndex;state->world.masterRng=savedRng;
    for(int record=77;record<120;++record)if(hashes[record-77]!=source_pixel_hash(
        M11_AssetLoader_Load(&state->assetLoader,(unsigned)record)))goto failed;
    printf("PASS: original %s MEDIA009 %d full-raster stairs/frame/door state cases, native pixels unchanged\n",edition,checks);
    return 1;
failed:
    fprintf(stderr,"FAIL: original %s MEDIA009 material kind%d forward%d side%d state%d vertical%d after%d cases\n",
        edition,input.kind,input.forward,input.side,input.doorState,input.doorVertical,checks);
    state->world.party.mapIndex=mapIndex;state->world.masterRng=savedRng;
    return 0;
}

/* Independent G0163/F0096 oracle, using only pixels decoded from originals.
 * DUNVIEW.C:581-594 and2389-2408 specify padded source strides and crops. */
static int check_original_legacy_walls(M11_GameViewState* state, const char* edition)
{
    static const struct {
        DM1_ViewSquareIndex square; int record, stride, height, left, top, width, sourceX; int center;
    } cases[] = {
        {DM1_VIEW_SQUARE_D3L,88,128,51,0,25,84,32,0},
        {DM1_VIEW_SQUARE_D3C,88,128,51,74,25,76,18,1},
        {DM1_VIEW_SQUARE_D3R,88,128,51,139,25,85,0,0},
        {DM1_VIEW_SQUARE_D2L,87,144,71,0,20,75,61,0},
        {DM1_VIEW_SQUARE_D2C,87,144,71,60,20,104,16,1},
        {DM1_VIEW_SQUARE_D2R,87,144,71,149,20,75,0,0},
        {DM1_VIEW_SQUARE_D1L,86,256,111,0,9,64,192,0},
        {DM1_VIEW_SQUARE_D1C,86,256,111,32,9,160,48,1},
        {DM1_VIEW_SQUARE_D1R,86,256,111,160,9,64,0,0},
        {DM1_VIEW_SQUARE_D0L,85,32,136,0,0,32,0,0},
        {DM1_VIEW_SQUARE_D0R,84,32,136,192,0,32,0,0}
    };
    const M11_AssetSlot* originals[5];
    unsigned int hashes[5];
    unsigned char framebuffer[320*200];
    int mapX=state->world.party.mapX, mapY=state->world.party.mapY;
    int direction=state->world.party.direction;
    int wallSet=state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
    for (int i=0;i<5;++i) {
        originals[i]=M11_AssetLoader_Load(&state->assetLoader,(unsigned int)(84+i+wallSet*13));
        if (!originals[i] || !originals[i]->pixels) return 0;
        hashes[i]=source_pixel_hash(originals[i]);
    }
    /* The preceding full Draw may already own the real cache. Release it
     * before deliberately exercising a failed-allocation capability. */
    free(state->dm1LegacyWallCache);
    state->dm1LegacyWallCache = NULL;
    for (int capability=0;capability<2;++capability) {
        /* Deliberately exercise the allocation-failure/native capability
         * state. This changes ownership policy, never original media bytes. */
        state->dm1LegacyWallCacheCapability=capability ? 0 : -1;
        for (int parity=0;parity<2;++parity) for (size_t c=0;c<sizeof(cases)/sizeof(cases[0]);++c) {
            const int flipped=capability && parity;
            int record=cases[c].record;
            const M11_AssetSlot* source;
            state->world.party.mapX=parity;
            state->world.party.mapY=state->world.party.direction=0;
            if (flipped && record==84) record=85;
            else if (flipped && record==85) record=84;
            source=originals[record-84];
            if (source->width!=cases[c].stride || source->height!=cases[c].height) return 0;
            memset(framebuffer,0xe7,sizeof(framebuffer));
            if (!M11_GameView_ProbeDrawDm1LegacyWall(state,cases[c].square,framebuffer,320,200) ||
                state->dm1LegacyWallCacheCapability!=(capability ? 1 : -1)) return 0;
            for (int y=0;y<200;++y) for (int x=0;x<320;++x) {
                unsigned char expected=0xe7;
                if (x>=cases[c].left && x<cases[c].left+cases[c].width &&
                    y>=33+cases[c].top && y<33+cases[c].top+cases[c].height) {
                    int sx=cases[c].sourceX+x-cases[c].left;
                    const int sy=y-33-cases[c].top;
                    unsigned char pixel;
                    if (flipped) {
                        int content=record==88 ? 116 : record==87 ? 136 : cases[c].stride;
                        int shift=record==88 ? 11 : record==87 ? 8 : 0;
                        pixel=sx>=content ? 10 : source->pixels[sy*cases[c].stride+cases[c].stride-1-sx-shift];
                    } else pixel=source->pixels[sy*cases[c].stride+sx];
                    if (cases[c].center || pixel!=10) expected=pixel;
                }
                if (framebuffer[y*320+x]!=expected) {
                    fprintf(stderr,"%s legacy wall capability%d parity%d case%zu pixel%d,%d got%u expected%u\n",
                        edition,capability,parity,c,x,y,framebuffer[y*320+x],expected);
                    return 0;
                }
            }
        }
        printf("PASS: original %s MEDIA009 capability%d eleven full wall rasters both parities\n",edition,capability);
    }
    for (int i=0;i<5;++i) if (source_pixel_hash(originals[i])!=hashes[i]) return 0;
    state->world.party.mapX=mapX;
    state->world.party.mapY=mapY;
    state->world.party.direction=direction;
    return 1;
}

/* Original demo dm contains the complete source G0021 RGB4 table at
 * file0x1142. Compare the renderer's actual RGB8 palette, not a copy of the
 * C constants, and preserve the real frame's pixel indices through copper
 * tagging at all six source light levels and the inventory switch. */
static int check_original_amiga_palette(
    M11_GameViewState* state, const char* archive,
    const unsigned char* originalFrame)
{
    char programPath[M12_ASSET_DATA_DIR_CAPACITY];
    unsigned char* program = NULL;
    size_t programBytes = 0;
    unsigned char frame[320 * 200];
    uint8_t rgb8[256][3];
    int ok = 0;
    const int length = snprintf(programPath, sizeof(programPath), "%s::dm", archive);
    if (length < 0 || (size_t)length >= sizeof(programPath) ||
        !asset_file_matches_md5(programPath,"f402d7fe70d1df20141e6e520a242f74") ||
        !asset_read_path_alloc(programPath,&program,&programBytes) ||
        programBytes != 163236u) goto done;
    for (int inventory = 0; inventory < 2; ++inventory) {
        for (int level = 0; level < 6; ++level) {
            memcpy(frame,originalFrame,sizeof(frame));
            if (!M11_GameView_ProbeApplyDm1AmigaRuntimePalette(
                    frame,320,200,level,inventory) ||
                !M11_Render_CopyIndexedPaletteRgb8(rgb8)) goto done;
            for (int raw = 0; raw < 256; ++raw) {
                const int bank = raw >> 4;
                const size_t offset = 0x1142u + 2u *
                    ((size_t)(bank >= 1 && bank <= 6 ? bank - 1 : 0) * 16u + (unsigned)(raw & 15));
                const unsigned int source = ((unsigned)program[offset] << 8) | program[offset+1];
                for (int channel = 0; channel < 3; ++channel) {
                    const unsigned char expected = (unsigned char)(((source >> (8-4*channel)) & 15u) * 17u);
                    if (rgb8[raw][channel] != expected) goto done;
                }
            }
            for (int y = 0; y < 200; ++y) {
                const int bank = !inventory && y >= 31 && y < 170 ? level + 1 : 0;
                for (int x = 0; x < 320; ++x) {
                    const unsigned char expected = (unsigned char)
                        ((bank << 4) | (originalFrame[y*320+x] & 15));
                    if (frame[y*320+x] != expected) goto done;
                }
            }
        }
    }
    /* Verify the public live Draw actually publishes the source palette
     * and full-width band tags, rather than only the test producer. */
    M11_GameView_Draw(state,frame,320,200);
    if (!M11_Render_CopyIndexedPaletteRgb8(rgb8)) goto done;
    for (int y = 0; y < 200; ++y) {
        const int bank = y >= 31 && y < 170 ? M11_GameView_GetDungeonPaletteIndex(state) + 1 : 0;
        for (int x = 0; x < 320; ++x) {
            if ((frame[y*320+x] >> 4) != bank) goto done;
        }
    }
    if (rgb8[15][0] != 255 || rgb8[15][1] != 255 || rgb8[15][2] != 255 ||
        rgb8[2][0] != 136 || rgb8[2][1] != 136 || rgb8[2][2] != 136) goto done;
    puts("PASS: original demo executable RGB4 palette all256 RGB8 entries, six light levels, copper boundaries and inventory switch");
    ok = 1;
done:
    free(program);
    if (!ok) fprintf(stderr,"original Amiga palette/copper oracle failed\n");
    return ok;
}

static int check_original_legacy_arrows(const char *env_name,
                                        const char *edition,
                                        const char *graphics_md5,
                                        int atari)
{
    const char *path = getenv(env_name);
    const M12_AssetVersionStatus *version;
    M12_AssetStatus *scan;
    M11_GameViewState *state;
    M11_GameLaunchSpec spec = {0};
    const M11_AssetSlot *arrows;
    unsigned char framebuffer[320 * 200];
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    int index, result = 0, nonblack = 0;
    if (!path || !path[0]) return 1;
    scan = calloc(1, sizeof(*scan));
    state = calloc(1, sizeof(*state));
    if (!scan || !state) { free(scan); free(state); return 0; }
    M11_GameView_Init(state);
    M12_AssetStatus_ScanGame(scan, path, "dm1");
    index = M12_AssetStatus_FindVersionIndex("dm1", edition);
    version = index < 0 ? NULL :
        M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
    if (!version || !version->matched ||
        strcmp(version->matchedMd5, graphics_md5) ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(scan, edition,
                                               runtime, sizeof(runtime))) goto done;
    spec.title = "Dungeon Master";
    spec.gameId = "dm1";
    spec.sourceId = "dm1";
    spec.dataDir = runtime;
    spec.verifiedAssetPath = version->matchedPath;
    spec.verifiedAssetMd5 = version->matchedMd5;
    if (!M11_GameView_Start(state, &spec) ||
        (atari ? !state->assetLoader.atariStDm1 :
         (!state->assetLoader.legacyDm1 || !state->assetLoader.legacyBigEndian)))
        goto done;
    arrows = M11_AssetLoader_Load(&state->assetLoader, 13);
    if (!arrows || !arrows->loaded || !arrows->pixels ||
        arrows->width != 96 || arrows->height != 45) goto done;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    for (int y = 0; y < 45; ++y) {
        for (int x = 0; x < 96; ++x) {
            unsigned char source = arrows->pixels[y * 96 + x] & 15;
            if ((framebuffer[(124 + y) * 320 + 224 + x] & 15) != source)
                goto done;
            nonblack += source != 0;
        }
    }
    if (!nonblack) goto done;
    if (!check_original_legacy_walls(state,edition)) goto done;
    if (atari && (!check_original_door_orientation(&spec,edition) ||
        !check_original_native_item_walk(&spec,edition) ||
        !check_original_native_objects(state,edition))) goto done;
    if (atari && (!check_original_legacy_materials(state,edition) ||
        !asset_file_matches_md5(state->assetLoader.graphicsDatPath,graphics_md5))) goto done;
    if (!atari && !check_original_amiga_palette(state,path,framebuffer)) goto done;
    printf("PASS: original %s C013 matches all 4320 presented indices\n", edition);
    {
        static const int turn_x[4] = { 257, 243, 314, 300 };
        for (int click = 0; click < 4; ++click) {
            int direction, expected;
            M11_GameView_Shutdown(state);
            M11_GameView_Init(state);
            if (!M11_GameView_Start(state, &spec)) goto done;
            direction = state->world.party.direction;
            expected = (direction + (click < 2 ? 3 : 1)) & 3;
            if (M11_GameView_HandlePointer(state, turn_x[click], 135, 1) !=
                    M11_GAME_INPUT_REDRAW ||
                state->world.party.direction != expected) {
                fprintf(stderr, "FAIL: %s turn click at %d,135 direction %d expected %d\n",
                    edition, turn_x[click], state->world.party.direction, expected);
                goto done;
            }
        }
        printf("PASS: original %s native turn edges and centers change direction\n", edition);
    }
    result = 1;
done:
    if (!result) fprintf(stderr, "FAIL: original %s native movement arrows\n", edition);
    M11_GameView_Shutdown(state);
    free(state);
    free(scan);
    return result;
}

int main(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_Dm1WallOrnamentHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!check_original_amiga_background()) return 1;
    if (!check_original_legacy_arrows("FIRESTAFF_DM1_AMIGA_DEMO_ARROWS_ARCHIVE",
            "amiga-demo-en", "491ca939f9abb33ceeb26619b841fe91", 0) ||
        !check_original_legacy_arrows("FIRESTAFF_DM1_ATARI_ARROWS_ARCHIVE",
            "st12-en", "9ce2eaf7a9e78620e3f17594437caffa", 1)) return 1;
    if (!graphicsPath) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    if (!asset_file_matches_md5(graphicsPath, "fa6b1aa29e191418713bf2cda93d962e")) {
        fprintf(stderr, "original I34E graphics identity mismatch\n");
        return 1;
    }
    {
        M11_GameLaunchSpec spec={0};
        spec.title="Dungeon Master";spec.gameId=spec.sourceId="dm1";
        spec.dataDir=getenv("FIRESTAFF_DM1_DATA_DIR");
        spec.verifiedAssetPath=graphicsPath;
        spec.verifiedAssetMd5="fa6b1aa29e191418713bf2cda93d962e";
        if(!check_original_door_orientation(&spec,"PC34") ||
           !check_original_pc34_d0_walls(&spec))return 1;
        {
            char dungeonPath[2048];
            const int length = snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", spec.dataDir);
            if (length < 0 || (size_t)length >= sizeof(dungeonPath)) return 1;
            spec.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
            spec.dungeonPath = dungeonPath;
            if (!check_original_pc34_d0_walls(&spec)) return 1;
        }
    }
    M11_GameView_Init(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        M11_GameView_Shutdown(&state);
        fprintf(stderr, "configured PC34 GRAPHICS.DAT failed to load\n");
        return 1;
    }
    state.assetsAvailable = 1;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    strcpy(state.sourceId, "dm1");
    if (!check_original_d2_pixels(&state)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!M11_GameView_ProbeDrawDm1SideWallOrnamentHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 side-wall ornament material draw failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.globalOrnamentIndex != 1 ||
        receipt.viewWallIndex != 1 || receipt.graphicIndex != 261 ||
        receipt.transparentColor != 10 || !receipt.flipHorizontal ||
        !receipt.paletteMapValid || receipt.paletteMap[2] != 12 ||
        receipt.width <= 0 || receipt.height <= 0) {
        fprintf(stderr, "real PC34 side-wall ornament receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0107 side-wall ornament host material receipt");
    return 0;
}
