#!/usr/bin/env bash
# po/validate_po_layout.sh — Slice 1 structural validation for po/ layout.
# Verifies that the expected files exist and are valid PO/POT format.
# No runtime i18n is tested here — this is filesystem contract only.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

DOMAINS=(startup-menu dm1 csb dm2)
ERRORS=0

echo "=== po/ layout validation ==="

for domain in "${DOMAINS[@]}"; do
    pot="${domain}.pot"
    enpo="${domain}.en.po"

    # Check .pot exists
    if [ ! -f "$pot" ]; then
        echo "FAIL: missing $pot"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK:   $pot exists"
    fi

    # Check .en.po exists
    if [ ! -f "$enpo" ]; then
        echo "FAIL: missing $enpo"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK:   $enpo exists"
    fi

    # Basic PO header validation (check for Content-Type header)
    for f in "$pot" "$enpo"; do
        if [ -f "$f" ]; then
            if ! grep -q 'Content-Type: text/plain; charset=UTF-8' "$f"; then
                echo "FAIL: $f missing valid PO header"
                ERRORS=$((ERRORS + 1))
            fi
        fi
    done

    # Check .en.po has Language: en
    if [ -f "$enpo" ]; then
        if ! grep -q '"Language: en\\n"' "$enpo"; then
            echo "FAIL: $enpo missing Language: en header"
            ERRORS=$((ERRORS + 1))
        fi
    fi
done

echo ""
if [ "$ERRORS" -eq 0 ]; then
    echo "PASS: all ${#DOMAINS[@]} domains validated (${#DOMAINS[@]} .pot + ${#DOMAINS[@]} .en.po)"
else
    echo "FAIL: $ERRORS error(s) found"
    exit 1
fi
