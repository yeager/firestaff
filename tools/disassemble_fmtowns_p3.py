#!/usr/bin/env python3
"""Disassemble a bounded address range from a Phar Lap P3 load image.

This is a local development aid. It reads the selected original executable
in place and emits text only; it never extracts or writes game media.
P3 field offsets match dm2_v1_boot_parse_fmtowns_p3 in dm2_v1_boot.c.
Linear mode decodes the requested span as-is. Control-flow mode follows only
direct calls and branches within that same bounded virtual-address window;
indirect targets and edges outside the window are reported, not followed.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def format_instruction(insn) -> str:
    raw = " ".join(f"{byte:02x}" for byte in insn.bytes)
    return f"{insn.address:08x}: {raw:<29} {insn.mnemonic:<9} {insn.op_str}"


def disassemble_control_flow(disassembler, code: bytes, base: int,
                             start: int, instruction_limit: int) -> None:
    from capstone import CS_GRP_CALL, CS_GRP_IRET, CS_GRP_JUMP, CS_GRP_RET
    from capstone.x86_const import X86_OP_IMM

    end = base + len(code)
    pending = [start]
    queued = {start}
    block_starts = set()
    decoded_addresses = set()
    decoded_count = 0

    def enqueue(address: int) -> None:
        if base <= address < end and address not in queued:
            queued.add(address)
            pending.append(address)

    def follow_edge(kind: str, address: int) -> None:
        if base <= address < end:
            print(f"                  ; follow {kind} -> 0x{address:x}")
            enqueue(address)
        else:
            print(f"                  ; {kind} -> 0x{address:x} outside bounded range")

    while pending and decoded_count < instruction_limit:
        block = pending.pop()
        if block in block_starts:
            continue
        block_starts.add(block)
        cursor = block

        while base <= cursor < end and decoded_count < instruction_limit:
            if cursor in decoded_addresses and cursor != block:
                break
            offset = cursor - base
            insn = next(disassembler.disasm(code[offset:], cursor, count=1), None)
            if insn is None:
                print(f"{cursor:08x}: <undecodable byte; block stopped>")
                break

            if insn.address not in decoded_addresses:
                print(format_instruction(insn))
                decoded_addresses.add(insn.address)
            decoded_count += 1
            next_address = insn.address + insn.size
            groups = set(insn.groups)

            if CS_GRP_CALL in groups:
                if (insn.mnemonic == "call" and insn.operands and
                        insn.operands[0].type == X86_OP_IMM):
                    follow_edge("call", int(insn.operands[0].imm))
                else:
                    print("                  ; indirect or far call target not followed")
                cursor = next_address
                continue

            if CS_GRP_JUMP in groups:
                if (insn.mnemonic != "ljmp" and insn.operands and
                        insn.operands[0].type == X86_OP_IMM):
                    follow_edge("branch", int(insn.operands[0].imm))
                else:
                    print("                  ; indirect or far branch target not followed")
                if insn.mnemonic not in {"jmp", "ljmp"}:
                    print(f"                  ; conditional fallthrough -> 0x{next_address:x}")
                    enqueue(next_address)
                break

            if CS_GRP_RET in groups or CS_GRP_IRET in groups:
                break
            cursor = next_address

    if pending:
        print(f"<control-flow traversal stopped at {instruction_limit} instructions>")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("program", type=Path, help="P3 executable path")
    target = parser.add_mutually_exclusive_group()
    target.add_argument("--address", type=lambda value: int(value, 0))
    target.add_argument("--symbol", help="Resolve an exact name from the embedded SYM1 table")
    parser.add_argument("--length", type=lambda value: int(value, 0), default=0x80)
    parser.add_argument("--sha256", help="Require this exact executable SHA-256")
    parser.add_argument("--flow", action="store_true",
                        help="Follow direct control-flow edges inside the bounded address range")
    parser.add_argument("--max-instructions", type=int, default=4096,
                        help="Maximum decoded instructions in --flow mode (default: 4096)")
    args = parser.parse_args()

    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        parser.error("Python capstone is required for disassembly")

    data = args.program.read_bytes()
    if len(data) < 0x80 or data[:2] != b"P3":
        parser.error("input is not a complete P3 executable")
    digest = hashlib.sha256(data).hexdigest()
    if args.sha256 and digest.lower() != args.sha256.lower():
        parser.error(f"SHA-256 mismatch: got {digest}")

    level = u16(data, 2)
    header_size = u16(data, 4)
    file_size = u32(data, 6)
    runtime_offset = u32(data, 0x0C)
    runtime_size = u32(data, 0x10)
    relocation_offset = u32(data, 0x14)
    relocation_size = u32(data, 0x18)
    image_offset = u32(data, 0x26)
    image_size = u32(data, 0x2A)
    symbol_offset = u32(data, 0x2E)
    symbol_size = u32(data, 0x32)
    entry = u32(data, 0x68)
    memory_size = u32(data, 0x74)
    if (level != 1 or header_size < 0x80 or file_size == 0 or
            file_size > len(data) or runtime_offset < header_size or
            runtime_offset > len(data) or runtime_size > len(data) - runtime_offset or
            image_offset < header_size or image_offset > len(data) or
            image_size > len(data) - image_offset or
            image_offset + image_size > file_size or
            relocation_offset > len(data) or
            relocation_size > len(data) - relocation_offset or
            (relocation_size and relocation_offset < header_size) or
            (symbol_offset and (symbol_offset < header_size or
                                symbol_offset > len(data) or
                                symbol_size > len(data) - symbol_offset)) or
            memory_size < image_size or entry >= memory_size):
        parser.error("P3 header bounds are inconsistent")

    address = entry
    if args.address is not None:
        address = args.address
    elif args.symbol is not None:
        if not symbol_offset or not symbol_size:
            parser.error("input has no embedded SYM1 symbol table")
        symbol_table = data[symbol_offset:symbol_offset + symbol_size]
        if len(symbol_table) != symbol_size or symbol_table[:4] != b"SYM1" or symbol_size < 0x22:
            parser.error("embedded SYM1 table is missing or malformed")
        cursor = 0x22
        matches = []
        while cursor < len(symbol_table):
            name_size = symbol_table[cursor]
            cursor += 1
            if (name_size == 0 or name_size > 127 or
                    name_size > len(symbol_table) - cursor or
                    len(symbol_table) - cursor - name_size < 6):
                parser.error("embedded SYM1 record is malformed")
            name = symbol_table[cursor:cursor + name_size]
            value = u32(symbol_table, cursor + name_size)
            try:
                decoded_name = name.decode("ascii")
            except UnicodeDecodeError:
                parser.error("embedded SYM1 contains a non-ASCII symbol name")
            if decoded_name == args.symbol:
                matches.append(value)
            cursor += name_size + 6
        if len(matches) != 1:
            reason = "not found" if not matches else "ambiguous"
            parser.error(f"symbol {args.symbol!r} is {reason} in SYM1")
        address = matches[0]
    if (args.length <= 0 or address < 0 or address > image_size or
            args.length > image_size - address):
        parser.error("requested address range is outside the P3 load image")
    if args.max_instructions <= 0:
        parser.error("--max-instructions must be greater than zero")

    code = data[image_offset + address:image_offset + address + args.length]
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True
    print(f"file={args.program}")
    print(f"sha256={digest}")
    print(f"level={level} header_size=0x{header_size:x} declared_file_size=0x{file_size:x} image_offset=0x{image_offset:x} image_size=0x{image_size:x}")
    print(f"entry=0x{entry:x} symbols=0x{symbol_offset:x}+0x{symbol_size:x}")
    if args.symbol is not None:
        print(f"symbol={args.symbol} address=0x{address:x}")
    mode = "control-flow" if args.flow else "linear"
    print(f"disassembly={mode} 0x{address:x}..0x{address + args.length:x} (P3 virtual addresses)")
    if args.flow:
        disassemble_control_flow(disassembler, code, address, address,
                                 args.max_instructions)
    else:
        for insn in disassembler.disasm(code, address):
            print(format_instruction(insn))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
