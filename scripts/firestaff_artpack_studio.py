#!/usr/bin/env python3
"""Firestaff V2.2 Artpack Studio.

Cross-platform Tk/Pillow tool for building modern V2.2 artpacks for all
Firestaff games. It edits the shared modern_asset_manifest.json convention,
loads V1/reference images and V2.2 target images, supports pixel/color edits,
imports generated art, and can call an external AI generator command.

AI generation is intentionally command-based instead of hard-coding one cloud
API. Set FIRESTAFF_ARTPACK_AI_COMMAND to a command template containing any of:
  {prompt_file} {output} {source} {game} {category} {asset_id} {width} {height}

Example:
  FIRESTAFF_ARTPACK_AI_COMMAND='my-generator --prompt {prompt_file} --ref {source} --out {output}'

The tool never ships copyrighted game data. It only reads local operator files
and writes artpack PNGs/manifests under ~/.firestaff/assets/<game>/modern by
default.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import os
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from string import Template
from typing import Any, Iterable

try:
    from PIL import Image, ImageColor, ImageDraw
except Exception as exc:  # pragma: no cover - exercised by startup path
    raise SystemExit(
        "Firestaff Artpack Studio requires Pillow. Install with: "
        "python3 -m pip install Pillow\n"
        f"Import error: {exc}"
    )

if "--self-test" in sys.argv:
    class _DummyTk:
        Tk = object
        Canvas = object
        Event = object
        Misc = object

    class _DummyTtk:
        Frame = object

    tk = _DummyTk()  # type: ignore[assignment]
    ttk = _DummyTtk()  # type: ignore[assignment]
    ImageTk = None  # type: ignore[assignment]
    colorchooser = filedialog = messagebox = simpledialog = None  # type: ignore[assignment]
else:
    try:
        import tkinter as tk
        from tkinter import colorchooser, filedialog, messagebox, simpledialog, ttk
        from PIL import ImageTk
    except Exception as exc:  # pragma: no cover - exercised by startup path
        raise SystemExit(f"Tkinter is required for the GUI: {exc}")


GAMES = ("dm1", "csb", "dm2", "theron", "nexus")

COMMON_CATEGORIES = (
    "wall_shapes",
    "floor_shapes",
    "creature_shapes",
    "object_shapes",
    "projectile_shapes",
    "explosion_shapes",
    "door_shapes",
    "field_shapes",
    "ui_chrome",
    "champion_portraits",
    "title_frames",
    "entrance_frames",
    "menu_surfaces",
)

GAME_EXTRA_CATEGORIES = {
    "csb": ("chaos_runes", "dsa_scrolls"),
    "dm2": ("weather_shapes", "hud_widgets", "tech_ui"),
    "theron": ("soul_room", "track02_levels", "pcengine_ui"),
    "nexus": ("saturn_menu", "dgn_textures", "structure2_textures", "sfx_icons"),
}

DM1_REQUIRED_SLOTS = [
    ("wall_shapes", "wall_d3_carved_hero_01", "wall_d3_carved_hero_01.png"),
    ("floor_shapes", "floor_plain_hero_01", "floor_plain_hero_01.png"),
    ("floor_shapes", "floor_pit_hero_01", "floor_pit_hero_01.png"),
    ("creature_shapes", "creature_demon_hero_01", "creature_demon_hero_01.png"),
    ("champion_portraits", "champion_warrior_hero_01", "champion_warrior_hero_01.png"),
    ("door_shapes", "door_hero_01", "door_hero_01.png"),
    ("field_shapes", "field_teleporter_hero_01", "field_teleporter_hero_01.png"),
]

DEFAULT_REQUIRED = {
    "dm1": DM1_REQUIRED_SLOTS,
    "csb": [
        ("wall_shapes", "csb_wall_d3_carved_hero_01", "csb_wall_d3_carved_hero_01.png"),
        ("floor_shapes", "csb_floor_plain_hero_01", "csb_floor_plain_hero_01.png"),
        ("creature_shapes", "csb_creature_demon_hero_01", "csb_creature_demon_hero_01.png"),
    ],
    "dm2": [
        ("wall_shapes", "dm2_wall_cave_hero_01", "dm2_wall_cave_hero_01.png"),
        ("floor_shapes", "dm2_floor_hero_01", "dm2_floor_hero_01.png"),
        ("ui_chrome", "dm2_hud_frame_hero_01", "dm2_hud_frame_hero_01.png"),
    ],
    "theron": [
        ("soul_room", "theron_soul_room_hero_01", "theron_soul_room_hero_01.png"),
        ("track02_levels", "theron_level_wall_hero_01", "theron_level_wall_hero_01.png"),
        ("ui_chrome", "theron_pcengine_frame_hero_01", "theron_pcengine_frame_hero_01.png"),
    ],
    "nexus": [
        ("saturn_menu", "nexus_menu_panel_hero_01", "nexus_menu_panel_hero_01.png"),
        ("dgn_textures", "nexus_dgn_wall_hero_01", "nexus_dgn_wall_hero_01.png"),
        ("structure2_textures", "nexus_structure2_hero_01", "nexus_structure2_hero_01.png"),
    ],
}


def fnv1a32(data: bytes) -> int:
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h


def default_modern_dir(game: str) -> Path:
    return Path.home() / ".firestaff" / "assets" / game / "modern"


def utc_now() -> str:
    return _dt.datetime.now(_dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def safe_asset_filename(asset_id: str) -> str:
    cleaned = []
    for ch in asset_id.strip():
        if ch.isalnum() or ch in ("_", "-", "."):
            cleaned.append(ch)
        else:
            cleaned.append("_")
    name = "".join(cleaned).strip("._")
    return (name or "asset") + ".png"


def ensure_rgba(img: Image.Image) -> Image.Image:
    if img.mode != "RGBA":
        return img.convert("RGBA")
    return img


def categories_for_game(game: str) -> list[str]:
    cats = list(COMMON_CATEGORIES)
    for cat in GAME_EXTRA_CATEGORIES.get(game, ()):
        if cat not in cats:
            cats.append(cat)
    return cats


@dataclass
class AssetEntry:
    category: str
    asset_id: str
    source_file: str
    width: int
    height: int
    generator: str
    notes: str = ""


class Artpack:
    def __init__(self, game: str, root: Path):
        if game not in GAMES:
            raise ValueError(f"unsupported game: {game}")
        self.game = game
        self.root = root.expanduser().resolve()
        self.manifest_path = self.root / "modern_asset_manifest.json"
        self.data: dict[str, Any] = {}

    def load_or_create(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        if self.manifest_path.exists():
            with self.manifest_path.open("r", encoding="utf-8") as fp:
                loaded = json.load(fp)
            if not isinstance(loaded, dict):
                raise ValueError("modern_asset_manifest.json must be a JSON object")
            self.data = loaded
        else:
            self.data = {}
        self.data.setdefault("manifestVersion", "1.0.0")
        self.data.setdefault("packId", f"firestaff-{self.game}-v22-modern")
        self.data.setdefault("game", self.game)
        self.data.setdefault("tool", "firestaff_artpack_studio")
        for category in categories_for_game(self.game):
            self.data.setdefault(category, [])

    def save(self) -> None:
        self.root.mkdir(parents=True, exist_ok=True)
        tmp = self.manifest_path.with_suffix(".json.tmp")
        with tmp.open("w", encoding="utf-8") as fp:
            json.dump(self.data, fp, indent=2, ensure_ascii=False)
            fp.write("\n")
        tmp.replace(self.manifest_path)

    def entries(self) -> list[AssetEntry]:
        out: list[AssetEntry] = []
        for category in categories_for_game(self.game):
            raw = self.data.get(category, [])
            if not isinstance(raw, list):
                continue
            for entry in raw:
                if not isinstance(entry, dict):
                    continue
                asset_id = str(entry.get("id") or "").strip()
                if not asset_id:
                    continue
                out.append(
                    AssetEntry(
                        category=category,
                        asset_id=asset_id,
                        source_file=str(entry.get("source_file") or safe_asset_filename(asset_id)),
                        width=int(entry.get("width") or 0),
                        height=int(entry.get("height") or 0),
                        generator=str(entry.get("generator") or ""),
                        notes=str(entry.get("notes") or ""),
                    )
                )
        return out

    def find_raw_entry(self, category: str, asset_id: str) -> dict[str, Any] | None:
        raw = self.data.setdefault(category, [])
        if not isinstance(raw, list):
            raise ValueError(f"manifest category {category!r} must be a list")
        for entry in raw:
            if isinstance(entry, dict) and entry.get("id") == asset_id:
                return entry
        return None

    def upsert_asset(
        self,
        category: str,
        asset_id: str,
        image_path: Path,
        generator: str,
        notes: str = "",
    ) -> AssetEntry:
        if not category:
            raise ValueError("category is required")
        if not asset_id:
            raise ValueError("asset id is required")
        img = ensure_rgba(Image.open(image_path))
        out_dir = self.root / category
        out_dir.mkdir(parents=True, exist_ok=True)
        source_file = safe_asset_filename(asset_id)
        out_path = out_dir / source_file
        img.save(out_path)
        raw = self.data.setdefault(category, [])
        if not isinstance(raw, list):
            raise ValueError(f"manifest category {category!r} must be a list")
        entry = self.find_raw_entry(category, asset_id)
        if entry is None:
            entry = {"id": asset_id}
            raw.append(entry)
        entry.update(
            {
                "id": asset_id,
                "source_file": source_file,
                "width": img.width,
                "height": img.height,
                "generator": generator or "operator_import",
                "updatedAtUtc": utc_now(),
            }
        )
        if notes:
            entry["notes"] = notes
        self.save()
        return AssetEntry(category, asset_id, source_file, img.width, img.height, entry["generator"], notes)

    def asset_path(self, entry: AssetEntry) -> Path:
        return self.root / entry.category / entry.source_file

    def required_slots(self) -> list[tuple[str, str, str]]:
        return list(DEFAULT_REQUIRED.get(self.game, []))

    def validate_required(self) -> list[str]:
        errors: list[str] = []
        for category, asset_id, default_file in self.required_slots():
            raw = self.find_raw_entry(category, asset_id)
            if raw is None:
                errors.append(f"{category}/{asset_id}: missing manifest entry")
                continue
            generator = str(raw.get("generator") or "")
            if not generator or generator == "placeholder":
                errors.append(f"{category}/{asset_id}: generator is not real")
            source_file = str(raw.get("source_file") or default_file)
            path = self.root / category / source_file
            if not path.exists():
                errors.append(f"{category}/{asset_id}: missing file {path}")
                continue
            try:
                img = Image.open(path)
            except Exception as exc:
                errors.append(f"{category}/{asset_id}: cannot read image: {exc}")
                continue
            if int(raw.get("width") or 0) != img.width or int(raw.get("height") or 0) != img.height:
                errors.append(f"{category}/{asset_id}: manifest size does not match image")
        return errors

    def write_finish_receipt(self, reviewer: str) -> Path:
        self.save()
        required = self.required_slots()
        if not required:
            reviewed = [entry.asset_id for entry in self.entries()]
        else:
            errors = self.validate_required()
            if errors:
                raise ValueError("Cannot write finish_receipt.json:\n" + "\n".join(errors))
            reviewed = [asset_id for _, asset_id, _ in required]
        receipt = {
            "receiptVersion": "1.0.0",
            "manifestPath": str(self.manifest_path),
            "manifestHashFnv1a": f"{fnv1a32(self.manifest_path.read_bytes()):08x}",
            "reviewer": reviewer or os.environ.get("USER", "operator"),
            "reviewedAtUtc": utc_now(),
            "gateTarget": "FINISHED_REAL",
            "reviewedSlots": reviewed,
            "notes": "Generated by Firestaff V2.2 Artpack Studio after local validation.",
        }
        out = self.root / "finish_receipt.json"
        tmp = out.with_suffix(".json.tmp")
        with tmp.open("w", encoding="utf-8") as fp:
            json.dump(receipt, fp, indent=2)
            fp.write("\n")
        tmp.replace(out)
        return out


class PixelCanvas(ttk.Frame):
    def __init__(self, master: tk.Misc, title: str, editable: bool):
        super().__init__(master)
        self.title = title
        self.editable = editable
        self.image: Image.Image | None = None
        self.photo: ImageTk.PhotoImage | None = None
        self.zoom = tk.IntVar(value=2)
        self.tool = tk.StringVar(value="pencil")
        self.brush = tk.IntVar(value=1)
        self.color = "#00ffff"
        self.path: Path | None = None
        self._dragging = False

        top = ttk.Frame(self)
        top.pack(fill="x")
        ttk.Label(top, text=title).pack(side="left")
        ttk.Label(top, text="Zoom").pack(side="left", padx=(12, 2))
        ttk.Spinbox(top, from_=1, to=16, width=4, textvariable=self.zoom, command=self.render).pack(side="left")
        if editable:
            ttk.Label(top, text="Brush").pack(side="left", padx=(12, 2))
            ttk.Spinbox(top, from_=1, to=32, width=4, textvariable=self.brush).pack(side="left")
            ttk.Button(top, text="Color", command=self.choose_color).pack(side="left", padx=4)
            ttk.Button(top, text="Pencil", command=lambda: self.tool.set("pencil")).pack(side="left")
            ttk.Button(top, text="Pick", command=lambda: self.tool.set("pick")).pack(side="left")
            ttk.Button(top, text="Fill", command=lambda: self.tool.set("fill")).pack(side="left")

        self.canvas = tk.Canvas(self, width=512, height=384, background="#202020", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)
        self.canvas.bind("<Button-1>", self.on_down)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_up)
        self.zoom.trace_add("write", lambda *_: self.render())

    def load(self, path: Path) -> None:
        self.path = path
        self.image = ensure_rgba(Image.open(path))
        self.render()

    def set_image(self, img: Image.Image, path: Path | None = None) -> None:
        self.path = path
        self.image = ensure_rgba(img.copy())
        self.render()

    def choose_color(self) -> None:
        chosen = colorchooser.askcolor(self.color, parent=self)
        if chosen and chosen[1]:
            self.color = chosen[1]

    def render(self) -> None:
        self.canvas.delete("all")
        if self.image is None:
            self.canvas.create_text(20, 20, anchor="nw", fill="#aaaaaa", text="No image")
            return
        z = max(1, int(self.zoom.get() or 1))
        view = self.image.resize((self.image.width * z, self.image.height * z), Image.Resampling.NEAREST)
        self.photo = ImageTk.PhotoImage(view)
        self.canvas.create_image(0, 0, anchor="nw", image=self.photo)
        self.canvas.config(scrollregion=(0, 0, view.width, view.height))
        if z >= 8:
            self.draw_grid(view.width, view.height, z)

    def draw_grid(self, w: int, h: int, z: int) -> None:
        for x in range(0, w + 1, z):
            self.canvas.create_line(x, 0, x, h, fill="#333333")
        for y in range(0, h + 1, z):
            self.canvas.create_line(0, y, w, y, fill="#333333")

    def canvas_to_pixel(self, event: tk.Event) -> tuple[int, int] | None:
        if self.image is None:
            return None
        z = max(1, int(self.zoom.get() or 1))
        x = int(self.canvas.canvasx(event.x) // z)
        y = int(self.canvas.canvasy(event.y) // z)
        if x < 0 or y < 0 or x >= self.image.width or y >= self.image.height:
            return None
        return x, y

    def on_down(self, event: tk.Event) -> None:
        if not self.editable:
            return
        self._dragging = True
        self.apply_tool(event)

    def on_drag(self, event: tk.Event) -> None:
        if self.editable and self._dragging and self.tool.get() == "pencil":
            self.apply_tool(event)

    def on_up(self, _event: tk.Event) -> None:
        self._dragging = False

    def apply_tool(self, event: tk.Event) -> None:
        pos = self.canvas_to_pixel(event)
        if pos is None or self.image is None:
            return
        x, y = pos
        tool = self.tool.get()
        if tool == "pick":
            rgba = self.image.getpixel((x, y))
            self.color = "#%02x%02x%02x" % rgba[:3]
            return
        if tool == "fill":
            self.flood_fill(x, y, ImageColor.getcolor(self.color, "RGBA"))
        else:
            draw = ImageDraw.Draw(self.image)
            b = max(1, int(self.brush.get() or 1))
            rgba = ImageColor.getcolor(self.color, "RGBA")
            draw.rectangle((x, y, x + b - 1, y + b - 1), fill=rgba)
        self.render()

    def flood_fill(self, x: int, y: int, color: tuple[int, int, int, int]) -> None:
        if self.image is None:
            return
        target = self.image.getpixel((x, y))
        if target == color:
            return
        stack = [(x, y)]
        px = self.image.load()
        w, h = self.image.size
        while stack:
            cx, cy = stack.pop()
            if cx < 0 or cy < 0 or cx >= w or cy >= h:
                continue
            if px[cx, cy] != target:
                continue
            px[cx, cy] = color
            stack.extend(((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)))


class ArtpackStudio(tk.Tk):
    def __init__(self, initial_game: str, initial_root: Path | None):
        super().__init__()
        self.title("Firestaff V2.2 Artpack Studio")
        self.geometry("1280x820")
        self.minsize(1000, 680)
        self.game = tk.StringVar(value=initial_game)
        self.root = tk.StringVar(value=str(initial_root or default_modern_dir(initial_game)))
        self.category = tk.StringVar(value="wall_shapes")
        self.asset_id = tk.StringVar(value="wall_d3_carved_hero_01")
        self.generator = tk.StringVar(value="operator_import")
        self.ai_command = tk.StringVar(value=os.environ.get("FIRESTAFF_ARTPACK_AI_COMMAND", ""))
        self.status = tk.StringVar(value="Ready")
        self.pack: Artpack | None = None
        self.source_path: Path | None = None
        self.target_path: Path | None = None
        self._build_ui()
        self.open_pack()

    def _build_ui(self) -> None:
        top = ttk.Frame(self, padding=8)
        top.pack(fill="x")
        ttk.Label(top, text="Game").pack(side="left")
        game_box = ttk.Combobox(top, textvariable=self.game, values=GAMES, width=8, state="readonly")
        game_box.pack(side="left", padx=4)
        game_box.bind("<<ComboboxSelected>>", lambda _e: self.on_game_changed())
        ttk.Label(top, text="Pack").pack(side="left", padx=(12, 2))
        ttk.Entry(top, textvariable=self.root, width=70).pack(side="left", fill="x", expand=True)
        ttk.Button(top, text="Browse", command=self.browse_pack).pack(side="left", padx=4)
        ttk.Button(top, text="Open/Create", command=self.open_pack).pack(side="left")
        ttk.Button(top, text="Validate", command=self.validate_pack).pack(side="left", padx=4)
        ttk.Button(top, text="Write Receipt", command=self.write_receipt).pack(side="left")

        main = ttk.PanedWindow(self, orient="horizontal")
        main.pack(fill="both", expand=True, padx=8, pady=8)

        left = ttk.Frame(main, width=320)
        main.add(left, weight=0)
        ttk.Label(left, text="Assets").pack(anchor="w")
        self.asset_list = tk.Listbox(left, height=22)
        self.asset_list.pack(fill="both", expand=True)
        self.asset_list.bind("<<ListboxSelect>>", lambda _e: self.on_asset_selected())

        form = ttk.LabelFrame(left, text="Current asset", padding=6)
        form.pack(fill="x", pady=8)
        ttk.Label(form, text="Category").grid(row=0, column=0, sticky="w")
        self.category_box = ttk.Combobox(form, textvariable=self.category, values=categories_for_game(self.game.get()), width=28)
        self.category_box.grid(row=0, column=1, sticky="ew")
        ttk.Label(form, text="Asset id").grid(row=1, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.asset_id).grid(row=1, column=1, sticky="ew")
        ttk.Label(form, text="Generator").grid(row=2, column=0, sticky="w")
        ttk.Entry(form, textvariable=self.generator).grid(row=2, column=1, sticky="ew")
        form.columnconfigure(1, weight=1)

        buttons = ttk.Frame(left)
        buttons.pack(fill="x")
        ttk.Button(buttons, text="Load V1/ref", command=self.load_source).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Load V2.2 target", command=self.load_target).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Import target to pack", command=self.import_target).pack(fill="x", pady=2)
        ttk.Button(buttons, text="Save edited target", command=self.save_edited_target).pack(fill="x", pady=2)

        ai = ttk.LabelFrame(left, text="AI generation hook", padding=6)
        ai.pack(fill="x", pady=8)
        ttk.Entry(ai, textvariable=self.ai_command).pack(fill="x")
        ttk.Button(ai, text="Write prompt", command=self.write_prompt_only).pack(fill="x", pady=2)
        ttk.Button(ai, text="Run AI command", command=self.run_ai_command).pack(fill="x", pady=2)

        right = ttk.PanedWindow(main, orient="vertical")
        main.add(right, weight=1)
        canvases = ttk.PanedWindow(right, orient="horizontal")
        right.add(canvases, weight=1)
        self.source_canvas = PixelCanvas(canvases, "V1/reference", editable=False)
        self.target_canvas = PixelCanvas(canvases, "V2.2 target/editor", editable=True)
        canvases.add(self.source_canvas, weight=1)
        canvases.add(self.target_canvas, weight=1)

        log_frame = ttk.Frame(right)
        right.add(log_frame, weight=0)
        self.log = tk.Text(log_frame, height=8, wrap="word")
        self.log.pack(fill="both", expand=True)

        status = ttk.Label(self, textvariable=self.status, anchor="w")
        status.pack(fill="x", padx=8, pady=(0, 8))

    def log_line(self, msg: str) -> None:
        self.log.insert("end", msg + "\n")
        self.log.see("end")
        self.status.set(msg)

    def on_game_changed(self) -> None:
        self.root.set(str(default_modern_dir(self.game.get())))
        self.category_box.configure(values=categories_for_game(self.game.get()))
        self.category.set(categories_for_game(self.game.get())[0])
        required = DEFAULT_REQUIRED.get(self.game.get(), [])
        if required:
            self.category.set(required[0][0])
            self.asset_id.set(required[0][1])
        self.open_pack()

    def browse_pack(self) -> None:
        selected = filedialog.askdirectory(title="Select V2.2 modern artpack directory")
        if selected:
            self.root.set(selected)
            self.open_pack()

    def open_pack(self) -> None:
        try:
            self.pack = Artpack(self.game.get(), Path(self.root.get()))
            self.pack.load_or_create()
            self.pack.save()
            self.refresh_asset_list()
            self.log_line(f"Opened {self.pack.game} artpack: {self.pack.root}")
        except Exception as exc:
            messagebox.showerror("Open artpack failed", str(exc), parent=self)

    def refresh_asset_list(self) -> None:
        self.asset_list.delete(0, "end")
        if not self.pack:
            return
        for entry in self.pack.entries():
            self.asset_list.insert("end", f"{entry.category}/{entry.asset_id}  {entry.width}x{entry.height}")

    def on_asset_selected(self) -> None:
        if not self.pack:
            return
        sel = self.asset_list.curselection()
        if not sel:
            return
        entries = self.pack.entries()
        if sel[0] >= len(entries):
            return
        entry = entries[sel[0]]
        self.category.set(entry.category)
        self.asset_id.set(entry.asset_id)
        self.generator.set(entry.generator or "operator_import")
        path = self.pack.asset_path(entry)
        if path.exists():
            self.target_path = path
            self.target_canvas.load(path)

    def load_source(self) -> None:
        path = filedialog.askopenfilename(
            title="Load V1/reference image",
            filetypes=[("Images", "*.png *.bmp *.gif *.jpg *.jpeg *.tga *.webp"), ("All files", "*.*")],
        )
        if path:
            self.source_path = Path(path)
            self.source_canvas.load(self.source_path)
            self.log_line(f"Loaded reference: {self.source_path}")

    def load_target(self) -> None:
        path = filedialog.askopenfilename(
            title="Load V2.2 target image",
            filetypes=[("Images", "*.png *.bmp *.gif *.jpg *.jpeg *.tga *.webp"), ("All files", "*.*")],
        )
        if path:
            self.target_path = Path(path)
            self.target_canvas.load(self.target_path)
            self.log_line(f"Loaded target: {self.target_path}")

    def import_target(self) -> None:
        if not self.pack:
            return
        if self.target_canvas.image is None:
            messagebox.showwarning("No target image", "Load or generate a V2.2 target first.", parent=self)
            return
        with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
            tmp_path = Path(tmp.name)
        try:
            self.target_canvas.image.save(tmp_path)
            entry = self.pack.upsert_asset(
                self.category.get().strip(),
                self.asset_id.get().strip(),
                tmp_path,
                self.generator.get().strip() or "operator_import",
                "Imported by Firestaff Artpack Studio.",
            )
            self.target_path = self.pack.asset_path(entry)
            self.refresh_asset_list()
            self.log_line(f"Imported {entry.category}/{entry.asset_id}")
        except Exception as exc:
            messagebox.showerror("Import failed", str(exc), parent=self)
        finally:
            tmp_path.unlink(missing_ok=True)

    def save_edited_target(self) -> None:
        if self.target_canvas.image is None:
            return
        if self.target_path is None:
            path = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG", "*.png")])
            if not path:
                return
            self.target_path = Path(path)
        self.target_canvas.image.save(self.target_path)
        self.log_line(f"Saved edited image: {self.target_path}")

    def current_prompt(self) -> str:
        source_note = str(self.source_path) if self.source_path else "no reference image loaded"
        target_size = "same as reference"
        if self.source_canvas.image:
            target_size = f"{self.source_canvas.image.width}x{self.source_canvas.image.height}"
        return (
            f"Create Firestaff V2.2 modern art for {self.game.get()}.\n"
            f"Category: {self.category.get()}\n"
            f"Asset id: {self.asset_id.get()}\n"
            f"Target size: {target_size}\n"
            f"Reference: {source_note}\n"
            "Style requirements: preserve gameplay readability, clear pixel silhouettes, "
            "no text overlays, no UI labels unless the source asset contains them, "
            "transparent background only when the reference uses transparency, "
            "and no copyrighted replacement from another game.\n"
        )

    def write_prompt_only(self) -> None:
        path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Text", "*.txt")])
        if not path:
            return
        Path(path).write_text(self.current_prompt(), encoding="utf-8")
        self.log_line(f"Wrote AI prompt: {path}")

    def run_ai_command(self) -> None:
        command = self.ai_command.get().strip()
        if not command:
            messagebox.showinfo(
                "AI command not configured",
                "Set FIRESTAFF_ARTPACK_AI_COMMAND or enter a command template first.",
                parent=self,
            )
            return
        with tempfile.TemporaryDirectory(prefix="firestaff-art-ai-") as td:
            tmp = Path(td)
            prompt_file = tmp / "prompt.txt"
            output = tmp / "generated.png"
            source = tmp / "source.png"
            prompt_file.write_text(self.current_prompt(), encoding="utf-8")
            if self.source_canvas.image:
                self.source_canvas.image.save(source)
            values = {
                "prompt_file": str(prompt_file),
                "output": str(output),
                "source": str(source if source.exists() else ""),
                "game": self.game.get(),
                "category": self.category.get(),
                "asset_id": self.asset_id.get(),
                "width": str(self.source_canvas.image.width if self.source_canvas.image else 512),
                "height": str(self.source_canvas.image.height if self.source_canvas.image else 512),
            }
            expanded = Template(command.replace("{", "${").replace("}", "}")).safe_substitute(values)
            try:
                subprocess.run(expanded, shell=True, check=True)
                if not output.exists():
                    raise RuntimeError(f"AI command did not create {output}")
                self.target_canvas.load(output)
                self.target_path = None
                self.generator.set("ai_command")
                self.log_line("AI command generated target image")
            except Exception as exc:
                messagebox.showerror("AI generation failed", str(exc), parent=self)

    def validate_pack(self) -> None:
        if not self.pack:
            return
        errors = self.pack.validate_required()
        if errors:
            self.log_line("Validation failed:\n" + "\n".join(errors))
            messagebox.showwarning("Validation failed", "\n".join(errors), parent=self)
        else:
            self.log_line("Validation passed for required slots")
            messagebox.showinfo("Validation passed", "Required slots are complete.", parent=self)

    def write_receipt(self) -> None:
        if not self.pack:
            return
        reviewer = simpledialog.askstring("Reviewer", "Reviewer name:", parent=self) or os.environ.get("USER", "operator")
        try:
            path = self.pack.write_finish_receipt(reviewer)
            self.log_line(f"Wrote receipt: {path}")
        except Exception as exc:
            messagebox.showerror("Receipt failed", str(exc), parent=self)


def self_test() -> int:
    with tempfile.TemporaryDirectory(prefix="firestaff-artpack-studio-test-") as td:
        root = Path(td) / "assets" / "dm1" / "modern"
        pack = Artpack("dm1", root)
        pack.load_or_create()
        img_path = Path(td) / "wall.png"
        Image.new("RGBA", (8, 8), (12, 34, 56, 255)).save(img_path)
        pack.upsert_asset("wall_shapes", "wall_d3_carved_hero_01", img_path, "operator_import")
        assert pack.manifest_path.exists()
        assert pack.find_raw_entry("wall_shapes", "wall_d3_carved_hero_01") is not None
        errors = pack.validate_required()
        assert any("floor_plain_hero_01" in err for err in errors)
        for category, asset_id, filename in DM1_REQUIRED_SLOTS[1:]:
            p = Path(td) / filename
            Image.new("RGBA", (8, 8), (1, 2, 3, 255)).save(p)
            pack.upsert_asset(category, asset_id, p, "operator_import")
        assert pack.validate_required() == []
        receipt = pack.write_finish_receipt("self-test")
        assert receipt.exists()
        data = json.loads(receipt.read_text(encoding="utf-8"))
        assert data["gateTarget"] == "FINISHED_REAL"
        assert len(data["reviewedSlots"]) == len(DM1_REQUIRED_SLOTS)
    print("firestaff_artpack_studio self-test: PASS")
    return 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    ap = argparse.ArgumentParser(description="Firestaff V2.2 Artpack Studio")
    ap.add_argument("--game", choices=GAMES, default="dm1")
    ap.add_argument("--pack-dir", type=Path)
    ap.add_argument("--self-test", action="store_true")
    return ap.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        return self_test()
    app = ArtpackStudio(args.game, args.pack_dir)
    app.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
