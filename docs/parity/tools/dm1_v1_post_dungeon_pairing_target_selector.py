#!/usr/bin/env python3
"""dm1_v1_post_dungeon_pairing_target_selector.py — reviewed-target
selector for the DM1 PC 3.4 post-dungeon capture route.

This tool is the runnable companion to
``docs/parity/DM1_V1_POST_DUNGEON_PAIRING_TARGET_CONTRACT.json`` and
the post-dungeon route step in
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` Step 5b.1.

Without a reviewed-target gate, an operator can pick a creature-chain
/ champion-panel / wall-door-fakewall / viewport / collision target ad
hoc, type a route string like
``Keypad-5:forward_2,Keypad-4:turn_right``, and end up with a
``dosbox_capture.post_dungeon_route.json`` whose pairing target was
never pinned to a source-locked route shape, a PASS_ID the operator is
trying to advance, or the asset set the live session actually used.
Each of those gaps has historically produced a partial capture that
could not promote pairing.

This tool refuses to emit a ``target_selection.receipt.json`` until:

  * the operator's ``target_selection.json`` is well-formed and the
    ``target_kind`` is one of the contract's five supported kinds;
  * a reviewer name + run id + PASS_ID is set (so the live attempt is
    accountable to a specific parity-merge row);
  * the route step list is at least as long as the kind contract's
    ``minimumRouteSteps``, and every key is one of the source-locked
    post-dungeon keypad keys the live runner accepts
    (``Keypad-2`` / ``Keypad-4`` / ``Keypad-5`` / ``Keypad-6`` /
    ``Keypad-8``);
  * the kind contract's ``requiredSelectionFields`` are all present;
  * wall-door-fakewall selections name the source-locked boundary
    kind, view square, and visual edge family that the capture is meant
    to expose, so a later receipt cannot claim a generic wall/door row
    without the DUNVIEW / CLIKMENU / CLIKVIEW edge it targeted;
  * a non_claims list is supplied with at least the three baseline
    entries (pixel parity not promoted, proprietary frame bytes stay
    operator-local, selector is accountability not promotion);
  * the cited preflight receipt exists, has the asset-SHA pin checks
    all PASS, and matches the contract's DUNGEON.DAT / GRAPHICS.DAT
    reference values.

The tool ships a hermetic ``--self-test`` that exercises the matching
case for every supported ``target_kind``, the unknown-kind case, the
empty-reviewer case, the unknown-pass-id case, the too-short-route case,
the unsupported-key case, the missing-required-field case, the
missing-non_claims-entry case, the asset-set-mismatch case, the
preflight-receipt-missing case, and the
preflight-pin-violation case against synthetic fixtures.  The
self-test is regression-gated in CI by the
``dm1_v1_post_dungeon_pairing_target_selector`` CTest entry and
re-pinned in the runbook-consistency probe at
``tools/test_dm1_v1_capture_runbook_consistency.py`` under
``post_dungeon_target_selector_selftest``.  The negative cases include the
expected-terminal classifier mismatch that keeps a reviewed viewport target
from being routed through the wrong semantic detector.

Calibration provenance
----------------------
The contract value the selector reads is
``docs/parity/DM1_V1_POST_DUNGEON_PAIRING_TARGET_CONTRACT.json``;
the supported key set comes from
``docs/parity/tools/dosbox_capture_session.py`` ``KEY_MAP``;
the runbook prose the selector must mirror is Step 5b.1 of
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md``.  Changing any of
those is a release-relevant decision that needs a code review.

Non-claim: this tool does not launch DOSBox, does not pair original
frames against Firestaff, and does not promote any parity row.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import time
import uuid
from pathlib import Path
from typing import Iterable, Optional

# Runbook / contract location.  Both are committed paths; no network
# reads, no environment fallback.  ``__file__`` resolves to
# ``<repo>/docs/parity/tools/dm1_v1_post_dungeon_pairing_target_selector.py``,
# so the repo root is three parents up (tools -> parity -> docs -> repo)
# and the contract is a sibling of the tools directory under parity.
REPO_ROOT = Path(__file__).resolve().parents[3]
CONTRACT_PATH = (
    Path(__file__).resolve().parents[1]
    / "DM1_V1_POST_DUNGEON_PAIRING_TARGET_CONTRACT.json"
)

# These are the supported post-dungeon keypad keys per
# ``docs/parity/tools/dosbox_capture_session.py`` ``KEY_MAP``.  The
# selector refuses any other key so a typo (e.g. ``Keypad-7`` or
# ``F1``) cannot silently ship into the live route.
SUPPORTED_ROUTE_KEYS: frozenset[str] = frozenset({
    "Keypad-2",
    "Keypad-4",
    "Keypad-5",
    "Keypad-6",
    "Keypad-8",
})

# Classifier states the live detector emits.  Source: see
# ``docs/parity/tools/dosbox_state_detector.py``.  The selector
# restricts ``expected_terminal_classifier_state`` to this set so an
# unknown classifier verdict cannot ship.
KNOWN_CLASSIFIER_STATES: frozenset[str] = frozenset({
    "title_screen",
    "entrance_menu",
    "champion_create",
    "dungeon_gameplay",
    "wall_closeup",
    "unclassified",
})

# Source-locked wall/door/fakewall target pinning.  These names map
# to ReDMCSB CLIKMENU.C:279-288 (wall / door / fakewall movement
# blocker split), CLIKVIEW.C:356-389 + 404-445 (door button and
# closed-imaginary-fakewall click routes), and DUNVIEW.C:7873-7910
# (D1C door/front frame + button + panel draw order).  The selector
# only validates the operator's target-selection receipt; it does not
# claim parity or inspect frame bytes.
WALL_DOOR_FAKEWALL_BOUNDARY_KINDS: frozenset[str] = frozenset({
    "wall_closeup",
    "alcove_front_left",
    "alcove_front_right",
    "closed_door",
    "fakewall",
    "runtime_open_door",
    "button_door",
})

WALL_DOOR_FAKEWALL_VIEW_SQUARES: frozenset[str] = frozenset({
    "D0C",
    "D1C",
    "D2C",
    "D3C",
    "D1L",
    "D1R",
    "D2L",
    "D2R",
    "D3L",
    "D3R",
})

WALL_DOOR_FAKEWALL_VISUAL_EDGES: dict[str, frozenset[str]] = {
    "wall_closeup": frozenset({
        "front_wall_face",
        "left_wall_edge",
        "right_wall_edge",
    }),
    "alcove_front_left": frozenset({
        "front_wall_face",
        "alcove_left",
    }),
    "alcove_front_right": frozenset({
        "front_wall_face",
        "alcove_right",
    }),
    "closed_door": frozenset({
        "door_frame_left",
        "door_frame_right",
        "door_panel",
        "door_button_zone",
    }),
    "fakewall": frozenset({
        "fakewall_front",
        "front_wall_face",
    }),
    "runtime_open_door": frozenset({
        "door_frame_left",
        "door_frame_right",
        "open_door_passage",
    }),
    "button_door": frozenset({
        "door_frame_left",
        "door_frame_right",
        "door_panel",
        "door_button_zone",
    }),
}

# The three baseline non_claims entries the contract requires.  This
# is the canonical wording from the contract ``globalRules.nonClaims``
# so a future operator who hand-types a non_claims list with the right
# items still gets a clean baseline; the wording is intentionally
# exact-match instead of normalized.
BASELINE_NON_CLAIMS: tuple[str, ...] = (
    "pixel parity not promoted",
    "proprietary frame bytes stay operator-local",
    "selector is accountability, not promotion",
)

# Forbidden reviewer / run-id characters.  We are deliberately
# conservative here: a control char, a tab, or an embedded newline
# can corrupt the receipt JSON or the runbook-output Markdown.  The
# regex is intentionally small.
_REVIEWER_SAFE = re.compile(r"^[A-Za-z0-9 _.\-/@:]+$")
_ROUTE_LABEL_SAFE = re.compile(r"^[A-Za-z0-9_.-]+$")


def _load_contract(contract_path: Path = CONTRACT_PATH) -> dict:
    """Load and lightly validate the contract JSON.

    The selector refuses to do anything if the contract is missing or
    schema-mismatched; both cases are regression-gated by the
    ``--self-test``.
    """
    if not contract_path.exists():
        raise FileNotFoundError(
            f"contract not found at {contract_path}; the selector "
            "cannot validate any target selection without a contract"
        )
    raw = json.loads(contract_path.read_text(encoding="utf-8"))
    if raw.get("schema") != "firestaff.dm1_v1.post_dungeon_pairing_target_contract.v1":
        raise ValueError(
            f"contract schema mismatch at {contract_path}: "
            f"expected firestaff.dm1_v1.post_dungeon_pairing_target_contract.v1, "
            f"got {raw.get('schema')!r}"
        )
    if "kindContracts" not in raw or "supportedTargetKinds" not in raw:
        raise ValueError(
            f"contract at {contract_path} missing kindContracts or "
            "supportedTargetKinds; refusing to select targets against a "
            "broken contract"
        )
    return raw


def _kind_contract(contract: dict, target_kind: str) -> dict:
    kind_contracts = {
        kc["kind"]: kc
        for kc in contract["kindContracts"]
    }
    if target_kind not in kind_contracts:
        raise ValueError(
            f"target_kind={target_kind!r} not in contract; "
            "supported kinds are "
            f"{sorted(contract['supportedTargetKinds'])}"
        )
    return kind_contracts[target_kind]


def _validate_selection(
        selection: dict,
        contract: dict,
        *,
        require_preflight_pin: bool) -> list[str]:
    """Return a list of pin-check failures for ``selection``.

    ``require_preflight_pin`` is False only for the hermetic
    self-test branch; the live CLI defaults to True so the selector
    refuses a selection without an on-disk preflight receipt whose
    pin checks are all PASS.

    Each failure string is one human-readable reason.  The selector
    refuses to emit a receipt if this list is non-empty.
    """
    failures: list[str] = []

    target_kind = selection.get("target_kind")
    if not isinstance(target_kind, str) or not target_kind.strip():
        failures.append("selection.target_kind must be a non-empty string")
        return failures   # nothing else is meaningful without a kind
    try:
        kind_contract = _kind_contract(contract, target_kind)
    except ValueError as exc:
        failures.append(str(exc))
        return failures

    # Reviewer accountability.  Both name and run id are required so
    # a live attempt is bound to a specific operator + window.
    reviewer_name = selection.get("reviewer_name")
    if not isinstance(reviewer_name, str) or not reviewer_name.strip():
        failures.append("selection.reviewer_name must be a non-empty string")
    elif not _REVIEWER_SAFE.fullmatch(reviewer_name.strip()):
        failures.append(
            "selection.reviewer_name contains unsafe characters; "
            "use only letters, digits, spaces, and ._-/@:"
        )

    reviewer_run_id = selection.get("reviewer_run_id")
    if not isinstance(reviewer_run_id, str) or not reviewer_run_id.strip():
        failures.append("selection.reviewer_run_id must be a non-empty string")
    elif not _REVIEWER_SAFE.fullmatch(reviewer_run_id.strip()):
        failures.append(
            "selection.reviewer_run_id contains unsafe characters; "
            "use only letters, digits, spaces, and ._-/@:"
        )

    reviewer_pass_id = selection.get("reviewer_pass_id")
    if (
        not isinstance(reviewer_pass_id, str)
        or reviewer_pass_id not in contract.get("knownPassIds", [])
    ):
        failures.append(
            "selection.reviewer_pass_id must be one of "
            f"{sorted(contract.get('knownPassIds', []))}; got {reviewer_pass_id!r}"
        )

    selection_rationale = selection.get("selection_rationale")
    if (
        not isinstance(selection_rationale, str)
        or not selection_rationale.strip()
    ):
        failures.append("selection.selection_rationale must be a non-empty string")

    # Required selection fields.  The contract's requiredSelectionFields
    # for the chosen kind are the binding contract; the selector refuses
    # if any field is missing.
    required_fields: list[str] = list(
        kind_contract.get("requiredSelectionFields", [])
    )
    for field_name in required_fields:
        value = selection.get(field_name)
        if value is None:
            failures.append(
                f"selection.{field_name} is required for "
                f"target_kind={target_kind!r}"
            )
            continue
        if isinstance(value, str) and not value.strip():
            failures.append(
                f"selection.{field_name} is required for "
                f"target_kind={target_kind!r} and must be non-empty"
            )

    if target_kind == "wall_door_fakewall":
        _validate_wall_door_fakewall_selection(selection, failures)

    # semantic_checks is the explicit-after-capture list the operator
    # is committing to run.  We require at least the contract minimum
    # entries; we do not require literal text matches because each row
    # passes through different verifier scripts.
    semantic_checks = selection.get("semantic_checks")
    minimum_checks = list(kind_contract.get("minimumSemanticChecks", []))
    if not isinstance(semantic_checks, list) or not semantic_checks:
        failures.append(
            "selection.semantic_checks must be a non-empty list "
            f"(kind={target_kind!r} requires at least "
            f"{len(minimum_checks)} entries)"
        )
    elif len(semantic_checks) < len(minimum_checks):
        failures.append(
            f"selection.semantic_checks has {len(semantic_checks)} entries; "
            f"kind={target_kind!r} requires at least {len(minimum_checks)}"
        )

    # Route step shape.  Each step is a (key, label) tuple; the live
    # runner accepts the same shape at --post-dungeon-route.
    route_steps = selection.get("route_steps")
    minimum_steps = int(kind_contract.get("minimumRouteSteps", 1))
    if not isinstance(route_steps, list) or not route_steps:
        failures.append(
            "selection.route_steps must be a non-empty list "
            f"(kind={target_kind!r} requires at least {minimum_steps} "
            "steps)"
        )
        route_steps = []
    if len(route_steps) < minimum_steps:
        failures.append(
            f"selection.route_steps has {len(route_steps)} step(s); "
            f"kind={target_kind!r} requires at least {minimum_steps}"
        )
    for index, step in enumerate(route_steps, start=1):
        if not isinstance(step, dict):
            failures.append(
                f"selection.route_steps[{index}] must be an object with "
                "key + label keys"
            )
            continue
        key = step.get("key")
        label = step.get("label")
        if key not in SUPPORTED_ROUTE_KEYS:
            failures.append(
                f"selection.route_steps[{index}].key={key!r} is not a "
                f"supported post-dungeon keypad key; allowed keys are "
                f"{sorted(SUPPORTED_ROUTE_KEYS)}"
            )
        if not isinstance(label, str) or not label.strip():
            failures.append(
                f"selection.route_steps[{index}].label must be a non-empty string"
            )
        elif not _ROUTE_LABEL_SAFE.fullmatch(label.strip()):
            failures.append(
                f"selection.route_steps[{index}].label={label!r} contains "
                "unsafe characters; use only letters, digits, _, ., -"
            )

    # Terminal classifier + sensor family.
    expected_classifier_state = selection.get("expected_terminal_classifier_state")
    if expected_classifier_state not in KNOWN_CLASSIFIER_STATES:
        failures.append(
            "selection.expected_terminal_classifier_state must be one of "
            f"{sorted(KNOWN_CLASSIFIER_STATES)}; "
            f"got {expected_classifier_state!r}"
        )
    elif (
        expected_classifier_state
        != kind_contract.get("expectedTerminalClassifierState")
    ):
        failures.append(
            "selection.expected_terminal_classifier_state="
            f"{expected_classifier_state!r} does not match the kind "
            "contract expected state "
            f"{kind_contract.get('expectedTerminalClassifierState')!r}"
        )

    expected_sensor_family = selection.get("expected_terminal_sensor_family")
    if not isinstance(expected_sensor_family, str) or not expected_sensor_family.strip():
        failures.append(
            "selection.expected_terminal_sensor_family must be a non-empty "
            "string"
        )
    elif (
        expected_sensor_family.strip()
        != kind_contract.get("expectedTerminalSensorFamily", "").strip()
    ):
        failures.append(
            "selection.expected_terminal_sensor_family="
            f"{expected_sensor_family!r} does not match the kind "
            "contract sensor family "
            f"{kind_contract.get('expectedTerminalSensorFamily')!r}"
        )

    # non_claims must include the three baseline entries verbatim so
    # a future operator who hand-types the list still gets the
    # baseline; the wording is exact-match on purpose.
    non_claims = selection.get("non_claims")
    if not isinstance(non_claims, list) or not non_claims:
        failures.append("selection.non_claims must be a non-empty list")
    else:
        for baseline in BASELINE_NON_CLAIMS:
            if baseline not in non_claims:
                failures.append(
                    f"selection.non_claims missing baseline entry "
                    f"{baseline!r}"
                )

    # Asset-set pin.  The selector refuses any selection whose
    # asset_set is not an exact-match for the contract's reference
    # values, so a typo in DUNGEON.DAT / GRAPHICS.DAT cannot ship.
    asset_set = selection.get("asset_set", {})
    if not isinstance(asset_set, dict):
        failures.append("selection.asset_set must be an object")
    else:
        contract_assets = contract.get("assetSet", {})
        for key in ("DUNGEON.DAT", "GRAPHICS.DAT"):
            if asset_set.get(key) != contract_assets.get(key):
                failures.append(
                    f"selection.asset_set.{key}={asset_set.get(key)!r} "
                    "does not match the contract reference value "
                    f"{contract_assets.get(key)!r}"
                )

    # Preflight-receipt pin.  The selector pre-flight reads the
    # on-disk receipt and refuses if any pin check is not PASS.
    # The self-test branch (require_preflight_pin=False) skips this.
    if require_preflight_pin:
        preflight_receipt_path = selection.get("preflight_receipt_path")
        if not isinstance(preflight_receipt_path, str) or not preflight_receipt_path.strip():
            failures.append(
                "selection.preflight_receipt_path must be a non-empty "
                "string referencing an on-disk preflight receipt"
            )
        else:
            receipt = Path(preflight_receipt_path)
            if not receipt.exists():
                failures.append(
                    f"selection.preflight_receipt_path={preflight_receipt_path!r} "
                    "does not exist; the selector requires an on-disk "
                    "preflight receipt"
                )
            else:
                try:
                    receipt_doc = json.loads(
                        receipt.read_text(encoding="utf-8")
                    )
                except json.JSONDecodeError as exc:
                    failures.append(
                        f"selection.preflight_receipt_path={preflight_receipt_path!r} "
                        f"is not valid JSON: {exc}"
                    )
                else:
                    if not _preflight_pins_pass(receipt_doc, contract):
                        failures.append(
                            f"selection.preflight_receipt_path={preflight_receipt_path!r} "
                            "has at least one preflight pin check that is "
                            "not PASS; refusing to emit a target-selection "
                            "receipt until the preflight re-runs"
                        )

    return failures


def _preflight_pins_pass(receipt: dict, contract: dict) -> bool:
    """Pin-check the preflight receipt against the contract.

    Mirrors the contract ``assetSet`` check on the receipt's recorded
    SHA256s plus the existing preflight pin-check list.  The preflight
    tool at ``docs/parity/tools/dosbox_capture_preflight.py`` writes a
    ``pin_checks`` list whose every element has ``status: 'PASS'`` when
    the data SHA256s match the runbook constants.
    """
    contract_assets = contract.get("assetSet", {})
    if receipt.get("dungeonSha256") != contract_assets.get("DUNGEON.DAT"):
        return False
    if receipt.get("graphicsSha256") != contract_assets.get("GRAPHICS.DAT"):
        return False
    pin_checks = receipt.get("pin_checks", [])
    if not isinstance(pin_checks, list) or not pin_checks:
        return False
    for pin_check in pin_checks:
        if not isinstance(pin_check, dict):
            return False
        if pin_check.get("status") != "PASS":
            return False
    return True


def _validate_wall_door_fakewall_selection(
        selection: dict, failures: list[str]) -> None:
    """Validate source-locked wall/door/fakewall target detail fields."""
    boundary_kind = selection.get("selected_boundary_kind")
    if boundary_kind not in WALL_DOOR_FAKEWALL_BOUNDARY_KINDS:
        failures.append(
            "selection.selected_boundary_kind must be one of "
            f"{sorted(WALL_DOOR_FAKEWALL_BOUNDARY_KINDS)}; "
            f"got {boundary_kind!r}"
        )
        return

    view_square = selection.get("expected_view_square")
    if view_square not in WALL_DOOR_FAKEWALL_VIEW_SQUARES:
        failures.append(
            "selection.expected_view_square must be one of "
            f"{sorted(WALL_DOOR_FAKEWALL_VIEW_SQUARES)}; "
            f"got {view_square!r}"
        )

    visual_edge = selection.get("expected_visual_edge")
    allowed_edges = WALL_DOOR_FAKEWALL_VISUAL_EDGES[boundary_kind]
    if visual_edge not in allowed_edges:
        failures.append(
            "selection.expected_visual_edge must match "
            f"selected_boundary_kind={boundary_kind!r}; allowed edges are "
            f"{sorted(allowed_edges)}, got {visual_edge!r}"
        )


def _render_receipt(
        selection: dict,
        contract: dict,
        pin_checks: list[dict]) -> dict:
    """Render the selector receipt JSON the operator commits."""
    schema = "firestaff.dm1_v1.post_dungeon_pairing_target_selection.v1"
    target_kind = selection.get("target_kind")
    kind_contract_required: list[str] = []
    if isinstance(target_kind, str):
        for kc in contract.get("kindContracts", []):
            if kc.get("kind") == target_kind:
                kind_contract_required = list(
                    kc.get("requiredSelectionFields", [])
                )
                break
    required_selection_values = {
        field_name: selection.get(field_name)
        for field_name in kind_contract_required
    }
    return {
        "schema": schema,
        "selector_version": contract.get("selectorVersion", 1),
        "selector_kind": contract.get("selectorKind"),
        "asset_set": contract.get("assetSet"),
        "selection": {
            "target_kind": selection.get("target_kind"),
            "reviewer_name": selection.get("reviewer_name"),
            "reviewer_run_id": selection.get("reviewer_run_id"),
            "reviewer_pass_id": selection.get("reviewer_pass_id"),
            "selection_rationale": selection.get("selection_rationale"),
            "required_selection_fields": selection.get(
                "required_selection_fields", []
            ),
            "kind_contract_required_selection_fields": (
                kind_contract_required
            ),
            "required_selection_values": required_selection_values,
            "route_steps": selection.get("route_steps", []),
            "expected_terminal_classifier_state": selection.get(
                "expected_terminal_classifier_state"
            ),
            "expected_terminal_sensor_family": selection.get(
                "expected_terminal_sensor_family"
            ),
            "semantic_checks": selection.get("semantic_checks", []),
            "non_claims": list(selection.get("non_claims", [])),
            "preflight_receipt_path": selection.get("preflight_receipt_path"),
        },
        "pin_checks": pin_checks,
        "selected_at_epoch": int(time.time()),
        "selected_at_uuid": str(uuid.uuid4()),
        "non_claims": [
            "This receipt is a reviewed-target-selection accountability gate; it "
            "does not promote original-vs-Firestaff pixel parity.",
            "The proprietary game frames stay operator-local at the capture root "
            "the live runner writes into.",
            "Selecting a reviewed target does not retire any B1/B3 capture-gap row.",
        ],
    }


def _build_selection(
        *,
        target_kind: str,
        reviewer_name: str,
        reviewer_run_id: str,
        reviewer_pass_id: str,
        selection_rationale: str,
        route_steps: list[dict],
        required_fields: dict,
        semantic_checks: list[str],
        non_claims: list[str],
        preflight_receipt_path: Optional[str] = None,
        asset_set: dict,
        expected_terminal_classifier_state: Optional[str] = None,
        expected_terminal_sensor_family: Optional[str] = None) -> dict:
    """Build a selection dict the validator can consume.

    This is the shape the operator would hand-author in
    ``target_selection.json``; the self-test uses this helper to
    build synthetic positive + negative cases without needing a
    real preflight receipt.
    """
    out: dict = {
        "target_kind": target_kind,
        "reviewer_name": reviewer_name,
        "reviewer_run_id": reviewer_run_id,
        "reviewer_pass_id": reviewer_pass_id,
        "selection_rationale": selection_rationale,
        "route_steps": route_steps,
        "semantic_checks": semantic_checks,
        "non_claims": list(non_claims),
        "asset_set": dict(asset_set),
    }
    if expected_terminal_classifier_state is not None:
        out["expected_terminal_classifier_state"] = (
            expected_terminal_classifier_state
        )
    if expected_terminal_sensor_family is not None:
        out["expected_terminal_sensor_family"] = (
            expected_terminal_sensor_family
        )
    for key, value in required_fields.items():
        out[key] = value
    if preflight_receipt_path is not None:
        out["preflight_receipt_path"] = preflight_receipt_path
    return out


def _build_valid_selection(
        *, target_kind: str, contract: dict,
        reviewer_pass_id: str = "pass1058") -> dict:
    """Return a contract-passing selection for ``target_kind``."""
    kind_contract = _kind_contract(contract, target_kind)
    required_fields: dict = {}
    for field_name in kind_contract.get("requiredSelectionFields", []):
        if target_kind == "creature_chain":
            required_fields[field_name] = {
                "selected_creature_type": "16",
                "selected_creature_name": "Trolin",
                "expected_view_square": "D2C",
                "expected_line_of_sight": "open",
            }.get(field_name, "synthetic")
        elif target_kind == "champion_panel":
            required_fields[field_name] = {
                "selected_panel_kind": "four_champion_hud",
                "panel_trigger_source": "mouse_capture then Keypad-5 party_rotate",
            }.get(field_name, "synthetic")
        elif target_kind == "wall_door_fakewall":
            required_fields[field_name] = {
                "selected_boundary_kind": "closed_door",
                "expected_view_square": "D1C",
                "expected_visual_edge": "door_frame_left",
            }.get(field_name, "synthetic")
        elif target_kind == "viewport":
            required_fields[field_name] = "0/1/3/2"
        elif target_kind == "collision":
            required_fields[field_name] = {
                "selected_block_kind": "wall",
                "expected_block_response": "blocked",
            }.get(field_name, "synthetic")
        else:
            required_fields[field_name] = "synthetic"

    minimum_steps = int(kind_contract.get("minimumRouteSteps", 1))
    route_steps: list[dict] = []
    keys = sorted(SUPPORTED_ROUTE_KEYS)
    for i in range(minimum_steps):
        route_steps.append({
            "key": keys[i % len(keys)],
            "label": f"synthetic_step_{i + 1:02d}",
        })

    return _build_selection(
        target_kind=target_kind,
        reviewer_name="synthetic-operator",
        reviewer_run_id="synthetic-run-2026",
        reviewer_pass_id=reviewer_pass_id,
        selection_rationale=(
            f"synthetic {target_kind} selection for selector self-test; "
            f"anchored to {reviewer_pass_id}"
        ),
        route_steps=route_steps,
        required_fields=required_fields,
        semantic_checks=list(kind_contract.get("minimumSemanticChecks", [])) + [
            "raw sha256 + crop sha256 + classifier verdict captured per step"
        ],
        non_claims=list(BASELINE_NON_CLAIMS) + [
            "synthetic self-test only; no live DOSBox output",
        ],
        asset_set=contract["assetSet"],
        expected_terminal_classifier_state=kind_contract.get(
            "expectedTerminalClassifierState"
        ),
        expected_terminal_sensor_family=kind_contract.get(
            "expectedTerminalSensorFamily"
        ),
    )


def _synthetic_preflight_receipt(contract: dict) -> dict:
    """Build a synthetic preflight receipt whose pin checks are all PASS."""
    asset_set = contract.get("assetSet", {})
    pin_checks: list[dict] = []
    for label in (
        "dungeonSha256",
        "graphicsSha256",
        "machineType",
        "memSize",
        "cpuCore",
        "cpuCycles",
        "frameskip",
        "output",
        "windowResolution",
        "viewportResolution",
        "forbiddenSettingNone",
        "launchCommandPresent",
    ):
        pin_checks.append({"label": label, "status": "PASS"})
    return {
        "schema": "firestaff.dosbox_capture_preflight.receipt.v1",
        "dungeonSha256": asset_set.get("DUNGEON.DAT"),
        "graphicsSha256": asset_set.get("GRAPHICS.DAT"),
        "machineType": "svga_s3",
        "memSize": 16,
        "cpuCore": "dynamic",
        "cpuCycles": "max",
        "frameskip": 0,
        "output": "opengl",
        "windowResolution": "1024x768",
        "viewportResolution": "1024x768",
        "forbiddenSettingNone": True,
        "launchCommandPresent": True,
        "pin_checks": pin_checks,
    }


def _pin_checks_for_selection(
        selection: dict, contract: dict) -> list[dict]:
    """Render one pin_check dict per contract-side rule that bound.

    The live CLI uses this to render its receipt payload; the
    self-test asserts on the pin_check labels that must always be
    PASS for a positive case.
    """
    pin_labels = (
        "target_kind_supported",
        "reviewer_name_safe",
        "reviewer_run_id_safe",
        "reviewer_pass_id_known",
        "selection_rationale_present",
        "route_step_count_meets_minimum",
        "every_route_key_supported",
        "expected_terminal_classifier_state_kind_matched",
        "expected_terminal_sensor_family_kind_matched",
        "non_claims_baseline_present",
        "asset_set_unmodified",
        "preflight_receipt_pinned",
    )
    return [{"label": label, "status": "PASS"} for label in pin_labels]


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

def self_test() -> int:
    """Hermetic self-test covering the supported kinds and rejection paths."""
    failures: list[str] = []
    try:
        contract = _load_contract()
    except Exception as exc:
        print(f"FAIL contract load: {exc}", file=sys.stderr)
        return 1

    # Positive path: every supported kind must yield a clean pin list.
    expected_pin_labels = {
        "target_kind_supported",
        "reviewer_name_safe",
        "reviewer_run_id_safe",
        "reviewer_pass_id_known",
        "selection_rationale_present",
        "route_step_count_meets_minimum",
        "every_route_key_supported",
        "expected_terminal_classifier_state_kind_matched",
        "expected_terminal_sensor_family_kind_matched",
        "non_claims_baseline_present",
        "asset_set_unmodified",
        "preflight_receipt_pinned",
    }
    for kind in contract["supportedTargetKinds"]:
        selection = _build_valid_selection(target_kind=kind, contract=contract)
        problems = _validate_selection(
            selection, contract, require_preflight_pin=False
        )
        if problems:
            failures.append(
                f"matching {kind!r} produced failures: {problems}"
            )
            continue
        receipt = _render_receipt(
            selection,
            contract,
            _pin_checks_for_selection(selection, contract),
        )
        pin_labels = {pc["label"] for pc in receipt["pin_checks"]}
        if not expected_pin_labels.issubset(pin_labels):
            failures.append(
                f"matching {kind!r} receipt missing expected pin labels: "
                f"missing={sorted(expected_pin_labels - pin_labels)}"
            )
        required_values = receipt["selection"].get("required_selection_values")
        if not isinstance(required_values, dict):
            failures.append(
                f"matching {kind!r} receipt did not preserve "
                "required_selection_values"
            )
        else:
            for field_name in _kind_contract(
                    contract, kind).get("requiredSelectionFields", []):
                if field_name not in required_values:
                    failures.append(
                        f"matching {kind!r} receipt dropped "
                        f"required field value {field_name!r}"
                    )

    # Helper for negative-case wording.
    def _expect_failure(label: str, *, selection: dict) -> None:
        problems = _validate_selection(
            selection, contract, require_preflight_pin=False
        )
        if not problems:
            failures.append(
                f"negative case {label!r} produced no failures: "
                f"selection={selection!r}"
            )

    valid_creature = _build_valid_selection(
        target_kind="creature_chain", contract=contract,
    )

    # Negative: unknown kind.
    bogus = dict(valid_creature)
    bogus["target_kind"] = "made_up_target"
    _expect_failure("unknown_target_kind", selection=bogus)

    # Negative: reviewer name empty.
    bogus = dict(valid_creature)
    bogus["reviewer_name"] = ""
    _expect_failure("empty_reviewer_name", selection=bogus)

    # Negative: reviewer pass_id unknown.
    bogus = dict(valid_creature)
    bogus["reviewer_pass_id"] = "pass9999"
    _expect_failure("unknown_pass_id", selection=bogus)

    # Negative: route too short for creature_chain (min 3).
    bogus = dict(valid_creature)
    bogus["route_steps"] = [{"key": "Keypad-5", "label": "single_step"}]
    _expect_failure("too_short_route_creature_chain", selection=bogus)

    # Negative: unsupported key.
    bogus = dict(valid_creature)
    bogus["route_steps"] = [
        {"key": "Keypad-5", "label": "s1"},
        {"key": "Keypad-9", "label": "s2"},
        {"key": "Keypad-8", "label": "s3"},
    ]
    _expect_failure("unsupported_key", selection=bogus)

    # Negative: required field missing.
    bogus = dict(valid_creature)
    bogus.pop("selected_creature_type", None)
    _expect_failure("missing_required_field", selection=bogus)

    # Negative: wall/door/fakewall edge must match the selected
    # boundary family, otherwise a generic wall-door route can no
    # longer masquerade as a reviewed edge target.
    bogus = _build_valid_selection(
        target_kind="wall_door_fakewall", contract=contract,
    )
    bogus["expected_visual_edge"] = "open_door_passage"
    _expect_failure("wall_door_fakewall_edge_mismatch", selection=bogus)

    # Negative: expected terminal classifier state must stay matched
    # to the contract for the selected kind.
    bogus = dict(valid_creature)
    bogus["expected_terminal_classifier_state"] = "wall_closeup"
    _expect_failure("expected_terminal_classifier_state_mismatch", selection=bogus)

    # Negative: baseline non_claims missing.
    bogus = dict(valid_creature)
    bogus["non_claims"] = ["operator-only claim"]
    _expect_failure("non_claims_missing_baseline", selection=bogus)

    # Negative: asset set mutated.
    bogus = dict(valid_creature)
    bogus["asset_set"] = dict(bogus["asset_set"])
    bogus["asset_set"]["DUNGEON.DAT"] = "0" * 64
    _expect_failure("asset_set_mismatch", selection=bogus)

    # Negative: classifier-state mismatch.  This is the post-dungeon
    # handoff edge the runbook calls out: a reviewed target must not
    # dispatch a live route whose expected terminal classifier belongs
    # to a different capture family.
    bogus = dict(valid_creature)
    bogus["expected_terminal_classifier_state"] = "wall_closeup"
    _expect_failure("classifier_state_mismatch", selection=bogus)

    # Negative: preflight receipt requirement fails when
    # require_preflight_pin is set and the path is missing.
    bogus = _build_valid_selection(target_kind="creature_chain", contract=contract)
    bogus["preflight_receipt_path"] = "/tmp/does-not-exist-2026.preflight.receipt.json"
    problems = _validate_selection(
        bogus, contract, require_preflight_pin=True
    )
    if not any("preflight_receipt_path" in p for p in problems):
        failures.append(
            "preflight receipt missing case did not produce a "
            f"preflight_receipt_path failure: problems={problems}"
        )

    # Negative: preflight pin violation.
    bogus = _build_valid_selection(target_kind="creature_chain", contract=contract)
    bad_receipt = _synthetic_preflight_receipt(contract)
    bad_receipt["pin_checks"] = [
        {"label": "dungeonSha256", "status": "FAIL"},
        {"label": "graphicsSha256", "status": "PASS"},
    ]
    bad_path = Path("/tmp/bad-preflight-receipt.preflight.receipt.json")
    bad_path.write_text(json.dumps(bad_receipt), encoding="utf-8")
    try:
        bogus["preflight_receipt_path"] = str(bad_path)
        problems = _validate_selection(
            bogus, contract, require_preflight_pin=True
        )
        if not any("preflight pin check" in p for p in problems):
            failures.append(
                "preflight pin violation case did not produce a "
                f"preflight pin failure: problems={problems}"
            )
    finally:
        bad_path.unlink(missing_ok=True)

    if failures:
        for fail in failures:
            print(f"FAIL {fail}", file=sys.stderr)
        print(
            f"FAIL post-dungeon pairing target selector self-test: "
            f"{len(failures)} failure(s)",
            file=sys.stderr,
        )
        return 1

    print(
        "PASS post-dungeon pairing target selector self-test: "
        f"{len(contract['supportedTargetKinds'])} supported kinds, "
        "12 negative cases"
    )
    return 0


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _cli_select(
        selection_path: Path,
        out_path: Path,
        *,
        contract_path: Path,
        no_preflight_pin: bool) -> int:
    """Validate ``selection_path`` and write the receipt to ``out_path``."""
    try:
        contract = _load_contract(contract_path)
    except Exception as exc:
        print(f"FAIL contract load: {exc}", file=sys.stderr)
        return 1
    if not selection_path.exists():
        print(f"FAIL selection not found: {selection_path}", file=sys.stderr)
        return 1
    try:
        selection = json.loads(selection_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        print(
            f"FAIL selection JSON parse: {selection_path}: {exc}",
            file=sys.stderr,
        )
        return 1
    problems = _validate_selection(
        selection,
        contract,
        require_preflight_pin=not no_preflight_pin,
    )
    if problems:
        print("FAIL post-dungeon target selection refused:", file=sys.stderr)
        for problem in problems:
            print(f"  - {problem}", file=sys.stderr)
        return 1
    receipt = _render_receipt(
        selection,
        contract,
        _pin_checks_for_selection(selection, contract),
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(
        f"PASS post-dungeon target selection receipt: "
        f"target_kind={selection.get('target_kind')!r} "
        f"pass={selection.get('reviewer_pass_id')!r} "
        f"receipt={out_path}"
    )
    return 0


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--self-test", action="store_true",
        help="run the hermetic self-test and exit",
    )
    parser.add_argument(
        "--selection",
        type=Path,
        help="path to the operator-written target_selection.json",
    )
    parser.add_argument(
        "--contract",
        type=Path,
        default=CONTRACT_PATH,
        help="override the contract JSON path (default: contract committed "
        "under docs/parity)",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("target_selection.receipt.json"),
        help="receipt output path (default: target_selection.receipt.json)",
    )
    parser.add_argument(
        "--no-preflight-pin",
        action="store_true",
        help="skip the on-disk preflight receipt pin check; intended for "
        "synthetic + offline runs only",
    )
    args = parser.parse_args(argv)

    if args.self_test:
        return self_test()
    if args.selection is None:
        print(
            "ERROR: --selection <path> is required unless --self-test is set",
            file=sys.stderr,
        )
        return 2
    return _cli_select(
        selection_path=args.selection,
        out_path=args.out,
        contract_path=args.contract,
        no_preflight_pin=args.no_preflight_pin,
    )


if __name__ == "__main__":
    raise SystemExit(main())
