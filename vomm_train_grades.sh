#!/bin/bash
LANG_SIZE=256
DEFAULT_MODEL_DEPTH=5

# Train grade-specific models
# Done for every symbol set

# GRADES = $(data/route_grade.csv | cut -f 1 -d ',' | sort -u)

# Params: [Model depth] [Excluded routes]
if [ -z "$1" ]; then
    # If model depth is not specified
    MODEL_DEPTH=$DEFAULT_MODEL_DEPTH
else
    MODEL_DEPTH=$1
fi

for k in {1..4}; do
    echo "Symbol set $k"
    if [ -d vomm/data/grades/set_$k ]; then
        rm -rf vomm/data/grades/set_$k/*
    fi
    ruby vomm_train_grades.rb $k $LANG_SIZE $MODEL_DEPTH $2
done