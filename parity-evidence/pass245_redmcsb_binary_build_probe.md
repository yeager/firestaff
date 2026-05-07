# Pass245: ReDMCSB binary build/run probe on N2

Daniel asked why we were not compiling/running ReDMCSB as a binary. This pass audited `ReDMCSB_WIP20210206` and exercised the most useful N2-local route without using <private-host> or committing original/build artifacts.

## Findings

- ReDMCSB is not a native Linux host-port of Dungeon Master/CSB. It is a Windows-orchestrated multi-toolchain build harness that produces original target executables inside emulators.
- For DM1 PC 3.4 English (`I34E`), the supported target is 16-bit DOS output built with the bundled Turbo C++ 1.01, Turbo Assembler 2.0, Turbo Link, and LZEXE under DOSBox.
- Original `GRAPHICS.DAT` / `DUNGEON.DAT` are not needed to compile `DM.EXE` or `FIRES`; they are needed to run the game path through the DOS launcher/runtime.
- The useful runtime binary for the current DM1 V1 movement/viewport/debugger blocker is `I34E/FIRES`, not a host executable. It can be built and run under DOSBox, but it does not replace the need for DOS/debugger instrumentation to bind movement/viewport state to CS:IP/runtime data.

## N2 build probe

Scratch path only: `<firestaff-data>/redmcsb-n2-build-probe/`.

- Built `I34E/DM.EXE` (EXEID 72) under native N2 `dosbox` + `xvfb-run`.
  - Output: `HARDDISK/BUILD/I34E/DM.EXE`, size 11471.
  - SHA256: `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`.
  - Compared identical to `Reference/Original/I34E/DM.EXE`.
- Built `I34E/FIRES` (EXEID 74) under native N2 `dosbox` + `xvfb-run`.
  - Output: `HARDDISK/BUILD/I34E/FIRES`, size 94841.
  - SHA256: `3e59c1a8d4dec64c3cac1fd06a064077754cd012bef8d627cca969cad32311d0`.
  - Compared identical to `Reference/ReDMCSB/I34E/FIRES`, and different from `Reference/Original/I34E/FIRES` as documented by upstream ReDMCSB.

## Runtime probe

Scratch path only: `<firestaff-data>/redmcsb-n2-build-probe/runtime-i34e-built-fires/`.

- Copied the local original PC34 game folder to scratch and replaced only `FIRES` with the built ReDMCSB `I34E/FIRES`.
- `DM.EXE` launched under DOSBox and remained running until the 12 second guard timeout (`rc=124`), with no DOSBox stderr crash.
- Direct `FIRES` invocation exited immediately (`rc=0`), which is consistent with `FIRES` being the engine loaded by the launcher path rather than a useful standalone harness.

## Blocker status

Removed: “we have not tried compiling ReDMCSB on N2” is no longer true.

Remaining: this does not by itself solve DM1 V1 movement/viewport runtime binding. The built artifact is still a DOS executable that must be driven through DOSBox/DOSBox-X/QEMU/debugger capture; it does not expose a native host API or bypass the CS:IP/state-symbol work.
