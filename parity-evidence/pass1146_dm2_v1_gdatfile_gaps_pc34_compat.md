# Pass 1146 — DM2 gdatfile gaps filled

## Source

skproject/SKULLWIN/c_gdatfile.cpp

## Module

include/dm2_v1_gdatfile_pc34_compat.h
src/dm2/dm2_v1_gdatfile_pc34_compat.c

## Functions added

| Function | skproject source | Description |
|----------|-----------------|-------------|
| dm2_v1_gdat_alloc_new_bmp | DM2_ALLOC_NEW_BMP (line 560) | Allocate bitmap via alloc_pict_buff |
| dm2_v1_gdat_load_entry_data_to | DM2_LOAD_GDAT_ENTRY_DATA_TO (line 818) | Load GDAT entry by cls1/cls2/type/idx |
| dm2_v1_gdat_track_underlay | DM2_TRACK_UNDERLAY (line 997) | Binary search in underlay table |

## Architecture

dm2_v1_gdat_alloc_new_bmp delegates to alloc_pict_buff.
dm2_v1_gdat_load_entry_data_to uses DM2_V1_GdatLoadEntryCallbacks.
dm2_v1_gdat_track_underlay uses DM2_V1_GdatTrackUnderlayReceipt.

## Verification

Compiles without errors via ninja -C build.
