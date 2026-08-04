# Pass 1094 — DM2 string utilities (skstr.cpp)

## Source

skproject/SKWINSPX/src/v5/skstr.{h,cpp}

## What was ported

DM2's string formatting utilities used by the HUD text system:
character-to-script conversion, integer-to-decimal, formatted number
with optional padding, and strided fill.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| DM2_SKCHR_TO_SCRIPTCHR | dm2_v1_skchr_to_scriptchr |
| DM2_LTOA10 | dm2_v1_ltoa10 |
| DM2_FMT_NUM | dm2_v1_fmt_num |
| DM2_FILL_STR | dm2_v1_fill_str |

## Tests

11 tests: uppercase letter mapping, dot mapping, other-char fallback,
positive/zero/negative ltoa10, unpadded/zero/padded fmt_num,
fill with step 1 and step 2.

## Files

- `include/dm2_v1_string_pc34_compat.h`
- `src/dm2/dm2_v1_string_pc34_compat.c`
- `tests/test_dm2_v1_string_pc34_compat.c`
