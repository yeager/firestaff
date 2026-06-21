#!/usr/bin/env bash
# po/validate_po_layout.sh — Structural validation for po/ layout.
#
# Verifies that the expected files exist, are valid PO/POT format, and
# reports translation completeness per language per domain. It separates
# non-empty fallback/scaffold msgstr values from native translations by
# treating msgstr == msgid as fallback coverage. No runtime i18n is
# tested here — this is filesystem contract only.
#
# Exit codes:
#   0 = all structural checks pass. Zero/fallback-only native coverage is
#       reported as WARN/FALL but does not fail the structural CI gate.
#   1 = missing required files, malformed headers, or fatal errors

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# All i18n domains Firestaff ships (added: firestaff + nexus + theron
# to the original startup-menu/dm1/csb/dm2 quartet).
DOMAINS=(startup-menu dm1 csb dm2 firestaff nexus theron)

# Per-domain "all known locales" list. Used to enumerate catalogs we
# expect to exist. The script is structural, not strict: missing locale
# files are reported as warnings, not errors, but completeness is
# reported per locale so a translator can see which gaps remain.
#
# English (.en.po) is required for every domain.
KNOWN_LOCALES=(en sv fr de ja zh cs da es fi hu it ko nl no pl pt ru tr)

ERRORS=0
WARNINGS=0

# po_count <mode> <file>
# mode = "total" -> print number of non-empty msgid entries (catalog size)
# mode = "translated" -> print number of entries with non-empty msgstr
# mode = "native" -> print number of entries whose non-empty msgstr differs
#                  from msgid (fallback/scaffold msgstr values excluded)
#
# Implementation: write the awk program to a temp file (because we cannot
# safely interpolate multi-line awk with BEGIN blocks into a bash string
# without escaping headaches) and invoke it with awk -f.
po_count() {
    local mode="$1"
    local file="$2"
    local awk_tmp
    awk_tmp="$(mktemp -t po_validate_awk.XXXXXX)"
    cat > "$awk_tmp" <<'AWK_EOF'
BEGIN {
    msgid = ""
    msgstr = ""
    have_entry = 0
    in_id = 0
    in_str = 0
    total = 0
    translated = 0
    native = 0
}
function po_fragment(line, value) {
    value = line
    sub(/^[^\"]*\"/, "", value)
    sub(/\"[[:space:]]*$/, "", value)
    return value
}
function finish_entry() {
    if (have_entry && msgid != "") {
        total++
        if (msgstr != "") {
            translated++
            if (msgstr != msgid) native++
        }
    }
}
/^msgid "/ {
    finish_entry()
    msgid = po_fragment($0)
    msgstr = ""
    have_entry = 1
    in_id = 1
    in_str = 0
    next
}
/^msgstr "/ {
    msgstr = po_fragment($0)
    in_id = 0
    in_str = 1
    next
}
/^"/ {
    if (in_id) msgid = msgid po_fragment($0)
    else if (in_str) msgstr = msgstr po_fragment($0)
    next
}
{
    next
}
END {
    finish_entry()
    if (count_mode == "translated") print translated
    else if (count_mode == "native") print native
    else print total
}
AWK_EOF
    awk -v count_mode="$mode" -f "$awk_tmp" "$file"
    local rc=$?
    rm -f "$awk_tmp"
    return $rc
}

echo "=== po/ layout validation ==="

for domain in "${DOMAINS[@]}"; do
    pot="${domain}.pot"
    enpo="${domain}.en.po"

    echo ""
    echo "--- domain: ${domain} ---"

    # Required: .pot template
    if [ ! -f "$pot" ]; then
        echo "FAIL: missing $pot"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK:   $pot exists"
    fi

    # Required: .en.po
    if [ ! -f "$enpo" ]; then
        echo "FAIL: missing $enpo"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK:   $enpo exists"
        # Basic header check on .en.po
        if ! grep -q '"Language: en\\n"' "$enpo"; then
            echo "FAIL: $enpo missing 'Language: en' header"
            ERRORS=$((ERRORS + 1))
        fi
        # Content-Type header check on both .pot and .en.po
        for f in "$pot" "$enpo"; do
            if [ -f "$f" ]; then
                if ! grep -q 'Content-Type: text/plain; charset=UTF-8' "$f"; then
                    echo "FAIL: $f missing valid PO Content-Type header"
                    ERRORS=$((ERRORS + 1))
                fi
            fi
        done

        # Count msgid + non-empty msgstr for completeness. Native coverage
        # excludes scaffold/fallback entries where msgstr mirrors msgid.
        total=$(po_count total "$enpo" || echo 0)
        translated=$(po_count translated "$enpo" || echo 0)
        if [ "$total" -gt 0 ]; then
            pct=$(( translated * 100 / total ))
            echo "INFO: $enpo coverage: ${translated}/${total} msgstr nonblank (${pct}%)"
        fi
    fi

    # Per-locale coverage report (warnings only)
    for loc in "${KNOWN_LOCALES[@]}"; do
        f="${domain}.${loc}.po"
        if [ -f "$f" ]; then
            tot=$(po_count total "$f" || echo 0)
            tra=$(po_count translated "$f" || echo 0)
            native=$(po_count native "$f" || echo 0)
            if [ "$tot" -gt 0 ]; then
                pct=$(( tra * 100 / tot ))
                native_pct=$(( native * 100 / tot ))
                marker="OK  "
                if [ "$loc" = "en" ]; then
                    marker="SRC "
                elif [ "$tra" -eq 0 ]; then
                    marker="WARN"
                    WARNINGS=$((WARNINGS + 1))
                elif [ "$native" -eq 0 ]; then
                    marker="FALL"
                    WARNINGS=$((WARNINGS + 1))
                elif [ "$native_pct" -lt 50 ]; then
                    marker="PART"
                fi
                if [ "$loc" = "en" ]; then
                    printf "  %s %-30s nonblank %3d/%3d (%d%%) native source\n" "$marker" "$f" "$tra" "$tot" "$pct"
                else
                    printf "  %s %-30s nonblank %3d/%3d (%d%%) native %3d/%3d (%d%%)\n" \
                        "$marker" "$f" "$tra" "$tot" "$pct" "$native" "$tot" "$native_pct"
                fi
            fi
        fi
    done
done

echo ""
echo "=== summary ==="
echo "domains: ${#DOMAINS[@]}"
echo "errors:  ${ERRORS}"
echo "warnings: ${WARNINGS} (locale file with zero or fallback-only native coverage)"

if [ "$ERRORS" -gt 0 ]; then
    echo "FAIL: ${ERRORS} structural error(s) found"
    exit 1
fi
echo "PASS: structural layout validated"
