#!/usr/bin/env python3
"""Build a source-only, 10x DM1 V2.2 ``.fsart`` archive.

The script reads a local original DM1 ``GRAPHICS.DAT`` (or its PC34 ZIP member)
through Artpack Studio's IMG3 decoder. It never generates replacement geometry:
every PNG is an enlarged version of a decoded original record. The enlargement
expands the original indexed palette into smooth RGB transitions using
interpolation, deblocking and restrained local contrast recovery.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import io
import json
import sys
import zipfile
from pathlib import Path

from PIL import Image, ImageEnhance, ImageFilter


def load_studio(repo_root: Path):
    source = repo_root / "scripts" / "firestaff_artpack_studio.py"
    spec = importlib.util.spec_from_file_location("firestaff_artpack_studio", source)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {source}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def restore_10x(image, scale: int):
    """Return a palette-expanded, source-only 10x restoration."""
    width = image.width * scale
    height = image.height * scale
    restored = image.resize((width, height), resample=Image.Resampling.LANCZOS)
    restored = restored.filter(ImageFilter.MedianFilter(3))
    restored = restored.filter(ImageFilter.GaussianBlur(0.65))
    restored = ImageEnhance.Color(restored).enhance(1.13)
    restored = ImageEnhance.Contrast(restored).enhance(1.06)
    return restored.filter(ImageFilter.UnsharpMask(radius=2.6, percent=72,
                                                    threshold=10))


def read_graphics_dat(graphics_dat: Path) -> tuple[Path, bytes, dict]:
    """Read an original GRAPHICS.DAT, directly from a PC34 ZIP when needed.

    The shipped PC34 corpus normally contains DATA/GRAPHICS.DAT inside a ZIP.
    Keep the input in memory instead of extracting it beside the user's game
    data or into the repository.  The returned receipt records both archive
    and member identity so a generated artpack remains source-auditable.
    """
    if graphics_dat.suffix.lower() != ".zip":
        source_bytes = graphics_dat.read_bytes()
        return graphics_dat, source_bytes, {
            "file": graphics_dat.name,
            "sha256": hashlib.sha256(source_bytes).hexdigest(),
        }

    with zipfile.ZipFile(graphics_dat) as archive:
        members = [
            info for info in archive.infolist()
            if not info.is_dir() and Path(info.filename).name.upper() == "GRAPHICS.DAT"
        ]
        if not members:
            raise ValueError(f"{graphics_dat} contains no GRAPHICS.DAT member")
        members.sort(key=lambda info: (info.filename.upper() != "DATA/GRAPHICS.DAT",
                                       info.filename.upper()))
        member = members[0]
        try:
            source_bytes = archive.read(member)
        except zipfile.BadZipFile as exc:
            # The supplied PC34 archive has a DOS-era separator mismatch:
            # its central directory says DATA/GRAPHICS.DAT while the local
            # header says DATA\\GRAPHICS.DAT. ZipFile otherwise verifies the
            # member's CRC, so retry only this exact slash-to-backslash form.
            dos_member_name = member.filename.replace("/", "\\")
            if "File name in directory" not in str(exc) or dos_member_name == member.filename:
                raise
            member.orig_filename = dos_member_name
            source_bytes = archive.read(member)

    return Path(member.filename), source_bytes, {
        "file": Path(member.filename).name,
        "sha256": hashlib.sha256(source_bytes).hexdigest(),
        "archive": graphics_dat.name,
        "archive_sha256": hashlib.sha256(graphics_dat.read_bytes()).hexdigest(),
        "archive_member": member.filename,
    }


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--graphics-dat", type=Path,
                        default=Path.home() / ".firestaff" / "data" / "dm1" / "GRAPHICS.DAT",
                        help="original GRAPHICS.DAT or a ZIP containing DATA/GRAPHICS.DAT")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--scale", type=int, default=10)
    parser.add_argument("--max-source-width", type=int, default=320,
                        help="largest original bitmap width to admit")
    parser.add_argument("--max-source-height", type=int, default=200,
                        help="largest original bitmap height to admit")
    parser.add_argument("--limit", type=int, default=0,
                        help="write at most this many decodable records (for smoke testing)")
    args = parser.parse_args()
    if args.scale < 2 or args.scale > 10:
        raise SystemExit("--scale must be between 2 and 10")
    if args.max_source_width < 1 or args.max_source_height < 1:
        raise SystemExit("source bitmap bounds must be positive")

    graphics_dat = args.graphics_dat.expanduser().resolve()
    if not graphics_dat.is_file() and graphics_dat.name.upper() == "GRAPHICS.DAT":
        pc34_archive = graphics_dat.parent / "Dungeon-Master_DOS_EN_Version-34.zip"
        if pc34_archive.is_file():
            graphics_dat = pc34_archive
    if not graphics_dat.is_file():
        raise SystemExit(f"missing original GRAPHICS.DAT or PC34 archive: {graphics_dat}")
    output = args.output.expanduser().resolve()
    if output.suffix.lower() != ".fsart":
        output = output.with_suffix(".fsart")
    output.parent.mkdir(parents=True, exist_ok=True)

    studio = load_studio(repo_root)
    try:
        source_path, source_bytes, source_receipt = read_graphics_dat(graphics_dat)
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        raise SystemExit(f"cannot read original GRAPHICS.DAT: {exc}") from exc
    assets, warnings = studio.parse_graphics_dat_assets(source_path, source_bytes, "dm1")
    manifest = {
        "manifestVersion": "1.0.0",
        "packId": "firestaff-dm1-v22-source-10x",
        "game": "dm1",
        "tool": "build_dm1_v22_source_fsart.py",
        "source": {
            **source_receipt,
            "scale": args.scale,
            "pipeline": "Lanczos, median deblock, Gaussian smoothing, palette expansion, restrained local contrast",
            "syntheticContent": False,
        },
        "warnings": warnings,
        "skippedAssets": [],
    }
    for category in studio.categories_for_game("dm1"):
        manifest[category] = []

    written = 0
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED,
                         compresslevel=6, allowZip64=True) as archive:
        for asset in assets:
            if args.limit and written >= args.limit:
                break
            if asset.width <= 0 or asset.height <= 0:
                manifest["skippedAssets"].append({"id": asset.asset_id,
                                                  "reason": "zero-size source record"})
                continue
            # GRAPHICS.DAT also carries non-bitmap tables. Their descriptor
            # bytes can resemble huge dimensions, but they have no image
            # encoding to upscale. Keep those records explicit and out of
            # the artpack rather than treating data as synthetic graphics.
            if (asset.width > args.max_source_width or
                    asset.height > args.max_source_height):
                manifest["skippedAssets"].append({
                    "id": asset.asset_id,
                    "reason": (
                        "descriptor exceeds original display bitmap bounds "
                        f"({asset.width}x{asset.height} > "
                        f"{args.max_source_width}x{args.max_source_height})"
                    ),
                })
                continue
            record = source_bytes[asset.offset:asset.offset + asset.compressed_bytes]
            try:
                original = studio.expand_img3_to_image(record, asset.width, asset.height)
                restored = restore_10x(original, args.scale)
            except Exception as exc:
                manifest["skippedAssets"].append({"id": asset.asset_id,
                                                  "reason": str(exc)})
                continue
            filename = f"{asset.asset_id}_10x.png"
            member = f"{asset.category}/{filename}"
            png = io.BytesIO()
            restored.save(png, format="PNG", optimize=False, compress_level=6)
            archive.writestr(member, png.getvalue())
            manifest[asset.category].append({
                "id": asset.asset_id,
                "source_file": filename,
                "width": restored.width,
                "height": restored.height,
                "generator": "original_graphics_dat_10x_palette_expansion",
                "source_graphic_index": asset.index,
                "source_sha256": asset.sha256,
            })
            written += 1
            if written % 25 == 0:
                print(f"wrote {written}/{len(assets)} source records", flush=True)
        manifest["assetCount"] = written
        archive.writestr("modern_asset_manifest.json",
                         json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")

    print(f"wrote {written} source-derived 10x assets: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
