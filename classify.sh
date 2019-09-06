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

# Outputs log-losses

# Routes expressed in symbolset {1..4}
RID1=("1\n1\n2\n3\n3\n1\n4\n5\n1\n5\n1\n1\n2\n4\n1\n6\n1\n7\n8\n1\n4\n9" "1\n1\n2\n3\n3\n4\n5\n6\n4\n6\n7\n8\n2\n5\n8\n9\n1\n10\n11\n1\n5\n12" "1\n2\n3\n4\n4\n1\n5\n6\n1\n6\n1\n7\n3\n5\n7\n8\n1\n9\n10\n1\n5\n11" "1\n2\n3\n4\n4\n5\n6\n7\n5\n7\n8\n9\n3\n6\n9\n10\n1\n11\n12\n1\n6\n13")
RID2=("1\n10\n1\n4\n1\n11\n1\n1\n12\n13\n1\n8\n1\n2\n14\n10\n2\n1\n15\n15\n15\n16\n17" "1\n13\n1\n14\n4\n15\n16\n17\n18\n19\n1\n11\n16\n2\n20\n13\n2\n1\n21\n21\n21\n22\n23" "1\n12\n1\n5\n1\n13\n7\n7\n14\n15\n1\n10\n7\n3\n16\n12\n3\n1\n17\n17\n17\n18\n19" "1\n14\n1\n15\n5\n16\n17\n18\n19\n20\n1\n12\n17\n3\n21\n14\n3\n1\n22\n22\n22\n23\n24")
# RID2=("11\n18\n23\n11\n23\n1" "15\n36\n37\n15\n37\n1" "13\n22\n30\n13\n30\n1" "16\n40\n41\n16\n41\n1")
RIDS=($RID1 $RID2)

for r in ${!RIDS[*]}; do
    echo -e "\n--- Route $(($r+1)) ---"
    for k in {1..4}; do
        echo -e "\nSet $k"
        # Use every model found
        for model in `find vomm/data/grades/set_$k -name '*ser'`; do
            printf "$model    \t\t"
            echo -e ${RIDS[r][k-1]} | java -cp vomm/src Test eval $model
        done
    done
done
