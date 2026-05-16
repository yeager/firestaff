#!/usr/bin/env python3
"""Extract Nexus Saturn ISO — low-memory streaming version."""
import struct, os
from pathlib import Path

TRACK1 = "/home/trv2/.openclaw/data/firestaff-original-games/DM/_unpacked_by_archive/n2/fbd91afd6d37_Dungeon-Master-Nexus_SEGA-Saturn_JA.zip/Dungeon Master Nexus (Japan) (Track 1).bin"
OUTDIR = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/nexus-saturn")
SECTOR_SIZE = 2352
DATA_OFF = 16
DATA_SZ = 2048

def read_sector(f, n):
    f.seek(n * SECTOR_SIZE + DATA_OFF)
    return f.read(DATA_SZ)

def read_sectors(f, start, count):
    d = b""
    for i in range(count):
        d += read_sector(f, start + i)
    return d

def parse_dir(data, off):
    if off >= len(data): return None
    rl = data[off]
    if rl == 0: return None
    lba = struct.unpack_from("<I", data, off+2)[0]
    sz = struct.unpack_from("<I", data, off+10)[0]
    fl = data[off+25]
    nl = data[off+32]
    nm = data[off+33:off+33+nl].decode("ascii", errors="replace")
    if ";" in nm: nm = nm[:nm.index(";")]
    return {"name": nm, "lba": lba, "size": sz, "is_dir": bool(fl & 2), "rec_len": rl}

def list_dir(f, lba, sz):
    sc = (sz + DATA_SZ - 1) // DATA_SZ
    d = read_sectors(f, lba, sc)
    entries, off = [], 0
    while off < len(d):
        r = parse_dir(d, off)
        if r is None:
            nb = ((off // DATA_SZ) + 1) * DATA_SZ
            if nb <= off: break
            off = nb; continue
        if r["name"] not in ("", "\x00", "\x01"):
            entries.append(r)
        off += r["rec_len"]
    return entries

def walk(f, lba, sz, prefix, depth=0):
    entries = list_dir(f, lba, sz)
    files = []
    for e in entries:
        path = f"{prefix}/{e['name']}" if prefix else e["name"]
        ind = "  " * depth
        if e["is_dir"]:
            print(f"{ind}[DIR] {e['name']}/")
            files += walk(f, e["lba"], e["size"], path, depth+1)
        else:
            kb = e["size"] / 1024
            print(f"{ind}      {e['name']}  ({kb:.0f} KB)")
            files.append({"path": path, **e})
    return files

def extract(f, entry, out):
    out.parent.mkdir(parents=True, exist_ok=True)
    sc = (entry["size"] + DATA_SZ - 1) // DATA_SZ
    with open(out, "wb") as of:
        remaining = entry["size"]
        for i in range(sc):
            sector = read_sector(f, entry["lba"] + i)
            chunk = min(remaining, DATA_SZ)
            of.write(sector[:chunk])
            remaining -= chunk

print(f"Extracting: {TRACK1}\n")
with open(TRACK1, "rb") as f:
    pvd = read_sector(f, 16)
    assert pvd[1:6] == b"CD001"
    vol = pvd[40:72].decode("ascii", errors="replace").strip()
    root_lba = struct.unpack_from("<I", pvd, 158)[0]
    root_sz = struct.unpack_from("<I", pvd, 166)[0]
    print(f"Volume: {vol}")
    print(f"Root: LBA={root_lba} size={root_sz}\n")
    
    files = walk(f, root_lba, root_sz, "")
    
    print(f"\n=== {len(files)} files ===")
    total = sum(e["size"] for e in files)
    print(f"Total: {total/1024/1024:.1f} MB\n")
    
    # Classify
    cats = {}
    for e in files:
        ext = e["name"].rsplit(".",1)[-1].upper() if "." in e["name"] else "NONE"
        cats.setdefault(ext, []).append(e)
    for ext in sorted(cats):
        t = sum(e["size"] for e in cats[ext])
        print(f"  .{ext}: {len(cats[ext])} files ({t/1024:.0f} KB)")
    
    # Extract
    OUTDIR.mkdir(parents=True, exist_ok=True)
    print(f"\nExtracting to {OUTDIR}...")
    for e in files:
        extract(f, e, OUTDIR / e["path"])
    
    # File listing
    with open(OUTDIR / "FILE_LISTING.txt", "w") as lf:
        for e in sorted(files, key=lambda x: x["path"]):
            lf.write(f"{e['size']:>10}  {e['path']}\n")
    
    print(f"Done: {len(files)} files extracted")
