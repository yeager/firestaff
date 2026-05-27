#!/usr/bin/env python3
"""
generate_nexus_v1_fixtures.py
=============================
Generate deterministic synthetic DGN and DMDF blobs for Nexus V1 verification
without requiring game data.

Outputs:
  fixtures/nexus_v1_dgn_lev00_synthetic.bin   — 32x32 DGN, Layout B
  fixtures/nexus_v1_dmdf_scorpion_synthetic.bin — valid DMDF with 8 verts, 12 faces
  fixtures/nexus_v1_dmdf_invalid_magic.bin    — bad magic
  fixtures/nexus_v1_dmdf_zero_verts.bin       — vertex_count=0 (rejected)
  fixtures/nexus_v1_dmdf_oversized_verts.bin  — vertex_count=99999 (rejected)
  fixtures/nexus_v1_save_synthetic.dat         — synthetic save file (FNXS format)

Provenance: none required (synthetic, deterministic)

Source-lock refs:
  DGN Layout B: src/nexus/nexus_v1_dungeon.c (nexus_v1_level_load)
  DMDF:         src/nexus/nexus_v1_dmdf_model.c (nexus_v1_dmdf_is_valid/load)
  Save:         src/nexus/nexus_v1_save_load.c (NEXUS_SAVE_MAGIC = 'FNXS')
"""

import struct
import os
from pathlib import Path

FIXTURES_DIR = Path(__file__).parent / "fixtures"
FIXTURES_DIR.mkdir(exist_ok=True)


def write_dgn_synthetic():
    """32x32 DGN, Layout B (no w/h header at byte 0).
    All floor except wall at (5,5) and (10,7), pit at (15,3)."""
    buf = bytearray(2048 + 64)

    # Force Layout B: byte 0 = 33 (w=33 > 32 → Layout A fails, falls to Layout B)
    buf[0] = 33

    # Build 32x32 grid of little-endian uint16_t
    for gy in range(32):
        for gx in range(32):
            off = (gy * 32 + gx) * 2
            val = 1  # floor
            if gx == 5 and gy == 5:
                val = 0  # wall
            elif gx == 10 and gy == 7:
                val = 0  # wall
            elif gx == 15 and gy == 3:
                val = 2  # pit
            buf[off] = (val >> 8) & 0xFF
            buf[off + 1] = val & 0xFF

    path = FIXTURES_DIR / "nexus_v1_dgn_lev00_synthetic.bin"
    path.write_bytes(bytes(buf))
    print(f"Written: {path} ({len(buf)} bytes)")


def write_dmdf_synthetic_valid():
    """Valid DMDF: magic + header + 8 vertices + 12 triangle faces."""
    data = bytearray()

    # Magic
    data.extend(b"DMDF")

    # file_size (placeholder — fill after)
    file_size_offset = len(data)
    data.extend(struct.pack(">I", 0))

    # section_count
    data.extend(struct.pack(">I", 2))

    # flags
    data.extend(struct.pack(">I", 0))

    # reserved[4]
    data.extend(struct.pack(">IIII", 0, 0, 0, 0))

    # data_offset = 48 (after header)
    data.extend(struct.pack(">I", 48))

    # vertex_offset = 0 (unused)
    data.extend(struct.pack(">I", 0))

    # vertex_count (fill later)
    vc_offset = len(data)
    data.extend(struct.pack(">I", 0))

    # face_count (fill later)
    fc_offset = len(data)
    data.extend(struct.pack(">I", 0))

    # Fill file_size
    file_size = len(data) + 8 * 10 + 12 * 6  # header + verts + faces
    struct.pack_into(">I", data, file_size_offset, file_size)
    struct.pack_into(">I", data, vc_offset, 8)
    struct.pack_into(">I", data, fc_offset, 12)

    # Vertex data (8 vertices, 10 bytes each: x,y,z,u,v as int16)
    # Simple box shape: 8 cube vertices
    vertices = [
        (0, 0, 0, 0, 0),
        (256, 0, 0, 256, 0),
        (256, 256, 0, 256, 256),
        (0, 256, 0, 0, 256),
        (0, 0, 256, 0, 0),
        (256, 0, 256, 256, 0),
        (256, 256, 256, 256, 256),
        (0, 256, 256, 0, 256),
    ]
    for x, y, z, u, v in vertices:
        data.extend(struct.pack(">hhhhh", x, y, z, u, v))

    # Face data (12 triangles = 36 uint16 indices)
    faces = [
        # front face
        0, 1, 2, 0, 2, 3,
        # back face
        4, 6, 5, 4, 7, 6,
        # left face
        0, 3, 7, 0, 7, 4,
        # right face
        1, 5, 6, 1, 6, 2,
        # top face
        3, 2, 6, 3, 6, 7,
        # bottom face
        0, 4, 5, 0, 5, 1,
    ]
    for idx in faces:
        data.extend(struct.pack(">H", idx))

    path = FIXTURES_DIR / "nexus_v1_dmdf_scorpion_synthetic.bin"
    path.write_bytes(bytes(data))
    print(f"Written: {path} ({len(data)} bytes)")


