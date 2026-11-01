#!/usr/bin/env python3
"""Stop one Mednafen capture when its trace outputs exceed a byte budget."""

from __future__ import annotations

import argparse
import os
import signal
import stat
import sys
import time
from pathlib import Path


def total_bytes(paths: list[str], directory: Path | None, prefix: str | None) -> int:
    total = 0
    seen_files: set[tuple[int, int]] = set()

    def add_file(info: os.stat_result) -> None:
        nonlocal total
        if not stat.S_ISREG(info.st_mode):
            return
        identity = (info.st_dev, info.st_ino)
        if identity in seen_files:
            return
        seen_files.add(identity)
        total += info.st_size

    for raw_path in paths:
        try:
            info = os.stat(raw_path, follow_symlinks=False)
        except FileNotFoundError:
            continue
        add_file(info)
    if directory is not None and prefix is not None:
        try:
            with os.scandir(directory) as entries:
                for entry in entries:
                    if not entry.name.startswith(prefix + "."):
                        continue
                    try:
                        add_file(entry.stat(follow_symlinks=False))
                    except FileNotFoundError:
                        continue
        except FileNotFoundError:
            pass
    return total


def write_marker(path: Path, message: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("x", encoding="utf-8") as marker:
        marker.write(message)
        marker.write("\n")


def stop_capture_group(pid: int) -> None:
    try:
        os.killpg(pid, signal.SIGTERM)
    except ProcessLookupError:
        return
    # A brief grace lets Mednafen close files. The capture is already rejected,
    # so terminate any process that ignored TERM instead of risking a writer
    # continuing after the watcher exits.
    time.sleep(0.15)
    try:
        os.killpg(pid, signal.SIGKILL)
    except (ProcessLookupError, PermissionError):
        pass


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pid", type=int, required=True, help="capture timeout process-group leader")
    parser.add_argument("--limit-bytes", type=int, required=True)
    parser.add_argument("--marker", type=Path, required=True)
    parser.add_argument("--poll-ms", type=int, default=100)
    parser.add_argument("--directory", type=Path)
    parser.add_argument("--prefix", help="include regular files named PREFIX.* in directory")
    parser.add_argument("--path", action="append", default=[], help="additional capture output file")
    args = parser.parse_args()

    if (args.pid <= 1 or args.limit_bytes <= 0 or not 10 <= args.poll_ms <= 5000 or
            bool(args.directory) != bool(args.prefix) or
            (not args.path and not args.directory)):
        parser.error("pid, byte limit, or polling interval is outside its valid range")
    try:
        if os.getpgid(args.pid) != args.pid:
            write_marker(
                args.marker,
                "status=watchdog_error\n"
                f"process_group={args.pid}\n"
                "error=capture timeout process is not its process-group leader\n",
            )
            os.kill(args.pid, signal.SIGTERM)
            return 3
    except ProcessLookupError:
        return 0
    except Exception as error:
        try:
            write_marker(
                args.marker,
                "status=watchdog_error\n"
                f"process_group={args.pid}\n"
                f"error={type(error).__name__}: {error}\n",
            )
        except Exception:
            pass
        try:
            os.kill(args.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
        return 3

    try:
        while True:
            size = total_bytes(args.path, args.directory, args.prefix)
            if size > args.limit_bytes:
                message = (
                    "status=exceeded\n"
                    f"limit_bytes={args.limit_bytes}\n"
                    f"observed_bytes={size}\n"
                    f"process_group={args.pid}\n"
                )
                write_marker(args.marker, message)
                print(
                    f"capture output budget exceeded: {size} > {args.limit_bytes} bytes; "
                    f"stopping process group {args.pid}",
                    file=sys.stderr,
                    flush=True,
                )
                stop_capture_group(args.pid)
                return 2
            time.sleep(args.poll_ms / 1000.0)
    except KeyboardInterrupt:
        return 0
    except Exception as error:  # Fail closed if the guard itself cannot monitor.
        try:
            write_marker(
                args.marker,
                "status=watchdog_error\n"
                f"process_group={args.pid}\n"
                f"error={type(error).__name__}: {error}\n",
            )
        except Exception:
            pass
        stop_capture_group(args.pid)
        print(f"capture output watchdog failed: {error}", file=sys.stderr, flush=True)
        return 3


if __name__ == "__main__":
    raise SystemExit(main())
