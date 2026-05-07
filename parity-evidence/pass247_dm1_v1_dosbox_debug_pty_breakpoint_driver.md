# Pass247 — DOSBox debugger PTY breakpoint driver

Status: `PASS_BREAKPOINT_COMMANDS_ACCEPTED`.

Method: run `/usr/bin/dosbox-debug` inside tmux with `TERM=vt100`, use autoexec `DEBUG COMMAND.COM` to enter the debugger loop, then send debugger commands with `tmux send-keys ... Enter`.

Accepted commands proved by `BPLIST`:

- `BP 22AF:06E9`
- `BP 1EA4:010D`
- `BP 1EA4:01AA`
- `BP 1859:0516`
- `BP 2AFF:110E`

Reusable tool: `tools/pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver.py`.

Bounded evidence (verification tree, text-only):

- `parity-evidence/verification/pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver/manifest.json`
- `parity-evidence/verification/pass247_dm1_v1_dosbox_debug_pty_breakpoint_driver/dosbox_debug_pty_breakpoints.clean.txt`

Important terminal finding: `TERM=xterm`, `TERM=screen`, and `TERM=linux` could show the debugger UI, but this DOSBox build did not reliably parse Enter for command submission. `TERM=vt100` made `HELP`, `BP`, and `BPLIST` reach the debugger command parser.