def write_dmdf_invalid_magic():
    """DMDF with wrong magic bytes."""
    data = bytearray(64)
    data[0:4] = b"XXXX"  # bad magic
    path = FIXTURES_DIR / "nexus_v1_dmdf_invalid_magic.bin"
    path.write_bytes(bytes(data))
    print(f"Written: {path}")


def write_dmdf_zero_verts():
    """DMDF with vertex_count = 0 (rejected)."""
    data = bytearray(64)
    data[0:4] = b"DMDF"
    struct.pack_into(">I", data, 4, 64)   # file_size
    struct.pack_into(">I", data, 8, 1)     # section_count
    struct.pack_into(">I", data, 28, 48)   # data_offset
    struct.pack_into(">I", data, 36, 0)    # vertex_count = 0
    struct.pack_into(">I", data, 40, 0)    # face_count = 0
    path = FIXTURES_DIR / "nexus_v1_dmdf_zero_verts.bin"
    path.write_bytes(bytes(data))
    print(f"Written: {path}")


def write_dmdf_oversized():
    """DMDF with vertex_count = 99999 (rejected)."""
    data = bytearray(64)
    data[0:4] = b"DMDF"
    struct.pack_into(">I", data, 4, 64)
    struct.pack_into(">I", data, 8, 1)
    struct.pack_into(">I", data, 28, 48)
    struct.pack_into(">I", data, 36, 99999)  # vertex_count way too large
    struct.pack_into(">I", data, 40, 0)
    path = FIXTURES_DIR / "nexus_v1_dmdf_oversized_verts.bin"
    path.write_bytes(bytes(data))
    print(f"Written: {path}")


def write_save_synthetic():
    """Synthetic FNXS save file for round-trip testing."""
    header = bytearray(64)  # Nexus_V1_SaveHeader = 64 bytes

    # magic = 'FNXS' = 0x53584E46
    struct.pack_into("<I", header, 0, 0x53584E46)
    # version = 2
    struct.pack_into("<H", header, 4, 2)
    # header_size = 64
    struct.pack_into("<H", header, 6, 64)
    # data_size = 0 (no champion/world data in synthetic)
    struct.pack_into("<I", header, 8, 0)
    # champion_data_size = 0
    struct.pack_into("<I", header, 12, 0)
    # world_data_size = 0
    struct.pack_into("<I", header, 16, 0)
    # game_time = 12345
    struct.pack_into("<I", header, 20, 12345)
    # crc32 = 0 (no data to checksum)
    struct.pack_into("<I", header, 24, 0)
    # current_level = 0
    struct.pack_into("<i", header, 28, 0)
    # party_x = 11
    struct.pack_into("<i", header, 32, 11)
    # party_y = 29
    struct.pack_into("<i", header, 36, 29)
    # party_dir = 0 (North)
    struct.pack_into("<i", header, 40, 0)
    # state_hash = 0x2F7A8BC4E6D09125 (low 32 bits)
    struct.pack_into("<I", header, 44, 0xD09125C4)
    # description = "Nexus V1 synthetic save"
    desc = b"Nexus V1 synthetic save\x00"
    header[48:48 + len(desc)] = desc

    path = FIXTURES_DIR / "nexus_v1_save_synthetic.dat"
    path.write_bytes(bytes(header))
    print(f"Written: {path} (64 bytes)")


def main():
    print("Generating Nexus V1 synthetic fixtures...")
    print()

    write_dgn_synthetic()
    write_dmdf_synthetic_valid()
    write_dmdf_invalid_magic()
    write_dmdf_zero_verts()
    write_dmdf_oversized()
    write_save_synthetic()

    print()
    print("All fixtures written to fixtures/")
    print()
    print("Fixture usage:")
    print("  DGN synthetic:     ./build/firestaff_nexus_v1_asset_manifest_probe")
    print("  DMDF valid:        ./build/firestaff_nexus_v1_dmdf_model_gate_probe")
    print("  DMDF invalid:      nexus_v1_dmdf_is_valid() returns 0")
    print("  DMDF zero verts:   nexus_v1_dmdf_load() returns -1 (vertex_count=0)")
    print("  DMDF oversized:    nexus_v1_dmdf_load() returns -1 (vertex_count=99999)")
    print("  Save synthetic:    ./build/firestaff_nexus_v1_save_load_round_trip_probe")


if __name__ == "__main__":
    main()