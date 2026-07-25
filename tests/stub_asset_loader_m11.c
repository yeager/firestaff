#include "asset_loader_m11.h"

int M11_AssetLoader_Init(M11_AssetLoader* loader, const char* graphicsDatPath) {
    (void)loader; (void)graphicsDatPath; return 0;
}
void M11_AssetLoader_Shutdown(M11_AssetLoader* loader) { (void)loader; }
int M11_AssetLoader_IsReady(const M11_AssetLoader* loader) { (void)loader; return 0; }
const M11_AssetSlot* M11_AssetLoader_Load(M11_AssetLoader* loader, unsigned int graphicIndex) {
    (void)loader; (void)graphicIndex; return 0;
}
void M11_AssetLoader_Blit(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int tc) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)tc;
}
void M11_AssetLoader_BlitRegion(const M11_AssetSlot* s,
    int sx, int sy, int sw, int sh,
    unsigned char* fb, int w, int h, int x, int y, int tc) {
    (void)s;(void)sx;(void)sy;(void)sw;(void)sh;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)tc;
}
void M11_AssetLoader_BlitScaled(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int dw, int dh, int tc) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)dw;(void)dh;(void)tc;
}
void M11_AssetLoader_BlitScaledMirror(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int dw, int dh, int tc) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)dw;(void)dh;(void)tc;
}
void M11_AssetLoader_BlitScaledReplace(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int dw, int dh, int tc,
    int rs9, int rd9, int rs10, int rd10) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)dw;(void)dh;(void)tc;
    (void)rs9;(void)rd9;(void)rs10;(void)rd10;
}
void M11_AssetLoader_BlitScaledMirrorReplace(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int dw, int dh, int tc,
    int rs9, int rd9, int rs10, int rd10) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)dw;(void)dh;(void)tc;
    (void)rs9;(void)rd9;(void)rs10;(void)rd10;
}
void M11_AssetLoader_BlitSubRectScaled(const M11_AssetSlot* s, unsigned char* fb,
    int w, int h, int x, int y, int dw, int dh,
    int sx, int sy, int sw, int sh, int tc) {
    (void)s;(void)fb;(void)w;(void)h;(void)x;(void)y;(void)dw;(void)dh;
    (void)sx;(void)sy;(void)sw;(void)sh;(void)tc;
}
