#!/usr/bin/env python3
"""Inspect a raw Saturn VDP2 frame for exact Nexus title source spans.

This is a deliberately narrow provenance tool.  It recognises only the
authenticated English ``TITLE.BIN``/``TITLE.CG`` pair and reports raw,
word-swapped byte spans in VDP2 VRAM and CRAM.  A character-generator upload
by itself is not a title screen: an original display-list/layer-owner capture
is still required.  Therefore this tool never grants rendering admission.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp2_composition import visible_character_spans
from analyze_nexus_vdp2_write_trace import replay_vram_bus_writes
from nexus_vdp2_registers import detect_byte_order, read_u16


TITLE_BIN_SHA256 = "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"
TITLE_BIN_JP_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
TITLE_CG_SHA256 = "fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580"
TITLE_MAPD_OFFSET = 0xE278
TITLE_MAP_COUNT = 5
TITLE_MAP_BYTES = 64 * 28 * 4
TITLE_PALETTE_OFFSET = 0x40 + TITLE_MAP_COUNT * 0x1C04
LOGOBG_SHA256 = "431d97413f8b731db2709ad3c5a7b5f6ae2e2370b751e67e479c050f08ab14c1"
WRITER_REGISTER_HEADER = "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1"
WRITER_REGISTER_LINE = re.compile(
    r"^frame=(?P<frame>[0-9]+) addr=0x(?P<address>[0-9a-fA-F]+) "
    r"pc=0x(?P<pc>[0-9a-fA-F]+)(?P<registers>(?: r[0-9]+=0x[0-9a-fA-F]+)+)"
    r"(?: .*)?$"
)
REGISTER_VALUE = re.compile(r" r(?P<register>[0-9]+)=0x(?P<value>[0-9a-fA-F]+)")
TITLE_COPY_PC = 0x06041FA0


def wordswapped(data: bytes) -> bytes:
    """Return pair-reversed data, or reject an odd Saturn word span."""
    if len(data) % 2:
        raise ValueError("Saturn word span has odd length")
    return b"".join(data[index:index + 2][::-1]
                    for index in range(0, len(data), 2))


def title_spans(title_bin: bytes, title_cg: bytes) -> tuple[bytes, list[bytes], bytes]:
    """Validate the documented MAPD/TIBG record and retain raw source spans."""
    if hashlib.sha256(title_bin).hexdigest() not in {
            TITLE_BIN_SHA256, TITLE_BIN_JP_SHA256}:
        raise ValueError("TITLE.BIN hash mismatch")
    if hashlib.sha256(title_cg).hexdigest() != TITLE_CG_SHA256:
        raise ValueError("TITLE.CG hash mismatch")
    record = title_bin[TITLE_MAPD_OFFSET:]
    if len(record) < TITLE_PALETTE_OFFSET + 32 or \
            record[:4] != b"MAPD" or record[8:12] != b"TIBG":
        raise ValueError("TITLE.BIN MAPD/TIBG record missing")
    maps: list[bytes] = []
    for index in range(TITLE_MAP_COUNT):
        offset = 0x40 + index * 0x1C04
        if (int.from_bytes(record[offset:offset + 2], "big") != 64 or
                int.from_bytes(record[offset + 2:offset + 4], "big") != 28):
            raise ValueError(f"TITLE.BIN map {index} geometry is invalid")
        maps.append(record[offset + 4:offset + 4 + TITLE_MAP_BYTES])
    # TITLE.CG begins with a 32-byte source header.  The remaining payload is
    # the documented contiguous 4bpp character-generator span.
    if len(title_cg) <= 32 or (len(title_cg) - 32) % 32:
        raise ValueError("TITLE.CG character-generator span is invalid")
    return title_cg[32:], maps, record[TITLE_PALETTE_OFFSET:TITLE_PALETTE_OFFSET + 32]


def logobg_spans(logobg: bytes) -> tuple[bytes, bytes]:
    """Validate the retail PP envelope and retain its pixel/CLUT spans.

    These are deliberately kept separate from TITLE.CG: an exact VDP2 match
    identifies bytes resident in a witness, not the layer consumer or host
    placement that would be needed to render them in Firestaff.
    """
    if hashlib.sha256(logobg).hexdigest() != LOGOBG_SHA256:
        raise ValueError("LOGOBG.DG2 hash mismatch")
    if len(logobg) < 6 or logobg[:2] != b"PP":
        raise ValueError("LOGOBG.DG2 PP header is missing")
    width = int.from_bytes(logobg[2:4], "big")
    height = int.from_bytes(logobg[4:6], "big")
    pixel_offset = 6 + 256 * 2
    if width != 320 or height != 224 or len(logobg) != pixel_offset + width * height:
        raise ValueError("LOGOBG.DG2 geometry is invalid")
    return logobg[pixel_offset:], logobg[6:pixel_offset]


def cue_track1(cue: Path) -> Path:
    """Return the first CUE FILE member without unpacking it."""
    for line in cue.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(r'^\s*FILE\s+"([^"]+)"\s+\S+', line, re.IGNORECASE)
        if match:
            return cue.parent / match.group(1)
    raise ValueError("CUE has no quoted track member")


def iso_members_in_memory(track: Path, wanted: set[str] | None) -> dict[str, bytes]:
    """Read named ISO9660 members from a 2048- or raw-2352-byte Track 1.

    The full disc image is read only as the caller's supplied game data; no
    member is materialized on disk.
    """
    image = track.read_bytes()
    sector_bytes = 2352 if len(image) % 2352 == 0 else 2048
    user_offset = 16 if sector_bytes == 2352 else 0

    def sector(lba: int) -> bytes:
        start = lba * sector_bytes + user_offset
        end = start + 2048
        if start < 0 or end > len(image):
            raise ValueError("ISO sector lies outside Track 1")
        return image[start:end]

    pvd = sector(16)
    if pvd[1:6] != b"CD001":
        raise ValueError("Track 1 has no ISO9660 primary volume descriptor")
    members: dict[str, bytes] = {}

    def read_extent(lba: int, size: int) -> bytes:
        return b"".join(sector(lba + index)
                        for index in range((size + 2047) // 2048))[:size]

    def walk(record: bytes) -> None:
        if len(record) < 34:
            raise ValueError("ISO9660 directory record is truncated")
        directory_lba = int.from_bytes(record[2:6], "little")
        directory_size = int.from_bytes(record[10:14], "little")
        directory = read_extent(directory_lba, directory_size)
        offset = 0
        while offset < len(directory):
            length = directory[offset]
            if length == 0:
                offset = ((offset // 2048) + 1) * 2048
                continue
            child = directory[offset:offset + length]
            if len(child) < 34:
                raise ValueError("ISO9660 child record is truncated")
            name_length = child[32]
            name = child[33:33 + name_length]
            if name not in (b"\x00", b"\x01"):
                normalized = name.decode("ascii", "replace").split(";", 1)[0].upper()
                if child[25] & 2:
                    walk(child)
                elif wanted is None or normalized in wanted:
                    lba = int.from_bytes(child[2:6], "little")
                    size = int.from_bytes(child[10:14], "little")
                    members[normalized] = read_extent(lba, size)
            offset += length

    walk(pvd[156:])
    missing = wanted - members.keys() if wanted is not None else set()
    if missing:
        raise ValueError("ISO9660 member missing: " + ",".join(sorted(missing)))
    return members


def read_title_assets(data_dir: Path | None, cue: Path | None) -> tuple[bytes, bytes, bytes]:
    if cue is not None:
        members = iso_members_in_memory(cue_track1(cue),
                                        {"TITLE.BIN", "TITLE.CG", "LOGOBG.DG2"})
        return members["TITLE.BIN"], members["TITLE.CG"], members["LOGOBG.DG2"]
    if data_dir is None:
        raise ValueError("either --data-dir or --cue is required")
    return ((data_dir / "TITLE.BIN").read_bytes(),
            (data_dir / "TITLE.CG").read_bytes(),
            (data_dir / "LOGOBG.DG2").read_bytes())


def find_span(haystack: bytes, source: bytes) -> tuple[int, int]:
    return haystack.find(source), haystack.find(wordswapped(source))


def find_span_with_swapped(haystack: bytes, source: bytes,
                           swapped_source: bytes) -> tuple[int, int]:
    """Find a pre-normalized source span in one captured VDP2 region."""
    return haystack.find(source), haystack.find(swapped_source)


def complete_disc_member_hits(vram: bytes, members: dict[str, bytes]) -> list[tuple[str, int, int]]:
    """Find complete, even-sized retail members resident in VDP2 VRAM.

    This deliberately does not look for arbitrary slices: a hit identifies a
    complete source member copied verbatim (or word-swapped) into a captured
    VDP2 image.  It remains insufficient to identify layer ownership,
    placement, decoding, or to admit host rendering.
    """
    hits: list[tuple[str, int, int]] = []
    for name, source in sorted(members.items()):
        if len(source) < 64 or len(source) % 2:
            continue
        exact, swapped = find_span(vram, source)
        if exact >= 0 or swapped >= 0:
            hits.append((name, exact, swapped))
    return hits


def map_row_hits(vram: bytes, maps: list[bytes]) -> list[tuple[int, int, int, int]]:
    """Find non-empty complete MAPD rows in a VDP2 VRAM witness.

    A title map may be placed or strided by retail code, so row residence is
    reported separately from a complete-map or layer-consumer claim.  Empty
    rows are excluded because cleared VRAM makes them non-probative.
    """
    hits: list[tuple[int, int, int, int]] = []
    for map_index, map_bytes in enumerate(maps):
        if len(map_bytes) != TITLE_MAP_BYTES:
            raise ValueError("invalid MAPD plane length")
        for row_index in range(28):
            row = map_bytes[row_index * 256:(row_index + 1) * 256]
            if not any(row):
                continue
            exact, swapped = find_span(vram, row)
            if exact >= 0 or swapped >= 0:
                hits.append((map_index, row_index, exact, swapped))
    return hits


def title_sh2_copy_plan(path: Path, title_cg_raw: bytes) -> tuple[int, int, int, int]:
    """Verify observed SH-2 copy-plan fields against the real title member.

    The writer-register hook samples the retail byte copier after it loads a
    source byte into r3.  This binds the observed source-buffer tail to the
    exact title member and checks its complete planned length/destination.
    It is still not a CD-read or VDP display-list ownership proof.
    """
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != WRITER_REGISTER_HEADER:
        raise ValueError("bad writer-register header")
    rows: list[tuple[int, int, dict[int, int]]] = []
    for line_number, line in enumerate(lines[1:], 2):
        match = WRITER_REGISTER_LINE.fullmatch(line)
        if not match:
            raise ValueError(f"malformed writer-register line {line_number}")
        if int(match["pc"], 16) != TITLE_COPY_PC:
            continue
        registers = {int(item["register"]): int(item["value"], 16)
                     for item in REGISTER_VALUE.finditer(match["registers"])}
        if not {0, 1, 3, 4, 6, 14}.issubset(registers):
            raise ValueError(f"missing copier register at line {line_number}")
        rows.append((int(match["frame"], 10), int(match["address"], 16), registers))
    if len(rows) < 8:
        raise ValueError("fewer than eight title writer-register samples")
    # The hardware writer is shared by multiple uploads in the same frame
    # window. Select the source-authenticated TITLE.CG plan instead of
    # assuming that the first observed invocation owns the title.
    candidates = [row for row in rows if row[2][6] == len(title_cg_raw)]
    if not candidates:
        raise ValueError("no copy-plan length equals TITLE.CG")
    frame, _, first = candidates[0]
    source_base = first[14]
    destination_base = first[4]
    length = first[6]
    rows = [row for row in candidates
            if row[0] == frame and row[2][14] == source_base and
            row[2][4] == destination_base]
    source_byte_rows = 0
    for row_frame, address, registers in rows:
        if (row_frame != frame or registers[14] != source_base or
                registers[4] != destination_base or registers[6] != length):
            raise ValueError("copy-plan registers are not stable")
        offset = registers[0] - source_base
        if not 0 <= offset < length:
            raise ValueError("copier source address lies outside planned buffer")
        if (registers[3] & 0xFF) != title_cg_raw[offset]:
            raise ValueError("copier source byte does not equal TITLE.CG")
        if (registers[1] & 0xFFFFF) != address:
            raise ValueError("VDP2 bus address does not equal copier destination")
        if registers[1] != destination_base + offset:
            raise ValueError("copier destination/source offsets differ")
        source_byte_rows += 1
    return frame, source_base, destination_base, source_byte_rows


def describe_position(exact: int, swapped: int) -> str:
    return f"exact=0x{exact:x} word_swap=0x{swapped:x}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--data-dir", type=Path)
    source.add_argument("--cue", type=Path,
                        help="read TITLE members from supplied CUE/Track 1 in memory")
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--frame", type=int,
                        help="inspect one zero-based frame instead of all frames")
    parser.add_argument("--scan-complete-disc-members", action="store_true",
                        help="with --cue, search complete retail members in VDP2 VRAM")
    parser.add_argument("--vdp2-write-trace", type=Path,
                        help="same-run producer trace; proves bus-byte source writes only")
    parser.add_argument("--vdp2-writer-registers", type=Path,
                        help="same-run SH-2 copier-register trace for TITLE.CG")
    args = parser.parse_args()
    if args.capture_frames <= 0 or (args.frame is not None and
                                    (args.frame < 0 or args.frame >= args.capture_frames)):
        print("NEXUS_TITLE_VDP2_SOURCE_INVALID: invalid frame selection")
        return 1
    if args.scan_complete_disc_members and args.cue is None:
        print("NEXUS_TITLE_VDP2_SOURCE_INVALID: full-disc scan requires --cue")
        return 1
    try:
        title_bin, title_cg_raw, logobg_raw = read_title_assets(args.data_dir, args.cue)
        title_cg, maps, palette = title_spans(title_bin, title_cg_raw)
        logobg_pixels, logobg_palette = logobg_spans(logobg_raw)
        disc_members = (iso_members_in_memory(cue_track1(args.cue), None)
                        if args.scan_complete_disc_members and args.cue else None)
    except (OSError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP2_SOURCE_INVALID: {error}")
        return 1

    cg_frames: list[int] = []
    map_frames: list[int] = []
    palette_frames: list[int] = []
    logobg_pixel_frames: list[int] = []
    logobg_palette_frames: list[int] = []
    title_cg_swapped = wordswapped(title_cg)
    title_cg_raw_swapped = wordswapped(title_cg_raw)
    maps_swapped = [wordswapped(source) for source in maps]
    palette_swapped = wordswapped(palette)
    try:
        trace_source_verified = False
        trace_source_position = -1
        trace_source_prefix_bytes = 0
        if args.vdp2_write_trace is not None:
            trace_vram, trace_writes = replay_vram_bus_writes(args.vdp2_write_trace)
            # The authenticated title witness contains 167,936 one-byte
            # writes while the retail member is 167,968 bytes.  Do not turn
            # the already-resident final 32 bytes into a false claim that
            # this interval wrote the entire member: prove precisely the
            # contiguous prefix represented by the trace instead.
            trace_source_prefix_bytes = min(trace_writes, len(title_cg_raw))
            if trace_source_prefix_bytes >= 64:
                trace_source_position = trace_vram.find(
                    title_cg_raw[:trace_source_prefix_bytes])
                trace_source_verified = trace_source_position >= 0
            print(f"vdp2_write_trace_vram_writes={trace_writes}")
            print(f"title_cg_trace_source_prefix_bytes={trace_source_prefix_bytes}")
            print("title_cg_trace_source_prefix_join=verified" if trace_source_verified
                  else "title_cg_trace_source_prefix_join=unbound")
        if args.vdp2_writer_registers is not None:
            copy_frame, copy_source, copy_destination, copy_rows = title_sh2_copy_plan(
                args.vdp2_writer_registers, title_cg_raw)
            print(f"title_cg_sh2_copy_frame={copy_frame}")
            print(f"title_cg_sh2_copy_source_base=0x{copy_source:x}")
            print(f"title_cg_sh2_copy_destination_base=0x{copy_destination:x}")
            print(f"title_cg_sh2_source_byte_rows={copy_rows}")
            print("title_cg_sh2_copy_plan=verified")
        frames = iter_frame_regions_file(args.capture, args.capture_frames)
        for index, frame in frames:
            if args.frame is not None and index != args.frame:
                continue
            cg_exact, cg_swapped = find_span_with_swapped(
                frame["vdp2-vram"], title_cg, title_cg_swapped)
            map_positions = [find_span_with_swapped(frame["vdp2-vram"], source,
                                                    swapped)
                             for source, swapped in zip(maps, maps_swapped)]
            row_positions = map_row_hits(frame["vdp2-vram"], maps)
            palette_exact, palette_swapped_position = find_span_with_swapped(
                frame["vdp2-cram"], palette, palette_swapped)
            logobg_pixels_exact, logobg_pixels_swapped = find_span(
                frame["vdp2-vram"], logobg_pixels)
            logobg_palette_exact, logobg_palette_swapped = find_span(
                frame["vdp2-cram"], logobg_palette)
            if cg_exact >= 0 or cg_swapped >= 0:
                cg_frames.append(index)
            if any(exact >= 0 or swapped >= 0 for exact, swapped in map_positions):
                map_frames.append(index)
            if palette_exact >= 0 or palette_swapped_position >= 0:
                palette_frames.append(index)
            if logobg_pixels_exact >= 0 or logobg_pixels_swapped >= 0:
                logobg_pixel_frames.append(index)
            if logobg_palette_exact >= 0 or logobg_palette_swapped >= 0:
                logobg_palette_frames.append(index)
            if args.frame is not None or cg_exact >= 0 or cg_swapped >= 0 or \
                    index in map_frames or index in palette_frames or \
                    index in logobg_pixel_frames or index in logobg_palette_frames:
                registers = frame["vdp2-regs"]
                byte_order = detect_byte_order(registers)
                tvmd = read_u16(registers, 0x00, byte_order)
                bgon = read_u16(registers, 0x20, byte_order)
                chctla = read_u16(registers, 0x28, byte_order)
                print(f"frame={index} register_byte_order={byte_order} "
                      f"tvmd=0x{tvmd:04x} bgon=0x{bgon:04x} chctla=0x{chctla:04x}")
                print("title_cg_vram_" + describe_position(cg_exact, cg_swapped) +
                      f" bytes={len(title_cg)}")
                try:
                    _, nbg1_characters = visible_character_spans(
                        frame["vdp2-vram"], registers, byte_order, 1)
                    title_start = cg_swapped if cg_swapped >= 0 else cg_exact
                    title_end = title_start + len(title_cg) if title_start >= 0 else -1
                    overlap = sorted(address for address in nbg1_characters
                                     if title_start <= address < title_end)
                    print("title_cg_nbg1_visible_character_overlap=" +
                          (",".join(f"0x{address:x}" for address in overlap)
                           if overlap else "none"))
                except ValueError as error:
                    print(f"title_cg_nbg1_visible_character_overlap=unbound:{error}")
                if trace_source_verified:
                    snapshot_position = frame["vdp2-vram"].find(title_cg_raw_swapped)
                    print(f"title_cg_trace_bus_position=0x{trace_source_position:x} "
                          f"snapshot_word_swap_position=0x{snapshot_position:x}")
                for map_index, (exact, swapped) in enumerate(map_positions):
                    print(f"title_map_{map_index}_vram_" +
                          describe_position(exact, swapped) +
                          f" bytes={len(maps[map_index])}")
                print("title_map_row_vram_hits=" +
                      (",".join(f"map{map_index}:row{row_index}:" +
                                describe_position(exact, swapped)
                                for map_index, row_index, exact, swapped in row_positions)
                       if row_positions else "none"))
                print("title_palette_cram_" +
                      describe_position(palette_exact, palette_swapped_position) +
                      f" bytes={len(palette)}")
                print("logobg_pixels_vram_" +
                      describe_position(logobg_pixels_exact, logobg_pixels_swapped) +
                      f" bytes={len(logobg_pixels)}")
                print("logobg_palette_cram_" +
                      describe_position(logobg_palette_exact, logobg_palette_swapped) +
                      f" bytes={len(logobg_palette)}")
                if disc_members is not None:
                    hits = complete_disc_member_hits(frame["vdp2-vram"], disc_members)
                    print("complete_disc_member_vram_hits=" +
                          (",".join(f"{name}:exact=0x{exact:x}:word_swap=0x{swapped:x}"
                                    for name, exact, swapped in hits) if hits else "none"))
    except (OSError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP2_SOURCE_INVALID: {error}")
        return 1

    print("title_cg_vram_source_join=verified" if cg_frames
          else "title_cg_vram_source_join=unbound")
    print("title_map_vram_source_join=verified" if map_frames
          else "title_map_vram_source_join=unbound")
    print("title_palette_cram_source_join=verified" if palette_frames
          else "title_palette_cram_source_join=unbound")
    print("logobg_pixels_vram_source_join=verified" if logobg_pixel_frames
          else "logobg_pixels_vram_source_join=unbound")
    print("logobg_palette_cram_source_join=verified" if logobg_palette_frames
          else "logobg_palette_cram_source_join=unbound")
    print("title_consumer_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
