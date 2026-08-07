#!/bin/sh
# po/update.sh — Regenerate startup-menu.pot from source and merge into .po files.
#
# Usage:  cd <project-root> && sh po/update.sh
#
# Requires: xgettext, msgmerge (GNU gettext-tools)

set -e

DOMAIN="startup-menu"
POT="po/${DOMAIN}.pot"
SOURCES="src/ui/menu_startup_m12.c src/engine/main_loop_m11.c"

# 1. Extract translatable strings from source via xgettext.
xgettext \
    --keyword=_ \
    --language=C \
    --from-code=UTF-8 \
    --sort-by-file \
    --no-wrap \
    --package-name=firestaff-startup-menu \
    --copyright-holder=Firestaff \
    --msgid-bugs-address=daniel@danielnylander.se \
    --output="${POT}" \
    ${SOURCES}

count=$(grep -c '^msgid ' "${POT}")
echo "POT updated: ${POT} (${count} entries)"

# 2. Merge new POT into each .po file, preserving existing translations.
for po in po/${DOMAIN}.*.po; do
    lang=$(echo "${po}" | sed "s|po/${DOMAIN}\.\(.*\)\.po|\1|")
    msgmerge --update --no-wrap --backup=none "${po}" "${POT}"
    translated=$(msgfmt --statistics -o /dev/null "${po}" 2>&1 | grep -o '[0-9]* translated' | grep -o '[0-9]*' || echo 0)
    echo "  ${lang}: merged (${translated} translated)"
done

echo "Done. Review fuzzy (#, fuzzy) entries in each .po file."
