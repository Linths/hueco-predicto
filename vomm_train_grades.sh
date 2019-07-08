#!/bin/bash
# ========== HEADER =======================================================
# A Fair Grade: Assessing Difficulty of Climbing Routes through Machine Learning
# Copyright (C) 2019  Lindsay Kempen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# =========================================================================

# Trains grade-specific models. Done for every symbol set (6).
# Params: [Model depth] [Excluded routes]

LANG_SIZE=256
DEFAULT_MODEL_DEPTH=5
# GRADES = $(data/route_grade.csv | cut -f 1 -d ',' | sort -u)

if [ -z "$1" ]; then
    # If model depth is not specified
    MODEL_DEPTH=$DEFAULT_MODEL_DEPTH
else
    MODEL_DEPTH=$1
fi

for k in {1..4}; do
    echo "Symbol set $k"
    rm -rf vomm/data/grades/set_$k/*
    ruby vomm_train_grades.rb $k $LANG_SIZE $MODEL_DEPTH $2
done