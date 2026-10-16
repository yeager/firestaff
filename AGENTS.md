# Firestaff contributor instructions

Firestaff is a native C11/SDL3 engine for DM1, CSB, DM2, Nexus and Theron's
Quest. Prioritize DM1, then CSB, DM2, Nexus and Theron. Savegame work is
currently deferred. Consult [the project guide](docs/PROJECT_GUIDE.md) only
when its architecture or reference material is relevant to the task.

## Required boundaries

- Use authentic game data whenever available. Synthetic fixtures may help
  obtain authentic evidence, but must not substitute for missing parity.
  V2.2 may use synthetic artwork.
- No emulator or BIOS dependency at runtime. Emulators and disassembly may
  be used for reference and verification.
- Read game archives into bounded memory, without extracting or caching
  game data to disk at runtime. Discover editions by hashes and block launch
  when required data is missing.
- Keep secrets, credentials, user game media and BIOS images out of Git and
  external uploads. Run Gitleaks with redacted output before commit/push.
- Never use /tmp. Use a task-specific build directory or /dev/shm. Limit local
  builds to -j1 and tests to -j2. Preserve unrelated work and processes.
- Use the dedicated `trv2` build host for full builds, sanitizer builds,
  large test selections, disassembly, and other memory-intensive work. Keep
  local execution to short checks that require the locally installed original
  game media or an interactive display; never record connection details or
  credentials in the repository.
- Repository documentation/comments must be English. User-facing strings
  belong in the po/ localization workflow, including Original mode.
- Never create a release, tag or dispatch a release workflow without an
  explicit user release request. Push approval alone is not release approval.

## Source fidelity

Consult relevant original/reference code before changing game logic.
For DM1/CSB, ReDMCSB is available under
`reference/redmcsb-20210206/Toolchains/Common/Source/`.
Archive: http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z
Cite relevant source file, function and line numbers in implementation
comments. Do not assume one edition's behavior applies to all platforms.
Follow surrounding C11 conventions; public headers belong in include/.

## Verification and publication

Complete the requested implementation and fix failures it causes. Run the
smallest relevant checks, expanding coverage when shared runtime paths
change. Distinguish source comparisons, original-media tests and emulator
captures. Missing-media skips and narrow passing tests do not establish
full game/platform parity.

Commit verified changes in coherent batches. The main session pushes verified
batches to GitHub main, preferably without branches; subagents must not push.
Confirm the preceding GitHub Actions run has completed before another push.
Inspect the resulting run after pushing; avoid unnecessary CI dispatches.
While Actions runs, continue useful local work without pushing.

Keep TODO*.md for open work and DONE*.md for verified completed work; update
the relevant files when completing a job. Update RELEASE_NOTES.md for
release-facing changes without creating a release. Public status must
distinguish proven functionality from gaps. Public screenshots must be real
runtime captures, not generated or mocked scenes.

When a parity issue is blocked, document the evidence and remaining gap,
then continue another in-scope issue without lowering the parity target.

For an explicitly requested release, CMakeLists.txt owns the version.
CMake generates firestaff_version.h from include/firestaff_version.h.in;
never commit that generated header. Keep src/shared/changelog_m12.c in sync.
After changing the CMake version, always regenerate and commit the source
SPDX SBOM with `python3 tools/generate_spdx_sbom.py`. Run
`python3 tests/test_generate_spdx_sbom.py` before pushing the release
candidate; the release-notes-policy CI job requires the committed SBOM to
match the current version.
For every release, compare the candidate with the most recent published tag
using `git log --no-merges --oneline <previous-tag>..HEAD` and
`git diff --stat <previous-tag>..HEAD`. Write that version's release-notes
section from this interval: cover every user-facing and developer change
since the previous release, grouping related commits where useful and
including work from other contributors. State important remaining
limitations. Do not carry changes already
released under an earlier tag into the new section. Check the published
GitHub Release text against the same interval before considering it complete.
