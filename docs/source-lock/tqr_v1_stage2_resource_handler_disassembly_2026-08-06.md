# Theron's Quest — stage-2 resource handler receipt

## Status

The authenticated US and JP Track 19 projections contain the same 162-byte
HuC6280 handler at CPU address `$4C3F`. In both images it is file offset
`0x1F443F` and has FNV-1a-32 `0x46360D97`. The focused
`test_theron_v1_huc6280_disassembly` test verifies the identity, range and
receipt for both variants.

This is a generic resource-command handler. It is not yet proof that any one
of the seven Track 02 level blocks is the command's input or output.

## Proven contract

The handler's source order is:

1. `L4EC9` resolves the selected record and fills the working source/length
   words at `L4EC3/L4EC4` and `L4EC7/L4EC8`.
2. `L4C4C` indexes the four-byte table at `($02)` with
   `L4EC1 * 4`. It preserves the raw four bytes at `$3447-$344A` and adds
   `$3008` before publishing the four HuC6280 MPR values at
   `$3B7E-$3B81`.
3. `L4C6D` saves the active MPRs and `L4B00` establishes the contiguous
   `$6000-$DFFF` windows used by the byte/bitstream path.
4. The handler initializes the source/output work window at `$6000`, calls
   the source-owned helpers, and reaches the verified variable-bit routine
   at bank `$1F` through that helper chain. The produced byte count is
   returned in `$3B7C/$3B7D`.
5. The result is handed to `L36FC` with source `$3002/$3003 = $6000`,
   destination `$3004/$3005 = L4EC5/L4EC6`, and byte count
   `$3006/$3007 = $3B7C/$3B7D`.

The static code therefore proves the bank-table and destination-register
contract, including the fact that the destination pointer is supplied by the
command record rather than invented by the host.

## What remains open

The current receipt does not prove which `$02` table row, `L4EC1` command
index, source record, or destination value is used for a Track 02 level. The
post-startup capture still lacks the game-owned CD read and executing-PC join
that would bind those operands to one of the real seven level blocks. No map,
object, tile, bitmap or palette semantics are promoted from this handler alone.

Source: `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm`
around `L4C3F-L4CE0`, plus the byte-identical JP/US retail receipts.
