#!/usr/bin/env python3
"""Inspect an authentic Theron JP Mednafen 1.32.1 PCE Fast save state."""

from __future__ import annotations

import argparse
import gzip
import hashlib
import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path


MEDNAFEN_STATE_VERSION = 0x00103201
MAX_COMPRESSED_BYTES = 16 * 1024 * 1024
MAX_INFLATED_BYTES = 16 * 1024 * 1024
MAX_PREVIEW_PIXELS = 2_000_000
SECTION_HEADER_BYTES = 36
SECTION_NAME_BYTES = 32
RAW_SECTOR_BYTES = 2352
MODE1_USER_OFFSET = 16
MODE1_USER_BYTES = 2048
JP_TRACK02_BYTES = 8_102_640
JP_TRACK02_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
JP_AKUTUBA_SOURCE_SPAN = (0x9A11D, 0x9A609)
JP_AKUTUBA_MAP_SPANS = (
    (0x9A303, 48),
    (0x9A333, 247),
    (0x9A42A, 323),
    (0x9A56D, 156),
)
CDRAM_BANK_BASE = 0x68
CDRAM_BYTES = 0x40000
CDRAM_VARIABLE = "ROMSpace + 0x68 * 8192"


class StateFormatError(ValueError):
    """The input is not a complete supported Mednafen PCE Fast state."""


@dataclass(frozen=True)
class StateVariable:
    offset: int
    size: int


@dataclass(frozen=True)
class StateSection:
    offset: int
    size: int
    variables: dict[str, StateVariable]


@dataclass(frozen=True)
class PCEFastState:
    payload: bytes
    version: int
    total_length: int
    preview_width: int
    preview_height: int
    preview_offset: int
    big_endian_variables: bool
    sections: dict[str, StateSection]


