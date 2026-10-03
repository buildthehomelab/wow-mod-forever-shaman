#!/usr/bin/env bash
#
# Write the module's spell changes into a 3.3.5a (12340) client's Spell.dbc, for an optional
# client patch:
#
# - Mining in Ghost Wolf: the client blocks spells that say "not while shapeshifted" before it
#   asks the server, so without this the client still says you can't do that while shapeshifted.
# - Ghost Wolf indoors: the client also refuses outdoors-only spells indoors by itself.
#
# Usage: tools/patch-forever-shaman-dbc.sh <Spell.dbc> [output dir]
#   <Spell.dbc>   3.3.5a Spell.dbc: the client's own, or one another module's script already
#                 patched (it may be the output: it's read first)
#   [output dir]  default: ./DBFilesClient (ready to pack into an MPQ)
#
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    sed -n '9,12p' "$0" | sed 's/^# \{0,1\}//'
    exit 1
fi

python3 - "$1" "${2:-DBFilesClient}" <<'PY'
import os, struct, sys

spell_src, out_dir = sys.argv[1:3]

# Mining (every rank), mining a creature's corpse and Engineering salvage, which
# ForeverShaman.GhostWolfGathering makes usable in Ghost Wolf. Must match
# GHOST_WOLF_GATHERING_SPELLS in src/ForeverShaman.cpp.
GHOST_WOLF_GATHERING_SPELLS = [2575, 2576, 3564, 10248, 29354, 50310, 32606, 49383]
FORM_MASK_GHOST_WOLF = 1 << (16 - 1)  # FORM_GHOSTWOLF
SPELL_ATTR0_NOT_SHAPESHIFTED = 0x10000
SPELL_ATTR2_ALLOW_WHILE_NOT_SHAPESHIFTED = 0x80000
SPELL_GHOST_WOLF = 2645
SPELL_ATTR0_ONLY_OUTDOORS = 0x8000

SPELL_FIELDS = 234
ATTRIBUTES = 4                # m_attributes
ATTRIBUTES_EX2 = 6            # m_attributesExB
STANCES = 12                  # m_shapeshiftMask

with open(spell_src, "rb") as f:
    data = f.read()
magic, records, fields, record_size, string_size = struct.unpack_from("<4s4I", data, 0)
if magic != b"WDBC" or fields != SPELL_FIELDS or record_size != SPELL_FIELDS * 4:
    sys.exit(f"{spell_src}: not a 3.3.5a Spell.dbc (magic={magic!r} fields={fields} recordSize={record_size})")
rows = [list(struct.unpack_from(f"<{fields}I", data, 20 + i * record_size)) for i in range(records)]
strings = data[20 + records * record_size:]
by_id = {row[0]: row for row in rows}

# Same as ApplyGhostWolfGathering: add Ghost Wolf to the spell's forms, and let it be used outside
# a form too. Only adds Ghost Wolf, so mod-forever-druid's script can add the druid forms to the
# same spells before or after this one.
for spell_id in GHOST_WOLF_GATHERING_SPELLS:
    row = by_id.get(spell_id)
    if row is None:
        sys.exit(f"{spell_src}: spell {spell_id} not found")
    if not row[ATTRIBUTES] & SPELL_ATTR0_NOT_SHAPESHIFTED:
        sys.exit(f"{spell_src}: spell {spell_id} isn't blocked while shapeshifted; is this a 3.3.5a Spell.dbc?")
    row[STANCES] |= FORM_MASK_GHOST_WOLF
    row[ATTRIBUTES_EX2] |= SPELL_ATTR2_ALLOW_WHILE_NOT_SHAPESHIFTED
print(f"  {len(GHOST_WOLF_GATHERING_SPELLS)} mining spells: usable in Ghost Wolf")

# Same as ApplyGhostWolfIndoors: Ghost Wolf is no longer outdoors only.
row = by_id.get(SPELL_GHOST_WOLF)
if row is None:
    sys.exit(f"{spell_src}: spell {SPELL_GHOST_WOLF} not found")
row[ATTRIBUTES] &= ~SPELL_ATTR0_ONLY_OUTDOORS
print(f"  Ghost Wolf ({SPELL_GHOST_WOLF}): usable indoors")

os.makedirs(out_dir, exist_ok=True)
out = os.path.join(out_dir, "Spell.dbc")
with open(out, "wb") as f:
    f.write(struct.pack("<4s4I", b"WDBC", len(rows), SPELL_FIELDS, SPELL_FIELDS * 4, len(strings)))
    for row in rows:
        f.write(struct.pack(f"<{SPELL_FIELDS}I", *row))
    f.write(strings)
print(f"Wrote {out}")
PY

cat <<'EOF2'

Next:
  1. Pack the file into a client patch MPQ as DBFilesClient\Spell.dbc. Only the newest MPQ's
     copy is used, so start from the Spell.dbc your current patch already ships (other modules'
     changes) and put the result back in that same patch.
  2. Players who get the new patch should delete their Cache/ folder.
EOF2
