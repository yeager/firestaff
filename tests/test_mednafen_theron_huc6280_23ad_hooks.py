#!/usr/bin/env python3
"""Exercise the read-only source-anchor validator on disposable fixtures."""

from pathlib import Path
import subprocess
import tempfile


SCRIPT = Path(__file__).resolve().parents[1] / "scripts/validate_mednafen_theron_huc6280_23ad_hooks.py"
FIXTURES = {
    "src/pce_fast/huc6280.cpp": """
#define HU_Page1\t\tPage1_local
#define LOAD_LOCALS()
#define SAVE_LOCALS()
#define RdAtPC() RdOp(HU_PC)
static INLINE uint8 RdMem(unsigned int A)
static INLINE void WrMem(unsigned int A, uint8 V)
static INLINE uint8 RdOp(unsigned int A)
#define SET_MPR(arg_i, arg_v)
#define TAM     for(int i = 0; i < 8; i ++) {
\tGetOutBMT:
\tSAVE_LOCALS();
FixPC_PC();
PUSH_PC();
#define POP_PC_AP()
""",
    "src/pce_fast/huc6280_ops.inc": """
case 0x53: // TAMi
HU_A = HU_Page1[HU_X];
HU_Page1[HU_X] /*PAGE1_W[HU_X]*/ =  HU_A;
""",
    "configure.ac": "MEDNAFEN_VERSION='1.32.1'\n",
}


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="firestaff-23ad-hook-fixture-") as temp:
        root = Path(temp)
        for relative, content in FIXTURES.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
        good = subprocess.run(["python3", str(SCRIPT), str(root)], capture_output=True, text=True)
        if good.returncode != 0 or "PASS:" not in good.stdout:
            raise SystemExit(f"FAIL: expected fixture to pass: {good.stderr}{good.stdout}")
        ops = root / "src/pce_fast/huc6280_ops.inc"
        ops.write_text(ops.read_text().replace("case 0x53: // TAMi", "case 0x52: // TMAi"))
        bad = subprocess.run(["python3", str(SCRIPT), str(root)], capture_output=True, text=True)
        if bad.returncode == 0 or "missing anchor" not in bad.stderr:
            raise SystemExit("FAIL: missing hook anchor was not rejected")
    print("PASS: HuC6280 hook validator accepts required anchors and rejects a missing one")


if __name__ == "__main__":
    main()
