#!/usr/bin/env python3
"""Small checksum-addressed store helper for Firestaff V2 asset prototypes."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
import tempfile
from pathlib import Path, PurePosixPath
from typing import Any

STORE_ROOT = Path("assets-v2/store")
DIGEST_NAME = "sha256"


def repo_relative(path: Path) -> str:
    if path.is_absolute() or ".." in path.parts:
        raise SystemExit(f"path must stay repository-relative: {path}")
    return path.as_posix()


def require_repository_relative(path: Path, label: str) -> None:
    if path.is_absolute() or ".." in path.parts:
        raise SystemExit(f"{label} must be repository-relative and must not escape the repository: {path}")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def normalized_extension(path: Path) -> str:
    suffix = path.suffix.lower()
    return suffix if suffix else ".bin"


def store_path_for(source: Path, digest: str, store_root: Path) -> Path:
    return store_root / DIGEST_NAME / digest[:2] / f"{digest}{normalized_extension(source)}"


def manifest_snippet(
    *,
    source: Path,
    target: Path,
    digest: str,
    asset_id: str | None,
    family: str | None,
    role: str | None,
) -> dict[str, Any]:
    snippet: dict[str, Any] = {
        "id": asset_id or f"fs.v2.asset.{digest[:16]}",
        "storage": {
            "algorithm": DIGEST_NAME,
            "sha256": digest,
            "path": repo_relative(target),
            "sizeBytes": source.stat().st_size,
        },
        "source": {
            "fileName": source.name,
            "extension": normalized_extension(source),
        },
        "status": "stored",
    }
    if family:
        snippet["family"] = family
    if role:
        snippet["role"] = role
    return snippet


def command_ingest(args: argparse.Namespace) -> int:
    source = Path(args.file)
    if not source.is_file():
        print(f"input file not found: {source}", file=sys.stderr)
        return 2

    store_root = Path(args.store_root)
    require_repository_relative(store_root, "store root")
    digest = sha256_file(source)
    target = store_path_for(source, digest, store_root)
    target.parent.mkdir(parents=True, exist_ok=True)

    if target.exists():
        if sha256_file(target) != digest:
            print(f"refusing to overwrite mismatched store object: {target}", file=sys.stderr)
            return 1
    else:
        shutil.copy2(source, target)

    snippet = manifest_snippet(
        source=source,
        target=target,
        digest=digest,
        asset_id=args.asset_id,
        family=args.family,
        role=args.role,
    )
    rendered = json.dumps(snippet, indent=2, sort_keys=True) + "\n"
    if args.manifest_out:
        out = Path(args.manifest_out)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0


def iter_snippets(payload: Any) -> list[dict[str, Any]]:
    if isinstance(payload, dict) and isinstance(payload.get("assets"), list):
        return payload["assets"]
    if isinstance(payload, dict):
        return [payload]
    if isinstance(payload, list):
        return payload
    raise ValueError("manifest snippet must be an object, an array, or an object with an assets array")


def validate_snippet(snippet: dict[str, Any], store_root: Path) -> list[str]:
    errors: list[str] = []
    storage = snippet.get("storage")
    if not isinstance(storage, dict):
        return ["missing storage object"]

    digest = storage.get("sha256")
    rel_path = storage.get("path")
    size_bytes = storage.get("sizeBytes")
    if not isinstance(digest, str) or len(digest) != 64 or any(c not in "0123456789abcdef" for c in digest):
        errors.append("storage.sha256 must be a lowercase 64-character hex digest")
    if not isinstance(rel_path, str):
        errors.append("storage.path must be a repository-relative string")
        return errors
    if rel_path.startswith("/") or ".." in PurePosixPath(rel_path).parts:
        errors.append("storage.path must not be absolute or escape the repository")
        return errors

    expected_prefix = None
    if isinstance(digest, str) and len(digest) >= 2:
        expected_prefix = f'{store_root.as_posix().rstrip("/")}/{DIGEST_NAME}/{digest[:2]}/'
    if expected_prefix and not rel_path.startswith(expected_prefix):
        errors.append(f"storage.path must start with {expected_prefix}")
    if isinstance(digest, str) and not Path(rel_path).name.startswith(digest):
        errors.append("stored file name must begin with the sha256 digest")

    target = Path(rel_path)
    if not target.is_file():
        errors.append(f"stored file is missing: {rel_path}")
        return errors
    actual_digest = sha256_file(target)
    if actual_digest != digest:
        errors.append(f"stored file checksum mismatch: expected {digest}, got {actual_digest}")
    actual_size = target.stat().st_size
    if size_bytes is not None and size_bytes != actual_size:
        errors.append(f"stored file size mismatch: expected {size_bytes}, got {actual_size}")
    return errors


def command_validate(args: argparse.Namespace) -> int:
    store_root = Path(args.store_root)
    require_repository_relative(store_root, "store root")
    payload = json.loads(Path(args.manifest).read_text(encoding="utf-8"))
    snippets = iter_snippets(payload)
    all_errors: list[str] = []
    for index, snippet in enumerate(snippets):
        label = snippet.get("id", f"snippet[{index}]") if isinstance(snippet, dict) else f"snippet[{index}]"
        if not isinstance(snippet, dict):
            all_errors.append(f"{label}: snippet must be an object")
            continue
        for error in validate_snippet(snippet, store_root):
            all_errors.append(f"{label}: {error}")
    if all_errors:
        for error in all_errors:
            print(error, file=sys.stderr)
        return 1
    print(f"validated {len(snippets)} asset store snippet(s)")
    return 0


def command_self_test(args: argparse.Namespace) -> int:
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        sample = tmp_path / "sample.PNG"
        sample.write_bytes(b"firestaff v2 asset store self-test\n")
        manifest = tmp_path / "snippet.json"
        digest = sha256_file(sample)
        target = store_path_for(sample, digest, Path(args.store_root))
        existed = target.exists()
        rc = command_ingest(argparse.Namespace(
            file=str(sample),
            store_root=args.store_root,
            asset_id="fs.v2.asset.self-test",
            family="tooling",
            role="asset store self-test",
            manifest_out=str(manifest),
        ))
        if rc != 0:
            return rc
        validate_rc = command_validate(argparse.Namespace(manifest=str(manifest), store_root=args.store_root))
        if not existed and target.exists():
            target.unlink()
            for directory in [target.parent, target.parent.parent, target.parent.parent.parent]:
                try:
                    directory.rmdir()
                except OSError:
                    break
        return validate_rc


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Ingest and validate checksum-addressed Firestaff V2 asset store objects.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    ingest = subparsers.add_parser("ingest", help="copy a file into assets-v2/store/sha256/xx/<sha>.<ext>")
    ingest.add_argument("file", help="file to ingest")
    ingest.add_argument("--asset-id", help="manifest id to emit; defaults to fs.v2.asset.<sha-prefix>")
    ingest.add_argument("--family", help="optional manifest family field")
    ingest.add_argument("--role", help="optional manifest role field")
    ingest.add_argument("--store-root", default=STORE_ROOT.as_posix(), help="repository-relative store root")
    ingest.add_argument("--manifest-out", help="write the emitted manifest snippet to this JSON file")
    ingest.set_defaults(func=command_ingest)

    validate = subparsers.add_parser("validate", help="validate emitted manifest snippet(s) against the store")
    validate.add_argument("manifest", help="JSON object, JSON array, or object with an assets array")
    validate.add_argument("--store-root", default=STORE_ROOT.as_posix(), help="repository-relative store root")
    validate.set_defaults(func=command_validate)

    self_test = subparsers.add_parser("self-test", help="run a small ingest/validate round trip")
    self_test.add_argument("--store-root", default=STORE_ROOT.as_posix(), help="repository-relative store root")
    self_test.set_defaults(func=command_self_test)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
