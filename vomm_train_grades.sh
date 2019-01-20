#!/bin/bash
LANG_SIZE=256
MODEL_DEPTH=5

# Train grade-specific models
# Done for every symbol set

# GRADES = $(data/route_grade.csv | cut -f 1 -d ',' | sort -u)

for k in {1..4}; do
    echo "Symbol set $k"
    if [ -d vomm/data/grades/set_$k ]; then
        rm -rf vomm/data/grades/set_$k/*
    fi
    ruby vomm_train_grades.rb $k $LANG_SIZE $MODEL_DEPTH
done

# echo -e "1\n1\n1\n1\n1\n1\n1" | java -cp vomm/src Test eval vomm/data/grades/set_1/vomm_V0.ser
# echo -e "1\n1\n1\n1\n1\n1\n1" | java -cp vomm/src Test eval vomm/data/grades/set_1/vomm_V2.ser
# echo -e "1\n1\n1\n1\n1\n1\n1" | java -cp vomm/src Test eval vomm/data/grades/set_1/vomm_V3.ser

# echo -e "1\n1\n2\n3\n3\n1\n4\n5\n1\n5\n1\n1\n2\n4\n1\n6\n1\n7\n8\n1\n4\n9" | java -cp vomm/src Test eval vomm/data/grades/set_1/vomm_V0-V3.ser