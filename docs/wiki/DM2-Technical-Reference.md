# Dungeon Master II: Skullkeep Technical Reference

## Scope

DM2 V1 follows skproject. It is independent from the DM1/CSB compatibility
path and requires original PC `GRAPHICS.DAT` plus `DUNGEON.DAT` by hash.

## Boot and GDAT

An incomplete profile remains at the launcher and does not clear the viewport
or paint generated floor/ceiling art. The boot receipt records source identity,
typed GDAT sections, title/menu, HUD, dungeon, and no-fallback readiness.

The GDAT pipeline exposes `dtPalIRGB`, `dtPalette16`, `dt07` interface/title
records, Rect14 placement, map GRAPHICSSET values, and map-chip assets. The
viewport resolves raw GDAT addresses before decode. Missing mandatory material
is a no-draw receipt, never a substitute. Wall cache keys include graphics
style to prevent material leakage between maps.

## Runtime

M10 owns party, door, item, creature, projectile, actuator, and dungeon state.
The viewport consumes typed plans. Outdoor routes pass the active source sky,
ground, and weather state; indoor routes are clear. Rain, mist, and thunder do
not create procedural pixels without a verified original overlay asset.

## Verification

```bash
cmake --build build --target test_dm2_v1_boot_profile_smoke \
  test_dm2_v1_runtime_handoff_smoke test_dm2_v1_weather_no_synthetic_overlay \
  --parallel
./build/test_dm2_v1_boot_profile_smoke
./build/test_dm2_v1_runtime_handoff_smoke
./build/test_dm2_v1_weather_no_synthetic_overlay
```
