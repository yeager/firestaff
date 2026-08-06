
#include "nexus_v1_engine.h"
#include "nexus_v1_dgn_multi_level_capture_adjudicator.h"
#include "nexus_v1_saturn_save_capture.h"
#include "nexus_v1_prs3_dgn_placement_adapter.h"
#include "nexus_v1_structure1f_placement_binding.h"
#include "asset_find_by_hash.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_viewport.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_drops.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_font_s2d.h"
#include "nexus_v1_face_bin.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define strcasecmp _stricmp
#else
#include <dirent.h>
#endif

typedef struct {
    const char *name;
    const char *md5;
} Nexus_V1_KnownFileHash;

static uint64_t nexus_v1_owner_material_capture_target_fnv1a64(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target);
static int nexus_v1_slev_trace_value(const char *text, size_t size,
                                     const char *key, char *out_value,
                                     size_t out_value_size);
static int nexus_v1_slev_trace_hex_u32(const char *text, uint32_t *out_value);
static int nexus_v1_slev_trace_decimal_u32(const char *text,
                                           uint32_t *out_value);
static int nexus_v1_slev_trace_is_sha256(const char *text);
static int nexus_v1_slev_trace_hex_u64(const char *text,
                                       uint64_t *out_value);
static uint64_t nexus_v1_slev_trace_fnv1a64(const uint8_t *data,
                                             size_t size);
static void nexus_v1_load_shop_catalog(Nexus_V1_Engine *engine);

static const Nexus_V1_KnownFileHash g_nexus_known_boot_files[] = {
    {"DM.BIN", "e88d60859f65f08fa622e1992b02280f"},
    {"TITLE.CG", "80fa961fa95d7a0cb57e9a62f48786c8"},
    {"WARNING.BIN", "15c87a09af36e9579dfbd88a5af87477"},
    {"WARNING.BIN", "eb246b67f7758f23310221ac9b9efe2d"},
    {"WARNING.BIN", "9002866163ad733a75346547a4c7e0b5"},
    {"GAMEOVER.BIN", "0426cb045a495c151a138fd2c77370e2"},
    {"STABG.BIN", "e77d4dd48dd280ec299cfc8ee8851114"},
    {"SMAP00.BIN", "6e8e582d6ceb4b482dc994e7faa448d6"},
    {"SMAP01.BIN", "7d8379ea25ef2c4f4f591c5b0fc96a47"},
    {"SMAP02.BIN", "0accda60ebff223cf4c101694ff77b88"},
    {"SMAP03.BIN", "f2e2d64619a6299f83beed56a7ed4dc0"},
    {"SMAP04.BIN", "cb1f80fbca6249fd3163ec369518f04f"},
    {"SMAP05.BIN", "515afc77c702066a5d9c316a50ae4e16"},
    {"SMAP06.BIN", "74d2c16051225d194b614dada0d22f01"},
    {"SMAP07.BIN", "d04eaf63e72a66c630dac866e925470b"},
    {"SMAP08.BIN", "66e43e3ba88ebc4a6e820d879c305313"},
    {"SMAP09.BIN", "8f2ef4ea2e145062d72b5522cd77f7b7"},
    {"SMAP10.BIN", "5fdb45e5a727cf3d3fb018533c567803"},
    {"SMAP11.BIN", "1f072332769f0198e673d01d7df9c00e"},
    {"SMAP12.BIN", "bb81dac9241319617bc71b2e9b5b5735"},
    {"SMAP13.BIN", "c1a8e374ad3010f69f47b8e5d4a81875"},
    {"SMAP14.BIN", "78af8eecb251ecb9d2f752387bf1dda0"},
    {"SMAP15.BIN", "be63ff7a94e4e7e79ce0a354ef51054d"},
    {"FACE.BIN", "bd9ca16ea68043984e2804067b6cd66f"},
    {"FONT256.S2D", "427735a9997e692d85f2d81158dba423"},
    {"FONT256.S2D", "7bea3db1ccfe7cd8f32e364685cb0937"},
    {"FONT256.S2D", "fc1de196b359ae2f56a1c1fee94a6dbf"},
    {"MENU.BPK", "c2776768ff25287c79013a1452253ca0"},
    {"MENU.BPK", "a6f2272a4f6cb3c6b3b33012bc5b15ed"},
    {"MENU.BPK", "fcf8a00fbb92593ed9ae908f8e285cda"},
    {"ITEM.IBS", "309dc91bd14ded1223c72dd6c743f17c"},
    {"ITEM.IBS", "be3ea97919c7e802e5b151aad20fd6ec"},
    {"SN_FLOOR.MNS", "85c517e8e0bd84e00da58295dca5b409"},
    {"SN_WALL.MNS", "ae67ca9fa8d09481e1849a42aaaa2eb6"},
    {"ANTMAN.MNS", "d578212f99f9ad1a61ade2a06484e04a"},
    {"BIGWORM.MNS", "457fb32e4975b109f435f478bbf59899"},
    {"BORKETH.MNS", "ff6e7a0fcd50ba30cab0f93c48970c66"},
    {"CHAOS.MNS", "dc82e11302eb58cd8cf200e7268946d1"},
    {"DRA_ZOM.MNS", "c189e8f41a33a546302f631106f841ea"},
    {"D_GOLD.MNS", "0955e39f0807dea30acd5eff051fc56d"},
    {"D_RED.MNS", "c5dd72925db0df2bbfe9ddc05160d171"},
    {"D_SILVER.MNS", "a0929c9eed3bb7064086031d17e18b73"},
    {"GHOST.MNS", "201f3e0766821d28c6122a7cbd652447"},
    {"GIGGLER.MNS", "76311d88bda1889200b5442eb8acd5d4"},
    {"GOLEM.MNS", "9cd105a43119faf50537de026a9fd034"},
    {"GRN_DRA.MNS", "507271933bfe9a7b6ab8f7a01d9b1813"},
    {"H_HOUND.MNS", "f6d704310950624a67886be616735557"},
    {"LAS_MON.MNS", "590a45db6cb62c224c415ea1cd1c4b3b"},
    {"LORD_RIB.MNS", "aa76813ac43ea79a3df55dcec6cdc7f3"},
    {"MINI_DRA.MNS", "07c0affc959f52f00d9276104af727e8"},
    {"MUMMY.MNS", "6a8c849cbb87e218caa7dcd96c483311"},
    {"OBAKE.MNS", "d05a0ee97ade3c0db5492525c82381ec"},
    {"OITU.MNS", "b8aa4490b3695a72571126c213515f88"},
    {"RAT.MNS", "ef7b68f95978255f878c1fedcd6d547a"},
    {"RED_DRA.MNS", "163ab5fc2ea25165798dfbf8c4c3affb"},
    {"ROCKPILE.MNS", "478c8fccb2dcc3d82a442ae399e8e910"},
    {"SCORPION.MNS", "3655bfa98a005beabdcdea13058ab18f"},
    {"SCREAMER.MNS", "c3af0af2f0110b76e637622caeda3524"},
    {"S_SHIELD.MNS", "de4930cf4ec25c56a0f419ad66f65680"},
    {"S_SWORD.MNS", "2ecc9f49a07f55f0e6fc9414f0b6a30c"},
    {"VEXIRK.MNS", "44574143d331debd748f4ec6ba133269"},
    {"WORM.MNS", "25c34e051ea5cf4ab2ca37a60f89ef78"},
    {"LEV00.DGN", "603ec9c531a92539babdda84ab09e78e"},
    {"LEV01.DGN", "751e1442bf7dccbd41bf146b5be144ab"},
    {"LEV02.DGN", "e2cb85d9fedc27f894a84e0f465fcde1"},
    {"LEV03.DGN", "19637d6b59849565f64565aed786d7ea"},
    {"LEV04.DGN", "85abc1b822e5c66ec4e99f1f676c140e"},
    {"LEV05.DGN", "ed5d54ab0ac1c927c1346dd966c8a5cc"},
    {"LEV06.DGN", "58c336ff6146e7216f0081e726823ea1"},
    {"LEV07.DGN", "c19e6038a017a320515ecbb66f6da197"},
    {"LEV08.DGN", "9bfc31bea631345a3660c2645be0e95b"},
    {"LEV09.DGN", "32a6450f29eb7babd73fcbe7a0310f22"},
    {"LEV10.DGN", "2928440e9c21457929f1323a28a42f70"},
    {"LEV11.DGN", "d7be5cd0d6e5c10afe99ec9950614fad"},
    {"LEV12.DGN", "db1cf70d6730615f73f191fad5e11e32"},
    {"LEV13.DGN", "f8876d0181d79727013236a6b597b99b"},
    {"LEV14.DGN", "a634dd5e95567ecbbbc332350c8cf12b"},
    {"LEV15.DGN", "5e6e237074f1e6b0decc629868a51f3c"},
    {"SLEV00.BIN", "59c01cbdd224152a6176687cdebeea9e"},
    {"SLEV01.BIN", "b3b14a73db311b7cf1bf417e858e5350"},
    {"SLEV02.BIN", "a77b9cae611a01fbc6a68958f9252b48"},
    {"SLEV03.BIN", "6600745773cd7058d3125747ead5b612"},
    {"SLEV04.BIN", "1d678eaff22d0827899a8ffa7377f06b"},
    {"SLEV05.BIN", "9166ae024df53462b7b47ca81db56fc4"},
    {"SLEV06.BIN", "7c737f7532677babfc28848198a8288d"},
    {"SLEV07.BIN", "d607ef9b6ee52c0730b92ed22843c2da"},
    {"SLEV08.BIN", "5f1bfbd324648ac6872c73d3d282cd6d"},
    {"SLEV09.BIN", "f588811de05a2099633f7dba5a0ca956"},
    {"SLEV10.BIN", "e346ee3c7db858cbd872ca947eb79309"},
    {"SLEV11.BIN", "a7deb8241dfa633804793cf07e841a84"},
    {"SLEV12.BIN", "fc4028c61279d7d10fc771b040486c85"},
    {"SLEV13.BIN", "2a988cf44049e59e647976d93126dedb"},
    {"SLEV14.BIN", "8116d450f4f60af67e8a86b559beb5ab"},
    {"SLEV15.BIN", "5b71918f112e10b3d8c40092565cce53"},
    {"SNDLEV00.SAL", "ea8493341fd8ad4f20335629e6dbdbbc"},
    {"SNDLEV01.SAL", "ea8493341fd8ad4f20335629e6dbdbbc"},
    {"SNDLEV02.SAL", "729a66977e1661808d104059ff21e95e"},
    {"SNDLEV03.SAL", "5c357157d68b2878881e1e0a293d3058"},
    {"SNDLEV04.SAL", "9d8d8b793801234b8f4b0e64e1135afc"},
    {"SNDLEV05.SAL", "db21b7945b65ccfbb7a4246b1f5dca7b"},
    {"SNDLEV06.SAL", "2d7698144c64996536e8240ee7bfea08"},
    {"SNDLEV07.SAL", "9b31400c2b3c7468b8f88c1fd09c8bca"},
    {"SNDLEV08.SAL", "0e5caba79b2e31963739784f6941f3c5"},
    {"SNDLEV09.SAL", "f311e79dd6e4be376c0466ea34a27b10"},
    {"SNDLEV10.SAL", "4b655b6cf8c6caebe99dd0b3b55d39c0"},
    {"SNDLEV11.SAL", "7a9509b7d777f1468ecf987107f1aed0"},
    {"SNDLEV12.SAL", "59e70afc5cf607c6d268811cbed961cd"},
    {"SNDLEV13.SAL", "14a1f88abc0363d7a96b2a267d89e7a4"},
    {"SNDLEV14.SAL", "1c12a4f3d3dfc9892cdf54955abbca62"},
    {"SNDLEV15.SAL", "d8cfb5da08d5fc8d86834d81d8997eac"},
    {"SNDLEV00.MAP", "232afa942754027ecf49702703c72e83"},
    {"SNDLEV01.MAP", "232afa942754027ecf49702703c72e83"},
    {"SNDLEV02.MAP", "e724a7b953a6ee9d4bb7d5c2114d5310"},
    {"SNDLEV03.MAP", "91be9e82471be25036889b6801e7fcd3"},
    {"SNDLEV04.MAP", "64f95657b2745acdbed9d938ba5dfd9e"},
    {"SNDLEV05.MAP", "95be564a755500e2605b6c83f742f37f"},
    {"SNDLEV06.MAP", "8d0a168e11ebeea2c424a81a474c9d17"},
    {"SNDLEV07.MAP", "2c8def9015004a9955706c7f41d319be"},
    {"SNDLEV08.MAP", "4e5bee7797d2b3b06a54bb55e6809e90"},
    {"SNDLEV09.MAP", "47af8003ec0900979fa939288cc1b549"},
    {"SNDLEV10.MAP", "89f984a9eb3be797c37515766e658c12"},
    {"SNDLEV11.MAP", "130bf4977263076710aaf722c3078f0c"},
    {"SNDLEV12.MAP", "5cdd004b21437268ef51bdc6be33988d"},
    {"SNDLEV13.MAP", "1f3a1f6ddae837f8140063a637d5fbbc"},
    {"SNDLEV14.MAP", "fd3b5d9894265d0753aee0e0ddb02500"},
    {"SNDLEV15.MAP", "9757c71fe8afad9ad3be58543640270d"},
    {"SDDRVS.TSK", "9a2bfe6df8b4a69077054ca2dbf78cb4"},
    {NULL, NULL}
};

/* The catalog identifies canonical LEV entries by MD5.  Hash the bytes that
 * nexus_v1_read_file() just materialized, rather than re-opening a path after
 * the DGN parser has consumed it: a path-level scan alone leaves a TOCTOU
 * gap between source discovery and Structure3 binding. */
typedef struct {
    uint32_t state[4];
} Nexus_V1_Md5;

#define NEXUS_V1_MD5_F(x,y,z) (((x) & (y)) | (~(x) & (z)))
#define NEXUS_V1_MD5_G(x,y,z) (((x) & (z)) | ((y) & ~(z)))
#define NEXUS_V1_MD5_H(x,y,z) ((x) ^ (y) ^ (z))
#define NEXUS_V1_MD5_I(x,y,z) ((y) ^ ((x) | ~(z)))
#define NEXUS_V1_MD5_ROTATE(x,n) (((x) << (n)) | ((x) >> (32U - (n))))
#define NEXUS_V1_MD5_STEP(f,a,b,c,d,x,s,k) do { \
    (a) += f((b), (c), (d)) + (x) + (uint32_t)(k); \
    (a) = NEXUS_V1_MD5_ROTATE((a), (s)); \
    (a) += (b); \
} while (0)

static void nexus_v1_md5_block(Nexus_V1_Md5 *md5, const uint8_t block[64])
{
    uint32_t x[16];
    uint32_t a = md5->state[0], b = md5->state[1];
    uint32_t c = md5->state[2], d = md5->state[3];
    int i;
    for (i = 0; i < 16; ++i) {
        x[i] = (uint32_t)block[i * 4] |
            ((uint32_t)block[i * 4 + 1] << 8) |
            ((uint32_t)block[i * 4 + 2] << 16) |
            ((uint32_t)block[i * 4 + 3] << 24);
    }
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,a,b,c,d,x[0],7,0xd76aa478); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,d,a,b,c,x[1],12,0xe8c7b756); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,c,d,a,b,x[2],17,0x242070db); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,b,c,d,a,x[3],22,0xc1bdceee);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,a,b,c,d,x[4],7,0xf57c0faf); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,d,a,b,c,x[5],12,0x4787c62a); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,c,d,a,b,x[6],17,0xa8304613); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,b,c,d,a,x[7],22,0xfd469501);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,a,b,c,d,x[8],7,0x698098d8); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,d,a,b,c,x[9],12,0x8b44f7af); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,c,d,a,b,x[10],17,0xffff5bb1); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,b,c,d,a,x[11],22,0x895cd7be);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,a,b,c,d,x[12],7,0x6b901122); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,d,a,b,c,x[13],12,0xfd987193); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,c,d,a,b,x[14],17,0xa679438e); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_F,b,c,d,a,x[15],22,0x49b40821);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,a,b,c,d,x[1],5,0xf61e2562); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,d,a,b,c,x[6],9,0xc040b340); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,c,d,a,b,x[11],14,0x265e5a51); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,b,c,d,a,x[0],20,0xe9b6c7aa);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,a,b,c,d,x[5],5,0xd62f105d); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,d,a,b,c,x[10],9,0x02441453); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,c,d,a,b,x[15],14,0xd8a1e681); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,b,c,d,a,x[4],20,0xe7d3fbc8);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,a,b,c,d,x[9],5,0x21e1cde6); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,d,a,b,c,x[14],9,0xc33707d6); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,c,d,a,b,x[3],14,0xf4d50d87); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,b,c,d,a,x[8],20,0x455a14ed);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,a,b,c,d,x[13],5,0xa9e3e905); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,d,a,b,c,x[2],9,0xfcefa3f8); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,c,d,a,b,x[7],14,0x676f02d9); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_G,b,c,d,a,x[12],20,0x8d2a4c8a);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,a,b,c,d,x[5],4,0xfffa3942); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,d,a,b,c,x[8],11,0x8771f681); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,c,d,a,b,x[11],16,0x6d9d6122); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,b,c,d,a,x[14],23,0xfde5380c);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,a,b,c,d,x[1],4,0xa4beea44); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,d,a,b,c,x[4],11,0x4bdecfa9); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,c,d,a,b,x[7],16,0xf6bb4b60); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,b,c,d,a,x[10],23,0xbebfbc70);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,a,b,c,d,x[13],4,0x289b7ec6); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,d,a,b,c,x[0],11,0xeaa127fa); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,c,d,a,b,x[3],16,0xd4ef3085); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,b,c,d,a,x[6],23,0x04881d05);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,a,b,c,d,x[9],4,0xd9d4d039); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,d,a,b,c,x[12],11,0xe6db99e5); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,c,d,a,b,x[15],16,0x1fa27cf8); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_H,b,c,d,a,x[2],23,0xc4ac5665);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,a,b,c,d,x[0],6,0xf4292244); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,d,a,b,c,x[7],10,0x432aff97); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,c,d,a,b,x[14],15,0xab9423a7); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,b,c,d,a,x[5],21,0xfc93a039);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,a,b,c,d,x[12],6,0x655b59c3); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,d,a,b,c,x[3],10,0x8f0ccc92); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,c,d,a,b,x[10],15,0xffeff47d); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,b,c,d,a,x[1],21,0x85845dd1);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,a,b,c,d,x[8],6,0x6fa87e4f); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,d,a,b,c,x[15],10,0xfe2ce6e0); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,c,d,a,b,x[6],15,0xa3014314); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,b,c,d,a,x[13],21,0x4e0811a1);
    NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,a,b,c,d,x[4],6,0xf7537e82); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,d,a,b,c,x[11],10,0xbd3af235); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,c,d,a,b,x[2],15,0x2ad7d2bb); NEXUS_V1_MD5_STEP(NEXUS_V1_MD5_I,b,c,d,a,x[9],21,0xeb86d391);
    md5->state[0] += a; md5->state[1] += b;
    md5->state[2] += c; md5->state[3] += d;
}

int nexus_v1_dgn_bytes_match_canonical_md5(
    const uint8_t *data, int size, const char *expected)
{
    Nexus_V1_Md5 md5;
    uint8_t tail[128];
    uint64_t bits;
    size_t offset = 0U;
    size_t tail_size;
    char actual[33];
    static const char hex[] = "0123456789abcdef";
    int i;

    if (!data || size < 0 || !expected || strlen(expected) != 32U) return 0;
    md5.state[0] = 0x67452301U; md5.state[1] = 0xefcdab89U;
    md5.state[2] = 0x98badcfeU; md5.state[3] = 0x10325476U;
    bits = (uint64_t)(unsigned int)size * 8U;
    while (offset + 64U <= (size_t)size) {
        nexus_v1_md5_block(&md5, data + offset);
        offset += 64U;
    }
    tail_size = (size_t)size - offset;
    memset(tail, 0, sizeof(tail));
    if (tail_size) memcpy(tail, data + offset, tail_size);
    tail[tail_size++] = 0x80U;
    if (tail_size > 56U) {
        nexus_v1_md5_block(&md5, tail);
        memset(tail, 0, 64U);
    }
    for (i = 0; i < 8; ++i) tail[56 + i] = (uint8_t)(bits >> (i * 8));
    nexus_v1_md5_block(&md5, tail);
    for (i = 0; i < 16; ++i) {
        uint8_t byte = (uint8_t)(md5.state[i / 4] >> ((i % 4) * 8));
        actual[i * 2] = hex[byte >> 4];
        actual[i * 2 + 1] = hex[byte & 15U];
    }
    actual[32] = '\0';
    return strcasecmp(actual, expected) == 0;
}

/* Verify a canonical MD5 directly against the bytes of an already-open
 * Track 1 ISO entry.  The directory-wide asset_find_by_md5 scan used here
 * before could only publish a match when the discovery resolved to this
 * very "<iso>::<name>" entry, so hashing the entry in place proves the
 * same fact in O(entry size).  The full-tree scan re-read GB-scale asset
 * directories once per boot file and pushed cold-cache Nexus launches
 * past every runtime-gate timeout. */
static int nexus_v1_iso_entry_matches_canonical_md5(
    Nexus_V1_Engine *engine, const Nexus_ISOFile *file, const char *md5) {
    uint8_t *entry_bytes;
    int entry_size;
    int verified;
    if (!engine || !file || !md5 || file->size == 0U ||
        file->size > 64U * 1024U * 1024U) {
        return 0;
    }
    entry_bytes = (uint8_t *)malloc(file->size);
    if (!entry_bytes) {
        return 0;
    }
    entry_size = nexus_iso_read_file(&engine->iso, file, entry_bytes,
                                     (int)file->size);
    verified = entry_size == (int)file->size &&
        nexus_v1_dgn_bytes_match_canonical_md5(entry_bytes, entry_size, md5);
    free(entry_bytes);
    return verified;
}

void nexus_v1_dgn_renderer_handoff_require_canonical_source(
    Nexus_V1_DgnRendererHandoffReceipt *receipt,
    int canonical_source_verified)
{
    if (!receipt) return;
    receipt->canonical_source_verified = canonical_source_verified ? 1 : 0;
    if (receipt->canonical_source_verified) return;
    receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_CANONICAL_SOURCE;
    receipt->can_render_dgn_mesh = 0;
    receipt->blocks_real_dgn_mesh_render = 1;
    receipt->fallback_visuals_permitted = 0;
}

#undef NEXUS_V1_MD5_STEP
#undef NEXUS_V1_MD5_ROTATE
#undef NEXUS_V1_MD5_I
#undef NEXUS_V1_MD5_H
#undef NEXUS_V1_MD5_G
#undef NEXUS_V1_MD5_F

static void nexus_v1_free_structure2_surfaces(Nexus_V1_Engine *engine) {
    int i;
    if (!engine) return;
    for (i = 0; i < NEXUS_DGN_MAX_STRUCTURE2_TEXTURES; ++i) {
        free(engine->structure2_surfaces[i].pixels);
        engine->structure2_surfaces[i].pixels = NULL;
    }
    engine->structure2_surface_count = 0;
    memset(&engine->structure2_decode_receipt, 0,
           sizeof(engine->structure2_decode_receipt));
}

static int nexus_v1_decode_structure2_animation_materials(
    Nexus_V1_Engine *engine, const uint8_t *data, int size) {
    (void)data;
    (void)size;
    if (!engine) return 0;
    nexus_v1_dmdf_free_material_bank(&engine->animated_floor_materials);
    engine->animated_floor_material_route_valid = 0;
    nexus_v1_free_structure2_surfaces(engine);

    /* DMWeb identifies the Structure2 descriptor envelope and bounded
     * payload candidates, but that is not a Saturn pixel/CLUT/VDP1 capture.
     * Do not promote the documented 08h/28h hypotheses into host surfaces
     * while the retail source lane lacks an authenticated original capture.
     * The non-DMWeb lane remains available for isolated compatibility
     * fixtures. */
    if (engine->current_level.geometry_info.dmweb_container)
        return 0;
    if (nexus_v1_current_level_decode_structure2_textures(
            engine, engine->structure2_surfaces,
            NEXUS_DGN_MAX_STRUCTURE2_TEXTURES,
            &engine->structure2_decode_receipt) == 1) {
        engine->structure2_surface_count =
            engine->structure2_decode_receipt.decoded_count;
    }
    return engine->structure2_surface_count > 0 ? 1 : 0;
}

static const char *nexus_known_boot_file_md5(const char *name) {
    int i;
    if (!name) return NULL;
    for (i = 0; g_nexus_known_boot_files[i].name; ++i) {
        if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0) {
            return g_nexus_known_boot_files[i].md5;
        }
    }
    return NULL;
}

const char *nexus_v1_known_file_md5(const char *name)
{
    return nexus_known_boot_file_md5(name);
}

static int nexus_path_is_file(const char *path);

static uint64_t nexus_v1_dgn_bytes_fnv1a64(const uint8_t *data, int size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    int index;

    if (!data || size <= 0) return 0U;
    for (index = 0; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint32_t nexus_v1_dgn_read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
        ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

static uint16_t nexus_v1_dgn_read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
}

static int nexus_v1_dgn_source_bytes_match(
    const Nexus_V1_DgnStructure2SourceReceipt *source,
    const uint8_t *data, int size)
{
    if (source && source->loaded_dgn_identity_bytes && data && size > 0 &&
        source->loaded_bytes_bound && source->loaded_dgn_size == size) {
        return memcmp(source->loaded_dgn_identity_bytes, data,
                      (size_t)size) == 0;
    }
    return source && source->loaded_bytes_bound &&
        source->loaded_dgn_size == size &&
        source->loaded_dgn_fnv1a64 != 0U &&
        source->loaded_dgn_fnv1a64 == nexus_v1_dgn_bytes_fnv1a64(data, size);
}

static int nexus_v1_level_aux_source_receipt(
    Nexus_V1_Engine *engine, const char *name,
    Nexus_V1_LevelAuxSourceReceipt *out_receipt) {
    char path[512];
    char found_path[ASSET_PATH_MAX];
    const char *md5;
    const Nexus_ISOFile *file;
    int i;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !name) return 0;
    strncpy(out_receipt->canonical_name, name,
            sizeof(out_receipt->canonical_name) - 1U);
    md5 = nexus_known_boot_file_md5(name);
    if (!md5) return 0;
    strncpy(out_receipt->canonical_md5, md5,
            sizeof(out_receipt->canonical_md5) - 1U);
    if (engine->source == NEXUS_SRC_EXTRACTED) {
        snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
        out_receipt->exact_source_entry_observed = nexus_path_is_file(path);
        out_receipt->hash_discovery_attempted = 1;
        if (out_receipt->exact_source_entry_observed) {
            for (i = 0; g_nexus_known_boot_files[i].name && !out_receipt->canonical_hash_verified; ++i) {
                if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0 &&
                    asset_file_matches_md5(path, g_nexus_known_boot_files[i].md5)) {
                    out_receipt->canonical_hash_verified = 1;
                    strncpy(out_receipt->canonical_md5, g_nexus_known_boot_files[i].md5,
                            sizeof(out_receipt->canonical_md5) - 1U);
                }
            }
        }
        /* A wrong canonical filename must not hide a hash-verified renamed
         * retail member elsewhere in the configured data root. */
        if (!out_receipt->canonical_hash_verified) {
            for (i = 0; g_nexus_known_boot_files[i].name && !out_receipt->canonical_hash_verified; ++i) {
                if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0 &&
                    asset_find_by_md5(engine->data_dir, g_nexus_known_boot_files[i].md5,
                                      found_path, (int)sizeof(found_path), 8)) {
                    out_receipt->canonical_hash_verified = 1;
                    strncpy(out_receipt->canonical_md5, g_nexus_known_boot_files[i].md5,
                            sizeof(out_receipt->canonical_md5) - 1U);
                }
            }
        }
        /* The extracted European retail layout keeps some auxiliary DMDF
         * resources only in the co-located Track 1 ISO.  Mirror
         * nexus_v1_read_file() here so the provenance receipt authenticates
         * the bytes that the decoder actually receives; absence of a loose
         * file is not evidence that the canonical source is missing. */
        if (!out_receipt->canonical_hash_verified &&
            engine->supplemental_iso_valid) {
            file = nexus_iso_find(&engine->supplemental_iso, name);
            out_receipt->exact_source_entry_observed =
                out_receipt->exact_source_entry_observed || file != NULL;
            for (i = 0; g_nexus_known_boot_files[i].name &&
                        !out_receipt->canonical_hash_verified; ++i) {
                if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0 &&
                    nexus_v1_iso_entry_matches_canonical_md5(
                        engine, file, g_nexus_known_boot_files[i].md5)) {
                    out_receipt->canonical_hash_verified = 1;
                    strncpy(out_receipt->canonical_md5,
                            g_nexus_known_boot_files[i].md5,
                            sizeof(out_receipt->canonical_md5) - 1U);
                }
            }
        }
    } else if (engine->source == NEXUS_SRC_ISO) {
        file = nexus_iso_find(&engine->iso, name);
        out_receipt->exact_source_entry_observed = file != NULL;
        out_receipt->hash_discovery_attempted = 1;
        for (i = 0; g_nexus_known_boot_files[i].name && !out_receipt->canonical_hash_verified; ++i) {
            if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0 &&
                nexus_v1_iso_entry_matches_canonical_md5(engine, file, g_nexus_known_boot_files[i].md5)) {
                out_receipt->canonical_hash_verified = 1;
                strncpy(out_receipt->canonical_md5, g_nexus_known_boot_files[i].md5,
                        sizeof(out_receipt->canonical_md5) - 1U);
            }
        }
    }
    return 0;
}

int nexus_v1_warning_bin_source_receipt(
    Nexus_V1_Engine *engine, Nexus_V1_LevelAuxSourceReceipt *out_receipt)
{
    return nexus_v1_level_aux_source_receipt(engine, "WARNING.BIN", out_receipt);
}

static int nexus_v1_structure2_source_receipt(
    Nexus_V1_Engine *engine, int level_index, const Nexus_V1_Level *level,
    const uint8_t *loaded_dgn_data, int loaded_dgn_size,
    Nexus_V1_DgnStructure2SourceReceipt *out_receipt) {
    char name[16];
    char path[512];
    char found_path[ASSET_PATH_MAX];
    const char *md5;
    const Nexus_ISOFile *file;
    int loaded_bytes_canonical;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = level_index;
    out_receipt->payload_decoder_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !level || level_index < 0 || level_index > 15) return 0;

    snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
    strncpy(out_receipt->canonical_name, name,
            sizeof(out_receipt->canonical_name) - 1U);
    md5 = nexus_known_boot_file_md5(name);
    if (!md5) return 0;
    strncpy(out_receipt->canonical_md5, md5,
            sizeof(out_receipt->canonical_md5) - 1U);
    loaded_bytes_canonical = nexus_v1_dgn_bytes_match_canonical_md5(
        loaded_dgn_data, loaded_dgn_size, md5);
    /* A canonical hash alone cannot admit a malformed local descriptor
     * layout. Retain the existing no-decoder policy, but require the parsed
     * descriptor targets to remain within the one proven Structure2 envelope
     * before publishing this source to a host route. */
    out_receipt->structure2_payload_envelope_valid =
        nexus_v1_level_structure2_source_envelope_valid(level) ? 1 : 0;

    if (engine->source == NEXUS_SRC_EXTRACTED) {
        snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
        out_receipt->exact_source_entry_observed = nexus_path_is_file(path);
        out_receipt->hash_discovery_attempted = 1;
        if (out_receipt->exact_source_entry_observed) {
            out_receipt->canonical_hash_verified =
                asset_file_matches_md5(path, md5) ? 1 : 0;
        }
        if (!out_receipt->canonical_hash_verified) {
            /* Match nexus_v1_read_extracted_file(): a wrong canonical name
             * must not hide a hash-verified renamed retail member. */
            out_receipt->canonical_hash_verified = asset_find_by_md5(
                engine->data_dir, md5, found_path, (int)sizeof(found_path), 8);
        }
    } else if (engine->source == NEXUS_SRC_ISO) {
        file = nexus_iso_find(&engine->iso, name);
        out_receipt->exact_source_entry_observed = file != NULL;
        /* The generic scanner hashed ISO entries only to bind its match
         * to this already-opened Track 1 entry; hash that entry in place
         * instead of re-scanning the whole asset tree per level. */
        out_receipt->hash_discovery_attempted = 1;
        out_receipt->canonical_hash_verified =
            nexus_v1_iso_entry_matches_canonical_md5(engine, file, md5);
    }
    /* The source scan establishes media location, while the caller's MD5
     * establishes identity for the exact buffer consumed by Structure3.
     * Both are required before any existing source binder may use it. */
    out_receipt->canonical_hash_verified =
        out_receipt->canonical_hash_verified && loaded_bytes_canonical;
    out_receipt->materialization_bound =
        out_receipt->canonical_hash_verified &&
        out_receipt->structure2_payload_envelope_valid;
    if (out_receipt->materialization_bound && loaded_dgn_data &&
        loaded_dgn_size > 0) {
        out_receipt->loaded_dgn_size = loaded_dgn_size;
        out_receipt->loaded_dgn_fnv1a64 =
            nexus_v1_dgn_bytes_fnv1a64(loaded_dgn_data, loaded_dgn_size);
        out_receipt->loaded_bytes_bound =
            out_receipt->loaded_dgn_fnv1a64 != 0U;
    }
    return 0;
}

static int nexus_path_has_ext(const char *path, const char *ext) {
    size_t path_len;
    size_t ext_len;
    if (!path || !ext) return 0;
    path_len = strlen(path);
    ext_len = strlen(ext);
    if (path_len < ext_len) return 0;
    return strcasecmp(path + path_len - ext_len, ext) == 0;
}

static int nexus_path_is_file(const char *path) {
    struct stat st;
    return path && stat(path, &st) == 0 && (st.st_mode & S_IFMT) != S_IFDIR;
}

static int nexus_v1_file_has_dmdf_magic(const char *path) {
    unsigned char magic[4];
    FILE *fp;
    if (!path || !path[0]) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return magic[0] == 'D' && magic[1] == 'M' &&
           magic[2] == 'D' && magic[3] == 'F';
}

#ifdef _WIN32
static int nexus_v1_find_dmdf_family_file_recursive(const char *dir,
                                                    const char *ext,
                                                    char *out_path,
                                                    int out_size,
                                                    int depth) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[512];
    if (!dir || !ext || !out_path || out_size <= 0 || depth < 0) return 0;
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        char path[512];
        if (strcmp(fd.cFileName, ".") == 0 ||
            strcmp(fd.cFileName, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (nexus_v1_find_dmdf_family_file_recursive(path, ext, out_path,
                                                         out_size, depth - 1)) {
                FindClose(h);
                return 1;
            }
        } else if (nexus_path_has_ext(path, ext) &&
                   nexus_v1_file_has_dmdf_magic(path)) {
            snprintf(out_path, (size_t)out_size, "%s", path);
            FindClose(h);
            return 1;
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return 0;
}
#else
static int nexus_v1_find_dmdf_family_file_recursive(const char *dir,
                                                    const char *ext,
                                                    char *out_path,
                                                    int out_size,
                                                    int depth) {
    DIR *d;
    struct dirent *ent;
    if (!dir || !ext || !out_path || out_size <= 0 || depth < 0) return 0;
    d = opendir(dir);
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL) {
        char path[512];
        struct stat st;
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0) {
            continue;
        }
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            if (nexus_v1_find_dmdf_family_file_recursive(path, ext, out_path,
                                                         out_size, depth - 1)) {
                closedir(d);
                return 1;
            }
        } else if (nexus_path_has_ext(path, ext) &&
                   nexus_v1_file_has_dmdf_magic(path)) {
            snprintf(out_path, (size_t)out_size, "%s", path);
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}
#endif

static int nexus_v1_find_dmdf_family_file(const char *dir,
                                          const char *name,
                                          char *out_path,
                                          int out_size) {
    if (!name) return 0;
    if (nexus_path_has_ext(name, ".MNS")) {
        return nexus_v1_find_dmdf_family_file_recursive(dir, ".MNS",
                                                        out_path, out_size, 8);
    }
    if (nexus_path_has_ext(name, ".DMDF")) {
        return nexus_v1_find_dmdf_family_file_recursive(dir, ".DMDF",
                                                        out_path, out_size, 8);
    }
    return 0;
}

static const Nexus_DMDFTextureSurface *nexus_v1_plan_surface(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_DgnRenderCommand *command,
    int use_static_mns_material_route,
    int use_bpk_material_route)
{
    const Nexus_DMDFMaterialBank *bank;
    const Nexus_DMDFTextureSurface *surface;
    if (!engine || !command) return NULL;
    /* A Structure2 image descriptor is source provenance only until its
     * original payload is decoded. It cannot smuggle an animated surface
     * through either static material route. */
    if (command->animated_texture_declared &&
        command->animated_texture_structure2_image_valid &&
        engine->animated_floor_material_route_valid &&
        command->animated_texture_structure2_image_id <
            NEXUS_DMDF_MATERIAL_COUNT &&
        command->animated_texture_structure2_image_id <
            (uint16_t)engine->animated_floor_materials.surface_count) {
        surface = &engine->animated_floor_materials.surfaces[
            command->animated_texture_structure2_image_id];
        return surface->valid &&
            ((use_static_mns_material_route && !surface->from_bpk) ||
             (use_bpk_material_route && surface->from_bpk)) ? surface : NULL;
    }
    bank = (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
            command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
        ? &engine->floor_materials : &engine->wall_materials;
    if (!bank->valid) {
        return NULL;
    }
    /* Structure1B selectors are opaque until the Saturn selector transform
     * is captured. Do not let a future, incorrectly admitted route index a
     * decoded MNS bank past its bounded descriptor-backed surface count. */
    if (command->material_id >= (uint16_t)bank->surface_count) {
        return NULL;
    }
    surface = &bank->surfaces[command->material_id];
    if (!surface->valid) return NULL;
    /* Never let a decoded MNS bank make an unproved Structure1B transform
     * look renderable, and never cross-source a BPK command with MNS pixels. */
    if (use_static_mns_material_route && !surface->from_bpk) return surface;
    if (use_bpk_material_route && surface->from_bpk) return surface;
    return NULL;
}

/* Retain the complete byte-proved owner chain for corpus inspection without
 * claiming the final model-index-to-mesh-entry relation. The latter remains
 * explicitly false until original Saturn evidence establishes it. */
static void nexus_v1_dgn_structure1f_owner_model_selector_corpus_receipt(
    int level_index, const Nexus_V1_DgnStructure2SourceReceipt *source,
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FOwnerModelSelectorCorpusReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    Nexus_V1_DgnStructure3ModelReferenceReceipt model_references;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt face_selectors;
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt model_face_selectors;

    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = level_index;
    out_receipt->no_draw_only = 1;
    if (!source || !level || level_index < 0 || level_index >= 16 ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound) {
        return;
    }
    out_receipt->canonical_lev_source_bound = 1;
    memset(&spatial, 0, sizeof(spatial));
    memset(&relation, 0, sizeof(relation));
    memset(&model_references, 0, sizeof(model_references));
    memset(&face_selectors, 0, sizeof(face_selectors));
    memset(&model_face_selectors, 0, sizeof(model_face_selectors));
    if (nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0 ||
        !spatial.valid ||
        nexus_v1_level_structure1a_relation_receipt(level, &relation) != 0 ||
        !relation.complete ||
        nexus_v1_level_structure3_model_reference_receipt(
            level, &model_references) != 0 || !model_references.complete ||
        nexus_v1_level_structure1f_face_selector_receipt(
            level, &face_selectors) != 0 || !face_selectors.complete ||
        face_selectors.face_semantics_proven ||
        nexus_v1_level_structure3_model_face_selector_receipt(
            level, &model_face_selectors) != 0 ||
        !model_face_selectors.complete ||
        model_face_selectors.attachment_semantics_proven) {
        return;
    }
    out_receipt->structure1f_owner_row_count =
        spatial.structure1a_bound_entry_count;
    out_receipt->structure1a_resolved_row_count =
        relation.resolved_entry_count;
    out_receipt->structure3_model_reference_count =
        model_references.resolved_model_reference_count;
    out_receipt->structure1f_face_selector_count =
        face_selectors.resolved_face_selector_count;
    out_receipt->structure3_model_face_selector_pair_count =
        model_face_selectors.resolved_pair_count;
    out_receipt->owner_model_selector_binding_complete =
        out_receipt->structure1f_owner_row_count ==
            out_receipt->structure1a_resolved_row_count &&
        out_receipt->structure1a_resolved_row_count ==
            out_receipt->structure3_model_reference_count &&
        out_receipt->structure3_model_reference_count ==
            out_receipt->structure1f_face_selector_count &&
        out_receipt->structure1f_face_selector_count ==
            out_receipt->structure3_model_face_selector_pair_count;
    out_receipt->valid = out_receipt->owner_model_selector_binding_complete;
}

int nexus_v1_inspect_dgn_material_corpus(
    Nexus_V1_Engine *engine,
    Nexus_V1_DgnMaterialCorpusReceipt *out_receipt)
{
    uint8_t *floor_refs = NULL;
    uint8_t *ceiling_refs = NULL;
    uint8_t *wall_refs = NULL;
    size_t floor_count = 0U;
    size_t ceiling_count = 0U;
    size_t wall_count = 0U;
    int level_index;
    Nexus_V1_DgnMaterialCorpusReceipt receipt;

    if (!engine || !out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.attempted = 1;
    receipt.expected_level_count = 16;
    receipt.fallback_visuals_permitted = 0;
    receipt.floor_coverage.category = NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR;
    receipt.ceiling_coverage.category = NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING;
    receipt.wall_coverage.category = NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL;

    floor_refs = (uint8_t *)malloc((size_t)receipt.expected_level_count *
                                   NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE);
    ceiling_refs = (uint8_t *)malloc((size_t)receipt.expected_level_count *
                                     NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE);
    wall_refs = (uint8_t *)malloc((size_t)receipt.expected_level_count *
                                  NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE * 4U);
    if (!floor_refs || !ceiling_refs || !wall_refs) goto done;

    for (level_index = 0; level_index < receipt.expected_level_count;
         ++level_index) {
        char name[16];
        uint8_t *data;
        int size = 0;
        int loaded_bytes_canonical;
        int x;
        int y;
        int structure3_face_total = 0;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure2SourceReceipt level_source;
        snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
        data = nexus_v1_read_file(engine, name, &size);
        if (!data) continue;
        loaded_bytes_canonical = nexus_v1_dgn_bytes_match_canonical_md5(
            data, size, nexus_known_boot_file_md5(name));
        ++receipt.readable_level_count;
        if (!loaded_bytes_canonical) {
            free(data);
            continue;
        }
        memset(&level, 0, sizeof(level));
        if (nexus_v1_level_load(&level, data, size, level_index) != 0) {
            free(data);
            continue;
        }
        memset(&level_source, 0, sizeof(level_source));
        (void)nexus_v1_structure2_source_receipt(
            engine, level_index, &level, data, size, &level_source);
        receipt.structure2_sources[level_index] = level_source;
        if (!level_source.canonical_hash_verified ||
            !level_source.materialization_bound ||
            !level_source.loaded_bytes_bound) {
            free(data);
            continue;
        }
        /* The generic geometry_info.mesh_ready bit describes the bounded
         * post-grid/collision envelope. The viewport's real Structure3
         * material path uses this extractor instead; use the same receipt for
         * the corpus census so parsed retail mesh geometry is not reported as
         * absent merely because Saturn transform/material semantics remain
         * capture-gated. */
        if (nexus_v1_level_structure3_mesh_geometry_ready(
                &level, data, size, &structure3_face_total)) {
            ++receipt.geometry_ready_level_count;
        }
        free(data);
        ++receipt.parsed_level_count;
        receipt.structure3_payloads[level_index] = level.structure3_payload;
        receipt.structure3_directories[level_index] = level.structure3_directory;
        receipt.structure3_entry_headers[level_index] =
            level.structure3_entry_headers;
        (void)nexus_v1_level_structure3_model_reference_receipt(
            &level, &receipt.structure3_model_references[level_index]);
        (void)nexus_v1_level_structure1a_transform_selector_receipt(
            &level, &receipt.structure1a_transform_selectors[level_index]);
        (void)nexus_v1_level_structure1f_face_selector_receipt(
            &level, &receipt.structure1f_face_selectors[level_index]);
        (void)nexus_v1_level_structure1f_rotation_selector_receipt(
            &level, &receipt.structure1f_rotation_selectors[level_index]);
        (void)nexus_v1_level_structure1f_face_rotation_pair_receipt(
            &level, &receipt.structure1f_face_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_offset_pair_receipt(
            &level, &receipt.structure1f_offset_pairs[level_index]);
        (void)nexus_v1_level_structure1f_wall_payload_selector_receipt(
            &level, &receipt.structure1f_wall_payload_selectors[level_index]);
        (void)nexus_v1_level_structure1f_wall_sensor_destination_receipt(
            &level, &receipt.structure1f_wall_sensor_destinations[level_index]);
        (void)nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
            &level, &receipt.structure1f_wall_sensor_control_selectors[level_index]);
        (void)nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
            &level, &receipt.structure1f_wall_sensor_control_destination_tuples[level_index]);
        (void)nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
            &level, &receipt.structure1f_wall_sensor_model_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
            &level, &receipt.structure1f_wall_decoration_model_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_alcove_payload_selector_receipt(
            &level, &receipt.structure1f_alcove_payload_selectors[level_index]);
        (void)nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
            &level, &receipt.structure1f_alcove_payload_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
            &level, &receipt.structure1f_floor_sensor_control_selectors[level_index]);
        (void)nexus_v1_level_structure1f_floor_sensor_destination_receipt(
            &level, &receipt.structure1f_floor_sensor_destinations[level_index]);
        (void)nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
            &level, &receipt.structure1f_floor_sensor_model_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
            &level, &receipt.structure1f_floor_sensor_extent_pairs[level_index]);
        (void)nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
            &level, &receipt.structure1f_floor_decoration_payload_selectors[level_index]);
        (void)nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
            &level, &receipt.structure1f_floor_decoration_rotation_selectors[level_index]);
        (void)nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
            &level, &receipt.structure1f_floor_decoration_model_rotation_pairs[level_index]);
        (void)nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(
            &level, &receipt.structure1f_floor_decoration_offset_pairs[level_index]);
        (void)nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
            &level, &receipt.structure1f_floor_decoration_control_extents[level_index]);
        (void)nexus_v1_level_structure1f_item_attribute_pair_receipt(
            &level, &receipt.structure1f_item_attribute_pairs[level_index]);
        (void)nexus_v1_level_structure1f_item_location_pair_receipt(
            &level, &receipt.structure1f_item_location_pairs[level_index]);
        (void)nexus_v1_level_structure1f_item_coordinate_pair_receipt(
            &level, &receipt.structure1f_item_coordinate_pairs[level_index]);
        nexus_v1_dgn_structure1f_owner_model_selector_corpus_receipt(
            level_index, &level_source, &level,
            &receipt.structure1f_owner_model_selectors[level_index]);
        (void)nexus_v1_level_structure3_ordinal_correlation_receipt(
            &level, &receipt.structure3_ordinal_correlations[level_index]);
        if (level.structure3_payload.declared) {
            ++receipt.structure3_payload_declared_level_count;
        }
        if (level.structure3_payload.valid) {
            ++receipt.structure3_payload_valid_level_count;
            receipt.structure3_payload_byte_count +=
                level.structure3_payload.byte_size;
            receipt.structure3_payload_nonzero_byte_count +=
                level.structure3_payload.nonzero_byte_count;
            receipt.structure3_payload_transition_count +=
                level.structure3_payload.byte_transition_count;
            receipt.structure3_nonzero_byte_run_count +=
                level.structure3_payload.nonzero_byte_run_count;
            if (level.structure3_payload.longest_nonzero_byte_run >
                receipt.structure3_longest_nonzero_byte_run) {
                receipt.structure3_longest_nonzero_byte_run =
                    level.structure3_payload.longest_nonzero_byte_run;
            }
            receipt.structure3_zero_block_count +=
                level.structure3_payload.zero_block_count;
            receipt.structure3_nonzero_block_count +=
                level.structure3_payload.nonzero_block_count;
            receipt.structure3_nonzero_block_run_count +=
                level.structure3_payload.nonzero_block_run_count;
            if (level.structure3_payload.longest_nonzero_block_run >
                receipt.structure3_longest_nonzero_block_run) {
                receipt.structure3_longest_nonzero_block_run =
                    level.structure3_payload.longest_nonzero_block_run;
            }
        }
        if (level.structure3_directory.valid) {
            ++receipt.structure3_directory_valid_level_count;
            receipt.structure3_directory_entry_count +=
                level.structure3_directory.entry_count;
        }
        if (level.structure3_entry_headers.valid) {
            ++receipt.structure3_entry_header_valid_level_count;
            receipt.structure3_entry_header_entry_count +=
                level.structure3_entry_headers.entry_count;
            receipt.structure3_entry_header_first_region_element_count +=
                level.structure3_entry_headers.first_region_element_count;
            receipt.structure3_entry_header_second_region_element_count +=
                level.structure3_entry_headers.second_region_element_count;
        }
        if (receipt.structure3_model_references[level_index].complete) {
            ++receipt.structure3_model_reference_complete_level_count;
        }
        if (receipt.structure1a_transform_selectors[level_index].complete) {
            ++receipt.structure1a_transform_selector_complete_level_count;
        }
        if (receipt.structure1f_face_selectors[level_index].complete) {
            ++receipt.structure1f_face_selector_complete_level_count;
        }
        if (receipt.structure1f_rotation_selectors[level_index].complete) {
            ++receipt.structure1f_rotation_selector_complete_level_count;
        }
        if (receipt.structure1f_face_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_face_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_offset_pairs[level_index].complete) {
            ++receipt.structure1f_offset_pair_complete_level_count;
        }
        if (receipt.structure1f_wall_payload_selectors[level_index].complete) {
            ++receipt.structure1f_wall_payload_selector_complete_level_count;
        }
        if (receipt.structure1f_wall_sensor_destinations[level_index].complete) {
            ++receipt.structure1f_wall_sensor_destination_complete_level_count;
        }
        if (receipt.structure1f_wall_sensor_control_selectors[level_index].complete) {
            ++receipt.structure1f_wall_sensor_control_selector_complete_level_count;
        }
        if (receipt.structure1f_wall_sensor_control_destination_tuples[level_index].complete) {
            ++receipt.structure1f_wall_sensor_control_destination_tuple_complete_level_count;
        }
        if (receipt.structure1f_wall_sensor_model_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_wall_sensor_model_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_wall_decoration_model_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_wall_decoration_model_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_alcove_payload_selectors[level_index].complete) {
            ++receipt.structure1f_alcove_payload_selector_complete_level_count;
        }
        if (receipt.structure1f_alcove_payload_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_alcove_payload_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_floor_sensor_control_selectors[level_index].complete) {
            ++receipt.structure1f_floor_sensor_control_selector_complete_level_count;
        }
        if (receipt.structure1f_floor_sensor_destinations[level_index].complete) {
            ++receipt.structure1f_floor_sensor_destination_complete_level_count;
        }
        if (receipt.structure1f_floor_sensor_model_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_floor_sensor_model_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_floor_sensor_extent_pairs[level_index].complete) {
            ++receipt.structure1f_floor_sensor_extent_pair_complete_level_count;
        }
        if (receipt.structure1f_floor_decoration_payload_selectors[level_index].complete) {
            ++receipt.structure1f_floor_decoration_payload_selector_complete_level_count;
        }
        if (receipt.structure1f_floor_decoration_rotation_selectors[level_index].complete) {
            ++receipt.structure1f_floor_decoration_rotation_selector_complete_level_count;
        }
        if (receipt.structure1f_floor_decoration_model_rotation_pairs[level_index].complete) {
            ++receipt.structure1f_floor_decoration_model_rotation_pair_complete_level_count;
        }
        if (receipt.structure1f_floor_decoration_offset_pairs[level_index].complete) {
            ++receipt.structure1f_floor_decoration_offset_pair_complete_level_count;
        }
        if (receipt.structure1f_floor_decoration_control_extents[level_index].complete) {
            ++receipt.structure1f_floor_decoration_control_extent_complete_level_count;
        }
        if (receipt.structure1f_item_attribute_pairs[level_index].complete) {
            ++receipt.structure1f_item_attribute_pair_complete_level_count;
        }
        if (receipt.structure1f_item_location_pairs[level_index].complete) {
            ++receipt.structure1f_item_location_pair_complete_level_count;
        }
        if (receipt.structure1f_item_coordinate_pairs[level_index].complete) {
            ++receipt.structure1f_item_coordinate_pair_complete_level_count;
        }
        if (receipt.structure1f_owner_model_selectors[level_index].valid) {
            ++receipt.structure1f_owner_model_selector_complete_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .zero_based_block_ordinal_mapping_disproven) {
            ++receipt.structure3_zero_based_block_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .one_based_block_ordinal_mapping_disproven) {
            ++receipt.structure3_one_based_block_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .zero_based_byte_run_ordinal_mapping_disproven) {
            ++receipt.structure3_zero_based_byte_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .one_based_byte_run_ordinal_mapping_disproven) {
            ++receipt.structure3_one_based_byte_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .zero_based_run_ordinal_mapping_disproven) {
            ++receipt.structure3_zero_based_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .one_based_run_ordinal_mapping_disproven) {
            ++receipt.structure3_one_based_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .direct_block_ordinal_mapping_disproven) {
            ++receipt.structure3_direct_block_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .direct_byte_run_ordinal_mapping_disproven) {
            ++receipt.structure3_direct_byte_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .direct_run_ordinal_mapping_disproven) {
            ++receipt.structure3_direct_run_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .zero_based_directory_ordinal_mapping_disproven) {
            ++receipt.structure3_zero_based_directory_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .one_based_directory_ordinal_mapping_disproven) {
            ++receipt.structure3_one_based_directory_ordinal_mapping_disproven_level_count;
        }
        if (receipt.structure3_ordinal_correlations[level_index]
                .direct_directory_ordinal_mapping_disproven) {
            ++receipt.structure3_direct_directory_ordinal_mapping_disproven_level_count;
        }
        if (level.geometry_info.mesh_ready) ++receipt.geometry_ready_level_count;
        if (level.geometry_info.structure1f_valid) {
            ++receipt.structure1f_valid_level_count;
            receipt.structure1f_typed_entry_count += level.structure1f_entry_count;
        }
        if (level.geometry_info.structure1g_present)
            ++receipt.structure1g_present_level_count;
        if (level.geometry_info.structure1g_valid) {
            ++receipt.structure1g_valid_level_count;
            receipt.structure1g_animated_texture_count +=
                level.structure1g_entry_count;
            receipt.structure1g_sequence_count +=
                level.geometry_info.structure1g_sequence_count;
            receipt.structure1g_floor_animation_cell_count +=
                level.structure1g_floor_animation_cell_count;
            receipt.structure1g_floor_animation_bound_count +=
                level.structure1g_floor_animation_bound_count;
            for (int entry = 0; entry < level.structure1g_entry_count; ++entry) {
                receipt.structure1g_image_instruction_count +=
                    level.structure1g_entries[entry].image_instruction_count;
                receipt.structure1g_goto_instruction_count +=
                    level.structure1g_entries[entry].goto_instruction_count;
                receipt.structure1g_structure2_image_instruction_bound_count +=
                    level.structure1g_entries[entry]
                        .structure2_image_instruction_bound_count;
                receipt.structure1g_structure2_image_instruction_unbound_count +=
                    level.structure1g_entries[entry]
                        .structure2_image_instruction_unbound_count;
                if (level.structure1g_entries[entry].first_structure2_image_valid)
                    receipt.structure1g_structure2_first_image_bound_count++;
            }
        }
        if (level.structure2_texture_table_valid) {
            receipt.structure2_valid_level_count++;
            receipt.structure2_texture_count += level.structure2_texture_count;
        }
        if (level.structure2_payload.valid) {
            receipt.structure2_payload_envelope_valid_level_count++;
            receipt.structure2_opaque_payload_byte_count +=
                level.structure2_payload.opaque_payload_size;
            receipt.structure2_nonzero_descriptor_offset_count +=
                level.structure2_payload.nonzero_descriptor_offset_count;
            receipt.structure2_descriptor_offsets_in_opaque_payload_count +=
                level.structure2_payload
                    .nonzero_descriptor_offsets_in_opaque_payload_count;
            receipt.structure2_descriptor_offsets_outside_opaque_payload_count +=
                level.structure2_payload
                    .nonzero_descriptor_offsets_outside_opaque_payload_count;
            receipt.structure2_descriptor_offsets_word_bounded_count +=
                level.structure2_payload
                    .nonzero_descriptor_offsets_word_bounded_count;
            receipt.structure2_descriptor_offsets_unaligned_count +=
                level.structure2_payload
                    .nonzero_descriptor_offsets_unaligned_count;
            receipt.structure2_descriptor_offset_unique_count +=
                level.structure2_payload.nonzero_descriptor_offset_unique_count;
            receipt.structure2_descriptor_offset_reused_count +=
                level.structure2_payload.nonzero_descriptor_offset_reused_count;
            if (level.structure2_payload.local_payload_offset_pattern_observed) {
                receipt.structure2_local_payload_offset_pattern_level_count++;
            }
            if (level.structure2_payload
                    .local_payload_word_aligned_offset_pattern_observed) {
                receipt.structure2_local_payload_word_aligned_offset_pattern_level_count++;
            }
            if (level.structure2_payload
                    .local_payload_word_bounded_offset_pattern_observed) {
                receipt.structure2_local_payload_word_bounded_offset_pattern_level_count++;
            }
            if (level.structure2_payload.material_or_image_data_proven) {
                receipt.structure2_material_or_image_data_proven_level_count++;
            }
        }
        if (receipt.structure2_sources[level_index].canonical_hash_verified) {
            ++receipt.structure2_canonical_source_verified_level_count;
        }
        if (receipt.structure2_sources[level_index].materialization_bound) {
            ++receipt.structure2_materialization_bound_level_count;
        }
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                int dir;
                floor_refs[floor_count++] = level.floor_material_refs[y][x];
                ceiling_refs[ceiling_count++] =
                    level.ceiling_material_refs[y][x];
                for (dir = 0; dir < 4; ++dir) {
                    wall_refs[wall_count++] =
                        level.wall_material_refs[y][x][dir];
                }
            }
        }
    }
    (void)nexus_v1_dmdf_material_category_coverage_receipt(
        &engine->floor_materials, NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR,
        floor_refs, floor_count, &receipt.floor_coverage);
    /* The live viewport deliberately resolves typed ceiling selectors through
     * FLOORS material data. This is recorded as bank provenance only, not a
     * claim that the DGN selector denotes a separate asset family. */
    (void)nexus_v1_dmdf_material_category_coverage_receipt(
        &engine->floor_materials, NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING,
        ceiling_refs, ceiling_count, &receipt.ceiling_coverage);
    (void)nexus_v1_dmdf_material_category_coverage_receipt(
        &engine->wall_materials, NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
        wall_refs, wall_count, &receipt.wall_coverage);
    receipt.floor_container = engine->floor_bpk_container;
    receipt.wall_container = engine->wall_bpk_container;
    receipt.static_mns_sources = engine->dgn_static_material_sources;
    receipt.bpk_host_routes_complete =
        engine->floor_bpk_host_route.host_consumed_surfaces &&
        engine->wall_bpk_host_route.host_consumed_surfaces;
    receipt.static_mns_host_route_complete =
        receipt.static_mns_sources.canonical_pair_bound &&
        receipt.static_mns_sources.structure1b_selector_binding_proven &&
        !receipt.static_mns_sources.fallback_visuals_permitted &&
        engine->floor_mns_material_route_valid &&
        engine->wall_mns_material_route_valid;
    receipt.material_coverage_complete =
        receipt.parsed_level_count == receipt.expected_level_count &&
        receipt.geometry_ready_level_count == receipt.expected_level_count &&
        receipt.floor_coverage.covered && receipt.ceiling_coverage.covered &&
        receipt.wall_coverage.covered;
    /* Retail Track 1 static geometry consumes only the authenticated MNS
     * pair. BPK remains an independent optional route and cannot hold the
     * real corpus receipt hostage or promote opaque MENU.BPK PRS3 bytes. */
    receipt.host_route_evidence_complete =
        receipt.material_coverage_complete &&
        receipt.static_mns_host_route_complete;

done:
    free(floor_refs);
    free(ceiling_refs);
    free(wall_refs);
    engine->dgn_material_corpus = receipt;
    *out_receipt = receipt;
    return receipt.parsed_level_count == receipt.expected_level_count ? 0 : -1;
}

void nexus_v1_invalidate_dgn_material_plan(Nexus_V1_Engine *engine) {
    Nexus_V1_DgnMaterialPlan *plan;
    if (!engine) return;
    plan = &engine->dgn_material_plan;
    plan->generation++;
    plan->invalidation_count++;
    plan->valid = 0;
    plan->receipt.plan_ready = 0;
}

int nexus_v1_current_level_dgn_face_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt)
{
    Nexus_V1_DgnFaceMaterialBinding bindings[NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES];
    Nexus_V1_DgnFaceMaterialInput input;
    Nexus_V1_DgnStructure3FaceMaterialReceipt materials;
    char level_name[16];
    int binding_count = 0;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT;
    snprintf(level_name, sizeof(level_name), "LEV%02d.DGN",
             engine ? engine->game.current_level : 0);
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0 ||
        !nexus_v1_dgn_bytes_match_canonical_md5(
            engine->current_level_dgn_data, engine->current_level_dgn_size,
            nexus_known_boot_file_md5(level_name))) {
        out_receipt->status = NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE;
        return 0;
    }
    if (nexus_v1_level_collect_structure3_face_material_bindings(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, bindings,
            NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES, &binding_count) != 0) {
        return 0;
    }
    if (nexus_v1_level_structure3_face_material_receipt(
            &engine->current_level, &materials) != 0 ||
        !materials.valid || !materials.face_receipt_valid ||
        !materials.selector_bindings_complete ||
        materials.textured_face_count != binding_count) {
        out_receipt->status = NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE;
        return 0;
    }
    memset(&input, 0, sizeof(input));
    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.dgn_bytes = engine->current_level_dgn_data;
    input.dgn_size = engine->current_level_dgn_size;
    /* The MD5 above authenticated this exact retained launch buffer. */
    input.canonical_dgn_bytes = engine->current_level_dgn_data;
    input.canonical_dgn_size = engine->current_level_dgn_size;
    input.canonical_source_verified = 1;
    input.bindings = bindings;
    input.face_count = binding_count;
    input.structure2_descriptor_count = engine->current_level.structure2_texture_count;
    input.material_selector_count = 256;
    /* Geometry readiness comes from the restored Structure3 mesh
     * extractor over this exact authenticated buffer, not from
     * level.geometry_info.mesh_ready (that field gates collision and
     * post-grid record validation and stays 0 for the whole retail
     * LEV00-LEV15 corpus even though every mesh entry extracts and
     * builds to READY_GEOMETRY). */
    {
        const int geometry_ready =
            nexus_v1_level_structure3_mesh_geometry_ready(
                &engine->current_level, engine->current_level_dgn_data,
                engine->current_level_dgn_size, NULL);
        input.geometry_source_bound = geometry_ready;
        input.geometry_can_submit_geometry = geometry_ready;
    }
    input.geometry_material_face_count = binding_count;
    input.geometry_can_submit_textured_raster = 0;
    input.geometry_fallback_visuals_permitted = 0;
    return nexus_v1_dgn_face_material_validate(&input, out_receipt) ? 0 : -1;
}

void nexus_v1_sync_dgn_runtime_pose(Nexus_V1_Engine *engine,
                                    int level, int party_x, int party_y,
                                    int party_dir) {
    if (!engine) return;
    party_dir &= 3;
    if (engine->game.current_level != level) {
        nexus_v1_engine_clear_external_prs3_placement_receipt(engine);
    } else if (engine->game.party_x != party_x ||
               engine->game.party_y != party_y ||
        engine->game.party_dir != party_dir) {
        nexus_v1_invalidate_dgn_material_plan(engine);
    }
    engine->game.current_level = level;
    engine->game.party_x = party_x;
    engine->game.party_y = party_y;
    engine->game.party_dir = party_dir;
}

void nexus_v1_engine_clear_external_prs3_placement_receipt(
    Nexus_V1_Engine *engine)
{
    if (!engine) return;
    engine->external_prs3_placement_valid = 0;
    engine->external_saturn_save_capture_valid = 0;
    engine->external_saturn_save_card_fnv1a64 = 0U;
    engine->external_saturn_save_route_epoch = 0U;
    engine->external_prs3_placement_level = 0;
    engine->external_prs3_placement_route_epoch = 0U;
    engine->external_prs3_placement_dgn_fnv1a64 = 0U;
    engine->external_prs3_placement_descriptor_index = 0U;
    engine->external_prs3_placement_frame_sequence = 0U;
    engine->external_prs3_placement_command_sequence = 0U;
    engine->external_prs3_placement_descriptor_fnv1a64 = 0U;
    engine->external_prs3_placement_trace_fnv1a64 = 0U;
    engine->external_prs3_placement_trace_size = 0U;
    engine->external_prs3_placement_header_fnv1a64 = 0U;
    engine->external_prs3_placement_bitmap_fnv1a64 = 0U;
    engine->external_prs3_placement_bitmap_offset = 0U;
    engine->external_prs3_placement_bitmap_size = 0U;
    engine->external_prs3_placement_palt_fnv1a64 = 0U;
    engine->external_prs3_placement_palt_size = 0U;
    engine->external_dgn_campaign_capture_ready = 0;
    engine->external_dgn_campaign_capture_level = -1;
    engine->external_dgn_campaign_capture_dgn_fnv1a64 = 0U;
    engine->external_dgn_campaign_capture_trace_fnv1a64 = 0U;
    engine->external_dgn_campaign_capture_trace_size = 0U;
    engine->external_dgn_campaign_capture_frame_sequence = 0U;
    engine->external_dgn_campaign_capture_command_sequence = 0U;
    engine->external_structure1f_placement_valid = 0;
    engine->external_structure1f_placement_dgn_fnv1a64 = 0U;
    engine->external_structure1f_placement_descriptor_index = 0U;
    engine->external_structure1f_placement_frame_sequence = 0U;
    engine->external_structure1f_placement_command_sequence = 0U;
    engine->external_structure1f_placement_descriptor_fnv1a64 = 0U;
    nexus_v1_invalidate_dgn_material_plan(engine);
}

int nexus_v1_engine_set_saturn_save_capture_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    const Nexus_V1_SaturnSaveCaptureReceipt *receipt)
{
    if (!engine) return 0;
    engine->external_saturn_save_capture_valid = 0;
    engine->external_saturn_save_card_fnv1a64 = 0U;
    engine->external_saturn_save_route_epoch = 0U;
    if (!receipt || !route_epoch || route_epoch <= engine->external_saturn_save_last_route_epoch ||
        !receipt->valid || receipt->status != NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE ||
        !receipt->opaque_only || receipt->native_save_fallback_permitted ||
        !receipt->title_route_bound || !receipt->champion_route_bound ||
        receipt->image_bytes != NEXUS_V1_SATURN_SAVE_IMAGE_BYTES || !receipt->image_fnv1a64) return 0;
    engine->external_saturn_save_capture_valid = 1;
    engine->external_saturn_save_card_fnv1a64 = receipt->image_fnv1a64;
    engine->external_saturn_save_route_epoch = route_epoch;
    engine->external_saturn_save_last_route_epoch = route_epoch;
    return 1;
}
int nexus_v1_engine_saturn_save_capture_ready(const Nexus_V1_Engine *engine,
                                               uint64_t route_epoch, uint64_t card_fnv1a64)
{ return engine && engine->external_saturn_save_capture_valid && route_epoch &&
    route_epoch == engine->external_saturn_save_route_epoch && card_fnv1a64 &&
    card_fnv1a64 == engine->external_saturn_save_card_fnv1a64; }

int nexus_v1_engine_set_dgn_multi_level_capture_adjudication(
    Nexus_V1_Engine *engine,
    const struct Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt *receipt)
{
    const Nexus_V1_DgnMultiLevelCaptureCoverage *coverage;
    if (!engine) return 0;
    engine->external_dgn_campaign_capture_ready = 0;
    engine->external_dgn_campaign_capture_level = -1;
    engine->external_dgn_campaign_capture_dgn_fnv1a64 = 0U;
    engine->external_dgn_campaign_capture_trace_fnv1a64 = 0U;
    engine->external_dgn_campaign_capture_trace_size = 0U;
    engine->external_dgn_campaign_capture_frame_sequence = 0U;
    engine->external_dgn_campaign_capture_command_sequence = 0U;
    if (!receipt || !receipt->valid || !receipt->opaque_original_capture_only ||
        receipt->decoder_promoted || receipt->mesh_semantics_permitted ||
        receipt->render_permitted || !engine->level_loaded ||
        engine->game.current_level < 0 || engine->game.current_level >= 16) return 0;
    coverage = &receipt->levels[engine->game.current_level];
    if (!coverage->valid || !coverage->opaque_original_capture_covered ||
        !coverage->dgn_fnv1a64 || !coverage->trace_fnv1a64 || !coverage->trace_size || !coverage->frame_sequence ||
        !coverage->command_sequence ||
        coverage->dgn_fnv1a64 != engine->current_level_structure2_source.loaded_dgn_fnv1a64 ||
        !engine->external_prs3_placement_valid ||
        engine->external_prs3_placement_level != engine->game.current_level ||
        engine->external_prs3_placement_dgn_fnv1a64 != coverage->dgn_fnv1a64 ||
        engine->external_prs3_placement_trace_fnv1a64 != coverage->trace_fnv1a64 ||
        engine->external_prs3_placement_trace_size != coverage->trace_size ||
        engine->external_prs3_placement_frame_sequence != coverage->frame_sequence ||
        engine->external_prs3_placement_command_sequence != coverage->command_sequence) return 0;
    engine->external_dgn_campaign_capture_ready = 1;
    engine->external_dgn_campaign_capture_level = engine->game.current_level;
    engine->external_dgn_campaign_capture_dgn_fnv1a64 = coverage->dgn_fnv1a64;
    engine->external_dgn_campaign_capture_trace_fnv1a64 = coverage->trace_fnv1a64;
    engine->external_dgn_campaign_capture_trace_size = coverage->trace_size;
    engine->external_dgn_campaign_capture_frame_sequence = coverage->frame_sequence;
    engine->external_dgn_campaign_capture_command_sequence = coverage->command_sequence;
    return 1;
}

int nexus_v1_engine_current_level_dgn_capture_ready(const Nexus_V1_Engine *engine)
{
    return engine && engine->level_loaded && engine->external_dgn_campaign_capture_ready &&
        engine->external_dgn_campaign_capture_level == engine->game.current_level &&
        engine->external_dgn_campaign_capture_dgn_fnv1a64 &&
        engine->external_dgn_campaign_capture_dgn_fnv1a64 ==
            engine->current_level_structure2_source.loaded_dgn_fnv1a64 &&
        engine->external_dgn_campaign_capture_trace_fnv1a64 &&
        engine->external_dgn_campaign_capture_trace_size &&
        engine->external_prs3_placement_valid &&
        engine->external_prs3_placement_level == engine->game.current_level &&
        engine->external_prs3_placement_dgn_fnv1a64 == engine->external_dgn_campaign_capture_dgn_fnv1a64 &&
        engine->external_prs3_placement_trace_fnv1a64 == engine->external_dgn_campaign_capture_trace_fnv1a64 &&
        engine->external_prs3_placement_trace_size == engine->external_dgn_campaign_capture_trace_size &&
        engine->external_prs3_placement_frame_sequence == engine->external_dgn_campaign_capture_frame_sequence &&
        engine->external_prs3_placement_command_sequence == engine->external_dgn_campaign_capture_command_sequence &&
        engine->external_dgn_campaign_capture_frame_sequence &&
        engine->external_dgn_campaign_capture_command_sequence;
}

int nexus_v1_engine_set_external_prs3_placement_receipt(
    Nexus_V1_Engine *engine,
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *receipt)
{
    if (!engine) return 0;

    /* An attempted replacement is a new external route epoch.  Clear the
     * prior immutable binding before validation so rejected candidate bytes
     * cannot leave stale placement evidence eligible for material planning. */
    nexus_v1_engine_clear_external_prs3_placement_receipt(engine);

    if (!receipt || !receipt->valid || !receipt->trace_bound ||
        !receipt->dgn_source_bound || !receipt->descriptor_envelope_bound ||
        !receipt->command_coordinates_bound || !receipt->no_draw_only ||
        !receipt->blocks_real_dgn_mesh_render ||
        receipt->fallback_visuals_permitted || !receipt->trace_fnv1a64 || !receipt->trace_size ||
        !receipt->dgn_fnv1a64 || !engine->level_loaded ||
        !receipt->dgn_placement_observed || !receipt->frame_sequence ||
        !receipt->command_sequence || !receipt->descriptor_fnv1a64 ||
        !receipt->prs3_header_span_fnv1a64 ||
        !receipt->prs3_bitmap_candidate_fnv1a64 ||
        !receipt->prs3_bitmap_candidate_size ||
        !receipt->palt_candidate_fnv1a64 ||
        receipt->palt_candidate_size != 512U ||
        receipt->dgn_fnv1a64 !=
            engine->current_level_structure2_source.loaded_dgn_fnv1a64)
        return 0;
    engine->external_prs3_placement_valid = 1;
    engine->external_prs3_placement_level = engine->game.current_level;
    engine->external_prs3_placement_dgn_fnv1a64 = receipt->dgn_fnv1a64;
    engine->external_prs3_placement_descriptor_index = receipt->descriptor_index;
    engine->external_prs3_placement_frame_sequence = receipt->frame_sequence;
    engine->external_prs3_placement_command_sequence = receipt->command_sequence;
    engine->external_prs3_placement_descriptor_fnv1a64 =
        receipt->descriptor_fnv1a64;
    engine->external_prs3_placement_trace_fnv1a64 = receipt->trace_fnv1a64;
    engine->external_prs3_placement_trace_size = receipt->trace_size;
    engine->external_prs3_placement_header_fnv1a64 =
        receipt->prs3_header_span_fnv1a64;
    engine->external_prs3_placement_bitmap_fnv1a64 =
        receipt->prs3_bitmap_candidate_fnv1a64;
    engine->external_prs3_placement_bitmap_offset =
        receipt->prs3_bitmap_candidate_offset;
    engine->external_prs3_placement_bitmap_size =
        receipt->prs3_bitmap_candidate_size;
    engine->external_prs3_placement_palt_fnv1a64 =
        receipt->palt_candidate_fnv1a64;
    engine->external_prs3_placement_palt_size = receipt->palt_candidate_size;
    return 1;
}

int nexus_v1_engine_set_external_prs3_replay_placement_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t expected_trace_fnv1a64, uint64_t expected_dgn_fnv1a64,
    uint64_t expected_bitmap_candidate_fnv1a64,
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *receipt)
{
    uint64_t prior_epoch;
    if (!engine) return 0;
    prior_epoch = engine->external_prs3_replay_last_route_epoch;
    if (!route_epoch || !receipt || route_epoch <= prior_epoch ||
        receipt->trace_fnv1a64 != expected_trace_fnv1a64 ||
        receipt->dgn_fnv1a64 != expected_dgn_fnv1a64 ||
        receipt->prs3_bitmap_candidate_fnv1a64 !=
            expected_bitmap_candidate_fnv1a64) {
        nexus_v1_engine_clear_external_prs3_placement_receipt(engine);
        return 0;
    }
    if (!nexus_v1_engine_set_external_prs3_placement_receipt(engine, receipt))
        return 0;
    engine->external_prs3_placement_route_epoch = route_epoch;
    engine->external_prs3_replay_last_route_epoch = route_epoch;
    return 1;
}

int nexus_v1_engine_set_external_structure1f_placement_binding(
    Nexus_V1_Engine *engine,
    const Nexus_V1_Structure1FPlacementBindingReceipt *receipt)
{
    if (!engine) return 0;
    engine->external_structure1f_placement_valid = 0;
    engine->external_structure1f_placement_dgn_fnv1a64 = 0U;
    engine->external_structure1f_placement_descriptor_index = 0U;
    engine->external_structure1f_placement_frame_sequence = 0U;
    engine->external_structure1f_placement_command_sequence = 0U;
    engine->external_structure1f_placement_descriptor_fnv1a64 = 0U;
    nexus_v1_invalidate_dgn_material_plan(engine);
    if (!receipt || !receipt->valid || !receipt->placement_observed ||
        !receipt->no_draw_only || !receipt->blocks_real_dgn_mesh_render ||
        receipt->fallback_visuals_permitted || !receipt->dgn_fnv1a64 ||
        !receipt->frame_sequence || !receipt->command_sequence ||
        !receipt->descriptor_fnv1a64 || !engine->external_prs3_placement_valid ||
        receipt->dgn_fnv1a64 != engine->external_prs3_placement_dgn_fnv1a64 ||
        receipt->descriptor_index !=
            engine->external_prs3_placement_descriptor_index ||
        receipt->frame_sequence !=
            engine->external_prs3_placement_frame_sequence ||
        receipt->command_sequence !=
            engine->external_prs3_placement_command_sequence ||
        receipt->descriptor_fnv1a64 !=
            engine->external_prs3_placement_descriptor_fnv1a64) {
        return 0;
    }
    engine->external_structure1f_placement_valid = 1;
    engine->external_structure1f_placement_dgn_fnv1a64 = receipt->dgn_fnv1a64;
    engine->external_structure1f_placement_descriptor_index =
        receipt->descriptor_index;
    engine->external_structure1f_placement_frame_sequence =
        receipt->frame_sequence;
    engine->external_structure1f_placement_command_sequence =
        receipt->command_sequence;
    engine->external_structure1f_placement_descriptor_fnv1a64 =
        receipt->descriptor_fnv1a64;
    return 1;
}

int nexus_v1_current_external_structure1f_placement_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_ExternalStructure1FPlacementReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !engine->level_loaded ||
        !engine->external_prs3_placement_valid ||
        !engine->external_structure1f_placement_valid ||
        engine->external_prs3_placement_level != engine->game.current_level ||
        !engine->external_prs3_placement_dgn_fnv1a64 ||
        !engine->external_prs3_placement_frame_sequence ||
        !engine->external_prs3_placement_command_sequence ||
        !engine->external_structure1f_placement_descriptor_fnv1a64 ||
        engine->external_prs3_placement_dgn_fnv1a64 !=
            engine->current_level_structure2_source.loaded_dgn_fnv1a64 ||
        engine->external_structure1f_placement_dgn_fnv1a64 !=
            engine->external_prs3_placement_dgn_fnv1a64 ||
        engine->external_structure1f_placement_descriptor_index !=
            engine->external_prs3_placement_descriptor_index ||
        engine->external_structure1f_placement_frame_sequence !=
            engine->external_prs3_placement_frame_sequence ||
        engine->external_structure1f_placement_command_sequence !=
            engine->external_prs3_placement_command_sequence ||
        engine->external_structure1f_placement_descriptor_fnv1a64 !=
            engine->external_prs3_placement_descriptor_fnv1a64) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->no_draw_only = 1;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    out_receipt->dgn_fnv1a64 = engine->external_structure1f_placement_dgn_fnv1a64;
    out_receipt->descriptor_index =
        engine->external_structure1f_placement_descriptor_index;
    out_receipt->frame_sequence =
        engine->external_structure1f_placement_frame_sequence;
    out_receipt->command_sequence =
        engine->external_structure1f_placement_command_sequence;
    out_receipt->descriptor_fnv1a64 =
        engine->external_structure1f_placement_descriptor_fnv1a64;
    return 1;
}

const Nexus_V1_DgnMaterialPlan *nexus_v1_prepare_dgn_material_plan(
    Nexus_V1_Engine *engine, int party_x, int party_y, int party_dir)
{
    Nexus_V1_DgnMaterialPlan *plan;
    int static_mns_route_bound;
    int bpk_material_route_bound;
    int structure2_source_bound;
    int item_ibs_authorized_count;
    int i;
    Nexus_V1_DgnStructure2SourceReceipt structure2_source;
    Nexus_V1_DgnActiveStructure2DescriptorReceipt structure2_descriptors;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget placement_target;

    if (!engine || !engine->level_loaded ||
        !engine->current_level.geometry_info.dmweb_container) return NULL;
    plan = &engine->dgn_material_plan;
    party_dir &= 3;
    if (plan->valid && plan->level == engine->game.current_level &&
        plan->party_x == party_x && plan->party_y == party_y &&
        plan->party_dir == party_dir &&
        plan->geometry_generation == plan->generation) {
        plan->cache_hit_count++;
        return plan;
    }

    memset(plan->commands, 0, sizeof(plan->commands));
    memset(&plan->receipt, 0, sizeof(plan->receipt));
    memset(&plan->structure3_face_material_source, 0,
           sizeof(plan->structure3_face_material_source));
    plan->structure3_face_material_source_consumed = 0;
    memset(plan->structure2_floor_command_sources, 0,
           sizeof(plan->structure2_floor_command_sources));
    memset(&plan->structure2_floor_command_source_receipt, 0,
           sizeof(plan->structure2_floor_command_source_receipt));
    plan->structure2_source_level_index = -1;
    plan->structure2_source_canonical_hash_verified = 0;
    plan->structure2_source_envelope_valid = 0;
    plan->external_prs3_placement_bound = 0;
    plan->external_prs3_placement_trace_fnv1a64 = 0U;
    plan->external_prs3_placement_header_fnv1a64 = 0U;
    plan->external_prs3_placement_bitmap_fnv1a64 = 0U;
    plan->external_prs3_placement_bitmap_offset = 0U;
    plan->external_prs3_placement_bitmap_size = 0U;
    plan->external_prs3_placement_palt_fnv1a64 = 0U;
    plan->external_prs3_placement_palt_size = 0U;
    plan->external_prs3_placement_dgn_fnv1a64 = 0U;
    plan->external_prs3_placement_descriptor_index = 0U;
    plan->external_prs3_placement_frame_sequence = 0U;
    plan->external_prs3_placement_command_sequence = 0U;
    plan->external_prs3_placement_route_epoch = 0U;
    plan->external_prs3_placement_descriptor_target_bound = 0;
    plan->external_prs3_placement_descriptor_fnv1a64 = 0U;
    plan->external_prs3_placement_image_anchor_offset = 0U;
    plan->external_prs3_placement_image_candidate_fnv1a64 = 0U;
    plan->external_prs3_placement_palette_anchor_offset = 0U;
    plan->external_prs3_placement_palette_candidate_fnv1a64 = 0U;
    plan->external_structure1f_placement_bound = 0;
    plan->external_structure1f_placement_dgn_fnv1a64 = 0U;
    plan->external_structure1f_placement_descriptor_index = 0U;
    plan->external_structure1f_placement_frame_sequence = 0U;
    plan->external_structure1f_placement_command_sequence = 0U;
    plan->external_structure1f_placement_descriptor_fnv1a64 = 0U;
    plan->structure2_floor_command_sources_consumed = 0;
    memset(plan->structure1f_item_command_bindings, 0,
           sizeof(plan->structure1f_item_command_bindings));
    memset(&plan->structure1f_item_command_binding_receipt, 0,
           sizeof(plan->structure1f_item_command_binding_receipt));
    plan->structure1f_item_command_sources_consumed = 0;
    memset(plan->structure1f_item_floor_materials, 0,
           sizeof(plan->structure1f_item_floor_materials));
    memset(&plan->structure1f_item_floor_material_receipt, 0,
           sizeof(plan->structure1f_item_floor_material_receipt));
    plan->structure1f_item_floor_materials_consumed = 0;
    memset(plan->structure1f_direct_floor_sources, 0,
           sizeof(plan->structure1f_direct_floor_sources));
    memset(&plan->structure1f_direct_floor_source_receipt, 0,
           sizeof(plan->structure1f_direct_floor_source_receipt));
    plan->structure1f_direct_floor_sources_consumed = 0;
    memset(plan->structure1a_owned_cell_sources, 0,
           sizeof(plan->structure1a_owned_cell_sources));
    memset(&plan->structure1a_owned_cell_source_receipt, 0,
           sizeof(plan->structure1a_owned_cell_source_receipt));
    plan->structure1a_owned_cell_sources_consumed = 0;
    memset(plan->structure1a_structure3_topology_candidates, 0,
           sizeof(plan->structure1a_structure3_topology_candidates));
    memset(&plan->structure1a_structure3_topology_candidate_receipt, 0,
           sizeof(plan->structure1a_structure3_topology_candidate_receipt));
    plan->structure1a_structure3_topology_candidates_consumed = 0;
    item_ibs_authorized_count = 0;
    plan->level = engine->game.current_level;
    plan->party_x = party_x;
    plan->party_y = party_y;
    plan->party_dir = party_dir;
    plan->valid = 0;
    plan->generation++;
    plan->geometry_generation = plan->generation;
    plan->rebuild_count++;
    /* The direct retail MNS pair is independently authenticated and may
     * supply static Structure1B surfaces. Every other material route,
     * including an otherwise host-ready BPK fixture, remains tied to the
     * current level's canonical Structure2 source receipt. This prevents a
     * stale BPK plan from becoming drawable after its level provenance has
     * been withdrawn, while leaving the hash-bound MNS route independent of
     * opaque Structure2 image payloads. */
    static_mns_route_bound =
        engine->dgn_static_material_sources.canonical_pair_bound &&
        engine->dgn_static_material_sources.structure1b_selector_binding_proven &&
        engine->floor_mns_material_route_valid &&
        engine->wall_mns_material_route_valid;
    bpk_material_route_bound =
        engine->floor_bpk_container.host_route_permitted &&
        engine->wall_bpk_container.host_route_permitted &&
        engine->floor_bpk_host_route_valid &&
        engine->wall_bpk_host_route_valid &&
        engine->floor_bpk_host_route.host_consumed_surfaces &&
        engine->wall_bpk_host_route.host_consumed_surfaces;
    memset(&structure2_source, 0, sizeof(structure2_source));
    memset(&structure2_descriptors, 0, sizeof(structure2_descriptors));
    (void)nexus_v1_current_level_structure2_source_receipt(
        engine, &structure2_source);
    (void)nexus_v1_current_level_structure2_descriptor_receipt(
        engine, &structure2_descriptors);
    structure2_source_bound =
        structure2_source.level_index == engine->game.current_level &&
        structure2_source.canonical_hash_verified &&
        structure2_source.structure2_payload_envelope_valid &&
        structure2_source.materialization_bound &&
        structure2_source.loaded_bytes_bound &&
        !structure2_source.fallback_visuals_permitted;
    structure2_source_bound = structure2_source_bound &&
        engine->external_prs3_placement_valid &&
        engine->external_prs3_placement_level == engine->game.current_level &&
        engine->external_prs3_placement_dgn_fnv1a64 ==
            structure2_source.loaded_dgn_fnv1a64 &&
        engine->external_prs3_placement_trace_fnv1a64 != 0U &&
        engine->external_prs3_placement_frame_sequence != 0U &&
        engine->external_prs3_placement_command_sequence != 0U &&
        structure2_descriptors.valid &&
        structure2_descriptors.level_index == engine->game.current_level &&
        structure2_descriptors.source_bytes_fnv1a64 ==
            structure2_source.loaded_dgn_fnv1a64 &&
        structure2_descriptors.descriptor_count > 0 &&
        engine->external_prs3_placement_descriptor_index <
            (uint32_t)structure2_descriptors.descriptor_count;
    structure2_source_bound = structure2_source_bound &&
        engine->external_prs3_placement_valid &&
        engine->external_prs3_placement_level == engine->game.current_level &&
        engine->external_prs3_placement_route_epoch != 0U &&
        engine->external_prs3_placement_header_fnv1a64 != 0U &&
        engine->external_prs3_placement_bitmap_fnv1a64 != 0U &&
        engine->external_prs3_placement_bitmap_size != 0U &&
        engine->external_prs3_placement_palt_fnv1a64 != 0U &&
        engine->external_prs3_placement_palt_size == 512U;
    memset(&placement_target, 0, sizeof(placement_target));
    if (structure2_source_bound &&
        (nexus_v1_engine_build_structure2_descriptor_capture_target(
             engine, (int)engine->external_prs3_placement_descriptor_index,
             &placement_target) != 1 ||
         !placement_target.valid ||
         placement_target.level_index != engine->game.current_level ||
         placement_target.source_bytes_fnv1a64 !=
             engine->external_prs3_placement_dgn_fnv1a64 ||
         placement_target.descriptor_index !=
             (int)engine->external_prs3_placement_descriptor_index ||
         !placement_target.descriptor_bytes_fnv1a64 ||
         placement_target.descriptor_bytes_fnv1a64 !=
             engine->external_prs3_placement_descriptor_fnv1a64 ||
         !placement_target.image_payload_candidate_bound ||
         !placement_target.image_payload_candidate_fnv1a64 ||
         !placement_target.no_draw_only ||
         placement_target.fallback_visuals_permitted)) {
        structure2_source_bound = 0;
    }
    plan->external_prs3_placement_bound = structure2_source_bound;
    if (structure2_source_bound) {
        plan->external_prs3_placement_trace_fnv1a64 =
            engine->external_prs3_placement_trace_fnv1a64;
        plan->external_prs3_placement_header_fnv1a64 =
            engine->external_prs3_placement_header_fnv1a64;
        plan->external_prs3_placement_bitmap_fnv1a64 =
            engine->external_prs3_placement_bitmap_fnv1a64;
        plan->external_prs3_placement_bitmap_offset =
            engine->external_prs3_placement_bitmap_offset;
        plan->external_prs3_placement_bitmap_size =
            engine->external_prs3_placement_bitmap_size;
        plan->external_prs3_placement_palt_fnv1a64 =
            engine->external_prs3_placement_palt_fnv1a64;
        plan->external_prs3_placement_palt_size =
            engine->external_prs3_placement_palt_size;
        plan->external_prs3_placement_dgn_fnv1a64 =
            engine->external_prs3_placement_dgn_fnv1a64;
        plan->external_prs3_placement_descriptor_index =
            engine->external_prs3_placement_descriptor_index;
        plan->external_prs3_placement_frame_sequence =
            engine->external_prs3_placement_frame_sequence;
        plan->external_prs3_placement_command_sequence =
            engine->external_prs3_placement_command_sequence;
        plan->external_prs3_placement_route_epoch =
            engine->external_prs3_placement_route_epoch;
        plan->external_prs3_placement_descriptor_target_bound = 1;
        plan->external_prs3_placement_descriptor_fnv1a64 =
            placement_target.descriptor_bytes_fnv1a64;
        plan->external_prs3_placement_image_anchor_offset =
            placement_target.image_payload_anchor_offset;
        plan->external_prs3_placement_image_candidate_fnv1a64 =
            placement_target.image_payload_candidate_fnv1a64;
        plan->external_prs3_placement_palette_anchor_offset =
            placement_target.palette_payload_anchor_offset;
        plan->external_prs3_placement_palette_candidate_fnv1a64 =
            placement_target.palette_payload_candidate_fnv1a64;
        if (engine->external_structure1f_placement_valid &&
            engine->external_structure1f_placement_dgn_fnv1a64 ==
                plan->external_prs3_placement_dgn_fnv1a64 &&
            engine->external_structure1f_placement_descriptor_index ==
                plan->external_prs3_placement_descriptor_index &&
            engine->external_structure1f_placement_frame_sequence ==
                plan->external_prs3_placement_frame_sequence &&
            engine->external_structure1f_placement_command_sequence ==
                plan->external_prs3_placement_command_sequence &&
            engine->external_structure1f_placement_descriptor_fnv1a64 ==
                plan->external_prs3_placement_descriptor_fnv1a64) {
            plan->external_structure1f_placement_bound = 1;
            plan->external_structure1f_placement_dgn_fnv1a64 =
                engine->external_structure1f_placement_dgn_fnv1a64;
            plan->external_structure1f_placement_descriptor_index =
                engine->external_structure1f_placement_descriptor_index;
            plan->external_structure1f_placement_frame_sequence =
                engine->external_structure1f_placement_frame_sequence;
            plan->external_structure1f_placement_command_sequence =
                engine->external_structure1f_placement_command_sequence;
            plan->external_structure1f_placement_descriptor_fnv1a64 =
                engine->external_structure1f_placement_descriptor_fnv1a64;
        }
    }
    if (structure2_source.canonical_hash_verified) {
        plan->structure2_source_level_index = structure2_source.level_index;
        plan->structure2_source_canonical_hash_verified = 1;
        plan->structure2_source_envelope_valid =
            structure2_source.structure2_payload_envelope_valid;
    }
    plan->receipt.structure2_source_materialization_bound =
        structure2_source_bound;
    plan->receipt.static_mns_source_pair_bound =
        engine->dgn_static_material_sources.canonical_pair_bound;
    plan->receipt.structure1b_selector_binding_proven =
        engine->dgn_static_material_sources.structure1b_selector_binding_proven;
    plan->receipt.structure2_vdp1_palette_binding_proven = 0;
    plan->receipt.item_ibs_vdp1_command_proven = 0;
    plan->receipt.bpk_material_route_bound = bpk_material_route_bound;
    /* Every visible DGN command still originates in the active LEV bytes,
     * including the static MNS material route.  Do not let a valid texture
     * pair present geometry after the retained DGN has diverged from its
     * canonical package receipt. */
    if (!structure2_source_bound) {
        plan->receipt.status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    if (!static_mns_route_bound &&
        engine->dgn_static_material_sources.canonical_pair_bound &&
        engine->floor_mns_material_route_valid &&
        engine->wall_mns_material_route_valid &&
        !engine->dgn_static_material_sources.structure1b_selector_binding_proven) {
        plan->receipt.status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1B_SELECTOR;
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    if (!static_mns_route_bound && !bpk_material_route_bound) {
        plan->receipt.status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    if (nexus_v1_level_build_dgn_view_render_plan(
            &engine->current_level, party_x, party_y, party_dir,
            plan->commands, NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &plan->receipt) != 0) {
        return NULL;
    }
    /* Rebuild the documented face-selector table from the exact LEV buffer
     * authenticated by the launcher. A parser receipt or a file name alone
     * cannot enter this source boundary, and this receipt grants no draw. */
    if (nexus_v1_current_level_dgn_face_material_source_receipt(
            engine, &plan->structure3_face_material_source) != 0 ||
        !plan->structure3_face_material_source.can_submit_raster_input) {
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->structure3_face_material_source_consumed = 1;
    /* Consume the real DGN command plan once, before its no-draw Structure2
     * gate returns control to the host. The resulting records are raw
     * package provenance only: neither this engine cache nor the viewport
     * can treat them as a decoded texture, palette, animation, or fallback. */
    if (nexus_v1_dgn_bind_structure2_animated_floor_sources(
            &engine->current_level, plan->commands,
            plan->receipt.command_count,
            plan->structure2_floor_command_sources,
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &plan->structure2_floor_command_source_receipt) != 0) {
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->structure2_floor_command_sources_consumed =
        structure2_source_bound &&
        plan->structure2_floor_command_source_receipt.complete &&
        !plan->structure2_floor_command_source_receipt
            .fallback_visuals_permitted;
    /* Preserve every visible direct Structure1F row on its exact floor
     * command before the existing no-draw semantics gate rejects the plan.
     * This is an immutable runtime source receipt, never a request to draw
     * or interpret the copied record. */
    if (nexus_v1_dgn_bind_direct_structure1f_floor_sources(
            &engine->current_level, plan->commands,
            plan->receipt.command_count, plan->structure1f_direct_floor_sources,
            NEXUS_V1_DGN_RUNTIME_DIRECT_SOURCE_MAX,
            &plan->structure1f_direct_floor_source_receipt) != 0) {
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->structure1f_direct_floor_sources_consumed =
        plan->structure1f_direct_floor_source_receipt.complete &&
        !plan->structure1f_direct_floor_source_receipt
             .fallback_visuals_permitted;
    /* Structure1A-bound rows have a verified owner cell and model record,
     * but no face/material/pixel semantics. Keep their cell anchor in the
     * host plan as source evidence only. */
    if (nexus_v1_dgn_bind_structure1a_owned_cell_sources(
            &engine->current_level, plan->commands,
            plan->receipt.command_count, plan->structure1a_owned_cell_sources,
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &plan->structure1a_owned_cell_source_receipt) != 0) {
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->structure1a_owned_cell_sources_consumed =
        plan->structure1a_owned_cell_source_receipt.complete &&
        !plan->structure1a_owned_cell_source_receipt
             .fallback_visuals_permitted;
    if (nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
            &engine->current_level, plan->structure1a_owned_cell_sources,
            plan->structure1a_owned_cell_source_receipt
                .floor_command_source_count,
            plan->structure1a_structure3_topology_candidates,
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &plan->structure1a_structure3_topology_candidate_receipt) != 0) {
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->structure1a_structure3_topology_candidates_consumed =
        /* The owner-cell candidate cannot make an unverified Structure3
         * material join look like source evidence. Consume the parser's
         * bounded face-material receipt alongside the topology envelope, but
         * retain the existing no-draw status: neither receipt identifies a
         * model ordinal, face transform, texel layout, nor raster operation. */
        plan->structure1a_structure3_topology_candidate_receipt.complete &&
        plan->receipt.structure3_face_materials.face_receipt_valid &&
        plan->receipt.structure3_face_materials.valid &&
        plan->receipt.structure3_face_materials.face_count ==
            plan->receipt.structure3_faces.face_count &&
        plan->receipt.structure3_face_materials.textured_face_count ==
            plan->receipt.structure3_faces.textured_face_count &&
        plan->receipt.structure3_face_materials.static_texture_selector_count ==
            plan->receipt.structure3_face_materials.static_texture_bound_count &&
        plan->receipt.structure3_face_materials.animated_texture_selector_count ==
            plan->receipt.structure3_face_materials.animated_texture_bound_count &&
        plan->receipt.structure3_face_materials.unsupported_textured_fill_count ==
            0 &&
        plan->receipt.structure3_face_materials.selector_bindings_complete &&
        !plan->structure1a_structure3_topology_candidate_receipt
             .fallback_visuals_permitted;
    if (plan->structure1a_structure3_topology_candidates_consumed) {
        const Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt
            *topology = &plan->structure1a_structure3_topology_candidate_receipt;
        const Nexus_V1_DgnStructure3PayloadReceipt *payload =
            &engine->current_level.structure3_payload;

        plan->receipt.structure1a_structure3_topology_candidate_count =
            topology->topology_candidate_count;
        plan->receipt.structure1a_structure3_topology_structure1f_binding_count =
            topology->structure1f_binding_count;
        plan->receipt
            .structure1a_structure3_topology_structure1f_face_selector_semantics_proven =
            0;
        plan->receipt
            .structure1a_structure3_topology_structure1a_row_binding_count =
            topology->structure1a_row_binding_count;
        plan->receipt
            .structure1a_structure3_topology_structure1a_kind_semantics_proven =
            0;
        plan->receipt
            .structure1a_structure3_topology_structure1a_model_rotation_binding_count =
            topology->structure1a_model_rotation_binding_count;
        plan->receipt
            .structure1a_structure3_topology_structure1a_model_rotation_semantics_proven =
            0;
        plan->receipt.structure1a_structure3_topology_blocked_invalid_source_count =
            topology->blocked_invalid_source_count;
        plan->receipt.structure1a_structure3_topology_blocked_payload_count =
            topology->blocked_payload_count;
        plan->receipt
            .structure1a_structure3_topology_direct_ordinal_mapping_disproven_count =
            topology->direct_ordinal_mapping_disproven_count;
        plan->receipt.structure1a_structure3_topology_complete = 1;
        plan->receipt.structure1a_structure3_payload_block_offset =
            payload->block_offset;
        plan->receipt.structure1a_structure3_payload_block_count =
            payload->block_count;
        plan->receipt.structure1a_structure3_payload_nonzero_byte_run_count =
            payload->nonzero_byte_run_count;
        plan->receipt.structure1a_structure3_payload_nonzero_block_run_count =
            payload->nonzero_block_run_count;
        plan->receipt.structure1a_structure3_payload_raw_hash =
            payload->raw_payload_hash;
    }
    if (plan->receipt.blocks_real_dgn_mesh_render) {
        /* Source-only topology commands must not fall through to the
         * material loop and become a host plan. */
        plan->receipt.plan_ready = 0;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    /* Direct Structure1Fa records get the same one-way host consumption.
     * The binder returns only original ITEM.IBS descriptor references for
     * floor commands in the current view; it cannot turn an icon/floor byte
     * stream into a raster surface. Unseen records remain covered by the
     * existing Structure1F no-draw gate. */
    if (engine->item_ibs_runtime_source.source_bound) {
        int item_binding_count;
        if (nexus_v1_dgn_bind_structure1f_item_materials(
                &engine->current_level, &engine->item_ibs_bank,
                plan->commands, plan->receipt.command_count,
                plan->structure1f_item_command_bindings,
                NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                &plan->structure1f_item_command_binding_receipt) != 0) {
            plan->receipt.blocks_real_dgn_mesh_render = 1;
            plan->receipt.fallback_visuals_permitted = 0;
            return NULL;
        }
        plan->structure1f_item_command_sources_consumed =
            (plan->structure1f_item_command_binding_receipt
                 .bound_regular_inventory_count +
             plan->structure1f_item_command_binding_receipt
                 .bound_special_floor_palette_count) > 0 &&
            !plan->structure1f_item_command_binding_receipt
                 .fallback_visuals_permitted;
        item_binding_count =
            plan->structure1f_item_command_binding_receipt
                .bound_regular_inventory_count +
            plan->structure1f_item_command_binding_receipt
                .bound_special_floor_palette_count;
        /* Pass descriptor-0008 through the runtime plan only as an exact
         * packed source receipt. The consumer explicitly leaves
         * texel_order_proven and draw_authorized clear. */
        if (nexus_v1_dgn_consume_structure1f_item_floor_materials(
                plan->structure1f_item_command_bindings, item_binding_count,
                plan->commands, plan->receipt.command_count,
                plan->structure1f_item_floor_materials,
                NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
                &plan->structure1f_item_floor_material_receipt) != 0) {
            plan->receipt.blocks_real_dgn_mesh_render = 1;
            plan->receipt.fallback_visuals_permitted = 0;
            return NULL;
        }
        plan->structure1f_item_floor_materials_consumed =
            plan->structure1f_item_floor_material_receipt.complete &&
            !plan->structure1f_item_floor_material_receipt
                 .fallback_visuals_permitted;
    }
    if (!plan->receipt.plan_ready) {
        return NULL;
    }
    /* The lower-level plan builder owns its receipt initialization. Preserve
     * the source evidence for host diagnostics; it is not a static-material
     * permission bit. */
    plan->receipt.structure2_source_materialization_bound =
        structure2_source_bound;
    plan->receipt.structure2_vdp1_palette_binding_proven = 0;
    for (i = 0; i < plan->structure1f_item_floor_material_receipt
             .command_material_count; ++i) {
        if (plan->structure1f_item_floor_materials[i].draw_authorized) {
            ++item_ibs_authorized_count;
        }
    }
    plan->receipt.item_ibs_vdp1_command_proven =
        item_ibs_authorized_count ==
            plan->structure1f_item_floor_material_receipt.command_material_count &&
        item_ibs_authorized_count > 0;
    /* Static MNS takes precedence only after the selector is independently
     * proven. Otherwise the BPK route must carry every command itself. */
    plan->receipt.uses_static_mns_material_route = static_mns_route_bound;
    plan->receipt.uses_bpk_material_route =
        !static_mns_route_bound && bpk_material_route_bound;

    for (i = 0; i < plan->receipt.command_count; ++i) {
        const Nexus_DMDFTextureSurface *surface =
            nexus_v1_plan_surface(
                engine, &plan->commands[i],
                plan->receipt.uses_static_mns_material_route,
                plan->receipt.uses_bpk_material_route);
        if (!surface) {
            /* DMWeb DGN structure selects IDs; a verified BPK host route
             * plus the matching decoded source surface is required. */
            if (plan->receipt.missing_material_count == 0) {
                plan->receipt.first_missing_material_id =
                    plan->commands[i].material_id;
                plan->receipt.first_missing_material_kind =
                    plan->commands[i].kind;
            }
            plan->receipt.missing_material_count++;
        }
    }
    if (plan->receipt.missing_material_count > 0) {
        plan->receipt.plan_ready = 0;
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->valid = 1;
    return plan;
}

int nexus_v1_engine_build_structure1a_structure3_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt receipt;
    const Nexus_V1_DgnMaterialPlan *plan;

    if (!out_target || !out_receipt) return 0;
    memset(out_target, 0, sizeof(*out_target));
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.active_canonical_lev_bound =
        engine->current_level_structure2_source.level_index ==
            engine->game.current_level &&
        engine->current_level_structure2_source.canonical_hash_verified &&
        engine->current_level_structure2_source.materialization_bound &&
        engine->current_level_structure2_source.loaded_bytes_bound &&
        nexus_v1_dgn_source_bytes_match(&engine->current_level_structure2_source,
                                        engine->current_level_dgn_data,
                                        engine->current_level_dgn_size);
    if (!receipt.active_canonical_lev_bound) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_prepare_dgn_material_plan(
        engine, engine->game.party_x, engine->game.party_y,
        engine->game.party_dir);
    plan = &engine->dgn_material_plan;
    receipt.material_plan_prepared = plan->level == engine->game.current_level &&
        plan->structure1a_structure3_topology_candidates_consumed &&
        plan->structure1a_structure3_topology_candidate_receipt.complete &&
        !plan->structure1a_structure3_topology_candidate_receipt
             .fallback_visuals_permitted;
    if (!receipt.material_plan_prepared || topology_candidate_index < 0 ||
        topology_candidate_index >= plan->structure1a_structure3_topology_candidate_receipt
            .topology_candidate_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.topology_candidate_bound = 1;
    receipt.target_built = nexus_v1_dgn_structure1a_structure3_capture_target_build(
        &engine->current_level, engine->current_level_dgn_data,
        engine->current_level_dgn_size, engine->game.current_level, 1,
        &plan->structure1a_structure3_topology_candidates[
            topology_candidate_index], structure3_entry_index,
        structure3_face_ordinal, out_target);
    if (!receipt.target_built) memset(out_target, 0, sizeof(*out_target));
    *out_receipt = receipt;
    return receipt.target_built;
}

int nexus_v1_engine_write_structure1a_structure3_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *path,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt target;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!path || !path[0] ||
        !nexus_v1_engine_build_structure1a_structure3_capture_target(
            engine, topology_candidate_index, structure3_entry_index,
            structure3_face_ordinal, &target, &receipt)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.target_written =
        nexus_v1_dgn_structure1a_structure3_capture_target_write(path, &target);
    *out_receipt = receipt;
    return receipt.target_written;
}

static uint8_t *nexus_read_host_file(const char *path, int *out_size) {
    uint8_t *buf = NULL;
    FILE *fp;
    long fsize;
    size_t got;
    if (!path || !path[0]) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    fsize = ftell(fp);
    if (fsize <= 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    buf = (uint8_t *)malloc((size_t)fsize);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    got = fread(buf, 1, (size_t)fsize, fp);
    fclose(fp);
    if (got != (size_t)fsize) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = (int)fsize;
    return buf;
}

static uint8_t *nexus_v1_read_extracted_file(Nexus_V1_Engine *engine,
                                             const char *name,
                                             int *out_size) {
    char path[512];
    int i;
    int known_name = 0;
    int exact_path_verified = 0;
    if (!engine || !name) return NULL;
    snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
    if (nexus_path_is_file(path)) {
        /* A canonical filename is only a lookup key.  It must not override
         * the retail hash contract when a user-supplied file has been
         * replaced or fabricated in-place. */
        for (i = 0; g_nexus_known_boot_files[i].name; ++i) {
            if (strcasecmp(g_nexus_known_boot_files[i].name, name) != 0) {
                continue;
            }
            known_name = 1;
            if (asset_file_matches_md5(path, g_nexus_known_boot_files[i].md5)) {
                exact_path_verified = 1;
                break;
            }
        }
        if (!known_name || exact_path_verified) {
            return nexus_read_host_file(path, out_size);
        }
    }
    /* Keep hash discovery aligned with the source receipt. Several retail
     * files, notably MENU.BPK, have verified revision hashes; using only the
     * first table entry would authenticate an alternate revision and then
     * fail to materialize it when the source entry is renamed. */
    for (i = 0; g_nexus_known_boot_files[i].name; ++i) {
        if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0 &&
            asset_find_by_md5(engine->data_dir, g_nexus_known_boot_files[i].md5,
                              path, (int)sizeof(path), 8)) {
            return nexus_read_host_file(path, out_size);
        }
    }
    if (nexus_v1_find_dmdf_family_file(engine->data_dir, name, path,
                                       (int)sizeof(path))) {
        return nexus_read_host_file(path, out_size);
    }
    return NULL;
}

static uint8_t *nexus_v1_read_iso_reader_file(
    Nexus_ISOReader *reader, const Nexus_ISOFile *file, int *out_size) {
    uint8_t *buf;
    int n;
    if (!reader || !file || file->size == 0U) return NULL;
    buf = (uint8_t *)malloc(file->size);
    if (!buf) return NULL;
    n = nexus_iso_read_file(reader, file, buf, (int)file->size);
    if (n < 0) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = (int)file->size;
    return buf;
}

static uint8_t *nexus_v1_read_iso_file(Nexus_V1_Engine *engine,
                                       const Nexus_ISOFile *file,
                                       int *out_size);
static int find_iso(const char *dir, char *disc_path, int max_len);

/* DGN material containers are deliberately narrower than the general asset
 * resolver. They must be an exact source entry named FLOORS.BPK or WALLS.BPK:
 * no hash fallback, DMDF-family scan, MENU.BPK, or opaque archive payload can
 * stand in while canonical retail hashes remain unknown. */
static uint8_t *nexus_v1_read_exact_material_container(
    Nexus_V1_Engine *engine, const char *name, int *out_size) {
    char path[512];
    const Nexus_ISOFile *file;
    if (!engine || !name) return NULL;
    if (engine->source == NEXUS_SRC_EXTRACTED) {
        snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
        if (nexus_path_is_file(path)) {
            return nexus_read_host_file(path, out_size);
        }
        if (engine->supplemental_iso_valid) {
            file = nexus_iso_find(&engine->supplemental_iso, name);
            return nexus_v1_read_iso_reader_file(
                &engine->supplemental_iso, file, out_size);
        }
        return NULL;
    }
    if (engine->source != NEXUS_SRC_ISO) return NULL;
    file = nexus_iso_find(&engine->iso, name);
    return nexus_v1_read_iso_file(engine, file, out_size);
}

static void nexus_v1_inspect_dgn_material_container(
    Nexus_V1_Engine *engine,
    const char *name,
    Nexus_V1_DgnMaterialCategory category,
    Nexus_V1_DgnMaterialContainerReceipt *out_receipt) {
    int size = 0;
    uint8_t *data;

    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->category = category;
    out_receipt->blocks_real_surface_render = 1;
    if (!engine || !name) return;

    data = nexus_v1_read_exact_material_container(engine, name, &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    out_receipt->exact_name_observed = 1;
    out_receipt->source_present = 1;
    out_receipt->format_valid = nexus_v1_bpk_archive_parse(
        data, (size_t)size, &out_receipt->archive) == 0;
    free(data);

    /* No canonical FLOORS/WALLS container hash is known from the verified
     * retail Track 1 listing. A well-formed, named file is useful evidence,
     * but it cannot be consumed or promote material until that identity gate
     * exists. */
    out_receipt->identity_verified = 0;
    out_receipt->host_route_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;
}

static int nexus_v1_iso_file_has_dmdf_magic(Nexus_V1_Engine *engine,
                                            const Nexus_ISOFile *file) {
    unsigned char magic[4];
    if (!engine || !file || file->size < sizeof(magic)) return 0;
    if (nexus_iso_read_file_chunk(&engine->iso, file, 0, magic,
                                  (int)sizeof(magic)) != (int)sizeof(magic)) {
        return 0;
    }
    return magic[0] == 'D' && magic[1] == 'M' &&
           magic[2] == 'D' && magic[3] == 'F';
}

static const Nexus_ISOFile *nexus_v1_find_iso_dmdf_family_file(
    Nexus_V1_Engine *engine, const char *name) {
    const char *ext = NULL;
    int i;
    if (!engine || !name) return NULL;
    if (nexus_path_has_ext(name, ".MNS")) {
        ext = ".MNS";
    } else if (nexus_path_has_ext(name, ".DMDF")) {
        ext = ".DMDF";
    } else {
        return NULL;
    }
    for (i = 0; i < engine->iso.file_count; ++i) {
        const Nexus_ISOFile *candidate = &engine->iso.files[i];
        if (!candidate->is_dir &&
            nexus_path_has_ext(candidate->name, ext) &&
            nexus_v1_iso_file_has_dmdf_magic(engine, candidate)) {
            return candidate;
        }
    }
    return NULL;
}

static uint8_t *nexus_v1_read_iso_file(Nexus_V1_Engine *engine,
                                       const Nexus_ISOFile *file,
                                       int *out_size) {
    return engine ? nexus_v1_read_iso_reader_file(&engine->iso, file,
                                                   out_size) : NULL;
}

static void nexus_v1_try_open_supplemental_iso(Nexus_V1_Engine *engine) {
    char disc_path[512];
    int n;
    if (!engine || engine->source != NEXUS_SRC_EXTRACTED ||
        !find_iso(engine->data_dir, disc_path, sizeof(disc_path))) {
        return;
    }
    if (nexus_path_has_ext(disc_path, ".cue")) {
        n = nexus_iso_open_cue(&engine->supplemental_iso, disc_path);
    } else {
        n = nexus_iso_open(&engine->supplemental_iso, disc_path);
    }
    if (n > 0 && nexus_iso_is_nexus(&engine->supplemental_iso)) {
        engine->supplemental_iso_valid = 1;
        printf("Nexus: supplemental retail ISO %s with %d files\n",
               disc_path, n);
        return;
    }
    nexus_iso_close(&engine->supplemental_iso);
}

static int nexus_try_open_disc_path(Nexus_V1_Engine *engine, const char *path) {
    int n = -1;
    if (!engine || !path || !path[0]) return 0;
    if (nexus_path_has_ext(path, ".cue")) {
        n = nexus_iso_open_cue(&engine->iso, path);
    } else if (nexus_path_has_ext(path, ".bin") ||
               nexus_path_has_ext(path, ".iso")) {
        n = nexus_iso_open(&engine->iso, path);
    }
    if (n > 0 && nexus_iso_is_nexus(&engine->iso)) {
        engine->source = NEXUS_SRC_ISO;
        printf("Nexus: opened disc image %s with %d files\n", path, n);
        return 1;
    }
    nexus_iso_close(&engine->iso);
    return 0;
}

/* Try to find ISO/CUE/BIN in data directory — cross-platform */
#ifdef _WIN32
static int find_iso(const char *dir, char *disc_path, int max_len) {
    static const char* const patterns[] = {"*.cue", "*.bin", "*.iso", NULL};
    WIN32_FIND_DATAA fd;
    HANDLE h = INVALID_HANDLE_VALUE;
    char pattern[512];
    int i;
    for (i = 0; patterns[i] != NULL; ++i) {
        snprintf(pattern, sizeof(pattern), "%s\\%s", dir, patterns[i]);
        h = FindFirstFileA(pattern, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            snprintf(disc_path, max_len, "%s\\%s", dir, fd.cFileName);
            FindClose(h);
            return 1;
        }
    }
    return 0;
}
#else
static int find_iso(const char *dir, char *disc_path, int max_len) {
    static const char* const exts[] = {".cue", ".bin", ".iso", NULL};
    DIR *d = opendir(dir);
    struct dirent *ent;
    int ext_index;
    if (!d) return 0;
    for (ext_index = 0; exts[ext_index] != NULL; ++ext_index) {
        rewinddir(d);
        while ((ent = readdir(d)) != NULL) {
            int len = (int)strlen(ent->d_name);
            int ext_len = (int)strlen(exts[ext_index]);
            if (len > ext_len &&
                strcasecmp(ent->d_name + len - ext_len, exts[ext_index]) == 0) {
                snprintf(disc_path, max_len, "%s/%s", dir, ent->d_name);
                closedir(d);
                return 1;
            }
        }
    }
    closedir(d);
    return 0;
}
#endif

/* Check if extracted files exist */
static int has_extracted(const char *dir) {
    const char *dm_bin_md5 = nexus_known_boot_file_md5("DM.BIN");
    const char *lev00_md5 = nexus_known_boot_file_md5("LEV00.DGN");
    const char *required[] = {dm_bin_md5, lev00_md5, NULL};
    char paths[2][ASSET_PATH_MAX];
    int matched[2] = {0, 0};
    char direct_path[ASSET_PATH_MAX];
    if (!dir || !dm_bin_md5 || !lev00_md5) return 0;

    /* Fast path for the normal extracted layout. This also prevents a
     * co-located ISO or external archive from changing source selection. */
    snprintf(direct_path, sizeof(direct_path), "%s/DM.BIN", dir);
    if (asset_file_matches_md5(direct_path, dm_bin_md5)) {
        snprintf(direct_path, sizeof(direct_path), "%s/LEV00.DGN", dir);
        if (asset_file_matches_md5(direct_path, lev00_md5)) return 1;
    }

    /* Use the ordinary-file scan explicitly. A mixed root may also contain
     * an ISO whose virtual member would otherwise win traversal order. */
    (void)asset_find_all_files_by_md5_list(
        dir, required, paths, matched, 2, 8);
    return matched[0] && matched[1];
}

static void nexus_v1_load_startup_faces(Nexus_V1_Engine *engine) {
    int face_size = 0;
    Nexus_UI_FaceLayout face_layout;
    int i;
    uint8_t *face_data;
    if (!engine) return;
    engine->ui_faces_loaded = 0;
    engine->ui_faces_expected = 0;
    engine->ui_faces_fallback = 0;
    face_data = nexus_v1_read_file(engine, "FACE.BIN", &face_size);
    if (!face_data) return;
    (void)nexus_ui_face_layout_detect(face_data, face_size, &face_layout);

    /* DMWeb defines the canonical FACE.BIN records as 20 56x56 portraits:
     * each has a 64-entry BGR555 palette and a PRS3 pixel stream. The loader
     * retains those source pixels; presentation remains no-draw until VDP1
     * placement and command order are captured. */
    for (i = 0; i < engine->champions.champion_count; ++i) {
        const int portrait_index = engine->champions.champions[i].portrait_index;
        int load_result;
        if (portrait_index < 0 ||
            portrait_index >= NEXUS_FACE_BIN_PORTRAIT_COUNT) continue;
        if (face_layout.valid && portrait_index >= face_layout.entry_count)
            continue;
        engine->ui_faces_expected++;
        if (face_layout.valid && portrait_index < face_layout.entry_count) {
            Nexus_UI_FaceCompactRecordDescriptor descriptor;
            if (!nexus_ui_face_compact_record_descriptor(face_data, face_size,
                                                         portrait_index,
                                                         &descriptor)) {
                load_result = -1;
            } else {
            load_result = nexus_ui_load_face_record(&engine->ui,
                                                    face_data + descriptor.prefix_offset,
                                                    descriptor.prefix_size +
                                                        (int)descriptor.prs3_size,
                                                    portrait_index,
                                                    face_layout.portrait_w,
                                                    face_layout.portrait_h,
                                                    NULL);
            }
        } else {
            load_result = -1;
        }
        if (load_result > 0) {
            engine->ui_faces_loaded++;
        } else {
            engine->ui_faces_fallback++;
        }
    }
    free(face_data);
}

static void nexus_v1_load_startup_surface_file(Nexus_V1_Engine *engine,
                                               const char *name,
                                               int (*loader)(Nexus_UI_Manager *,
                                                             const uint8_t *,
                                                             int,
                                                             const uint32_t *)) {
    int size = 0;
    int load_result;
    uint8_t *data;

    if (!engine || !name || !loader) return;
    engine->ui_startup_surfaces_expected++;
    data = nexus_v1_read_file(engine, name, &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    load_result = loader(&engine->ui, data, size, NULL);
    if (load_result > 0) {
        engine->ui_startup_surfaces_loaded++;
    } else if (load_result == 0) {
        engine->ui_startup_surfaces_fallback++;
    }
    free(data);
}

/* LOGOBG.DG2 is a real startup layer, but its VDP2 ownership is not part of
 * the four-surface launch gate. Decode and retain it when present without
 * treating an absent optional layer as a synthetic fallback or a launch
 * failure. */
static void nexus_v1_load_optional_startup_surface_file(
    Nexus_V1_Engine *engine,
    const char *name,
    int (*loader)(Nexus_UI_Manager *, const uint8_t *, int,
                  const uint32_t *))
{
    int size = 0;
    uint8_t *data;

    if (!engine || !name || !loader) return;
    data = nexus_v1_read_file(engine, name, &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    (void)loader(&engine->ui, data, size, NULL);
    free(data);
}

static void nexus_v1_load_startup_surfaces(Nexus_V1_Engine *engine) {
    if (!engine) return;
    engine->ui_startup_surfaces_loaded = 0;
    engine->ui_startup_surfaces_expected = 0;
    engine->ui_startup_surfaces_fallback = 0;
    nexus_v1_load_startup_surface_file(engine, "TITLE.CG",
                                       nexus_ui_load_title);
    nexus_v1_load_startup_surface_file(engine, "WARNING.BIN",
                                       nexus_ui_load_warning);
    nexus_v1_load_startup_surface_file(engine, "GAMEOVER.BIN",
                                       nexus_ui_load_gameover);
    nexus_v1_load_startup_surface_file(engine, "STABG.BIN",
                                       nexus_ui_load_stabg);
    nexus_v1_load_optional_startup_surface_file(engine, "LOGOBG.DG2",
                                                nexus_ui_load_logobg);
}

static void nexus_v1_load_champion_panel_geometry(Nexus_V1_Engine *engine)
{
    uint8_t *data;
    int size = 0;

    if (!engine) return;
    memset(&engine->champion_panel_source, 0,
           sizeof(engine->champion_panel_source));
    memset(engine->champion_panel_stat_bars, 0,
           sizeof(engine->champion_panel_stat_bars));
    memset(engine->champion_panel_inv_slots, 0,
           sizeof(engine->champion_panel_inv_slots));
    memset(engine->champion_panel_equip_slots, 0,
           sizeof(engine->champion_panel_equip_slots));
    engine->champion_panel_geometry_bound = 0;
    if (nexus_v1_level_aux_source_receipt(
            engine, "DM.BIN", &engine->champion_panel_source) != 0 ||
        !engine->champion_panel_source.canonical_hash_verified) {
        return;
    }
    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data) return;
    if (nexus_v1_champion_panel_parse_dm_bin(
            data, (size_t)size,
            engine->champion_panel_stat_bars,
            engine->champion_panel_inv_slots,
            engine->champion_panel_equip_slots) == 0) {
        engine->champion_panel_geometry_bound = 1;
    }
    free(data);
}

static void nexus_v1_load_hud_geometry(Nexus_V1_Engine *engine)
{
    uint8_t *data;
    int size = 0;
    size_t layout_count = 0U;
    size_t hit_rect_count = 0U;

    if (!engine) return;
    memset(&engine->hud_geometry_source, 0,
           sizeof(engine->hud_geometry_source));
    memset(engine->hud_layout, 0, sizeof(engine->hud_layout));
    memset(engine->hud_hit_rects, 0, sizeof(engine->hud_hit_rects));
    engine->hud_layout_count = 0U;
    engine->hud_hit_rect_count = 0U;
    engine->hud_geometry_bound = 0;
    if (nexus_v1_level_aux_source_receipt(
            engine, "DM.BIN", &engine->hud_geometry_source) != 0 ||
        !engine->hud_geometry_source.canonical_hash_verified) {
        return;
    }
    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data) return;
    if (nexus_v1_hud_layout_parse_dm_bin(
            data, (size_t)size, engine->hud_layout,
            NEXUS_HUD_LAYOUT_ENTRY_COUNT, &layout_count) == 0 &&
        nexus_v1_hud_hit_rects_parse_dm_bin(
            data, (size_t)size, engine->hud_hit_rects,
            NEXUS_HIT_RECT_COUNT, &hit_rect_count) == 0 &&
        layout_count == NEXUS_HUD_LAYOUT_ENTRY_COUNT &&
        hit_rect_count == NEXUS_HIT_RECT_COUNT) {
        engine->hud_layout_count = layout_count;
        engine->hud_hit_rect_count = hit_rect_count;
        engine->hud_geometry_bound = 1;
    }
    free(data);
}

static void nexus_v1_load_menu_bpk_decode_receipt(Nexus_V1_Engine *engine) {
    int size = 0;
    int dm_bin_size = 0;
    uint8_t *data;
    uint8_t *dm_bin = NULL;

    if (!engine) return;
    engine->menu_bpk_decode_receipt_valid = 0;
    engine->menu_bpk_decode_receipt_attempted = 0;
    engine->menu_bpk_upload_receipt_valid = 0;
    engine->menu_bpk_upload_row_count = 0;
    engine->menu_bpk_package_fnv1a64 = 0U;
    engine->menu_bpk_no_draw_host_valid = 0;
    engine->menu_bpk_no_draw_host_route_epoch = 0U;
    engine->menu_bpk_no_draw_host_package_fnv1a64 = 0U;
    engine->menu_bpk_no_draw_host_entry_index = 0U;
    engine->menu_bpk_no_draw_host_payload_offset = 0U;
    engine->menu_bpk_no_draw_host_payload_size = 0U;
    engine->menu_bpk_no_draw_host_payload_fnv1a64 = 0U;
    memset(&engine->menu_bpk_no_draw_host_compression, 0,
           sizeof(engine->menu_bpk_no_draw_host_compression));
    memset(&engine->menu_bpk_decode_receipt, 0,
           sizeof(engine->menu_bpk_decode_receipt));
    engine->menu_bpk_prs3_execution_evidence_valid = 0;
    memset(&engine->menu_bpk_prs3_execution_evidence, 0,
           sizeof(engine->menu_bpk_prs3_execution_evidence));
    memset(&engine->menu_bpk_upload_receipt, 0,
           sizeof(engine->menu_bpk_upload_receipt));
    memset(engine->menu_bpk_upload_rows, 0,
           sizeof(engine->menu_bpk_upload_rows));
    (void)nexus_v1_level_aux_source_receipt(engine, "MENU.BPK",
                                             &engine->menu_bpk_source);
    /* The archive parser accepts generic BPPK shapes for diagnostics and
     * tests. The startup route must only consume the authenticated retail
     * MENU.BPK, never a same-named custom archive. */
    if (!engine->menu_bpk_source.canonical_hash_verified) {
        return;
    }

    (void)nexus_v1_level_aux_source_receipt(
        engine, "DM.BIN",
        &engine->menu_bpk_prs3_execution_evidence.dm_bin_source);
    engine->menu_bpk_prs3_execution_evidence.menu_bpk_source_hash_verified =
        1;

    data = nexus_v1_read_file(engine, "MENU.BPK", &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    engine->menu_bpk_package_fnv1a64 =
        nexus_v1_dgn_bytes_fnv1a64(data, size);
    if (!engine->menu_bpk_package_fnv1a64) {
        free(data);
        return;
    }

    engine->menu_bpk_decode_receipt_attempted = 1;
    if (nexus_v1_bpk_archive_runtime_decode_receipt(
            data,
            (size_t)size,
            &engine->menu_bpk_decode_receipt) == 0) {
        engine->menu_bpk_decode_receipt_valid = 1;
    }
    if (nexus_v1_bpk_archive_runtime_upload_plan(
            data,
            (size_t)size,
            engine->menu_bpk_upload_rows,
            NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS,
            &engine->menu_bpk_upload_receipt) == 0) {
        engine->menu_bpk_upload_receipt_valid = 1;
        engine->menu_bpk_upload_row_count =
            (engine->menu_bpk_upload_receipt.planned_rows >
             NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS)
                ? (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS
                : (int)engine->menu_bpk_upload_receipt.planned_rows;
    }

    /* The loader receipt comes from the same exact boot assets that the
     * decoder would consume. It proves only outer framing and static SH-2
     * instructions; neither fact describes PRS3 commands or pixels. */
    if (engine->menu_bpk_prs3_execution_evidence.dm_bin_source
            .canonical_hash_verified) {
        dm_bin = nexus_v1_read_file(engine, "DM.BIN", &dm_bin_size);
    }
    if (dm_bin && dm_bin_size > 0 &&
        nexus_v1_prs3_cross_asset_frame_receipt_verified(
            dm_bin, (size_t)dm_bin_size, 1, data, (size_t)size, 1,
            &engine->menu_bpk_prs3_execution_evidence.cross_asset) &&
        nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, (size_t)dm_bin_size, 1,
            &engine->menu_bpk_prs3_execution_evidence.sh2_loader)) {
        engine->menu_bpk_prs3_execution_evidence.cross_asset_framing_verified =
            1;
        engine->menu_bpk_prs3_execution_evidence.sh2_loader_route_verified =
            1;
        engine->menu_bpk_prs3_execution_evidence.valid = 1;
        engine->menu_bpk_prs3_execution_evidence_valid = 1;
    }
    /* No present evidence establishes opcode grammar, output bytes, palette,
     * or VDP1 presentation. Keep every decoder/render promotion fail-closed. */
    engine->menu_bpk_prs3_execution_evidence.decoder_promoted = 0;
    engine->menu_bpk_prs3_execution_evidence.runtime_decode_permitted = 0;
    engine->menu_bpk_prs3_execution_evidence.fallback_visuals_permitted = 0;
    free(dm_bin);
    free(data);
}

static void nexus_engine_script_handler(const Nexus_ScriptAction *action,
                                        void *user_data) {
    Nexus_V1_Engine *engine = (Nexus_V1_Engine *)user_data;
    if (!action || !engine) return;

    switch (action->opcode) {
    case NEXUS_OP_TELEPORT:
        engine->mechanics->party_x = action->x;
        engine->mechanics->party_y = action->y;
        if (action->level != engine->mechanics->map_index)
            engine->mechanics->pending_level_change = action->level;
        break;
    case NEXUS_OP_SPAWN:
        nexus_v1_creature_spawn_on_level(&engine->creatures,
            action->value, action->x, action->y, 0, action->level);
        break;
    case NEXUS_OP_SET_SQUARE:
        if (action->x >= 0 && action->x < NEXUS_MAX_MAP_SIZE &&
            action->y >= 0 && action->y < NEXUS_MAX_MAP_SIZE)
            engine->current_level.squares[action->y][action->x] = (uint8_t)action->value;
        break;
    case NEXUS_OP_SOUND:
        nexus_sound_play(&engine->audio, (Nexus_SoundEvent)action->value);
        break;
    case NEXUS_OP_TRIGGER_DOOR: {
        int di = nexus_v1_door_find(&engine->doors, action->x, action->y,
            engine->mechanics->map_index);
        if (di >= 0) nexus_v1_door_toggle(&engine->doors, di);
        break;
    }
    case NEXUS_OP_GIVE_ITEM: {
        int ci;
        for (ci = 0; ci < engine->champions.party_count; ci++) {
            int slot;
            Nexus_V1_Champion *ch = &engine->champions.champions[
                engine->champions.party[ci]];
            for (slot = 0; slot < NEXUS_INVENTORY_SLOTS; slot++) {
                if (ch->inventory[slot] == 0xFFU) {
                    ch->inventory[slot] = (uint8_t)action->item_id;
                    goto give_done;
                }
            }
        }
        give_done:
        break;
    }
    case NEXUS_OP_AWARD_XP: {
        int ci;
        for (ci = 0; ci < engine->champions.party_count; ci++) {
            int idx = engine->champions.party[ci];
            nexus_v1_experience_award_kill(&engine->experience,
                &engine->champions.champions[idx], idx, action->value);
        }
        break;
    }
    case NEXUS_OP_DISPLAY_MESSAGE: {
        /* Real SLEV action ownership and the DMN text-table lookup are not
         * source-authenticated yet. Never place a fabricated message ID in
         * the production HUD; the script runtime remains fail-closed until
         * its original consumer is recovered. */
        (void)action->message_id;
        break;
    }
    case NEXUS_OP_SET_FLAG:
        if (action->flag_index >= 0 && action->flag_index < NEXUS_SCRIPT_MAX_FLAGS)
            engine->script_vm.flags[action->flag_index] = (uint8_t)action->value;
        break;
    case NEXUS_OP_END_GAME:
        engine->mechanics->game_over = 1;
        engine->mechanics->game_over_reason = 1;
        nexus_v1_gameover_victory(&engine->gameover);
        break;
    default:
        break;
    }
}

int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir) {
    char disc_path[512];
    if (!engine || !data_dir) return -1;
    memset(engine, 0, sizeof(*engine));
    strncpy(engine->data_dir, data_dir, sizeof(engine->data_dir) - 1);

    /* Prefer a complete hash-verified extracted corpus when present. This
     * lets materialized retail files win over a parallel ISO entry whose
     * sector payload may be a different revision. ISO remains the fallback
     * for users who keep the disc image only. */
    if (has_extracted(data_dir)) {
        engine->source = NEXUS_SRC_EXTRACTED;
        nexus_v1_try_open_supplemental_iso(engine);
        printf("Nexus: using extracted files from %s\n", data_dir);
    }

    if (engine->source == NEXUS_SRC_NONE && nexus_path_is_file(data_dir)) {
        (void)nexus_try_open_disc_path(engine, data_dir);
    } else if (engine->source == NEXUS_SRC_NONE &&
               find_iso(data_dir, disc_path, sizeof(disc_path))) {
        (void)nexus_try_open_disc_path(engine, disc_path);
    }

    if (engine->source == NEXUS_SRC_NONE) {
        printf("Nexus: no game data found in %s\n", data_dir);
        return -1;
    }

    /* Init game state */
    nexus_v1_game_init(&engine->game, data_dir);
    engine->audio_enabled = 1;
    nexus_sound_set_data_root(&engine->audio, data_dir);

    /* DGN Structure1B references resolve through the retail environment
     * resources, not guessed FLOORS/WALLS BPK names.  SN_FLOOR.MNS and
     * SN_WALL.MNS are named Track 1 DMDF files whose top-level TEXT sections
     * carry the original BGR555 material descriptors. MENU.BPK remains a
     * separate PRS3-gated menu route and is never substituted here. */
    {
        int material_size = 0;
        uint8_t *material_data;
        memset(&engine->dgn_static_material_sources, 0,
               sizeof(engine->dgn_static_material_sources));
        engine->dgn_static_material_sources.fallback_visuals_permitted = 0;
        (void)nexus_v1_level_aux_source_receipt(
            engine, "SN_FLOOR.MNS",
            &engine->dgn_static_material_sources.floor_mns);
        (void)nexus_v1_level_aux_source_receipt(
            engine, "SN_WALL.MNS",
            &engine->dgn_static_material_sources.wall_mns);
        engine->dgn_static_material_sources.canonical_pair_bound =
            engine->dgn_static_material_sources.floor_mns.canonical_hash_verified &&
            engine->dgn_static_material_sources.wall_mns.canonical_hash_verified;

        material_data = nexus_v1_read_file(engine, "SN_FLOOR.MNS",
                                            &material_size);
        if (material_data &&
            engine->dgn_static_material_sources.floor_mns
                .canonical_hash_verified) {
            engine->floor_mns_material_route_valid =
                nexus_v1_dmdf_decode_text_material_bank(material_data,
                                                         material_size,
                                                         &engine->floor_materials);
        }
        free(material_data);
        nexus_v1_inspect_dgn_material_container(
            engine, "FLOORS.BPK", NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR,
            &engine->floor_bpk_container);
        material_data = nexus_v1_read_file(engine, "SN_WALL.MNS",
                                            &material_size);
        if (material_data &&
            engine->dgn_static_material_sources.wall_mns
                .canonical_hash_verified) {
            engine->wall_mns_material_route_valid =
                nexus_v1_dmdf_decode_text_material_bank(material_data,
                                                         material_size,
                                                         &engine->wall_materials);
        }
        free(material_data);
        nexus_v1_inspect_dgn_material_container(
            engine, "WALLS.BPK", NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL,
            &engine->wall_bpk_container);
    }

    /* Init champion pool from DMWeb's real RLOWFIX.BIN/PLRD records.  The
     * old hardcoded roster remains fixture-only; production fails closed if
     * the retail descriptor resource is absent or malformed. */
    {
        int rlowfix_size = 0;
        uint8_t *rlowfix = nexus_v1_read_file(engine, "RLOWFIX.BIN",
                                              &rlowfix_size);
        int champions_bound = 0;
        if (rlowfix) {
            champions_bound = nexus_v1_champions_init_from_rlowfix(
                &engine->champions, rlowfix, (size_t)rlowfix_size);
            /* DMWeb DecodeRES/DecodeTEXT offsets for the authenticated
             * European RLOWFIX corpus. Keep these as source receipts; the
             * Saturn TEXT/FONT256 consumer remains capture-gated. */
            engine->startup_menu_text_source_bound =
                nexus_v1_rlowfix_text_parse(
                    rlowfix, (size_t)rlowfix_size, UINT32_C(0xF270),
                    &engine->startup_menu_text_source) &&
                nexus_v1_rlowfix_tabl_parse(
                    rlowfix, (size_t)rlowfix_size, UINT32_C(0x1232C),
                    &engine->startup_menu_tabl_source);
            {
                const uint32_t font_offsets[3] = {
                    UINT32_C(0xC0), UINT32_C(0x1C2C), UINT32_C(0x3F78)
                };
                const uint32_t font_counts[3] = { 291U, 250U, 710U };
                const uint32_t font_widths[3] = { 6U, 12U, 12U };
                Nexus_V1_ResDecodeResult res;
                int font_index;
                engine->startup_menu_font012_bound =
                    nexus_v1_res_decode(rlowfix, rlowfix_size, &res);
                for (font_index = 0; font_index < 3 &&
                     engine->startup_menu_font012_bound; ++font_index) {
                    const Nexus_V1_ResEntry *entry =
                        nexus_v1_res_find(&res, "FONT", font_index);
                    if (!entry || entry->offset != font_offsets[font_index] ||
                        !nexus_v1_font012_parse(
                            rlowfix + entry->offset, entry->size,
                            (uint32_t)font_index,
                            &engine->startup_menu_font012[font_index]) ||
                        engine->startup_menu_font012[font_index].character_count !=
                            font_counts[font_index] ||
                        engine->startup_menu_font012[font_index].character_width !=
                            font_widths[font_index] ||
                        engine->startup_menu_font012[font_index].character_height !=
                            12U) {
                        engine->startup_menu_font012_bound = 0;
                    }
                }
            }
        }
        if (!champions_bound) {
            memset(&engine->champions, 0, sizeof(engine->champions));
            engine->champions.leader_index = -1;
        }
        free(rlowfix);
    }

    /* Init startup UI surfaces after source selection and champion roster. */
    nexus_ui_manager_init(&engine->ui);
    nexus_v1_load_startup_surfaces(engine);
    nexus_v1_load_startup_faces(engine);
    nexus_v1_load_champion_panel_geometry(engine);
    nexus_v1_load_hud_geometry(engine);
    nexus_v1_load_menu_bpk_decode_receipt(engine);

    /* Init creature manager and load real stats from the source-bound
     * RLOWFIX.BIN CRET member.  Use nexus_v1_read_file() so an ISO-only root
     * receives the same authenticated virtual member as PLRD/champions. */
    nexus_v1_creatures_init_production(&engine->creatures);
    {
        int rlowfix_size = 0;
        uint8_t *rlowfix = nexus_v1_read_file(engine, "RLOWFIX.BIN",
                                              &rlowfix_size);
        if (rlowfix) {
            nexus_v1_creatures_load_cret_bytes(
                &engine->creatures, rlowfix, (size_t)rlowfix_size);
        }
        free(rlowfix);
    }

    /* Init projectile manager */
    nexus_v1_projectiles_init(&engine->projectiles);

    /* Init automap */
    nexus_v1_automap_init(&engine->automap);

    /* Init light */
    nexus_v1_light_init(&engine->light);

    /* Init status effects and experience */
    {
        int si;
        for (si = 0; si < 4; si++) {
            nexus_v1_status_init(&engine->champion_status[si]);
            nexus_v1_xp_init(&engine->champion_xp[si]);
        }
    }

    /* Init triggers */
    nexus_v1_triggers_init(&engine->triggers);

    /* Init rest */
    nexus_v1_rest_init(&engine->rest);
    nexus_v1_messages_init(&engine->messages);
    nexus_v1_throw_init(&engine->thrown);
    nexus_v1_hunger_init(&engine->hunger);
    nexus_v1_action_timers_init(&engine->action_timers);
    nexus_v1_spawners_init(&engine->spawners);
    nexus_v1_damage_display_init(&engine->damage_display);
    nexus_v1_formation_init(&engine->formation, engine->champions.party_count);
    nexus_v1_door_manager_init(&engine->doors);
    nexus_v1_gameover_init(&engine->gameover);
    nexus_v1_npc_manager_init(&engine->npcs);
    nexus_v1_shop_manager_init(&engine->shops);
    nexus_v1_load_shop_catalog(engine);
    nexus_v1_trap_manager_init(&engine->traps);
    nexus_v1_transition_table_init(&engine->transitions);
    nexus_v1_experience_init(&engine->experience);
    nexus_v1_fountain_manager_init(&engine->fountains);
    nexus_v1_switch_manager_init(&engine->switches);
    nexus_v1_container_manager_init(&engine->containers);
    {
        char save_dir[512];
        nexus_v1_save_default_dir(save_dir, sizeof(save_dir));
        nexus_v1_save_init(&engine->save_manager, save_dir);
    }
    engine->viewport = calloc(1, sizeof(Nexus_Viewport));
    if (engine->viewport)
        nexus_viewport_init(engine->viewport);
    /* Init sound engine */
    nexus_sound_init(&engine->audio);
    (void)nexus_v1_level_aux_source_receipt(
        engine, "SDDRVS.TSK", &engine->sound_driver_source);
    nexus_sound_set_driver_canonical_source_verified(
        &engine->audio,
        engine->sound_driver_source.canonical_hash_verified);
    (void)nexus_sound_level_runtime_receipt(&engine->audio,
                                            &engine->sfx_runtime_receipt);
    nexus_script_vm_init(&engine->script_vm);
    nexus_script_vm_set_handler(&engine->script_vm,
                                nexus_engine_script_handler, engine);
    (void)nexus_script_vm_runtime_receipt(&engine->script_vm,
                                          &engine->script_runtime_receipt);

    /* Load font */
    {
        int font_size = 0;
        uint8_t *font_data = nexus_v1_read_file(engine, "FONT256.S2D", &font_size);
        if (font_data) {
            Nexus_V1_FontS2dDecodeResult font_regions;
            if (nexus_v1_font_s2d_decode(font_data, font_size, &font_regions) == 1) {
                if (nexus_v1_font_load_from_s2d(&engine->font, font_data,
                                                 font_size, &font_regions) > 0) {
                    /* The CG tiles are real source bytes, but the Saturn
                     * page/attribute character-code relation is not proven.
                     * Retain the diagnostic payload without advertising a
                     * drawable runtime font. */
                    engine->font_loaded = 0;
                    printf("Nexus font: FONT256.S2D retained (%d CG tiles, "
                           "glyph mapping pending)\n", engine->font.char_count);
                } else {
                    printf("Nexus font: FONT256.S2D regions admitted; glyph mapping pending\n");
                    /* Region admission is not a drawable font. Keep the
                     * runtime flag clear until the real glyph mapping and
                     * payload handoff are source-bound. */
                    engine->font_loaded = 0;
                }
            }
            free(font_data);
        }
    }

    /* Allocate and init mechanics state (opaque pointer).
     * mechanics owns its memory; freed in nexus_v1_shutdown().
     * Source: DM1 CLIKMENU.C F0366. */
    engine->mechanics = (Nexus_MechanicsState *)calloc(1, sizeof(Nexus_MechanicsState));
    if (engine->mechanics) {
        nexus_mechanics_init(engine->mechanics,
            engine->game.party_x, engine->game.party_y,
            engine->game.party_dir);
    }

    engine->initialized = 1;
    printf("Nexus V1 engine initialized (source: %s)\n",
        engine->source == NEXUS_SRC_ISO ? "ISO" : "extracted");
    return 0;
}

uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size) {
    uint8_t *buf = NULL;
    if (!engine || !name) return NULL;

    if (engine->source == NEXUS_SRC_ISO) {
        const Nexus_ISOFile *f = nexus_iso_find(&engine->iso, name);
        if (!f) {
            f = nexus_v1_find_iso_dmdf_family_file(engine, name);
        }
        buf = nexus_v1_read_iso_file(engine, f, out_size);
    } else if (engine->source == NEXUS_SRC_EXTRACTED) {
        buf = nexus_v1_read_extracted_file(engine, name, out_size);
        if (!buf && engine->supplemental_iso_valid) {
            const Nexus_ISOFile *f = nexus_iso_find(
                &engine->supplemental_iso, name);
            if (f) {
                buf = nexus_v1_read_iso_reader_file(
                    &engine->supplemental_iso, f, out_size);
            }
        }
    }
    return buf;
}

int nexus_v1_engine_dm_bin_vdp1_register_table_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1RegisterTableReceipt *out_receipt)
{
    /* This exact big-endian word sequence is present once in the canonical
     * retail executable at DM.BIN+0x77114. It is deliberately retained as a
     * static capture anchor, not decoded as SH-2 control flow. */
    static const uint32_t expected_words[8] = {
        UINT32_C(0x060967fc), UINT32_C(0x060972bc), UINT32_C(0x25d00000),
        UINT32_C(0x06097428), UINT32_C(0x060972c0), UINT32_C(0x25f80004),
        UINT32_C(0x0609747c), UINT32_C(0x25d00010)
    };
    Nexus_V1_DmBinVdp1RegisterTableReceipt receipt;
    uint8_t *data;
    int size = 0;
    int offset;
    int word;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.table_offset = -1;
    receipt.no_draw_only = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine ||
        nexus_v1_level_aux_source_receipt(engine, "DM.BIN", &receipt.source) != 0 ||
        !receipt.source.canonical_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }

    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data || size < (int)(sizeof(expected_words))) {
        free(data);
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = size;
    receipt.source_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(data, size);
    for (offset = 0; offset <= size - (int)sizeof(expected_words); offset += 4) {
        for (word = 0; word < (int)(sizeof(expected_words) / sizeof(expected_words[0]));
             ++word) {
            if (nexus_v1_dgn_read_be32(data + offset + word * 4) !=
                expected_words[word]) {
                break;
            }
        }
        if (word == (int)(sizeof(expected_words) / sizeof(expected_words[0]))) {
            if (receipt.table_offset < 0) receipt.table_offset = offset;
            ++receipt.table_occurrence_count;
        }
    }
    free(data);
    receipt.vdp1_register_base_0x25d00000_observed =
        receipt.table_occurrence_count > 0;
    receipt.vdp1_register_offset_0x10_observed =
        receipt.table_occurrence_count > 0;
    receipt.static_vdp1_register_table_proven =
        receipt.table_occurrence_count == 1;
    *out_receipt = receipt;
    return receipt.static_vdp1_register_table_proven ? 1 : 0;
}

int nexus_v1_engine_dm_bin_vdp1_state_route_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1StateRouteReceipt *out_receipt)
{
    /* The static map is retained verbatim from the canonical Japanese retail
     * executable. It has one VDP1-VRAM base and six VDP1 register literals.
     * We only decode the documented SH-2 MOV.L @(disp,PC),Rn addressing needed
     * to prove that executable code references the map; no opcode is run. */
    static const uint32_t expected_words[19] = {
        UINT32_C(0x25d00002), UINT32_C(0x25d00004), UINT32_C(0x00008000),
        UINT32_C(0x25d00006), UINT32_C(0x25d00008), UINT32_C(0x0000ffff),
        UINT32_C(0x25d0000a), UINT32_C(0x060972ba), UINT32_C(0x060972b6),
        UINT32_C(0x25c00000), UINT32_C(0x0609747c), UINT32_C(0x060972bc),
        UINT32_C(0x25d00000), UINT32_C(0x06097490), UINT32_C(0x060972b4),
        UINT32_C(0x060972b8), UINT32_C(0x0609748e), UINT32_C(0x060972c0),
        UINT32_C(0x060972c2)
    };
    Nexus_V1_DmBinVdp1StateRouteReceipt receipt;
    uint8_t *data;
    int size = 0;
    int offset;
    int word;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.table_offset = -1;
    receipt.first_sh2_literal_load_offset = -1;
    receipt.last_sh2_literal_load_offset = -1;
    receipt.no_draw_only = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine ||
        nexus_v1_level_aux_source_receipt(engine, "DM.BIN", &receipt.source) != 0 ||
        !receipt.source.canonical_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }

    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data || size < (int)sizeof(expected_words)) {
        free(data);
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = size;
    receipt.source_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(data, size);
    for (offset = 0; offset <= size - (int)sizeof(expected_words); offset += 4) {
        for (word = 0; word < (int)(sizeof(expected_words) / sizeof(expected_words[0]));
             ++word) {
            if (nexus_v1_dgn_read_be32(data + offset + word * 4) !=
                expected_words[word]) {
                break;
            }
        }
        if (word == (int)(sizeof(expected_words) / sizeof(expected_words[0]))) {
            if (receipt.table_offset < 0) receipt.table_offset = offset;
            ++receipt.table_occurrence_count;
        }
    }
    if (receipt.table_occurrence_count != 1) {
        free(data);
        *out_receipt = receipt;
        return 0;
    }
    /* SH-2 MOV.L @(disp,PC),Rn is Dnxx. Its word-aligned target is
     * (PC + 4) rounded down to four bytes plus disp * 4. */
    for (offset = 0; offset + 1 < size; offset += 2) {
        uint16_t opcode = nexus_v1_dgn_read_be16(data + offset);
        int target;
        uint32_t literal;

        if ((opcode & UINT16_C(0xf000)) != UINT16_C(0xd000)) continue;
        target = ((offset + 4) & ~3) + (int)((opcode & UINT16_C(0x00ff)) * 4U);
        if (target < receipt.table_offset || target + 4 > receipt.table_offset +
                (int)sizeof(expected_words)) {
            continue;
        }
        literal = nexus_v1_dgn_read_be32(data + target);
        ++receipt.sh2_pc_relative_literal_load_count;
        if (receipt.first_sh2_literal_load_offset < 0)
            receipt.first_sh2_literal_load_offset = offset;
        receipt.last_sh2_literal_load_offset = offset;
        if ((literal & UINT32_C(0xfffffff0)) == UINT32_C(0x25d00000))
            ++receipt.vdp1_register_literal_load_count;
        if (literal == UINT32_C(0x25c00000))
            ++receipt.vdp1_vram_literal_load_count;
    }
    free(data);
    receipt.static_sh2_literal_loads_proven =
        receipt.sh2_pc_relative_literal_load_count == 21 &&
        receipt.vdp1_register_literal_load_count == 6 &&
        receipt.vdp1_vram_literal_load_count == 1;
    receipt.vdp1_vram_command_storage_candidate_proven =
        receipt.static_sh2_literal_loads_proven;
    *out_receipt = receipt;
    return receipt.static_sh2_literal_loads_proven ? 1 : 0;
}

int nexus_v1_engine_dm_bin_vdp1_state_write_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1StateWriteReceipt *out_receipt)
{
    enum {
        CONTROL_WINDOW_OFFSET = 0x7d3b6,
        CODE_WINDOW_OFFSET = 0x7d3c0,
        STATE_TABLE_OFFSET = 0x7d498
    };
    /* The sequence below keeps each source literal and its consuming SH-2
     * MOV.W visible. It is not an emulator: the verifier accepts only this
     * exact static dataflow corridor in the hash-bound original executable. */
    static const uint16_t expected_code_words[12] = {
        UINT16_C(0xdd3a), UINT16_C(0x2321), UINT16_C(0x2011),
        UINT16_C(0xe300), UINT16_C(0xd235), UINT16_C(0xd136),
        UINT16_C(0xd039), UINT16_C(0x2121), UINT16_C(0xd235),
        UINT16_C(0x2231), UINT16_C(0xd336), UINT16_C(0x23d1)
    };
    static const uint16_t expected_control_words[8] = {
        UINT16_C(0xe102), UINT16_C(0xd038), UINT16_C(0xe600),
        UINT16_C(0xd438), UINT16_C(0xd336), UINT16_C(0xdd3a),
        UINT16_C(0x2321), UINT16_C(0x2011)
    };
    static const uint16_t expected_vram_handoff_words[2] = {
        UINT16_C(0xd234), UINT16_C(0x2e22)
    };
    Nexus_V1_DmBinVdp1StateWriteReceipt receipt;
    uint8_t *data;
    int size = 0;
    int word;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.code_window_offset = CODE_WINDOW_OFFSET;
    receipt.state_table_offset = STATE_TABLE_OFFSET;
    receipt.vdp1_vram_base_literal_offset = STATE_TABLE_OFFSET + 36;
    receipt.vdp1_vram_base_load_offset = 0x7d3e8;
    receipt.vdp1_vram_base_r14_store_offset = 0x7d3ea;
    receipt.no_draw_only = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine ||
        nexus_v1_level_aux_source_receipt(engine, "DM.BIN", &receipt.source) != 0 ||
        !receipt.source.canonical_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }
    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data || CODE_WINDOW_OFFSET + (int)sizeof(expected_code_words) > size ||
        CONTROL_WINDOW_OFFSET + (int)sizeof(expected_control_words) > size ||
        receipt.vdp1_vram_base_r14_store_offset +
            (int)sizeof(expected_vram_handoff_words) > size ||
        receipt.vdp1_vram_base_literal_offset + 4 > size ||
        STATE_TABLE_OFFSET + 28 > size) {
        free(data);
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = size;
    receipt.source_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(data, size);
    for (word = 0; word < (int)(sizeof(expected_code_words) / sizeof(expected_code_words[0]));
         ++word) {
        if (nexus_v1_dgn_read_be16(data + CODE_WINDOW_OFFSET + word * 2) !=
            expected_code_words[word]) {
            free(data);
            *out_receipt = receipt;
            return 0;
        }
    }
    for (word = 0;
         word < (int)(sizeof(expected_control_words) / sizeof(expected_control_words[0]));
         ++word) {
        if (nexus_v1_dgn_read_be16(data + CONTROL_WINDOW_OFFSET + word * 2) !=
            expected_control_words[word]) {
            free(data);
            *out_receipt = receipt;
            return 0;
        }
    }
    for (word = 0;
         word < (int)(sizeof(expected_vram_handoff_words) /
                      sizeof(expected_vram_handoff_words[0]));
         ++word) {
        if (nexus_v1_dgn_read_be16(data + receipt.vdp1_vram_base_load_offset +
                                   word * 2) != expected_vram_handoff_words[word]) {
            free(data);
            *out_receipt = receipt;
            return 0;
        }
    }
    /* PC-relative source literals for D235, D136, D235, D336 and DD3A.
     * The immediately following MOV.W opcodes encode Rm -> @Rn, so the
     * observed dataflow is: 0x8000 -> 0x25d00006, zero -> 0x25d00008,
     * and 0xffff -> 0x25d0000a. */
    /* MOV #2,R1; MOV.L literal,R0; ...; MOV.W R1,@R0 is the original
     * static +0x04 control-register write. It is capture evidence only: an
     * execution trace is still needed before claiming command emission. */
    receipt.vdp1_register_0x04_write_proven =
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 4) ==
            UINT32_C(0x25d00004);
    receipt.vdp1_register_0x04_value = 2U;
    receipt.vdp1_command_control_candidate_proven =
        receipt.vdp1_register_0x04_write_proven;
    /* The literal load and immediate MOV.L R2,@R14 prove a bounded memory
     * handoff only. They do not identify R14 or a command-list address. */
    receipt.vdp1_vram_base_r14_store_proven =
        nexus_v1_dgn_read_be32(data + receipt.vdp1_vram_base_literal_offset) ==
            UINT32_C(0x25c00000);
    receipt.vdp1_register_0x06_write_proven =
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 8) ==
            UINT32_C(0x00008000) &&
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 12) ==
            UINT32_C(0x25d00006);
    receipt.vdp1_register_0x06_value = UINT16_C(0x8000);
    receipt.vdp1_register_0x08_write_proven =
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 16) ==
            UINT32_C(0x25d00008);
    receipt.vdp1_register_0x08_value = 0U;
    receipt.vdp1_register_0x0a_write_proven =
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 20) ==
            UINT32_C(0x0000ffff) &&
        nexus_v1_dgn_read_be32(data + STATE_TABLE_OFFSET + 24) ==
            UINT32_C(0x25d0000a);
    receipt.vdp1_register_0x0a_value = UINT16_C(0xffff);
    receipt.static_instruction_dataflow_proven =
        receipt.vdp1_register_0x04_write_proven &&
        receipt.vdp1_register_0x06_write_proven &&
        receipt.vdp1_register_0x08_write_proven &&
        receipt.vdp1_register_0x0a_write_proven;
    free(data);
    *out_receipt = receipt;
    return receipt.static_instruction_dataflow_proven ? 1 : 0;
}

static void nexus_v1_load_item_ibs_runtime_source(Nexus_V1_Engine *engine)
{
    uint8_t *data;
    int size = 0;

    if (!engine) return;
    nexus_itemdef_clear_ibs_declarations();
    memset(&engine->item_ibs_bank, 0, sizeof(engine->item_ibs_bank));
    memset(&engine->item_ibs_runtime_source, 0,
           sizeof(engine->item_ibs_runtime_source));
    engine->item_ibs_runtime_source.fallback_visuals_permitted = 0;
    (void)nexus_v1_level_aux_source_receipt(
        engine, "ITEM.IBS", &engine->item_ibs_runtime_source.source);
    if (!engine->item_ibs_runtime_source.source.canonical_hash_verified) {
        return;
    }
    data = nexus_v1_read_file(engine, "ITEM.IBS", &size);
    if (!data) return;
    engine->item_ibs_runtime_source.parsed_bank_valid =
        nexus_v1_item_ibs_parse_verified(
            data, size, 1, &engine->item_ibs_bank) == 0;
    free(data);
    engine->item_ibs_runtime_source.source_bound =
        engine->item_ibs_runtime_source.parsed_bank_valid &&
        engine->item_ibs_runtime_source.source.canonical_hash_verified;
    if (engine->item_ibs_runtime_source.source_bound) {
        nexus_itemdef_bind_ibs_bank(
            &engine->item_ibs_bank,
            NEXUS_V1_ITEM_IBS_DECLARATION_COUNT);
    }
}

static void nexus_v1_load_shop_catalog(Nexus_V1_Engine *engine)
{
    Nexus_V1_LevelAuxSourceReceipt source;
    uint8_t *data;
    int size = 0;

    if (!engine) return;
    engine->shops.catalog_count = 0;
    engine->shops.catalog_source_bound = 0;
    memset(engine->shops.catalog, 0, sizeof(engine->shops.catalog));
    memset(&source, 0, sizeof(source));
    if (nexus_v1_level_aux_source_receipt(engine, "DM.BIN", &source) != 0 ||
        !source.canonical_hash_verified) {
        return;
    }
    data = nexus_v1_read_file(engine, "DM.BIN", &size);
    if (!data) return;
    (void)nexus_v1_shop_bind_dm_bin(&engine->shops, data, (size_t)size, 1);
    free(data);
}

static void nexus_v1_clear_structure3_runtime_source(Nexus_V1_Engine *engine)
{
    if (!engine) return;
    free(engine->structure3_runtime_source.texture_span);
    free(engine->structure3_runtime_source.palette_state);
    free(engine->structure3_runtime_source.vdp1_state);
    free(engine->structure3_runtime_source.transform_state);
    free(engine->structure3_runtime_source.normal_culling_state);
    free(engine->structure3_runtime_source.vdp1_command);
    memset(&engine->structure3_runtime_source, 0,
           sizeof(engine->structure3_runtime_source));
    engine->structure3_runtime_source.blocks_real_dgn_mesh_render = 1;
}

/* Keep the engine boundary independent of the launcher/importer receipt.
 * This repeats the capture reader's six length-prefixed FNV lanes before the
 * raw bytes enter engine-owned storage. It verifies transport identity only;
 * it assigns no VDP1, palette, transform, or draw semantics. */
static uint64_t nexus_v1_structure3_capture_bundle_fnv1a64(
    const Nexus_V1_DgnStructure3CaptureImport *capture)
{
    const uint8_t *spans[6];
    size_t sizes[6];
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t span;

    if (!capture) return 0U;
    spans[0] = capture->texture_span;
    spans[1] = capture->palette_state;
    spans[2] = capture->vdp1_state;
    spans[3] = capture->transform_state;
    spans[4] = capture->normal_culling_state;
    spans[5] = capture->vdp1_command;
    sizes[0] = capture->texture_span_size;
    sizes[1] = capture->palette_state_size;
    sizes[2] = capture->vdp1_state_size;
    sizes[3] = capture->transform_state_size;
    sizes[4] = capture->normal_culling_state_size;
    sizes[5] = capture->vdp1_command_size;
    for (span = 0U; span < 6U; ++span) {
        uint8_t length[8];
        size_t byte;
        if (!spans[span] || sizes[span] == 0U) return 0U;
        for (byte = 0U; byte < sizeof(length); ++byte) {
            length[byte] = (uint8_t)(sizes[span] >> (byte * 8U));
            hash ^= length[byte];
            hash *= UINT64_C(1099511628211);
        }
        for (byte = 0U; byte < sizes[span]; ++byte) {
            hash ^= spans[span][byte];
            hash *= UINT64_C(1099511628211);
        }
    }
    return hash;
}

static int nexus_v1_copy_structure3_capture_span(
    uint8_t **out_data, int *out_size, const uint8_t *data, size_t size)
{
    uint8_t *copy;

    if (!out_data || !out_size || !data || size == 0U || size > (size_t)INT_MAX)
        return 0;
    copy = (uint8_t *)malloc(size);
    if (!copy) return 0;
    memcpy(copy, data, size);
    *out_data = copy;
    *out_size = (int)size;
    return 1;
}

/* This is a capture-admission gate, not a decoder. The raw lanes may enter
 * engine-owned storage only when the full VDP1 snapshot independently
 * contains both the captured command and its CMDSRCA texture window. */
static int nexus_v1_structure3_capture_vdp1_snapshot_ready(
    const Nexus_V1_DgnStructure3CaptureImport *capture)
{
    Nexus_V1_Vdp1TextureCommand command;
    uint32_t offset;
    int command_occurrences = 0;

    if (!capture || !capture->texture_span || !capture->vdp1_state ||
        !capture->vdp1_command ||
        capture->vdp1_state_size != NEXUS_V1_VDP1_VRAM_BYTES ||
        capture->vdp1_command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        capture->texture_span_size == 0U ||
        nexus_v1_vdp1_texture_command_parse(capture->vdp1_command,
            (int)capture->vdp1_command_size, &command) != 0 ||
        !command.texture_command || !command.colour_mode_documented ||
        !command.texture_source_range_valid ||
        capture->texture_span_size != (size_t)command.texture_byte_count ||
        command.texture_source_byte_end > NEXUS_V1_VDP1_VRAM_BYTES ||
        !command.link_target_range_valid) {
        return 0;
    }
    if (memcmp(capture->texture_span,
            capture->vdp1_state + command.texture_source_byte_offset,
            capture->texture_span_size) != 0) {
        return 0;
    }
    for (offset = 0U;
         offset <= NEXUS_V1_VDP1_VRAM_BYTES - NEXUS_V1_VDP1_COMMAND_BYTES;
         offset += 8U) {
        if (memcmp(capture->vdp1_state + offset, capture->vdp1_command,
                   NEXUS_V1_VDP1_COMMAND_BYTES) == 0) {
            ++command_occurrences;
        }
    }
    return command_occurrences == 1;
}

int nexus_v1_engine_consume_structure3_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *candidate,
    const Nexus_V1_DgnStructure3FaceCaptureBindingReceipt *binding,
    const Nexus_V1_DgnStructure3CaptureImport *capture)
{
    Nexus_V1_DgnStructure3MeshEntryReceipt entry;
    Nexus_V1_DgnStructure3Vector *vertices = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    Nexus_V1_DgnStructure3Vector *normals = NULL;
    Nexus_V1_DgnStructure3RuntimeSource source;
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt rebound;
    int slot;
    int slot_count;

    if (!engine || !candidate || !binding || !capture ||
        !capture->texture_span || !capture->palette_state ||
        !capture->vdp1_state || !capture->transform_state ||
        !capture->normal_culling_state || !capture->vdp1_command ||
        capture->texture_span_size == 0U || capture->palette_state_size == 0U ||
        capture->vdp1_state_size == 0U || capture->transform_state_size == 0U ||
        capture->normal_culling_state_size == 0U ||
        capture->vdp1_command_size == 0U ||
        capture->texture_span_size > (size_t)INT_MAX ||
        capture->palette_state_size > (size_t)INT_MAX ||
        capture->vdp1_state_size > (size_t)INT_MAX ||
        capture->transform_state_size > (size_t)INT_MAX ||
        capture->normal_culling_state_size > (size_t)INT_MAX ||
        capture->vdp1_command_size > (size_t)INT_MAX ||
        !capture->capture_session_fnv1a64 || !capture->capture_bundle_fnv1a64 ||
        !capture->capture_trace_order_fnv1a64 ||
        !capture->capture_bundle_hash_verified ||
        !capture->capture_trace_order_verified ||
        !capture->original_saturn_capture_verified || !engine->level_loaded ||
        !engine->current_level_dgn_data || engine->current_level_dgn_size <= 0 ||
        !engine->current_level_structure2_source.canonical_hash_verified ||
        !engine->current_level_structure2_source.materialization_bound ||
        !nexus_v1_dgn_source_bytes_match(
            &engine->current_level_structure2_source,
            engine->current_level_dgn_data, engine->current_level_dgn_size) ||
        engine->current_level_structure2_source.level_index !=
            engine->game.current_level ||
        !binding->dgn_source_hash_verified || !binding->capture_source_verified ||
        !binding->complete_source_binding ||
        binding->renderer_handoff_ready || !binding->blocks_real_dgn_mesh_render)
        return 0;

    /* The launcher may already have authenticated this receipt, but never
     * accept its booleans as a substitute for the bytes now entering M11.
     * Recompute both the transport bundle and the DGN face binding against
     * the currently loaded canonical level. */
    if (nexus_v1_structure3_capture_bundle_fnv1a64(capture) !=
        capture->capture_bundle_fnv1a64) return 0;
    if (!nexus_v1_structure3_capture_vdp1_snapshot_ready(capture)) return 0;
    memset(&rebound, 0, sizeof(rebound));
    if (nexus_v1_dgn_bind_structure3_face_capture_candidate(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, 1, 1, candidate,
            capture->texture_span, (int)capture->texture_span_size,
            capture->palette_state, (int)capture->palette_state_size,
            capture->vdp1_state, (int)capture->vdp1_state_size,
            capture->transform_state, (int)capture->transform_state_size,
            capture->normal_culling_state,
            (int)capture->normal_culling_state_size,
            capture->vdp1_command, (int)capture->vdp1_command_size,
            &rebound) != 0 || !rebound.complete_source_binding ||
        rebound.renderer_handoff_ready ||
        !rebound.blocks_real_dgn_mesh_render) return 0;

    memset(&entry, 0, sizeof(entry));
    if (nexus_v1_level_extract_structure3_mesh_entry(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, (int)candidate->entry_index,
            NULL, 0, NULL, 0, NULL, 0, &entry) != -1 ||
        !entry.source_identity_valid || candidate->face_ordinal >=
            (uint32_t)entry.face_count) return 0;
    vertices = (Nexus_V1_DgnStructure3Vector *)calloc(
        (size_t)entry.vertex_count, sizeof(*vertices));
    faces = (Nexus_V1_DgnStructure3Face *)calloc(
        (size_t)entry.face_count, sizeof(*faces));
    normals = (Nexus_V1_DgnStructure3Vector *)calloc(
        (size_t)entry.normal_count, sizeof(*normals));
    if ((entry.vertex_count && !vertices) || (entry.face_count && !faces) ||
        (entry.normal_count && !normals) ||
        nexus_v1_level_extract_structure3_mesh_entry(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, (int)candidate->entry_index,
            vertices, entry.vertex_count, faces, entry.face_count, normals,
            entry.normal_count, &entry) != 0 || !entry.valid) {
        free(vertices);
        free(faces);
        free(normals);
        return 0;
    }

    memset(&source, 0, sizeof(source));
    source.level_index = engine->game.current_level;
    source.entry_index = candidate->entry_index;
    source.face_ordinal = candidate->face_ordinal;
    source.face = faces[candidate->face_ordinal];
    slot_count = source.face.triangle ? 3 : 4;
    for (slot = 0; slot < slot_count; ++slot) {
        uint16_t index = source.face.vertex_indexes[slot];
        if (index >= (uint16_t)entry.vertex_count) {
            free(vertices);
            free(faces);
            free(normals);
            return 0;
        }
        source.vertices[slot] = vertices[index];
    }
    source.vertex_slot_count = slot_count;
    source.normal = normals[candidate->face_ordinal];
    if (!nexus_v1_copy_structure3_capture_span(
            &source.texture_span, &source.texture_span_size,
            capture->texture_span, capture->texture_span_size) ||
        !nexus_v1_copy_structure3_capture_span(
            &source.palette_state, &source.palette_state_size,
            capture->palette_state, capture->palette_state_size) ||
        !nexus_v1_copy_structure3_capture_span(
            &source.vdp1_state, &source.vdp1_state_size,
            capture->vdp1_state, capture->vdp1_state_size) ||
        !nexus_v1_copy_structure3_capture_span(
            &source.transform_state, &source.transform_state_size,
            capture->transform_state, capture->transform_state_size) ||
        !nexus_v1_copy_structure3_capture_span(
            &source.normal_culling_state, &source.normal_culling_state_size,
            capture->normal_culling_state, capture->normal_culling_state_size) ||
        !nexus_v1_copy_structure3_capture_span(
            &source.vdp1_command, &source.vdp1_command_size,
            capture->vdp1_command, capture->vdp1_command_size)) {
        free(source.texture_span);
        free(source.palette_state);
        free(source.vdp1_state);
        free(source.transform_state);
        free(source.normal_culling_state);
        free(source.vdp1_command);
        free(vertices);
        free(faces);
        free(normals);
        return 0;
    }
    source.capture_session_fnv1a64 = capture->capture_session_fnv1a64;
    source.capture_bundle_fnv1a64 = capture->capture_bundle_fnv1a64;
    source.capture_trace_order_fnv1a64 = capture->capture_trace_order_fnv1a64;
    source.capture_bundle_hash_verified =
        capture->capture_bundle_hash_verified != 0;
    source.capture_trace_order_verified =
        capture->capture_trace_order_verified != 0;
    source.original_saturn_capture_verified =
        capture->original_saturn_capture_verified != 0;
    source.binding = rebound;
    source.valid = 1;
    source.blocks_real_dgn_mesh_render = 1;
    free(vertices);
    free(faces);
    free(normals);

    nexus_v1_clear_structure3_runtime_source(engine);
    engine->structure3_runtime_source = source;
    return 1;
}

int nexus_v1_engine_consume_structure3_raw_capture_manifest(
    Nexus_V1_Engine *engine, const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RuntimeCaptureIntakeReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3RuntimeCaptureIntakeReceipt receipt;
    Nexus_V1_DgnStructure3RawCaptureHostReceipt raw_host;
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    int accepted;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!engine || !manifest_text || !paths || !attestation ||
        !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    receipt.active_canonical_lev_bound =
        source->level_index == engine->game.current_level &&
        source->canonical_hash_verified && source->materialization_bound &&
        source->loaded_bytes_bound &&
        nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                        engine->current_level_dgn_size);
    if (!receipt.active_canonical_lev_bound) {
        *out_receipt = receipt;
        return 0;
    }
    nexus_v1_dgn_structure3_raw_capture_host_receipt_clear(&raw_host);
    receipt.raw_capture_host_intake_invoked = 1;
    accepted = nexus_v1_dgn_structure3_raw_capture_host_intake(
        &engine->current_level, engine->current_level_dgn_data,
        engine->current_level_dgn_size, 1, manifest_text, manifest_size,
        paths, attestation, &raw_host);
    receipt.manifest_parsed = raw_host.manifest_parsed;
    receipt.all_trace_lanes_authenticated = raw_host.raw_reader.manifest_accepted &&
        raw_host.raw_reader.all_spans_read &&
        raw_host.raw_reader.raw_span_hashes_match &&
        raw_host.raw_reader.attestation_session_matches &&
        raw_host.raw_reader.attestation_bundle_matches &&
        raw_host.raw_reader.attestation_trace_order_matches &&
        raw_host.raw_reader.original_saturn_source_attested &&
        raw_host.raw_reader.import_ready;
    receipt.complete_source_binding = accepted &&
        raw_host.host.host_dgn_source_verified &&
        raw_host.host.capture_source_verified &&
        raw_host.host.manifest_parsed && raw_host.host.importer_invoked &&
        raw_host.host.import_receipt.complete_source_binding &&
        raw_host.host.import_receipt.blocks_real_dgn_mesh_render;
    if (receipt.all_trace_lanes_authenticated &&
        receipt.complete_source_binding) {
        receipt.engine_consume_invoked = 1;
        receipt.runtime_source_consumed = nexus_v1_engine_consume_structure3_capture(
            engine, &raw_host.host.manifest.candidate,
            &raw_host.host.import_receipt.binding,
            &raw_host.raw_reader.import_packet);
    }
    nexus_v1_dgn_structure3_raw_capture_host_receipt_release(&raw_host);
    *out_receipt = receipt;
    return receipt.runtime_source_consumed;
}

static int nexus_v1_structure3_capture_candidate_equal(
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *left,
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *right)
{
    return left && right &&
        left->dgn_fnv1a64 == right->dgn_fnv1a64 &&
        left->structure3_payload_fnv1a32 == right->structure3_payload_fnv1a32 &&
        left->typed_mesh_corpus_fnv1a32 == right->typed_mesh_corpus_fnv1a32 &&
        left->entry_index == right->entry_index &&
        left->face_ordinal == right->face_ordinal &&
        left->face_row_fnv1a32 == right->face_row_fnv1a32 &&
        left->referenced_vertex_rows_fnv1a32 ==
            right->referenced_vertex_rows_fnv1a32 &&
        left->normal_row_fnv1a32 == right->normal_row_fnv1a32 &&
        left->fill_selector == right->fill_selector;
}

static int nexus_v1_structure1a_structure3_capture_target_equal(
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *left,
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *right)
{
    return left && right && left->valid && right->valid &&
        left->level_index == right->level_index &&
        left->owner_x == right->owner_x && left->owner_y == right->owner_y &&
        left->structure1f_entry_index == right->structure1f_entry_index &&
        left->structure1f_family == right->structure1f_family &&
        left->structure1f_tag == right->structure1f_tag &&
        left->structure1f_face_selector == right->structure1f_face_selector &&
        left->structure1a_index == right->structure1a_index &&
        left->structure1a_kind == right->structure1a_kind &&
        left->structure3_model_index == right->structure3_model_index &&
        left->z_rotation == right->z_rotation &&
        left->structure3_payload_fnv1a32 == right->structure3_payload_fnv1a32 &&
        left->structure3_entry_mapping_proven == 0 &&
        right->structure3_entry_mapping_proven == 0 &&
        left->capture_producer_required && right->capture_producer_required &&
        left->original_saturn_capture_required &&
        right->original_saturn_capture_required &&
        left->no_draw_only && right->no_draw_only &&
        !left->fallback_visuals_permitted &&
        !right->fallback_visuals_permitted &&
        left->face_target.valid && right->face_target.valid &&
        left->face_target.level_index == right->face_target.level_index &&
        nexus_v1_structure3_capture_candidate_equal(&left->face_target.candidate,
                                                     &right->face_target.candidate) &&
        left->face_target.entry_byte_offset == right->face_target.entry_byte_offset &&
        left->face_target.vertex_byte_offset == right->face_target.vertex_byte_offset &&
        left->face_target.face_byte_offset == right->face_target.face_byte_offset &&
        left->face_target.normal_byte_offset == right->face_target.normal_byte_offset &&
        left->face_target.vertex_count == right->face_target.vertex_count &&
        left->face_target.face_count == right->face_target.face_count &&
        left->face_target.capture_producer_required &&
        right->face_target.capture_producer_required &&
        left->face_target.original_saturn_capture_required &&
        right->face_target.original_saturn_capture_required &&
        left->face_target.no_draw_only && right->face_target.no_draw_only &&
        !left->face_target.fallback_visuals_permitted &&
        !right->face_target.fallback_visuals_permitted;
}

int nexus_v1_engine_bind_structure1a_structure3_runtime_correlation(
    Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *target,
    Nexus_V1_DgnStructure1AStructure3RuntimeCorrelationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate owner;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt rebuilt;
    Nexus_V1_DgnStructure3RuntimeSource *runtime;
    const Nexus_V1_DgnStructure1FEntry *entry;
    const Nexus_V1_DgnStructure1AModel *model;
    Nexus_V1_DgnStructure1AStructure3RuntimeCorrelationReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !target || !engine->level_loaded ||
        !engine->current_level_dgn_data || engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.active_canonical_lev_bound =
        engine->current_level_structure2_source.level_index ==
            engine->game.current_level &&
        engine->current_level_structure2_source.canonical_hash_verified &&
        engine->current_level_structure2_source.materialization_bound &&
        engine->current_level_structure2_source.loaded_bytes_bound &&
        nexus_v1_dgn_source_bytes_match(&engine->current_level_structure2_source,
                                        engine->current_level_dgn_data,
                                        engine->current_level_dgn_size);
    runtime = &engine->structure3_runtime_source;
    receipt.runtime_capture_attested = runtime->valid &&
        runtime->level_index == engine->game.current_level &&
        runtime->capture_bundle_hash_verified &&
        runtime->capture_trace_order_verified &&
        runtime->original_saturn_capture_verified &&
        runtime->binding.complete_source_binding &&
        !runtime->binding.renderer_handoff_ready &&
        runtime->binding.blocks_real_dgn_mesh_render &&
        runtime->blocks_real_dgn_mesh_render;
    if (!receipt.active_canonical_lev_bound || !receipt.runtime_capture_attested ||
        !target->valid || target->level_index != engine->game.current_level ||
        target->structure3_entry_mapping_proven || !target->no_draw_only ||
        target->fallback_visuals_permitted ||
        target->structure1f_entry_index < 0 ||
        target->structure1f_entry_index >=
            engine->current_level.structure1f_entry_count ||
        !engine->current_level.structure1a_table_valid ||
        target->structure1a_index >=
            (uint16_t)engine->current_level.structure1a_model_count ||
        target->face_target.candidate.entry_index != runtime->entry_index ||
        target->face_target.candidate.face_ordinal != runtime->face_ordinal) {
        *out_receipt = receipt;
        return 0;
    }
    entry = &engine->current_level.structure1f_entries[
        target->structure1f_entry_index];
    model = &engine->current_level.structure1a_models[target->structure1a_index];
    if (entry->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
        !entry->structure1a_relation_valid ||
        entry->family != target->structure1f_family ||
        entry->tag != target->structure1f_tag ||
        entry->face != target->structure1f_face_selector ||
        entry->structure1a_index != target->structure1a_index ||
        entry->structure1a_owner_x != target->owner_x ||
        entry->structure1a_owner_y != target->owner_y ||
        entry->structure1a_structure3_model_index != target->structure3_model_index ||
        entry->structure1a_z_rotation != target->z_rotation ||
        model->kind != target->structure1a_kind ||
        model->structure3_model_index != target->structure3_model_index ||
        model->z_rotation != target->z_rotation) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&owner, 0, sizeof(owner));
    owner.entry_index = target->structure1f_entry_index;
    owner.owner_x = target->owner_x;
    owner.owner_y = target->owner_y;
    owner.structure1f_family = target->structure1f_family;
    owner.structure1f_tag = target->structure1f_tag;
    owner.structure1f_face_selector = target->structure1f_face_selector;
    owner.structure1f_structure1a_index = target->structure1a_index;
    owner.structure1f_binding_proven = 1;
    owner.structure1a_kind = target->structure1a_kind;
    owner.structure1a_row_binding_proven = 1;
    owner.structure3_model_index = target->structure3_model_index;
    owner.z_rotation = target->z_rotation;
    owner.structure1a_model_rotation_binding_proven = 1;
    owner.structure3_byte_size = engine->current_level.structure3_payload.byte_size;
    owner.structure3_raw_payload_hash =
        engine->current_level.structure3_payload.raw_payload_hash;
    memset(&rebuilt, 0, sizeof(rebuilt));
    if (!nexus_v1_dgn_structure1a_structure3_capture_target_build(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, engine->game.current_level, 1,
            &owner, runtime->entry_index, runtime->face_ordinal, &rebuilt) ||
        !nexus_v1_structure1a_structure3_capture_target_equal(target, &rebuilt)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.target_source_revalidated = 1;
    runtime->structure1a_owner_correlation_bound = 1;
    runtime->structure1a_owner_x = target->owner_x;
    runtime->structure1a_owner_y = target->owner_y;
    runtime->structure1f_entry_index = target->structure1f_entry_index;
    runtime->structure1f_family = target->structure1f_family;
    runtime->structure1f_tag = target->structure1f_tag;
    runtime->structure1f_face_selector = target->structure1f_face_selector;
    runtime->structure1a_index = target->structure1a_index;
    runtime->structure1a_kind = target->structure1a_kind;
    runtime->structure3_model_index = target->structure3_model_index;
    runtime->z_rotation = target->z_rotation;
    runtime->structure3_owner_payload_fnv1a32 = target->structure3_payload_fnv1a32;
    runtime->structure3_entry_mapping_proven = 0;
    runtime->blocks_real_dgn_mesh_render = 1;
    receipt.owner_context_bound = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_structure3_render_packet(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3RenderPacket *out_packet)
{
    const Nexus_V1_DgnStructure3RuntimeSource *source;
    const Nexus_V1_DgnStructure3FaceCaptureBindingReceipt *binding;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded) return 0;
    source = &engine->structure3_runtime_source;
    binding = &source->binding;
    if (!source->valid || source->level_index != engine->game.current_level ||
        !source->capture_bundle_hash_verified ||
        !source->capture_trace_order_fnv1a64 ||
        !source->capture_trace_order_verified ||
        !source->original_saturn_capture_verified ||
        !binding->candidate_framing_valid ||
        !binding->dgn_source_hash_verified ||
        !binding->capture_source_verified || !binding->dgn_source_matches ||
        !binding->structure3_payload_matches ||
        !binding->typed_mesh_corpus_matches || !binding->entry_face_matches ||
        !binding->face_row_matches ||
        !binding->referenced_vertex_rows_match || !binding->normal_row_matches ||
        !binding->fill_selector_matches || !binding->texture_span_matches ||
        !binding->palette_state_matches || !binding->vdp1_state_matches ||
        !binding->transform_state_matches ||
        !binding->normal_culling_state_matches ||
        !binding->vdp1_command_matches ||
        !binding->vdp1_command_format_matches ||
        !binding->vdp1_texture_span_size_matches ||
        !binding->complete_source_binding ||
        binding->renderer_handoff_ready ||
        !binding->blocks_real_dgn_mesh_render ||
        source->vertex_slot_count < 3 || source->vertex_slot_count > 4 ||
        !source->texture_span || source->texture_span_size <= 0 ||
        !source->palette_state || source->palette_state_size <= 0 ||
        !source->vdp1_state || source->vdp1_state_size <= 0 ||
        !source->transform_state || source->transform_state_size <= 0 ||
        !source->normal_culling_state || source->normal_culling_state_size <= 0 ||
        !source->vdp1_command || source->vdp1_command_size <= 0) return 0;

    out_packet->valid = 1;
    out_packet->source_geometry_bound = 1;
    out_packet->level_index = source->level_index;
    out_packet->entry_index = source->entry_index;
    out_packet->face_ordinal = source->face_ordinal;
    out_packet->face = source->face;
    out_packet->vertices = source->vertices;
    out_packet->vertex_count = source->vertex_slot_count;
    out_packet->normal = &source->normal;
    out_packet->texture_span = source->texture_span;
    out_packet->texture_span_size = source->texture_span_size;
    out_packet->palette_state = source->palette_state;
    out_packet->palette_state_size = source->palette_state_size;
    out_packet->vdp1_state = source->vdp1_state;
    out_packet->vdp1_state_size = source->vdp1_state_size;
    out_packet->transform_state = source->transform_state;
    out_packet->transform_state_size = source->transform_state_size;
    out_packet->normal_culling_state = source->normal_culling_state;
    out_packet->normal_culling_state_size = source->normal_culling_state_size;
    out_packet->vdp1_command = source->vdp1_command;
    out_packet->vdp1_command_size = source->vdp1_command_size;
    out_packet->structure1a_owner_correlation_bound =
        source->structure1a_owner_correlation_bound;
    out_packet->structure1a_owner_x = source->structure1a_owner_x;
    out_packet->structure1a_owner_y = source->structure1a_owner_y;
    out_packet->structure1f_entry_index = source->structure1f_entry_index;
    out_packet->structure1f_family = source->structure1f_family;
    out_packet->structure1f_tag = source->structure1f_tag;
    out_packet->structure1f_face_selector = source->structure1f_face_selector;
    out_packet->structure1a_index = source->structure1a_index;
    out_packet->structure1a_kind = source->structure1a_kind;
    out_packet->structure3_model_index = source->structure3_model_index;
    out_packet->z_rotation = source->z_rotation;
    out_packet->structure3_owner_payload_fnv1a32 =
        source->structure3_owner_payload_fnv1a32;
    out_packet->structure3_entry_mapping_proven = 0;
    return 1;
}

int nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt receipt;
    Nexus_V1_DgnStructure3RenderPacket packet;
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        !nexus_v1_dgn_source_bytes_match(source,
                                         engine->current_level_dgn_data,
                                         engine->current_level_dgn_size)) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&packet, 0, sizeof(packet));
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) != 1 ||
        !packet.valid || !packet.source_geometry_bound ||
        !packet.no_draw_only || !packet.blocks_real_dgn_mesh_render ||
        !packet.vdp1_command ||
        packet.vdp1_command_size != NEXUS_V1_VDP1_COMMAND_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = engine->game.current_level;
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    receipt.original_saturn_capture_bound = 1;
    receipt.complete_vdp1_command_record = 1;
    receipt.command_format_parsed = nexus_v1_vdp1_texture_command_parse(
        packet.vdp1_command, packet.vdp1_command_size, &receipt.command) == 0;
    receipt.texture_primitive_observed = receipt.command_format_parsed &&
        receipt.command.texture_command;
    receipt.texture_format_framed = receipt.texture_primitive_observed &&
        receipt.command.colour_mode_documented &&
        receipt.command.texture_byte_count > 0U &&
        receipt.command.texture_source_range_valid;
    receipt.texture_span_size_matches_command = receipt.texture_format_framed &&
        packet.texture_span_size == (int)receipt.command.texture_byte_count;
    receipt.coordinate_words_framed = receipt.texture_primitive_observed &&
        receipt.command.coordinate_words_framed;
    /* Command-table framing never proves payload byte order, VDP1 colour
     * interpretation, palette/CLUT relation, or host draw behavior. */
    receipt.valid = receipt.command_format_parsed;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_current_level_structure3_vdp1_vram_window_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt receipt;
    Nexus_V1_DgnStructure3RenderPacket packet;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
                       engine, &receipt.command_framing) != 1 ||
        !receipt.command_framing.valid ||
        !receipt.command_framing.texture_format_framed ||
        !receipt.command_framing.texture_span_size_matches_command) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&packet, 0, sizeof(packet));
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) != 1 ||
        !packet.valid || !packet.vdp1_state ||
        packet.vdp1_state_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        !packet.texture_span || packet.texture_span_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = receipt.command_framing.level_index;
    receipt.source_byte_count = receipt.command_framing.source_byte_count;
    receipt.source_bytes_fnv1a64 = receipt.command_framing.source_bytes_fnv1a64;
    receipt.original_saturn_capture_bound = 1;
    receipt.complete_vdp1_vram_snapshot = 1;
    if (receipt.command_framing.command.texture_source_byte_end >
            NEXUS_V1_VDP1_VRAM_BYTES ||
        packet.texture_span_size !=
            (int)receipt.command_framing.command.texture_byte_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.texture_lane_matches_vram_window = memcmp(packet.texture_span,
        packet.vdp1_state +
            receipt.command_framing.command.texture_source_byte_offset,
        (size_t)packet.texture_span_size) == 0;
    receipt.valid = receipt.texture_lane_matches_vram_window;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_current_level_structure3_vdp1_command_vram_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt receipt;
    Nexus_V1_DgnStructure3RenderPacket packet;
    uint32_t offset;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.command_record_byte_offset = UINT32_MAX;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
                       engine, &receipt.command_framing) != 1 ||
        !receipt.command_framing.valid ||
        !receipt.command_framing.complete_vdp1_command_record) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&packet, 0, sizeof(packet));
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) != 1 ||
        !packet.valid || !packet.vdp1_state ||
        packet.vdp1_state_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        !packet.vdp1_command ||
        packet.vdp1_command_size != NEXUS_V1_VDP1_COMMAND_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = receipt.command_framing.level_index;
    receipt.source_byte_count = receipt.command_framing.source_byte_count;
    receipt.source_bytes_fnv1a64 = receipt.command_framing.source_bytes_fnv1a64;
    receipt.original_saturn_capture_bound = 1;
    receipt.complete_vdp1_vram_snapshot = 1;
    for (offset = 0U;
         offset <= NEXUS_V1_VDP1_VRAM_BYTES - NEXUS_V1_VDP1_COMMAND_BYTES;
         offset += 8U) {
        if (memcmp(packet.vdp1_state + offset, packet.vdp1_command,
                   NEXUS_V1_VDP1_COMMAND_BYTES) != 0)
            continue;
        if (receipt.command_record_occurrence_count == 0)
            receipt.command_record_byte_offset = offset;
        ++receipt.command_record_occurrence_count;
    }
    receipt.command_record_unique_in_vram =
        receipt.command_record_occurrence_count == 1;
    receipt.command_link_byte_offset = receipt.command_framing.command.link_byte_offset;
    receipt.command_link_target_bounded =
        receipt.command_framing.command.link_target_range_valid;
    if (receipt.command_link_target_bounded) {
        receipt.command_link_target_record_framed =
            nexus_v1_vdp1_texture_command_parse(packet.vdp1_state +
                    receipt.command_link_byte_offset,
                NEXUS_V1_VDP1_COMMAND_BYTES,
                &receipt.command_link_target) == 0;
    }
    receipt.valid = receipt.command_record_unique_in_vram &&
        receipt.command_link_target_bounded &&
        receipt.command_link_target_record_framed;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_current_level_dgn_renderer_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveLevelRendererSourceReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    Nexus_V1_DgnStructure3RenderPacket packet;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;

    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !nexus_v1_dgn_source_bytes_match(source,
                                         engine->current_level_dgn_data,
                                         engine->current_level_dgn_size)) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->package_source_bound = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    out_receipt->structure3_payload_bound =
        engine->current_level.structure3_payload.valid &&
        engine->current_level.structure3_payload.byte_size > 0;
    if (out_receipt->structure3_payload_bound) {
        out_receipt->structure3_payload_byte_count =
            engine->current_level.structure3_payload.byte_size;
        out_receipt->structure3_payload_fnv1a32 =
            engine->current_level.structure3_payload.raw_payload_hash;
    }

    memset(&packet, 0, sizeof(packet));
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) > 0) {
        out_receipt->original_saturn_capture_bound = 1;
        out_receipt->texture_span_bound = packet.texture_span_size > 0;
        out_receipt->palette_state_bound = packet.palette_state_size > 0;
        out_receipt->vdp1_state_bound = packet.vdp1_state_size > 0;
        out_receipt->transform_state_bound = packet.transform_state_size > 0;
        out_receipt->normal_culling_state_bound =
            packet.normal_culling_state_size > 0;
        out_receipt->vdp1_command_bound = packet.vdp1_command_size > 0;
    }
    out_receipt->vdp1_command_format_framed =
        nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
            engine, &out_receipt->vdp1_command_framing) == 1;
    out_receipt->vdp1_texture_format_framed =
        out_receipt->vdp1_command_format_framed &&
        out_receipt->vdp1_command_framing.texture_format_framed &&
        out_receipt->vdp1_command_framing.texture_span_size_matches_command;
    out_receipt->vdp1_coordinate_words_framed =
        out_receipt->vdp1_command_format_framed &&
        out_receipt->vdp1_command_framing.coordinate_words_framed;
    out_receipt->vdp1_vram_window_bound =
        nexus_v1_current_level_structure3_vdp1_vram_window_receipt(
            engine, &out_receipt->vdp1_vram_window) == 1;
    out_receipt->vdp1_command_vram_bound =
        nexus_v1_current_level_structure3_vdp1_command_vram_receipt(
            engine, &out_receipt->vdp1_command_vram) == 1;

    /* The opaque capture may be source-bound, but the Saturn codecs and VDP1
     * command meaning are not known. Keep every renderer consumer fail-closed. */
    out_receipt->texture_decode_unproven = 1;
    out_receipt->palette_decode_unproven = 1;
    out_receipt->vdp1_draw_unproven = 1;
    out_receipt->transform_culling_unproven = 1;
    return 1;
}

static void nexus_v1_load_smap_runtime(Nexus_V1_Engine *engine, int level)
{
    char name[32];
    int size = 0;
    uint8_t *data;
    uint32_t *pixels;

    if (!engine) return;
    free(engine->smap_rgba);
    engine->smap_rgba = NULL;
    memset(&engine->smap_runtime_receipt, 0,
           sizeof(engine->smap_runtime_receipt));
    engine->smap_runtime_receipt.level_index = level;
    engine->smap_runtime_receipt.no_draw_only = 1;
    engine->smap_runtime_receipt.fallback_visuals_permitted = 0;
    snprintf(name, sizeof(name), "SMAP%02d.BIN", level);
    (void)nexus_v1_level_aux_source_receipt(
        engine, name, &engine->smap_runtime_receipt.source);
    if (!engine->smap_runtime_receipt.source.canonical_hash_verified)
        return;

    data = nexus_v1_read_file(engine, name, &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    pixels = (uint32_t *)malloc((size_t)NEXUS_SMAP_PIXEL_WIDTH *
                                (size_t)NEXUS_SMAP_PIXEL_HEIGHT *
                                sizeof(*pixels));
    if (pixels && nexus_v1_smap_parse_header(
            data, size, &engine->smap_runtime_receipt.header) &&
        nexus_v1_smap_decode(data, size, pixels,
            NEXUS_SMAP_PIXEL_WIDTH * NEXUS_SMAP_PIXEL_HEIGHT,
            &engine->smap_runtime_receipt.decode)) {
        engine->smap_runtime_receipt.valid = 1;
        engine->smap_runtime_receipt.decoded_pixels_retained = 1;
        engine->smap_rgba = pixels;
        pixels = NULL;
    }
    free(pixels);
    free(data);
}

int nexus_v1_load_level(Nexus_V1_Engine *engine, int level) {
    char name[32];
    char resolved_level_path[ASSET_PATH_MAX];
    char script_name[32];
    char sal_name[32];
    char map_name[32];
    int size = 0;
    int script_size = 0;
    int sal_size = 0;
    int map_size = 0;
    uint8_t *data;
    uint8_t *script_data;
    uint8_t *sal_data;
    uint8_t *map_data;
    const char *canonical_md5;
    int loaded_bytes_canonical;
    Nexus_V1_DungeonStartReceipt dungeon_start;

    if (!engine || level < 0 || level > 15) return -1;
    /* A reload may replace the same numeric level with fresh source bytes.
     * Retained capture evidence is therefore never valid across this boundary. */
    nexus_v1_engine_clear_external_prs3_placement_receipt(engine);
    free(engine->smap_rgba);
    engine->smap_rgba = NULL;
    memset(&engine->smap_runtime_receipt, 0,
           sizeof(engine->smap_runtime_receipt));
    engine->m11_direct_lev_dungeon_no_draw_valid = 0;
    engine->m11_direct_lev_dungeon_route_epoch = 0U;
    memset(&engine->m11_direct_lev_dungeon, 0,
           sizeof(engine->m11_direct_lev_dungeon));
    engine->current_level_source_path[0] = '\0';
    snprintf(name, sizeof(name), "LEV%02d.DGN", level);

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) {
        printf("Nexus: failed to load %s\n", name);
        return -1;
    }

    canonical_md5 = nexus_known_boot_file_md5(name);
    loaded_bytes_canonical = nexus_v1_dgn_bytes_match_canonical_md5(
        data, size, canonical_md5);
    if (!loaded_bytes_canonical) {
        /* This is intentionally before nexus_v1_level_load(): that call
         * performs the Structure3 face/material identifier joins. */
        printf("Nexus: canonical byte hash mismatch for %s\n", name);
        free(data);
        return -1;
    }

    /* Preserve the exact source selected by the same identity hash that
     * admitted `data`. A canonical filename is not a source path: renamed
     * extracted retail bytes and ISO members must remain distinguishable in
     * startup/resume receipts. */
    resolved_level_path[0] = '\0';
    if (engine->source == NEXUS_SRC_EXTRACTED &&
        asset_find_by_md5(engine->data_dir, canonical_md5,
                          resolved_level_path,
                          (int)sizeof(resolved_level_path), 8)) {
        snprintf(engine->current_level_source_path,
                 sizeof(engine->current_level_source_path),
                 "%s",
                 resolved_level_path);
    } else if (engine->source == NEXUS_SRC_ISO && engine->iso.path[0]) {
        snprintf(engine->current_level_source_path,
                 sizeof(engine->current_level_source_path),
                 "%s::%s",
                 engine->iso.path,
                 name);
    }

    nexus_v1_invalidate_dgn_material_plan(engine);
    free(engine->current_level_dgn_data);
    free(engine->current_level_dgn_identity_data);
    engine->current_level_dgn_data = NULL;
    engine->current_level_dgn_identity_data = NULL;
    engine->current_level_dgn_size = 0;
    nexus_v1_clear_structure3_runtime_source(engine);
    int r = nexus_v1_level_load(&engine->current_level, data, size, level);
    if (r < 0) {
        free(data);
        return -1;
    }
    engine->current_level_dgn_data = data;
    engine->current_level_dgn_size = size;
    engine->current_level_dgn_identity_data = malloc((size_t)size);
    if (!engine->current_level_dgn_identity_data) {
        free(data);
        engine->current_level_dgn_data = NULL;
        engine->current_level_dgn_size = 0;
        engine->level_loaded = 0;
        return -1;
    }
    memcpy(engine->current_level_dgn_identity_data, data, (size_t)size);
    (void)nexus_v1_structure2_source_receipt(
        engine, level, &engine->current_level, data, size,
        &engine->current_level_structure2_source);
    engine->current_level_structure2_source.loaded_dgn_identity_bytes =
        engine->current_level_dgn_identity_data;

    /* Parsing establishes bounded DGN structure only. Do not promote parsed
     * bytes into item, animation, mechanics, or viewport ownership unless
     * the active source receipt proves that this exact loaded level is the
     * canonical retail LEV entry. */
    if (!engine->current_level_structure2_source.canonical_hash_verified ||
        !engine->current_level_structure2_source.materialization_bound ||
        !engine->current_level_structure2_source.loaded_bytes_bound) {
        printf("Nexus: refusing unverified DGN source for %s\n", name);
        free(data);
        free(engine->current_level_dgn_identity_data);
        memset(&engine->current_level, 0, sizeof(engine->current_level));
        engine->current_level_dgn_data = NULL;
        engine->current_level_dgn_identity_data = NULL;
        engine->current_level_dgn_size = 0;
        engine->level_loaded = 0;
        return -1;
    }

    /* Only hash-bound canonical bytes may cross this point. */
    nexus_v1_load_item_ibs_runtime_source(engine);
    nexus_v1_load_shop_catalog(engine);
    (void)nexus_v1_decode_structure2_animation_materials(engine, data, size);

    /* Nexus source-lock: docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md
     * fixes new game at LEV00 (11,29,N). Accept it only after the real
     * Structure1B cell has been decoded, so corrupt media or a parser change
     * cannot start mechanics or DGN rendering inside a wall. */
    if (level == NEXUS_V1_INITIAL_PARTY_LEVEL && !engine->game.game_started) {
        if (!nexus_v1_game_resolve_dungeon_start(
                &engine->current_level,
                level,
                NEXUS_V1_INITIAL_PARTY_X,
                NEXUS_V1_INITIAL_PARTY_Y,
                NEXUS_V1_INITIAL_PARTY_DIR,
                &dungeon_start) ||
            !nexus_v1_game_apply_dungeon_start(&engine->game,
                                                &dungeon_start)) {
            engine->game.dungeon_start = dungeon_start;
            return -1;
        }
        if (engine->mechanics) {
            nexus_mechanics_init(engine->mechanics,
                                 engine->game.party_x,
                                 engine->game.party_y,
                                 engine->game.party_dir);
        }
    }

    engine->level_loaded = 1;
    nexus_v1_load_smap_runtime(engine, level);
    if (engine->mechanics) {
        nexus_v1_mechanics_load_level(engine, level);
    }
    nexus_v1_sync_dgn_runtime_pose(engine, level, engine->game.party_x,
                                   engine->game.party_y, engine->game.party_dir);

    snprintf(script_name, sizeof(script_name), "SLEV%02d.BIN", level);
    memset(&engine->level_aux_runtime_receipt, 0,
           sizeof(engine->level_aux_runtime_receipt));
    engine->level_aux_runtime_receipt.level_index = level;
    engine->level_aux_runtime_receipt.fallback_visuals_permitted = 0;
    engine->level_aux_runtime_receipt.sound_driver =
        engine->sound_driver_source;
    (void)nexus_v1_level_aux_source_receipt(
        engine, script_name, &engine->level_aux_runtime_receipt.slev);
    script_data = nexus_v1_read_file(engine, script_name, &script_size);
    (void)nexus_script_vm_load_canonical_level(
        &engine->script_vm, level, script_data, script_size,
        engine->level_aux_runtime_receipt.slev.canonical_hash_verified);
    (void)nexus_script_vm_runtime_receipt(&engine->script_vm,
                                          &engine->script_runtime_receipt);
    memset(&engine->script_trace_admission, 0,
           sizeof(engine->script_trace_admission));
    engine->script_trace_admission.status = NEXUS_V1_SLEV_TRACE_MISSING;
    engine->script_trace_admission.level_index = level;
    engine->script_trace_admission.blocks_real_script_dispatch = 1;
    engine->script_trace_admission.fallback_visuals_permitted = 0;
    memset(&engine->script_trace_host_receipt, 0,
           sizeof(engine->script_trace_host_receipt));
    engine->script_trace_host_receipt.status = NEXUS_V1_SLEV_TRACE_HOST_MISSING;
    engine->script_trace_host_receipt.level_index = level;
    engine->script_trace_host_receipt.blocks_real_script_dispatch = 1;
    engine->script_trace_host_receipt.fallback_visuals_permitted = 0;
    free(script_data);
    nexus_script_on_level_load(&engine->script_vm, level);

    snprintf(sal_name, sizeof(sal_name), "SNDLEV%02d.SAL", level);
    snprintf(map_name, sizeof(map_name), "SNDLEV%02d.MAP", level);
    (void)nexus_v1_level_aux_source_receipt(
        engine, sal_name, &engine->level_aux_runtime_receipt.sal);
    (void)nexus_v1_level_aux_source_receipt(
        engine, map_name, &engine->level_aux_runtime_receipt.map);
    engine->level_aux_runtime_receipt.canonical_pair_bound =
        engine->level_aux_runtime_receipt.slev.canonical_hash_verified &&
        engine->level_aux_runtime_receipt.sal.canonical_hash_verified &&
        engine->level_aux_runtime_receipt.map.canonical_hash_verified;
    sal_data = nexus_v1_read_file(engine, sal_name, &sal_size);
    map_data = nexus_v1_read_file(engine, map_name, &map_size);
    (void)nexus_sound_load_canonical_level(
        &engine->audio, level, sal_data, sal_size, map_data, map_size,
        engine->level_aux_runtime_receipt.sal.canonical_hash_verified,
        engine->level_aux_runtime_receipt.map.canonical_hash_verified);
    nexus_sound_set_driver_canonical_source_verified(
        &engine->audio,
        engine->level_aux_runtime_receipt.sound_driver
            .canonical_hash_verified);
    (void)nexus_sound_level_runtime_receipt(&engine->audio,
                                            &engine->sfx_runtime_receipt);
    memset(&engine->sound_trace_admission, 0,
           sizeof(engine->sound_trace_admission));
    engine->sound_trace_admission.status = NEXUS_V1_SAL_TRACE_MISSING;
    engine->sound_trace_admission.level_index = level;
    engine->sound_trace_admission.blocks_real_sfx_playback = 1;
    engine->sound_trace_admission.fallback_visuals_permitted = 0;
    memset(&engine->sound_trace_host_receipt, 0,
           sizeof(engine->sound_trace_host_receipt));
    engine->sound_trace_host_receipt.status = NEXUS_V1_SAL_TRACE_HOST_MISSING;
    engine->sound_trace_host_receipt.level_index = level;
    engine->sound_trace_host_receipt.blocks_real_sfx_playback = 1;
    engine->sound_trace_host_receipt.fallback_visuals_permitted = 0;
    free(sal_data);
    free(map_data);

    /* Update CD audio track */
    int new_track = nexus_v1_cd_track_for_level(level);
    if (new_track >= 0 && new_track != engine->current_cd_track &&
        engine->audio_enabled) {
        engine->current_cd_track = new_track;
        (void)nexus_sound_cd_track(&engine->audio, new_track);
        printf("Nexus: CD track %d for level %d\n", new_track, level);
        /* FUTURE: CD audio playback via SDL_mixer.
         * DM Nexus (Saturn) uses CD-DA tracks for music. */
    }

    return 0;
}

int nexus_v1_current_level_structure2_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2SourceReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !engine->level_loaded) return 0;
    if (engine->current_level_structure2_source.level_index !=
            engine->game.current_level ||
        !engine->current_level_structure2_source.canonical_hash_verified ||
        !engine->current_level_structure2_source.loaded_bytes_bound ||
        !engine->current_level_structure2_source.materialization_bound ||
        !engine->current_level_structure2_source
            .structure2_payload_envelope_valid ||
        engine->current_level_structure2_source.fallback_visuals_permitted ||
        !engine->current_level_dgn_data || engine->current_level_dgn_size <= 0 ||
        !nexus_v1_dgn_source_bytes_match(
            &engine->current_level_structure2_source,
            engine->current_level_dgn_data, engine->current_level_dgn_size)) {
        return 0;
    }
    *out_receipt = engine->current_level_structure2_source;
    return 0;
}

int nexus_v1_current_level_extract_structure3_mesh_entry(
    const Nexus_V1_Engine *engine, int entry_index,
    Nexus_V1_DgnStructure3Vector *out_vertices, int max_vertices,
    Nexus_V1_DgnStructure3Face *out_faces, int max_faces,
    Nexus_V1_DgnStructure3Vector *out_normals, int max_normals,
    Nexus_V1_DgnStructure3MeshEntryReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3MeshEntryReceipt empty;
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(&empty, 0, sizeof(empty));
    empty.entry_index = entry_index;
    *out_receipt = empty;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return -1;

    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->loaded_bytes_bound ||
        !source->materialization_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                          engine->current_level_dgn_size)) {
        return -1;
    }
    return nexus_v1_level_extract_structure3_mesh_entry(
        &engine->current_level, engine->current_level_dgn_data,
        engine->current_level_dgn_size, entry_index, out_vertices,
        max_vertices, out_faces, max_faces, out_normals, max_normals,
        out_receipt);
}

int nexus_v1_current_level_structure3_directory_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3DirectoryReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure3_directory_receipt(
            &engine->current_level, &out_receipt->directory) != 0 ||
        !out_receipt->directory.valid) return 0;
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    return 1;
}

int nexus_v1_current_level_structure3_mesh_semantic_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3MeshSemanticReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
            &engine->current_level, &out_receipt->mesh_semantics) != 0 ||
        !out_receipt->mesh_semantics.source_facts_complete ||
        !out_receipt->mesh_semantics.blocks_real_dgn_mesh_render) return 0;
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    return 1;
}

int nexus_v1_current_level_structure3_face_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3FaceFramingReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure3_entry_header_receipt(
            &engine->current_level, &out_receipt->entry_headers) != 0 ||
        !out_receipt->entry_headers.valid ||
        nexus_v1_level_structure3_face_receipt(
            &engine->current_level, &out_receipt->faces) != 0 ||
        !out_receipt->faces.valid || !out_receipt->faces.entry_headers_valid ||
        !out_receipt->faces.face_vertex_indexes_valid) return 0;
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    return 1;
}

int nexus_v1_current_level_structure3_face_material_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure3_face_receipt(
            &engine->current_level, &out_receipt->faces) != 0 ||
        !out_receipt->faces.valid ||
        !out_receipt->faces.face_topology_accounting_valid ||
        nexus_v1_level_structure3_face_material_receipt(
            &engine->current_level, &out_receipt->materials) != 0 ||
        !out_receipt->materials.valid ||
        !out_receipt->materials.face_receipt_valid ||
        !out_receipt->materials.selector_bindings_complete ||
        !out_receipt->materials.selector_reuse_accounting_valid ||
        out_receipt->materials.material_or_draw_semantics_proven ||
        out_receipt->materials.face_count != out_receipt->faces.face_count) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    out_receipt->face_topology_material_binding_complete = 1;
    return 1;
}

int nexus_v1_current_level_structure1a_owner_chain_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure1AOwnerChainReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure1f_spatial_receipt(
            &engine->current_level, &out_receipt->spatial) != 0 ||
        !out_receipt->spatial.valid ||
        nexus_v1_level_structure1a_boundary_receipt(
            &engine->current_level, &out_receipt->boundary) != 0 ||
        !out_receipt->boundary.valid ||
        nexus_v1_level_structure1a_relation_receipt(
            &engine->current_level, &out_receipt->relation) != 0 ||
        !out_receipt->relation.complete ||
        nexus_v1_level_structure3_model_reference_receipt(
            &engine->current_level, &out_receipt->model_references) != 0 ||
        !out_receipt->model_references.complete ||
        nexus_v1_level_structure1f_face_selector_receipt(
            &engine->current_level, &out_receipt->face_selectors) != 0 ||
        !out_receipt->face_selectors.complete ||
        out_receipt->face_selectors.face_semantics_proven ||
        nexus_v1_level_structure3_model_face_selector_receipt(
            &engine->current_level, &out_receipt->model_face_selectors) != 0 ||
        !out_receipt->model_face_selectors.complete ||
        out_receipt->model_face_selectors.attachment_semantics_proven ||
        out_receipt->relation.structure1f_bound_entry_count !=
            out_receipt->model_references.structure1f_bound_entry_count ||
        out_receipt->relation.resolved_entry_count !=
            out_receipt->face_selectors.resolved_face_selector_count ||
        out_receipt->model_references.resolved_model_reference_count !=
            out_receipt->model_face_selectors.resolved_pair_count) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    out_receipt->owner_chain_complete = 1;
    return 1;
}

int nexus_v1_current_level_structure2_descriptor_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure2DescriptorReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    const Nexus_V1_Level *level;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    level = &engine->current_level;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->structure2_payload_envelope_valid ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        !level->structure2_texture_table_valid ||
        level->structure2_texture_count <= 0 ||
        !nexus_v1_level_structure2_source_envelope_valid(level) ||
        !level->structure1g_structure2_bindings_complete) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    out_receipt->descriptor_count = level->structure2_texture_count;
    out_receipt->structure1g_entry_count = level->structure1g_entry_count;
    out_receipt->structure1g_structure2_bindings_complete = 1;
    out_receipt->payload = level->structure2_payload;
    out_receipt->descriptor_layout_complete = 1;
    return 1;
}

typedef struct {
    int descriptor_index;
    uint32_t image_offset;
    uint32_t palette_offset;
    Nexus_V1_DgnStructure2PayloadAnchorPacket image;
    Nexus_V1_DgnStructure2PayloadAnchorPacket palette;
    int image_found;
    int palette_found;
} Nexus_V1_DescriptorCaptureAnchorSearch;

static int nexus_v1_find_descriptor_capture_anchor(
    void *context, const Nexus_V1_DgnStructure2PayloadAnchorPacket *packet)
{
    Nexus_V1_DescriptorCaptureAnchorSearch *search =
        (Nexus_V1_DescriptorCaptureAnchorSearch *)context;

    if (!search || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) {
        return -1;
    }
    if (packet->descriptor_index != search->descriptor_index ||
        packet->candidate_byte_count == 0U ||
        packet->next_anchor_offset <= packet->payload_anchor_offset) {
        return 0;
    }
    if (!packet->palette_anchor &&
        packet->payload_anchor_offset == search->image_offset) {
        search->image = *packet;
        search->image_found = 1;
    }
    if (packet->palette_anchor && search->palette_offset != 0U &&
        packet->payload_anchor_offset == search->palette_offset) {
        search->palette = *packet;
        search->palette_found = 1;
    }
    return 0;
}

int nexus_v1_engine_build_structure2_descriptor_capture_target(
    const Nexus_V1_Engine *engine, int descriptor_index,
    Nexus_V1_DgnStructure2DescriptorCaptureTarget *out_target)
{
    Nexus_V1_DgnActiveStructure2DescriptorReceipt source_receipt;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt anchor_scene;
    Nexus_V1_DescriptorCaptureAnchorSearch anchor_search;
    const Nexus_V1_Level *level;
    const uint8_t *data;
    int size;
    int structure2_byte_offset;
    int descriptor_byte_offset;
    int opaque_payload_byte_offset;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->level_index = -1;
    out_target->no_draw_only = 1;
    if (!engine || descriptor_index < 0 ||
        nexus_v1_current_level_structure2_descriptor_receipt(
            engine, &source_receipt) != 1 ||
        descriptor_index >= source_receipt.descriptor_count) {
        return 0;
    }
    level = &engine->current_level;
    data = engine->current_level_dgn_data;
    size = engine->current_level_dgn_size;
    if (!data || size < 0x18 ||
        (data[0x14] == 0U && data[0x15] == 0U)) {
        return 0;
    }
    structure2_byte_offset =
        (int)(((unsigned int)data[0x14] << 8) | data[0x15]) *
        NEXUS_DGN_BLOCK_SIZE;
    descriptor_byte_offset = structure2_byte_offset +
        descriptor_index * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    opaque_payload_byte_offset = structure2_byte_offset +
        level->structure2_payload.opaque_payload_offset;
    if (structure2_byte_offset < 0 ||
        descriptor_byte_offset < structure2_byte_offset ||
        descriptor_byte_offset > size - NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES ||
        opaque_payload_byte_offset < structure2_byte_offset ||
        opaque_payload_byte_offset > size -
            level->structure2_payload.opaque_payload_size) {
        return 0;
    }
    memset(&anchor_scene, 0, sizeof(anchor_scene));
    memset(&anchor_search, 0, sizeof(anchor_search));
    anchor_search.descriptor_index = descriptor_index;
    anchor_search.image_offset =
        level->structure2_textures[descriptor_index].image_relative_offset;
    anchor_search.palette_offset =
        level->structure2_textures[descriptor_index].palette_relative_offset;
    if (nexus_v1_current_level_visit_structure2_payload_anchors(
            engine, nexus_v1_find_descriptor_capture_anchor, &anchor_search,
            &anchor_scene) != 1 || !anchor_scene.valid ||
        anchor_scene.level_index != engine->game.current_level ||
        anchor_scene.source_byte_count != size ||
        anchor_scene.source_bytes_fnv1a64 != source_receipt.source_bytes_fnv1a64 ||
        !anchor_search.image_found ||
        (anchor_search.palette_offset != 0U && !anchor_search.palette_found) ||
        structure2_byte_offset + (int)anchor_search.image.payload_anchor_offset >
            size - (int)anchor_search.image.candidate_byte_count ||
        (anchor_search.palette_offset != 0U &&
         structure2_byte_offset + (int)anchor_search.palette.payload_anchor_offset >
             size - (int)anchor_search.palette.candidate_byte_count)) {
        return 0;
    }
    out_target->valid = 1;
    out_target->level_index = engine->game.current_level;
    out_target->source_byte_count = size;
    out_target->source_bytes_fnv1a64 = source_receipt.source_bytes_fnv1a64;
    out_target->descriptor_index = descriptor_index;
    out_target->descriptor_byte_offset = descriptor_byte_offset;
    out_target->descriptor = level->structure2_textures[descriptor_index];
    out_target->opaque_payload_byte_offset = opaque_payload_byte_offset;
    out_target->opaque_payload_byte_count =
        level->structure2_payload.opaque_payload_size;
    out_target->descriptor_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + descriptor_byte_offset, NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES);
    out_target->opaque_payload_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + opaque_payload_byte_offset,
        level->structure2_payload.opaque_payload_size);
    out_target->image_payload_anchor_offset =
        anchor_search.image.payload_anchor_offset;
    out_target->image_payload_next_anchor_offset =
        anchor_search.image.next_anchor_offset;
    out_target->image_payload_candidate_byte_count =
        anchor_search.image.candidate_byte_count;
    out_target->image_payload_candidate_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + structure2_byte_offset +
            (int)anchor_search.image.payload_anchor_offset,
        (int)anchor_search.image.candidate_byte_count);
    out_target->image_payload_candidate_bound = 1;
    if (anchor_search.palette_offset != 0U) {
        out_target->palette_payload_anchor_offset =
            anchor_search.palette.payload_anchor_offset;
        out_target->palette_payload_next_anchor_offset =
            anchor_search.palette.next_anchor_offset;
        out_target->palette_payload_candidate_byte_count =
            anchor_search.palette.candidate_byte_count;
        out_target->palette_payload_candidate_fnv1a64 =
            nexus_v1_dgn_bytes_fnv1a64(
                data + structure2_byte_offset +
                    (int)anchor_search.palette.payload_anchor_offset,
                (int)anchor_search.palette.candidate_byte_count);
        out_target->palette_payload_candidate_bound = 1;
        out_target->shared_image_palette_payload_anchor =
            anchor_search.palette.payload_anchor_offset ==
            anchor_search.image.payload_anchor_offset;
    }
    out_target->capture_producer_required = 1;
    out_target->original_saturn_capture_required = 1;
    return 1;
}

int nexus_v1_engine_write_structure2_descriptor_capture_target(
    const Nexus_V1_Engine *engine, int descriptor_index, const char *path,
    Nexus_V1_DgnStructure2DescriptorCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure2DescriptorCaptureTarget target;
    char temporary_path[1024];
    FILE *file;
    int write_ok;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    memset(&target, 0, sizeof(target));
    out_target->level_index = -1;
    out_target->no_draw_only = 1;
    if (!path || !path[0] ||
        nexus_v1_engine_build_structure2_descriptor_capture_target(
            engine, descriptor_index, &target) != 1 ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path)) {
        return 0;
    }
    file = fopen(temporary_path, "wb");
    if (!file) return 0;
    write_ok = fprintf(file,
                       "NEXUS_STRUCTURE2_DESCRIPTOR_CAPTURE_TARGET_V1\n"
                       "level=%d\nsource_bytes=%d\nsource_fnv1a64=%016llx\n"
                       "descriptor_index=%d\ndescriptor_byte_offset=%d\n"
                       "image_id=%u\nencoding=%u\npalette_id=%u\nwidth=%u\nheight=%u\n"
                       "image_relative_offset=%u\npalette_relative_offset=%u\n"
                       "opaque_payload_byte_offset=%d\nopaque_payload_byte_count=%d\n"
                       "descriptor_fnv1a64=%016llx\nopaque_payload_fnv1a64=%016llx\n"
                       "image_anchor_offset=%u\nimage_next_anchor_offset=%u\n"
                       "image_candidate_byte_count=%u\nimage_candidate_fnv1a64=%016llx\n"
                       "palette_anchor_offset=%u\npalette_next_anchor_offset=%u\n"
                       "palette_candidate_byte_count=%u\npalette_candidate_fnv1a64=%016llx\n"
                       "palette_candidate_present=%d\n"
                       "shared_image_palette_payload_anchor=%d\n"
                       "requested_observations=source-read,palette-state,vdp1-vram,vdp1-command\n"
                       "original_saturn_capture_required=1\nno_draw_only=1\n",
                       target.level_index, target.source_byte_count,
                       (unsigned long long)target.source_bytes_fnv1a64,
                       target.descriptor_index, target.descriptor_byte_offset,
                       target.descriptor.image_id, target.descriptor.encoding,
                       target.descriptor.palette_id, target.descriptor.width,
                       target.descriptor.height, target.descriptor.image_relative_offset,
                       target.descriptor.palette_relative_offset,
                       target.opaque_payload_byte_offset,
                       target.opaque_payload_byte_count,
                       (unsigned long long)target.descriptor_bytes_fnv1a64,
                       (unsigned long long)target.opaque_payload_fnv1a64,
                       target.image_payload_anchor_offset,
                       target.image_payload_next_anchor_offset,
                       target.image_payload_candidate_byte_count,
                       (unsigned long long)target.image_payload_candidate_fnv1a64,
                       target.palette_payload_anchor_offset,
                       target.palette_payload_next_anchor_offset,
                       target.palette_payload_candidate_byte_count,
                       (unsigned long long)target.palette_payload_candidate_fnv1a64,
                       target.palette_payload_candidate_bound,
                       target.shared_image_palette_payload_anchor) >= 0;
    if (!write_ok || fclose(file) != 0 || rename(temporary_path, path) != 0) {
        remove(temporary_path);
        return 0;
    }
    *out_target = target;
    return 1;
}

int nexus_v1_current_level_structure2_format_evidence_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2FormatEvidenceReceipt *out_receipt)
{
    Nexus_V1_DgnActiveStructure2DescriptorReceipt descriptor_receipt;
    const Nexus_V1_Level *level;
    uint32_t opaque_start;
    uint32_t opaque_end;
    int index;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || nexus_v1_current_level_structure2_descriptor_receipt(
            engine, &descriptor_receipt) != 1) {
        return 0;
    }
    level = &engine->current_level;
    opaque_start = (uint32_t)level->structure2_payload.opaque_payload_offset;
    opaque_end = opaque_start +
        (uint32_t)level->structure2_payload.opaque_payload_size;
    if (level->structure2_payload.opaque_payload_offset < 0 ||
        level->structure2_payload.opaque_payload_size <= 0 ||
        opaque_end <= opaque_start) {
        return 0;
    }
    for (index = 0; index < level->structure2_texture_count; ++index) {
        const Nexus_V1_DgnStructure2Texture *descriptor =
            &level->structure2_textures[index];
        int palette_present = descriptor->palette_relative_offset != 0U;

        if (descriptor->width == 0U || descriptor->height == 0U ||
            descriptor->image_relative_offset < opaque_start ||
            descriptor->image_relative_offset >= opaque_end ||
            (palette_present &&
             (descriptor->palette_relative_offset < opaque_start ||
              descriptor->palette_relative_offset >= opaque_end))) {
            return 0;
        }
        ++out_receipt->image_payload_anchor_count;
        if (palette_present) ++out_receipt->palette_payload_anchor_count;
        else ++out_receipt->palette_payload_absent_count;
        if (descriptor->encoding == 0x0008U) {
            ++out_receipt->encoding_0x0008_count;
            if (palette_present)
                ++out_receipt->encoding_0x0008_palette_anchor_count;
            else
                ++out_receipt->encoding_0x0008_palette_absent_count;
        } else if (descriptor->encoding == 0x0028U) {
            ++out_receipt->encoding_0x0028_count;
            if (palette_present)
                ++out_receipt->encoding_0x0028_palette_anchor_count;
            else
                ++out_receipt->encoding_0x0028_palette_absent_count;
        } else {
            ++out_receipt->unobserved_encoding_count;
        }
    }
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = descriptor_receipt.source_bytes_fnv1a64;
    out_receipt->descriptor_count = level->structure2_texture_count;
    out_receipt->image_payload_anchors_complete =
        out_receipt->image_payload_anchor_count == out_receipt->descriptor_count;
    out_receipt->descriptor_format_classes_complete =
        out_receipt->encoding_0x0008_count +
            out_receipt->encoding_0x0028_count +
            out_receipt->unobserved_encoding_count ==
        out_receipt->descriptor_count;
    if (level->structure2_payload.material_or_image_data_proven &&
        out_receipt->image_payload_anchors_complete &&
        out_receipt->unobserved_encoding_count == 0) {
        out_receipt->pixel_span_proven = 1;
        out_receipt->palette_addressing_proven = 1;
        out_receipt->vdp1_format_proven = 1;
        out_receipt->decoder_permitted = 1;
    }
    out_receipt->valid = out_receipt->image_payload_anchors_complete &&
        out_receipt->descriptor_format_classes_complete;
    return out_receipt->valid;
}

uint32_t nexus_v1_saturn_15bit_to_rgba(uint16_t color)
{
    uint32_t r = (color & 0x001FU) << 3;
    uint32_t g = ((color >> 5) & 0x001FU) << 3;
    uint32_t b = ((color >> 10) & 0x001FU) << 3;
    r |= r >> 5;
    g |= g >> 5;
    b |= b >> 5;
    return (0xFFU << 24) | (r << 16) | (g << 8) | b;
}

int nexus_v1_current_level_decode_structure2_textures(
    const Nexus_V1_Engine *engine,
    Nexus_DMDFTextureSurface *out_surfaces, int max_surfaces,
    Nexus_V1_DgnStructure2TextureDecodeReceipt *out_receipt)
{
    Nexus_V1_DgnStructure2FormatEvidenceReceipt evidence;
    Nexus_V1_DgnStructure2TextureDecodeReceipt receipt;
    const Nexus_V1_Level *level;
    const uint8_t *payload;
    uint32_t structure2_offset;
    uint32_t dgn_size;
    int descriptor_index;

    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!engine || !out_surfaces || max_surfaces <= 0) return 0;
    if (nexus_v1_current_level_structure2_format_evidence_receipt(
            engine, &evidence) != 1 || !evidence.valid ||
        !evidence.decoder_permitted) return 0;

    level = &engine->current_level;
    if (level->structure2_texture_count <= 0 ||
        level->structure2_texture_count > max_surfaces ||
        !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    if ((uint64_t)engine->current_level_dgn_size > UINT32_MAX) return 0;
    dgn_size = (uint32_t)engine->current_level_dgn_size;
    if (dgn_size < 0x16U) return 0;

    structure2_offset = ((uint32_t)engine->current_level_dgn_data[0x14] << 8 |
        engine->current_level_dgn_data[0x15]) * NEXUS_DGN_BLOCK_SIZE;
    payload = engine->current_level_dgn_data;
    receipt.level_index = engine->game.current_level;
    receipt.descriptor_count = level->structure2_texture_count;

    for (descriptor_index = 0;
         descriptor_index < level->structure2_texture_count;
         ++descriptor_index) {
        const Nexus_V1_DgnStructure2Texture *tex =
            &level->structure2_textures[descriptor_index];
        Nexus_DMDFTextureSurface *surface = &out_surfaces[descriptor_index];
        uint32_t pixel_count = (uint32_t)tex->width * tex->height;
        uint8_t *indices;
        uint32_t i;

        memset(surface, 0, sizeof(*surface));
        if (pixel_count == 0U || pixel_count > 1024U * 1024U) continue;
        indices = (uint8_t *)malloc(pixel_count);
        if (!indices) continue;

        if (tex->encoding == 0x0008U) {
            uint32_t palette_bytes = 16U * 2U;
            uint32_t image_bytes = (pixel_count + 1U) / 2U;
            uint32_t image_abs;
            uint32_t palette_abs;
            const uint8_t *src;
            const uint8_t *pal_src;
            int c;

            /* DMWeb DMNDataFileDecoder.vbs reads both regions from the
             * Structure2 block. Validate them before forming pointers. */
            if (structure2_offset > dgn_size ||
                tex->palette_relative_offset > dgn_size - structure2_offset ||
                tex->image_relative_offset > dgn_size - structure2_offset ||
                palette_bytes > dgn_size - structure2_offset -
                    tex->palette_relative_offset ||
                image_bytes > dgn_size - structure2_offset -
                    tex->image_relative_offset) {
                free(indices);
                continue;
            }
            palette_abs = structure2_offset + tex->palette_relative_offset;
            image_abs = structure2_offset + tex->image_relative_offset;
            src = payload + image_abs;
            pal_src = payload + palette_abs;

            for (c = 0; c < 16; ++c) {
                uint16_t raw = (uint16_t)(pal_src[c * 2] << 8 |
                    pal_src[c * 2 + 1]);
                surface->palette[c] = nexus_v1_saturn_15bit_to_rgba(raw);
            }
            for (i = 0; i < image_bytes; ++i) {
                indices[i * 2] = (src[i] >> 4) & 0x0FU;
                indices[i * 2 + 1] = src[i] & 0x0FU;
            }
            surface->pixels = indices;
            surface->width = tex->width;
            surface->height = tex->height;
            surface->valid = 1;
            ++receipt.encoding_0x0008_decoded;
            ++receipt.decoded_count;
        } else if (tex->encoding == 0x0028U) {
            uint32_t palette_count = 1U;
            uint32_t image_bytes = pixel_count * 2U;
            uint32_t image_abs;
            const uint8_t *src;
            int overflow = 0;

            if (structure2_offset > dgn_size ||
                tex->image_relative_offset > dgn_size - structure2_offset ||
                image_bytes > dgn_size - structure2_offset -
                    tex->image_relative_offset) {
                free(indices);
                continue;
            }
            image_abs = structure2_offset + tex->image_relative_offset;
            src = payload + image_abs;

            memset(surface->palette, 0, sizeof(surface->palette));
            for (i = 0; i < pixel_count; ++i) {
                uint16_t raw = (uint16_t)(src[i * 2] << 8 | src[i * 2 + 1]);
                uint32_t rgba = nexus_v1_saturn_15bit_to_rgba(raw);
                uint32_t slot;
                int found = 0;

                /* DMWeb DMNDataFileDecoder.vbs Structure2 encoding 28h:
                 * every word is a direct 15-bit colour. Bit 15 is ignored;
                 * it is not a transparency flag for this resource. */
                for (slot = 1U; slot < palette_count; ++slot) {
                    if (surface->palette[slot] == rgba) {
                        indices[i] = (uint8_t)slot;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    if (palette_count >= 256U) {
                        overflow = 1;
                        break;
                    }
                    surface->palette[palette_count] = rgba;
                    indices[i] = (uint8_t)palette_count;
                    ++palette_count;
                }
            }
            if (overflow) {
                free(indices);
                ++receipt.palette_overflow_count;
                continue;
            }
            surface->pixels = indices;
            surface->width = tex->width;
            surface->height = tex->height;
            surface->valid = 1;
            ++receipt.encoding_0x0028_decoded;
            ++receipt.decoded_count;
        } else {
            free(indices);
        }
    }

    receipt.valid = receipt.decoded_count == receipt.descriptor_count;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

int nexus_v1_current_level_visit_structure2_payload_anchors(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2PayloadAnchorConsumer consumer, void *context,
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt *out_receipt)
{
    Nexus_V1_DgnActiveStructure2DescriptorReceipt descriptor_receipt;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt receipt;
    const Nexus_V1_Level *level;
    uint32_t anchors[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES * 2];
    uint32_t opaque_start, opaque_end;
    int anchor_count = 0;
    int packet_ordinal = 0;
    int descriptor_index, index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || nexus_v1_current_level_structure2_descriptor_receipt(
                       engine, &descriptor_receipt) != 1 ||
        !descriptor_receipt.valid) {
        *out_receipt = receipt;
        return 0;
    }
    level = &engine->current_level;
    if (level->structure2_texture_count <= 0 ||
        level->structure2_texture_count > NEXUS_DGN_MAX_STRUCTURE2_TEXTURES ||
        level->structure2_payload.opaque_payload_offset < 0 ||
        level->structure2_payload.opaque_payload_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    opaque_start = (uint32_t)level->structure2_payload.opaque_payload_offset;
    if ((uint32_t)level->structure2_payload.opaque_payload_size >
        UINT32_MAX - opaque_start) {
        *out_receipt = receipt;
        return 0;
    }
    opaque_end = opaque_start +
        (uint32_t)level->structure2_payload.opaque_payload_size;
    if (opaque_end <= opaque_start) {
        *out_receipt = receipt;
        return 0;
    }
    for (descriptor_index = 0;
         descriptor_index < level->structure2_texture_count;
         ++descriptor_index) {
        const Nexus_V1_DgnStructure2Texture *descriptor =
            &level->structure2_textures[descriptor_index];

        if (descriptor->width == 0U || descriptor->height == 0U ||
            descriptor->image_relative_offset < opaque_start ||
            descriptor->image_relative_offset >= opaque_end) {
            *out_receipt = receipt;
            return 0;
        }
        anchors[anchor_count++] = descriptor->image_relative_offset;
        if (descriptor->palette_relative_offset != 0U) {
            if (descriptor->palette_relative_offset < opaque_start ||
                descriptor->palette_relative_offset >= opaque_end) {
                *out_receipt = receipt;
                return 0;
            }
            anchors[anchor_count++] = descriptor->palette_relative_offset;
        }
    }
    receipt.level_index = engine->game.current_level;
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.source_bytes_fnv1a64 = descriptor_receipt.source_bytes_fnv1a64;
    receipt.descriptor_count = level->structure2_texture_count;
    receipt.anchor_count = anchor_count;
    for (descriptor_index = 0;
         descriptor_index < level->structure2_texture_count;
         ++descriptor_index) {
        const Nexus_V1_DgnStructure2Texture *descriptor =
            &level->structure2_textures[descriptor_index];
        int lane;

        for (lane = 0; lane < 2; ++lane) {
            Nexus_V1_DgnStructure2PayloadAnchorPacket packet;
            uint32_t anchor = lane == 0 ? descriptor->image_relative_offset :
                descriptor->palette_relative_offset;
            uint32_t next_anchor = opaque_end;
            int unique = 1;

            if (lane == 1 && anchor == 0U) continue;
            for (index = 0; index < anchor_count; ++index) {
                if (anchors[index] > anchor && anchors[index] < next_anchor)
                    next_anchor = anchors[index];
                if (anchors[index] == anchor && index < packet_ordinal)
                    unique = 0;
            }
            if (next_anchor <= anchor ||
                next_anchor - anchor > (uint32_t)INT_MAX ||
                receipt.candidate_interval_byte_count > INT_MAX -
                    (int)(next_anchor - anchor)) {
                *out_receipt = receipt;
                return 0;
            }
            memset(&packet, 0, sizeof(packet));
            packet.valid = 1;
            packet.level_index = receipt.level_index;
            packet.source_byte_count = receipt.source_byte_count;
            packet.source_bytes_fnv1a64 = receipt.source_bytes_fnv1a64;
            packet.descriptor_index = descriptor_index;
            packet.descriptor_offset = ((uint32_t)engine->current_level_dgn_data[0x14] << 8 | engine->current_level_dgn_data[0x15]) * NEXUS_DGN_BLOCK_SIZE + (uint32_t)descriptor_index * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
            packet.descriptor_length = NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
            packet.package_fnv1a64 = receipt.source_bytes_fnv1a64;
            if (packet.descriptor_offset > (uint32_t)packet.source_byte_count || packet.descriptor_length > (uint32_t)packet.source_byte_count - packet.descriptor_offset) { *out_receipt = receipt; return 0; }
            packet.descriptor_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(engine->current_level_dgn_data + packet.descriptor_offset, packet.descriptor_length);
            packet.palette_anchor = lane;
            packet.payload_anchor_offset = anchor;
            packet.next_anchor_offset = next_anchor;
            packet.candidate_byte_count = next_anchor - anchor;
            packet.descriptor = *descriptor;
            packet.no_draw_only = 1;
            packet.blocks_real_dgn_mesh_render = 1;
            if (consumer && consumer(context, &packet) != 0) {
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.consumed_anchor_count;
            if (lane == 0) ++receipt.image_anchor_count;
            else ++receipt.palette_anchor_count;
            if (unique) ++receipt.unique_anchor_count;
            else ++receipt.reused_anchor_count;
            receipt.candidate_interval_byte_count +=
                (int)packet.candidate_byte_count;
            ++packet_ordinal;
        }
    }
    receipt.valid = receipt.consumed_anchor_count == receipt.anchor_count &&
        receipt.image_anchor_count == receipt.descriptor_count &&
        receipt.image_anchor_count + receipt.palette_anchor_count ==
            receipt.anchor_count &&
        receipt.unique_anchor_count + receipt.reused_anchor_count ==
            receipt.anchor_count;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

typedef struct {
    int descriptor_index;
    uint32_t image_offset;
    uint32_t palette_offset;
    uint32_t image_next_anchor_offset;
    uint32_t image_candidate_byte_count;
    uint32_t palette_next_anchor_offset;
    uint32_t palette_candidate_byte_count;
    int image_anchor_found;
    int palette_anchor_found;
} Nexus_V1_StaticMaterialPayloadAnchorSearch;

static int nexus_v1_find_static_material_payload_anchor(
    void *context, const Nexus_V1_DgnStructure2PayloadAnchorPacket *packet)
{
    Nexus_V1_StaticMaterialPayloadAnchorSearch *search =
        (Nexus_V1_StaticMaterialPayloadAnchorSearch *)context;

    if (!search || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) {
        return -1;
    }
    if (packet->descriptor_index != search->descriptor_index ||
        !packet->package_fnv1a64 || packet->package_fnv1a64 != packet->source_bytes_fnv1a64 || !packet->descriptor_length || !packet->descriptor_fnv1a64 || packet->descriptor_offset > (uint32_t)packet->source_byte_count || packet->descriptor_length > (uint32_t)packet->source_byte_count - packet->descriptor_offset ||
        packet->candidate_byte_count == 0U ||
        packet->next_anchor_offset <= packet->payload_anchor_offset) {
        return 0;
    }
    if (!packet->palette_anchor &&
        packet->payload_anchor_offset == search->image_offset) {
        search->image_anchor_found = 1;
        search->image_next_anchor_offset = packet->next_anchor_offset;
        search->image_candidate_byte_count = packet->candidate_byte_count;
    }
    if (packet->palette_anchor && search->palette_offset != 0U &&
        packet->payload_anchor_offset == search->palette_offset) {
        search->palette_anchor_found = 1;
        search->palette_next_anchor_offset = packet->next_anchor_offset;
        search->palette_candidate_byte_count = packet->candidate_byte_count;
    }
    return 0;
}

int nexus_v1_engine_build_structure3_static_material_capture_target(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget *out_target)
{
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget descriptor_target;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt anchor_scene;
    Nexus_V1_StaticMaterialPayloadAnchorSearch anchor_search;
    const Nexus_V1_Level *level;
    const uint8_t *data;
    uint32_t entry_relative_offset;
    uint32_t face_relative_offset;
    uint16_t face_count;
    int descriptor_index;
    int structure2_byte_offset;
    int face_byte_offset;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->level_index = -1;
    out_target->no_draw_only = 1;
    if (!engine ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 ||
        structure3_entry_index >=
            (uint32_t)engine->current_level.structure3_directory.entry_count) {
        return 0;
    }
    level = &engine->current_level;
    data = engine->current_level_dgn_data;
    if (!data || level->structure3_payload.byte_offset < 0 ||
        level->structure3_payload.byte_size < 4 ||
        level->structure3_payload.byte_offset > engine->current_level_dgn_size ||
        level->structure3_payload.byte_size >
            engine->current_level_dgn_size - level->structure3_payload.byte_offset) {
        return 0;
    }
    entry_relative_offset = nexus_v1_dgn_read_be32(
        data + level->structure3_payload.byte_offset + 4 +
        structure3_entry_index * 4U);
    if (entry_relative_offset > (uint32_t)level->structure3_payload.byte_size ||
        (uint32_t)level->structure3_payload.byte_size - entry_relative_offset < 40U) {
        return 0;
    }
    face_count = (uint16_t)((unsigned int)
        data[level->structure3_payload.byte_offset + entry_relative_offset + 6] << 8 |
        data[level->structure3_payload.byte_offset + entry_relative_offset + 7]);
    face_relative_offset = nexus_v1_dgn_read_be32(
        data + level->structure3_payload.byte_offset + entry_relative_offset + 16);
    if (face_ordinal >= face_count ||
        face_relative_offset > (uint32_t)level->structure3_payload.byte_size ||
        (uint32_t)level->structure3_payload.byte_size - face_relative_offset < 12U ||
        face_ordinal > ((uint32_t)level->structure3_payload.byte_size -
                        face_relative_offset - 12U) / 12U) {
        return 0;
    }
    face_byte_offset = level->structure3_payload.byte_offset +
        (int)face_relative_offset + (int)face_ordinal * 12;
    if (face_byte_offset < level->structure3_payload.byte_offset ||
        face_byte_offset > engine->current_level_dgn_size - 12) {
        return 0;
    }
    if ((data[face_byte_offset + 8] & 0x40U) == 0U ||
        data[face_byte_offset + 10] != 0U) {
        return 0;
    }
    for (descriptor_index = 0;
         descriptor_index < level->structure2_texture_count;
         ++descriptor_index) {
        if (level->structure2_textures[descriptor_index].image_id ==
            data[face_byte_offset + 11]) break;
    }
    if (descriptor_index == level->structure2_texture_count ||
        nexus_v1_engine_build_structure2_descriptor_capture_target(
            engine, descriptor_index, &descriptor_target) != 1) {
        return 0;
    }
    structure2_byte_offset = descriptor_target.descriptor_byte_offset -
        descriptor_target.descriptor_index * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    if (structure2_byte_offset < 0 ||
        descriptor_target.descriptor.image_relative_offset <
            (uint32_t)(descriptor_target.opaque_payload_byte_offset -
                       structure2_byte_offset) ||
        descriptor_target.descriptor.image_relative_offset >=
            (uint32_t)(descriptor_target.opaque_payload_byte_offset -
                       structure2_byte_offset) +
            (uint32_t)descriptor_target.opaque_payload_byte_count ||
        structure2_byte_offset +
            (int)descriptor_target.descriptor.image_relative_offset >=
            engine->current_level_dgn_size) {
        return 0;
    }
    if (descriptor_target.descriptor.palette_relative_offset != 0U &&
        (descriptor_target.descriptor.palette_relative_offset <
             (uint32_t)(descriptor_target.opaque_payload_byte_offset -
                        structure2_byte_offset) ||
         descriptor_target.descriptor.palette_relative_offset >=
             (uint32_t)(descriptor_target.opaque_payload_byte_offset -
                        structure2_byte_offset) +
                 (uint32_t)descriptor_target.opaque_payload_byte_count ||
         structure2_byte_offset +
             (int)descriptor_target.descriptor.palette_relative_offset >=
             engine->current_level_dgn_size)) {
        return 0;
    }
    memset(&anchor_scene, 0, sizeof(anchor_scene));
    memset(&anchor_search, 0, sizeof(anchor_search));
    anchor_search.descriptor_index = descriptor_index;
    anchor_search.image_offset = descriptor_target.descriptor.image_relative_offset;
    anchor_search.palette_offset = descriptor_target.descriptor.palette_relative_offset;
    if (nexus_v1_current_level_visit_structure2_payload_anchors(
            engine, nexus_v1_find_static_material_payload_anchor, &anchor_search,
            &anchor_scene) != 1 || !anchor_scene.valid ||
        anchor_scene.level_index != engine->game.current_level ||
        anchor_scene.source_byte_count != engine->current_level_dgn_size ||
        anchor_scene.source_bytes_fnv1a64 !=
            descriptor_target.source_bytes_fnv1a64 ||
        !anchor_search.image_anchor_found ||
        (anchor_search.palette_offset != 0U &&
         !anchor_search.palette_anchor_found)) {
        return 0;
    }
    out_target->valid = 1;
    out_target->level_index = engine->game.current_level;
    out_target->source_byte_count = engine->current_level_dgn_size;
    out_target->source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    out_target->structure3_entry_index = structure3_entry_index;
    out_target->face_ordinal = face_ordinal;
    out_target->face_byte_offset = face_byte_offset;
    out_target->face_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + face_byte_offset, 12);
    out_target->face.vertex_indexes[0] =
        (uint16_t)(((unsigned int)data[face_byte_offset] << 8) |
                   data[face_byte_offset + 1]);
    out_target->face.vertex_indexes[1] =
        (uint16_t)(((unsigned int)data[face_byte_offset + 2] << 8) |
                   data[face_byte_offset + 3]);
    out_target->face.vertex_indexes[2] =
        (uint16_t)(((unsigned int)data[face_byte_offset + 4] << 8) |
                   data[face_byte_offset + 5]);
    out_target->face.vertex_indexes[3] =
        (uint16_t)(((unsigned int)data[face_byte_offset + 6] << 8) |
                   data[face_byte_offset + 7]);
    out_target->face.flags = data[face_byte_offset + 8];
    out_target->face.raw_byte_9 = data[face_byte_offset + 9];
    out_target->face.fill_selector =
        (uint16_t)(((unsigned int)data[face_byte_offset + 10] << 8) |
                   data[face_byte_offset + 11]);
    out_target->face.triangle =
        out_target->face.vertex_indexes[2] == out_target->face.vertex_indexes[3];
    out_target->static_texture_selector = data[face_byte_offset + 11];
    out_target->descriptor_target = descriptor_target;
    out_target->static_selector_descriptor_bound = 1;
    out_target->image_payload_byte_offset = structure2_byte_offset +
        (int)descriptor_target.descriptor.image_relative_offset;
    out_target->image_payload_anchor_bound = 1;
    out_target->image_payload_next_anchor_offset =
        anchor_search.image_next_anchor_offset;
    out_target->image_payload_candidate_byte_count =
        anchor_search.image_candidate_byte_count;
    out_target->image_payload_interval_bound = 1;
    if (descriptor_target.descriptor.palette_relative_offset != 0U) {
        out_target->palette_payload_byte_offset = structure2_byte_offset +
            (int)descriptor_target.descriptor.palette_relative_offset;
        out_target->palette_payload_anchor_bound = 1;
        out_target->palette_payload_next_anchor_offset =
            anchor_search.palette_next_anchor_offset;
        out_target->palette_payload_candidate_byte_count =
            anchor_search.palette_candidate_byte_count;
        out_target->palette_payload_interval_bound = 1;
    } else {
        out_target->palette_payload_byte_offset = -1;
    }
    out_target->capture_producer_required = 1;
    out_target->original_saturn_capture_required = 1;
    return 1;
}

int nexus_v1_engine_build_structure1a_structure3_material_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt owner_route;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt owner_target;
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;

    if (!out_target || !out_receipt) return -1;
    memset(out_target, 0, sizeof(*out_target));
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&owner_route, 0, sizeof(owner_route));
    memset(&owner_target, 0, sizeof(owner_target));
    memset(&material_target, 0, sizeof(material_target));
    out_target->level_index = -1;
    out_target->no_draw_only = 1;
    out_target->blocks_real_dgn_mesh_render = 1;
    if (!engine ||
        nexus_v1_engine_build_structure1a_structure3_capture_target(
            engine, topology_candidate_index, structure3_entry_index,
            structure3_face_ordinal, &owner_target, &owner_route) != 1 ||
        !owner_route.active_canonical_lev_bound ||
        !owner_route.material_plan_prepared ||
        !owner_route.topology_candidate_bound || !owner_route.target_built ||
        !owner_target.valid || owner_target.structure3_entry_mapping_proven ||
        !owner_target.no_draw_only || owner_target.fallback_visuals_permitted ||
        nexus_v1_engine_build_structure3_static_material_capture_target(
            engine, structure3_entry_index, structure3_face_ordinal,
            &material_target) != 1 || !material_target.valid ||
        !material_target.static_selector_descriptor_bound ||
        !material_target.image_payload_interval_bound ||
        material_target.image_payload_candidate_byte_count == 0U ||
        (material_target.descriptor_target.descriptor.palette_relative_offset != 0U &&
         (!material_target.palette_payload_interval_bound ||
          material_target.palette_payload_candidate_byte_count == 0U)) ||
        !material_target.no_draw_only || material_target.fallback_visuals_permitted) {
        *out_receipt = owner_route;
        return 0;
    }
    out_target->valid = 1;
    out_target->level_index = owner_target.level_index;
    out_target->owner_face_target = owner_target;
    out_target->material_target = material_target;
    out_target->owner_face_source_bound = 1;
    out_target->static_material_source_bound = 1;
    /* The two source inputs share a capture session but not a proved model map. */
    out_target->owner_to_entry_mapping_proven = 0;
    out_target->capture_producer_required = 1;
    out_target->original_saturn_capture_required = 1;
    *out_receipt = owner_route;
    return 1;
}

int nexus_v1_engine_write_structure1a_structure3_material_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *path,
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget target;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt receipt;
    char temporary_path[1024];
    FILE *file;
    int write_ok;

    if (!out_target || !out_receipt) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->level_index = -1;
    out_target->no_draw_only = 1;
    out_target->blocks_real_dgn_mesh_render = 1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!path || !path[0] ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path) ||
        nexus_v1_engine_build_structure1a_structure3_material_capture_target(
            engine, topology_candidate_index, structure3_entry_index,
            structure3_face_ordinal, &target, &receipt) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    file = fopen(temporary_path, "wb");
    if (!file) {
        *out_receipt = receipt;
        return 0;
    }
    write_ok = fprintf(file,
                       "%s\n"
                       "level=%d\nsource_bytes=%d\nsource_fnv1a64=%016llx\n"
                       "owner_x=%d\nowner_y=%d\nstructure1f_entry_index=%d\n"
                       "structure1f_family=%d\nstructure1f_tag=%u\n"
                       "structure1f_face_selector=%u\nstructure1a_index=%u\n"
                       "structure1a_kind=%u\nstructure3_model_index=%u\n"
                       "z_rotation=%u\nstructure3_payload_fnv1a32=%08x\n"
                       "structure3_entry_index=%u\nface_ordinal=%u\n"
                       "face_row_fnv1a32=%08x\nreferenced_vertex_rows_fnv1a32=%08x\n"
                       "normal_row_fnv1a32=%08x\nfill_selector=%u\n"
                       "descriptor_index=%d\ndescriptor_fnv1a64=%016llx\n"
                       "opaque_payload_fnv1a64=%016llx\n"
                       "image_anchor_offset=%u\nimage_next_anchor_offset=%u\n"
                       "image_candidate_byte_count=%u\nimage_candidate_fnv1a64=%016llx\n"
                       "palette_candidate_present=%d\npalette_anchor_offset=%u\n"
                       "palette_next_anchor_offset=%u\npalette_candidate_byte_count=%u\n"
                       "palette_candidate_fnv1a64=%016llx\n"
                       "capture_target_fnv1a64=%016llx\n"
                       "requested_observations=source-read,structure1a-owner,structure3-face,palette-state,vdp1-vram,vdp1-command,transform,culling\n"
                       "original_saturn_capture_required=1\nno_draw_only=1\n",
                       NEXUS_V1_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_MAGIC,
                       target.level_index,
                       target.material_target.source_byte_count,
                       (unsigned long long)target.material_target.source_bytes_fnv1a64,
                       target.owner_face_target.owner_x,
                       target.owner_face_target.owner_y,
                       target.owner_face_target.structure1f_entry_index,
                       (int)target.owner_face_target.structure1f_family,
                       target.owner_face_target.structure1f_tag,
                       target.owner_face_target.structure1f_face_selector,
                       target.owner_face_target.structure1a_index,
                       target.owner_face_target.structure1a_kind,
                       target.owner_face_target.structure3_model_index,
                       target.owner_face_target.z_rotation,
                       target.owner_face_target.structure3_payload_fnv1a32,
                       target.owner_face_target.face_target.candidate.entry_index,
                       target.owner_face_target.face_target.candidate.face_ordinal,
                       target.owner_face_target.face_target.candidate.face_row_fnv1a32,
                       target.owner_face_target.face_target.candidate
                           .referenced_vertex_rows_fnv1a32,
                       target.owner_face_target.face_target.candidate.normal_row_fnv1a32,
                       target.owner_face_target.face_target.candidate.fill_selector,
                       target.material_target.descriptor_target.descriptor_index,
                       (unsigned long long)target.material_target.descriptor_target
                           .descriptor_bytes_fnv1a64,
                       (unsigned long long)target.material_target.descriptor_target
                           .opaque_payload_fnv1a64,
                       target.material_target.descriptor_target
                           .image_payload_anchor_offset,
                       target.material_target.descriptor_target
                           .image_payload_next_anchor_offset,
                       target.material_target.descriptor_target
                           .image_payload_candidate_byte_count,
                       (unsigned long long)target.material_target.descriptor_target
                           .image_payload_candidate_fnv1a64,
                       target.material_target.descriptor_target
                           .palette_payload_candidate_bound,
                       target.material_target.descriptor_target
                           .palette_payload_anchor_offset,
                       target.material_target.descriptor_target
                           .palette_payload_next_anchor_offset,
                       target.material_target.descriptor_target
                           .palette_payload_candidate_byte_count,
                       (unsigned long long)target.material_target.descriptor_target
                           .palette_payload_candidate_fnv1a64,
                       (unsigned long long)
                           nexus_v1_owner_material_capture_target_fnv1a64(
                               &target)) > 0;
    if (fclose(file) != 0) write_ok = 0;
    if (!write_ok || rename(temporary_path, path) != 0) {
        remove(temporary_path);
        *out_receipt = receipt;
        return 0;
    }
    receipt.target_written = 1;
    *out_target = target;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_structure3_package_geometry_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3PackageGeometryPacket *out_packet)
{
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;
    Nexus_V1_DgnStructure3MeshEntryReceipt mesh_receipt;
    Nexus_V1_DgnStructure3Vector *vertices = NULL;
    Nexus_V1_DgnStructure3Vector *normals = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    const Nexus_V1_DgnStructure3Face *face;
    int slot_count;
    int result = 0;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->level_index = -1;
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine || nexus_v1_engine_build_structure3_static_material_capture_target(
                       engine, structure3_entry_index, face_ordinal,
                       &material_target) != 1 ||
        !material_target.valid) {
        return 0;
    }

    memset(&mesh_receipt, 0, sizeof(mesh_receipt));
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, NULL, 0, NULL, 0, NULL, 0,
            &mesh_receipt) != -1 || !mesh_receipt.source_identity_valid ||
        mesh_receipt.vertex_count <= 0 || mesh_receipt.face_count <= 0 ||
        mesh_receipt.normal_count < mesh_receipt.face_count ||
        face_ordinal >= (uint32_t)mesh_receipt.face_count) {
        return 0;
    }
    vertices = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.vertex_count * sizeof(*vertices));
    faces = (Nexus_V1_DgnStructure3Face *)malloc(
        (size_t)mesh_receipt.face_count * sizeof(*faces));
    normals = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.normal_count * sizeof(*normals));
    if (!vertices || !faces || !normals) goto cleanup;
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, vertices,
            mesh_receipt.vertex_count, faces, mesh_receipt.face_count, normals,
            mesh_receipt.normal_count, &mesh_receipt) != 0 ||
        !mesh_receipt.valid || !mesh_receipt.source_identity_valid ||
        face_ordinal >= (uint32_t)mesh_receipt.face_count) {
        goto cleanup;
    }
    face = &faces[face_ordinal];
    slot_count = face->triangle ? 3 : 4;
    if (slot_count < 3 || slot_count > 4 ||
        face->vertex_indexes[0] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[1] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[2] >= (uint16_t)mesh_receipt.vertex_count ||
        (slot_count == 4 &&
         face->vertex_indexes[3] >= (uint16_t)mesh_receipt.vertex_count) ||
        face->vertex_indexes[0] != material_target.face.vertex_indexes[0] ||
        face->vertex_indexes[1] != material_target.face.vertex_indexes[1] ||
        face->vertex_indexes[2] != material_target.face.vertex_indexes[2] ||
        face->vertex_indexes[3] != material_target.face.vertex_indexes[3] ||
        face->flags != material_target.face.flags ||
        face->raw_byte_9 != material_target.face.raw_byte_9 ||
        face->fill_selector != material_target.face.fill_selector ||
        face->triangle != material_target.face.triangle) {
        goto cleanup;
    }

    out_packet->valid = 1;
    out_packet->source_geometry_bound = 1;
    out_packet->material_descriptor_bound = 1;
    out_packet->level_index = material_target.level_index;
    out_packet->source_byte_count = material_target.source_byte_count;
    out_packet->source_bytes_fnv1a64 = material_target.source_bytes_fnv1a64;
    out_packet->structure3_entry_index = structure3_entry_index;
    out_packet->face_ordinal = face_ordinal;
    out_packet->face_offset = (uint32_t)material_target.face_byte_offset;
    out_packet->face_length = 12U;
    out_packet->face_fnv1a64 = material_target.face_bytes_fnv1a64;
    out_packet->package_fnv1a64 = material_target.source_bytes_fnv1a64;
    out_packet->descriptor_fnv1a64 = material_target.descriptor_target.descriptor_bytes_fnv1a64;
    out_packet->image_offset = material_target.image_payload_byte_offset;
    out_packet->image_length = material_target.image_payload_candidate_byte_count;
    out_packet->palette_offset = material_target.palette_payload_byte_offset;
    out_packet->palette_length = material_target.palette_payload_candidate_byte_count;
    out_packet->face = *face;
    out_packet->vertex_slot_count = slot_count;
    out_packet->vertices[0] = vertices[face->vertex_indexes[0]];
    out_packet->vertices[1] = vertices[face->vertex_indexes[1]];
    out_packet->vertices[2] = vertices[face->vertex_indexes[2]];
    if (slot_count == 4)
        out_packet->vertices[3] = vertices[face->vertex_indexes[3]];
    out_packet->normal = normals[face_ordinal];
    out_packet->material_target = material_target;
    {
        uint16_t fill = face->fill_selector;
        int desc_idx = fill & 0x00FFU;
        if ((fill & 0xFF00U) == 0x0000U &&
            desc_idx >= 0 && desc_idx < engine->structure2_surface_count &&
            engine->structure2_surfaces[desc_idx].valid) {
            out_packet->texture_surface_index = desc_idx;
            out_packet->texture_surface_valid = 1;
        }
    }
    result = 1;

cleanup:
    free(normals);
    free(faces);
    free(vertices);
    return result;
}

int nexus_v1_current_level_visit_structure3_package_geometry(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3PackageGeometryConsumer consumer, void *context,
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded) {
        *out_receipt = receipt;
        return 0;
    }
    /* The face-material receipt API requires an output object. Keep the
     * source admission explicit rather than treating a loaded level alone as
     * permission to traverse package geometry. */
    {
        Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
        if (nexus_v1_current_level_structure3_face_material_receipt(
                engine, &material_receipt) != 1 || !material_receipt.valid) {
            *out_receipt = receipt;
            return 0;
        }
        receipt.level_index = material_receipt.level_index;
        receipt.source_byte_count = material_receipt.source_byte_count;
        receipt.source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    }
    receipt.structure3_entry_count =
        engine->current_level.structure3_directory.entry_count;
    if (receipt.structure3_entry_count <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    for (entry_index = 0; entry_index < receipt.structure3_entry_count;
         ++entry_index) {
        int face_ordinal;
        int face_count =
            engine->current_level.structure3_entry_face_counts[entry_index];
        if (face_count < 0) {
            *out_receipt = receipt;
            return 0;
        }
        for (face_ordinal = 0; face_ordinal < face_count; ++face_ordinal) {
            Nexus_V1_DgnStructure3PackageGeometryPacket packet;
            ++receipt.candidate_face_count;
            if (nexus_v1_current_level_structure3_package_geometry_packet(
                    engine, (uint32_t)entry_index, (uint32_t)face_ordinal,
                    &packet) != 1) {
                continue;
            }
            if (!packet.valid || !packet.source_geometry_bound ||
                !packet.material_descriptor_bound || !packet.no_draw_only ||
                packet.fallback_visuals_permitted ||
                !packet.blocks_real_dgn_mesh_render) {
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.static_material_face_count;
            if (consumer && consumer(context, &packet) != 0) {
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.consumed_face_count;
        }
    }
    receipt.valid = 1;
    receipt.complete =
        receipt.consumed_face_count == receipt.static_material_face_count;
    *out_receipt = receipt;
    return receipt.complete ? 1 : 0;
}

int nexus_v1_current_level_structure3_animated_material_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3AnimatedMaterialPacket *out_packet)
{
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    Nexus_V1_DgnStructure3MeshEntryReceipt mesh_receipt;
    Nexus_V1_DgnStructure3Vector *vertices = NULL;
    Nexus_V1_DgnStructure3Vector *normals = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget descriptor_target;
    const Nexus_V1_DgnStructure1GEntry *animation = NULL;
    const Nexus_V1_DgnStructure3Face *face;
    const uint8_t *data;
    uint32_t entry_relative_offset;
    uint32_t face_relative_offset;
    uint16_t source_face_count;
    int descriptor_index;
    int animation_index;
    int face_byte_offset;
    int slot_count;
    int result = 0;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->level_index = -1;
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 ||
        structure3_entry_index >=
            (uint32_t)engine->current_level.structure3_directory.entry_count ||
        !engine->current_level_dgn_data) {
        return 0;
    }
    data = engine->current_level_dgn_data;
    entry_relative_offset = nexus_v1_dgn_read_be32(
        data + engine->current_level.structure3_payload.byte_offset + 4 +
        structure3_entry_index * 4U);
    if (entry_relative_offset >
            (uint32_t)engine->current_level.structure3_payload.byte_size ||
        (uint32_t)engine->current_level.structure3_payload.byte_size -
            entry_relative_offset < 40U) {
        return 0;
    }
    source_face_count = (uint16_t)((unsigned int)
        data[engine->current_level.structure3_payload.byte_offset +
             entry_relative_offset + 6] << 8 |
        data[engine->current_level.structure3_payload.byte_offset +
             entry_relative_offset + 7]);
    face_relative_offset = nexus_v1_dgn_read_be32(
        data + engine->current_level.structure3_payload.byte_offset +
        entry_relative_offset + 16);
    if (face_ordinal >= source_face_count ||
        face_relative_offset >
            (uint32_t)engine->current_level.structure3_payload.byte_size ||
        (uint32_t)engine->current_level.structure3_payload.byte_size -
            face_relative_offset < 12U ||
        face_ordinal > ((uint32_t)engine->current_level.structure3_payload.byte_size -
                        face_relative_offset - 12U) / 12U) {
        return 0;
    }
    face_byte_offset = engine->current_level.structure3_payload.byte_offset +
        (int)face_relative_offset + (int)face_ordinal * 12;
    if (face_byte_offset < engine->current_level.structure3_payload.byte_offset ||
        face_byte_offset > engine->current_level_dgn_size - 12 ||
        (data[face_byte_offset + 8] & 0x40U) == 0U ||
        (data[face_byte_offset + 10] & 0xffU) != 0x08U) {
        return 0;
    }
    for (animation_index = 0;
         animation_index < engine->current_level.structure1g_entry_count;
         ++animation_index) {
        if (engine->current_level.structure1g_entries[animation_index]
                .animation_id == data[face_byte_offset + 11]) {
            animation = &engine->current_level.structure1g_entries[animation_index];
            break;
        }
    }
    if (!animation || !animation->first_structure2_image_valid ||
        animation->sequence_instruction_count <= 0 ||
        animation->structure2_image_instruction_unbound_count != 0) {
        return 0;
    }
    for (descriptor_index = 0;
         descriptor_index < engine->current_level.structure2_texture_count;
         ++descriptor_index) {
        if (engine->current_level.structure2_textures[descriptor_index].image_id ==
            animation->first_structure2_image_id) break;
    }
    if (descriptor_index == engine->current_level.structure2_texture_count ||
        nexus_v1_engine_build_structure2_descriptor_capture_target(
            engine, descriptor_index, &descriptor_target) != 1) {
        return 0;
    }
    memset(&mesh_receipt, 0, sizeof(mesh_receipt));
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, NULL, 0, NULL, 0, NULL, 0,
            &mesh_receipt) != -1 || !mesh_receipt.source_identity_valid ||
        mesh_receipt.vertex_count <= 0 || mesh_receipt.face_count <= 0 ||
        mesh_receipt.normal_count < mesh_receipt.face_count ||
        face_ordinal >= (uint32_t)mesh_receipt.face_count) {
        return 0;
    }
    vertices = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.vertex_count * sizeof(*vertices));
    faces = (Nexus_V1_DgnStructure3Face *)malloc(
        (size_t)mesh_receipt.face_count * sizeof(*faces));
    normals = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.normal_count * sizeof(*normals));
    if (!vertices || !faces || !normals) goto cleanup;
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, vertices,
            mesh_receipt.vertex_count, faces, mesh_receipt.face_count, normals,
            mesh_receipt.normal_count, &mesh_receipt) != 0 ||
        !mesh_receipt.valid || !mesh_receipt.source_identity_valid) {
        goto cleanup;
    }
    face = &faces[face_ordinal];
    slot_count = face->triangle ? 3 : 4;
    if (slot_count < 3 || slot_count > 4 ||
        face->vertex_indexes[0] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[1] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[2] >= (uint16_t)mesh_receipt.vertex_count ||
        (slot_count == 4 &&
         face->vertex_indexes[3] >= (uint16_t)mesh_receipt.vertex_count) ||
        face->flags != data[face_byte_offset + 8] ||
        face->fill_selector != (uint16_t)(0x0800U | animation->animation_id)) {
        goto cleanup;
    }
    out_packet->valid = 1;
    out_packet->source_geometry_bound = 1;
    out_packet->animation_declaration_bound = 1;
    out_packet->first_descriptor_bound = 1;
    out_packet->level_index = engine->game.current_level;
    out_packet->source_byte_count = engine->current_level_dgn_size;
    out_packet->source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    out_packet->structure3_entry_index = structure3_entry_index;
    out_packet->face_ordinal = face_ordinal;
    out_packet->face_byte_offset = face_byte_offset;
    out_packet->face_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + face_byte_offset, 12);
    out_packet->face = *face;
    out_packet->vertex_slot_count = slot_count;
    out_packet->vertices[0] = vertices[face->vertex_indexes[0]];
    out_packet->vertices[1] = vertices[face->vertex_indexes[1]];
    out_packet->vertices[2] = vertices[face->vertex_indexes[2]];
    if (slot_count == 4)
        out_packet->vertices[3] = vertices[face->vertex_indexes[3]];
    out_packet->normal = normals[face_ordinal];
    out_packet->structure1g_entry_index = animation_index;
    out_packet->animation_id = animation->animation_id;
    out_packet->first_image_index = animation->first_image_index;
    out_packet->first_structure2_image_id = animation->first_structure2_image_id;
    out_packet->sequence_word_offset = animation->sequence_word_offset;
    out_packet->sequence_instruction_count = animation->sequence_instruction_count;
    out_packet->image_instruction_count = animation->image_instruction_count;
    out_packet->goto_instruction_count = animation->goto_instruction_count;
    out_packet->first_descriptor_target = descriptor_target;
    result = 1;

cleanup:
    free(normals);
    free(faces);
    free(vertices);
    return result;
}

int nexus_v1_current_level_visit_structure3_animated_materials(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialConsumer consumer, void *context,
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt receipt;
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    const uint8_t *data;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 || !material_receipt.valid ||
        !engine->current_level_dgn_data) {
        *out_receipt = receipt;
        return 0;
    }
    data = engine->current_level_dgn_data;
    receipt.level_index = material_receipt.level_index;
    receipt.source_byte_count = material_receipt.source_byte_count;
    receipt.source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    receipt.structure3_entry_count =
        engine->current_level.structure3_directory.entry_count;
    if (receipt.structure3_entry_count <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    for (entry_index = 0; entry_index < receipt.structure3_entry_count;
         ++entry_index) {
        uint32_t entry_relative_offset;
        uint32_t face_relative_offset;
        uint16_t face_count;
        int face_ordinal;

        entry_relative_offset = nexus_v1_dgn_read_be32(
            data + engine->current_level.structure3_payload.byte_offset + 4 +
            entry_index * 4);
        if (entry_relative_offset >
                (uint32_t)engine->current_level.structure3_payload.byte_size ||
            (uint32_t)engine->current_level.structure3_payload.byte_size -
                entry_relative_offset < 40U) {
            *out_receipt = receipt;
            return 0;
        }
        face_count = (uint16_t)((unsigned int)
            data[engine->current_level.structure3_payload.byte_offset +
                 entry_relative_offset + 6] << 8 |
            data[engine->current_level.structure3_payload.byte_offset +
                 entry_relative_offset + 7]);
        face_relative_offset = nexus_v1_dgn_read_be32(
            data + engine->current_level.structure3_payload.byte_offset +
            entry_relative_offset + 16);
        if (face_count != engine->current_level
                              .structure3_entry_face_counts[entry_index] ||
            face_relative_offset >
                (uint32_t)engine->current_level.structure3_payload.byte_size ||
            (uint32_t)engine->current_level.structure3_payload.byte_size -
                face_relative_offset < (uint32_t)face_count * 12U) {
            *out_receipt = receipt;
            return 0;
        }
        for (face_ordinal = 0; face_ordinal < (int)face_count;
             ++face_ordinal) {
            const uint8_t *face = data +
                engine->current_level.structure3_payload.byte_offset +
                face_relative_offset + face_ordinal * 12;
            Nexus_V1_DgnStructure3AnimatedMaterialPacket packet;

            ++receipt.candidate_face_count;
            if ((face[8] & 0x40U) == 0U || face[10] != 0x08U) continue;
            ++receipt.animated_face_count;
            if (nexus_v1_current_level_structure3_animated_material_packet(
                    engine, (uint32_t)entry_index, (uint32_t)face_ordinal,
                    &packet) != 1 || !packet.valid ||
                !packet.source_geometry_bound ||
                !packet.animation_declaration_bound ||
                !packet.first_descriptor_bound ||
                packet.animation_execution_permitted ||
                packet.transform_semantics_proven ||
                packet.pixel_palette_vdp1_semantics_proven ||
                packet.decoder_permitted || !packet.no_draw_only ||
                packet.fallback_visuals_permitted ||
                !packet.blocks_real_dgn_mesh_render) {
                *out_receipt = receipt;
                return 0;
            }
            if (consumer && consumer(context, &packet) != 0) {
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.consumed_face_count;
        }
    }
    /* The typed face-material receipt is the cross-check that every 08xx
     * source row entered exactly this traversal, not merely any face count. */
    if (receipt.candidate_face_count != material_receipt.faces.face_count ||
        receipt.animated_face_count !=
            material_receipt.materials.animated_texture_selector_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.complete =
        receipt.consumed_face_count == receipt.animated_face_count;
    *out_receipt = receipt;
    return receipt.complete ? 1 : 0;
}

static int nexus_v1_current_level_structure3_animated_material_image_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal, uint32_t image_instruction_ordinal,
    Nexus_V1_DgnStructure3AnimatedMaterialImagePacket *out_packet)
{
    Nexus_V1_DgnStructure3AnimatedMaterialPacket face_packet;
    Nexus_V1_DgnStructure1Layout layout;
    const Nexus_V1_DgnStructure1GEntry *animation;
    const uint8_t *sequence;
    int sequence_offset;
    int cursor;
    uint32_t found = 0;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->level_index = -1;
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine ||
        nexus_v1_current_level_structure3_animated_material_packet(
            engine, structure3_entry_index, face_ordinal, &face_packet) != 1 ||
        !face_packet.valid ||
        face_packet.structure1g_entry_index < 0 ||
        face_packet.structure1g_entry_index >=
            engine->current_level.structure1g_entry_count ||
        image_instruction_ordinal >= (uint32_t)face_packet.image_instruction_count ||
        !engine->current_level_dgn_data ||
        nexus_v1_dgn_structure1_layout(&layout, engine->current_level_dgn_data,
                                       engine->current_level_dgn_size) != 0 ||
        !layout.structure1g.valid) {
        return 0;
    }
    animation = &engine->current_level.structure1g_entries[
        face_packet.structure1g_entry_index];
    sequence_offset = layout.structure1g.animation_data_relative_offset +
        (int)animation->sequence_word_offset * 4;
    if (sequence_offset < 0 || sequence_offset > layout.structure1g.size - 4) {
        return 0;
    }
    sequence = engine->current_level_dgn_data + layout.structure1_offset +
        layout.structure1g.relative_offset;
    for (cursor = sequence_offset; cursor <= layout.structure1g.size - 4;
         cursor += 4) {
        const uint16_t instruction = nexus_v1_dgn_read_be16(sequence + cursor);
        uint16_t local_image_id;
        int descriptor_index;

        if (instruction == 0xffffU) break;
        if (instruction == 0xfffeU) continue;
        if (instruction < NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) return 0;
        if (found++ != image_instruction_ordinal) continue;
        local_image_id = (uint16_t)(instruction -
            NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX);
        for (descriptor_index = 0;
             descriptor_index < engine->current_level.structure2_texture_count;
             ++descriptor_index) {
            if (engine->current_level.structure2_textures[descriptor_index]
                    .image_id == local_image_id) break;
        }
        if (descriptor_index == engine->current_level.structure2_texture_count ||
            nexus_v1_engine_build_structure2_descriptor_capture_target(
                engine, descriptor_index, &out_packet->descriptor_target) != 1) {
            return 0;
        }
        out_packet->valid = 1;
        out_packet->source_geometry_bound = 1;
        out_packet->animation_declaration_bound = 1;
        out_packet->descriptor_bound = 1;
        out_packet->level_index = face_packet.level_index;
        out_packet->structure3_entry_index = structure3_entry_index;
        out_packet->face_ordinal = face_ordinal;
        out_packet->structure1g_entry_index =
            face_packet.structure1g_entry_index;
        out_packet->image_instruction_ordinal = image_instruction_ordinal;
        out_packet->instruction_byte_offset = layout.structure1_offset +
            layout.structure1g.relative_offset + cursor;
        out_packet->instruction_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
            sequence + cursor, 4);
        out_packet->global_image_index = instruction;
        out_packet->structure2_image_id = local_image_id;
        return 1;
    }
    return 0;
}

typedef struct {
    const Nexus_V1_Engine *engine;
    Nexus_V1_DgnStructure3AnimatedMaterialImageConsumer consumer;
    void *consumer_context;
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt *receipt;
} Nexus_V1_AnimatedMaterialImageVisitor;

static int nexus_v1_visit_structure3_animated_material_images_for_face(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialPacket *face)
{
    Nexus_V1_AnimatedMaterialImageVisitor *visitor =
        (Nexus_V1_AnimatedMaterialImageVisitor *)context;
    uint32_t image_ordinal;

    if (!visitor || !face || !face->valid || face->image_instruction_count <= 0)
        return -1;
    ++visitor->receipt->animated_face_count;
    visitor->receipt->declared_image_instruction_count +=
        face->image_instruction_count;
    for (image_ordinal = 0;
         image_ordinal < (uint32_t)face->image_instruction_count;
         ++image_ordinal) {
        Nexus_V1_DgnStructure3AnimatedMaterialImagePacket image;

        if (nexus_v1_current_level_structure3_animated_material_image_packet(
                visitor->engine, face->structure3_entry_index,
                face->face_ordinal, image_ordinal, &image) != 1 ||
            !image.valid || !image.source_geometry_bound ||
            !image.animation_declaration_bound || !image.descriptor_bound ||
            image.animation_execution_permitted ||
            image.pixel_palette_vdp1_semantics_proven || image.decoder_permitted ||
            !image.no_draw_only || image.fallback_visuals_permitted ||
            !image.blocks_real_dgn_mesh_render) {
            return -1;
        }
        if (visitor->consumer &&
            visitor->consumer(visitor->consumer_context, &image) != 0) {
            return -1;
        }
        ++visitor->receipt->consumed_image_instruction_count;
    }
    return 0;
}

int nexus_v1_current_level_visit_structure3_animated_material_images(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialImageConsumer consumer,
    void *context,
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt receipt;
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt face_scene;
    Nexus_V1_AnimatedMaterialImageVisitor visitor;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&face_scene, 0, sizeof(face_scene));
    memset(&visitor, 0, sizeof(visitor));
    visitor.engine = engine;
    visitor.consumer = consumer;
    visitor.consumer_context = context;
    visitor.receipt = &receipt;
    if (!engine || nexus_v1_current_level_visit_structure3_animated_materials(
                       engine,
                       nexus_v1_visit_structure3_animated_material_images_for_face,
                       &visitor, &face_scene) != 1 || !face_scene.valid ||
        !face_scene.complete ||
        receipt.animated_face_count != face_scene.animated_face_count ||
        receipt.declared_image_instruction_count <= 0 ||
        receipt.consumed_image_instruction_count !=
            receipt.declared_image_instruction_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = face_scene.level_index;
    receipt.source_byte_count = face_scene.source_byte_count;
    receipt.source_bytes_fnv1a64 = face_scene.source_bytes_fnv1a64;
    receipt.complete = 1;
    *out_receipt = receipt;
    return 1;
}

typedef struct {
    int descriptor_index;
    uint32_t image_offset;
    uint32_t palette_offset;
    int image_anchor_found;
    int palette_anchor_found;
} Nexus_V1_AnimatedImagePayloadAnchorSearch;

static int nexus_v1_find_animated_image_payload_anchor(
    void *context, const Nexus_V1_DgnStructure2PayloadAnchorPacket *packet)
{
    Nexus_V1_AnimatedImagePayloadAnchorSearch *search =
        (Nexus_V1_AnimatedImagePayloadAnchorSearch *)context;

    if (!search || !packet || !packet->valid || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) {
        return -1;
    }
    if (packet->descriptor_index != search->descriptor_index) {
        return 0;
    }
    if (!packet->palette_anchor &&
        packet->payload_anchor_offset == search->image_offset) {
        search->image_anchor_found = 1;
    }
    if (packet->palette_anchor && search->palette_offset != 0U &&
        packet->payload_anchor_offset == search->palette_offset) {
        search->palette_anchor_found = 1;
    }
    return 0;
}

typedef struct {
    const Nexus_V1_Engine *engine;
    Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt *receipt;
} Nexus_V1_AnimatedMaterialPayloadVisitor;

static int nexus_v1_visit_structure3_animated_material_payload_anchor(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialImagePacket *image)
{
    Nexus_V1_AnimatedMaterialPayloadVisitor *visitor =
        (Nexus_V1_AnimatedMaterialPayloadVisitor *)context;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt anchor_scene;
    Nexus_V1_AnimatedImagePayloadAnchorSearch search;

    if (!visitor || !visitor->engine || !visitor->receipt || !image ||
        !image->valid || !image->source_geometry_bound ||
        !image->animation_declaration_bound || !image->descriptor_bound ||
        !image->descriptor_target.valid ||
        image->animation_execution_permitted ||
        image->pixel_palette_vdp1_semantics_proven || image->decoder_permitted ||
        !image->no_draw_only || image->fallback_visuals_permitted ||
        !image->blocks_real_dgn_mesh_render) {
        return -1;
    }
    memset(&anchor_scene, 0, sizeof(anchor_scene));
    memset(&search, 0, sizeof(search));
    search.descriptor_index = image->descriptor_target.descriptor_index;
    search.image_offset = image->descriptor_target.descriptor.image_relative_offset;
    search.palette_offset = image->descriptor_target.descriptor.palette_relative_offset;
    if (nexus_v1_current_level_visit_structure2_payload_anchors(
            visitor->engine, nexus_v1_find_animated_image_payload_anchor,
            &search, &anchor_scene) != 1 || !anchor_scene.valid ||
        anchor_scene.level_index != image->level_index ||
        anchor_scene.source_byte_count != image->descriptor_target.source_byte_count ||
        anchor_scene.source_bytes_fnv1a64 !=
            image->descriptor_target.source_bytes_fnv1a64 ||
        !search.image_anchor_found ||
        (search.palette_offset != 0U && !search.palette_anchor_found)) {
        return -1;
    }
    ++visitor->receipt->declared_image_instruction_count;
    ++visitor->receipt->consumed_image_instruction_count;
    ++visitor->receipt->image_payload_anchor_count;
    if (search.palette_offset != 0U)
        ++visitor->receipt->palette_payload_anchor_count;
    return 0;
}

int nexus_v1_current_level_visit_structure3_animated_material_payload_anchors(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt receipt;
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt image_scene;
    Nexus_V1_AnimatedMaterialPayloadVisitor visitor;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&image_scene, 0, sizeof(image_scene));
    memset(&visitor, 0, sizeof(visitor));
    visitor.engine = engine;
    visitor.receipt = &receipt;
    if (!engine ||
        nexus_v1_current_level_visit_structure3_animated_material_images(
            engine, nexus_v1_visit_structure3_animated_material_payload_anchor,
            &visitor, &image_scene) != 1 || !image_scene.valid ||
        !image_scene.complete || image_scene.declared_image_instruction_count <= 0 ||
        receipt.declared_image_instruction_count !=
            image_scene.declared_image_instruction_count ||
        receipt.consumed_image_instruction_count !=
            receipt.declared_image_instruction_count ||
        receipt.image_payload_anchor_count !=
            receipt.declared_image_instruction_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = image_scene.level_index;
    receipt.source_byte_count = image_scene.source_byte_count;
    receipt.source_bytes_fnv1a64 = image_scene.source_bytes_fnv1a64;
    receipt.complete = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_structure3_untextured_face_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3UntexturedFacePacket *out_packet)
{
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    Nexus_V1_DgnStructure3MeshEntryReceipt mesh_receipt;
    Nexus_V1_DgnStructure3Vector *vertices = NULL;
    Nexus_V1_DgnStructure3Vector *normals = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    const Nexus_V1_DgnStructure3Face *face;
    const uint8_t *data;
    uint32_t entry_relative_offset;
    uint32_t face_relative_offset;
    uint16_t source_face_count;
    int face_byte_offset;
    int slot_count;
    int result = 0;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->level_index = -1;
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 ||
        structure3_entry_index >=
            (uint32_t)engine->current_level.structure3_directory.entry_count ||
        !engine->current_level_dgn_data) {
        return 0;
    }
    data = engine->current_level_dgn_data;
    entry_relative_offset = nexus_v1_dgn_read_be32(
        data + engine->current_level.structure3_payload.byte_offset + 4 +
        structure3_entry_index * 4U);
    if (entry_relative_offset >
            (uint32_t)engine->current_level.structure3_payload.byte_size ||
        (uint32_t)engine->current_level.structure3_payload.byte_size -
            entry_relative_offset < 40U) return 0;
    source_face_count = (uint16_t)((unsigned int)
        data[engine->current_level.structure3_payload.byte_offset +
             entry_relative_offset + 6] << 8 |
        data[engine->current_level.structure3_payload.byte_offset +
             entry_relative_offset + 7]);
    face_relative_offset = nexus_v1_dgn_read_be32(
        data + engine->current_level.structure3_payload.byte_offset +
        entry_relative_offset + 16);
    if (face_ordinal >= source_face_count ||
        face_relative_offset >
            (uint32_t)engine->current_level.structure3_payload.byte_size ||
        (uint32_t)engine->current_level.structure3_payload.byte_size -
            face_relative_offset < 12U ||
        face_ordinal > ((uint32_t)engine->current_level.structure3_payload.byte_size -
                        face_relative_offset - 12U) / 12U) return 0;
    face_byte_offset = engine->current_level.structure3_payload.byte_offset +
        (int)face_relative_offset + (int)face_ordinal * 12;
    if (face_byte_offset < engine->current_level.structure3_payload.byte_offset ||
        face_byte_offset > engine->current_level_dgn_size - 12 ||
        (data[face_byte_offset + 8] & 0x40U) != 0U) return 0;

    memset(&mesh_receipt, 0, sizeof(mesh_receipt));
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, NULL, 0, NULL, 0, NULL, 0,
            &mesh_receipt) != -1 || !mesh_receipt.source_identity_valid ||
        mesh_receipt.vertex_count <= 0 || mesh_receipt.face_count <= 0 ||
        mesh_receipt.normal_count < mesh_receipt.face_count ||
        face_ordinal >= (uint32_t)mesh_receipt.face_count) return 0;
    vertices = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.vertex_count * sizeof(*vertices));
    faces = (Nexus_V1_DgnStructure3Face *)malloc(
        (size_t)mesh_receipt.face_count * sizeof(*faces));
    normals = (Nexus_V1_DgnStructure3Vector *)malloc(
        (size_t)mesh_receipt.normal_count * sizeof(*normals));
    if (!vertices || !faces || !normals) goto cleanup;
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)structure3_entry_index, vertices,
            mesh_receipt.vertex_count, faces, mesh_receipt.face_count, normals,
            mesh_receipt.normal_count, &mesh_receipt) != 0 ||
        !mesh_receipt.valid || !mesh_receipt.source_identity_valid) goto cleanup;
    face = &faces[face_ordinal];
    slot_count = face->triangle ? 3 : 4;
    if (slot_count < 3 || slot_count > 4 ||
        face->vertex_indexes[0] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[1] >= (uint16_t)mesh_receipt.vertex_count ||
        face->vertex_indexes[2] >= (uint16_t)mesh_receipt.vertex_count ||
        (slot_count == 4 &&
         face->vertex_indexes[3] >= (uint16_t)mesh_receipt.vertex_count) ||
        face->flags != data[face_byte_offset + 8] ||
        face->raw_byte_9 != data[face_byte_offset + 9] ||
        face->fill_selector != (uint16_t)(((unsigned int)
            data[face_byte_offset + 10] << 8) | data[face_byte_offset + 11])) {
        goto cleanup;
    }
    out_packet->valid = 1;
    out_packet->source_geometry_bound = 1;
    out_packet->raw_fill_bound = 1;
    out_packet->level_index = engine->game.current_level;
    out_packet->source_byte_count = engine->current_level_dgn_size;
    out_packet->source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    out_packet->structure3_entry_index = structure3_entry_index;
    out_packet->face_ordinal = face_ordinal;
    out_packet->face_byte_offset = face_byte_offset;
    out_packet->face_bytes_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + face_byte_offset, 12);
    out_packet->face = *face;
    out_packet->vertex_slot_count = slot_count;
    out_packet->vertices[0] = vertices[face->vertex_indexes[0]];
    out_packet->vertices[1] = vertices[face->vertex_indexes[1]];
    out_packet->vertices[2] = vertices[face->vertex_indexes[2]];
    if (slot_count == 4)
        out_packet->vertices[3] = vertices[face->vertex_indexes[3]];
    out_packet->normal = normals[face_ordinal];
    out_packet->raw_fill_selector = face->fill_selector;
    result = 1;

cleanup:
    free(normals);
    free(faces);
    free(vertices);
    return result;
}

int nexus_v1_current_level_visit_structure3_untextured_faces(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3UntexturedFaceConsumer consumer, void *context,
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt receipt;
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 || !material_receipt.valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = material_receipt.level_index;
    receipt.source_byte_count = material_receipt.source_byte_count;
    receipt.source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    receipt.structure3_entry_count =
        engine->current_level.structure3_directory.entry_count;
    if (receipt.structure3_entry_count <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    for (entry_index = 0; entry_index < receipt.structure3_entry_count;
         ++entry_index) {
        int face_ordinal;
        int face_count =
            engine->current_level.structure3_entry_face_counts[entry_index];
        if (face_count < 0) {
            *out_receipt = receipt;
            return 0;
        }
        for (face_ordinal = 0; face_ordinal < face_count; ++face_ordinal) {
            Nexus_V1_DgnStructure3UntexturedFacePacket packet;
            ++receipt.candidate_face_count;
            if (nexus_v1_current_level_structure3_untextured_face_packet(
                    engine, (uint32_t)entry_index, (uint32_t)face_ordinal,
                    &packet) != 1) continue;
            ++receipt.untextured_face_count;
            if (!packet.valid || !packet.source_geometry_bound ||
                !packet.raw_fill_bound || packet.flat_fill_semantics_proven ||
                packet.transform_semantics_proven ||
                packet.pixel_palette_vdp1_semantics_proven ||
                packet.decoder_permitted || !packet.no_draw_only ||
                packet.fallback_visuals_permitted ||
                !packet.blocks_real_dgn_mesh_render ||
                (packet.face.flags & 0x40U) != 0U) {
                *out_receipt = receipt;
                return 0;
            }
            if (consumer && consumer(context, &packet) != 0) {
                *out_receipt = receipt;
                return 0;
            }
            ++receipt.consumed_face_count;
        }
    }
    if (receipt.candidate_face_count != material_receipt.faces.face_count ||
        receipt.untextured_face_count !=
            material_receipt.materials.non_textured_face_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.complete = receipt.consumed_face_count == receipt.untextured_face_count;
    *out_receipt = receipt;
    return receipt.complete ? 1 : 0;
}

int nexus_v1_current_level_structure3_complete_source_scene_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt receipt;
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt material_receipt;
    Nexus_V1_DgnActiveStructure2DescriptorReceipt descriptor_receipt;
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt payload_scene;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&descriptor_receipt, 0, sizeof(descriptor_receipt));
    memset(&payload_scene, 0, sizeof(payload_scene));
    if (!engine ||
        nexus_v1_current_level_structure3_face_material_receipt(
            engine, &material_receipt) != 1 || !material_receipt.valid ||
        nexus_v1_current_level_structure2_descriptor_receipt(
            engine, &descriptor_receipt) != 1 || !descriptor_receipt.valid ||
        nexus_v1_current_level_visit_structure3_package_geometry(
            engine, NULL, NULL, &receipt.static_scene) != 1 ||
        nexus_v1_current_level_visit_structure3_animated_materials(
            engine, NULL, NULL, &receipt.animated_scene) != 1 ||
        nexus_v1_current_level_visit_structure3_untextured_faces(
            engine, NULL, NULL, &receipt.untextured_scene) != 1 ||
        nexus_v1_current_level_visit_structure2_payload_anchors(
            engine, NULL, NULL, &payload_scene) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = material_receipt.level_index;
    receipt.source_byte_count = material_receipt.source_byte_count;
    receipt.source_bytes_fnv1a64 = material_receipt.source_bytes_fnv1a64;
    receipt.face_count = material_receipt.faces.face_count;
    receipt.structure2_descriptor_count = payload_scene.descriptor_count;
    receipt.structure2_payload_anchor_count = payload_scene.anchor_count;
    receipt.structure2_payload_anchors_consumed =
        payload_scene.consumed_anchor_count;
    receipt.structure2_image_anchor_count = payload_scene.image_anchor_count;
    receipt.structure2_palette_anchor_count = payload_scene.palette_anchor_count;
    receipt.structure2_payload_coverage_complete =
        payload_scene.valid && payload_scene.level_index == receipt.level_index &&
        payload_scene.source_byte_count == receipt.source_byte_count &&
        payload_scene.source_bytes_fnv1a64 == receipt.source_bytes_fnv1a64 &&
        payload_scene.descriptor_count == descriptor_receipt.descriptor_count &&
        payload_scene.descriptor_count > 0 &&
        payload_scene.image_anchor_count == payload_scene.descriptor_count &&
        payload_scene.consumed_anchor_count == payload_scene.anchor_count &&
        payload_scene.image_anchor_count + payload_scene.palette_anchor_count ==
            payload_scene.anchor_count &&
        payload_scene.no_draw_only && !payload_scene.fallback_visuals_permitted &&
        payload_scene.blocks_real_dgn_mesh_render;
    if (receipt.animated_scene.animated_face_count > 0) {
        if (nexus_v1_current_level_visit_structure3_animated_material_images(
                engine, NULL, NULL, &receipt.animated_image_scene) != 1 ||
            !receipt.animated_image_scene.valid ||
            !receipt.animated_image_scene.complete ||
            receipt.animated_image_scene.declared_image_instruction_count <= 0 ||
            receipt.animated_image_scene.consumed_image_instruction_count !=
                receipt.animated_image_scene.declared_image_instruction_count ||
            receipt.animated_image_scene.animated_face_count !=
                receipt.animated_scene.animated_face_count) {
            *out_receipt = receipt;
            return 0;
        }
        if (nexus_v1_current_level_visit_structure3_animated_material_payload_anchors(
                engine, &receipt.animated_payload_scene) != 1 ||
            !receipt.animated_payload_scene.valid ||
            !receipt.animated_payload_scene.complete ||
            receipt.animated_payload_scene.declared_image_instruction_count !=
                receipt.animated_image_scene.declared_image_instruction_count ||
            receipt.animated_payload_scene.consumed_image_instruction_count !=
                receipt.animated_payload_scene.declared_image_instruction_count ||
            receipt.animated_payload_scene.image_payload_anchor_count !=
                receipt.animated_payload_scene.declared_image_instruction_count) {
            *out_receipt = receipt;
            return 0;
        }
        receipt.animated_image_coverage_complete = 1;
        receipt.animated_payload_coverage_complete = 1;
    } else {
        /* No 08xx face has no image sequence to resolve, but remains no-draw. */
        receipt.animated_image_scene.valid = 1;
        receipt.animated_image_scene.complete = 1;
        receipt.animated_image_scene.level_index = receipt.level_index;
        receipt.animated_image_scene.source_byte_count =
            receipt.source_byte_count;
        receipt.animated_image_scene.source_bytes_fnv1a64 =
            receipt.source_bytes_fnv1a64;
        receipt.animated_image_scene.no_draw_only = 1;
        receipt.animated_image_scene.blocks_real_dgn_mesh_render = 1;
        receipt.animated_image_coverage_complete = 1;
        receipt.animated_payload_scene.valid = 1;
        receipt.animated_payload_scene.complete = 1;
        receipt.animated_payload_scene.level_index = receipt.level_index;
        receipt.animated_payload_scene.source_byte_count =
            receipt.source_byte_count;
        receipt.animated_payload_scene.source_bytes_fnv1a64 =
            receipt.source_bytes_fnv1a64;
        receipt.animated_payload_scene.no_draw_only = 1;
        receipt.animated_payload_scene.blocks_real_dgn_mesh_render = 1;
        receipt.animated_payload_coverage_complete = 1;
    }
    receipt.traversed_face_count =
        receipt.static_scene.consumed_face_count +
        receipt.animated_scene.consumed_face_count +
        receipt.untextured_scene.consumed_face_count;
    receipt.category_coverage_complete =
        receipt.static_scene.valid && receipt.static_scene.complete &&
        receipt.animated_scene.valid && receipt.animated_scene.complete &&
        receipt.animated_image_coverage_complete &&
        receipt.animated_image_scene.valid &&
        receipt.animated_image_scene.complete &&
        receipt.animated_payload_coverage_complete &&
        receipt.animated_payload_scene.valid &&
        receipt.animated_payload_scene.complete &&
        receipt.untextured_scene.valid && receipt.untextured_scene.complete &&
        receipt.static_scene.source_bytes_fnv1a64 ==
            receipt.source_bytes_fnv1a64 &&
        receipt.animated_scene.source_bytes_fnv1a64 ==
            receipt.source_bytes_fnv1a64 &&
        receipt.animated_image_scene.level_index == receipt.level_index &&
        receipt.animated_image_scene.source_byte_count ==
            receipt.source_byte_count &&
        receipt.animated_image_scene.source_bytes_fnv1a64 ==
            receipt.source_bytes_fnv1a64 &&
        receipt.animated_payload_scene.level_index == receipt.level_index &&
        receipt.animated_payload_scene.source_byte_count ==
            receipt.source_byte_count &&
        receipt.animated_payload_scene.source_bytes_fnv1a64 ==
            receipt.source_bytes_fnv1a64 &&
        receipt.untextured_scene.source_bytes_fnv1a64 ==
            receipt.source_bytes_fnv1a64 &&
        receipt.structure2_payload_coverage_complete &&
        receipt.static_scene.static_material_face_count ==
            material_receipt.materials.static_texture_selector_count &&
        receipt.animated_scene.animated_face_count ==
            material_receipt.materials.animated_texture_selector_count &&
        receipt.untextured_scene.untextured_face_count ==
            material_receipt.materials.non_textured_face_count &&
        material_receipt.materials.unsupported_textured_fill_count == 0 &&
        receipt.traversed_face_count == receipt.face_count;
    if (receipt.category_coverage_complete) {
        Nexus_V1_DgnStructure2FormatEvidenceReceipt fmt;
        if (nexus_v1_current_level_structure2_format_evidence_receipt(
                engine, &fmt) == 1 && fmt.valid && fmt.decoder_permitted) {
            receipt.decoder_permitted = 1;
        }
    }
    receipt.valid = receipt.category_coverage_complete;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

int nexus_v1_current_level_visit_structure1f_source_scene(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure1FSourceConsumer consumer, void *context,
    Nexus_V1_DgnStructure1FSourceSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FSourceSceneReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    int index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&spatial, 0, sizeof(spatial));
    memset(&relation, 0, sizeof(relation));
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_structure1f_spatial_receipt(&engine->current_level,
                                                   &spatial) != 0 ||
        !spatial.valid ||
        nexus_v1_level_structure1a_relation_receipt(&engine->current_level,
                                                    &relation) != 0 ||
        (spatial.structure1a_bound_entry_count > 0 && !relation.complete)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = engine->game.current_level;
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    receipt.entry_count = engine->current_level.structure1f_entry_count;
    for (index = 0; index < receipt.entry_count; ++index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &engine->current_level.structure1f_entries[index];
        Nexus_V1_DgnStructure1FSourcePacket packet;
        int direct = entry->family <= NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS;
        int owned = entry->family >= NEXUS_V1_DGN_STRUCTURE1F_ALCOVES;

        if ((!direct && !owned) || (direct && owned) ||
            (owned && !entry->structure1a_relation_valid)) {
            *out_receipt = receipt;
            return 0;
        }
        switch (entry->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS: ++receipt.item_entry_count; break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            ++receipt.floor_decoration_entry_count; break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            ++receipt.floor_sensor_entry_count; break;
        case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES: ++receipt.alcove_entry_count; break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
            ++receipt.wall_decoration_entry_count; break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
            ++receipt.wall_sensor_entry_count; break;
        default:
            *out_receipt = receipt;
            return 0;
        }
        memset(&packet, 0, sizeof(packet));
        packet.valid = 1;
        packet.level_index = receipt.level_index;
        packet.source_byte_count = receipt.source_byte_count;
        packet.source_bytes_fnv1a64 = receipt.source_bytes_fnv1a64;
        packet.entry_index = index;
        packet.entry = *entry;
        packet.descriptor_offset = entry->raw_record_offset;
        packet.descriptor_length = entry->raw_record_length;
        packet.descriptor_fnv1a64 = entry->raw_record_fnv1a64;
        packet.package_fnv1a64 = source->loaded_dgn_fnv1a64;
        if (!packet.descriptor_length ||
            packet.descriptor_offset > (uint32_t)engine->current_level_dgn_size ||
            packet.descriptor_length > (uint32_t)engine->current_level_dgn_size - packet.descriptor_offset) {
            *out_receipt = receipt;
            return 0;
        }
        packet.direct_coordinate_source = direct;
        packet.structure1a_owner_source = owned;
        packet.no_draw_only = 1;
        packet.blocks_real_dgn_mesh_render = 1;
        if (consumer && consumer(context, &packet) != 0) {
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.consumed_entry_count;
        if (direct) ++receipt.direct_coordinate_entry_count;
        if (owned) ++receipt.structure1a_owner_entry_count;
    }
    receipt.family_coverage_complete =
        receipt.entry_count == receipt.consumed_entry_count &&
        receipt.direct_coordinate_entry_count == spatial.direct_coordinate_entry_count &&
        receipt.structure1a_owner_entry_count == spatial.structure1a_bound_entry_count &&
        receipt.item_entry_count == spatial.item_entry_count &&
        receipt.floor_decoration_entry_count == spatial.floor_decoration_entry_count &&
        receipt.floor_sensor_entry_count == spatial.floor_sensor_entry_count &&
        receipt.alcove_entry_count + receipt.wall_decoration_entry_count +
            receipt.wall_sensor_entry_count == spatial.structure1a_bound_entry_count;
    receipt.valid = receipt.family_coverage_complete;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

typedef struct {
    int wanted_entry_index;
    int found;
    Nexus_V1_DgnStructure1FSourcePacket packet;
} Nexus_V1_DgnStructure1FEntryLookupContext;

static int nexus_v1_structure1f_entry_lookup_consumer(
    void *context, const Nexus_V1_DgnStructure1FSourcePacket *packet)
{
    Nexus_V1_DgnStructure1FEntryLookupContext *lookup =
        (Nexus_V1_DgnStructure1FEntryLookupContext *)context;
    if (!lookup || !packet || !packet->valid ||
        packet->entry_index != lookup->wanted_entry_index) {
        return 0;
    }
    lookup->packet = *packet;
    lookup->found = 1;
    return 1;
}

int nexus_v1_current_level_lookup_structure1f_source_entry(
    const Nexus_V1_Engine *engine, int entry_index,
    Nexus_V1_DgnStructure1FSourcePacket *out_packet)
{
    Nexus_V1_DgnStructure1FEntryLookupContext lookup;
    Nexus_V1_DgnStructure1FSourceSceneReceipt scene;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded || entry_index < 0 ||
        entry_index >= engine->current_level.structure1f_entry_count) {
        return 0;
    }
    memset(&lookup, 0, sizeof(lookup));
    memset(&scene, 0, sizeof(scene));
    lookup.wanted_entry_index = entry_index;
    /* The visitor returns 0 when a consumer deliberately stops after its
     * matching packet.  A lookup is therefore proven by the validated packet
     * itself; requiring whole-scene completion turns every early match into a
     * false miss. */
    (void)nexus_v1_current_level_visit_structure1f_source_scene(
        engine, nexus_v1_structure1f_entry_lookup_consumer, &lookup, &scene);
    if (!lookup.found) {
        return 0;
    }
    *out_packet = lookup.packet;
    return 1;
}

int nexus_v1_current_level_visit_structure1c_source_scene(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure1CSourceConsumer consumer, void *context,
    Nexus_V1_DgnStructure1CSourceSceneReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1CSourceSceneReceipt receipt;
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    uint8_t referenced[4096];
    uint16_t reference_occurrences[4096];
    uint8_t first_reference_x[4096];
    uint8_t first_reference_y[4096];
    uint8_t last_reference_x[4096];
    uint8_t last_reference_y[4096];
    int x, y, index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(referenced, 0, sizeof(referenced));
    memset(reference_occurrences, 0, sizeof(reference_occurrences));
    memset(first_reference_x, 0xff, sizeof(first_reference_x));
    memset(first_reference_y, 0xff, sizeof(first_reference_y));
    memset(last_reference_x, 0xff, sizeof(last_reference_x));
    memset(last_reference_y, 0xff, sizeof(last_reference_y));
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        !engine->current_level.geometry_info.collision_records_valid ||
        engine->current_level.geometry_info.structure1c_record_count <= 1 ||
        engine->current_level.geometry_info.structure1c_record_count >
            (int)sizeof(referenced) ||
        engine->current_level.geometry_info.structure1c_offset < 0 ||
        engine->current_level.geometry_info.structure1c_size !=
            engine->current_level.geometry_info.structure1c_record_count * 4 ||
        engine->current_level.geometry_info.structure1c_offset >
            engine->current_level_dgn_size ||
        engine->current_level.geometry_info.structure1c_size >
            engine->current_level_dgn_size -
                engine->current_level.geometry_info.structure1c_offset) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = engine->game.current_level;
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    receipt.record_count = engine->current_level.geometry_info.structure1c_record_count;
    receipt.indexed_record_count = receipt.record_count - 1;
    for (y = 0; y < engine->current_level.height; ++y) {
        for (x = 0; x < engine->current_level.width; ++x) {
            uint16_t ref = engine->current_level.collision_refs[y][x];
            if (ref > 0U && ref < (uint16_t)receipt.record_count) {
                referenced[ref] = 1U;
                if (reference_occurrences[ref] == 0U) {
                    first_reference_x[ref] = (uint8_t)x;
                    first_reference_y[ref] = (uint8_t)y;
                }
                if (reference_occurrences[ref] == UINT16_MAX) {
                    *out_receipt = receipt;
                    return 0;
                }
                ++reference_occurrences[ref];
                last_reference_x[ref] = (uint8_t)x;
                last_reference_y[ref] = (uint8_t)y;
                ++receipt.reference_occurrence_count;
            }
        }
    }
    for (index = 1; index < receipt.record_count; ++index) {
        Nexus_V1_DgnStructure1CSourcePacket packet;
        const uint8_t *record = engine->current_level_dgn_data +
            engine->current_level.geometry_info.structure1c_offset + index * 4;

        memset(&packet, 0, sizeof(packet));
        packet.valid = 1;
        packet.level_index = receipt.level_index;
        packet.source_byte_count = receipt.source_byte_count;
        packet.source_bytes_fnv1a64 = receipt.source_bytes_fnv1a64;
        packet.record_index = index;
        memcpy(packet.raw_bytes, record, sizeof(packet.raw_bytes));
        packet.referenced_by_structure1b = referenced[index] ? 1 : 0;
        packet.reference_occurrence_count = reference_occurrences[index];
        packet.first_reference_x = referenced[index]
            ? (int)first_reference_x[index] : -1;
        packet.first_reference_y = referenced[index]
            ? (int)first_reference_y[index] : -1;
        packet.last_reference_x = referenced[index]
            ? (int)last_reference_x[index] : -1;
        packet.last_reference_y = referenced[index]
            ? (int)last_reference_y[index] : -1;
        packet.no_draw_only = 1;
        packet.blocks_real_dgn_mesh_render = 1;
        if (consumer && consumer(context, &packet) != 0) {
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.consumed_record_count;
        if (packet.referenced_by_structure1b) ++receipt.referenced_record_count;
        else ++receipt.unreferenced_record_count;
    }
    receipt.valid = receipt.consumed_record_count == receipt.indexed_record_count &&
        receipt.referenced_record_count + receipt.unreferenced_record_count ==
            receipt.indexed_record_count;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

typedef struct {
    int wanted_record_index;
    int found;
    Nexus_V1_DgnStructure1CSourcePacket packet;
} Nexus_V1_DgnStructure1CCellLookupContext;

static int nexus_v1_structure1c_cell_lookup_consumer(
    void *context, const Nexus_V1_DgnStructure1CSourcePacket *packet)
{
    Nexus_V1_DgnStructure1CCellLookupContext *lookup =
        (Nexus_V1_DgnStructure1CCellLookupContext *)context;
    if (!lookup || !packet || !packet->valid ||
        packet->record_index != lookup->wanted_record_index) {
        return 0;
    }
    lookup->packet = *packet;
    lookup->found = 1;
    return 1;
}

int nexus_v1_current_level_lookup_structure1c_cell_source(
    const Nexus_V1_Engine *engine, int cell_x, int cell_y,
    Nexus_V1_DgnStructure1CCellSourcePacket *out_packet)
{
    Nexus_V1_DgnStructure1CCellLookupContext lookup;
    Nexus_V1_DgnStructure1CSourceSceneReceipt scene;
    uint16_t collision_ref;

    if (!out_packet) return -1;
    memset(out_packet, 0, sizeof(*out_packet));
    out_packet->cell_x = cell_x;
    out_packet->cell_y = cell_y;
    out_packet->no_draw_only = 1;
    out_packet->blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->level_loaded || cell_x < 0 || cell_y < 0 ||
        cell_x >= engine->current_level.width ||
        cell_y >= engine->current_level.height) {
        return 0;
    }
    collision_ref = engine->current_level.collision_refs[cell_y][cell_x];
    if (collision_ref == 0U) return 0;
    memset(&lookup, 0, sizeof(lookup));
    memset(&scene, 0, sizeof(scene));
    lookup.wanted_record_index = (int)collision_ref;
    /* As with Structure1F, a matching consumer intentionally stops the
     * source-scene walk before its whole-scene receipt is complete.  The
     * packet already carries the bounded reference provenance needed by this
     * cell lookup; whole-scene completion is enforced by the visitor's other
     * consumers, not by this early-match wrapper. */
    (void)nexus_v1_current_level_visit_structure1c_source_scene(
        engine, nexus_v1_structure1c_cell_lookup_consumer, &lookup, &scene);
    if (!lookup.found || !lookup.packet.referenced_by_structure1b ||
        lookup.packet.reference_occurrence_count <= 0) {
        return 0;
    }
    out_packet->collision_ref = collision_ref;
    out_packet->record = lookup.packet;
    out_packet->valid = 1;
    return 1;
}

int nexus_v1_current_level_transform_camera_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveTransformCameraFramingReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    Nexus_V1_DgnCellGeometry cell;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size) ||
        nexus_v1_level_get_cell_geometry(&engine->current_level,
                                         engine->game.party_x,
                                         engine->game.party_y, &cell) != 0 ||
        nexus_v1_level_structure1a_transform_selector_receipt(
            &engine->current_level, &out_receipt->transform_selectors) != 0)
        return 0;
    out_receipt->valid = 1;
    out_receipt->level_index = engine->game.current_level;
    out_receipt->source_byte_count = engine->current_level_dgn_size;
    out_receipt->source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    out_receipt->party_x = engine->game.party_x;
    out_receipt->party_y = engine->game.party_y;
    out_receipt->party_dir = engine->game.party_dir & 3;
    out_receipt->party_cell_geometry_valid = 1;
    out_receipt->party_square_type = cell.square_type;
    out_receipt->party_collision_ref = cell.collision_ref;
    out_receipt->party_post_grid_0x30_ref = cell.post_grid_0x30_ref;
    out_receipt->transform_selector_source_bound =
        out_receipt->transform_selectors.complete ? 1 : 0;
    return 1;
}

int nexus_v1_current_level_structure1f_face_mesh_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure1FFaceMeshReceipt *out_receipt)
{
    Nexus_V1_DgnActiveStructure1FFaceMeshReceipt receipt;
    const Nexus_V1_DgnStructure2SourceReceipt *source;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    receipt.level_index = engine->game.current_level;
    receipt.canonical_lev_source_bound = source->level_index == receipt.level_index &&
        source->canonical_hash_verified && source->materialization_bound &&
        nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                        engine->current_level_dgn_size);
    if (!receipt.canonical_lev_source_bound ||
        nexus_v1_level_structure3_attachment_receipt(&engine->current_level,
                                                     &receipt.attachment) != 0 ||
        !receipt.attachment.complete ||
        !receipt.attachment.record_to_face_normal_semantics_proven ||
        receipt.attachment.normal_plane_transform_or_draw_semantics_proven) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.face_mesh_ordinal_relation_proven = 1;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_build_structure1f_direct_mesh_binding(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt receipt;
    Nexus_V1_DgnStructure3AttachmentReceipt attachment;
    Nexus_V1_DgnStructure2SourceReceipt source_receipt;
    const Nexus_V1_DgnStructure2SourceReceipt *source = &source_receipt;
    const Nexus_V1_DgnStructure1FEntry *entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level_index = -1;
    receipt.no_draw_only = 1;
    if (!engine || !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0 ||
        structure1f_entry_index < 0 ||
        structure1f_entry_index >= engine->current_level.structure1f_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&source_receipt, 0, sizeof(source_receipt));
    if (nexus_v1_current_level_structure2_source_receipt(
            engine, &source_receipt) != 0 ||
        source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        !source->structure2_payload_envelope_valid ||
        source->fallback_visuals_permitted) {
        *out_receipt = receipt;
        return 0;
    }
    entry = &engine->current_level.structure1f_entries[structure1f_entry_index];
    if (entry->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
        !entry->structure1a_relation_valid ||
        !engine->current_level.structure1a_table_valid ||
        !engine->current_level.structure3_directory.valid ||
        entry->structure1a_index >=
            (uint16_t)engine->current_level.structure1a_model_count ||
        (int)entry->structure1a_structure3_model_index >=
            engine->current_level.structure3_directory.entry_count ||
        entry->face >= engine->current_level.structure3_entry_face_counts[
            entry->structure1a_structure3_model_index]) {
        *out_receipt = receipt;
        return 0;
    }
    memset(&attachment, 0, sizeof(attachment));
    if (nexus_v1_level_structure3_attachment_receipt(
            &engine->current_level, &attachment) != 0 ||
        !attachment.complete ||
        !attachment.record_to_face_normal_semantics_proven ||
        attachment.normal_plane_transform_or_draw_semantics_proven ||
        !nexus_v1_dgn_structure3_capture_target_build(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, engine->game.current_level, 1,
            entry->structure1a_structure3_model_index, entry->face,
            &receipt.face_target)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = engine->game.current_level;
    receipt.source_byte_count = engine->current_level_dgn_size;
    receipt.source_bytes_fnv1a64 = source->loaded_dgn_fnv1a64;
    receipt.structure1f_entry_index = structure1f_entry_index;
    receipt.owner_x = entry->structure1a_owner_x;
    receipt.owner_y = entry->structure1a_owner_y;
    receipt.structure1a_index = entry->structure1a_index;
    receipt.structure3_model_index = entry->structure1a_structure3_model_index;
    receipt.z_rotation = entry->structure1a_z_rotation;
    receipt.face_ordinal = entry->face;
    receipt.model_to_entry_proven = 1;
    receipt.face_ordinal_proven = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_build_structure1f_direct_mesh_geometry_packet(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectMeshGeometryPacket *out_packet)
{
    Nexus_V1_DgnStructure1FDirectMeshGeometryPacket packet;
    Nexus_V1_DgnStructure3MeshEntryReceipt receipt;
    Nexus_V1_DgnStructure3Vector *vertices = NULL, *normals = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    const Nexus_V1_DgnStructure3Face *face;
    int slots, result = 0;

    if (!out_packet) return -1;
    memset(&packet, 0, sizeof(packet));
    packet.no_draw_only = 1;
    packet.blocks_real_dgn_mesh_render = 1;
    if (nexus_v1_engine_build_structure1f_direct_mesh_binding(
            engine, structure1f_entry_index, &packet.direct_mesh) != 1 ||
        !packet.direct_mesh.valid || !packet.direct_mesh.model_to_entry_proven ||
        !packet.direct_mesh.face_ordinal_proven ||
        packet.direct_mesh.fallback_visuals_permitted) goto done;
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, packet.direct_mesh.structure3_model_index, NULL, 0, NULL, 0,
            NULL, 0, &receipt) != -1 || !receipt.source_identity_valid ||
        receipt.vertex_count <= 0 ||
        receipt.normal_count < receipt.face_count ||
        packet.direct_mesh.face_ordinal >= receipt.face_count) goto done;
    vertices = malloc((size_t)receipt.vertex_count * sizeof(*vertices));
    faces = malloc((size_t)receipt.face_count * sizeof(*faces));
    normals = malloc((size_t)receipt.normal_count * sizeof(*normals));
    if (!vertices || !faces || !normals ||
        nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, packet.direct_mesh.structure3_model_index, vertices,
            receipt.vertex_count, faces, receipt.face_count, normals,
            receipt.normal_count, &receipt) != 0 || !receipt.valid ||
        !receipt.source_identity_valid) goto done;
    face = &faces[packet.direct_mesh.face_ordinal];
    slots = face->triangle ? 3 : 4;
    if (slots < 3 || slots > 4 ||
        face->vertex_indexes[0] >= receipt.vertex_count ||
        face->vertex_indexes[1] >= receipt.vertex_count ||
        face->vertex_indexes[2] >= receipt.vertex_count ||
        (slots == 4 && face->vertex_indexes[3] >= receipt.vertex_count)) goto done;
    packet.valid = 1;
    packet.source_geometry_bound = 1;
    packet.face = *face;
    packet.vertex_slot_count = slots;
    packet.vertices[0] = vertices[face->vertex_indexes[0]];
    packet.vertices[1] = vertices[face->vertex_indexes[1]];
    packet.vertices[2] = vertices[face->vertex_indexes[2]];
    if (slots == 4) packet.vertices[3] = vertices[face->vertex_indexes[3]];
    packet.normal = normals[packet.direct_mesh.face_ordinal];
    result = 1;
done:
    free(normals); free(faces); free(vertices);
    *out_packet = packet;
    return result;
}

int nexus_v1_engine_build_structure1f_transform_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FTransformCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;

    if (!out_target) return -1;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    if (!engine || nexus_v1_engine_build_structure1f_direct_mesh_geometry_packet(
            engine, structure1f_entry_index, &target.geometry) != 1 ||
        !target.geometry.valid || !target.geometry.source_geometry_bound ||
        !target.geometry.no_draw_only || target.geometry.fallback_visuals_permitted ||
        nexus_v1_level_structure1a_transform_table_receipt(
            &engine->current_level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, &target.transform_table) != 1 ||
        !target.transform_table.valid || !target.transform_table.source_table_bound ||
        !target.transform_table.parsed_model_rows_match ||
        !target.transform_table.selectors.complete ||
        nexus_v1_level_structure1a_transform_selector_receipt(
            &engine->current_level, &target.transform_selectors) != 0 ||
        target.transform_selectors.resolved_selector_count <= 0 ||
        target.transform_table.selector_column_fnv1a64 == 0U) {
        *out_target = target;
        return 0;
    }
    target.transform_table_source_bound = 1;
    target.owner_transform_selector_source_bound = 1;
    target.capture_producer_required = 1;
    target.original_saturn_capture_required = 1;
    target.valid = 1;
    *out_target = target;
    return 1;
}

int nexus_v1_engine_write_structure1f_direct_face_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *path, Nexus_V1_DgnStructure1FTransformCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *face_target;
    char temporary_path[1024];
    FILE *file;
    int write_ok;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    memset(&target, 0, sizeof(target));
    if (!path || !path[0] ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path) ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &target) != 1 || !target.valid ||
        !target.geometry.valid || !target.geometry.source_geometry_bound ||
        !target.geometry.direct_mesh.valid ||
        !target.transform_table_source_bound ||
        !target.owner_transform_selector_source_bound ||
        !target.capture_producer_required ||
        !target.original_saturn_capture_required || !target.no_draw_only ||
        target.fallback_visuals_permitted || !target.blocks_real_dgn_mesh_render) {
        *out_target = target;
        return 0;
    }
    face_target = &target.geometry.direct_mesh.face_target.candidate;
    file = fopen(temporary_path, "wb");
    if (!file) {
        *out_target = target;
        return 0;
    }
    write_ok = fprintf(file,
                       "%s\n"
                       "level_index=%d\nsource_byte_count=%d\n"
                       "source_bytes_fnv1a64=%016llx\n"
                       "structure1f_entry_index=%d\nowner_x=%d\nowner_y=%d\n"
                       "structure1a_index=%u\nstructure3_model_index=%u\n"
                       "face_ordinal=%u\nz_rotation=%u\n"
                       "face_row_fnv1a32=%08x\n"
                       "referenced_vertex_rows_fnv1a32=%08x\n"
                       "normal_row_fnv1a32=%08x\nfill_selector=%u\n"
                       "vertex_indexes=%u,%u,%u,%u\n"
                       "structure1a_table_offset=%d\nstructure1a_table_bytes=%d\n"
                       "structure1a_table_fnv1a64=%016llx\n"
                       "selector_column_fnv1a64=%016llx\n"
                       "requested_observations=source-read,owner,face,vertices,normal,transform,culling,vdp1\n"
                       "original_saturn_capture_required=1\nno_draw_only=1\n",
                       NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_MAGIC,
                       target.geometry.direct_mesh.level_index,
                       target.geometry.direct_mesh.source_byte_count,
                       (unsigned long long)target.geometry.direct_mesh.source_bytes_fnv1a64,
                       target.geometry.direct_mesh.structure1f_entry_index,
                       target.geometry.direct_mesh.owner_x,
                       target.geometry.direct_mesh.owner_y,
                       target.geometry.direct_mesh.structure1a_index,
                       target.geometry.direct_mesh.structure3_model_index,
                       target.geometry.direct_mesh.face_ordinal,
                       target.geometry.direct_mesh.z_rotation,
                       face_target->face_row_fnv1a32,
                       face_target->referenced_vertex_rows_fnv1a32,
                       face_target->normal_row_fnv1a32,
                       face_target->fill_selector,
                       target.geometry.face.vertex_indexes[0],
                       target.geometry.face.vertex_indexes[1],
                       target.geometry.face.vertex_indexes[2],
                       target.geometry.face.vertex_indexes[3],
                       target.transform_table.table_byte_offset,
                       target.transform_table.table_byte_count,
                       (unsigned long long)target.transform_table.raw_table_fnv1a64,
                       (unsigned long long)target.transform_table.selector_column_fnv1a64) > 0;
    if (fclose(file) != 0) write_ok = 0;
    if (write_ok && rename(temporary_path, path) != 0) write_ok = 0;
    if (!write_ok) remove(temporary_path);
    *out_target = target;
    return write_ok ? 1 : 0;
}

int nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *manifest_text, size_t manifest_size,
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *face_target;
    const Nexus_V1_DgnStructure2SourceReceipt *source;
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt receipt;
    char value[96];
    char vertex_indexes[48];
    uint32_t parsed_u32;
    uint64_t parsed_u64;
    size_t magic_size;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_MISSING;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !manifest_text || manifest_size == 0U ||
        !engine->level_loaded || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0 ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &target) != 1 || !target.valid ||
        !target.geometry.source_geometry_bound ||
        !target.transform_table_source_bound ||
        !target.owner_transform_selector_source_bound) {
        *out_receipt = receipt;
        return 0;
    }
    source = &engine->current_level_structure2_source;
    if (source->level_index != engine->game.current_level ||
        !source->canonical_hash_verified || !source->materialization_bound ||
        !source->loaded_bytes_bound ||
        source->loaded_dgn_size != engine->current_level_dgn_size ||
        !nexus_v1_dgn_source_bytes_match(source, engine->current_level_dgn_data,
                                         engine->current_level_dgn_size)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.package_bytes_bound = 1;
    magic_size = strlen(NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_MAGIC);
    if (manifest_size <= magic_size ||
        memcmp(manifest_text,
               NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_MAGIC,
               magic_size) != 0 ||
        (manifest_text[magic_size] != '\n' && manifest_text[magic_size] != '\r')) {
        receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    face_target = &target.geometry.direct_mesh.face_target.candidate;
    (void)snprintf(vertex_indexes, sizeof(vertex_indexes), "%u,%u,%u,%u",
                   target.geometry.face.vertex_indexes[0],
                   target.geometry.face.vertex_indexes[1],
                   target.geometry.face.vertex_indexes[2],
                   target.geometry.face.vertex_indexes[3]);
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "level_index",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.level_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "source_byte_count", value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.source_byte_count ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "source_bytes_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.geometry.direct_mesh.source_bytes_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1f_entry_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1f_entry_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "owner_x",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.owner_x ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "owner_y",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.owner_y ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1a_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure3_model_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure3_model_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "face_ordinal",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.face_ordinal ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "z_rotation",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.z_rotation ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "face_row_fnv1a32", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != face_target->face_row_fnv1a32 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "referenced_vertex_rows_fnv1a32", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != face_target->referenced_vertex_rows_fnv1a32 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "normal_row_fnv1a32", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != face_target->normal_row_fnv1a32 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "fill_selector", value, sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != face_target->fill_selector ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "vertex_indexes", value, sizeof(value)) ||
        strcmp(value, vertex_indexes) != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_offset", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.transform_table.table_byte_offset ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_bytes", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_decimal_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.transform_table.table_byte_count ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.raw_table_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "selector_column_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.selector_column_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "original_saturn_capture_required", value,
                                    sizeof(value)) || strcmp(value, "1") != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "no_draw_only",
                                    value, sizeof(value)) || strcmp(value, "1") != 0) {
        receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_target_bound = 1;
    receipt.owner_geometry_bound = 1;
    receipt.transform_selectors_bound = 1;
    receipt.original_saturn_capture_required = 1;
    receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_bind_structure1f_direct_face_raw_capture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt receipt;
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *expected;
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *observed;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_MISSING;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !direct_manifest_text || direct_manifest_size == 0U) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
        engine, structure1f_entry_index, direct_manifest_text,
        direct_manifest_size, &receipt.direct_face);
    if (receipt.direct_face.status !=
        NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &target) != 1 || !target.valid ||
        !target.geometry.direct_mesh.model_to_entry_proven ||
        !target.geometry.direct_mesh.face_ordinal_proven) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_DIRECT_FACE;
        *out_receipt = receipt;
        return 0;
    }
    if (!raw_capture || !raw_capture->manifest_parsed ||
        !raw_capture->raw_reader_invoked || !raw_capture->host_intake_invoked ||
        !raw_capture->no_draw_only ||
        !raw_capture->raw_reader.manifest_accepted ||
        !raw_capture->raw_reader.all_spans_read ||
        !raw_capture->raw_reader.raw_span_hashes_match ||
        !raw_capture->raw_reader.attestation_session_matches ||
        !raw_capture->raw_reader.attestation_bundle_matches ||
        !raw_capture->raw_reader.attestation_trace_order_matches ||
        !raw_capture->raw_reader.original_saturn_source_attested ||
        !raw_capture->raw_reader.import_ready ||
        !raw_capture->raw_reader.import_packet.original_saturn_capture_verified ||
        !raw_capture->host.host_dgn_source_verified ||
        !raw_capture->host.capture_source_verified ||
        !raw_capture->host.manifest_parsed || !raw_capture->host.importer_invoked ||
        !raw_capture->host.no_draw_only ||
        !raw_capture->host.import_receipt.manifest_valid ||
        !raw_capture->host.import_receipt.spans_match_manifest ||
        !raw_capture->host.import_receipt.raw_span_hashes_match ||
        !raw_capture->host.import_receipt.capture_session_matches ||
        !raw_capture->host.import_receipt.capture_bundle_matches ||
        !raw_capture->host.import_receipt.capture_trace_order_matches ||
        !raw_capture->host.import_receipt.capture_bundle_hash_verified ||
        !raw_capture->host.import_receipt.capture_trace_order_verified ||
        !raw_capture->host.import_receipt.original_saturn_capture_verified ||
        !raw_capture->host.import_receipt.dgn_source_hash_verified ||
        !raw_capture->host.import_receipt.binder_invoked ||
        !raw_capture->host.import_receipt.complete_source_binding ||
        !raw_capture->host.import_receipt.blocks_real_dgn_mesh_render) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_CAPTURE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_capture_authenticated = 1;
    receipt.raw_capture_source_bound = 1;
    expected = &target.geometry.direct_mesh.face_target.candidate;
    observed = &raw_capture->host.manifest.candidate;
    if (observed->dgn_fnv1a64 != expected->dgn_fnv1a64 ||
        observed->structure3_payload_fnv1a32 !=
            expected->structure3_payload_fnv1a32 ||
        observed->typed_mesh_corpus_fnv1a32 != expected->typed_mesh_corpus_fnv1a32 ||
        observed->entry_index != expected->entry_index ||
        observed->face_ordinal != expected->face_ordinal ||
        observed->face_row_fnv1a32 != expected->face_row_fnv1a32 ||
        observed->referenced_vertex_rows_fnv1a32 !=
            expected->referenced_vertex_rows_fnv1a32 ||
        observed->normal_row_fnv1a32 != expected->normal_row_fnv1a32 ||
        observed->fill_selector != expected->fill_selector) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_FACE_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.direct_face_candidate_bound = 1;
    receipt.texture_lane_bound = 1;
    receipt.palette_lane_bound = 1;
    receipt.vdp1_state_lane_bound = 1;
    receipt.transform_lane_bound = 1;
    receipt.normal_culling_lane_bound = 1;
    receipt.vdp1_command_lane_bound = 1;
    receipt.status = NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_ACCEPTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_bind_structure1f_vdp1_material_capture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    Nexus_V1_DgnStructure1FVdp1MaterialReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FVdp1MaterialReceipt receipt;
    Nexus_V1_DgnStructure3RenderPacket packet;
    const Nexus_V1_DgnStructure3CaptureImport *capture;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_MISSING;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !raw_capture) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_engine_bind_structure1f_direct_face_raw_capture(
        engine, structure1f_entry_index, direct_manifest_text,
        direct_manifest_size, raw_capture, &receipt.direct_capture);
    if (receipt.direct_capture.status !=
        NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_ACCEPTED_OPAQUE) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_DIRECT_CAPTURE;
        *out_receipt = receipt;
        return 0;
    }
    memset(&packet, 0, sizeof(packet));
    capture = &raw_capture->raw_reader.import_packet;
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) != 1 ||
        !packet.valid || !packet.texture_span || !packet.palette_state ||
        !packet.vdp1_state || !packet.vdp1_command ||
        !capture->texture_span || !capture->palette_state ||
        !capture->vdp1_state || !capture->vdp1_command ||
        packet.texture_span_size != (int)capture->texture_span_size ||
        packet.palette_state_size != (int)capture->palette_state_size ||
        packet.vdp1_state_size != (int)capture->vdp1_state_size ||
        packet.vdp1_command_size != (int)capture->vdp1_command_size ||
        memcmp(packet.texture_span, capture->texture_span,
               capture->texture_span_size) != 0 ||
        memcmp(packet.palette_state, capture->palette_state,
               capture->palette_state_size) != 0 ||
        memcmp(packet.vdp1_state, capture->vdp1_state,
               capture->vdp1_state_size) != 0 ||
        memcmp(packet.vdp1_command, capture->vdp1_command,
               capture->vdp1_command_size) != 0) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_RUNTIME_CAPTURE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.runtime_lanes_match_authenticated_capture = 1;
    if (nexus_v1_current_level_structure3_vdp1_vram_window_receipt(
            engine, &receipt.texture_vram) != 1 ||
        !receipt.texture_vram.valid ||
        !receipt.texture_vram.texture_lane_matches_vram_window ||
        nexus_v1_current_level_structure3_vdp1_command_vram_receipt(
            engine, &receipt.command_vram) != 1 ||
        !receipt.command_vram.valid ||
        !receipt.command_vram.command_record_unique_in_vram) {
        receipt.status = NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_VDP1_LINK;
        *out_receipt = receipt;
        return 0;
    }
    receipt.texture_command_vram_bound = 1;
    receipt.palette_lane_bound = 1;
    receipt.command_colour_control =
        receipt.texture_vram.command_framing.command.colour_control;
    receipt.command_colour_control_bound = 1;
    receipt.texture_lane_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        packet.texture_span, (size_t)packet.texture_span_size);
    receipt.palette_lane_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        packet.palette_state, (size_t)packet.palette_state_size);
    receipt.status = NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_ACCEPTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_decode_structure1f_vdp1_lookup_texture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    uint16_t *out_colour_codes, size_t out_colour_code_count,
    const uint16_t *expected_colour_codes, size_t expected_colour_code_count,
    Nexus_V1_DgnStructure1FVdp1LookupDecodeReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FVdp1LookupDecodeReceipt receipt;
    Nexus_V1_DgnStructure3RenderPacket packet;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !out_colour_codes || out_colour_code_count == 0U) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_engine_bind_structure1f_vdp1_material_capture(
        engine, structure1f_entry_index, direct_manifest_text,
        direct_manifest_size, raw_capture, &receipt.material);
    if (receipt.material.status !=
        NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_ACCEPTED_OPAQUE) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.direct_face_capture_bound = 1;
    memset(&packet, 0, sizeof(packet));
    if (nexus_v1_current_level_structure3_render_packet(engine, &packet) != 1 ||
        !packet.valid || !packet.texture_span || !packet.vdp1_state ||
        !packet.vdp1_command ||
        nexus_v1_vdp1_decode_mode1_lookup_texture(
            packet.vdp1_command, packet.vdp1_command_size,
            packet.vdp1_state, packet.vdp1_state_size, packet.texture_span,
            packet.texture_span_size, out_colour_codes, out_colour_code_count,
            &receipt.lookup) != 1 || !receipt.lookup.valid ||
        !receipt.lookup.texture_lane_matches_vram ||
        !receipt.lookup.texture_high_nibble_first) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.lookup_colour_codes_bound = 1;
    if (expected_colour_codes && expected_colour_code_count > 0U) {
        receipt.pixel_output_witness_verified =
            nexus_v1_vdp1_lookup_colour_codes_match(
                out_colour_codes, receipt.lookup.output_pixel_count,
                expected_colour_codes, expected_colour_code_count);
    }
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_admit_structure1f_transform_capture_trace(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *transform_state, size_t transform_state_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;
    Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt receipt;
    char value[96];
    uint32_t parsed_u32;
    uint64_t parsed_u64;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MISSING;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !manifest_text || manifest_size == 0U || !raw_trace ||
        raw_trace_size == 0U || !transform_state || transform_state_size == 0U ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &target) != 1 || !target.valid ||
        !target.transform_table_source_bound ||
        !target.owner_transform_selector_source_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_target_bound = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "magic", value,
                                    sizeof(value)) ||
        strcmp(value, NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "producer", value,
                                    sizeof(value)) ||
        strcmp(value, "saturn-debugger") != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "trace_sha256",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_is_sha256(value)) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "level_index",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.level_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1f_entry_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1f_entry_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1a_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure3_model_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure3_model_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "face_ordinal",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.face_ordinal ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "z_rotation",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.z_rotation ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_offset", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.transform_table.table_byte_offset ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_bytes", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.transform_table.table_byte_count ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_table_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.raw_table_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "selector_column_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.selector_column_fnv1a64) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_target_bound = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "raw_trace_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size) ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "transform_state_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != nexus_v1_slev_trace_fnv1a64(transform_state,
                                                   transform_state_size)) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_OBSERVATIONS;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bytes_bound = 1;
    receipt.raw_trace_byte_count = raw_trace_size;
    receipt.raw_trace_fnv1a64 = nexus_v1_slev_trace_fnv1a64(raw_trace,
                                                             raw_trace_size);
    receipt.transform_state_bytes_bound = 1;
    receipt.transform_state_byte_count = transform_state_size;
    receipt.transform_state_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        transform_state, transform_state_size);
    if (!original_saturn_capture_verified) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_PROVENANCE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.original_saturn_capture_verified = 1;
    receipt.opaque_trace_admitted = 1;
    receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_ADMITTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

static uint8_t *nexus_v1_read_transform_trace_sidecar(const char *path,
                                                       size_t *out_size)
{
    FILE *file;
    long end;
    uint8_t *bytes = NULL;

    if (out_size) *out_size = 0U;
    if (!path || !path[0] || !out_size) return NULL;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (end = ftell(file)) <= 0L || end > INT_MAX ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)end + 1U)) ||
        fread(bytes, 1U, (size_t)end, file) != (size_t)end) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    bytes[end] = 0U;
    *out_size = (size_t)end;
    return bytes;
}

int nexus_v1_engine_parse_structure1f_transform_trace_attestation(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *attestation_text, size_t attestation_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *transform_state, size_t transform_state_size,
    Nexus_V1_DgnStructure1FTransformTraceAttestationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FTransformCaptureTarget target;
    Nexus_V1_DgnStructure1FTransformTraceAttestationReceipt receipt;
    char value[96];
    uint32_t parsed_u32;
    uint64_t parsed_u64;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_MISSING;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !attestation_text || attestation_size == 0U || !raw_trace ||
        raw_trace_size == 0U || !transform_state || transform_state_size == 0U ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &target) != 1 || !target.valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_target_bound = 1;
    if (!nexus_v1_slev_trace_value(attestation_text, attestation_size, "magic",
                                    value, sizeof(value)) ||
        strcmp(value, NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size, "reviewer",
                                    value, sizeof(value)) ||
        strcmp(value, "independent-saturn-review") != 0 ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "attestation_sha256", value, sizeof(value)) ||
        !nexus_v1_slev_trace_is_sha256(value)) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.attestation_sha256_present = 1;
    receipt.independent_saturn_review_declared = 1;
    if (!nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "level_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.level_index ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "structure1f_entry_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1f_entry_index ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "structure1a_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure1a_index ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "structure3_model_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.structure3_model_index ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "face_ordinal", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.face_ordinal ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "z_rotation", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)target.geometry.direct_mesh.z_rotation ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "structure1a_table_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.raw_table_fnv1a64 ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "selector_column_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != target.transform_table.selector_column_fnv1a64) {
        receipt.status =
            NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_target_bound = 1;
    if (!nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "raw_trace_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size) ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "transform_state_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != nexus_v1_slev_trace_fnv1a64(transform_state,
                                                   transform_state_size) ||
        !nexus_v1_slev_trace_value(attestation_text, attestation_size,
                                    "original_saturn_source_attested", value,
                                    sizeof(value)) || strcmp(value, "1") != 0) {
        receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_SIDECARS;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bound = 1;
    receipt.transform_state_bound = 1;
    receipt.original_saturn_source_attested = 1;
    receipt.status = NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_ADMITTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_ingest_structure1f_transform_capture_trace(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const Nexus_V1_DgnStructure1FTransformTracePaths *paths,
    Nexus_V1_DgnStructure1FTransformTraceFileIntakeReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FTransformTraceFileIntakeReceipt receipt;
    uint8_t *capture_target = NULL;
    uint8_t *manifest = NULL;
    uint8_t *raw_trace = NULL;
    uint8_t *transform_state = NULL;
    uint8_t *attestation = NULL;
    size_t capture_target_size = 0U;
    size_t manifest_size = 0U;
    size_t raw_trace_size = 0U;
    size_t transform_state_size = 0U;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !paths || !paths->capture_target_path || !paths->manifest_path ||
        !paths->raw_trace_path || !paths->transform_state_path ||
        !paths->attestation_path ||
        !paths->capture_target_path[0] || !paths->manifest_path[0] ||
        !paths->raw_trace_path[0] ||
        !paths->transform_state_path[0] || !paths->attestation_path[0] ||
        strcmp(paths->capture_target_path, paths->manifest_path) == 0 ||
        strcmp(paths->capture_target_path, paths->raw_trace_path) == 0 ||
        strcmp(paths->capture_target_path, paths->transform_state_path) == 0 ||
        strcmp(paths->capture_target_path, paths->attestation_path) == 0 ||
        strcmp(paths->manifest_path, paths->raw_trace_path) == 0 ||
        strcmp(paths->manifest_path, paths->transform_state_path) == 0 ||
        strcmp(paths->manifest_path, paths->attestation_path) == 0 ||
        strcmp(paths->raw_trace_path, paths->transform_state_path) == 0 ||
        strcmp(paths->raw_trace_path, paths->attestation_path) == 0 ||
        strcmp(paths->transform_state_path, paths->attestation_path) == 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.sidecar_paths_distinct = 1;
    capture_target = nexus_v1_read_transform_trace_sidecar(
        paths->capture_target_path, &capture_target_size);
    manifest = nexus_v1_read_transform_trace_sidecar(paths->manifest_path,
                                                      &manifest_size);
    raw_trace = nexus_v1_read_transform_trace_sidecar(paths->raw_trace_path,
                                                       &raw_trace_size);
    transform_state = nexus_v1_read_transform_trace_sidecar(
        paths->transform_state_path, &transform_state_size);
    {
        size_t attestation_size = 0U;
        attestation = nexus_v1_read_transform_trace_sidecar(paths->attestation_path,
                                                             &attestation_size);
        if (attestation) {
            receipt.attestation_bytes_read = 1;
            (void)nexus_v1_engine_parse_structure1f_transform_trace_attestation(
                engine, structure1f_entry_index, (const char *)attestation,
                attestation_size, raw_trace, raw_trace_size, transform_state,
                transform_state_size, &receipt.attestation);
        }
    }
    receipt.capture_target_bytes_read = capture_target != NULL;
    if (capture_target) {
        (void)nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
            engine, structure1f_entry_index, (const char *)capture_target,
            capture_target_size, &receipt.capture_target);
    }
    receipt.manifest_bytes_read = manifest != NULL;
    receipt.raw_trace_bytes_read = raw_trace != NULL;
    receipt.transform_state_bytes_read = transform_state != NULL;
    if (receipt.capture_target.status ==
            NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW &&
        manifest && raw_trace && transform_state &&
        receipt.attestation.original_saturn_source_attested) {
        (void)nexus_v1_engine_admit_structure1f_transform_capture_trace(
            engine, structure1f_entry_index, (const char *)manifest,
            manifest_size, raw_trace, raw_trace_size, transform_state,
            transform_state_size,
            receipt.attestation.original_saturn_source_attested,
            &receipt.admission);
    }
    free(attestation);
    free(transform_state);
    free(raw_trace);
    free(manifest);
    free(capture_target);
    *out_receipt = receipt;
    return receipt.admission.opaque_trace_admitted;
}

int nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget target;

    if (!out_target) return -1;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    /* Real LEV00..15 selectors exceed the 15-entry MNS TEXT banks. Until
     * Saturn proves the selector transform, this API must not claim a direct
     * Structure1F-to-Structure2 material owner. The geometry/transform
     * capture target remains available through the separate no-draw route. */
    if (!engine ||
        !engine->dgn_static_material_sources.structure1b_selector_binding_proven ||
        nexus_v1_engine_build_structure1f_direct_mesh_binding(
            engine, structure1f_entry_index, &target.direct_mesh) != 1 ||
        !target.direct_mesh.valid || !target.direct_mesh.model_to_entry_proven ||
        !target.direct_mesh.face_ordinal_proven ||
        !target.direct_mesh.no_draw_only ||
        target.direct_mesh.fallback_visuals_permitted ||
        nexus_v1_engine_build_structure3_static_material_capture_target(
            engine, target.direct_mesh.structure3_model_index,
            target.direct_mesh.face_ordinal, &target.static_material) != 1 ||
        !target.static_material.valid ||
        !target.static_material.static_selector_descriptor_bound ||
        !target.static_material.image_payload_anchor_bound ||
        !target.static_material.image_payload_interval_bound ||
        !target.static_material.capture_producer_required ||
        !target.static_material.original_saturn_capture_required ||
        !target.static_material.no_draw_only ||
        target.static_material.fallback_visuals_permitted ||
        target.static_material.level_index != target.direct_mesh.level_index ||
        target.static_material.source_byte_count != target.direct_mesh.source_byte_count ||
        target.static_material.source_bytes_fnv1a64 !=
            target.direct_mesh.source_bytes_fnv1a64 ||
        target.static_material.structure3_entry_index !=
            target.direct_mesh.structure3_model_index ||
        target.static_material.face_ordinal != target.direct_mesh.face_ordinal) {
        *out_target = target;
        return 0;
    }
    target.direct_face_material_bound = 1;
    target.capture_producer_required = 1;
    target.original_saturn_capture_required = 1;
    target.valid = 1;
    *out_target = target;
    return 1;
}

int nexus_v1_engine_build_structure1f2_face_adjacency_transform_receipt(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt receipt;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0 ||
        nexus_v1_current_level_lookup_structure1f_source_entry(
            engine, structure1f_entry_index, &receipt.structure1f_source) != 1 ||
        nexus_v1_engine_build_structure1f_transform_capture_target(
            engine, structure1f_entry_index, &receipt.transform_target) != 1 ||
        nexus_v1_current_level_structure2_source_receipt(
            engine, &receipt.structure2_source) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    if (!receipt.structure1f_source.valid ||
        receipt.structure1f_source.entry_index != structure1f_entry_index ||
        !receipt.structure1f_source.descriptor_length ||
        !receipt.structure1f_source.descriptor_fnv1a64 ||
        receipt.structure1f_source.package_fnv1a64 == 0U ||
        receipt.structure1f_source.package_fnv1a64 !=
            receipt.structure1f_source.source_bytes_fnv1a64 ||
        receipt.structure1f_source.source_byte_count !=
            engine->current_level_dgn_size ||
        receipt.structure1f_source.descriptor_offset >
            (uint32_t)engine->current_level_dgn_size ||
        receipt.structure1f_source.descriptor_length >
            (uint32_t)engine->current_level_dgn_size -
                receipt.structure1f_source.descriptor_offset ||
        nexus_v1_dgn_bytes_fnv1a64(
            engine->current_level_dgn_data +
                receipt.structure1f_source.descriptor_offset,
            (int)receipt.structure1f_source.descriptor_length) !=
            receipt.structure1f_source.descriptor_fnv1a64 ||
        !receipt.transform_target.valid ||
        !receipt.transform_target.geometry.source_geometry_bound ||
        !receipt.transform_target.transform_table_source_bound ||
        !receipt.transform_target.owner_transform_selector_source_bound ||
        !receipt.transform_target.no_draw_only ||
        receipt.transform_target.fallback_visuals_permitted ||
        receipt.structure2_source.level_index != engine->game.current_level ||
        !receipt.structure2_source.canonical_hash_verified ||
        !receipt.structure2_source.materialization_bound ||
        !receipt.structure2_source.loaded_bytes_bound ||
        !receipt.structure2_source.structure2_payload_envelope_valid ||
        receipt.structure2_source.loaded_dgn_size != engine->current_level_dgn_size ||
        receipt.structure2_source.loaded_dgn_fnv1a64 !=
            receipt.structure1f_source.package_fnv1a64 ||
        receipt.transform_target.geometry.direct_mesh.structure1f_entry_index !=
            structure1f_entry_index ||
        receipt.transform_target.geometry.direct_mesh.structure3_model_index !=
            receipt.transform_target.geometry.direct_mesh.face_target.candidate
                .entry_index ||
        receipt.transform_target.geometry.direct_mesh.face_ordinal !=
            receipt.transform_target.geometry.direct_mesh.face_target.candidate
                .face_ordinal) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_record_source_bound = 1;
    receipt.face_mesh_adjacency_bound = 1;
    receipt.structure2_source_envelope_bound = 1;
    receipt.owner_transform_selector_bound = 1;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *receipt)
{
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt current;

    if (!receipt || !receipt->valid || !receipt->structure1f_record_source_bound ||
        !receipt->face_mesh_adjacency_bound ||
        !receipt->structure2_source_envelope_bound ||
        !receipt->owner_transform_selector_bound ||
        receipt->transform_semantics_proven || !receipt->no_draw_only ||
        receipt->fallback_visuals_permitted ||
        !receipt->blocks_real_dgn_mesh_render ||
        !receipt->structure1f_source.no_draw_only ||
        !receipt->transform_target.no_draw_only ||
        !receipt->structure2_source.structure2_payload_envelope_valid ||
        receipt->structure1f_source.source_byte_count <= 0 ||
        receipt->structure1f_source.descriptor_offset >
            (uint32_t)receipt->structure1f_source.source_byte_count ||
        receipt->structure1f_source.descriptor_length >
            (uint32_t)receipt->structure1f_source.source_byte_count -
                receipt->structure1f_source.descriptor_offset) return 0;
    memset(&current, 0, sizeof(current));
    if (nexus_v1_engine_build_structure1f2_face_adjacency_transform_receipt(
            engine, receipt->structure1f_source.entry_index, &current) != 1 ||
        !current.valid ||
        current.structure1f_source.package_fnv1a64 !=
            receipt->structure1f_source.package_fnv1a64 ||
        current.structure1f_source.descriptor_offset !=
            receipt->structure1f_source.descriptor_offset ||
        current.structure1f_source.descriptor_length !=
            receipt->structure1f_source.descriptor_length ||
        current.structure1f_source.descriptor_fnv1a64 !=
            receipt->structure1f_source.descriptor_fnv1a64 ||
        current.transform_target.geometry.direct_mesh.structure3_model_index !=
            receipt->transform_target.geometry.direct_mesh.structure3_model_index ||
        current.transform_target.geometry.direct_mesh.face_ordinal !=
            receipt->transform_target.geometry.direct_mesh.face_ordinal ||
        current.transform_target.geometry.direct_mesh.z_rotation !=
            receipt->transform_target.geometry.direct_mesh.z_rotation ||
        current.structure2_source.loaded_dgn_size !=
            receipt->structure2_source.loaded_dgn_size ||
        current.structure2_source.loaded_dgn_fnv1a64 !=
            receipt->structure2_source.loaded_dgn_fnv1a64) return 0;
    return 1;
}

int nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget target;

    if (!out_target) return -1;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    if (nexus_v1_engine_build_structure1f_direct_mesh_binding(
            engine, structure1f_entry_index, &target.direct_mesh) != 1 ||
        !target.direct_mesh.valid || !target.direct_mesh.model_to_entry_proven ||
        !target.direct_mesh.face_ordinal_proven ||
        !target.direct_mesh.no_draw_only ||
        target.direct_mesh.fallback_visuals_permitted ||
        nexus_v1_current_level_structure3_untextured_face_packet(
            engine, target.direct_mesh.structure3_model_index,
            target.direct_mesh.face_ordinal, &target.untextured_face) != 1 ||
        !target.untextured_face.valid || !target.untextured_face.source_geometry_bound ||
        !target.untextured_face.raw_fill_bound ||
        !target.untextured_face.no_draw_only ||
        target.untextured_face.fallback_visuals_permitted ||
        !target.untextured_face.blocks_real_dgn_mesh_render ||
        target.untextured_face.level_index != target.direct_mesh.level_index ||
        target.untextured_face.source_byte_count != target.direct_mesh.source_byte_count ||
        target.untextured_face.source_bytes_fnv1a64 !=
            target.direct_mesh.source_bytes_fnv1a64 ||
        target.untextured_face.structure3_entry_index !=
            target.direct_mesh.structure3_model_index ||
        target.untextured_face.face_ordinal != target.direct_mesh.face_ordinal) {
        *out_target = target;
        return 0;
    }
    target.direct_face_untextured_bound = 1;
    target.capture_producer_required = 1;
    target.original_saturn_capture_required = 1;
    target.valid = 1;
    *out_target = target;
    return 1;
}

int nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget *out_target)
{
    Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget target;

    if (!out_target) return -1;
    memset(&target, 0, sizeof(target));
    target.no_draw_only = 1;
    target.blocks_real_dgn_mesh_render = 1;
    if (nexus_v1_engine_build_structure1f_direct_mesh_binding(
            engine, structure1f_entry_index, &target.direct_mesh) != 1 ||
        !target.direct_mesh.valid || !target.direct_mesh.model_to_entry_proven ||
        !target.direct_mesh.face_ordinal_proven ||
        !target.direct_mesh.no_draw_only ||
        target.direct_mesh.fallback_visuals_permitted ||
        nexus_v1_current_level_structure3_animated_material_packet(
            engine, target.direct_mesh.structure3_model_index,
            target.direct_mesh.face_ordinal, &target.animated_material) != 1 ||
        !target.animated_material.valid ||
        !target.animated_material.source_geometry_bound ||
        !target.animated_material.animation_declaration_bound ||
        !target.animated_material.no_draw_only ||
        target.animated_material.fallback_visuals_permitted ||
        !target.animated_material.blocks_real_dgn_mesh_render ||
        target.animated_material.level_index != target.direct_mesh.level_index ||
        target.animated_material.source_byte_count !=
            target.direct_mesh.source_byte_count ||
        target.animated_material.source_bytes_fnv1a64 !=
            target.direct_mesh.source_bytes_fnv1a64 ||
        target.animated_material.structure3_entry_index !=
            target.direct_mesh.structure3_model_index ||
        target.animated_material.face_ordinal != target.direct_mesh.face_ordinal) {
        *out_target = target;
        return 0;
    }
    target.direct_face_animated_material_bound = 1;
    target.capture_producer_required = 1;
    target.original_saturn_capture_required = 1;
    target.valid = 1;
    *out_target = target;
    return 1;
}

int nexus_v1_current_level_aux_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxRuntimeReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->level_aux_runtime_receipt;
    return 0;
}

int nexus_v1_current_level_smap_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_SmapRuntimeReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->level_index = -1;
    out_receipt->no_draw_only = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->smap_runtime_receipt;
    return 0;
}

int nexus_v1_current_level_aux_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxAdmissionReceipt *out_receipt)
{
    const Nexus_V1_LevelAuxRuntimeReceipt *aux;
    const Nexus_ScriptRuntimeReceipt *script;
    const Nexus_SfxRuntimeReceipt *sound;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_LEVEL_AUX_ADMISSION_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_script_dispatch = 1;
    out_receipt->blocks_real_sfx_playback = 1;
    if (!engine || !engine->level_loaded) return 0;

    aux = &engine->level_aux_runtime_receipt;
    script = &engine->script_runtime_receipt;
    sound = &engine->sfx_runtime_receipt;
    out_receipt->level_index = aux->level_index;
    out_receipt->canonical_sources_bound = aux->canonical_pair_bound &&
        aux->slev.canonical_hash_verified && aux->sal.canonical_hash_verified &&
        aux->map.canonical_hash_verified &&
        aux->sound_driver.canonical_hash_verified &&
        !aux->fallback_visuals_permitted;
    if (aux->level_index < 0 || aux->level_index != script->level_index ||
        aux->level_index != sound->level_index ||
        !out_receipt->canonical_sources_bound) {
        out_receipt->status = NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOURCE;
        return 0;
    }
    out_receipt->slev_task_profile_bound =
        script->status == NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT &&
        script->real_task_profile_supported && script->real_task_header_supported &&
        !script->dispatch_enabled && script->blocks_real_script_dispatch &&
        !script->fallback_visuals_permitted;
    if (!out_receipt->slev_task_profile_bound) {
        out_receipt->status = NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SCRIPT;
        return 0;
    }
    out_receipt->sal_map_profile_bound =
        sound->status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
        sound->sal_loaded && sound->map_loaded &&
        sound->sal_canonical_source_verified &&
        sound->map_canonical_source_verified &&
        !sound->sal_decode_supported && !sound->map_decode_supported;
    out_receipt->sound_driver_bound = sound->sound_driver_canonical_source_verified;
    if (!out_receipt->sal_map_profile_bound || !out_receipt->sound_driver_bound ||
        sound->playback_enabled || !sound->blocks_real_sfx_playback ||
        sound->fallback_visuals_permitted) {
        out_receipt->status = NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOUND;
        return 0;
    }
    out_receipt->status = NEXUS_V1_LEVEL_AUX_ADMISSION_READY_NO_RUNTIME;
    out_receipt->no_runtime_only = 1;
    return 1;
}

int nexus_v1_current_level_sound_route_receipt(
    const Nexus_V1_Engine *engine, int raw_map_selector,
    Nexus_V1_LevelSoundRouteReceipt *out_receipt) {
    Nexus_SoundMapWindow window;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_LEVEL_SOUND_ROUTE_MISSING;
    out_receipt->level_index = -1;
    out_receipt->raw_map_selector = raw_map_selector;
    out_receipt->sal_offset = -1;
    out_receipt->blocks_real_sfx_playback = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded || raw_map_selector < 0 ||
        raw_map_selector > 0xff) {
        return 0;
    }

    out_receipt->level_index = engine->level_aux_runtime_receipt.level_index;
    out_receipt->canonical_sal_source_verified =
        engine->level_aux_runtime_receipt.sal.canonical_hash_verified;
    out_receipt->canonical_map_source_verified =
        engine->level_aux_runtime_receipt.map.canonical_hash_verified;
    out_receipt->canonical_sound_driver_source_verified =
        engine->level_aux_runtime_receipt.sound_driver.canonical_hash_verified;
    if (!out_receipt->canonical_sal_source_verified ||
        !out_receipt->canonical_map_source_verified ||
        !out_receipt->canonical_sound_driver_source_verified ||
        engine->audio.current_level != out_receipt->level_index) {
        out_receipt->status = NEXUS_V1_LEVEL_SOUND_ROUTE_BLOCKED_SOURCE;
        return 0;
    }

    memset(&window, 0, sizeof(window));
    if (nexus_sound_map_lookup_raw_selector(&engine->audio, raw_map_selector,
                                            &window) != 0) {
        out_receipt->status = NEXUS_V1_LEVEL_SOUND_ROUTE_BLOCKED_SELECTOR;
        return 0;
    }

    out_receipt->status = NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE;
    out_receipt->map_attribute = window.attribute;
    out_receipt->sal_offset = window.sal_offset;
    out_receipt->sal_size = window.sal_size;
    out_receipt->map_window_unique_and_bounded = 1;
    /* The raw selector is not a host event and SAL stays opaque. */
    out_receipt->saturn_event_dispatch_proven = 0;
    out_receipt->sal_decode_proven = 0;
    out_receipt->playback_permitted = 0;
    return 1;
}

int nexus_v1_engine_build_sal_capture_target(
    const Nexus_V1_Engine *engine, int raw_map_selector,
    Nexus_V1_LevelSoundCaptureTargetReceipt *out_target) {
    Nexus_V1_LevelSoundRouteReceipt route;
    const Nexus_V1_LevelAuxRuntimeReceipt *sources;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->level_index = -1;
    out_target->original_saturn_driver_capture_required = 1;
    out_target->no_playback_only = 1;
    if (nexus_v1_current_level_sound_route_receipt(engine, raw_map_selector,
                                                   &route) != 1 ||
        route.status != NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE ||
        !route.canonical_sal_source_verified ||
        !route.canonical_map_source_verified ||
        !route.canonical_sound_driver_source_verified ||
        route.saturn_event_dispatch_proven || route.sal_decode_proven ||
        route.playback_permitted || !engine ||
        !engine->audio.sal_source_fnv1a64 ||
        !engine->audio.map_source_fnv1a64) {
        return 0;
    }
    sources = &engine->level_aux_runtime_receipt;
    if (sources->level_index != route.level_index ||
        !sources->sal.canonical_name[0] || !sources->sal.canonical_md5[0] ||
        !sources->map.canonical_name[0] || !sources->map.canonical_md5[0] ||
        !sources->sound_driver.canonical_name[0] ||
        !sources->sound_driver.canonical_md5[0]) {
        return 0;
    }
    out_target->level_index = route.level_index;
    snprintf(out_target->canonical_sal_name, sizeof(out_target->canonical_sal_name),
             "%s", sources->sal.canonical_name);
    snprintf(out_target->canonical_sal_md5, sizeof(out_target->canonical_sal_md5),
             "%s", sources->sal.canonical_md5);
    out_target->canonical_sal_fnv1a64 = engine->audio.sal_source_fnv1a64;
    snprintf(out_target->canonical_map_name, sizeof(out_target->canonical_map_name),
             "%s", sources->map.canonical_name);
    snprintf(out_target->canonical_map_md5, sizeof(out_target->canonical_map_md5),
             "%s", sources->map.canonical_md5);
    out_target->canonical_map_fnv1a64 = engine->audio.map_source_fnv1a64;
    snprintf(out_target->canonical_driver_name,
             sizeof(out_target->canonical_driver_name), "%s",
             sources->sound_driver.canonical_name);
    snprintf(out_target->canonical_driver_md5,
             sizeof(out_target->canonical_driver_md5), "%s",
             sources->sound_driver.canonical_md5);
    out_target->raw_map_selector = route.raw_map_selector;
    out_target->map_attribute = route.map_attribute;
    out_target->sal_offset = route.sal_offset;
    out_target->sal_size = route.sal_size;
    out_target->valid = 1;
    return 1;
}

int nexus_v1_engine_write_sal_capture_target(
    const Nexus_V1_Engine *engine, int raw_map_selector, const char *path,
    Nexus_V1_LevelSoundCaptureTargetReceipt *out_target) {
    char temporary_path[1024];
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    FILE *file;

    if (!path || !out_target ||
        nexus_v1_engine_build_sal_capture_target(engine, raw_map_selector,
                                                 &target) != 1 ||
        !target.valid || !target.original_saturn_driver_capture_required ||
        target.sal_decode_proven || target.playback_permitted ||
        !target.no_playback_only || target.fallback_visuals_permitted ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path)) {
        return 0;
    }
    file = fopen(temporary_path, "wb");
    if (!file) return 0;
    if (fprintf(file,
                NEXUS_V1_SAL_CAPTURE_TARGET_MAGIC "\n"
                "level_index=%x\ncanonical_sal_name=%s\ncanonical_sal_md5=%s\n"
                "canonical_sal_fnv1a64=%016llx\n"
                "canonical_map_name=%s\ncanonical_map_md5=%s\n"
                "canonical_map_fnv1a64=%016llx\n"
                "canonical_driver_name=%s\ncanonical_driver_md5=%s\n"
                "raw_map_selector=%x\nmap_attribute=%x\nsal_offset=%x\n"
                "sal_size=%x\ncapture_kind=original_saturn_sound_driver\n"
                "required_observations=selector_dispatch,sal_read,driver_output\n"
                "sal_decode_proven=0\nplayback_permitted=0\nno_playback_only=1\n",
                target.level_index, target.canonical_sal_name,
                target.canonical_sal_md5,
                (unsigned long long)target.canonical_sal_fnv1a64,
                target.canonical_map_name, target.canonical_map_md5,
                (unsigned long long)target.canonical_map_fnv1a64,
                target.canonical_driver_name,
                target.canonical_driver_md5, target.raw_map_selector,
                target.map_attribute, target.sal_offset, target.sal_size) < 0) {
        fclose(file);
        remove(temporary_path);
        return 0;
    }
    if (fclose(file) != 0 || rename(temporary_path, path) != 0) {
        remove(temporary_path);
        return 0;
    }
    *out_target = target;
    return 1;
}

int nexus_v1_current_level_script_route_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptRouteReceipt *out_receipt) {
    const Nexus_ScriptVM *vm;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_LEVEL_SCRIPT_ROUTE_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_script_dispatch = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;

    out_receipt->level_index = engine->level_aux_runtime_receipt.level_index;
    out_receipt->canonical_slev_source_verified =
        engine->level_aux_runtime_receipt.slev.canonical_hash_verified;
    vm = &engine->script_vm;
    if (!out_receipt->canonical_slev_source_verified ||
        !vm->candidate_source_loaded || !vm->canonical_source_verified ||
        vm->current_level != out_receipt->level_index) {
        out_receipt->status = NEXUS_V1_LEVEL_SCRIPT_ROUTE_BLOCKED_SOURCE;
        return 0;
    }

    out_receipt->candidate_source_bytes = vm->candidate_source_bytes;
    if (!vm->real_task_profile_supported || !vm->real_task_header_supported ||
        vm->real_task_header_size <= 0 || vm->real_task_word_count <= 0 ||
        vm->real_task_primary_literal_offset < vm->real_task_header_size ||
        vm->real_task_aux_literal_offset < vm->real_task_header_size) {
        out_receipt->status = NEXUS_V1_LEVEL_SCRIPT_ROUTE_BLOCKED_PROFILE;
        return 0;
    }

    out_receipt->status = NEXUS_V1_LEVEL_SCRIPT_ROUTE_BOUND_TASK_PROFILE;
    out_receipt->task_header_size = vm->real_task_header_size;
    out_receipt->task_word_count = vm->real_task_word_count;
    out_receipt->first_opcode = vm->real_task_first_opcode;
    out_receipt->setup_immediate = vm->real_task_setup_immediate;
    out_receipt->setup_immediate_provenance =
        vm->real_task_setup_immediate_provenance;
    out_receipt->primary_literal_offset = vm->real_task_primary_literal_offset;
    out_receipt->primary_literal_address = vm->real_task_primary_literal_address;
    out_receipt->primary_literal_provenance =
        vm->real_task_primary_literal_provenance;
    out_receipt->auxiliary_literal_offset = vm->real_task_aux_literal_offset;
    out_receipt->auxiliary_literal_address = vm->real_task_aux_literal_address;
    out_receipt->auxiliary_literal_provenance =
        vm->real_task_aux_literal_provenance;
    out_receipt->task_header_profile_bound = 1;
    /* Header framing is evidence, not a task interpreter or callback ABI. */
    out_receipt->saturn_task_dispatch_proven = 0;
    out_receipt->dispatch_permitted = 0;
    return 1;
}

int nexus_v1_engine_build_slev_capture_target(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptCaptureTargetReceipt *out_target) {
    Nexus_V1_LevelScriptRouteReceipt route;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->level_index = -1;
    out_target->original_saturn_execution_required = 1;
    out_target->no_dispatch_only = 1;
    out_target->fallback_visuals_permitted = 0;
    if (nexus_v1_current_level_script_route_receipt(engine, &route) != 1 ||
        route.status != NEXUS_V1_LEVEL_SCRIPT_ROUTE_BOUND_TASK_PROFILE ||
        !route.canonical_slev_source_verified ||
        route.saturn_task_dispatch_proven || route.dispatch_permitted ||
        !route.blocks_real_script_dispatch ||
        route.fallback_visuals_permitted || !engine ||
        !engine->script_vm.candidate_source_fnv1a64 ||
        engine->level_aux_runtime_receipt.slev.canonical_name[0] == '\0' ||
        engine->level_aux_runtime_receipt.slev.canonical_md5[0] == '\0') {
        return 0;
    }

    out_target->level_index = route.level_index;
    snprintf(out_target->canonical_slev_name,
             sizeof(out_target->canonical_slev_name), "%s",
             engine->level_aux_runtime_receipt.slev.canonical_name);
    snprintf(out_target->canonical_slev_md5,
             sizeof(out_target->canonical_slev_md5), "%s",
             engine->level_aux_runtime_receipt.slev.canonical_md5);
    out_target->source_byte_count = route.candidate_source_bytes;
    out_target->source_fnv1a64 =
        engine->script_vm.candidate_source_fnv1a64;
    out_target->task_header_size = route.task_header_size;
    out_target->first_opcode = route.first_opcode;
    out_target->setup_immediate = route.setup_immediate;
    out_target->primary_literal_offset = route.primary_literal_offset;
    out_target->primary_literal_address = route.primary_literal_address;
    out_target->auxiliary_literal_offset = route.auxiliary_literal_offset;
    out_target->auxiliary_literal_address = route.auxiliary_literal_address;
    out_target->task_body_dispatch_proven = 0;
    out_target->valid = 1;
    return 1;
}

int nexus_v1_engine_write_slev_capture_target(
    const Nexus_V1_Engine *engine, const char *path,
    Nexus_V1_LevelScriptCaptureTargetReceipt *out_target) {
    char temporary_path[1024];
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    FILE *file;

    if (!path || !out_target ||
        nexus_v1_engine_build_slev_capture_target(engine, &target) != 1 ||
        !target.valid || !target.original_saturn_execution_required ||
        target.task_body_dispatch_proven || !target.no_dispatch_only ||
        target.fallback_visuals_permitted ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path)) {
        return 0;
    }
    file = fopen(temporary_path, "wb");
    if (!file) return 0;
    if (fprintf(file,
                NEXUS_V1_SLEV_CAPTURE_TARGET_MAGIC "\n"
                "level_index=%x\ncanonical_slev_name=%s\n"
                "canonical_slev_md5=%s\nsource_byte_count=%x\n"
                "source_fnv1a64=%016llx\n"
                "task_header_size=%x\nfirst_opcode=%x\nsetup_immediate=%x\n"
                "primary_literal_offset=%x\nprimary_literal_address=%x\n"
                "auxiliary_literal_offset=%x\nauxiliary_literal_address=%x\n"
                "capture_kind=original_saturn_sh2_execution\n"
                "required_observations=entry_pc,task_body_transfer,callback_or_write\n"
                "task_body_dispatch_proven=0\nno_dispatch_only=1\n",
                target.level_index, target.canonical_slev_name,
                target.canonical_slev_md5, target.source_byte_count,
                (unsigned long long)target.source_fnv1a64,
                target.task_header_size, target.first_opcode,
                target.setup_immediate, target.primary_literal_offset,
                target.primary_literal_address, target.auxiliary_literal_offset,
                target.auxiliary_literal_address) < 0) {
        fclose(file);
        remove(temporary_path);
        return 0;
    }
    if (fclose(file) != 0) {
        remove(temporary_path);
        return 0;
    }
    if (rename(temporary_path, path) != 0) {
        remove(temporary_path);
        return 0;
    }
    *out_target = target;
    return 1;
}

static int nexus_v1_slev_trace_value(const char *text, size_t size,
                                     const char *key, char *out,
                                     size_t out_size) {
    const char *line = text;
    const char *end = text + size;
    size_t key_size;

    if (!text || !key || !out || out_size == 0) return 0;
    key_size = strlen(key);
    while (line < end) {
        const char *line_end = line;
        size_t value_size;
        while (line_end < end && *line_end != '\n' && *line_end != '\r')
            ++line_end;
        if ((size_t)(line_end - line) > key_size &&
            memcmp(line, key, key_size) == 0 && line[key_size] == '=') {
            value_size = (size_t)(line_end - line) - key_size - 1;
            if (value_size == 0 || value_size >= out_size) return 0;
            memcpy(out, line + key_size + 1, value_size);
            out[value_size] = '\0';
            return 1;
        }
        line = line_end;
        while (line < end && (*line == '\n' || *line == '\r')) ++line;
    }
    return 0;
}

static int nexus_v1_slev_trace_hex_u32(const char *text, uint32_t *out_value) {
    uint32_t value = 0;
    const char *cursor;

    if (!text || !text[0] || !out_value) return 0;
    for (cursor = text; *cursor; ++cursor) {
        uint32_t digit;
        if (*cursor >= '0' && *cursor <= '9') digit = (uint32_t)(*cursor - '0');
        else if (*cursor >= 'a' && *cursor <= 'f') digit = (uint32_t)(*cursor - 'a' + 10);
        else if (*cursor >= 'A' && *cursor <= 'F') digit = (uint32_t)(*cursor - 'A' + 10);
        else return 0;
        if (value > 0x0fffffffu) return 0;
        value = (value << 4) | digit;
    }
    *out_value = value;
    return 1;
}

static int nexus_v1_slev_trace_decimal_u32(const char *text,
                                           uint32_t *out_value) {
    uint32_t value = 0;
    const char *cursor;

    if (!text || !text[0] || !out_value) return 0;
    for (cursor = text; *cursor; ++cursor) {
        uint32_t digit;
        if (*cursor < '0' || *cursor > '9') return 0;
        digit = (uint32_t)(*cursor - '0');
        if (value > (UINT32_MAX - digit) / 10U) return 0;
        value = value * 10U + digit;
    }
    *out_value = value;
    return 1;
}

static int nexus_v1_slev_trace_is_sha256(const char *text) {
    int index;
    if (!text || strlen(text) != 64) return 0;
    for (index = 0; index < 64; ++index) {
        char value = text[index];
        if (!((value >= '0' && value <= '9') ||
              (value >= 'a' && value <= 'f'))) return 0;
    }
    return 1;
}

static int nexus_v1_slev_trace_hex_u64(const char *text,
                                       uint64_t *out_value);
static uint64_t nexus_v1_slev_trace_fnv1a64(const uint8_t *data,
                                            size_t size);
static int nexus_v1_slev_raw_trace_find(const uint8_t *data, size_t size,
                                        const char *needle, size_t *out_offset);

int nexus_v1_engine_admit_sal_driver_trace(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt) {
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    Nexus_V1_LevelSoundTraceAdmissionReceipt receipt;
    char value[96];
    char magic[96];
    uint32_t number;
    uint64_t hash;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SAL_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_sfx_playback = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !trace_text || trace_size == 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "raw_map_selector",
                                   value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &number) || number > 0xffU ||
        nexus_v1_engine_build_sal_capture_target(engine, (int)number,
                                                 &target) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = target.level_index;
    receipt.raw_map_selector = target.raw_map_selector;
    if (!nexus_v1_slev_trace_value(trace_text, trace_size, "magic", magic,
                                   sizeof(magic)) ||
        strcmp(magic, NEXUS_V1_SAL_TRACE_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "producer", value,
                                   sizeof(value)) ||
        strcmp(value, "mednafen-debugger") != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "trace_sha256",
                                   value, sizeof(value)) ||
        !nexus_v1_slev_trace_is_sha256(value)) {
        receipt.status = NEXUS_V1_SAL_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.mednafen_debugger_provenance = 1;
    receipt.trace_sha256_present = 1;
    if (!nexus_v1_slev_trace_value(trace_text, trace_size, "level_index", value,
                                   sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &number) ||
        (int)number != target.level_index ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_sal_name", value, sizeof(value)) ||
        strcmp(value, target.canonical_sal_name) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_sal_md5", value, sizeof(value)) ||
        strcmp(value, target.canonical_sal_md5) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_sal_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &hash) ||
        hash != target.canonical_sal_fnv1a64 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_map_name", value, sizeof(value)) ||
        strcmp(value, target.canonical_map_name) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_map_md5", value, sizeof(value)) ||
        strcmp(value, target.canonical_map_md5) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_map_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &hash) ||
        hash != target.canonical_map_fnv1a64 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_driver_name", value, sizeof(value)) ||
        strcmp(value, target.canonical_driver_name) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "canonical_driver_md5", value, sizeof(value)) ||
        strcmp(value, target.canonical_driver_md5) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "map_attribute", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &number) ||
        (int)number != target.map_attribute ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "sal_offset", value,
                                   sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &number) ||
        (int)number != target.sal_offset ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "sal_size", value,
                                   sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &number) ||
        (int)number != target.sal_size) {
        receipt.status = NEXUS_V1_SAL_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "selector_dispatch_pc", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.selector_dispatch_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "sal_read_pc", value,
                                   sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.sal_read_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "driver_output_pc",
                                   value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.driver_output_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "original_saturn_execution", value,
                                   sizeof(value)) || strcmp(value, "1") != 0 ||
        receipt.selector_dispatch_pc == 0U || receipt.sal_read_pc == 0U ||
        receipt.driver_output_pc == 0U) {
        receipt.status = NEXUS_V1_SAL_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.map_attribute = target.map_attribute;
    receipt.sal_offset = target.sal_offset;
    receipt.sal_size = target.sal_size;
    receipt.canonical_sal_fnv1a64 = target.canonical_sal_fnv1a64;
    receipt.canonical_map_fnv1a64 = target.canonical_map_fnv1a64;
    receipt.capture_target_bound = 1;
    receipt.original_saturn_execution_claimed = 1;
    receipt.trace_chain_complete = 1;
    receipt.driver_dispatch_proven = 0;
    receipt.sal_decode_proven = 0;
    receipt.playback_permitted = 0;
    receipt.status = NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE;
    engine->sound_trace_admission = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_sal_trace_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_SAL_TRACE_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_sfx_playback = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->sound_trace_admission;
    return 0;
}

int nexus_v1_engine_admit_sal_driver_trace_with_raw(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt) {
    Nexus_V1_LevelSoundTraceAdmissionReceipt receipt;
    char value[96];
    uint64_t expected_hash;
    uint64_t actual_hash;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SAL_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_sfx_playback = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !raw_trace || raw_trace_size == 0 || !trace_text ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                   "raw_trace_fnv1a64", value,
                                   sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        !(actual_hash = nexus_v1_slev_trace_fnv1a64(raw_trace,
                                                     raw_trace_size)) ||
        actual_hash != expected_hash) {
        receipt.status = NEXUS_V1_SAL_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_engine_admit_sal_driver_trace(engine, trace_text, trace_size,
                                               &receipt) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bytes_bound = 1;
    receipt.raw_trace_fnv1a64 = actual_hash;
    receipt.raw_trace_byte_count = raw_trace_size;
    engine->sound_trace_admission = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_consume_sal_driver_trace(
    Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceHostReceipt *out_receipt) {
    Nexus_V1_LevelSoundTraceHostReceipt receipt;
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    const Nexus_V1_SalDispatchEvidenceReceipt *evidence;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SAL_TRACE_HOST_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_sfx_playback = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = engine->level_aux_runtime_receipt.level_index;
    trace = &engine->sound_trace_admission;
    evidence = &engine->sound_dispatch_evidence;
    if (trace->status != NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE ||
        trace->level_index != receipt.level_index ||
        !trace->capture_target_bound || !trace->mednafen_debugger_provenance ||
        !trace->original_saturn_execution_claimed || !trace->trace_sha256_present ||
        !trace->raw_trace_bytes_bound || !trace->raw_trace_fnv1a64 ||
        trace->raw_trace_byte_count == 0 ||
        !trace->trace_chain_complete || trace->driver_dispatch_proven ||
        trace->sal_decode_proven || trace->playback_permitted ||
        !trace->blocks_real_sfx_playback || trace->fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SAL_TRACE_HOST_BLOCKED_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    if (evidence->status != NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED ||
        evidence->level_index != receipt.level_index ||
        !evidence->raw_trace_bound || !evidence->selector_dispatch_observed ||
        evidence->raw_trace_fnv1a64 != trace->raw_trace_fnv1a64 ||
        evidence->raw_trace_byte_count != trace->raw_trace_byte_count ||
        !evidence->sal_read_observed || !evidence->driver_output_observed ||
        !evidence->observation_order_proven || evidence->driver_dispatch_proven ||
        evidence->sal_decode_proven || evidence->playback_permitted ||
        !evidence->blocks_real_sfx_playback ||
        evidence->fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SAL_TRACE_HOST_BLOCKED_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_engine_build_sal_capture_target(engine,
                                                 trace->raw_map_selector,
                                                 &target) != 1 ||
        !target.valid || target.level_index != trace->level_index ||
        target.canonical_sal_fnv1a64 != trace->canonical_sal_fnv1a64 ||
        target.canonical_map_fnv1a64 != trace->canonical_map_fnv1a64 ||
        target.map_attribute != trace->map_attribute ||
        target.sal_offset != trace->sal_offset || target.sal_size != trace->sal_size ||
        target.sal_decode_proven || target.playback_permitted ||
        !target.no_playback_only || target.fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SAL_TRACE_HOST_BLOCKED_ACTIVE_ROUTE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = NEXUS_V1_SAL_TRACE_HOST_CONSUMED_OPAQUE;
    receipt.active_sal_target_revalidated = 1;
    receipt.admitted_trace_bound = 1;
    receipt.raw_map_selector = trace->raw_map_selector;
    receipt.map_attribute = trace->map_attribute;
    receipt.sal_offset = trace->sal_offset;
    receipt.sal_size = trace->sal_size;
    receipt.selector_dispatch_pc = trace->selector_dispatch_pc;
    receipt.sal_read_pc = trace->sal_read_pc;
    receipt.driver_output_pc = trace->driver_output_pc;
    receipt.host_consumed = 1;
    engine->sound_trace_host_receipt = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_sal_trace_host_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceHostReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_SAL_TRACE_HOST_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_sfx_playback = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->sound_trace_host_receipt;
    return 0;
}

static uint64_t nexus_v1_slev_trace_fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash = 1469598103934665603ULL;
    size_t index;
    if (!data || size == 0) return 0;
    for (index = 0; index < size; ++index) {
        hash ^= data[index];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t nexus_v1_capture_target_hash_mix(uint64_t hash,
                                                  uint64_t value)
{
    int byte_index;
    for (byte_index = 0; byte_index < 8; ++byte_index) {
        hash ^= (value >> (byte_index * 8)) & 0xffU;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t nexus_v1_owner_material_capture_target_fnv1a64(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *owner;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;

    if (!target || !target->valid) return 0;
    owner = &target->owner_face_target;
    descriptor = &target->material_target.descriptor_target;
    hash = nexus_v1_capture_target_hash_mix(hash, (uint64_t)target->level_index);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            target->material_target.source_bytes_fnv1a64);
    hash = nexus_v1_capture_target_hash_mix(hash, (uint64_t)owner->owner_x);
    hash = nexus_v1_capture_target_hash_mix(hash, (uint64_t)owner->owner_y);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            (uint64_t)owner->structure1f_entry_index);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            (uint64_t)owner->structure1f_family);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure1f_tag);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure1f_face_selector);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure1a_index);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure1a_kind);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure3_model_index);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->z_rotation);
    hash = nexus_v1_capture_target_hash_mix(hash, owner->structure3_payload_fnv1a32);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.entry_index);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.face_ordinal);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.face_row_fnv1a32);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.referenced_vertex_rows_fnv1a32);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.normal_row_fnv1a32);
    hash = nexus_v1_capture_target_hash_mix(
        hash, owner->face_target.candidate.fill_selector);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            (uint64_t)descriptor->descriptor_index);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            descriptor->descriptor_bytes_fnv1a64);
    hash = nexus_v1_capture_target_hash_mix(hash,
                                            descriptor->image_payload_anchor_offset);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->image_payload_next_anchor_offset);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->image_payload_candidate_byte_count);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->image_payload_candidate_fnv1a64);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->palette_payload_candidate_bound);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->palette_payload_anchor_offset);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->palette_payload_next_anchor_offset);
    hash = nexus_v1_capture_target_hash_mix(
        hash, descriptor->palette_payload_candidate_byte_count);
    return nexus_v1_capture_target_hash_mix(
        hash, descriptor->palette_payload_candidate_fnv1a64);
}

static int nexus_v1_slev_trace_hex_u64(const char *text, uint64_t *out_value) {
    uint64_t value = 0;
    const char *cursor;
    int digits = 0;

    if (!text || !text[0] || !out_value) return 0;
    for (cursor = text; *cursor; ++cursor) {
        uint64_t digit;
        if (*cursor >= '0' && *cursor <= '9') digit = (uint64_t)(*cursor - '0');
        else if (*cursor >= 'a' && *cursor <= 'f') digit = (uint64_t)(*cursor - 'a' + 10);
        else if (*cursor >= 'A' && *cursor <= 'F') digit = (uint64_t)(*cursor - 'A' + 10);
        else return 0;
        if (++digits > 16 || value > 0x0fffffffffffffffULL) return 0;
        value = (value << 4) | digit;
    }
    *out_value = value;
    return 1;
}

int nexus_v1_engine_admit_structure2_descriptor_capture_trace(
    const Nexus_V1_Engine *engine, int descriptor_index,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnStructure2TraceAdmissionReceipt *out_receipt) {
    Nexus_V1_DgnStructure2DescriptorCaptureTarget target;
    Nexus_V1_DgnStructure2TraceAdmissionReceipt receipt;
    char value[96];
    uint32_t parsed_index;
    uint64_t expected_hash;
    uint64_t actual_hash;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_STRUCTURE2_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.descriptor_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!engine || !manifest_text || manifest_size == 0 || !raw_trace ||
        raw_trace_size == 0 ||
        nexus_v1_engine_build_structure2_descriptor_capture_target(
            engine, descriptor_index, &target) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = target.level_index;
    receipt.descriptor_index = target.descriptor_index;
    receipt.capture_target_bound = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "magic", value,
                                    sizeof(value)) ||
        strcmp(value, NEXUS_V1_STRUCTURE2_SATURN_RAW_TRACE_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "producer", value,
                                    sizeof(value)) ||
        (strcmp(value, "external-saturn-capture") != 0 &&
         strcmp(value, "mednafen-debugger") != 0)) {
        receipt.status = NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "level_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        (int)parsed_index != target.level_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "descriptor_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        (int)parsed_index != target.descriptor_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "source_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.source_bytes_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "descriptor_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.descriptor_bytes_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "opaque_payload_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.opaque_payload_fnv1a64) {
        receipt.status = NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    if (!target.image_payload_candidate_bound ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "image_anchor_offset", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        parsed_index != target.image_payload_anchor_offset ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "image_next_anchor_offset", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        parsed_index != target.image_payload_next_anchor_offset ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "image_candidate_byte_count", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        parsed_index != target.image_payload_candidate_byte_count ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "image_candidate_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.image_payload_candidate_fnv1a64 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palette_candidate_present", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
        (parsed_index != 0U) != (target.palette_payload_candidate_bound != 0) ||
        (target.palette_payload_candidate_bound &&
         (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                      "palette_anchor_offset", value, sizeof(value)) ||
          !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
          parsed_index != target.palette_payload_anchor_offset ||
          !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                      "palette_next_anchor_offset", value, sizeof(value)) ||
          !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
          parsed_index != target.palette_payload_next_anchor_offset ||
          !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                      "palette_candidate_byte_count", value, sizeof(value)) ||
          !nexus_v1_slev_trace_hex_u32(value, &parsed_index) ||
          parsed_index != target.palette_payload_candidate_byte_count ||
          !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                      "palette_candidate_fnv1a64", value, sizeof(value)) ||
          !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
          expected_hash != target.palette_payload_candidate_fnv1a64))) {
        receipt.status = NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_target_bound = 1;
    receipt.image_payload_candidate_bound = 1;
    receipt.palette_payload_candidate_bound =
        target.palette_payload_candidate_bound;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "raw_trace_size", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != (uint64_t)raw_trace_size ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "raw_trace_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        !(actual_hash = nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size)) ||
        actual_hash != expected_hash) {
        receipt.status = NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_RAW_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bytes_bound = 1;
    receipt.raw_trace_byte_count = raw_trace_size;
    receipt.raw_trace_fnv1a64 = actual_hash;
    if (!original_saturn_capture_verified) {
        receipt.status = NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_PROVENANCE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.original_saturn_capture_verified = 1;
    receipt.opaque_trace_admitted = 1;
    receipt.status = NEXUS_V1_STRUCTURE2_TRACE_ADMITTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_admit_structure1a_structure3_material_capture_trace(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget target;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt target_route;
    Nexus_V1_DgnStructure2TraceAdmissionReceipt structure2_receipt;
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt receipt;
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *owner;
    char value[96];
    uint32_t parsed_u32;
    uint64_t parsed_u64;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_OWNER_MATERIAL_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.descriptor_index = -1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&target, 0, sizeof(target));
    memset(&target_route, 0, sizeof(target_route));
    if (!engine || !manifest_text || manifest_size == 0 || !raw_trace ||
        raw_trace_size == 0 ||
        nexus_v1_engine_build_structure1a_structure3_material_capture_target(
            engine, topology_candidate_index, structure3_entry_index,
            structure3_face_ordinal, &target, &target_route) != 1) {
        receipt.status = NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_BUNDLE;
        *out_receipt = receipt;
        return 0;
    }
    owner = &target.owner_face_target;
    receipt.level_index = target.level_index;
    receipt.descriptor_index = target.material_target.descriptor_target
        .descriptor_index;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "capture_target_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &parsed_u64) ||
        parsed_u64 != nexus_v1_owner_material_capture_target_fnv1a64(&target) ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "owner_x",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)owner->owner_x ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "owner_y",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)owner->owner_y ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1f_entry_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != (uint32_t)owner->structure1f_entry_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure1a_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != owner->structure1a_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "structure3_entry_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != owner->face_target.candidate.entry_index ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "face_ordinal", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != owner->face_target.candidate.face_ordinal ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "face_row_fnv1a32", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        parsed_u32 != owner->face_target.candidate.face_row_fnv1a32 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "descriptor_index", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &parsed_u32) ||
        (int)parsed_u32 != receipt.descriptor_index) {
        receipt.status = NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_TARGET;
        *out_receipt = receipt;
        return 0;
    }
    receipt.atomic_target_bound = 1;
    receipt.owner_face_bound = 1;
    memset(&structure2_receipt, 0, sizeof(structure2_receipt));
    if (nexus_v1_engine_admit_structure2_descriptor_capture_trace(
            engine, receipt.descriptor_index, manifest_text, manifest_size,
            raw_trace, raw_trace_size, original_saturn_capture_verified,
            &structure2_receipt) != 1) {
        receipt.original_saturn_capture_verified =
            structure2_receipt.original_saturn_capture_verified;
        receipt.status = NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_STRUCTURE2;
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure2_trace_admitted = structure2_receipt.opaque_trace_admitted;
    receipt.original_saturn_capture_verified =
        structure2_receipt.original_saturn_capture_verified;
    receipt.opaque_trace_admitted = receipt.structure2_trace_admitted;
    receipt.status = NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_admit_slev_execution_trace(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt) {
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    Nexus_V1_LevelScriptTraceAdmissionReceipt receipt;
    char value[96];
    char magic[96];
    char callback_kind[16];
    uint32_t source_byte_count;
    uint32_t entry_opcode;
    uint64_t source_fnv1a64;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SLEV_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !trace_text || trace_size == 0 ||
        nexus_v1_engine_build_slev_capture_target(engine, &target) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = target.level_index;
    if (!nexus_v1_slev_trace_value(trace_text, trace_size, "magic", magic,
                                    sizeof(magic)) ||
        strcmp(magic, NEXUS_V1_SLEV_TRACE_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "producer", value,
                                   sizeof(value)) ||
        strcmp(value, "mednafen-debugger") != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "trace_sha256",
                                   value, sizeof(value)) ||
        !nexus_v1_slev_trace_is_sha256(value)) {
        receipt.status = NEXUS_V1_SLEV_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.mednafen_debugger_provenance = 1;
    receipt.trace_sha256_present = 1;
    if (!nexus_v1_slev_trace_value(trace_text, trace_size, "level_index", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &source_byte_count) ||
        (int)source_byte_count != target.level_index ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "canonical_slev_name", value,
                                    sizeof(value)) ||
        strcmp(value, target.canonical_slev_name) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "canonical_slev_md5",
                                    value, sizeof(value)) ||
        strcmp(value, target.canonical_slev_md5) != 0 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "source_byte_count",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &source_byte_count) ||
        (int)source_byte_count != target.source_byte_count ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "source_fnv1a64",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &source_fnv1a64) ||
        source_fnv1a64 != target.source_fnv1a64 ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "entry_opcode", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &entry_opcode) ||
        (int)entry_opcode != target.first_opcode ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "task_header_size",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &source_byte_count) ||
        (int)source_byte_count != target.task_header_size ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "primary_literal_address", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &source_byte_count) ||
        (int)source_byte_count != target.primary_literal_address ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "auxiliary_literal_address", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &source_byte_count) ||
        (int)source_byte_count != target.auxiliary_literal_address) {
        receipt.status = NEXUS_V1_SLEV_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    if (!nexus_v1_slev_trace_value(trace_text, trace_size, "entry_pc", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.entry_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "task_body_pc", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.task_body_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "task_body_opcode",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.task_body_opcode) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "callback_or_write_pc", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u32(value, &receipt.callback_or_write_pc) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "callback_or_write_kind", callback_kind,
                                    sizeof(callback_kind)) ||
        (strcmp(callback_kind, "callback") != 0 &&
         strcmp(callback_kind, "write") != 0) ||
        !nexus_v1_slev_trace_value(trace_text, trace_size,
                                    "original_saturn_execution", value,
                                    sizeof(value)) || strcmp(value, "1") != 0 ||
        receipt.entry_pc == 0 || receipt.task_body_pc == 0 ||
        receipt.task_body_pc == receipt.entry_pc || receipt.task_body_opcode == 0 ||
        receipt.callback_or_write_pc == 0) {
        receipt.status = NEXUS_V1_SLEV_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.callback_or_write_is_write = strcmp(callback_kind, "write") == 0;
    receipt.source_fnv1a64 = source_fnv1a64;
    receipt.original_saturn_execution_claimed = 1;
    receipt.capture_target_bound = 1;
    receipt.trace_chain_complete = 1;
    receipt.task_body_dispatch_proven = 0;
    receipt.dispatch_permitted = 0;
    receipt.status = NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE;
    engine->script_trace_admission = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_engine_admit_slev_execution_trace_with_raw(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt) {
    Nexus_V1_LevelScriptTraceAdmissionReceipt receipt;
    Nexus_V1_LevelScriptTraceAdmissionReceipt previous;
    char value[96];
    uint64_t expected_hash;
    uint64_t actual_hash;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SLEV_TRACE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !raw_trace || raw_trace_size == 0 || !trace_text ||
        !nexus_v1_slev_trace_value(trace_text, trace_size, "raw_trace_fnv1a64",
                                   value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        !(actual_hash = nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size)) ||
        actual_hash != expected_hash) {
        receipt.status = NEXUS_V1_SLEV_TRACE_BLOCKED_RAW_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    previous = engine->script_trace_admission;
    if (nexus_v1_engine_admit_slev_execution_trace(engine, trace_text, trace_size,
                                                    &receipt) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bytes_bound = 1;
    receipt.raw_trace_fnv1a64 = actual_hash;
    receipt.raw_trace_byte_count = raw_trace_size;
    engine->script_trace_admission = receipt;
    *out_receipt = receipt;
    (void)previous;
    return 1;
}

int nexus_v1_current_level_slev_trace_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_SLEV_TRACE_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_script_dispatch = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->script_trace_admission;
    return 0;
}

int nexus_v1_engine_consume_slev_execution_trace(
    Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceHostReceipt *out_receipt) {
    Nexus_V1_LevelScriptTraceHostReceipt receipt;
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    const Nexus_V1_LevelScriptTraceAdmissionReceipt *trace;
    const Nexus_V1_SlevDispatchEvidenceReceipt *evidence;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SLEV_TRACE_HOST_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.level_index = engine->level_aux_runtime_receipt.level_index;
    trace = &engine->script_trace_admission;
    evidence = &engine->script_dispatch_evidence;
    if (trace->status != NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE ||
        trace->level_index != receipt.level_index ||
        !trace->capture_target_bound || !trace->mednafen_debugger_provenance ||
        !trace->original_saturn_execution_claimed || !trace->trace_sha256_present ||
        !trace->raw_trace_bytes_bound || !trace->raw_trace_fnv1a64 ||
        trace->raw_trace_byte_count == 0 ||
        !trace->trace_chain_complete || trace->task_body_dispatch_proven ||
        trace->dispatch_permitted || !trace->blocks_real_script_dispatch ||
        trace->fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    if (evidence->status != NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED ||
        evidence->level_index != receipt.level_index ||
        !evidence->raw_trace_bound || !evidence->entry_observed ||
        evidence->raw_trace_fnv1a64 != trace->raw_trace_fnv1a64 ||
        evidence->raw_trace_byte_count != trace->raw_trace_byte_count ||
        !evidence->task_body_observed ||
        !evidence->callback_or_write_observed ||
        !evidence->primary_literal_observed ||
        !evidence->auxiliary_literal_observed ||
        !evidence->observation_order_proven ||
        !evidence->literal_observation_proven ||
        evidence->task_body_dispatch_proven || evidence->dispatch_permitted ||
        !evidence->blocks_real_script_dispatch ||
        evidence->fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_TRACE;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_engine_build_slev_capture_target(engine, &target) != 1 ||
        !target.valid || target.level_index != trace->level_index ||
        !target.source_fnv1a64 ||
        trace->source_fnv1a64 != target.source_fnv1a64 ||
        target.task_body_dispatch_proven || !target.no_dispatch_only ||
        target.fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_ACTIVE_ROUTE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE;
    receipt.active_slev_target_revalidated = 1;
    receipt.admitted_trace_bound = 1;
    receipt.entry_pc = trace->entry_pc;
    receipt.task_body_pc = trace->task_body_pc;
    receipt.task_body_opcode = trace->task_body_opcode;
    receipt.callback_or_write_pc = trace->callback_or_write_pc;
    receipt.callback_or_write_is_write = trace->callback_or_write_is_write;
    receipt.host_consumed = 1;
    receipt.task_body_dispatch_proven = 0;
    receipt.dispatch_permitted = 0;
    engine->script_trace_host_receipt = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_slev_trace_host_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceHostReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_SLEV_TRACE_HOST_MISSING;
    out_receipt->level_index = -1;
    out_receipt->blocks_real_script_dispatch = 1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded) return 0;
    *out_receipt = engine->script_trace_host_receipt;
    return 0;
}

static int nexus_v1_slev_raw_trace_find(const uint8_t *data, size_t size,
                                        const char *needle, size_t *out_offset) {
    size_t needle_size;
    size_t index;
    if (out_offset) *out_offset = 0;
    if (!data || !needle || !out_offset ||
        !(needle_size = strlen(needle)) || needle_size > size)
        return 0;
    for (index = 0; index <= size - needle_size; ++index) {
        if (memcmp(data + index, needle, needle_size) == 0) {
            *out_offset = index;
            return 1;
        }
    }
    return 0;
}

int nexus_v1_build_sal_dispatch_evidence(
    Nexus_V1_Engine *engine, const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_SalDispatchEvidenceReceipt *out_receipt) {
    Nexus_V1_SalDispatchEvidenceReceipt receipt;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    char selector_dispatch[64];
    char sal_read[64];
    char driver_output[64];

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_sfx_playback = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded || !raw_trace || raw_trace_size == 0) {
        *out_receipt = receipt;
        return 0;
    }
    trace = &engine->sound_trace_admission;
    receipt.level_index = trace->level_index;
    if (trace->status != NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE ||
        !trace->raw_trace_bytes_bound ||
        trace->raw_trace_byte_count != raw_trace_size ||
        trace->raw_trace_fnv1a64 !=
            nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size)) {
        receipt.status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_BLOCKED_RAW;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bound = 1;
    receipt.raw_trace_fnv1a64 = trace->raw_trace_fnv1a64;
    receipt.raw_trace_byte_count = trace->raw_trace_byte_count;
    snprintf(selector_dispatch, sizeof(selector_dispatch),
             "pc=%08x selector-dispatch", trace->selector_dispatch_pc);
    snprintf(sal_read, sizeof(sal_read), "pc=%08x sal-read",
             trace->sal_read_pc);
    snprintf(driver_output, sizeof(driver_output), "pc=%08x driver-output",
             trace->driver_output_pc);
    receipt.selector_dispatch_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, selector_dispatch,
        &receipt.selector_dispatch_raw_offset);
    receipt.sal_read_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, sal_read, &receipt.sal_read_raw_offset);
    receipt.driver_output_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, driver_output,
        &receipt.driver_output_raw_offset);
    receipt.observation_order_proven = receipt.selector_dispatch_observed &&
        receipt.sal_read_observed && receipt.driver_output_observed &&
        receipt.selector_dispatch_raw_offset < receipt.sal_read_raw_offset &&
        receipt.sal_read_raw_offset < receipt.driver_output_raw_offset;
    if (!receipt.observation_order_proven) {
        receipt.status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_BLOCKED_OBSERVATION;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED;
    engine->sound_dispatch_evidence = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_build_slev_dispatch_evidence(
    Nexus_V1_Engine *engine, const uint8_t *raw_trace,
    size_t raw_trace_size, Nexus_V1_SlevDispatchEvidenceReceipt *out_receipt) {
    Nexus_V1_SlevDispatchEvidenceReceipt receipt;
    const Nexus_V1_LevelScriptTraceAdmissionReceipt *trace;
    char entry[64];
    char task_body[64];
    char callback_or_write[64];
    char primary_literal[32];
    char auxiliary_literal[32];

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SLEV_DISPATCH_EVIDENCE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_script_dispatch = 1;
    receipt.fallback_visuals_permitted = 0;
    if (!engine || !engine->level_loaded || !raw_trace || raw_trace_size == 0) {
        *out_receipt = receipt;
        return 0;
    }
    trace = &engine->script_trace_admission;
    receipt.level_index = trace->level_index;
    if (trace->status != NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE ||
        !trace->raw_trace_bytes_bound ||
        trace->raw_trace_byte_count != raw_trace_size ||
        trace->raw_trace_fnv1a64 !=
            nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size)) {
        receipt.status = NEXUS_V1_SLEV_DISPATCH_EVIDENCE_BLOCKED_RAW;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bound = 1;
    receipt.raw_trace_fnv1a64 = trace->raw_trace_fnv1a64;
    receipt.raw_trace_byte_count = trace->raw_trace_byte_count;
    snprintf(entry, sizeof(entry), "pc=%08x opcode=%04x",
             trace->entry_pc, engine->script_vm.real_task_first_opcode);
    snprintf(task_body, sizeof(task_body), "pc=%08x opcode=%04x",
             trace->task_body_pc, trace->task_body_opcode);
    snprintf(callback_or_write, sizeof(callback_or_write), "pc=%08x kind=%s",
             trace->callback_or_write_pc,
             trace->callback_or_write_is_write ? "write" : "callback");
    snprintf(primary_literal, sizeof(primary_literal), "literal=%08x",
             engine->script_vm.real_task_primary_literal_address);
    snprintf(auxiliary_literal, sizeof(auxiliary_literal), "literal=%08x",
             engine->script_vm.real_task_aux_literal_address);
    receipt.entry_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, entry, &receipt.entry_raw_offset);
    receipt.task_body_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, task_body, &receipt.task_body_raw_offset);
    receipt.callback_or_write_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, callback_or_write,
        &receipt.callback_or_write_raw_offset);
    receipt.primary_literal_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, primary_literal,
        &receipt.primary_literal_raw_offset);
    receipt.auxiliary_literal_observed = nexus_v1_slev_raw_trace_find(
        raw_trace, raw_trace_size, auxiliary_literal,
        &receipt.auxiliary_literal_raw_offset);
    receipt.callback_or_write_is_write = trace->callback_or_write_is_write;
    receipt.observation_order_proven = receipt.entry_observed &&
        receipt.task_body_observed && receipt.callback_or_write_observed &&
        receipt.entry_raw_offset < receipt.task_body_raw_offset &&
        receipt.task_body_raw_offset < receipt.callback_or_write_raw_offset;
    receipt.literal_observation_proven = receipt.primary_literal_observed &&
        receipt.auxiliary_literal_observed;
    if (!receipt.observation_order_proven ||
        !receipt.literal_observation_proven) {
        receipt.status = NEXUS_V1_SLEV_DISPATCH_EVIDENCE_BLOCKED_OBSERVATION;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED;
    receipt.task_body_dispatch_proven = 0;
    receipt.dispatch_permitted = 0;
    engine->script_dispatch_evidence = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_dgn_static_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStaticMaterialSourceReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->fallback_visuals_permitted = 0;
    if (!engine) return 0;
    *out_receipt = engine->dgn_static_material_sources;
    return 0;
}

int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name) {
    int size = 0;
    uint8_t *data;
    const char *canonical_md5;

    if (!engine || !name || engine->model_count >= NEXUS_MAX_MODELS) return -1;

    /* DMDF magic and a requested filename are format evidence only.  The
     * creature model pool is a runtime route, so admit only the named retail
     * MNS identities already catalogued from the real Nexus corpus.  This
     * closes the old renamed/synthetic-DMDF signature fallback without
     * affecting the separate diagnostic material-container readers. */
    if (!nexus_path_has_ext(name, ".MNS") ||
        !(canonical_md5 = nexus_v1_known_file_md5(name))) {
        return -1;
    }

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) return -1;

    if (!nexus_v1_dgn_bytes_match_canonical_md5(data, size, canonical_md5)) {
        free(data);
        return -1;
    }

    if (!nexus_v1_dmdf_is_valid(data, size)) {
        free(data);
        return -1;
    }

    int idx = engine->model_count;
    int r = nexus_v1_dmdf_load(&engine->models[idx], data, size, name);
    free(data);
    if (r < 0) return -1;

    engine->model_count++;
    return idx;
}

/* nexus_v1_engine_level_change — handle pending level transition.
 * Called by the M11 layer after mechanics_tick signals a level change.
 * Source: DM1 CLIKMENU.C F0364 — stairs/chute level load. */
int nexus_v1_engine_level_change(Nexus_V1_Engine *engine, int *out_new_level) {
    if (!engine || !out_new_level) return -1;
    *out_new_level = engine->mechanics->pending_level_change;
    if (engine->mechanics->pending_level_change < 0) return -1;
    /* Load new level */
    int r = nexus_v1_load_level(engine, engine->mechanics->pending_level_change);
    if (r < 0) return r;
    /* Reset stairs registry for new level */
    nexus_stairs_init();
    nexus_teleporters_init();
    nexus_doors_init();
    /* Initialize party position for new level */
    nexus_v1_sync_dgn_runtime_pose(engine, engine->mechanics->pending_level_change,
                                   engine->mechanics->party_x,
                                   engine->mechanics->party_y,
                                   engine->mechanics->party_dir);
    engine->mechanics->map_index = engine->mechanics->pending_level_change;
    engine->mechanics->pending_level_change = -1;
    return 0;
}

void nexus_v1_tick(Nexus_V1_Engine *engine) {
    int redraw = 0;

    if (!engine || !engine->initialized) return;

    /* Nexus uses the same V1 tick rate as DM1 (55ms / 18.2 Hz).
     * Game logic tick: process input, movement, square events, creature AI,
     * combat, resource drain, and script VM.
     * Source: DM1 CLIKMENU.C:269-323 (step result + cooldown),
     * CLIKMENU.C F0366 (game loop tick). */
    redraw = nexus_mechanics_tick(engine->mechanics, engine);
    if (engine->mechanics) {
        nexus_v1_sync_dgn_runtime_pose(engine, engine->mechanics->map_index,
                                       engine->mechanics->party_x,
                                       engine->mechanics->party_y,
                                       engine->mechanics->party_dir);
    }

    /* Handle pending level change (stairs/chute/pit).
     * Loaded here so the level data is ready for next tick's render.
     * Source: DM1 CLIKMENU.C F0364 — load new dungeon on stairs step. */
    if (engine->mechanics &&
        (engine->source == NEXUS_SRC_NONE ||
         nexus_v1_action_semantics_proven()) &&
        engine->mechanics->pending_level_change >= 0) {
        int new_level = -1;
        if (nexus_v1_engine_level_change(engine, &new_level) == 0) {
            printf("Nexus: party moved to level %d\n", new_level);
        }
        redraw = 1;
    }

    /* Handle pending teleport (SDDRVS.TSK or square-triggered).
     * Teleport target already committed in mechanics state;
     * sound effect already played in mechanics_tick.
     * Source: DM1 DUNGEON.C teleporter processing. */
    if (engine->mechanics &&
        (engine->source == NEXUS_SRC_NONE ||
         nexus_v1_action_semantics_proven()) &&
        engine->mechanics->pending_teleport) {
        /* Teleport committed in mechanics_tick — just log it here */
        printf("Nexus: party teleported to (%d,%d) level %d\n",
               engine->mechanics->party_x, engine->mechanics->party_y,
               engine->mechanics->teleport_target_level);
        redraw = 1;
    }

    if (engine->game.game_started) {
        int ci;
        /* DM1-derived rest regeneration has no authenticated Saturn
         * dispatcher or HUD/VDP consumer yet; keep retail state immutable. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_rest_tick(&engine->rest, &engine->champions);
        }
        /* The DM1-shaped hunger helper mutates food, water and health.  PLRD
         * does not provide authenticated provisions, and the Saturn
         * start/save consumer is still uncaptured; keep the retail engine
         * immutable until that route is proven. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_hunger_tick(&engine->hunger, &engine->champions);
        }
        if (nexus_v1_action_semantics_proven()) {
            Nexus_SpawnEvent spawn_events[8];
            int nspawn = nexus_v1_spawners_tick(&engine->spawners,
                spawn_events, 8);
            int si;
            for (si = 0; si < nspawn; si++) {
                int cr = nexus_v1_creature_spawn_on_level(
                    &engine->creatures,
                    spawn_events[si].creature_type,
                    spawn_events[si].x, spawn_events[si].y,
                    0, spawn_events[si].level);
                if (cr >= 0)
                    engine->spawners.spawners[spawn_events[si].spawner_index]
                        .spawned_creature = cr;
            }
        }
        /* These bounded DM1-shaped state machines have no authenticated
         * Saturn dispatcher/timer owner yet.  Keep fixture behavior, but do
         * not mutate retail action, door, or trap state speculatively. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_action_timers_tick(&engine->action_timers);
            nexus_v1_door_tick(&engine->doors);
            nexus_v1_trap_tick(&engine->traps);
        }
        if (nexus_v1_action_semantics_proven()) {
            nexus_v1_creatures_tick(&engine->creatures,
                                    engine->mechanics->party_x,
                                    engine->mechanics->party_y,
                                    engine->current_level.squares,
                                    engine->mechanics->map_index);
        }
        if (nexus_v1_action_semantics_proven()) {
            Nexus_ProjectileHit hits[8];
            int nhits = nexus_v1_projectiles_tick(&engine->projectiles,
                                                   engine->current_level.squares,
                                                   hits, 8);
            int hi;
            for (hi = 0; hi < nhits; hi++) {
                if (!nexus_v1_action_semantics_proven()) continue;
                if (hits[hi].hit_x == engine->mechanics->party_x &&
                    hits[hi].hit_y == engine->mechanics->party_y) {
                    int li = engine->champions.leader_index;
                    if (li >= 0 && li < engine->champions.party_count) {
                        int idx = engine->champions.party[li];
                        if (idx >= 0 && idx < 4 && engine->champions.champions[idx].alive) {
                            engine->champions.champions[idx].health -= hits[hi].damage;
                            if (engine->champions.champions[idx].health <= 0) {
                                engine->champions.champions[idx].health = 0;
                                engine->champions.champions[idx].alive = 0;
                            }
                            nexus_v1_damage_display_add(&engine->damage_display,
                                                        idx, hits[hi].damage, NEXUS_DMG_TAKEN);
                        }
                    }
                } else {
                    nexus_v1_creature_manager_damage_at(&engine->creatures,
                                                        hits[hi].hit_x, hits[hi].hit_y,
                                                        hits[hi].damage);
                }
                {
                    int thrown_item = nexus_v1_throw_on_hit(&engine->thrown,
                                                            hits[hi].slot_index);
                    if (thrown_item >= 0)
                        nexus_floor_drop(hits[hi].hit_x, hits[hi].hit_y,
                                         thrown_item, 1);
                }
            }
        }
        /* Creature follow-up (death XP, script callbacks and spawner state)
         * is part of the same unbound Saturn action/event route as AI and
         * attacks.  Do not let a stale or externally seeded actor mutate
         * retail state after the primary actor loop was correctly blocked. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
          {
            int ci2;
            for (ci2 = 0; ci2 < engine->creatures.active_count; ci2++) {
                Nexus_Creature *cr = &engine->creatures.active[ci2];
                if (!cr->alive) continue;
                if (cr->level != engine->mechanics->map_index) continue;
                if (cr->type_index < 0) continue;
                {
                    int cdist = nexus_v1_creature_distance(cr->x, cr->y,
                        engine->mechanics->party_x, engine->mechanics->party_y);
                    int ranged = engine->creatures.types[cr->type_index].ranged_type;
                    if (nexus_v1_action_semantics_proven() && ranged > 0 &&
                        cdist > 1 && cdist <= 5 && cr->state == 2 &&
                        cr->ai_timer % 8 == 0) {
                        int dir = 0;
                        int dx = engine->mechanics->party_x - cr->x;
                        int dy = engine->mechanics->party_y - cr->y;
                        if (abs(dx) > abs(dy))
                            dir = (dx > 0) ? 1 : 3;
                        else
                            dir = (dy > 0) ? 2 : 0;
                        nexus_v1_projectile_spawn(&engine->projectiles,
                            NEXUS_PROJ_FIREBALL, cr->x, cr->y, dir,
                            engine->creatures.types[cr->type_index].attack, 2, -1);
                    }
                }
                if (cr->state != 3) continue;
                if (nexus_v1_creature_distance(cr->x, cr->y,
                        engine->mechanics->party_x, engine->mechanics->party_y) <= 1) {
                    int li = engine->champions.leader_index;
                    if (li >= 0 && li < engine->champions.party_count) {
                        int idx = engine->champions.party[li];
                        if (idx >= 0 && idx < 4 && engine->champions.champions[idx].alive) {
                            int dmg = 0;
                            if (nexus_v1_action_semantics_proven() &&
                                nexus_v1_creature_attack(&engine->creatures, ci2,
                                    engine->champions.champions[idx].dexterity, &dmg)) {
                                engine->champions.champions[idx].health -= dmg;
                                if (engine->champions.champions[idx].health <= 0) {
                                    engine->champions.champions[idx].health = 0;
                                    engine->champions.champions[idx].alive = 0;
                                    nexus_v1_champion_on_death_update_leader(
                                        &engine->champions, li);
                                }
                                nexus_v1_damage_display_add(&engine->damage_display,
                                                            idx, dmg, NEXUS_DMG_TAKEN);
                                if (cr->type_index >= 0) {
                                    int psn = engine->creatures.types[cr->type_index].poison;
                                    if (psn > 0)
                                        nexus_v1_status_apply(
                                            &engine->champion_status[idx],
                                            NEXUS_STATUS_POISON, psn, 10);
                                }
                            }
                        }
                    }
                }
            }
          }
          {
            int ci3;
            for (ci3 = 0; ci3 < engine->creatures.active_count; ci3++) {
                Nexus_Creature *cr = &engine->creatures.active[ci3];
                if (cr->health <= 0 && !cr->alive && cr->type_index >= 0) {
                    int xp_val = engine->creatures.types[cr->type_index].experience_value;
                    if (xp_val > 0) {
                        int li = engine->champions.leader_index;
                        if (li >= 0 && li < engine->champions.party_count) {
                            int idx = engine->champions.party[li];
                            if (idx >= 0 && idx < 4 && engine->champions.champions[idx].alive) {
                                nexus_v1_experience_award_kill(&engine->experience,
                                    &engine->champions.champions[idx], idx, xp_val);
                                nexus_v1_experience_check_levelup(&engine->experience,
                                    &engine->champions.champions[idx], idx);
                            }
                        }
                    }
                    /* Saturn creature-drop records are not authenticated.
                     * The former DM1-compatible table in nexus_v1_drops.c
                     * must not manufacture Nexus loot in the live engine.
                     * Keep the death/XP/script path source-backed and leave
                     * the floor unchanged until a real Nexus drop capture is
                     * admitted. */
                    nexus_script_on_creature_dead(&engine->script_vm,
                        cr->type_index);
                    nexus_v1_spawner_on_creature_death(&engine->spawners,
                        ci3);
                    cr->type_index = -2;
                }
            }
          }
        }
        /* Torch/FUL expiry and ambient decay are likewise not promoted from
         * the DM1 compatibility layer without a Saturn action/render trace. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_light_tick(&engine->light);
        }
        /* These HUD timers and game-over transitions are also DM1-shaped
         * consumers; retain fixture behavior but keep retail state opaque
         * until the Saturn HUD/event trace is bound. */
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_damage_display_tick(&engine->damage_display);
            nexus_v1_messages_tick(&engine->messages);
        }
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
          for (ci = 0; ci < engine->champions.party_count; ++ci) {
            int idx = engine->champions.party[ci];
            if (idx >= 0 && idx < 4) {
                Nexus_V1_Champion *ch = &engine->champions.champions[idx];
                if (ch->alive) {
                    if (engine->source == NEXUS_SRC_NONE ||
                        nexus_v1_action_semantics_proven()) {
                        int poison_dmg = nexus_v1_status_poison_damage(
                            &engine->champion_status[idx]);
                        if (poison_dmg > 0) {
                            ch->health -= poison_dmg;
                            if (ch->health <= 0) {
                                ch->health = 0;
                                ch->alive = 0;
                                nexus_v1_champion_on_death_update_leader(
                                    &engine->champions, ci);
                            }
                        }
                        nexus_v1_status_tick(&engine->champion_status[idx]);
                    }
                }
            }
          }
        }
        if (engine->source == NEXUS_SRC_NONE ||
            nexus_v1_action_semantics_proven()) {
            nexus_v1_gameover_check(&engine->gameover, &engine->champions);
            nexus_v1_gameover_tick(&engine->gameover);
        }
    }

    if (redraw && engine->game.game_started) {
        engine->game.needs_redraw = 1;
        if (engine->viewport)
            nexus_viewport_render(engine->viewport, engine);
    }

    /* Increment game tick counter */
    engine->game.tick_count++;
    (void)redraw;
}

void nexus_v1_shutdown(Nexus_V1_Engine *engine) {
    int i;
    if (!engine) return;
    nexus_v1_clear_structure3_runtime_source(engine);
    free(engine->current_level_dgn_data);
    free(engine->current_level_dgn_identity_data);
    engine->current_level_dgn_data = NULL;
    engine->current_level_dgn_identity_data = NULL;
    /* Free mechanics state */
    free(engine->mechanics);
    engine->mechanics = NULL;
    free(engine->viewport);
    engine->viewport = NULL;
    for (i = 0; i < engine->model_count; i++)
        nexus_v1_dmdf_free(&engine->models[i]);
    nexus_ui_manager_free(&engine->ui);
    nexus_v1_font_free(&engine->font);
    nexus_v1_dmdf_free_material_bank(&engine->floor_materials);
    nexus_v1_dmdf_free_material_bank(&engine->wall_materials);
    nexus_v1_dmdf_free_material_bank(&engine->animated_floor_materials);
    nexus_v1_free_structure2_surfaces(engine);
    if (engine->source == NEXUS_SRC_ISO)
        nexus_iso_close(&engine->iso);
    if (engine->supplemental_iso_valid)
        nexus_iso_close(&engine->supplemental_iso);
    memset(engine, 0, sizeof(*engine));
    printf("Nexus V1 engine shut down\n");
}

int nexus_v1_startup_faces_loaded_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_loaded : 0;
}

int nexus_v1_startup_faces_expected_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_expected : 0;
}

int nexus_v1_startup_faces_fallback_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_fallback : 0;
}

int nexus_v1_startup_faces_ready(const Nexus_V1_Engine *engine) {
    if (!engine) return 0;
    return engine->ui_faces_expected > 0 &&
           engine->ui_faces_fallback == 0 &&
           engine->ui_faces_loaded == engine->ui_faces_expected;
}

int nexus_v1_startup_surfaces_loaded_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_loaded : 0;
}

int nexus_v1_startup_surfaces_expected_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_expected : 0;
}

int nexus_v1_startup_surfaces_fallback_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_fallback : 0;
}

int nexus_v1_startup_surfaces_ready(const Nexus_V1_Engine *engine) {
    if (!engine) return 0;
    return engine->ui_startup_surfaces_expected > 0 &&
           engine->ui_startup_surfaces_fallback == 0 &&
           engine->ui_startup_surfaces_loaded ==
               engine->ui_startup_surfaces_expected;
}

int nexus_v1_champion_panel_geometry_ready(const Nexus_V1_Engine *engine)
{
    return engine && engine->champion_panel_geometry_bound &&
           engine->champion_panel_source.canonical_hash_verified;
}

int nexus_v1_champion_panel_geometry(
    const Nexus_V1_Engine *engine,
    Nexus_PanelRect stat_bars[NEXUS_STAT_BAR_RECT_COUNT],
    Nexus_PanelRect inv_slots[NEXUS_INV_SLOT_RECT_COUNT],
    Nexus_PanelRect equip_slots[NEXUS_EQUIP_SLOT_RECT_COUNT])
{
    if (!nexus_v1_champion_panel_geometry_ready(engine) ||
        !stat_bars || !inv_slots || !equip_slots) {
        return -1;
    }
    memcpy(stat_bars, engine->champion_panel_stat_bars,
           sizeof(engine->champion_panel_stat_bars));
    memcpy(inv_slots, engine->champion_panel_inv_slots,
           sizeof(engine->champion_panel_inv_slots));
    memcpy(equip_slots, engine->champion_panel_equip_slots,
           sizeof(engine->champion_panel_equip_slots));
    return 0;
}

int nexus_v1_hud_geometry_ready(const Nexus_V1_Engine *engine)
{
    return engine && engine->hud_geometry_bound &&
           engine->hud_geometry_source.canonical_hash_verified &&
           engine->hud_layout_count == NEXUS_HUD_LAYOUT_ENTRY_COUNT &&
           engine->hud_hit_rect_count == NEXUS_HIT_RECT_COUNT;
}

int nexus_v1_hud_geometry(
    const Nexus_V1_Engine *engine,
    Nexus_HudElement layout[NEXUS_HUD_LAYOUT_ENTRY_COUNT],
    Nexus_HitRect hit_rects[NEXUS_HIT_RECT_COUNT],
    size_t *layout_count, size_t *hit_rect_count)
{
    if (layout_count) *layout_count = 0U;
    if (hit_rect_count) *hit_rect_count = 0U;
    if (!nexus_v1_hud_geometry_ready(engine) || !layout || !hit_rects)
        return 0;
    memcpy(layout, engine->hud_layout, sizeof(engine->hud_layout));
    memcpy(hit_rects, engine->hud_hit_rects, sizeof(engine->hud_hit_rects));
    if (layout_count) *layout_count = engine->hud_layout_count;
    if (hit_rect_count) *hit_rect_count = engine->hud_hit_rect_count;
    return 1;
}

int nexus_v1_hud_raw_hit_test(
    const Nexus_V1_Engine *engine,
    int screen_x, int screen_y,
    size_t *out_region_index, Nexus_HitRect *out_rect)
{
    size_t i;

    if (out_region_index) *out_region_index = 0U;
    if (out_rect) memset(out_rect, 0, sizeof(*out_rect));
    if (!nexus_v1_hud_geometry_ready(engine) || !out_region_index ||
        !out_rect || screen_x < INT16_MIN || screen_x > INT16_MAX ||
        screen_y < INT16_MIN || screen_y > INT16_MAX) {
        return 0;
    }
    for (i = 0U; i < engine->hud_hit_rect_count; ++i) {
        const Nexus_HitRect *rect = &engine->hud_hit_rects[i];
        /* Empty zeroed entries in the retail table are not hit regions. */
        if (rect->x1 >= rect->x2 || rect->y1 >= rect->y2) continue;
        if (screen_x >= rect->x1 && screen_x < rect->x2 &&
            screen_y >= rect->y1 && screen_y < rect->y2) {
            *out_region_index = i;
            *out_rect = *rect;
            return 1;
        }
    }
    return 0;
}

int nexus_v1_menu_bpk_decode_receipt_ready(const Nexus_V1_Engine *engine) {
    return engine && engine->menu_bpk_source.canonical_hash_verified
        ? engine->menu_bpk_decode_receipt_valid : 0;
}

int nexus_v1_menu_bpk_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxSourceReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine) return 0;
    *out_receipt = engine->menu_bpk_source;
    return 0;
}

int nexus_v1_menu_bpk_prs3_execution_evidence_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPrs3ExecutionEvidenceReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !engine->menu_bpk_prs3_execution_evidence_valid) {
        return -1;
    }
    *out_receipt = engine->menu_bpk_prs3_execution_evidence;
    return 0;
}

int nexus_v1_menu_bpk_decode_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeDecodeReceipt *out_receipt) {
    if (!engine || !out_receipt || !engine->menu_bpk_decode_receipt_valid) {
        return -1;
    }
    *out_receipt = engine->menu_bpk_decode_receipt;
    return 0;
}

int nexus_v1_menu_bpk_upload_plan_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadReceipt *out_receipt) {
    if (!engine || !out_receipt || !engine->menu_bpk_upload_receipt_valid) {
        return -1;
    }
    *out_receipt = engine->menu_bpk_upload_receipt;
    return 0;
}

int nexus_v1_menu_bpk_upload_plan_rows(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadRow *out_rows,
    int max_rows) {
    int count;
    if (!engine || !out_rows || max_rows <= 0 ||
        !engine->menu_bpk_upload_receipt_valid) {
        return -1;
    }
    count = engine->menu_bpk_upload_row_count;
    if (count > max_rows) count = max_rows;
    if (count > 0) {
        memcpy(out_rows, engine->menu_bpk_upload_rows,
               (size_t)count * sizeof(out_rows[0]));
    }
    return count;
}

int nexus_v1_engine_set_menu_bpk_no_draw_host_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    const Nexus_V1_BpkRuntimeUploadReceipt *upload,
    const Nexus_V1_BpkRuntimeUploadRow *row)
{
    int row_matches_engine = 0;
    int i;

    if (!engine) return 0;
    engine->menu_bpk_no_draw_host_valid = 0;
    engine->menu_bpk_no_draw_host_route_epoch = 0U;
    engine->menu_bpk_no_draw_host_package_fnv1a64 = 0U;
    engine->menu_bpk_no_draw_host_entry_index = 0U;
    engine->menu_bpk_no_draw_host_payload_offset = 0U;
    engine->menu_bpk_no_draw_host_payload_size = 0U;
    engine->menu_bpk_no_draw_host_payload_fnv1a64 = 0U;
    if (!route_epoch || route_epoch <= engine->menu_bpk_no_draw_host_last_route_epoch ||
        !package_fnv1a64 ||
        package_fnv1a64 != engine->menu_bpk_package_fnv1a64 ||
        !engine->menu_bpk_upload_receipt_valid || !upload || !row ||
        upload->first_prs3_entry_index == UINT32_MAX ||
        upload->unknown_prs3_mode_entries != 0U ||
        !row->compression.valid || !upload->first_prs3_compression.valid ||
        row->compression.entry_index != row->entry_index ||
        row->compression.mode_flags != row->mode ||
        row->compression.declared_pixel_count != row->prs3_pixel_count ||
        row->compression.declared_output_bytes != row->expected_output_bytes ||
        !row->compression.compressed_length ||
        !row->compression.compressed_fnv1a64 ||
        row->compression.decoder_promoted || row->compression.pixels_exposed ||
        row->compression.fallback_visuals_permitted ||
        !row->decode_blocked || !row->evidence_only ||
        row->decoded_pixels_emitted != 0U || row->fallback_visuals_permitted) {
        return 0;
    }
    if (upload->first_prs3_entry_index !=
            engine->menu_bpk_upload_receipt.first_prs3_entry_index ||
        upload->first_prs3_payload_fnv1a64 !=
            engine->menu_bpk_upload_receipt.first_prs3_payload_fnv1a64 ||
        memcmp(&upload->first_prs3_compression,
               &engine->menu_bpk_upload_receipt.first_prs3_compression,
               sizeof(upload->first_prs3_compression)) != 0) {
        return 0;
    }
    for (i = 0; i < engine->menu_bpk_upload_row_count; ++i) {
        const Nexus_V1_BpkRuntimeUploadRow *current =
            &engine->menu_bpk_upload_rows[i];
        if (current->entry_index == row->entry_index &&
            current->payload_offset == row->payload_offset &&
            current->payload_size == row->payload_size &&
            current->payload_fnv1a64 == row->payload_fnv1a64 &&
            current->prs3_version == row->prs3_version &&
            current->header_first_u32 == row->header_first_u32 &&
            current->header_minus_payload == row->header_minus_payload &&
            memcmp(&current->compression, &row->compression,
                   sizeof(row->compression)) == 0) {
            row_matches_engine = 1;
            break;
        }
    }
    if (!row_matches_engine) return 0;
    engine->menu_bpk_no_draw_host_last_route_epoch = route_epoch;
    engine->menu_bpk_no_draw_host_valid = 1;
    engine->menu_bpk_no_draw_host_route_epoch = route_epoch;
    engine->menu_bpk_no_draw_host_package_fnv1a64 = package_fnv1a64;
    engine->menu_bpk_no_draw_host_entry_index = row->entry_index;
    engine->menu_bpk_no_draw_host_payload_offset = row->payload_offset;
    engine->menu_bpk_no_draw_host_payload_size = row->payload_size;
    engine->menu_bpk_no_draw_host_payload_fnv1a64 = row->payload_fnv1a64;
    engine->menu_bpk_no_draw_host_compression = row->compression;
    return 1;
}

static int nexus_v1_engine_menu_bpk_no_draw_host_row_is_current(
    const Nexus_V1_Engine *engine)
{
    int index;

    if (!engine || !engine->menu_bpk_upload_receipt_valid ||
        engine->menu_bpk_upload_receipt.unknown_prs3_mode_entries != 0U ||
        engine->menu_bpk_upload_row_count <= 0 ||
        engine->menu_bpk_upload_row_count >
            (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS) return 0;
    for (index = 0; index < engine->menu_bpk_upload_row_count; ++index) {
        const Nexus_V1_BpkRuntimeUploadRow *row =
            &engine->menu_bpk_upload_rows[index];
        if (row->entry_index == engine->menu_bpk_no_draw_host_entry_index &&
            row->payload_offset == engine->menu_bpk_no_draw_host_payload_offset &&
            row->payload_size == engine->menu_bpk_no_draw_host_payload_size &&
            row->payload_fnv1a64 == engine->menu_bpk_no_draw_host_payload_fnv1a64 &&
            row->compression.valid &&
            row->compression.declared_output_bytes ==
                engine->menu_bpk_no_draw_host_compression.declared_output_bytes &&
            memcmp(&row->compression,
                   &engine->menu_bpk_no_draw_host_compression,
                   sizeof(row->compression)) == 0) return 1;
    }
    return 0;
}

int nexus_v1_engine_menu_bpk_no_draw_host_ready(
    const Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64)
{
    return engine && engine->menu_bpk_no_draw_host_valid &&
        engine->menu_bpk_no_draw_host_compression.valid &&
        !engine->menu_bpk_no_draw_host_compression.decoder_promoted &&
        !engine->menu_bpk_no_draw_host_compression.pixels_exposed &&
        !engine->menu_bpk_no_draw_host_compression.fallback_visuals_permitted &&
        route_epoch && route_epoch == engine->menu_bpk_no_draw_host_route_epoch &&
        package_fnv1a64 &&
        package_fnv1a64 == engine->menu_bpk_package_fnv1a64 &&
        package_fnv1a64 == engine->menu_bpk_no_draw_host_package_fnv1a64 &&
        nexus_v1_engine_menu_bpk_no_draw_host_row_is_current(engine);
}

static int nexus_v1_direct_lev_md5_is_valid(const char *md5)
{
    size_t i;
    if (!md5 || strlen(md5) != 32U) return 0;
    for (i = 0U; i < 32U; ++i) {
        if (!((md5[i] >= '0' && md5[i] <= '9') ||
              (md5[i] >= 'a' && md5[i] <= 'f'))) return 0;
    }
    return 1;
}

static int nexus_v1_engine_direct_lev_header_descriptor_is_current(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *provenance,
    int level_index, uint64_t dgn_fnv1a64)
{
    Nexus_V1_DgnStructure1Layout layout;
    uint64_t descriptor_offset;

    if (!engine || !provenance || !provenance->valid ||
        provenance->level_index != (uint32_t)level_index ||
        !provenance->no_draw_only || provenance->fallback_visuals_permitted ||
        !provenance->blocks_real_dgn_mesh_render ||
        provenance->package_fnv1a64 != dgn_fnv1a64 ||
        provenance->header_offset != 0U || provenance->header_length != 0x14U ||
        !provenance->header_fnv1a64 || !provenance->descriptor_length ||
        !provenance->descriptor_fnv1a64 || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size < NEXUS_DGN_BLOCK_SIZE ||
        provenance->header_length > (uint32_t)engine->current_level_dgn_size ||
        provenance->descriptor_offset > (uint32_t)engine->current_level_dgn_size ||
        provenance->descriptor_length > (uint32_t)engine->current_level_dgn_size -
            provenance->descriptor_offset ||
        nexus_v1_dgn_bytes_fnv1a64(engine->current_level_dgn_data,
                                    (int)provenance->header_length) !=
            provenance->header_fnv1a64 ||
        nexus_v1_dgn_bytes_fnv1a64(engine->current_level_dgn_data +
                                        provenance->descriptor_offset,
                                    (int)provenance->descriptor_length) !=
            provenance->descriptor_fnv1a64 ||
        nexus_v1_dgn_structure1_layout(&layout, engine->current_level_dgn_data,
                                       engine->current_level_dgn_size) != 0 ||
        !layout.valid || !layout.structure1f.valid ||
        layout.structure1f.total_entry_count < 0 ||
        provenance->descriptor_count !=
            (uint32_t)layout.structure1f.total_entry_count) return 0;
    descriptor_offset = (uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1f.relative_offset;
    return descriptor_offset == provenance->descriptor_offset &&
        layout.structure1f.size == (int)provenance->descriptor_length;
}

static int nexus_v1_engine_build_m11_structure1f_descriptor_intake(
    const Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    uint64_t package_fnv1a64,
    const Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *header_descriptor,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry,
    Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt *out_receipt)
{
    Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt receipt;
    Nexus_V1_DgnStructure1FSourcePacket current;
    const Nexus_V1_DgnStructure1FDirectMeshBindingReceipt *direct_mesh;
    uint64_t descriptor_end;
    uint64_t table_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!engine || !route_epoch || level_index < 0 || !package_fnv1a64 ||
        !header_descriptor || !header_descriptor->valid ||
        !geometry || !geometry->valid ||
        !geometry->structure1f_record_source_bound ||
        !geometry->face_mesh_adjacency_bound ||
        !geometry->structure2_source_envelope_bound ||
        !geometry->no_draw_only || geometry->fallback_visuals_permitted ||
        !geometry->blocks_real_dgn_mesh_render ||
        !nexus_v1_engine_direct_lev_header_descriptor_is_current(
            engine, header_descriptor, level_index, package_fnv1a64) ||
        !geometry->structure1f_source.valid ||
        geometry->structure1f_source.level_index != level_index ||
        geometry->structure1f_source.package_fnv1a64 != package_fnv1a64 ||
        geometry->structure1f_source.entry_index < 0 ||
        !geometry->structure1f_source.descriptor_length ||
        !geometry->structure1f_source.descriptor_fnv1a64 ||
        !geometry->structure2_source.loaded_bytes_bound ||
        geometry->structure2_source.loaded_dgn_fnv1a64 != package_fnv1a64 ||
        nexus_v1_current_level_lookup_structure1f_source_entry(
            engine, geometry->structure1f_source.entry_index, &current) != 1 ||
        !current.valid || current.package_fnv1a64 != package_fnv1a64 ||
        current.descriptor_offset != geometry->structure1f_source.descriptor_offset ||
        current.descriptor_length != geometry->structure1f_source.descriptor_length ||
        current.descriptor_fnv1a64 != geometry->structure1f_source.descriptor_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    descriptor_end = (uint64_t)current.descriptor_offset +
        (uint64_t)current.descriptor_length;
    table_end = (uint64_t)header_descriptor->descriptor_offset +
        (uint64_t)header_descriptor->descriptor_length;
    direct_mesh = &geometry->transform_target.geometry.direct_mesh;
    if (current.descriptor_offset < header_descriptor->descriptor_offset ||
        descriptor_end > table_end || !direct_mesh->valid ||
        direct_mesh->structure1f_entry_index != current.entry_index ||
        !direct_mesh->model_to_entry_proven || !direct_mesh->face_ordinal_proven ||
        !direct_mesh->no_draw_only || direct_mesh->fallback_visuals_permitted ||
        direct_mesh->source_bytes_fnv1a64 != package_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = (uint32_t)level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.structure1f_entry_index = current.entry_index;
    receipt.descriptor_offset = current.descriptor_offset;
    receipt.descriptor_length = current.descriptor_length;
    receipt.descriptor_fnv1a64 = current.descriptor_fnv1a64;
    receipt.structure3_model_index = direct_mesh->structure3_model_index;
    receipt.face_ordinal = direct_mesh->face_ordinal;
    receipt.face_mesh_reference_bound = 1;
    receipt.material_reference_opaque = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    return 1;
}

static int nexus_v1_engine_m11_structure1f_descriptor_matches(
    const Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt *left,
    const Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt *right)
{
    return left && right && left->valid && right->valid &&
        left->level_index == right->level_index &&
        left->route_epoch == right->route_epoch &&
        left->package_fnv1a64 == right->package_fnv1a64 &&
        left->structure1f_entry_index == right->structure1f_entry_index &&
        left->descriptor_offset == right->descriptor_offset &&
        left->descriptor_length == right->descriptor_length &&
        left->descriptor_fnv1a64 == right->descriptor_fnv1a64 &&
        left->structure3_model_index == right->structure3_model_index &&
        left->face_ordinal == right->face_ordinal &&
        left->face_mesh_reference_bound && right->face_mesh_reference_bound &&
        left->material_reference_opaque && right->material_reference_opaque &&
        left->no_draw_only && right->no_draw_only &&
        !left->fallback_visuals_permitted &&
        !right->fallback_visuals_permitted &&
        left->blocks_real_dgn_mesh_render &&
        right->blocks_real_dgn_mesh_render;
}

static int nexus_v1_engine_build_m11_structure2_face_descriptor_intake(
    const Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    uint64_t package_fnv1a64,
    const Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt *structure1f,
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *out_receipt)
{
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt receipt;
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;
    const Nexus_V1_DgnStructure3StaticMaterialCaptureTarget *material;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;
    uint64_t descriptor_end;
    uint64_t face_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    memset(&material_target, 0, sizeof(material_target));
    if (!engine || !route_epoch || level_index < 0 || !package_fnv1a64 ||
        !structure1f || !structure1f->valid ||
        structure1f->level_index != (uint32_t)level_index ||
        structure1f->route_epoch != route_epoch ||
        structure1f->package_fnv1a64 != package_fnv1a64 ||
        !structure1f->face_mesh_reference_bound ||
        !structure1f->material_reference_opaque ||
        !structure1f->no_draw_only ||
        structure1f->fallback_visuals_permitted ||
        !structure1f->blocks_real_dgn_mesh_render ||
        nexus_v1_engine_build_structure3_static_material_capture_target(
            engine, structure1f->structure3_model_index,
            structure1f->face_ordinal, &material_target) != 1 ||
        !material_target.valid ||
        !material_target.capture_producer_required ||
        !material_target.original_saturn_capture_required ||
        !material_target.no_draw_only ||
        material_target.fallback_visuals_permitted ||
        material_target.level_index != level_index ||
        material_target.source_bytes_fnv1a64 != package_fnv1a64 ||
        material_target.structure3_entry_index !=
            (uint32_t)structure1f->structure3_model_index ||
        material_target.face_ordinal != (uint32_t)structure1f->face_ordinal) {
        *out_receipt = receipt;
        return 0;
    }
    material = &material_target;
    descriptor = &material->descriptor_target;
    if (!material->valid || !material->static_selector_descriptor_bound ||
        !material->image_payload_anchor_bound ||
        !material->image_payload_interval_bound ||
        !material->capture_producer_required ||
        !material->original_saturn_capture_required || !material->no_draw_only ||
        material->fallback_visuals_permitted ||
        material->level_index != level_index ||
        material->source_bytes_fnv1a64 != package_fnv1a64 ||
        material->structure3_entry_index !=
            (uint32_t)structure1f->structure3_model_index ||
        material->face_ordinal != (uint32_t)structure1f->face_ordinal ||
        material->face_byte_offset < 0 || !material->face_bytes_fnv1a64 ||
        !descriptor->valid || descriptor->level_index != level_index ||
        descriptor->source_bytes_fnv1a64 != package_fnv1a64 ||
        descriptor->descriptor_byte_offset < 0 ||
        !descriptor->descriptor_bytes_fnv1a64 || !descriptor->no_draw_only ||
        descriptor->fallback_visuals_permitted ||
        !descriptor->image_payload_candidate_bound ||
        !descriptor->image_payload_candidate_byte_count ||
        !descriptor->image_payload_candidate_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    face_end = (uint64_t)(uint32_t)material->face_byte_offset + 12U;
    descriptor_end = (uint64_t)(uint32_t)descriptor->descriptor_byte_offset +
        NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    if (face_end > (uint64_t)engine->current_level_dgn_size ||
        descriptor_end > (uint64_t)engine->current_level_dgn_size ||
        nexus_v1_dgn_bytes_fnv1a64(engine->current_level_dgn_data +
                                        material->face_byte_offset,
                                    12) != material->face_bytes_fnv1a64 ||
        nexus_v1_dgn_bytes_fnv1a64(engine->current_level_dgn_data +
                                        descriptor->descriptor_byte_offset,
                                    NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES) !=
            descriptor->descriptor_bytes_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.level_index = (uint32_t)level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.structure1f_entry_index = structure1f->structure1f_entry_index;
    receipt.structure3_entry_index = material->structure3_entry_index;
    receipt.face_ordinal = material->face_ordinal;
    receipt.face_offset = (uint32_t)material->face_byte_offset;
    receipt.face_length = 12U;
    receipt.face_fnv1a64 = material->face_bytes_fnv1a64;
    receipt.structure2_descriptor_index = descriptor->descriptor_index;
    receipt.descriptor_offset = (uint32_t)descriptor->descriptor_byte_offset;
    receipt.descriptor_length = NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    receipt.descriptor_fnv1a64 = descriptor->descriptor_bytes_fnv1a64;
    receipt.image_candidate_offset = descriptor->image_payload_anchor_offset;
    receipt.image_candidate_length = descriptor->image_payload_candidate_byte_count;
    receipt.image_candidate_fnv1a64 = descriptor->image_payload_candidate_fnv1a64;
    receipt.palette_candidate_offset = descriptor->palette_payload_anchor_offset;
    receipt.palette_candidate_length = descriptor->palette_payload_candidate_byte_count;
    receipt.palette_candidate_fnv1a64 = descriptor->palette_payload_candidate_fnv1a64;
    receipt.face_descriptor_bound = 1;
    receipt.candidates_opaque = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    return 1;
}

static int nexus_v1_engine_m11_structure2_face_descriptor_matches(
    const Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *left,
    const Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *right)
{
    return left && right && left->valid && right->valid &&
        left->level_index == right->level_index &&
        left->route_epoch == right->route_epoch &&
        left->package_fnv1a64 == right->package_fnv1a64 &&
        left->structure1f_entry_index == right->structure1f_entry_index &&
        left->structure3_entry_index == right->structure3_entry_index &&
        left->face_ordinal == right->face_ordinal &&
        left->face_offset == right->face_offset &&
        left->face_length == right->face_length &&
        left->face_fnv1a64 == right->face_fnv1a64 &&
        left->structure2_descriptor_index == right->structure2_descriptor_index &&
        left->descriptor_offset == right->descriptor_offset &&
        left->descriptor_length == right->descriptor_length &&
        left->descriptor_fnv1a64 == right->descriptor_fnv1a64 &&
        left->image_candidate_offset == right->image_candidate_offset &&
        left->image_candidate_length == right->image_candidate_length &&
        left->image_candidate_fnv1a64 == right->image_candidate_fnv1a64 &&
        left->palette_candidate_offset == right->palette_candidate_offset &&
        left->palette_candidate_length == right->palette_candidate_length &&
        left->palette_candidate_fnv1a64 == right->palette_candidate_fnv1a64 &&
        left->face_descriptor_bound && right->face_descriptor_bound &&
        left->candidates_opaque && right->candidates_opaque &&
        left->no_draw_only && right->no_draw_only &&
        !left->fallback_visuals_permitted &&
        !right->fallback_visuals_permitted &&
        left->blocks_real_dgn_mesh_render &&
        right->blocks_real_dgn_mesh_render;
}

static int nexus_v1_engine_build_m11_structure3_topology_descriptor_intake(
    const Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    uint64_t package_fnv1a64,
    const Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt *structure1f,
    const Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt *face_descriptor,
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt *out_receipt)
{
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt receipt;
    Nexus_V1_DgnStructure3PackageGeometryPacket packet;
    Nexus_V1_DgnStructure3MeshEntryReceipt mesh;
    const Nexus_V1_Level *level;
    const uint8_t *data;
    uint32_t payload_offset;
    uint32_t payload_size;
    uint32_t entry_offset;
    uint32_t vertex_offset;
    uint32_t normal_offset;
    uint32_t vertex_count;
    uint32_t face_count;
    uint32_t slot_count;
    uint64_t vertex_table_offset;
    uint64_t vertex_table_length;
    uint64_t normal_row_offset;
    uint64_t referenced_hash = UINT64_C(1469598103934665603);
    uint32_t slot;
    uint32_t byte;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    memset(&packet, 0, sizeof(packet));
    memset(&mesh, 0, sizeof(mesh));
    if (!engine || !route_epoch || level_index < 0 || !package_fnv1a64 ||
        !structure1f || !face_descriptor || !structure1f->valid ||
        !face_descriptor->valid ||
        structure1f->level_index != (uint32_t)level_index ||
        face_descriptor->level_index != (uint32_t)level_index ||
        structure1f->route_epoch != route_epoch ||
        face_descriptor->route_epoch != route_epoch ||
        structure1f->package_fnv1a64 != package_fnv1a64 ||
        face_descriptor->package_fnv1a64 != package_fnv1a64 ||
        structure1f->structure1f_entry_index !=
            face_descriptor->structure1f_entry_index ||
        !face_descriptor->face_descriptor_bound ||
        !face_descriptor->candidates_opaque || !face_descriptor->no_draw_only ||
        face_descriptor->fallback_visuals_permitted ||
        !face_descriptor->blocks_real_dgn_mesh_render ||
        !engine->current_level_dgn_data || engine->current_level_dgn_size <= 0 ||
        engine->current_level.structure3_payload.byte_offset < 0 ||
        engine->current_level.structure3_payload.byte_size < 4 ||
        !engine->current_level.structure3_directory.valid ||
        !engine->current_level.structure3_entry_headers.valid ||
        !engine->current_level.structure3_faces.valid ||
        !engine->current_level.structure3_vectors.valid ||
        !engine->current_level.structure3_face_normal_pairs.valid ||
        nexus_v1_current_level_structure3_package_geometry_packet(
            engine, face_descriptor->structure3_entry_index,
            face_descriptor->face_ordinal, &packet) != 1 ||
        !packet.valid || !packet.source_geometry_bound ||
        packet.package_fnv1a64 != package_fnv1a64 ||
        packet.face_offset != face_descriptor->face_offset ||
        packet.face_length != face_descriptor->face_length ||
        packet.face_fnv1a64 != face_descriptor->face_fnv1a64 ||
        nexus_v1_current_level_extract_structure3_mesh_entry(
            engine, (int)face_descriptor->structure3_entry_index, NULL, 0,
            NULL, 0, NULL, 0, &mesh) != -1 ||
        !mesh.source_identity_valid || mesh.vertex_count <= 0 ||
        mesh.face_count <= 0 || mesh.normal_count < mesh.face_count ||
        face_descriptor->face_ordinal >= (uint32_t)mesh.face_count) {
        *out_receipt = receipt;
        return 0;
    }
    level = &engine->current_level;
    data = engine->current_level_dgn_data;
    payload_offset = (uint32_t)level->structure3_payload.byte_offset;
    payload_size = (uint32_t)level->structure3_payload.byte_size;
    if (payload_offset > (uint32_t)engine->current_level_dgn_size ||
        payload_size > (uint32_t)engine->current_level_dgn_size - payload_offset ||
        face_descriptor->structure3_entry_index >=
            (uint32_t)level->structure3_directory.entry_count ||
        payload_size < 8U ||
        face_descriptor->structure3_entry_index >
            (payload_size - 8U) / 4U) {
        *out_receipt = receipt;
        return 0;
    }
    entry_offset = nexus_v1_dgn_read_be32(
        data + payload_offset + 4U +
        face_descriptor->structure3_entry_index * 4U);
    if (entry_offset > payload_size || payload_size - entry_offset < 40U) {
        *out_receipt = receipt;
        return 0;
    }
    vertex_count = (uint32_t)nexus_v1_dgn_read_be16(
        data + payload_offset + entry_offset + 4U);
    face_count = (uint32_t)nexus_v1_dgn_read_be16(
        data + payload_offset + entry_offset + 6U);
    vertex_offset = nexus_v1_dgn_read_be32(
        data + payload_offset + entry_offset + 8U);
    normal_offset = nexus_v1_dgn_read_be32(
        data + payload_offset + entry_offset + 20U);
    vertex_table_offset = (uint64_t)payload_offset + vertex_offset;
    vertex_table_length = (uint64_t)vertex_count * 12U;
    normal_row_offset = (uint64_t)payload_offset + normal_offset +
        (uint64_t)face_descriptor->face_ordinal * 12U;
    if (vertex_count != (uint32_t)mesh.vertex_count ||
        face_count != (uint32_t)mesh.face_count ||
        vertex_table_offset > (uint64_t)engine->current_level_dgn_size ||
        vertex_table_length > (uint64_t)engine->current_level_dgn_size -
            vertex_table_offset ||
        normal_row_offset > (uint64_t)engine->current_level_dgn_size ||
        12U > (uint64_t)engine->current_level_dgn_size - normal_row_offset) {
        *out_receipt = receipt;
        return 0;
    }
    slot_count = packet.face.triangle ? 3U : 4U;
    if (slot_count < 3U || slot_count > 4U) {
        *out_receipt = receipt;
        return 0;
    }
    for (slot = 0U; slot < slot_count; ++slot) {
        uint64_t row_offset;
        if (packet.face.vertex_indexes[slot] >= vertex_count) {
            *out_receipt = receipt;
            return 0;
        }
        row_offset = vertex_table_offset +
            (uint64_t)packet.face.vertex_indexes[slot] * 12U;
        if (row_offset > (uint64_t)engine->current_level_dgn_size ||
            12U > (uint64_t)engine->current_level_dgn_size - row_offset) {
            *out_receipt = receipt;
            return 0;
        }
        for (byte = 0U; byte < 12U; ++byte) {
            referenced_hash ^= data[row_offset + byte];
            referenced_hash *= UINT64_C(1099511628211);
        }
    }
    receipt.valid = 1;
    receipt.level_index = (uint32_t)level_index;
    receipt.route_epoch = route_epoch;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.structure1f_entry_index = structure1f->structure1f_entry_index;
    receipt.structure3_entry_index = face_descriptor->structure3_entry_index;
    receipt.face_ordinal = face_descriptor->face_ordinal;
    receipt.face_offset = face_descriptor->face_offset;
    receipt.face_length = face_descriptor->face_length;
    receipt.face_fnv1a64 = face_descriptor->face_fnv1a64;
    receipt.vertex_table_offset = (uint32_t)vertex_table_offset;
    receipt.vertex_table_length = (uint32_t)vertex_table_length;
    receipt.vertex_table_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + vertex_table_offset, (int)vertex_table_length);
    receipt.vertex_count = vertex_count;
    receipt.face_vertex_index_count = slot_count;
    receipt.referenced_vertex_rows_fnv1a64 = referenced_hash;
    receipt.normal_offset = (uint32_t)normal_row_offset;
    receipt.normal_length = 12U;
    receipt.normal_fnv1a64 = nexus_v1_dgn_bytes_fnv1a64(
        data + normal_row_offset, 12);
    receipt.topology_framing_bound = 1;
    receipt.capture_required = 1;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    return 1;
}

static int nexus_v1_engine_m11_structure3_topology_descriptor_matches(
    const Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt *left,
    const Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt *right)
{
    return left && right && left->valid && right->valid &&
        left->level_index == right->level_index &&
        left->route_epoch == right->route_epoch &&
        left->package_fnv1a64 == right->package_fnv1a64 &&
        left->structure1f_entry_index == right->structure1f_entry_index &&
        left->structure3_entry_index == right->structure3_entry_index &&
        left->face_ordinal == right->face_ordinal &&
        left->face_offset == right->face_offset &&
        left->face_length == right->face_length &&
        left->face_fnv1a64 == right->face_fnv1a64 &&
        left->vertex_table_offset == right->vertex_table_offset &&
        left->vertex_table_length == right->vertex_table_length &&
        left->vertex_table_fnv1a64 == right->vertex_table_fnv1a64 &&
        left->vertex_count == right->vertex_count &&
        left->face_vertex_index_count == right->face_vertex_index_count &&
        left->referenced_vertex_rows_fnv1a64 ==
            right->referenced_vertex_rows_fnv1a64 &&
        left->normal_offset == right->normal_offset &&
        left->normal_length == right->normal_length &&
        left->normal_fnv1a64 == right->normal_fnv1a64 &&
        left->topology_framing_bound && right->topology_framing_bound &&
        left->capture_required && right->capture_required &&
        left->no_draw_only && right->no_draw_only &&
        !left->fallback_visuals_permitted &&
        !right->fallback_visuals_permitted &&
        left->blocks_real_dgn_mesh_render &&
        right->blocks_real_dgn_mesh_render;
}

int nexus_v1_engine_set_m11_direct_lev_dungeon_no_draw_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    const char *dgn_md5, uint64_t dgn_byte_count, uint64_t dgn_fnv1a64,
    const Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *header_descriptor,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry)
{
    Nexus_V1_DgnM11DirectLevNoDrawReceipt receipt;
    Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt structure1f_descriptor;
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt structure2_face_descriptor;
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt topology_descriptor;

    if (!engine) return 0;
    memset(&engine->m11_direct_lev_dungeon, 0,
           sizeof(engine->m11_direct_lev_dungeon));
    engine->m11_direct_lev_dungeon_no_draw_valid = 0;
    if (!route_epoch || route_epoch <=
            engine->m11_direct_lev_dungeon_last_route_epoch ||
        level_index < 0 || level_index != engine->game.current_level ||
        !nexus_v1_direct_lev_md5_is_valid(dgn_md5) || !dgn_byte_count ||
        !dgn_fnv1a64 || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0 ||
        dgn_byte_count != (uint64_t)engine->current_level_dgn_size ||
        dgn_fnv1a64 != nexus_v1_dgn_bytes_fnv1a64(
            engine->current_level_dgn_data, engine->current_level_dgn_size) ||
        engine->current_level_structure2_source.level_index != level_index ||
        !engine->current_level_structure2_source.canonical_hash_verified ||
        !engine->current_level_structure2_source.loaded_bytes_bound ||
        !engine->current_level_structure2_source.materialization_bound ||
        !engine->current_level_structure2_source.structure2_payload_envelope_valid ||
        engine->current_level_structure2_source.loaded_dgn_size !=
            engine->current_level_dgn_size ||
        engine->current_level_structure2_source.loaded_dgn_fnv1a64 !=
            dgn_fnv1a64 ||
        !nexus_v1_engine_direct_lev_header_descriptor_is_current(
            engine, header_descriptor, level_index, dgn_fnv1a64) ||
        !geometry || !nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
            engine, geometry) ||
        geometry->structure1f_source.level_index != level_index ||
        geometry->structure1f_source.source_byte_count !=
            engine->current_level_dgn_size ||
        geometry->structure1f_source.package_fnv1a64 != dgn_fnv1a64 ||
        geometry->structure2_source.loaded_dgn_fnv1a64 != dgn_fnv1a64 ||
        !nexus_v1_engine_build_m11_structure1f_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64, header_descriptor,
            geometry, &structure1f_descriptor)) return 0;
    if (!nexus_v1_engine_build_m11_structure2_face_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64,
            &structure1f_descriptor, &structure2_face_descriptor)) return 0;
    if (!nexus_v1_engine_build_m11_structure3_topology_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64,
            &structure1f_descriptor, &structure2_face_descriptor,
            &topology_descriptor)) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.level_index = level_index;
    receipt.route_epoch = route_epoch;
    snprintf(receipt.dgn_md5, sizeof(receipt.dgn_md5), "%s", dgn_md5);
    receipt.dgn_byte_count = dgn_byte_count;
    receipt.dgn_fnv1a64 = dgn_fnv1a64;
    receipt.header_descriptor = *header_descriptor;
    receipt.structure1f_descriptor = structure1f_descriptor;
    receipt.structure2_face_descriptor = structure2_face_descriptor;
    receipt.structure3_topology_descriptor = topology_descriptor;
    receipt.geometry = *geometry;
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    engine->m11_direct_lev_dungeon = receipt;
    engine->m11_direct_lev_dungeon_no_draw_valid = 1;
    engine->m11_direct_lev_dungeon_route_epoch = route_epoch;
    engine->m11_direct_lev_dungeon_last_route_epoch = route_epoch;
    return 1;
}

int nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
    const Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    const char *dgn_md5, uint64_t dgn_byte_count, uint64_t dgn_fnv1a64,
    Nexus_V1_DgnM11DirectLevNoDrawReceipt *out_receipt)
{
    const Nexus_V1_DgnM11DirectLevNoDrawReceipt *receipt;
    Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt current_descriptor;
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt current_face_descriptor;
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt current_topology_descriptor;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!engine || !engine->m11_direct_lev_dungeon_no_draw_valid ||
        !route_epoch || route_epoch != engine->m11_direct_lev_dungeon_route_epoch ||
        level_index < 0 || level_index != engine->game.current_level ||
        !nexus_v1_direct_lev_md5_is_valid(dgn_md5) || !dgn_byte_count ||
        !dgn_fnv1a64 || !engine->current_level_dgn_data ||
        engine->current_level_dgn_size <= 0) return 0;
    receipt = &engine->m11_direct_lev_dungeon;
    memset(&current_descriptor, 0, sizeof(current_descriptor));
    memset(&current_face_descriptor, 0, sizeof(current_face_descriptor));
    memset(&current_topology_descriptor, 0, sizeof(current_topology_descriptor));
    if (!receipt->valid || !receipt->no_draw_only ||
        receipt->fallback_visuals_permitted ||
        !receipt->blocks_real_dgn_mesh_render ||
        receipt->level_index != level_index ||
        receipt->route_epoch != route_epoch ||
        strcmp(receipt->dgn_md5, dgn_md5) != 0 ||
        receipt->dgn_byte_count != dgn_byte_count ||
        receipt->dgn_fnv1a64 != dgn_fnv1a64 ||
        dgn_byte_count != (uint64_t)engine->current_level_dgn_size ||
        dgn_fnv1a64 != nexus_v1_dgn_bytes_fnv1a64(
            engine->current_level_dgn_data, engine->current_level_dgn_size) ||
        !nexus_v1_engine_direct_lev_header_descriptor_is_current(
            engine, &receipt->header_descriptor, level_index, dgn_fnv1a64) ||
        !nexus_v1_engine_build_m11_structure1f_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64,
            &receipt->header_descriptor, &receipt->geometry,
            &current_descriptor) ||
        !nexus_v1_engine_m11_structure1f_descriptor_matches(
            &receipt->structure1f_descriptor, &current_descriptor) ||
        !nexus_v1_engine_build_m11_structure2_face_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64, &current_descriptor,
            &current_face_descriptor) ||
        !nexus_v1_engine_m11_structure2_face_descriptor_matches(
            &receipt->structure2_face_descriptor, &current_face_descriptor) ||
        !nexus_v1_engine_build_m11_structure3_topology_descriptor_intake(
            engine, route_epoch, level_index, dgn_fnv1a64, &current_descriptor,
            &current_face_descriptor, &current_topology_descriptor) ||
        !nexus_v1_engine_m11_structure3_topology_descriptor_matches(
            &receipt->structure3_topology_descriptor,
            &current_topology_descriptor) ||
        !nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
            engine, &receipt->geometry)) return 0;
    if (out_receipt) *out_receipt = *receipt;
    return 1;
}

static Nexus_V1_MenuBpkRendererHandoffStatus
nexus_v1_menu_bpk_handoff_status_from_decode_route(
    Nexus_V1_BpkRuntimeDecodeRoute route) {
    switch (route) {
    case NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED;
    case NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED;
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3;
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED;
    case NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES;
    case NEXUS_V1_BPK_DECODE_ROUTE_INVALID:
    default:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID;
    }
}

static Nexus_V1_MenuBpkPrs3PrerequisiteStatus
nexus_v1_menu_bpk_prs3_prerequisite_from_handoff(
    const Nexus_V1_MenuBpkRendererHandoffReceipt *handoff) {
    if (!handoff) {
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING;
    }
    if (!handoff->canonical_source_hash_verified) {
        return handoff->status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING
            ? NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING
            : NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SOURCE_UNVERIFIED;
    }
    if (handoff->blocks_real_menu_surface_render &&
        (handoff->status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED ||
         handoff->status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED)) {
        /* A source-authenticated surface still needs the Saturn
         * presentation capture before it is a renderer handoff. */
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SATURN_PRESENTATION;
    }
    switch (handoff->status) {
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED:
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED;
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED:
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED;
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED:
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_FRAME_INCOMPLETE;
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3:
        /* A bounded PRS3 frame is not decoded merely because its container
         * parsed. Original SH-2 opcode/output evidence comes first. */
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_AUTHENTIC_DECODER;
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES:
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID:
    default:
        return NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_INVALID;
    }
}

/* Decoding MENU.BPK produces source pixels only.  The production menu route
 * also needs the independently admitted PALT/palette-state/VDP1 capture;
 * do not let a host decode masquerade as a Saturn presentation handoff. */
static int nexus_v1_menu_bpk_palt_trace_is_admitted(
    const Nexus_V1_Engine *engine)
{
    const Nexus_V1_MenuBpkPaltTraceAdmissionReceipt *trace;

    if (!engine) return 0;
    trace = &engine->menu_bpk_palt_trace_admission;
    return trace->status == NEXUS_V1_MENU_BPK_PALT_TRACE_ADMITTED_OPAQUE &&
        trace->original_saturn_capture_verified &&
        trace->opaque_trace_admitted &&
        trace->raw_trace_bytes_bound &&
        trace->palt_memory_bytes_bound &&
        trace->palette_state_bytes_bound &&
        trace->vdp1_command_bytes_bound &&
        !trace->decoder_promoted && !trace->fallback_visuals_permitted;
}

int nexus_v1_menu_bpk_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkRendererHandoffReceipt *out_receipt) {
    const Nexus_V1_BpkRuntimeDecodeReceipt *decode;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING;
    out_receipt->prs3_prerequisite_status =
        NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING;
    out_receipt->decode_route = NEXUS_V1_BPK_DECODE_ROUTE_INVALID;
    /* MENU.BPK absence is never permission for a replacement menu surface. */
    out_receipt->blocks_real_menu_surface_render = 1;
    out_receipt->fallback_visuals_permitted = 0;

    if (!engine) return -1;
    out_receipt->attempted = engine->menu_bpk_decode_receipt_attempted;
    out_receipt->canonical_source_hash_verified =
        engine->menu_bpk_source.canonical_hash_verified ? 1 : 0;
    out_receipt->receipt_valid = engine->menu_bpk_decode_receipt_valid &&
        out_receipt->canonical_source_hash_verified;
    if (!out_receipt->canonical_source_hash_verified) {
        /* Preserve the user-facing distinction between no archive at all and
         * a present archive that fails the retail source identity gate. */
        if (engine->menu_bpk_source.exact_source_entry_observed ||
            engine->menu_bpk_source.hash_discovery_attempted) {
            out_receipt->status =
                NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE;
        }
        out_receipt->prs3_prerequisite_status =
            nexus_v1_menu_bpk_prs3_prerequisite_from_handoff(out_receipt);
        return 0;
    }
    if (engine->menu_bpk_upload_receipt_valid &&
        engine->menu_bpk_upload_receipt.palette_trailer_observed &&
        engine->menu_bpk_upload_receipt.palette_trailer.valid &&
        !engine->menu_bpk_upload_receipt.palette_trailer.palette_format_proven &&
        !engine->menu_bpk_upload_receipt.palette_trailer.decoder_promoted &&
        !engine->menu_bpk_upload_receipt.palette_trailer
             .fallback_visuals_permitted) {
        out_receipt->canonical_palette_trailer_bound = 1;
        out_receipt->palette_trailer =
            engine->menu_bpk_upload_receipt.palette_trailer;
    }
    if (!out_receipt->receipt_valid) {
        out_receipt->prs3_prerequisite_status =
            nexus_v1_menu_bpk_prs3_prerequisite_from_handoff(out_receipt);
        return 0;
    }

    decode = &engine->menu_bpk_decode_receipt;
    out_receipt->decode_route = decode->route;
    out_receipt->status =
        nexus_v1_menu_bpk_handoff_status_from_decode_route(decode->route);
    out_receipt->archive_entries = decode->archive_entries;
    out_receipt->surface_entries = decode->surface_entries;
    out_receipt->ready_stored_surfaces = decode->ready_stored_surfaces;
    out_receipt->blocked_prs3_surfaces = decode->blocked_prs3_surfaces;
    out_receipt->blocked_truncated_surfaces =
        decode->blocked_truncated_surfaces;
    out_receipt->prs3_stream_plans = decode->prs3_stream_plans;
    out_receipt->prs3_stream_plan_failures =
        decode->prs3_stream_plan_failures;
    out_receipt->first_blocked_entry = decode->first_blocked_entry;
    out_receipt->first_blocked_stream_offset =
        decode->first_blocked_stream_offset;
    out_receipt->first_blocked_stream_size =
        decode->first_blocked_stream_size;
    out_receipt->first_blocked_expected_output_bytes =
        decode->first_blocked_expected_output_bytes;

    out_receipt->can_render_stored_surfaces =
        (decode->route == NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED ||
         decode->route == NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED);
    out_receipt->blocks_real_menu_surface_render =
        (decode->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 ||
         decode->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED ||
         decode->route == NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES ||
         decode->route == NEXUS_V1_BPK_DECODE_ROUTE_INVALID);
    if (!out_receipt->blocks_real_menu_surface_render &&
        !nexus_v1_menu_bpk_palt_trace_is_admitted(engine)) {
        out_receipt->can_render_stored_surfaces = 0;
        out_receipt->blocks_real_menu_surface_render = 1;
        out_receipt->prs3_prerequisite_status =
            NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SATURN_PRESENTATION;
    }
    /* Stored original bytes may be presented by their own route, but no
     * BPK status ever authorizes a generated replacement surface. */
    out_receipt->fallback_visuals_permitted = 0;
    /* Preserve SATURN_PRESENTATION below when the source is not drawable. */
    out_receipt->prs3_prerequisite_status =
        nexus_v1_menu_bpk_prs3_prerequisite_from_handoff(out_receipt);
    return 0;
}

const char *nexus_v1_menu_bpk_renderer_handoff_status_name(
    Nexus_V1_MenuBpkRendererHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING: return "missing";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED:
        return "ready-stored";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3:
        return "blocked-prs3";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED:
        return "blocked-truncated";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES:
        return "no-surfaces";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE:
        return "blocked-source";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED:
        return "ready-decoded";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID: return "invalid";
    default: return "unknown";
    }
}

const char *nexus_v1_menu_bpk_prs3_prerequisite_status_name(
    Nexus_V1_MenuBpkPrs3PrerequisiteStatus status) {
    switch (status) {
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING:
        return "archive-missing";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SOURCE_UNVERIFIED:
        return "source-unverified";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_INVALID:
        return "archive-invalid";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_FRAME_INCOMPLETE:
        return "frame-incomplete";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_AUTHENTIC_DECODER:
        return "authentic-decoder-required";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SATURN_PRESENTATION:
        return "saturn-presentation-required";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED:
        return "ready-stored";
    default: return "unknown";
    }
}

const char *nexus_v1_menu_bpk_prs3_prerequisite_message(
    Nexus_V1_MenuBpkPrs3PrerequisiteStatus status) {
    switch (status) {
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING:
        return "MENU.BPK NOT FOUND";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SOURCE_UNVERIFIED:
        return "MENU.BPK SOURCE NOT VERIFIED";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_INVALID:
        return "MENU.BPK ARCHIVE INVALID";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_FRAME_INCOMPLETE:
        return "MENU.BPK PRS3 FRAME INVALID";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_AUTHENTIC_DECODER:
        return "MENU.BPK PRS3 TRACE REQUIRED";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SATURN_PRESENTATION:
        return "SATURN PRESENTATION TRACE REQUIRED";
    case NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED:
        return "MENU.BPK STORED SURFACE ROUTE";
    default:
        return "MENU.BPK STATUS UNKNOWN";
    }
}

int nexus_v1_engine_build_menu_bpk_palt_capture_target(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt *out_target)
{
    Nexus_V1_MenuBpkRendererHandoffReceipt handoff;
    const Nexus_V1_BpkPaletteTrailerReceipt *palette;

    if (!out_target) return -1;
    memset(out_target, 0, sizeof(*out_target));
    out_target->original_saturn_capture_required = 1;
    out_target->palt_memory_read_observation_required = 1;
    out_target->palette_state_observation_required = 1;
    out_target->vdp1_command_observation_required = 1;
    out_target->no_draw_only = 1;
    if (!engine ||
        nexus_v1_menu_bpk_renderer_handoff_receipt(engine, &handoff) != 0 ||
        !handoff.canonical_source_hash_verified ||
        !handoff.canonical_palette_trailer_bound ||
        !engine->menu_bpk_source.canonical_name[0] ||
        !engine->menu_bpk_source.canonical_md5[0]) {
        return 0;
    }
    palette = &handoff.palette_trailer;
    if (!palette->valid || !palette->record_bytes || !palette->entry_count ||
        !palette->entry_bytes || !palette->entry_bytes_fnv1a64 ||
        palette->palette_format_proven || palette->decoder_promoted ||
        palette->fallback_visuals_permitted) {
        return 0;
    }

    snprintf(out_target->canonical_menu_bpk_name,
             sizeof(out_target->canonical_menu_bpk_name), "%s",
             engine->menu_bpk_source.canonical_name);
    snprintf(out_target->canonical_menu_bpk_md5,
             sizeof(out_target->canonical_menu_bpk_md5), "%s",
             engine->menu_bpk_source.canonical_md5);
    out_target->palt_record_offset = palette->record_offset;
    out_target->palt_record_bytes = palette->record_bytes;
    out_target->palt_entry_count = palette->entry_count;
    out_target->palt_entry_bytes = palette->entry_bytes;
    out_target->palt_entry_bytes_fnv1a64 = palette->entry_bytes_fnv1a64;
    out_target->valid = 1;
    return 1;
}

int nexus_v1_engine_menu_bpk_palt_warning_palette_correlation(
    Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPaltWarningPaletteCorrelationReceipt *out_receipt)
{
    Nexus_V1_MenuBpkPaltWarningPaletteCorrelationReceipt receipt;
    Nexus_V1_LevelAuxSourceReceipt menu_bpk_source;
    Nexus_V1_LevelAuxSourceReceipt warning_source;
    Nexus_UI_Dgt2PpView warning_view;
    uint8_t *menu_bpk = NULL;
    uint8_t *warning = NULL;
    int menu_bpk_size = 0;
    int warning_size = 0;
    uint32_t entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    memset(&menu_bpk_source, 0, sizeof(menu_bpk_source));
    memset(&warning_source, 0, sizeof(warning_source));
    if (!engine ||
        nexus_v1_level_aux_source_receipt(engine, "MENU.BPK",
                                          &menu_bpk_source) != 0 ||
        nexus_v1_level_aux_source_receipt(engine, "WARNING.BIN",
                                          &warning_source) != 0 ||
        !menu_bpk_source.canonical_hash_verified ||
        !warning_source.canonical_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.menu_bpk_source_hash_verified = 1;
    receipt.warning_source_hash_verified = 1;
    menu_bpk = nexus_v1_read_file(engine, "MENU.BPK", &menu_bpk_size);
    warning = nexus_v1_read_file(engine, "WARNING.BIN", &warning_size);
    if (!menu_bpk || !warning ||
        nexus_v1_bpk_archive_inspect_palette_trailer(
            menu_bpk, (size_t)menu_bpk_size, &receipt.palt) != 0 ||
        !receipt.palt.valid || receipt.palt.entry_count != 256U ||
        receipt.palt.entry_bytes != 512U ||
        menu_bpk_size <= 0 ||
        receipt.palt.record_bytes > (uint32_t)menu_bpk_size ||
        receipt.palt.record_offset > (uint32_t)menu_bpk_size -
            receipt.palt.record_bytes ||
        nexus_ui_res_dgt2_pp_view(warning, (size_t)warning_size, 0U,
                                  &warning_view) != 0 ||
        !warning_view.clut_bgr555_be) {
        free(warning);
        free(menu_bpk);
        *out_receipt = receipt;
        return 0;
    }
    receipt.warning_dgt2_pp_bound = 1;
    receipt.warning_clut_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        warning_view.clut_bgr555_be, 512U);
    for (entry = 0U; entry < receipt.palt.entry_count; ++entry) {
        const uint8_t *palt_word = menu_bpk + receipt.palt.record_offset +
            12U + entry * 2U;
        const uint8_t *warning_word = warning_view.clut_bgr555_be + entry * 2U;
        uint16_t palt_value = (uint16_t)((uint16_t)palt_word[0] << 8) |
            (uint16_t)palt_word[1];
        uint16_t warning_value = (uint16_t)((uint16_t)warning_word[0] << 8) |
            (uint16_t)warning_word[1];
        if (palt_word[0] == warning_word[0] &&
            palt_word[1] == warning_word[1]) {
            ++receipt.matching_entry_count;
        } else {
            ++receipt.mismatched_entry_count;
        }
        if ((palt_value & 0x7fffU) == (warning_value & 0x7fffU)) {
            ++receipt.bgr555_low15_matching_entry_count;
            if (palt_value != warning_value) {
                ++receipt.high_bit_only_mismatch_count;
            }
        } else {
            ++receipt.bgr555_low15_mismatched_entry_count;
            ++receipt.colour_word_mismatch_count;
        }
    }
    /* This fixed canonical pair shares 224 indexed BE16 words. The 32
     * differing slots remain source data, not default colours or an alpha
     * convention. The DGT2 side supplies the documented BGR555 word layout;
     * PRS3 ownership and palette application are intentionally still absent. */
    receipt.indexed_word_alignment_proven =
        receipt.matching_entry_count == 224U &&
        receipt.mismatched_entry_count == 32U;
    receipt.bgr555_word_encoding_correlation_proven =
        receipt.indexed_word_alignment_proven;
    /* The documented DGT2 reader uses only BGR555's low 15 colour bits.
     * Thirty-one non-identical PALT words differ only in bit 15; index 130
     * remains a genuine colour-word disagreement. This is correlation, not
     * a PALT format declaration or an authorization to apply the palette. */
    receipt.bgr555_low15_correlation_proven =
        receipt.bgr555_low15_matching_entry_count == 255U &&
        receipt.bgr555_low15_mismatched_entry_count == 1U &&
        receipt.high_bit_only_mismatch_count == 31U &&
        receipt.colour_word_mismatch_count == 1U;
    receipt.valid = receipt.indexed_word_alignment_proven;
    free(warning);
    free(menu_bpk);
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_engine_write_menu_bpk_palt_capture_target(
    const Nexus_V1_Engine *engine, const char *path,
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt *out_target)
{
    char temporary_path[1024];
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt target;
    FILE *file;
    int write_ok;

    if (!path || !out_target ||
        nexus_v1_engine_build_menu_bpk_palt_capture_target(engine, &target) != 1 ||
        !target.valid || !target.original_saturn_capture_required ||
        !target.palt_memory_read_observation_required ||
        !target.palette_state_observation_required ||
        !target.vdp1_command_observation_required ||
        target.palt_palette_relation_proven || target.decoder_promoted ||
        !target.no_draw_only || target.fallback_visuals_permitted ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) >=
            (int)sizeof(temporary_path)) {
        return 0;
    }
    file = fopen(temporary_path, "wb");
    if (!file) return 0;
    write_ok = fprintf(file,
                       NEXUS_V1_MENU_BPK_PALT_CAPTURE_TARGET_MAGIC "\n"
                       "canonical_menu_bpk_name=%s\ncanonical_menu_bpk_md5=%s\n"
                       "palt_record_offset=%x\npalt_record_bytes=%x\n"
                       "palt_entry_count=%x\npalt_entry_bytes=%x\n"
                       "palt_entry_bytes_fnv1a64=%016llx\n"
                       "capture_kind=original_saturn_menu_palt_correlation\n"
                       "required_observations=palt_memory_read,palette_state,vdp1_command\n"
                       "palt_palette_relation_proven=0\ndecoder_promoted=0\n"
                       "no_draw_only=1\n",
                       target.canonical_menu_bpk_name,
                       target.canonical_menu_bpk_md5, target.palt_record_offset,
                       target.palt_record_bytes, target.palt_entry_count,
                       target.palt_entry_bytes,
                       (unsigned long long)target.palt_entry_bytes_fnv1a64) >= 0;
    if (fclose(file) != 0) write_ok = 0;
    if (!write_ok || rename(temporary_path, path) != 0) {
        remove(temporary_path);
        return 0;
    }
    *out_target = target;
    return 1;
}

int nexus_v1_engine_admit_menu_bpk_palt_trace(
    Nexus_V1_Engine *engine, const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *palt_memory, size_t palt_memory_size,
    const uint8_t *palette_state, size_t palette_state_size,
    const uint8_t *vdp1_command, size_t vdp1_command_size,
    int original_saturn_capture_verified,
    Nexus_V1_MenuBpkPaltTraceAdmissionReceipt *out_receipt)
{
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt target;
    Nexus_V1_MenuBpkPaltTraceAdmissionReceipt receipt;
    char value[96];
    uint64_t expected_hash;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_MISSING;
    receipt.no_draw_only = 1;
    if (!engine || !manifest_text || manifest_size == 0 || !raw_trace ||
        raw_trace_size == 0 || !palt_memory || palt_memory_size == 0 ||
        !palette_state || palette_state_size == 0 || !vdp1_command ||
        vdp1_command_size == 0 ||
        nexus_v1_engine_build_menu_bpk_palt_capture_target(engine, &target) != 1) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_target_bound = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size, "magic", value,
                                    sizeof(value)) ||
        strcmp(value, NEXUS_V1_MENU_BPK_PALT_TRACE_MAGIC) != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "producer", value,
                                    sizeof(value)) ||
        strcmp(value, "mednafen-debugger") != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size, "trace_sha256",
                                    value, sizeof(value)) ||
        !nexus_v1_slev_trace_is_sha256(value)) {
        receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_MALFORMED;
        *out_receipt = receipt;
        return 0;
    }
    receipt.mednafen_debugger_provenance = 1;
    receipt.trace_sha256_present = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "canonical_menu_bpk_md5", value,
                                    sizeof(value)) ||
        strcmp(value, target.canonical_menu_bpk_md5) != 0 ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_record_offset", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.palt_record_offset ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_record_bytes", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.palt_record_bytes ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_entry_count", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.palt_entry_count ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_entry_bytes", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.palt_entry_bytes ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_entry_bytes_fnv1a64", value,
                                    sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != target.palt_entry_bytes_fnv1a64) {
        receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_TARGET_MISMATCH;
        *out_receipt = receipt;
        return 0;
    }
    receipt.manifest_target_bound = 1;
    if (!nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "raw_trace_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != nexus_v1_slev_trace_fnv1a64(raw_trace, raw_trace_size) ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palt_memory_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != nexus_v1_slev_trace_fnv1a64(palt_memory, palt_memory_size) ||
        expected_hash != target.palt_entry_bytes_fnv1a64 ||
        palt_memory_size != target.palt_entry_bytes ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "palette_state_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != nexus_v1_slev_trace_fnv1a64(palette_state,
                                                      palette_state_size) ||
        !nexus_v1_slev_trace_value(manifest_text, manifest_size,
                                    "vdp1_command_fnv1a64", value, sizeof(value)) ||
        !nexus_v1_slev_trace_hex_u64(value, &expected_hash) ||
        expected_hash != nexus_v1_slev_trace_fnv1a64(vdp1_command,
                                                      vdp1_command_size)) {
        receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_OBSERVATIONS;
        *out_receipt = receipt;
        return 0;
    }
    receipt.raw_trace_bytes_bound = 1;
    receipt.raw_trace_byte_count = raw_trace_size;
    receipt.raw_trace_fnv1a64 = nexus_v1_slev_trace_fnv1a64(raw_trace,
                                                             raw_trace_size);
    receipt.palt_memory_bytes_bound = 1;
    receipt.palt_memory_byte_count = palt_memory_size;
    receipt.palt_memory_fnv1a64 = target.palt_entry_bytes_fnv1a64;
    receipt.palette_state_bytes_bound = 1;
    receipt.palette_state_byte_count = palette_state_size;
    receipt.palette_state_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        palette_state, palette_state_size);
    receipt.vdp1_command_bytes_bound = 1;
    receipt.vdp1_command_byte_count = vdp1_command_size;
    receipt.vdp1_command_fnv1a64 = nexus_v1_slev_trace_fnv1a64(
        vdp1_command, vdp1_command_size);
    if (!original_saturn_capture_verified) {
        receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_PROVENANCE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.original_saturn_capture_verified = 1;
    receipt.opaque_trace_admitted = 1;
    receipt.status = NEXUS_V1_MENU_BPK_PALT_TRACE_ADMITTED_OPAQUE;
    engine->menu_bpk_palt_trace_admission = receipt;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_current_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
        return 0;
    }
    if (nexus_v1_level_dgn_renderer_handoff_receipt(&engine->current_level,
                                                    out_receipt) != 0) {
        return -1;
    }
    nexus_v1_dgn_renderer_handoff_require_canonical_source(
        out_receipt,
        engine->current_level_structure2_source.canonical_hash_verified);
    return 0;
}

int nexus_v1_current_level_script_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_ScriptRuntimeReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_SCRIPT_RUNTIME_MISSING;
        out_receipt->level_index = -1;
        return 0;
    }
    *out_receipt = engine->script_runtime_receipt;
    return 0;
}

int nexus_v1_current_level_sfx_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_SfxRuntimeReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_SFX_RUNTIME_MISSING;
        out_receipt->level_index = -1;
        out_receipt->fallback_visuals_permitted = 0;
        return 0;
    }
    *out_receipt = engine->sfx_runtime_receipt;
    return 0;
}
