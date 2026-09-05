#!/usr/bin/env bash
# Regenerate source-backed templates, merge every catalog, and refresh README
# completion statistics. --check does the same in repository-local scratch.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:---update}"
DOMAINS=(startup-menu firestaff dm1 csb dm2 nexus theron)
if [[ "$MODE" != "--update" && "$MODE" != "--check" ]]; then echo "usage: po/update.sh [--update|--check]" >&2; exit 2; fi
for tool in xgettext msgmerge msgfmt msguniq; do command -v "$tool" >/dev/null 2>&1 || { echo "error: GNU gettext tool '$tool' is required" >&2; exit 2; }; done
WORK_PO="$ROOT/po"
SCRATCH=""
cleanup() { [[ -z "$SCRATCH" ]] || rm -rf -- "$SCRATCH"; }
trap cleanup EXIT INT TERM
if [[ "$MODE" == "--check" ]]; then
    SCRATCH="$ROOT/.po-update-check.$$"
    mkdir -p "$SCRATCH"
    cp -a "$ROOT/po" "$SCRATCH/po"
    WORK_PO="$SCRATCH/po"
fi
# Explicit source/keyword lists prevent debug literals becoming UI strings.
xgettext --keyword=_ --keyword=m12_tr:2 --keyword=m12_translate_for_locale:2 \
    --keyword=modern_tr:2 --language=C --from-code=UTF-8 --sort-by-file \
    --no-wrap --package-name=firestaff-startup-menu --copyright-holder=Firestaff \
    --msgid-bugs-address=daniel@danielnylander.se --directory="$ROOT" \
    --output="$WORK_PO/startup-menu.pot" src/ui/menu_startup_m12.c \
    src/ui/menu_startup_render_modern_m12.c src/ui/menu_startup_a11y_m12.c \
    src/ui/menu_hit_m12.c src/engine/main_loop_m11.c
xgettext --keyword=_ --language=Python --from-code=UTF-8 --sort-by-file \
    --no-wrap --package-name=firestaff-studio --copyright-holder=Firestaff \
    --msgid-bugs-address=daniel@danielnylander.se --directory="$ROOT" \
    --output="$WORK_PO/firestaff_studio.pot" \
    scripts/firestaff_artpack_studio.py scripts/firestaff_dungeon_studio.py \
    scripts/firestaff_savegame_editor.py
# DM2's original user-visible strings live in authenticated GDAT rather than
# C literals.  The checked extraction-marker file records the admitted PC-DOS
# GDAT keys and deliberately excludes command/animation/debug metadata.
xgettext --keyword=DM2_N_ --language=C --from-code=UTF-8 --sort-by-file \
    --add-comments=GDAT --no-wrap --package-name=firestaff-dm2 \
    --copyright-holder=Firestaff \
    --msgid-bugs-address=daniel@danielnylander.se --directory="$ROOT" \
    --output="$WORK_PO/dm2.pot" po/dm2_source_strings.c
# Every POT, including authenticated data-backed game templates, receives the
# same complete and deterministic metadata contract before canonicalization.
python3 "$ROOT/po/normalize_pot_headers.py" \
    "$WORK_PO/startup-menu.pot" "$WORK_PO/firestaff.pot" \
    "$WORK_PO/dm1.pot" "$WORK_PO/csb.pot" "$WORK_PO/dm2.pot" \
    "$WORK_PO/nexus.pot" "$WORK_PO/theron.pot" \
    "$WORK_PO/firestaff_studio.pot"
# Game templates also contain authenticated data-decoded text unavailable to
# xgettext. They are canonical inputs; every language is merged against them.
for domain in "${DOMAINS[@]}"; do
    pot="$WORK_PO/$domain.pot"
    [[ -f "$pot" ]] || { echo "error: missing $pot" >&2; exit 1; }
    # GNU msguniq intentionally emits no file for a header-only template.
    # Keep the guard for a newly introduced domain; every current game
    # template is expected to contain source-backed player text.
    if [[ "$(grep -c '^msgid ' "$pot")" -gt 1 ]]; then
        msguniq --use-first --no-wrap --output-file="$pot.canonical" "$pot"
        mv -- "$pot.canonical" "$pot"
    fi
    for catalog in "$WORK_PO/$domain".*.po; do
        [[ -e "$catalog" ]] || continue
        msgmerge --update --quiet --no-wrap --backup=none "$catalog" "$pot"
    done
done
msguniq --use-first --no-wrap --output-file="$WORK_PO/firestaff_studio.pot.canonical" "$WORK_PO/firestaff_studio.pot"
mv -- "$WORK_PO/firestaff_studio.pot.canonical" "$WORK_PO/firestaff_studio.pot"
for catalog in "$WORK_PO/studio"/*.po; do
    msgmerge --update --quiet --no-wrap --backup=none "$catalog" "$WORK_PO/firestaff_studio.pot"
done
python3 "$ROOT/po/generate_completion_table.py" --po-dir "$WORK_PO" --readme "$WORK_PO/README.md"
bash "$ROOT/po/validate_po_layout.sh" --po-dir "$WORK_PO"
if [[ "$MODE" == "--check" ]]; then
    status=0
    for path in README.md firestaff_studio.pot "${DOMAINS[@]/%/.pot}"; do
        if ! cmp -s "$ROOT/po/$path" "$WORK_PO/$path"; then echo "stale generated file: po/$path" >&2; diff -u "$ROOT/po/$path" "$WORK_PO/$path" || true; status=1; fi
    done
    for domain in "${DOMAINS[@]}"; do
        for catalog in "$WORK_PO/$domain".*.po; do name="${catalog##*/}"; if ! cmp -s "$ROOT/po/$name" "$catalog"; then echo "stale merged catalog: po/$name" >&2; status=1; fi; done
    done
    for catalog in "$WORK_PO/studio"/*.po; do name="${catalog##*/}"; if ! cmp -s "$ROOT/po/studio/$name" "$catalog"; then echo "stale merged catalog: po/studio/$name" >&2; status=1; fi; done
    [[ "$status" -eq 0 ]] || { echo "Run 'bash po/update.sh --update' and commit the result." >&2; exit 1; }
    echo "PASS: every POT, PO, and completion statistic is current"
else
    echo "Updated source-backed POTs, all PO files, and po/README.md"
fi
