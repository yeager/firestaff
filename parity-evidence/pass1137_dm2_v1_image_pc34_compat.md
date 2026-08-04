# Pass 1137 — DM2 V1 image loading/drawing (c_image.cpp)

## Source
skproject SKULLWIN/c_image.cpp — 17 functions for image loading and drawing.

## Ported functions
- c_imgdesc::init — zero image descriptor
- c_image::init — zero image struct and palette
- DM2_SET_IMAGE — populate imgdesc from database bitmap (width/height/res from header)
- DM2_ALLOCATE_IMG_COPY — allocate image copy (mode 0x8=256-color, 0x4=16-color)
- DM2_image_0b36_01cd — deallocate image copy
- imgdesc_getres / imgdesc_setres — resolution accessors
- init_global_images — initialize two global image structs (dm2_image1, dm2_image2)
- DM2_0b36_068f — compute image cache key (bmpid<<8 | mode&0xff)
- DM2_image_0b36_11c0 — draw image to button group area
- DM2_QUERY_PICST_IT — prepare image for rendering (stretch/cache check)
- DM2_DRAW_PICST — core image drawing function
- DM2_QUERY_MULTILAYERS_PIC — query multi-layer picture
- DM2_DRAW_TEMP_PICST — draw temporary picture (delegates to DRAW_PICST)
- DM2_DRAW_TRANSPARENT_STATIC_PIC — draw transparent static picture
- DM2_DRAW_STATIC_PIC — draw static picture
- DM2_DRAW_DUNGEON_GRAPHIC — draw dungeon graphic with palette
- DM2_image_0b36_1446 — pixel comparison test at (x,y)
- DEBUGBLIT — debug blit (no-op in release)

## Files
- include/dm2_v1_image_pc34_compat.h
- src/dm2/dm2_v1_image_pc34_compat.c
- tests/test_dm2_v1_image_pc34_compat.c

## Test
All tests pass (dm2_v1_image).
