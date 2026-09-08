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
#   0 = every shipped locale has a complete, non-fuzzy runtime catalog.
#   1 = missing required files, incomplete catalogs, malformed headers, or
#       other fatal errors.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ "${1:-}" = "--po-dir" ]; then
    [ "$#" -eq 2 ] || { echo "usage: $0 [--po-dir DIRECTORY]" >&2; exit 2; }
    PO_DIR="$2"
else
    [ "$#" -eq 0 ] || { echo "usage: $0 [--po-dir DIRECTORY]" >&2; exit 2; }
    PO_DIR="$SCRIPT_DIR"
fi
cd "$PO_DIR"

# All i18n domains Firestaff ships (added: firestaff + nexus + theron
# to the original startup-menu/dm1/csb/dm2 quartet).
DOMAINS=(startup-menu dm1 csb dm2 firestaff nexus theron firestaff_studio)

# Every shipped domain must provide every supported locale. English is the
# source locale: its empty msgstr values deliberately resolve through msgid
# at runtime, which is a complete English catalog rather than a gap.
KNOWN_LOCALES=(en sv fr de ja zh cs da es fi hu it ko nl no pl pt ru tr id)

ERRORS=0
WARNINGS=0

catalog_path() {
    local domain="$1"
    local locale="$2"
    if [ "$domain" = "firestaff_studio" ]; then
        printf 'studio/%s.po' "$locale"
    else
        printf '%s.%s.po' "$domain" "$locale"
    fi
}

# po_count <mode> <file>
# mode = "total" -> print number of non-empty msgid entries (catalog size)
# mode = "translated" -> print number of entries with non-empty msgstr
# mode = "native" -> print number of entries whose non-empty msgstr differs
#                  from msgid (fallback/scaffold msgstr values excluded)
#
po_count() {
    local mode="$1"
    local file="$2"
    awk -v count_mode="$mode" '
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
' "$file"
}

echo "=== po/ layout validation ==="

# Gettext deliberately ignores fuzzy entries at runtime.  A catalog that
# still contains one is therefore incomplete regardless of a non-empty
# msgstr, so reject them before reporting completion percentages.  Include
# Studio catalogs here even though their files live below po/studio/.
while IFS= read -r catalog; do
    fuzzy_count=$(msgattrib --only-fuzzy --no-obsolete "$catalog" |
        awk '/^msgid / { count++ } END { print count + 0 }')
    if [ "$fuzzy_count" -ne 0 ]; then
        echo "FAIL: $catalog contains $fuzzy_count active fuzzy translation(s)"
        ERRORS=$((ERRORS + 1))
    fi
done < <(find . -type f -name '*.po' -print | sort)

for domain in "${DOMAINS[@]}"; do
    pot="${domain}.pot"
    enpo="$(catalog_path "$domain" en)"

    echo ""
    echo "--- domain: ${domain} ---"

    # Required: .pot template
    if [ ! -f "$pot" ]; then
        echo "FAIL: missing $pot"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK:   $pot exists"
        if command -v msgfmt >/dev/null 2>&1 &&
           ! msgfmt --check --check-format -o /dev/null "$pot"; then
            echo "FAIL: $pot is not a valid gettext template"
            ERRORS=$((ERRORS + 1))
        fi
        # A POT is the English source template, never a translation catalog.
        # Every non-header msgstr must remain empty.
        pot_translated=$(po_count translated "$pot" || echo 0)
        if [ "$pot_translated" -ne 0 ]; then
            echo "FAIL: $pot contains ${pot_translated} translated msgstr value(s)"
            ERRORS=$((ERRORS + 1))
        fi
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

    # Every locale is shipped and therefore must be complete. A partial
    # catalog silently falls back to English in gettext, which violates the
    # product's full-localization contract even though the UI still renders.
    for loc in "${KNOWN_LOCALES[@]}"; do
        f="$(catalog_path "$domain" "$loc")"
        if [ ! -f "$f" ]; then
            echo "FAIL: missing required locale catalog $f"
            ERRORS=$((ERRORS + 1))
            continue
        fi
        if [ -f "$f" ]; then
            # A catalog must identify its own domain. This catches copied PO
            # headers that otherwise compile but belong to another game.
            if [ "$domain" != "firestaff_studio" ] &&
               grep -q 'Project-Id-Version:' "$f" &&
               ! grep -q "Project-Id-Version: firestaff-${domain}" "$f"; then
                if [ "$domain" != "firestaff" ] ||
                   ! grep -q 'Project-Id-Version: firestaff' "$f"; then
                    echo "FAIL: $f Project-Id-Version does not identify ${domain}"
                    ERRORS=$((ERRORS + 1))
                fi
            fi
            tot=$(po_count total "$f" || echo 0)
            tra=$(po_count translated "$f" || echo 0)
            native=$(po_count native "$f" || echo 0)
            if [ "$tot" -gt 0 ]; then
                if [ "$loc" = "en" ]; then
                    # English is intentionally represented by msgid in the
                    # source catalog. Treat it as a full runtime translation.
                    tra="$tot"
                fi
                pct=$(( tra * 100 / tot ))
                native_pct=$(( native * 100 / tot ))
                if [ "$tra" -ne "$tot" ]; then
                    echo "FAIL: $f is incomplete: ${tra}/${tot} active translations"
                    ERRORS=$((ERRORS + 1))
                fi
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
