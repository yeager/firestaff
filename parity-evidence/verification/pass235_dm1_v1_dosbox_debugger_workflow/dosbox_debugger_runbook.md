# Pass235 manual DOSBox debugger sequence

Automated entry capture:
1. Regenerate `FIRES.EXENEW` in a temp directory from N2-local `DungeonMasterPC34/FIRES` using Wine `unlzexe.exe`.
2. Temp-copy that unpacked image as `FIRES.EXE` only because DOSBox executes `.EXE` names reliably.
3. Start `TERM=xterm xvfb-run -a dosbox-debug -conf dosbox-debug.conf -exit` with autoexec command `DEBUG FIRES.EXE`.
4. The debugger stops at the program entry. Record the first code line `CS:0000 BADA26...` and register view `CS=....`.

Keystroke-level interactive continuation when a human display is available:
1. Run the generated config without `timeout`.
2. At debugger prompt `->`, type `CPU` Enter and confirm the entry `CS:IP`.
3. Type `LOGS 200` Enter or `LOG 200` Enter to write a bounded CPU log, then press `F5` to run.
4. Drive the game to the target command/movement/viewport scenario in the DOSBox window.
5. Press `Alt+Pause` to re-enter the debugger; type `CPU` Enter and `LOGS 200` Enter.
6. Do not promote F0380/F0365/F0366/F0267/F0128/F0097 until a FIRES.MAP/public symbol table or validated disassembly-address bridge supplies exact breakpoints/watchpoints.

Current hard blocker:
- DOSBox debugger accepts numeric `BP segment:offset` and `BPM segment:offset`; it does not understand ReDMCSB C function names.
- We have actual loader-entry runtime CS:IP, but no reproducible N2-local map from ReDMCSB source seams/globals to FIRES.EXENEW runtime offsets.
