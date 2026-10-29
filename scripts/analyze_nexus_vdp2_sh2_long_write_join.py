#!/usr/bin/env python3
"""Join SH-2 longword stores to the ordered VDP2 halfword write trace.

This read-only analyzer verifies the bus-width conversion for a selected
authentic Ymir capture. A match establishes writer attribution and byte order;
it does not assign tile, layer, or UI semantics to the written values.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def read_fields(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            result[key] = value
    return result


def csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def require_rows(rows: list[dict[str, str]], fields: dict[str, str], label: str) -> None:
    if fields.get("bundle_complete") != "1" or fields.get("trace_file_complete") != "1":
        raise ValueError(f"{label} trace receipt is not complete")
    if len(rows) != int(fields["trace_rows"]):
        raise ValueError(f"{label} trace row count does not match its receipt")


def inspect_nbg1_map(
    bundle: Path,
    frame: int,
    source_addresses: list[tuple[int, int]],
) -> None:
    """Bind longword destinations to visible NBG1 map cells for this witness."""
    registers = (bundle / "vdp2-registers.bin").read_bytes()
    vram = (bundle / "vdp2-vram.bin").read_bytes()
    register_receipt = read_fields(bundle / "vdp-register-snapshot.txt")
    if register_receipt.get("register_byte_order") != "big_endian":
        raise ValueError("NBG1 inspection requires the captured big-endian register image")
    if int(register_receipt.get("vdp1_frame_index", "-1")) != frame:
        raise ValueError("NBG1 register snapshot does not match the selected VDP2 frame")
    if len(registers) != 0x200 or len(vram) != 0x80000:
        raise ValueError("NBG1 inspection requires complete VDP2 registers and VRAM")

    def u16(offset: int) -> int:
        return int.from_bytes(registers[offset:offset + 2], "big")

    bgon = u16(0x20)
    chctla = u16(0x28)
    pncn1 = u16(0x32)
    plsz = u16(0x3A)
    mpofn = u16(0x3C)
    map_a, map_b = u16(0x44), u16(0x46)
    x_scroll = u16(0x80) & 0x7FF
    y_scroll = u16(0x88) & 0x7FF

    if not bgon & 0x0002:
        raise ValueError("NBG1 is disabled in the selected VDP2 frame")
    if chctla & 0x0200:
        raise ValueError("NBG1 is bitmap mode, not a character name table")
    character_size = (chctla >> 8) & 1
    color_code = (chctla >> 12) & 3
    if character_size != 0 or color_code != 0:
        raise ValueError("NBG1 character size or colour mode is outside this analyzer's case")
    if (pncn1 >> 15) & 1:
        raise ValueError("NBG1 uses an unsupported one-word PND")
    plane_size = (plsz >> 2) & 3
    map_offset = (mpofn >> 4) & 7
    map_numbers = (
        map_a & 0x3F, (map_a >> 8) & 0x3F,
        map_b & 0x3F, (map_b >> 8) & 0x3F,
    )
    if plane_size != 0 or map_offset != 0 or len(set(map_numbers)) != 1:
        raise ValueError("NBG1 map layout is outside this analyzer's bounded case")

    pnd_size = 0
    psshift = 13 - pnd_size - (character_size << 1)
    map_base_word = ((map_offset << 6) + map_numbers[0]) << psshift
    map_base_byte = (map_base_word * 2) & 0x7FFFF
    visible_addresses = {
        (map_base_byte + (((((cell_y * 8 + y_scroll) >> 3) & 0x3F) * 64 +
                           (((cell_x * 8 + x_scroll) >> 3) & 0x3F)) * 4)) & 0x7FFFF
        for cell_y in range(28)
        for cell_x in range(40)
    }

    decoded: list[tuple[int, int, int, int, int]] = []
    for address, value in source_addresses:
        offset = (address - map_base_byte) & 0x7FFFF
        if offset % 4:
            raise ValueError(f"SH-2 longword at {address:#x} is not PND-aligned")
        cell_index = offset // 4
        if cell_index >= 64 * 64:
            raise ValueError(f"SH-2 longword at {address:#x} lies outside NBG1 map plane")
        attribute = int.from_bytes(vram[address:address + 2], "big")
        char_number = int.from_bytes(vram[address + 2:address + 4], "big") & 0x7FFF
        if ((attribute << 16) | char_number) != value:
            raise ValueError(f"NBG1 VRAM snapshot disagrees with trace at {address:#x}")
        decoded.append((address, cell_index % 64, cell_index // 64, attribute, char_number))

    if not decoded or len({row[0] for row in decoded}) != len(decoded):
        raise ValueError("NBG1 writer rows are empty or duplicate a PND address")
    visible_count = sum(row[0] in visible_addresses for row in decoded)
    if visible_count != len(decoded):
        raise ValueError("one or more SH-2 PND writes lie outside the selected visible map")
    xs = [row[1] for row in decoded]
    ys = [row[2] for row in decoded]
    cells = {(row[1], row[2]) for row in decoded}
    width = max(xs) - min(xs) + 1
    height = max(ys) - min(ys) + 1
    if len(cells) != width * height:
        raise ValueError("NBG1 writer cells do not form a complete rectangle")

    pnd_values = sorted({(row[3], row[4]) for row in decoded})
    print(f"nbg1_enabled=1")
    print(f"nbg1_mode=character")
    print(f"nbg1_color_code={color_code}")
    print(f"nbg1_pnd_size=two_word")
    print(f"nbg1_plane_size={plane_size}")
    print(f"nbg1_map_offset={map_offset}")
    print(f"nbg1_map_numbers={','.join(f'0x{x:02x}' for x in map_numbers)}")
    print(f"nbg1_name_table_base=0x{map_base_byte:05x}")
    print(f"nbg1_scroll_pixels={x_scroll},{y_scroll}")
    print(f"nbg1_visible_pnd_addresses={len(visible_addresses)}")
    print(f"writer_visible_nbg1_pnds={visible_count}/{len(decoded)}")
    print(f"writer_nbg1_cell_matrix={width}x{height}")
    print(f"writer_nbg1_cell_bounds=x{min(xs)}..{max(xs)},y{min(ys)}..{max(ys)}")
    print("writer_nbg1_pnd_values=" + ",".join(
        f"0x{attribute:04x}/0x{char_number:04x}"
        for attribute, char_number in pnd_values
    ))
    if len(pnd_values) == 1:
        char_number = pnd_values[0][1]
        char_address = (char_number << 5) & 0x7FFFF
        char_data = vram[char_address:char_address + 32]
        if len(char_data) == 32:
            print(f"writer_nbg1_char_data_address=0x{char_address:05x}")
            print(f"writer_nbg1_char_data_bytes=32")
            print(f"writer_nbg1_char_data_nonzero_bytes={sum(byte != 0 for byte in char_data)}")
    print("nbg1_semantic_admission=blocked")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir trace-bundle directory")
    parser.add_argument("--frame", type=int, required=True, help="VDP1 frame to compare")
    parser.add_argument(
        "--pc", type=lambda value: int(value, 0),
        help="restrict SH-2 rows to one writer PC (for example 0x06023640)",
    )
    parser.add_argument(
        "--cpu", choices=("master", "slave", "both"), default="master",
        help="select a SH-2 trace source (default: master)",
    )
    parser.add_argument(
        "--bus-vram-end", type=lambda value: int(value, 0), default=0x05E80000,
        help="exclusive SH-2 bus end for VDP2 VRAM (default: 0x05e80000)",
    )
    parser.add_argument(
        "--inspect-nbg1-map", action="store_true",
        help="check selected SH-2 longword writes against the visible NBG1 name table",
    )
    args = parser.parse_args()

    try:
        vdp2_receipt = read_fields(args.bundle / "vdp2-memory-write-bundle.txt")
        sh2_receipt = read_fields(args.bundle / "sh2-vdp2-write-trace-bundle.txt")
        vdp2_rows = csv_rows(args.bundle / "vdp2-memory-write-trace.csv")
        sh2_rows = csv_rows(args.bundle / "sh2-vdp2-write-trace.csv")
        require_rows(vdp2_rows, vdp2_receipt, "VDP2")
        require_rows(sh2_rows, sh2_receipt, "SH-2")
        if int(vdp2_receipt.get("trace_dropped_total", "-1")) != 0:
            raise ValueError("VDP2 trace reports dropped rows")
        if int(sh2_receipt.get("master_dropped", "-1")) != 0 or int(
            sh2_receipt.get("slave_dropped", "-1")
        ) != 0:
            raise ValueError("SH-2 trace reports dropped rows")
        sh2_base = int(sh2_receipt["address_begin"], 0)
        vdp2_range_end = min(
            int(sh2_receipt["address_end_exclusive"], 0), args.bus_vram_end
        )
        target_vdp2 = [
            (int(row["address"], 0), int(row["width"]), int(row["value"], 0))
            for row in vdp2_rows
            if int(row["vdp1_frame"]) == args.frame and row["memory"] == "VRAM"
        ]
        expanded: list[tuple[int, int, int, int]] = []
        source_addresses: list[tuple[int, int]] = []
        source_rows = 0
        for row in sh2_rows:
            if int(row["vdp1_frame"]) != args.frame:
                continue
            if args.cpu != "both" and row.get("sh2") != args.cpu:
                continue
            pc = int(row["pc"], 0)
            if args.pc is not None and pc != args.pc:
                continue
            address = int(row["address"], 0)
            width = int(row["width"])
            value = int(row["value"], 0)
            if not sh2_base <= address < vdp2_range_end:
                continue
            if width == 2:
                expanded.append((address - sh2_base, width, value & 0xFFFF, pc))
            elif width == 4:
                source_addresses.append((address - sh2_base, value))
                expanded.append((address - sh2_base, 2, (value >> 16) & 0xFFFF, pc))
                expanded.append((address - sh2_base + 2, 2, value & 0xFFFF, pc))
            else:
                raise ValueError(f"unsupported SH-2 write width {width} at {address:#x}")
            source_rows += 1
    except (OSError, KeyError, ValueError) as error:
        parser.error(str(error))

    if not target_vdp2 or not expanded:
        parser.error("selected frame or writer has no VDP2 VRAM writes")
    if any(width != 2 for _address, width, _value in target_vdp2):
        parser.error("selected VDP2 trace contains a non-halfword VRAM event")

    expected = [(address, value) for address, _width, value, _pc in expanded]
    observed = [(address, value) for address, _width, value in target_vdp2]
    exact_ordered_match = expected == observed
    matched = 0
    cursor = 0
    for event in expected:
        while cursor < len(observed) and observed[cursor] != event:
            cursor += 1
        if cursor == len(observed):
            break
        matched += 1
        cursor += 1
    ordered_subsequence = matched == len(expected)

    print(f"frame={args.frame}")
    print(f"source_cpu={args.cpu}")
    print(f"source_pc={f'{args.pc:#010x}' if args.pc is not None else 'all'}")
    print(f"sh2_source_rows={source_rows}")
    print(f"sh2_expanded_halfword_events={len(expected)}")
    print(f"vdp2_vram_halfword_events={len(observed)}")
    print(f"ordered_subsequence_matches={matched}/{len(expected)}")
    print(f"exact_ordered_trace_match={int(exact_ordered_match)}")
    print("interpretation=writer_and_bus_value_join_only")
    if args.inspect_nbg1_map:
        if len(source_addresses) != source_rows:
            parser.error("NBG1 inspection requires every selected SH-2 source row to be width four")
        try:
            inspect_nbg1_map(args.bundle, args.frame, source_addresses)
        except (OSError, ValueError) as error:
            parser.error(str(error))
    return 0 if ordered_subsequence else 1


if __name__ == "__main__":
    raise SystemExit(main())