def _u32(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise StateFormatError("truncated 32-bit field")
    return struct.unpack_from("<I", data, offset)[0]


def _decompress_state(path: Path) -> tuple[bytes, bytes]:
    compressed = path.read_bytes()
    if not compressed or len(compressed) > MAX_COMPRESSED_BYTES:
        raise StateFormatError("compressed state is empty or exceeds the input limit")
    inflater = zlib.decompressobj(wbits=31)
    try:
        payload = inflater.decompress(compressed, MAX_INFLATED_BYTES + 1)
        if len(payload) > MAX_INFLATED_BYTES or inflater.unconsumed_tail:
            raise StateFormatError("inflated state exceeds the size limit")
        payload += inflater.flush()
    except (zlib.error, OSError) as exc:
        raise StateFormatError(f"invalid gzip state: {exc}") from exc
    if not inflater.eof or inflater.unused_data:
        raise StateFormatError("state must contain exactly one complete gzip member")
    if len(payload) > MAX_INFLATED_BYTES:
        raise StateFormatError("inflated state exceeds the size limit")
    return compressed, payload


def _parse_section_variables(
    payload: bytes, name: str, start: int, size: int
) -> dict[str, StateVariable]:
    end = start + size
    cursor = start
    variables: dict[str, StateVariable] = {}
    while cursor < end:
        name_size = payload[cursor]
        cursor += 1
        if name_size == 0 or cursor + name_size + 4 > end:
            raise StateFormatError(f"invalid variable header in section {name}")
        raw_name = payload[cursor : cursor + name_size]
        cursor += name_size
        try:
            variable_name = raw_name.decode("ascii")
        except UnicodeDecodeError as exc:
            raise StateFormatError(f"non-ASCII variable in section {name}") from exc
        variable_size = _u32(payload, cursor)
        cursor += 4
        variable_end = cursor + variable_size
        if variable_end > end:
            raise StateFormatError(f"variable exceeds section {name}")
        if variable_name in variables:
            raise StateFormatError(
                f"duplicate variable {variable_name!r} in section {name}"
            )
        variables[variable_name] = StateVariable(cursor, variable_size)
        cursor = variable_end
    if cursor != end:
        raise StateFormatError(f"section {name} does not end on a variable boundary")
    return variables


def parse_state_payload(payload: bytes) -> PCEFastState:
    """Parse the named-section save format used by Mednafen 1.32.1."""
    if len(payload) < 32 or payload[:8] != b"MDFNSVST":
        raise StateFormatError("missing MDFNSVST header")
    version = _u32(payload, 16)
    total_flags = _u32(payload, 20)
    total_length = total_flags & 0x7FFFFFFF
    big_endian_variables = bool(total_flags & 0x80000000)
    width = _u32(payload, 24)
    height = _u32(payload, 28)
    if version != MEDNAFEN_STATE_VERSION:
        raise StateFormatError(
            f"unsupported Mednafen state version 0x{version:08x}; "
            "this reader is source-locked to 1.32.1"
        )
    preview_pixels = width * height
    if not width or not height or preview_pixels > MAX_PREVIEW_PIXELS:
        raise StateFormatError("preview dimensions are empty or exceed the limit")
    section_cursor = 32 + preview_pixels * 3
    if total_length != len(payload) or section_cursor > total_length:
        raise StateFormatError("state length or preview boundary is inconsistent")

    sections: dict[str, StateSection] = {}
    cursor = section_cursor
    while cursor < total_length:
        if cursor + SECTION_HEADER_BYTES > total_length:
            raise StateFormatError("truncated section header")
        raw_name = payload[cursor : cursor + SECTION_NAME_BYTES]
        name_bytes = raw_name.split(b"\0", 1)[0]
        try:
            section_name = name_bytes.decode("ascii")
        except UnicodeDecodeError as exc:
            raise StateFormatError("non-ASCII section name") from exc
        if not section_name or any(raw_name[len(name_bytes) + 1 :]):
            raise StateFormatError("invalid section name padding")
        section_size = _u32(payload, cursor + SECTION_NAME_BYTES)
        body = cursor + SECTION_HEADER_BYTES
        body_end = body + section_size
        if body_end > total_length:
            raise StateFormatError(f"section {section_name} exceeds state length")
        if section_name in sections:
            raise StateFormatError(f"duplicate section {section_name}")
        variables = _parse_section_variables(
            payload, section_name, body, section_size
        )
        sections[section_name] = StateSection(body, section_size, variables)
        cursor = body_end
    if cursor != total_length:
        raise StateFormatError("section table does not end at the state boundary")
    return PCEFastState(
        payload=payload,
        version=version,
        total_length=total_length,
        preview_width=width,
        preview_height=height,
        preview_offset=32,
        big_endian_variables=big_endian_variables,
        sections=sections,
    )


def read_state(path: Path) -> tuple[bytes, PCEFastState]:
    compressed, payload = _decompress_state(path)
    return compressed, parse_state_payload(payload)


def _require_variable(
    state: PCEFastState, section_name: str, variable_name: str, expected_size: int
) -> tuple[StateSection, StateVariable]:
    section = state.sections.get(section_name)
    variable = section.variables.get(variable_name) if section else None
    if variable is None:
        raise StateFormatError(
            f"missing state variable {section_name}.{variable_name}"
        )
    if variable.size != expected_size:
        raise StateFormatError(
            f"unexpected size for {section_name}.{variable_name}: "
            f"{variable.size}, expected {expected_size}"
        )
    return section, variable


def _track02_user_projection(raw_track02: bytes) -> bytes:
    if (len(raw_track02) != JP_TRACK02_BYTES or
            hashlib.md5(raw_track02).hexdigest() != JP_TRACK02_MD5):
        raise StateFormatError("Track 02 is not the authenticated Japanese Rev. 1 BIN")
    if len(raw_track02) % RAW_SECTOR_BYTES:
        raise StateFormatError("Track 02 is not aligned to MODE1/2352 sectors")
    source_spans = [JP_AKUTUBA_SOURCE_SPAN, *JP_AKUTUBA_MAP_SPANS]
    source_sectors = {
        sector
        for start, size in source_spans
        for sector in range(start // MODE1_USER_BYTES,
                            (start + size - 1) // MODE1_USER_BYTES + 1)
    }
    for sector_index in source_sectors:
        start = sector_index * RAW_SECTOR_BYTES
        header = raw_track02[start : start + MODE1_USER_OFFSET]
        if (header[0] != 0 or header[1:11] != b"\xff" * 10 or
                header[11] != 0 or header[15] != 1):
            raise StateFormatError(
                f"authenticated Akutuba source sector {sector_index} is not MODE1"
            )

    projected = bytearray()
    for start in range(0, len(raw_track02), RAW_SECTOR_BYTES):
        sector = raw_track02[start : start + RAW_SECTOR_BYTES]
        projected.extend(sector[MODE1_USER_OFFSET : MODE1_USER_OFFSET + MODE1_USER_BYTES])
    return bytes(projected)


def _matches(container: bytes, needle: bytes) -> list[int]:
    found: list[int] = []
    cursor = 0
    while True:
        offset = container.find(needle, cursor)
        if offset < 0:
            return found
        found.append(offset)
        cursor = offset + 1


def _var_bytes(state: PCEFastState, section: str, variable: str, size: int) -> bytes:
    state_section, state_variable = _require_variable(
        state, section, variable, size
    )
    start = state_variable.offset
    return state.payload[start : start + state_variable.size]


def inspect_authentic_state(
    state: PCEFastState, raw_track02: bytes
) -> dict[str, object]:
    """Join authentic source bytes to named state sections and current MPR."""
    user_data = _track02_user_projection(raw_track02)
    source_start, source_end = JP_AKUTUBA_SOURCE_SPAN
    source_span = user_data[source_start:source_end]
    if len(source_span) != source_end - source_start:
        raise StateFormatError("Akutuba source span is outside Track 02")

    cd_section, cd_variable = _require_variable(
        state, "HuC", CDRAM_VARIABLE, CDRAM_BYTES
    )
    cd_start = cd_variable.offset
    cd_ram = state.payload[cd_start : cd_start + cd_variable.size]
    cdram_matches = _matches(cd_ram, source_span)
    if len(cdram_matches) != 1:
        raise StateFormatError(
            f"expected one Akutuba descriptor/map span in CD RAM, found {len(cdram_matches)}"
        )
    cdram_offset = cdram_matches[0]
    all_state_matches = _matches(state.payload, source_span)
    if len(all_state_matches) != 1:
        raise StateFormatError(
            f"expected one full-span match in state, found {len(all_state_matches)}"
        )
    if all_state_matches[0] != cd_start + cdram_offset:
        raise StateFormatError("CD RAM match does not agree with state-wide source match")

    main_section, base_ram_variable = _require_variable(
        state, "MAIN", "BaseRAM", 0x2000
    )
    base_start = base_ram_variable.offset
    base_ram = state.payload[base_start : base_start + base_ram_variable.size]
    grid_matches = []
    for map_index, (source_offset, size) in enumerate(JP_AKUTUBA_MAP_SPANS):
        grid = user_data[source_offset : source_offset + size]
        if len(grid) != size:
            raise StateFormatError(f"map {map_index} source span is truncated")
        cd_relative_expected = cdram_offset + source_offset - source_start
        if cd_relative_expected < 0 or cd_relative_expected + size > len(cd_ram):
            raise StateFormatError(f"map {map_index} falls outside CD RAM")
        if cd_ram[cd_relative_expected : cd_relative_expected + size] != grid:
            raise StateFormatError(f"map {map_index} does not match its source offset")
        grid_matches.append(_matches(base_ram, grid))

    mpr = _var_bytes(state, "CPU", "MPR", 8)
    physical_page = CDRAM_BANK_BASE + cdram_offset // 8192
    page_offset = cdram_offset % 8192
    mapped_slots = [slot for slot, bank in enumerate(mpr) if bank == physical_page]

    preview_size = state.preview_width * state.preview_height * 3
    preview = state.payload[
        state.preview_offset : state.preview_offset + preview_size
    ]
    vce_palette = _var_bytes(state, "VCE", "color_table", 0x400)
    _, vdc_vram = _require_variable(state, "VDC0", "VRAM", 0x10000)
    _, vdc_sat = _require_variable(state, "VDC0", "SAT", 0x200)

    def var_hash(section: str, variable: str, size: int) -> str:
        return hashlib.sha256(_var_bytes(state, section, variable, size)).hexdigest()

    registers: dict[str, int] = {}
    for name in ("CR", "BXR", "BYR", "MWR", "VDS_cache", "VSW_cache", "VDW_cache", "VCR_cache"):
        try:
            register = _var_bytes(state, "VDC0", name, 4 if name.endswith("_cache") else 2)
        except StateFormatError:
            continue
        registers[name] = int.from_bytes(register, "big" if state.big_endian_variables else "little")

    return {
        "state_version": state.version,
        "preview_width": state.preview_width,
        "preview_height": state.preview_height,
        "preview_sha256": hashlib.sha256(preview).hexdigest(),
        "cd_section_offset": cd_section.offset,
        "cd_variable_offset": cd_start,
        "base_ram_state_offset": base_start,
        "source_user_start": source_start,
        "source_user_end": source_end,
        "source_span_sha256": hashlib.sha256(source_span).hexdigest(),
        "cd_ram_relative_offset": cdram_offset,
        "cd_ram_state_offset": cd_start + cdram_offset,
        "physical_page": physical_page,
        "physical_page_offset": page_offset,
        "physical_address": (physical_page << 13) | page_offset,
        "current_mpr": list(mpr),
        "mapped_mpr_slots": mapped_slots,
        "base_ram_map_copy_offsets": grid_matches,
        "vce_palette_sha256": hashlib.sha256(vce_palette).hexdigest(),
        "vdc_vram_sha256": var_hash("VDC0", "VRAM", 0x10000),
        "vdc_sat_sha256": hashlib.sha256(
            _var_bytes(state, "VDC0", "SAT", 0x200)
        ).hexdigest(),
        "vdc_registers": registers,
    }


def _fmt_bytes(values: list[int]) -> str:
    return " ".join(f"{value:02x}" for value in values)


def print_report(compressed: bytes, state: PCEFastState, report: dict[str, object]) -> None:
    print(f"state_sha256={hashlib.sha256(compressed).hexdigest()}")
    print(f"state_version=0x{state.version:08x} (Mednafen 1.32.1)")
    print(f"state_bytes={len(state.payload)} sections={len(state.sections)}")
    print(
        "embedded_frame="
        f"{report['preview_width']}x{report['preview_height']} "
        f"rgb_sha256={report['preview_sha256']}"
    )
    print(
        "source=authentic JP Rev. 1 Track 02 "
        f"user=[0x{report['source_user_start']:x},0x{report['source_user_end']:x}) "
        f"sha256={report['source_span_sha256']}"
    )
    print(
        "state_match="
        f"HuC.{CDRAM_VARIABLE} "
        f"relative=0x{report['cd_ram_relative_offset']:x} "
        f"state_offset=0x{report['cd_ram_state_offset']:x}"
    )
    print(
        f"cd_ram_physical_page=0x{report['physical_page']:02x} "
        f"page_offset=0x{report['physical_page_offset']:04x} "
        f"physical_address=0x{report['physical_address']:06x}"
    )
    print(f"current_mpr={_fmt_bytes(report['current_mpr'])}")
    slots = report["mapped_mpr_slots"]
    print(
        "current_mpr_slots=" +
        (",".join(str(slot) for slot in slots) if slots else "none")
    )
    print(
        "base_ram_contiguous_map_copies=" +
        ",".join(
            f"map{index}:{','.join(f'0x{offset:x}' for offset in offsets) or 'none'}"
            for index, offsets in enumerate(report["base_ram_map_copy_offsets"])
        )
    )
    print(f"vce_color_table_sha256={report['vce_palette_sha256']}")
    print(f"vdc0_vram_sha256={report['vdc_vram_sha256']}")
    print(f"vdc0_sat_sha256={report['vdc_sat_sha256']}")
    print(
        "vdc0_registers=" + " ".join(
            f"{name}=0x{value:x}"
            for name, value in sorted(report["vdc_registers"].items())
        )
    )
    print(
        "boundary=source-byte presence and atomic snapshot inventory only; "
        "active map, guest consumer, pose binding, and renderer semantics "
        "remain unproven"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("state", type=Path, help="authentic Mednafen .mc0 save state")
    parser.add_argument("track02", type=Path, help="authentic Japanese Rev. 1 TQJP02.bin")
    args = parser.parse_args(argv)
    try:
        compressed, state = read_state(args.state)
        raw_track02 = args.track02.read_bytes()
        report = inspect_authentic_state(state, raw_track02)
    except (OSError, StateFormatError, gzip.BadGzipFile) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print_report(compressed, state, report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
