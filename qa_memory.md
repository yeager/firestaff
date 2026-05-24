# DM1 V1 Memory Safety Audit

## Memory Allocation Patterns

Dungeon Loader: Static tile storage (no heap). Bounds checks prevent overflow.
Graphics Loader: malloc in read_bitmap, freed in cleanup. NULL guards present.
Title Screen: screen_buffers[2] static allocation, free with NULL guard in cleanup.
Save/Load: malloc(blobSize), free on all error paths. NULL check on malloc result.
Decompressor: heap allocated decomp_buffer, freed in cleanup. LZW bounded.
RATING: SAFE - no memory leaks or buffer overflows found.

## String Operations

All snprintf (no sprintf/strcpy/strcat/gets/scanf). All uses have sizeof(dest) or caller-allocated buffers.
RATING: SAFE.

## Buffer Overflow Checks

Dungeon loader: w/h <= 32, level_count bounded, fread count validated.
Object interaction: hardcoded 32x32 checks inline (not using DM1_MAX_MAP_W/H - maintenance concern).
Tile get: full bounds check on state, level, x, y against MAX constants.
RATING: SAFE.

## Null Pointer Guards

All public APIs: state/path null checks on entry. RATING: SAFE.

## Issues Found

Issue 1: Hardcoded 32 in object_interaction.c instead of DM1_MAX_MAP_W/H constant.
Severity: LOW - no runtime bug, maintenance risk only.

Issue 2: No ferror() check after file operations.
Severity: LOW - count checks catch most failures.

## Summary

Overall: SAFE. No malloc without free on error paths. No sprintf/strcpy vulnerabilities.
No buffer overflow. Consistent null checks. No critical/high severity issues.
