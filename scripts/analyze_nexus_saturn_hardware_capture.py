#!/usr/bin/env python3
"""Verify all raw Saturn video domains in one continuous Nexus capture."""
from __future__ import annotations
import argparse, hashlib, json, mmap
from pathlib import Path
from validate_nexus_saturn_runtime_capture import (MDFN_RUNTIME_MAGIC, RUNTIME_MAGIC,
    VDP1_MAGIC, VDP1_MAGIC_MDFN, VDP1_MAGIC_V2, VDP1_PAYLOAD_BYTES,
    VDP1_STATE_RE, VDP2_MAGIC, VDP2_PAYLOAD_BYTES)

def fields(path: Path) -> dict[str, str]:
    result = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1); result[key] = value
    return result

def digest(view: bytes) -> str:
    return hashlib.sha256(view).hexdigest()

def starts(blob: mmap.mmap, token: bytes, offset: int = 0) -> bool:
    return blob[offset:offset + len(token)] == token

def inspect(raw: Path, manifest: Path) -> dict[str, object]:
    meta = fields(manifest); skip = int(meta["skip_frames"]); expected = int(meta["frame_limit"])
    states, active = [], 0
    domains = {name: set() for name in ("vdp1_vram", "vdp1_fb", "vdp2_regs", "vdp2_vram", "vdp2_cram")}
    with raw.open("rb") as stream, mmap.mmap(stream.fileno(), 0, access=mmap.ACCESS_READ) as blob:
        magic = next((m for m in (RUNTIME_MAGIC, MDFN_RUNTIME_MAGIC) if starts(blob, m)), None)
        if magic is None: raise ValueError("missing runtime magic")
        offset, frame = len(magic), 0
        while offset < len(blob):
            marker = f"frame={frame}\n".encode()
            if not starts(blob, marker, offset): raise ValueError("non-contiguous frame timing")
            offset += len(marker); state = b""
            for vdp1_magic in (VDP1_MAGIC_V2, VDP1_MAGIC_MDFN, VDP1_MAGIC):
                if starts(blob, vdp1_magic, offset): offset += len(vdp1_magic); break
            else: raise ValueError(f"missing VDP1 marker at frame {frame}")
            if starts(blob, b"state=", offset):
                end = blob.find(b"\n", offset); state = bytes(blob[offset:end])
                if end < 0 or not VDP1_STATE_RE.fullmatch(state): raise ValueError("invalid VDP1 registers")
                states.append(state); offset = end + 1
            v1 = blob[offset:offset + VDP1_PAYLOAD_BYTES]
            if len(v1) != VDP1_PAYLOAD_BYTES: raise ValueError("truncated VDP1 payload")
            domains["vdp1_vram"].add(digest(v1[:0x80000])); domains["vdp1_fb"].add(digest(v1[0x80000:]))
            match = VDP1_STATE_RE.fullmatch(state)
            if match and int(match.group(1), 16) and int(match.group(2), 16) and any(v1): active += 1
            offset += VDP1_PAYLOAD_BYTES
            if not starts(blob, VDP2_MAGIC, offset) and starts(blob, VDP2_MAGIC, offset + 1): offset += 1
            if not starts(blob, VDP2_MAGIC, offset): raise ValueError(f"missing VDP2 marker at frame {frame}")
            offset += len(VDP2_MAGIC); v2 = blob[offset:offset + VDP2_PAYLOAD_BYTES]
            regs, vram, cram = v2[:0x200], v2[0x200:0x80200], v2[0x80200:]
            if len(v2) != VDP2_PAYLOAD_BYTES or not all(any(x) for x in (regs, vram, cram)):
                raise ValueError(f"empty/truncated VDP2 domain at frame {frame}")
            domains["vdp2_regs"].add(digest(regs)); domains["vdp2_vram"].add(digest(vram)); domains["vdp2_cram"].add(digest(cram))
            offset += VDP2_PAYLOAD_BYTES; frame += 1
        if frame != expected: raise ValueError(f"manifest expects {expected} frames, got {frame}")
    if not states or not active: raise ValueError("no active state-bearing VDP1 frame")
    return {"schema":"FIRESTAFF_NEXUS_AUTHENTIC_HARDWARE_RECEIPT_V1",
        "disc_sha256":meta["disc_sha256"], "raw_sha256":meta["raw_sha256"],
        "timing":{"first_absolute_frame":skip,"last_absolute_frame":skip+expected-1,"captured_frames":expected,"contiguous":True},
        "vdp1":{"state_frames":len(states),"active_frames":active,"register_state_variants":len(set(states)),
                "vram_variants":len(domains["vdp1_vram"]),"framebuffer_variants":len(domains["vdp1_fb"])},
        "vdp2":{"register_variants":len(domains["vdp2_regs"]),"vram_variants":len(domains["vdp2_vram"]),"cram_variants":len(domains["vdp2_cram"])},
        "asset_semantics":"unassigned"}

def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument("raw",type=Path); parser.add_argument("manifest",type=Path); args=parser.parse_args()
    try: print(json.dumps(inspect(args.raw,args.manifest),sort_keys=True))
    except (OSError,KeyError,ValueError) as error: print(f"NEXUS_SATURN_HARDWARE_CAPTURE_INVALID: {error}"); return 1
    return 0
if __name__ == "__main__": raise SystemExit(main())
