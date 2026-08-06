# Theron V1 production placeholder boundary

The production `firestaff_theron` archive is guarded in two places:

- `CMakeLists.txt` excludes the inventoried fixture/compatibility sources
  before the production target is created.
- `tests/test_theron_v1_production_archive_source_boundary.sh` checks both
  the CMake exclusion rows and the final archive members.

The boundary currently covers twelve modules: the legacy compatibility,
creature, shop, startup-receipt, viewport/UI, V2 HUD, and V2.2 modern-art
implementations. Runtime no-op seams and the source-gated combat bridge are
not counted as visual or gameplay implementations; they remain fail-closed
until the corresponding original Track 02 consumer is proven.

This is an inventory guard, not a claim that the missing consumers are
decoded. A module may leave the boundary only with source/disassembly
evidence, a production handoff, and a regression against authenticated JP/US
data. No game data is copied into the repository.
