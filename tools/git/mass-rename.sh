#!/bin/bash
# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# mass-rename: update source files (gyp lists, #includes) to reflect
# a rename.  Expects "git diff --cached -M" to list a bunch of renames.
#
# To use:
#   1) git mv foo1 bar1; git mv foo2 bar2; etc.
#   2) *without committing*, ./tools/git/mass-rename.sh
#   3) look at git diff (without --cached) to see what the damage is
#   4) commit, then use tools/sort-headers.py to fix #include ordering:
#   for f in $(git diff --name-only origin); do ./tools/sort-headers.py $f; done

DIR="$( cd "$( dirname "$0" )" && pwd )"

# Make the 'read' used in the while loop split only on tabs/newlines.
IFS=$'\t\n'

git diff --cached --raw -M | while read attrs from to; do
    type=$(echo "$attrs" | cut -d' ' -f5)
    if echo "$type" | grep -q "^R"; then
        python $DIR/move_source_file.py --already-moved "$from" "$to"
    else
        echo "Skipping: $from -- not a rename?"
    fi
done
