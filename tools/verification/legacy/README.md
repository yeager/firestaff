# Retained verification scripts

This directory contains standalone CSB and DM2 source-analysis scripts kept
for historical and technical reference. They are not wired into CTest and do
not make runtime claims. Active project verifiers remain in `tools/` and are
listed by CMake.

The scripts discover the repository root from `CMakeLists.txt`, so they can be
run from any checkout location. Their optional reference-data inputs are
operator supplied and are never required by the Firestaff runtime.
