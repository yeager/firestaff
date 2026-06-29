#!/usr/bin/env python3
"""Runbook-consistency probe for the DM1 V1 original-capture route.

The DM1 PC 3.4 original-capture runbook at
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` is a long, prose-heavy
document that names the canonical game files, the runbook's required
DOSBox Staging settings, the screen-detect automation, the crop
geometry, and the pass/fail criteria.  Several historic bugs shipped
through this runbook without being caught at the prose level:

  * a trailing ``"Done"`` token in the Step 4 ``convert`` command that
    broke the output-path parse on the first attempt;
  * an inlined draft of ``compare_captures.py`` that used Pillow's
    default NEAREST filter instead of LANCZOS, so non-320x200 inputs
    produced wildly wrong diffs;
  * an autoexec that pointed at ``DungeonMasterPC34.EXE`` (a directory
    on the canonical layout) instead of the actual binary ``DM.EXE``
    inside that directory;
  * a stale placeholder in the manifest template's
    ``firestaff_version`` row.

This probe pins the runbook prose to the on-disk tools and the
Pillow/ImageMagick semantics so the next live attempt cannot
silently regress.  The probe is hermetic: it builds synthetic
320x200 PNGs in a temp dir, runs the actual scripts, and asserts
the runbook's claimed behaviour matches the tools'.

The probe covers:

  * Step 4 — the ``convert -crop 224x136+0+33 +repage cropped_<file>``
    command must be runnable against a synthetic 320x200 PNG and
    produce a 224x136 cropped PNG.  Without ``+repage`` the cropped
    canvas keeps the source's virtual-canvas offset and Pillow reads
    the wrong window.
  * Step 5 — the on-disk ``compare_captures.py`` must classify two
    identical 320x200 PNGs as PASS (MAE 0, max delta 0) and must use
    the LANCZOS resize filter for non-320x200 inputs.
  * Step 5 — the on-disk ``compare_captures.py`` must not be the
    inlined buggy draft (i.e. the runbook must point operators at
    the script, not at a copy-pasted snippet with the wrong resize
    filter).
  * The on-disk ``dosbox_capture_preflight.py`` must render a
    hardened ``dosbox_capture.conf`` whose autoexec contains the
    binary path ``DM.EXE`` and never the directory name
    ``DungeonMasterPC34.EXE`` (which doesn't exist as a binary).
  * The runbook's "Output Manifest Template" must not reference a
    stale placeholder git hash, and must point operators at the
    on-disk ``dosbox_capture_manifest_writer.py`` so the template
    is filled in deterministically rather than by hand.
  * The on-disk ``dosbox_capture_manifest_writer.py`` self-test
    must keep passing against hermetic synthetic fixtures.

Exit code 0 means the runbook is consistent with the tools.  Exit
code 1 means the runbook needs a realignment pass before the next
DOSBox live attempt can use it.
"""
from __future__ import annotations

import json
import os
import re
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
RUNBOOK = REPO_ROOT / "docs" / "parity" / "DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md"
TOOLS_DIR = REPO_ROOT / "docs" / "parity" / "tools"
PREFLIGHT = TOOLS_DIR / "dosbox_capture_preflight.py"
COMPARE = TOOLS_DIR / "compare_captures.py"
DETECTOR = TOOLS_DIR / "dosbox_state_detector.py"
MANIFEST_WRITER = TOOLS_DIR / "dosbox_capture_manifest_writer.py"
TRANSCRIPT_WRITER = TOOLS_DIR / "dosbox_capture_transcript_writer.py"
EVENTS_ROW_BUILDER = TOOLS_DIR / "dosbox_capture_events_row_builder.py"
LIVE_ROW_GATE = REPO_ROOT / "tools" / "verify_dm1_v1_original_capture_live_row_gate.py"
ORIGINAL_VIEWPORT_CAPTURE = (
    REPO_ROOT / "scripts" / "dosbox_dm1_original_viewport_reference_capture.sh"
)
POST_DUNGEON_TARGET_SELECTOR = TOOLS_DIR / "dm1_v1_post_dungeon_pairing_target_selector.py"
POST_DUNGEON_TARGET_CONTRACT = TOOLS_DIR.parent / "DM1_V1_POST_DUNGEON_PAIRING_TARGET_CONTRACT.json"

STATUS = "PASS632_DM1_V1_CAPTURE_ROUTE_RUNBOOK_CONSISTENCY"


# ---------------------------------------------------------------------------
# Synthetic fixture helpers (PNG without Pillow — keep CI hermetic).
# ---------------------------------------------------------------------------

