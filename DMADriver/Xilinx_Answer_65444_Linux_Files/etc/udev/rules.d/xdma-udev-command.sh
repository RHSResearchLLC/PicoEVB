#!/bin/bash
#
# Renames xdma%d_c2h_%d to xdma/card%d/c2h%d
# Renames xdma%d_h2c_%d to xdma/card%d/h2c%d
# Renames xdma%d_user   to xdma/card%d/user
# etc.
#
# Leon Woestenberg <leon@sidebranch.com>

echo $1 | \
/bin/sed 's:xdma\([0-9][0-9]*\)_events_\([0-9][0-9]*\):xdma/card\1/events\2:' | \
/bin/sed 's:xdma\([0-9][0-9]*\)_\([ch]2[ch]\)_\([0-9]*\):xdma/card\1/\2\3:' | \
/bin/sed 's:xdma\([0-9][0-9]*\)_bypass_\([ch]2[ch]\)_\([0-9]*\):xdma/card\1/bypass_\2\3:' | \
/bin/sed 's:xdma\([0-9][0-9]*\)_\([a-z]*\):xdma/card\1/\2:'


