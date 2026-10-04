# Firestaff codebase reduction plan

## Goal and boundaries

Reduce the size and maintenance cost of the checked-out source tree while
preserving every supported game's existing behavior. DM1, CSB and DM2 are the
first implementation scope; Nexus and Theron's Quest have separate owners.
Keep original-media tests and source evidence strong enough to detect a lost
startup, menu, input, sound, dungeon, or save path. Visual parity work is
deferred. Do not add original game media, credentials, or synthetic substitutes
to the repository. No release is part of this plan.

## Measured baseline (2026-10-04)

`git ls-files` reports 15,653 tracked files. Counting physical lines in
tracked text files gives approximately 3.9 million lines. The large areas
are:

| Area | Tracked text lines | Interpretation |
| --- | ---: | --- |
| `src/` | 1,104,513 | Production and compatibility implementation |
| `include/` | 311,077 | Public and internal declarations |
| `tests/` | 813,575 | Regression coverage, including original-media gates |
| `probes/` | 493,911 | Development probes and historical experiments |
| `parity-evidence/` | 413,369 | Source and comparison evidence |
| `po/` | 216,379 | Localization catalogs |
| `CMakeLists.txt` | 62,668 | Build and test declarations |

The three compiled UI image-array files
`src/ui/branding_firestaff_rail_m12.c`,
`src/ui/branding_logo_readme_m12.c`, and
`src/shared/card_art_generated_m12.c` account for 94,608 lines, almost all
decimal RGB values. `src/engine/m11_game_view.c` has 73,183 lines. These
figures describe different problems: packed presentation data, accumulated
build/test declarations, retained experiments, and large runtime owners.
Splitting a file without removing duplication is not counted as a size win.

## Work sequence

### 1. Record a behavior and size baseline

Before each reduction batch, record the tracked-file and line counts, build
time, package size, and the exact Git commit. Keep an original-media matrix
for CLI and M12 launches of DM1, CSB and DM2, including the default FM Towns
selection and the first source-owned runtime state. Keep the existing
cross-platform CI, save, input, audio, and startup gates. A passing narrow
test must not be described as complete game parity or physical M5 coverage.

### 2. Remove generated RGB numbers from the tracked C source

Pilot one launcher image. Store its exact bytes as a compact, versioned
project-owned asset and generate the C array in the build directory with a
deterministic, cross-platform tool. Compare every decoded byte and the public
array size with the current representation, then build and package on macOS,
Windows, and Linux. If the pilot passes, migrate the other two files. Keep the
bytes bundled with the executable; do not add a runtime search path or disk
cache for game media. Expected checkout reduction: about 94,600 lines, with
no intended pixel or behavior change.

### 3. Audit and retire historical probes and evidence

Build a manifest for every `probes/` and `parity-evidence/` file: producer,
consumer, source-media provenance, last successful run, and whether an active
test or document requires it. A literal-path search found 177 probe files
(about 184,000 lines) and 2,353 evidence files (about 174,000 lines) without
references from CMake, tests, scripts, docs, or tools. This is an audit queue,
not a deletion list: globs, basename lookups, manual investigations, and
historical proof can still depend on these files. Retire only items whose
current role is disproven or replaced by a stronger gate; preserve provenance
and checksums in a manifest and Git history. Do not rewrite Git history in
this phase. The candidate upper bound is about 358,000 lines, not a promised
saving.

### 4. Simplify the build and test graph

Inventory CMake targets and source lists before changing them. Move repeated
test setup into small CMake helpers and game-scoped manifests, keeping the
same test names, labels, environment, skip codes, and commands. Consolidate
tests only when their edition-specific assertions remain visible and run at
the same CI gates. Measure both CMake line reduction and configuration time;
do not replace many explicit tests with a single weak smoke check. The
62,668-line root CMake file is the first target.

### 5. Consolidate host orchestration, then game runtime owners

Start with low-risk duplication in launcher lifetime and error cleanup:
`src/engine/m11_game_view.c` has separate CSB and DM2 allocate/detach/apply/
cleanup sequences, and `src/engine/firestaff_game_loop.c` repeats direct
launch orchestration. Share host ownership only; keep each edition's media
admission and source receipts separate. Source review suggests roughly
75–180 lines of net removal here. Next inspect CSB's extra startup asset
scan before eliminating it, since intro ordering may require it.

Treat the 73,183-line M11 view, 33,301-line CSB runtime, and 25,591-line
DM2 SKProject core as separate owner-mapping projects. Extract cohesive
modules only after their call graph and state ownership are explicit. Remove
duplicate branches and dead adapters when a source comparison and a
regression prove that no edition uses them. Six old F9003–F9009 wrappers
(377 lines) are absent from the production CMake graph but still feed probe
scripts; they require probe migration before retirement.

## Batch acceptance and reporting

Each batch must state files and lines removed, replacement lines added, build
time and package-size changes, and the exact behavior protected. Use original
DM1/CSB media with ReDMCSB and original DM2 media with SKProject for changed
game paths. Run focused tests first, then the relevant CLI/M12 original-media
routes and hosted platform CI. Run Gitleaks with redacted output before each
commit and push. Wait for the preceding Actions run before another push to
main. Keep open parity gaps in TODO files and verified outcomes in DONE files.

The first measurable checkpoint is the lossless UI-data pilot. The second is
an audited probe/evidence retirement batch. Re-estimate any larger line-count
target from those results rather than deleting source-specific behavior to
meet an arbitrary number.
