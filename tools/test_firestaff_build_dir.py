#!/usr/bin/env python3
"""Test for the firestaff_build_dir helper."""
from __future__ import annotations

import os
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import find_build_dir, resolve_build_dir  # noqa: E402


class FindBuildDirTests(unittest.TestCase):
    def _make_project(self) -> Path:
        """Create a fake project root with a build/ subdir
        containing CTestTestfile.cmake."""
        tmp = Path(tempfile.mkdtemp(prefix="fbd_test_"))
        build = tmp / "build"
        build.mkdir()
        (build / "CTestTestfile.cmake").write_text("# sentinel")
        return tmp, build

    def test_env_var_wins(self) -> None:
        """FIRESTAFF_BUILD_DIR takes precedence."""
        tmp, _ = self._make_project()
        env_build = tmp / "external_build"
        env_build.mkdir()
        (env_build / "CTestTestfile.cmake").write_text("# sentinel")
        old = os.environ.get("FIRESTAFF_BUILD_DIR")
        try:
            os.environ["FIRESTAFF_BUILD_DIR"] = str(env_build)
            result = find_build_dir(tmp)
            self.assertEqual(result, env_build)
        finally:
            if old is None:
                os.environ.pop("FIRESTAFF_BUILD_DIR", None)
            else:
                os.environ["FIRESTAFF_BUILD_DIR"] = old

    def test_in_tree_build(self) -> None:
        """<root>/build/CTestTestfile.cmake is found."""
        tmp, build = self._make_project()
        result = find_build_dir(tmp)
        self.assertEqual(result, build)

    def test_multi_config_builds_dir(self) -> None:
        """<root>/builds/<cfg>/CTestTestfile.cmake is found."""
        tmp = Path(tempfile.mkdtemp(prefix="fbd_test_"))
        builds = tmp / "builds"
        builds.mkdir()
        n2 = builds / "n2-build"
        n2.mkdir()
        (n2 / "CTestTestfile.cmake").write_text("# sentinel")
        result = find_build_dir(tmp)
        self.assertEqual(result, n2)

    def test_no_build_returns_none(self) -> None:
        """No build anywhere -> None."""
        tmp = Path(tempfile.mkdtemp(prefix="fbd_test_"))
        result = find_build_dir(tmp)
        self.assertIsNone(result)

    def test_resolve_returns_fallback(self) -> None:
        """resolve_build_dir returns fallback when nothing found."""
        tmp = Path(tempfile.mkdtemp(prefix="fbd_test_"))
        fallback = tmp / "build"
        result = resolve_build_dir(tmp, fallback)
        self.assertEqual(result, fallback)

    def test_real_firestaff_project(self) -> None:
        """If the helper is run from the actual Firestaff repo
        it should find the in-tree build (build/ or builds/*)."""
        firestaff = Path(__file__).resolve().parents[1]
        # Only run if the build dirs actually exist; this is a
        # smoke test, not a hard requirement on the dev env.
        candidates = [
            firestaff / "build",
            firestaff / "builds",
        ]
        if not any(c.exists() for c in candidates):
            self.skipTest("Firestaff project has no build dir to test")
        result = find_build_dir(firestaff)
        self.assertIsNotNone(result)
        self.assertTrue((result / "CTestTestfile.cmake").exists())


if __name__ == "__main__":
    unittest.main()
