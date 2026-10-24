# Cross-game parity test sweep

`tools/parity_test_sweep.py` discovers and runs the CTest gates associated with
DM1, DM2, CSB, Theron, and Nexus. It uses explicit CTest labels first and
game-prefixed test names for older tests without labels. Cross-cutting tests
are omitted unless `--include-shared` is requested, so shared checks are not
silently credited to every game.

For runtime startup diagnostics, `tools/parity_debug_boot_sweep.py` runs the
real-data `--debug --boot-probe` route across the supported game/platform
matrix. It records the selected asset MD5 and startup receipt, distinguishes
startup blockers from missing media, and returns failure if any selected
route does not produce a boot receipt. Example on TRV2:

```sh
python3 tools/parity_debug_boot_sweep.py \
  --app build-debug/firestaff \
  --data-dir ~/.firestaff/data \
  --output parity-debug-boot-sweep.json
```

The debug sweep is useful for locating real-data and launch gaps. A boot
receipt does not prove gameplay or presentation parity, and a blocked startup
receipt remains a blocker even if the process exits successfully.

Replace `build-debug/firestaff` with the path to your Debug executable. The app path must be executable.

Configure and build Firestaff before running the sweep. Each game's test set
runs as a separate loop iteration, making missing test inventory and failures
visible per game:

```sh
python3 tools/parity_test_sweep.py --build-dir build --game all \
  --results build/parity-test-sweep.json
```

List the selected tests without executing them:

```sh
python3 tools/parity_test_sweep.py --build-dir build --game nexus --list
```

Inventory platform identifiers explicitly present in test names or CTest
labels, grouped by game:

```sh
python3 tools/parity_test_sweep.py --build-dir build --game all --list-platforms
```

An absent identifier means the current test inventory does not identify that
platform. It is a test-discovery gap, not proof that the game itself lacks
support.

Exercise the per-game execution loop on a small sample before a full sweep.
The report labels these results `partial-passed` and records both the full
discovered count and the selected count:

```sh
python3 tools/parity_test_sweep.py --build-dir build --game all \
  --limit-per-game 1 --parallel 1
```

Filter a game's tests by a platform token present in a CTest label or test
name (for example `saturn`, `fmtowns`, or `amiga`):

```sh
python3 tools/parity_test_sweep.py --build-dir build --game nexus \
  --platform saturn --list
```

The tool reports CTest discovery and execution only. A passing test proves
only its own assertion; it does not establish gameplay, visual, audio, or
cross-platform parity. Platform filters may return no tests when CTest entries
do not identify that platform. Such gaps should be fixed by adding correctly
scoped tests and labels, not by broadening attribution heuristics. Real-media
tests still require the authentic files and data directory expected by their
own CTest properties. CTest skip-safe entries are reported as
`passed-with-skips`, separately from a fully executed passing selection.
