# Pass162 C080 debugger/address gate

Classification: `blocked/gdb-missing-for-stock-symbol-gate`
First missing gate: `native gdb symbol-binding sanity check unavailable on this host`

## Local tools

- dosbox: `/opt/homebrew/bin/dosbox`
- dosbox-debug: `/opt/homebrew/bin/dosbox-debug`
- dosbox-x: `/opt/homebrew/bin/dosbox-x`
- gdb: `missing`
- lldb: `/usr/bin/lldb`
- file: `/usr/bin/file`
- python3: `/usr/bin/python3`

## Commands

- gdb sanity check: `gdb --batch -x /Users/bosse/Documents/Firestaff/parity-evidence/verification/pass162_c080_queue_trace/pass162_dm_exe_symbol_gate.gdb`
- DOSBox-X start point: `dosbox-x -conf /Users/bosse/Documents/Firestaff/parity-evidence/verification/pass162_c080_queue_trace/dosbox-x-pass162-runtime-gate.conf -break-start`

## Breakpoint order

1. **mouse translation / queue write**: break on F0359 entry and after L1109_i_Command is assigned/written; expect after x=111,y=82 left click, P0725/P0726 are 111/82, L1109_i_Command == 80, G0432_as_CommandQueue[last].Command == 80 with X=111,Y=82
2. **queue dequeue**: break when L1160/L1161/L1162 are loaded from G0432_as_CommandQueue; expect L1160_i_Command == 80 and L1161/L1162 == 111/82
3. **C080 dispatch / viewport normalization**: break on F0377 entry and after PC coordinate normalization; expect F0377 is entered; normalized point remains inside C05 wall ornament/portrait hit zone for the source-locked front wall
4. **front-wall sensor hit-state**: break on F0372 and F0280; log G0306/G0307/G0308, forward square, wall face, sensor type/data; expect pose map0 x=1 y=3 dir=South touches front square x=1 y=4 opposite face and reaches F0280(sensorData=10)

## Non-claims

- does not prove stock original binary reached C080/F0377/F0280
- does not classify mouse translation vs queue dequeue vs C080 dispatch vs F0280 until a DOS real-mode/source-symbol bridge or address map exists
- does not do coordinate guessing