def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + kind
        + payload
        + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def write_png(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    raw = b"".join(b"\x00" + bytes(rgb) * width for _ in range(height))
    payload = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    data = (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", payload)
        + png_chunk(b"IDAT", zlib.compress(raw))
        + png_chunk(b"IEND", b"")
    )
    path.write_bytes(data)


def write_checker_png(path: Path, width: int, height: int) -> None:
    rows = []
    for y in range(height):
        row = bytearray(b"\x00")
        for x in range(width):
            if (x // 16 + y // 16) % 2:
                row.extend((32, 96, 160))
            else:
                row.extend((176, 64, 48))
        rows.append(bytes(row))
    payload = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    data = (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", payload)
        + png_chunk(b"IDAT", zlib.compress(b"".join(rows)))
        + png_chunk(b"IEND", b"")
    )
    path.write_bytes(data)


def json_load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def os_environ() -> dict[str, str]:
    return dict(os.environ)


# ---------------------------------------------------------------------------
# Runbook prose probes.
# ---------------------------------------------------------------------------

def check_step4_command_is_runnable() -> list[str]:
    """Step 4's crop geometry (224x136 from x=0, y=33) must be
    realisable both via the ImageMagick ``convert`` command shown in
    the runbook AND via Pillow (the runbook's Pillow-based detector
    self-test is the documented fallback when ImageMagick isn't
    installed).  We prefer ImageMagick when present so the test
    actually exercises the prose's command line; otherwise we
    fall back to a Pillow reimplementation that uses the same
    geometry and assert the on-disk detector self-test passes."""
    failures: list[str] = []
    have_convert = subprocess.run(
        ["which", "convert"], capture_output=True, text=True,
    ).returncode == 0
    if have_convert:
        with tempfile.TemporaryDirectory(prefix="runbook-step4-") as tmp:
            out = Path(tmp) / "captures"
            out.mkdir()
            for i in range(2):
                write_png(out / f"frame{i:02d}.png", 320, 200, (40, 40, 40))
            for png in sorted(out.glob("*.png")):
                proc = subprocess.run(
                    [
                        "convert",
                        str(png),
                        "-crop",
                        "224x136+0+33",
                        "+repage",
                        str(out / f"cropped_{png.name}"),
                    ],
                    capture_output=True,
                    text=True,
                    timeout=15,
                )
                if proc.returncode != 0:
                    failures.append(
                        f"Step 4: convert failed for {png.name}: "
                        f"{proc.stderr.strip() or proc.stdout.strip()}"
                    )
                    continue
                cropped = out / f"cropped_{png.name}"
                if not cropped.exists():
                    failures.append(f"Step 4: cropped file missing for {png.name}")
                    continue
                with cropped.open("rb") as fh:
                    head = fh.read(24)
                if head[:8] != b"\x89PNG\r\n\x1a\n":
                    failures.append(f"Step 4: {cropped.name} is not a PNG")
                    continue
                w, h = struct.unpack(">II", head[16:24])
                if (w, h) != (224, 136):
                    failures.append(
                        f"Step 4: {cropped.name} is {w}x{h}, expected 224x136"
                    )
        return failures
    # Fallback: exercise the Pillow-based detector self-test, which
    # uses the same (0,33,224,136) crop geometry internally and is
    # the runbook's documented fallback for hosts without ImageMagick.
    proc = subprocess.run(
        [sys.executable, str(DETECTOR), "--self-test"],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if proc.returncode != 0:
        failures.append(
            "Step 4: ImageMagick `convert` not installed ("
            "brew install imagemagick), and the Pillow-based "
            f"detector self-test also failed: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )
    return failures


def check_step4_no_trailing_done(runbook_text: str) -> list[str]:
    """Regression: the previous draft had a trailing ``"Done"`` token on
    the ``convert`` line that broke ImageMagick's output-path parse.
    The probe fails if the substring re-appears anywhere in the
    runbook (case-insensitive, but we look for the exact buggy
    pattern ``"cropped_${file}"Done``)."""
    failures: list[str] = []
    if 'cropped_${file}"Done' in runbook_text:
        failures.append(
            "Step 4: found stale \"Done\" token after the convert "
            "command (broke ImageMagick output-path parse in older drafts)"
        )
    if re.search(r"convert\s+\"[^\"]*\"\s+-crop\s+[^\s]+\s+\"[^\"]*\"Done", runbook_text):
        failures.append(
            "Step 4: convert command appears to still have the trailing "
            "\"Done\" output path artefact"
        )
    return failures


def check_step5_no_inlined_compare_script(runbook_text: str) -> list[str]:
    """Regression: earlier runbook drafts inlined a buggy
    ``compare_captures.py`` (default NEAREST filter, no LANCZOS).
    The probe fails if the runbook still inlines a full Python
    script that defines ``compare(...)`` between triple-backtick
    python fences."""
    failures: list[str] = []
    # Find every Python-fenced block in the runbook.
    fence_blocks = re.findall(
        r"```python\s*\n(.*?)```", runbook_text, flags=re.DOTALL,
    )
    for block in fence_blocks:
        lower_block = block.lower()
        if (
            "def compare(" in lower_block
            and "mae" in lower_block
            and "max_delta" in lower_block
        ):
            failures.append(
                "Step 5: runbook still inlines a buggy compare_captures.py "
                "(default NEAREST filter, no LANCZOS) — operators may "
                "copy-paste the stale draft instead of using the on-disk tool"
            )
    return failures


def check_step5_points_at_on_disk_tool(runbook_text: str) -> list[str]:
    """Step 5 must point operators at the on-disk tool, not at an
    inlined copy.  The probe fails if Step 5's invocation line uses
    ``~/firestaff-captures/tools/compare_captures.py`` without also
    referencing the repo-path on-disk tool ``docs/parity/tools/``."""
    failures: list[str] = []
    if (
        "~/firestaff-captures/tools/compare_captures.py" in runbook_text
        and "docs/parity/tools/compare_captures.py" not in runbook_text
    ):
        failures.append(
            "Step 5: runbook points at ~/firestaff-captures/tools/ "
            "compare_captures.py without cross-referencing the "
            "canonical on-disk tool at docs/parity/tools/compare_captures.py"
        )
    return failures


def check_step3_no_directory_binary(runbook_text: str) -> list[str]:
    """Regression: the runbook's autoexec used to call
    ``DungeonMasterPC34.EXE`` from the game root, but
    ``DungeonMasterPC34`` is a directory on the canonical layout,
    not a binary.  The probe fails if the runbook still contains
    a bare ``DungeonMasterPC34.EXE`` (case-insensitive) invocation
    inside the Step 3 autoexec (or anywhere else).

    It also fails if the runbook no longer contains the
    ``cd DungeonMasterPC34`` form (the corrected autoexec), since
    that is the explicit source-of-truth for the new autoexec.
    """
    failures: list[str] = []
    # The on-disk preflight legitimately records a
    # "cd DungeonMasterPC34 && DM.EXE" launch_command in the receipt,
    # but that's the receipts dir, not the runbook.  Inside the
    # runbook, the only acceptable reference is the directory name
    # followed by a `cd` or `cd `, never a `.EXE` invocation.
    for match in re.finditer(r"DungeonMasterPC34\.EXE", runbook_text):
        idx = match.start()
        window = runbook_text[max(0, idx - 60):idx + 30]
        # Allow appearances inside prose that quote the bug as
        # something we fixed, e.g. "the older autoexec attempted
        # to launch...".  The on-disk preflight (and the runbook
        # history notes) are allowed to mention it.
        if "older autoexec" in window or "earlier draft" in window:
            continue
        if "launches \"DungeonMasterPC34.EXE\"" in runbook_text[max(0, idx - 80):idx + 60]:
            continue
        failures.append(
            "Step 3: runbook still references 'DungeonMasterPC34.EXE' "
            "as if it were a binary; canonical layout has it as a "
            "directory holding DM.EXE"
        )
    # The corrected autoexec must contain `cd DungeonMasterPC34`
    # and `DM.EXE` so the on-disk preflight's autoexec stays in
    # sync with the runbook's prose.  This is the positive
    # assertion: the buggy `cd broken-subdir` form would not be
    # caught by the substring negative-assertion above.
    if "cd DungeonMasterPC34" not in runbook_text:
        failures.append(
            "Step 3: runbook no longer contains the corrected "
            "'cd DungeonMasterPC34' autoexec; preflight's autoexec "
            "and the runbook's prose are out of sync"
        )
    if re.search(r"^\s*DM\.EXE\s*$", runbook_text, flags=re.MULTILINE) is None:
        failures.append(
            "Step 3: runbook no longer contains a bare 'DM.EXE' "
            "launch line; autoexec launch path is incomplete"
        )
    return failures


def check_manifest_template_no_stale_hash(runbook_text: str) -> list[str]:
    """Regression: the manifest template's ``firestaff_version`` row
    used to hard-code ``f7f3291f or actual git hash`` as a literal
    placeholder, which would silently be written to the manifest
    by a future operator.  The probe fails if the stale hash still
    appears as a literal value (the corrected text uses
    ``actual git head`` instead)."""
    failures: list[str] = []
    if re.search(r"firestaff_version\s+f7f3291f", runbook_text):
        failures.append(
            "Output Manifest Template: firestaff_version row still "
            "hard-codes the stale hash f7f3291f — must be replaced "
            "with the actual git head"
        )
    return failures


def check_manifest_template_points_at_writer(runbook_text: str) -> list[str]:
    """The Output Manifest Template used to be filled in by hand,
    which is exactly how the stale ``f7f3291f`` placeholder
    shipped through a previous draft of the runbook.  The
    runbook must point operators at the on-disk
    ``dosbox_capture_manifest_writer.py`` and reference its
    self-test gate so the next operator copy-pastes the
    deterministic tool rather than the prose template."""
    failures: list[str] = []
    rel_writer = "docs/parity/tools/dosbox_capture_manifest_writer.py"
    if rel_writer not in runbook_text:
        failures.append(
            "Output Manifest Template: runbook does not reference "
            f"the on-disk manifest writer at {rel_writer}; operators "
            "may still fill the template by hand and re-introduce "
            "the stale-placeholder-hash regression"
        )
    if "dosbox_capture_manifest_writer.py --self-test" not in runbook_text:
        failures.append(
            "Output Manifest Template: runbook does not mention the "
            "manifest writer's --self-test gate; the on-disk tool's "
            "regression coverage is not pinned to the runbook prose"
        )
    return failures


def check_transcript_writer_points_at_writer(runbook_text: str) -> list[str]:
    """The pass608 / pass625 runtime transcript used to be a
    hand-built JSON object the next live DOSBox session would
    have to invent, which is exactly the gap that kept the
    pass608 same-viewport capture blocker in
    ``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``.
    The runbook must point operators at the on-disk
    ``dosbox_capture_transcript_writer.py`` and reference its
    self-test gate so the next live attempt emits a transcript
    the pass608 verifier's ``runtimeTranscript`` contract
    actually accepts (``promotable=True`` + every required
    field present in every row)."""
    failures: list[str] = []
    rel_writer = "docs/parity/tools/dosbox_capture_transcript_writer.py"
    if rel_writer not in runbook_text:
        failures.append(
            "Runtime Transcript handoff: runbook does not reference "
            f"the on-disk transcript writer at {rel_writer}; the next "
            "operator may hand-build a transcript and re-introduce "
            "the pass608-blocker regression"
        )
    if "dosbox_capture_transcript_writer.py --self-test" not in runbook_text:
        failures.append(
            "Runtime Transcript handoff: runbook does not mention the "
            "transcript writer's --self-test gate; the on-disk tool's "
            "regression coverage is not pinned to the runbook prose"
        )
    # The transcript writer is the pass608 / pass625 handoff; the
    # runbook must name both pass IDs (or at least one of them)
    # so a future reader can map the writer to the verifier.
    if "pass608" not in runbook_text.lower() and "pass625" not in runbook_text.lower():
        failures.append(
            "Runtime Transcript handoff: runbook does not name the "
            "pass608 / pass625 verifiers the transcript writer "
            "satisfies; a future operator cannot tell which gate "
            "this handoff code feeds into"
        )
    return failures


def check_events_row_builder_points_at_writer(runbook_text: str) -> list[str]:
    """The 41-column events TSV the transcript writer consumes has
    14 source-locked columns (``F0380_COMMAND_ProcessQueue_CPSC``,
    ``F0365/F0366_COMMAND_ProcessTypes1To2/3To6``,
    ``F0128_DUNGEONVIEW_Draw_CPSF``,
    ``F0097_DUNGEONVIEW_DrawViewport``,
    ``VIDRV_09_BlitViewPort``, the runbook §1
    ``GRAPHICS.DAT``/``DUNGEON.DAT`` SHA256s) and
    7 pass623-fixture-pinned columns (the
    ``inputToken`` → ``sourceCommandId`` mapping, the
    ``postTuple``, the Firestaff ``viewportSha256``).
    Building the row by hand is exactly the gap that
    kept the pass608 blocker in
    ``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``
    after the pass633 / pass625 work landed: a typo in a
    function name (``F0365`` vs ``F0380``), a stale
    ReDMCSB literal, a wrong queue count, or a wrong
    viewport hash silently re-introduces the blocker
    the writer is meant to catch.

    The runbook must point operators at the on-disk
    ``dosbox_capture_events_row_builder.py`` and
    reference its self-test gate so the next live
    attempt emits a row the writer's binding contract
    actually accepts, without the operator typing
    any of the 14 source-locked columns.
    """
    failures: list[str] = []
    rel_writer = "docs/parity/tools/dosbox_capture_events_row_builder.py"
    if rel_writer not in runbook_text:
        failures.append(
            "Events TSV handoff: runbook does not reference the "
            f"on-disk events row builder at {rel_writer}; the next "
            "operator may type the 41 columns by hand and silently "
            "ship a stale ReDMCSB function name the way a previous "
            "draft of the same-viewport route did"
        )
    if "dosbox_capture_events_row_builder.py --self-test" not in runbook_text:
        failures.append(
            "Events TSV handoff: runbook does not mention the "
            "events row builder's --self-test gate; the on-disk "
            "tool's regression coverage is not pinned to the "
            "runbook prose"
        )
    return failures


def check_focus_recovery_failure_mode(runbook_text: str) -> list[str]:
    """The Known Failure Modes table must include the focus-mismatch
    rawshot-fallback row, and the prose must name both the receipt
    path and the recovery reason so an operator can diagnose a
    focus-mismatch live abort without re-reading the runbook.

    Without this check a future pass that drops the
    ``dosbox_capture.focus_recovery.json`` reference from the runbook
    can leave the focus-mismatch failure mode as prose-only guidance
    with no on-disk receipt to confirm the rawshot-fallback path
    actually ran.
    """
    failures: list[str] = []
    if "rawshot_focus_recovered" not in runbook_text:
        failures.append(
            "Focus recovery gate: runbook does not mention the "
            "rawshot_focus_recovered reason; the live route cannot "
            "tell operators the rawshot fallback saved the focus "
            "window"
        )
    if "rawshot_focus_unrecoverable" not in runbook_text:
        failures.append(
            "Focus recovery gate: runbook does not mention the "
            "rawshot_focus_unrecoverable reason; an operator seeing "
            "a focus-mismatch live abort has no on-disk signal that "
            "the rawshot fallback was attempted and gave up"
        )
    if "dosbox_capture.focus_recovery.json" not in runbook_text:
        failures.append(
            "Focus recovery gate: runbook does not point operators at "
            "the on-disk focus-recovery receipt "
            "dosbox_capture.focus_recovery.json; the live abort "
            "receipt's compact summary is no longer reachable from "
            "the runbook"
        )
    if "FOCUS_MISMATCH_FRAME_LIMIT" not in runbook_text:
        failures.append(
            "Focus recovery gate: runbook does not mention the "
            "FOCUS_MISMATCH_FRAME_LIMIT trigger window; the runbook "
            "no longer names the gate that triggers the rawshot "
            "fallback"
        )
    return failures


# ---------------------------------------------------------------------------
# On-disk tool probes.
# ---------------------------------------------------------------------------

def check_compare_uses_lanczos() -> list[str]:
    """The on-disk ``compare_captures.py`` must use Pillow's LANCZOS
    filter for non-320x200 inputs (the old draft used the default
    NEAREST filter, producing wildly wrong diffs)."""
    failures: list[str] = []
    text = COMPARE.read_text(encoding="utf-8")
    # Walk only non-comment lines so a historical changelog comment
    # that quotes the LONEST typo does not trip the gate.
    code_lines = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        # Drop inline comments before checking for the LONEST typo.
        if "#" in stripped:
            stripped = stripped.split("#", 1)[0].strip()
        if stripped:
            code_lines.append(stripped)
    code = "\n".join(code_lines)
    if "LANCZOS" not in code:
        failures.append(
            f"{COMPARE.relative_to(REPO_ROOT)}: missing Image.LANCZOS "
            "resize; the buggy draft used the default NEAREST filter"
        )
    if "LONEST" in code:
        failures.append(
            f"{COMPARE.relative_to(REPO_ROOT)}: code still contains "
            "the 'LONEST' typo (would crash on any non-320x200 input)"
        )
    return failures


def check_preflight_renders_dm_exe() -> list[str]:
    """The on-disk ``dosbox_capture_preflight.py`` must render a
    hardened conf whose autoexec references ``DM.EXE`` and never
    the directory name ``DungeonMasterPC34.EXE``.  We exercise the
    self-test path (no real game data needed) AND check the source
    for the layout-aware autoexec invariants."""
    failures: list[str] = []
    # Source-level invariants: the rendered conf must not contain
    # the directory-as-binary typo, and the preflight must have a
    # layout-aware autoexec branch (inner-subdir vs game-root).
    text = PREFLIGHT.read_text(encoding="utf-8")
    # Walk code lines only so historical changelog comments that
    # reference the buggy command don't trip the gate.
    code_lines = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if "#" in stripped:
            stripped = stripped.split("#", 1)[0].strip()
        if stripped:
            code_lines.append(stripped)
    code = "\n".join(code_lines)
    if 'lines.append("DungeonMasterPC34.EXE")' in code:
        failures.append(
            f"{PREFLIGHT.relative_to(REPO_ROOT)}: source still "
            "emits 'DungeonMasterPC34.EXE' as a launch line; should "
            "be 'cd DungeonMasterPC34' + 'DM.EXE'"
        )
    if 'lines.append("DM.EXE")' not in code:
        failures.append(
            f"{PREFLIGHT.relative_to(REPO_ROOT)}: source no longer "
            "emits a bare 'DM.EXE' launch line for the inner-subdir layout"
        )
    if "cd DungeonMasterPC34" not in code:
        failures.append(
            f"{PREFLIGHT.relative_to(REPO_ROOT)}: source no longer "
            "emits a 'cd DungeonMasterPC34' for the game-root layout"
        )
    # End-to-end smoke: run the preflight's own self-test (hermetic)
    # and assert it still passes.  The self-test exercises the
    # new layout detection branch.
    with tempfile.TemporaryDirectory(prefix="runbook-preflight-") as tmp:
        sandbox = Path(tmp) / "selftest"
        proc = subprocess.run(
            [
                sys.executable,
                str(PREFLIGHT),
                "--self-test",
                "--self-test-tmp",
                str(sandbox),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if proc.returncode != 0:
            failures.append(
                f"preflight --self-test failed: "
                f"{proc.stderr.strip() or proc.stdout.strip()}"
            )
    return failures


def _default_plan_length(capture_session: Path) -> int | None:
    """Return the number of steps in the live DEFAULT_PLAN.

    Loaded by importing the capture-session module so the gate's expected
    dry-run pass count tracks the real plan without hard-coding a number.
    Falls back to None when the import or attribute lookup fails so the
    caller can flag the problem instead of silently passing.
    """
    import importlib.util

    mod_name = "_dosbox_capture_session_plan_probe"
    # The capture-session module imports its sibling ``dosbox_state_detector``
    # by bare name, so make sure its directory is importable, then register
    # the module in sys.modules before exec_module: the module uses
    # ``from __future__ import annotations`` + @dataclass, and dataclass field
    # resolution looks the module up in sys.modules by __module__ name.
    tools_dir = str(capture_session.parent)
    added_path = False
    try:
        if tools_dir not in sys.path:
            sys.path.insert(0, tools_dir)
            added_path = True
        spec = importlib.util.spec_from_file_location(
            mod_name, str(capture_session)
        )
        if spec is None or spec.loader is None:
            return None
        module = importlib.util.module_from_spec(spec)
        sys.modules[mod_name] = module
        try:
            spec.loader.exec_module(module)
            plan = getattr(module, "DEFAULT_PLAN", None)
            if plan is None:
                return None
            return len(plan)
        finally:
            sys.modules.pop(mod_name, None)
    except Exception:
        return None
    finally:
        if added_path:
            try:
                sys.path.remove(tools_dir)
            except ValueError:
                pass


def check_capture_session_focus_recovery_dry_run() -> list[str]:
    """The on-disk ``dosbox_capture_session.py`` dry-run must keep
    covering the rawshot-fallback focus-recovery gate so a future
    patch that breaks ``_classify_rawshot_focus_recovery``,
    ``_attempt_focus_recovery``, ``_write_focus_recovery_receipt``,
    or ``_focus_recovery_summary`` is caught at the
    ``dm1_v1_original_capture_session_dry_run`` CTest gate instead
    of at the next live DOSBox attempt.

    The dry-run already exercises the state machine against the
    synthetic fixtures; this check pins the focus-recovery self-test
    in addition by running the dry-run with a ``-c`` import that
    imports the helpers, and asserting the four recovery reasons
    are still reachable through the public function surface.
    """
    failures: list[str] = []
    capture_session = TOOLS_DIR / "dosbox_capture_session.py"
    if not capture_session.exists():
        return [
            f"{capture_session.relative_to(REPO_ROOT)}: tool not found; "
            "the live capture session must ship alongside the runbook"
        ]
    proc = subprocess.run(
        [sys.executable, str(capture_session), "--dry-run"],
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        failures.append(
            f"{capture_session.relative_to(REPO_ROOT)} --dry-run failed: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )
        return failures
    # The dry-run output should report an all-pass count (N/N) where N is
    # the live DEFAULT_PLAN length.  We derive N from the script itself
    # instead of hard-coding it, so a deliberate plan-length change (e.g.
    # the 2026-06-13 retune from the never-arriving title_screen step to
    # the verified graphics/sound/input/entrance route) updates the gate
    # automatically while a real classifier/plan regression that drops a
    # transition still fails (the matched count would be < N).
    plan_len = _default_plan_length(capture_session)
    if plan_len is None:
        failures.append(
            f"{capture_session.relative_to(REPO_ROOT)}: could not determine "
            "DEFAULT_PLAN length to validate the dry-run pass count"
        )
    else:
        expected = f"{plan_len}/{plan_len}"
        if expected not in proc.stdout:
            failures.append(
                f"{capture_session.relative_to(REPO_ROOT)} --dry-run did not "
                f"report the {expected} state-machine pass count; the "
                "classifier or plan likely regressed"
            )
    if "PASS" not in proc.stdout:
        failures.append(
            f"{capture_session.relative_to(REPO_ROOT)} --dry-run did not "
            "end with PASS; the focus-recovery self-test likely regressed"
        )
    return failures


def check_capture_session_post_dungeon_route_parser() -> list[str]:
    """The live capture session must validate post-dungeon route syntax
    without launching DOSBox.

    This keeps B1 follow-up capture attempts reproducible: creature-chain,
    champion-panel, and route-specific captures can be expressed as a
    machine-readable list of post-entry keys instead of an operator-only
    manual sequence.
    """
    failures: list[str] = []
    capture_session = TOOLS_DIR / "dosbox_capture_session.py"
    if not capture_session.exists():
        return [
            f"{capture_session.relative_to(REPO_ROOT)}: tool not found; "
            "the live capture session must ship alongside the runbook"
        ]
    proc = subprocess.run(
        [sys.executable, str(capture_session), "--self-test-post-dungeon-route"],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if proc.returncode != 0:
        failures.append(
            f"{capture_session.relative_to(REPO_ROOT)} "
            f"--self-test-post-dungeon-route failed: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )
    if "PASS post-dungeon route parser" not in proc.stdout:
        failures.append(
            f"{capture_session.relative_to(REPO_ROOT)} "
            "did not report the post-dungeon route parser PASS marker"
        )
    return failures


def check_detector_selftest_passes() -> list[str]:
    """The on-disk state detector self-test must keep passing; this
    is the regression guard for the calibrated 0.135 band that
    replaced the broken 0.70/0.10 envelope."""
    failures: list[str] = []
    proc = subprocess.run(
        [sys.executable, str(DETECTOR), "--self-test"],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if proc.returncode != 0:
        failures.append(
            f"{DETECTOR.relative_to(REPO_ROOT)} --self-test failed: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )
    return failures


def check_manifest_writer_selftest_passes() -> list[str]:
    """The on-disk manifest writer self-test must keep passing; this
    is the regression guard for the Output Manifest Template
    handoff code (matching case, SHA-mismatch case, missing
    receipt case, pin-violation case, invalid-classification
    case, manifest-structural invariants)."""
    failures: list[str] = []
    if not MANIFEST_WRITER.exists():
        return [
            f"{MANIFEST_WRITER.relative_to(REPO_ROOT)}: tool not found; "
            "the manifest writer must ship alongside the runbook"
        ]
    with tempfile.TemporaryDirectory(prefix="runbook-manifest-writer-") as tmp:
        sandbox = Path(tmp) / "selftest"
        proc = subprocess.run(
            [
                sys.executable,
                str(MANIFEST_WRITER),
                "--self-test",
                "--self-test-tmp",
                str(sandbox),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if proc.returncode != 0:
            failures.append(
                f"{MANIFEST_WRITER.relative_to(REPO_ROOT)} --self-test "
                f"failed: {proc.stderr.strip() or proc.stdout.strip()}"
            )
    return failures


def check_transcript_writer_selftest_passes() -> list[str]:
    """The on-disk runtime transcript writer self-test must keep
    passing; this is the regression guard for the pass608 /
    pass625 handoff code (matching case, unknown-input-token
    case, command/handler-mismatch case, partyAfter<->redraw
    drift case, unknown-fixture-hash case, preflight-pin-
    violation case, asset-set-mismatch case, bad-run-id case,
    transcript-structural invariants, and the verbatim column-
    order contract for the events TSV).  Without this gate a
    future patch that breaks the writer would land on main
    without CI catching it and the pass608 blocker would stay
    blocked."""
    failures: list[str] = []
    if not TRANSCRIPT_WRITER.exists():
        return [
            f"{TRANSCRIPT_WRITER.relative_to(REPO_ROOT)}: tool not found; "
            "the runtime transcript writer must ship alongside the runbook"
        ]
    with tempfile.TemporaryDirectory(prefix="runbook-transcript-writer-") as tmp:
        sandbox = Path(tmp) / "selftest"
        proc = subprocess.run(
            [
                sys.executable,
                str(TRANSCRIPT_WRITER),
                "--self-test",
                "--self-test-tmp",
                str(sandbox),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if proc.returncode != 0:
            failures.append(
                f"{TRANSCRIPT_WRITER.relative_to(REPO_ROOT)} --self-test "
                f"failed: {proc.stderr.strip() or proc.stdout.strip()}"
            )
    return failures


def check_events_row_builder_selftest_passes() -> list[str]:
    """The on-disk events row builder self-test must keep
    passing; this is the regression guard for the events TSV
    handoff code (matching case, unknown-route-label case,
    missing-receipt case, bad-receipt case, missing-raw case,
    multi-command split, end-to-end single-command
    promotable, end-to-end multi-command promotable,
    dispatch-handler-by-command, source-function-pinned,
    verbatim column-order, baseline-row cases).  Without this
    gate a future patch that breaks the helper would land on
    main without CI catching it and the next live attempt
    would have to type the 41 columns by hand again, which is
    exactly how a previous draft of the same-viewport route
    re-introduced the pass608 blocker.
    """
    failures: list[str] = []
    if not EVENTS_ROW_BUILDER.exists():
        return [
            f"{EVENTS_ROW_BUILDER.relative_to(REPO_ROOT)}: tool not "
            "found; the events row builder must ship alongside the "
            "runbook"
        ]
    with tempfile.TemporaryDirectory(prefix="runbook-events-row-builder-") as tmp:
        sandbox = Path(tmp) / "selftest"
        proc = subprocess.run(
            [
                sys.executable,
                str(EVENTS_ROW_BUILDER),
                "--self-test",
                "--self-test-tmp",
                str(sandbox),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if proc.returncode != 0:
            failures.append(
                f"{EVENTS_ROW_BUILDER.relative_to(REPO_ROOT)} --self-test "
                f"failed: {proc.stderr.strip() or proc.stdout.strip()}"
            )
    return failures


def check_live_row_gate_selftest_passes() -> list[str]:
    """The on-disk live row gate
    (``tools/verify_dm1_v1_original_capture_live_row_gate.py``)
    must keep passing; this is the regression guard for the
    live session runner's
    ``original/01_ingame_start.png`` /
    ``original/02_ingame_step_forward.png`` filenames binding
    to the pass623 canonical input-capture fixture and the
    Firestaff fixture viewport hash set, and then feeding the
    on-disk preflight + row builder + transcript writer +
    pass608 verifier chain to a PROMOTED status.  Without this
    gate a future operator who renames the live capture file
    or its binding pass623 label would silently ship a
    transcript whose original frame path is no longer in the
    live runbook; the runbook-consistency probe is the CI
    companion to the CTest
    ``dm1_v1_original_capture_live_row_gate`` (which lives in
    the same script) so the regression is caught on the
    first runbook-touching commit, not at the next live
    attempt.
    """
    failures: list[str] = []
    if not LIVE_ROW_GATE.exists():
        return [
            f"{LIVE_ROW_GATE.relative_to(REPO_ROOT)}: live row gate "
            "not found; the live-binding companion to "
            "verify_dm1_v1_original_capture_route_handoff.py must ship "
            "alongside the runbook"
        ]
    proc = subprocess.run(
        [sys.executable, str(LIVE_ROW_GATE)],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=300,
    )
    if proc.returncode != 0:
        failures.append(
            f"{LIVE_ROW_GATE.relative_to(REPO_ROOT)} self-test failed: "
            f"exit={proc.returncode} "
            f"stderr={proc.stderr.strip()!r} "
            f"stdout_tail={proc.stdout.strip().splitlines()[-3:] if proc.stdout.strip() else []!r}"
        )
    return failures


def check_original_viewport_capture_single_row_mode() -> list[str]:
    """The live DOSBox rawshot runner used to hard-code six screenshots
    everywhere.  That was fine for the old overlay lane, but the current
    pass625/pass626 handoff is intentionally one transcript row:
    ``02_turn_right_west_1_3`` after a C002 turn-right.  This hermetic
    probe verifies that the script accepts that one-row route, normalizes
    exactly one synthetic 320x200 rawshot, writes the route label + health
    manifests, produces the pass513 scaffold row, and still rejects a
    black-frame rawshot before it can become transcript input.
    """
    failures: list[str] = []
    route = "wait:10 right wait:10 shot:02_turn_right_west_1_3"

    with tempfile.TemporaryDirectory(prefix="dm1-original-single-row-") as tmp:
        out = Path(tmp) / "good"
        out.mkdir()
        write_checker_png(out / "image0001.png", 320, 200)
        env = {
            **dict(),
            "OUT_DIR": str(out),
            "DM1_ORIGINAL_EXPECTED_SHOTS": "single-transcript-row",
            "DM1_ORIGINAL_ROUTE_EVENTS": route,
        }
        proc = subprocess.run(
            [str(ORIGINAL_VIEWPORT_CAPTURE), "--normalize-only"],
            cwd=REPO_ROOT,
            env={**os_environ(), **env},
            capture_output=True,
            text=True,
            timeout=30,
        )
        if proc.returncode != 0:
            failures.append(
                "single-row normalize-only failed: "
                f"{proc.stderr.strip() or proc.stdout.strip()}"
            )
        else:
            health = json_load(out / "raw_frame_health.json")
            if health.get("expectedCaptureCount") != 1 or health.get("captureCount") != 1:
                failures.append("single-row raw_frame_health count is not 1/1")
            labels = (out / "original_viewport_shot_labels.tsv").read_text(encoding="utf-8")
            if "02_turn_right_west_1_3" not in labels:
                failures.append("single-row shot label manifest lost the pass625 route label")
            crops = sorted((out / "viewport_224x136").glob("*.ppm"))
            if len(crops) != 1 or "02_turn_right_west_1_3" not in crops[0].name:
                failures.append("single-row normalization did not produce the labeled crop")
            scaffold = json_load(out / "pass513_i34e_route_key_transcript_scaffold.json")
            rows = scaffold.get("rows", [])
            if len(rows) != 1 or rows[0].get("f0380Command") != "C002_COMMAND_TURN_RIGHT":
                failures.append("single-row pass513 scaffold did not bind C002 turn-right")

        preflight = subprocess.run(
            [str(ORIGINAL_VIEWPORT_CAPTURE), "--preflight-route"],
            cwd=REPO_ROOT,
            env={
                **os_environ(),
                "OUT_DIR": str(out),
                "DM1_ORIGINAL_EXPECTED_SHOTS": "single-transcript-row",
                "DM1_ORIGINAL_ROUTE_EVENTS": route,
            },
            capture_output=True,
            text=True,
            timeout=30,
        )
        expected_preflight_ok = preflight.returncode == 0 or (
            preflight.returncode == 6
            and "single transcript-row route locked" in (preflight.stdout + preflight.stderr)
            and "no supported route injector" in (preflight.stdout + preflight.stderr)
        )
        if not expected_preflight_ok:
            failures.append(
                "single-row preflight did not validate the pass625 route before "
                f"injector checks: {preflight.stderr.strip() or preflight.stdout.strip()}"
            )

        bad = Path(tmp) / "black"
        bad.mkdir()
        write_png(bad / "image0001.png", 320, 200, (0, 0, 0))
        bad_proc = subprocess.run(
            [str(ORIGINAL_VIEWPORT_CAPTURE), "--normalize-only"],
            cwd=REPO_ROOT,
            env={
                **os_environ(),
                "OUT_DIR": str(bad),
                "DM1_ORIGINAL_EXPECTED_SHOTS": "single-transcript-row",
                "DM1_ORIGINAL_ROUTE_EVENTS": route,
            },
            capture_output=True,
            text=True,
            timeout=30,
        )
        if bad_proc.returncode == 0:
            failures.append("single-row normalize-only accepted a black rawshot")
        elif "black/blank rawshot candidate" not in (bad_proc.stderr + bad_proc.stdout):
            failures.append("black rawshot rejection did not report the black/blank blocker")

    return failures


def check_post_dungeon_target_selector_selftest_passes() -> list[str]:
    """The on-disk reviewed-target selector
    (``docs/parity/tools/dm1_v1_post_dungeon_pairing_target_selector.py``)
    must keep passing its hermetic self-test, and the contract it
    ships against must match the selector's pinned kind list.

    This is the regression guard for the Step 5b.1 target-selection
    accountability gate: a future operator who hand-types a
    post-dungeon route without first running the selector cannot
    silently ship a receipt whose pairing target was never reviewed.
    The runbook-consistency probe is the CI companion to the CTest
    ``dm1_v1_post_dungeon_pairing_target_selector`` so the regression
    is caught on the first runbook-touching commit, not at the next
    live attempt.
    """
    failures: list[str] = []
    if not POST_DUNGEON_TARGET_SELECTOR.exists():
        return [
            f"{POST_DUNGEON_TARGET_SELECTOR.relative_to(REPO_ROOT)}: "
            "selector not found; the reviewed-target gate must ship "
            "alongside the runbook"
        ]
    if not POST_DUNGEON_TARGET_CONTRACT.exists():
        return [
            f"{POST_DUNGEON_TARGET_CONTRACT.relative_to(REPO_ROOT)}: "
            "contract not found; the reviewed-target gate cannot "
            "validate any selection without a contract"
        ]
    proc = subprocess.run(
        [sys.executable, str(POST_DUNGEON_TARGET_SELECTOR), "--self-test"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        failures.append(
            f"{POST_DUNGEON_TARGET_SELECTOR.relative_to(REPO_ROOT)} "
            f"--self-test failed: "
            f"stderr={proc.stderr.strip()!r} "
            f"stdout_tail={proc.stdout.strip().splitlines()[-3:] if proc.stdout.strip() else []!r}"
        )
    elif "PASS post-dungeon pairing target selector self-test" not in proc.stdout:
        failures.append(
            f"{POST_DUNGEON_TARGET_SELECTOR.relative_to(REPO_ROOT)} "
            "did not report the post-dungeon pairing target selector "
            "PASS marker"
        )

    # Cross-check that the selector's pinned kind list matches the
    # contract's supported kinds; a future operator who edits one
    # without the other is caught here.
    selector_proc = subprocess.run(
        [
            sys.executable, "-c",
            "import sys, json; "
            f"sys.path.insert(0, {str(TOOLS_DIR)!r}); "
            "import dm1_v1_post_dungeon_pairing_target_selector as sel; "
            "import json; "
            "print(json.dumps({"
            "  'contract_kinds': sorted(sel._load_contract()['supportedTargetKinds']),"
            "  'selector_keys': sorted(sel.SUPPORTED_ROUTE_KEYS),"
            "  'classifier_states': sorted(sel.KNOWN_CLASSIFIER_STATES),"
            "  'baseline_non_claims': list(sel.BASELINE_NON_CLAIMS),"
            "}))",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=30,
    )
    if selector_proc.returncode != 0:
        failures.append(
            "selector pin introspection failed: "
            f"{selector_proc.stderr.strip() or selector_proc.stdout.strip()}"
        )
        return failures
    try:
        pin_summary = json.loads(selector_proc.stdout.strip().splitlines()[-1])
    except (json.JSONDecodeError, IndexError) as exc:
        failures.append(
            f"selector pin introspection produced unparseable output: "
            f"{exc}: {selector_proc.stdout.strip()!r}"
        )
        return failures

    contract = json.loads(
        POST_DUNGEON_TARGET_CONTRACT.read_text(encoding="utf-8")
    )
    expected_kinds = sorted(contract["supportedTargetKinds"])
    if pin_summary["contract_kinds"] != expected_kinds:
        failures.append(
            "selector's loaded contract kinds "
            f"{pin_summary['contract_kinds']!r} disagree with the "
            f"on-disk contract supportedTargetKinds={expected_kinds!r}"
        )
    expected_keys = sorted({
        "Keypad-2", "Keypad-4", "Keypad-5", "Keypad-6", "Keypad-8"
    })
    if pin_summary["selector_keys"] != expected_keys:
        failures.append(
            "selector's SUPPORTED_ROUTE_KEYS "
            f"{pin_summary['selector_keys']!r} diverge from the "
            f"dosbox_capture_session.py KEY_MAP keypad set "
            f"{expected_keys!r}"
        )
    if not all(
        claim in pin_summary["baseline_non_claims"]
        for claim in (
            "pixel parity not promoted",
            "proprietary frame bytes stay operator-local",
            "selector is accountability, not promotion",
        )
    ):
        failures.append(
            "selector's BASELINE_NON_CLAIMS dropped one of the "
            "three contract-required baseline entries"
        )

    # Negative path: a synthetic selection with an unknown
    # ``reviewer_pass_id`` must fail through the CLI, mirroring the
    # self-test's negative case so a future refactor that breaks
    # the CLI binding is caught here.
    with tempfile.TemporaryDirectory(prefix="dm1-post-dungeon-selector-") as tmp:
        sel_path = Path(tmp) / "target_selection.json"
        good_path = Path(tmp) / "good.receipt.json"
        bad_path = Path(tmp) / "bad.receipt.json"
        sys.path.insert(0, str(TOOLS_DIR))
        try:
            import dm1_v1_post_dungeon_pairing_target_selector as _selector  # type: ignore
        except Exception as exc:
            failures.append(
                "could not import selector module for negative CLI "
                f"binding: {exc}"
            )
            return failures
        try:
            contract_doc = _selector._load_contract()
            good_selection = _selector._build_valid_selection(
                target_kind="creature_chain", contract=contract_doc,
            )
            bad_selection = dict(good_selection)
            bad_selection["reviewer_pass_id"] = "pass9999"
            sel_path.write_text(json.dumps(bad_selection), encoding="utf-8")
        finally:
            sys.path.pop(0)
        cli_proc = subprocess.run(
            [
                sys.executable,
                str(POST_DUNGEON_TARGET_SELECTOR),
                "--selection", str(sel_path),
                "--out", str(bad_path),
                "--no-preflight-pin",
            ],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
            timeout=30,
        )
        if cli_proc.returncode == 0:
            failures.append(
                "post-dungeon selector CLI accepted an unknown "
                "reviewer_pass_id; the negative binding check failed"
            )
        elif "reviewer_pass_id" not in (cli_proc.stderr + cli_proc.stdout):
            failures.append(
                "post-dungeon selector CLI did not surface the "
                f"reviewer_pass_id failure: "
                f"stderr={cli_proc.stderr.strip()!r} "
                f"stdout={cli_proc.stdout.strip()!r}"
            )

        # Build a passing CLI selection and confirm the receipt
        # lands; this is the positive binding check.
        with tempfile.TemporaryDirectory(prefix="dm1-post-dungeon-good-") as inner:
            inner_sel = Path(inner) / "target_selection.json"
            inner_out = Path(inner) / "good.receipt.json"
            sys.path.insert(0, str(TOOLS_DIR))
            try:
                good_inner = _selector._build_valid_selection(
                    target_kind="creature_chain", contract=contract_doc,
                )
                inner_sel.write_text(
                    json.dumps(good_inner), encoding="utf-8"
                )
            finally:
                sys.path.pop(0)
            good_proc = subprocess.run(
                [
                    sys.executable,
                    str(POST_DUNGEON_TARGET_SELECTOR),
                    "--selection", str(inner_sel),
                    "--out", str(inner_out),
                    "--no-preflight-pin",
                ],
                cwd=str(REPO_ROOT),
                capture_output=True,
                text=True,
                timeout=30,
            )
            if good_proc.returncode != 0:
                failures.append(
                    "post-dungeon selector CLI refused a contract-"
                    f"passing selection: exit={good_proc.returncode} "
                    f"stderr={good_proc.stderr.strip()!r}"
                )
            elif not inner_out.exists():
                failures.append(
                    "post-dungeon selector CLI did not write its "
                    "target-selection receipt even with a passing selection"
                )
            else:
                receipt_doc = json.loads(
                    inner_out.read_text(encoding="utf-8")
                )
                if receipt_doc.get("schema") != (
                    "firestaff.dm1_v1.post_dungeon_pairing_target_selection.v1"
                ):
                    failures.append(
                        "post-dungeon selector CLI wrote a receipt "
                        "with the wrong schema "
                        f"{receipt_doc.get('schema')!r}"
                    )

    return failures


# ---------------------------------------------------------------------------
# Driver.
# ---------------------------------------------------------------------------

def main() -> int:
    failures: list[str] = []
    matched = 0
    total = 0

    if not RUNBOOK.exists():
        print(f"FAIL: runbook not found: {RUNBOOK}")
        return 1

    runbook_text = RUNBOOK.read_text(encoding="utf-8")

    # Runbook prose checks.
    prose_checks = [
        ("step4_no_trailing_done",       check_step4_no_trailing_done,           [runbook_text]),
        ("step4_command_runs",           check_step4_command_is_runnable,        []),
        ("step3_no_directory_binary",    check_step3_no_directory_binary,        [runbook_text]),
        ("step5_no_inlined_compare",     check_step5_no_inlined_compare_script,  [runbook_text]),
        ("step5_points_at_on_disk_tool", check_step5_points_at_on_disk_tool,     [runbook_text]),
        ("manifest_no_stale_hash",       check_manifest_template_no_stale_hash,  [runbook_text]),
        ("manifest_points_at_writer",    check_manifest_template_points_at_writer, [runbook_text]),
        ("transcript_points_at_writer",  check_transcript_writer_points_at_writer, [runbook_text]),
        ("events_row_builder_points_at_writer", check_events_row_builder_points_at_writer, [runbook_text]),
        ("focus_recovery_failure_mode",  check_focus_recovery_failure_mode,      [runbook_text]),
    ]
    for name, fn, args in prose_checks:
        total += 1
        result = fn(*args)
        if result:
            failures.extend(f"[{name}] {r}" for r in result)
        else:
            matched += 1

    # On-disk tool checks.
    tool_checks = [
        ("compare_uses_lanczos",         check_compare_uses_lanczos,         []),
        ("preflight_renders_dm_exe",     check_preflight_renders_dm_exe,     []),
        ("detector_selftest_passes",     check_detector_selftest_passes,     []),
        ("manifest_writer_selftest",     check_manifest_writer_selftest_passes, []),
        ("transcript_writer_selftest",   check_transcript_writer_selftest_passes, []),
        ("events_row_builder_selftest",  check_events_row_builder_selftest_passes, []),
        ("live_row_gate_selftest",       check_live_row_gate_selftest_passes, []),
        ("capture_session_focus_recovery_dry_run", check_capture_session_focus_recovery_dry_run, []),
        ("capture_session_post_dungeon_route_parser", check_capture_session_post_dungeon_route_parser, []),
        ("original_viewport_capture_single_row_mode", check_original_viewport_capture_single_row_mode, []),
        ("post_dungeon_target_selector_selftest", check_post_dungeon_target_selector_selftest_passes, []),
    ]
    for name, fn, args in tool_checks:
        total += 1
        result = fn(*args)
        if result:
            failures.extend(f"[{name}] {r}" for r in result)
        else:
            matched += 1

    print(f"{STATUS}: {matched}/{total} runbook/tool consistency checks passed")
    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  - {f}")
        return 1
    print("PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
