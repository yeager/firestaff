# Pass 1112 — DM2 GFX Blit Engine (c_gfx_blit.cpp)

## Source

skproject `SKULLWIN/c_gfx_blit.cpp` (~1260 lines, 37 functions).

## Target

- Header: `include/dm2_v1_gfx_blit_pc34_compat.h`
- Source: `src/dm2/dm2_v1_gfx_blit_pc34_compat.c`
- Test: `tests/test_dm2_v1_gfx_blit_pc34_compat.c`

## Ported Functions (37)

### Blitter lifecycle
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::init` | `dm2_v1_blit_init` |

### 4bpp-to-4bpp (blitline_44 family)
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blitline_44_` (DM2_blit_15B5) | `dm2_v1_blit_blitline_44_plain` |
| `c_blitter::blitline_44_ma` (R_190F) | `dm2_v1_blit_blitline_44_masked` |
| `c_blitter::blitline_44_mi` (R_1761) | `dm2_v1_blit_blitline_44_mirror` |
| `c_blitter::blitline_44_mima` (SKW_FIRE_BLIT_TO_MEMORY_ROW_4TO4BPP) | `dm2_v1_blit_blitline_44_mirror_masked` |
| `c_blitter::blitline_44` (SKW_FIRE_BLIT_TO_MEMORY_4TO4BPP) | `dm2_v1_blit_blitline_44` |

### 4bpp-to-8bpp (blitline_48 family)
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blitline_48_` (DM2_BLIT_TO_MEMORY_ROW_4TO8BPP_NOKEY) | `dm2_v1_blit_blitline_48_plain` |
| `c_blitter::blitline_48_ma` (DM2_blit_44c8_08ae) | `dm2_v1_blit_blitline_48_masked` |
| `c_blitter::blitline_48_mi` (R_2035) | `dm2_v1_blit_blitline_48_mirror` |
| `c_blitter::blitline_48_mima` (SKW_FIRE_BLIT_TO_MEMORY_ROW_4TO8BPP) | `dm2_v1_blit_blitline_48_mirror_masked` |
| `c_blitter::blitline_48` (SKW_FIRE_BLIT_TO_MEMORY_4TO8BPP) | `dm2_v1_blit_blitline_48` |

### 8bpp-to-8bpp (blitline_88 family)
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blitline_88_` (SKW_44c8_0b8d) | `dm2_v1_blit_blitline_88_plain` |
| `c_blitter::blitline_88_ma` (SKW_44c8_0bc5) | `dm2_v1_blit_blitline_88_masked` |
| `c_blitter::blitline_88_mi` (SKW_44c8_0bf8) | `dm2_v1_blit_blitline_88_mirror` |
| `c_blitter::blitline_88_mima` (SKW_44c8_0c3c) | `dm2_v1_blit_blitline_88_mirror_masked` |
| `c_blitter::blitline_88` (SKW_FIRE_BLIT_TO_MEMORY_8TO8BPP) | `dm2_v1_blit_blitline_88` |

### 8bpp-to-8bpp translated (blitline_88xlat family)
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blitline_88xlat_` (R_2871) | `dm2_v1_blit_blitline_88xlat_plain` |
| `c_blitter::blitline_88xlat_ma` (R_28A2) | `dm2_v1_blit_blitline_88xlat_masked` |
| `c_blitter::blitline_88xlat_mi` (R_28DF) | `dm2_v1_blit_blitline_88xlat_mirror` |
| `c_blitter::blitline_88xlat_mima` (R_291B) | `dm2_v1_blit_blitline_88xlat_mirror_masked` |
| `c_blitter::blitline_88xlat` (SKW_FIRE_BLIT_TO_MEMORY_8TO8BPP_TRANSLATED) | `dm2_v1_blit_blitline_88xlat` |

### Unified dispatcher
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blit` (DM2_BLIT_PICTURE) | `dm2_v1_blit_picture` |

### Fill operations
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::fill_line_4` (DM2_FILL_4BPP_PICT_LINE) | `dm2_v1_blit_fill_line_4` |
| `c_blitter::fill_line_8` (DM2_FILL_8BPP_PICT_LINE) | `dm2_v1_blit_fill_line_8` |
| `c_blitter::fill_4` (DM2_FILL_RECT_4BPP_PICT) | `dm2_v1_blit_fill_4` |
| `c_blitter::fill_8` (DM2_FILL_RECT_8BPP_PICT) | `dm2_v1_blit_fill_8` |
| `c_blitter::fill` (DM2_FILL_RECT_ANY) | `dm2_v1_blit_fill` |

### Stretch operations
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::calc_stretched_size` (DM2_CALC_STRETCHED_SIZE) | `dm2_v1_blit_calc_stretched_size` |
| `c_blitter::stretch16_sub1` (SKW_44c8_2143) | `stretch16_sub1` (static) |
| `c_blitter::stretch16_sub2` (SKW_44c8_20e5) | `stretch16_sub2` (static) |
| `c_blitter::stretch16` (DM2_STRETCH_BLIT_TO_MEMORY_4TO4BPP) | `dm2_v1_blit_stretch16` |
| `c_blitter::stretch256` (DM2_image_44c8_2351) | `dm2_v1_blit_stretch256` |

### Screen and special effects
| skproject | Firestaff |
|-----------|-----------|
| `c_blitter::blit_within_screen` (sub_25AF) | `dm2_v1_blit_within_screen` |
| `DM2_sub_blit_specialeffects` (DM2_blit_44c8_1e43) | `dm2_v1_blit_sub_specialeffects` |
| `DM2_blit_specialeffects` (DM2_blit_44c8_20a4) | `dm2_v1_blit_specialeffects` |
| `c_blitter::stretch_4to8` (DM2_guidraw_44c8_1aca) | `dm2_v1_blit_stretch_4to8` |

### Helpers (static, inline)
| skproject | Firestaff |
|-----------|-----------|
| `xlat` | Inlined as `palette[pix]` |
| `build_pixels16` | `build_pixels16` (static inline) |
| `build_pixels_masked16` | `build_pixels_masked16` (static inline) |

## Architecture Changes

- C++ `c_blitter` class → `DM2_V1_BlitterState` struct
- `c_blitter blitter` global singleton → state pointer passed to all functions
- `xblitb[0x1000]` loaded from file → passed via `dm2_v1_blit_init`
- `paldat.pal16to256ptr` global → callback `get_pal16to256`
- `DM2_UPDATE_BLIT_PALETTE` → callback `update_blit_palette`
- C++ `reinterpret_cast` → direct `uint8_t*` buffer operations
- `c_pixel16` / `c_pixel256` class methods → static inline nibble helpers

## Test Results

16/16 tests pass:

- init, init_null_xblitb
- fill_8, fill_4, fill_zero_rect
- blit_88_plain, blit_88_masked, blit_88_mirror, blit_88_vmirror, blit_88_zero_size
- blit_88xlat_plain
- blit_44_aligned_plain
- blit_picture_null_rect, blit_picture_88
- calc_stretched_size
- source_evidence
