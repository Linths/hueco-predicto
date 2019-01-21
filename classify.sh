#!/bin/bash

# Routes expressed in symbolset {1..4}
RID1=("1\n1\n2\n3\n3\n1\n4\n5\n1\n5\n1\n1\n2\n4\n1\n6\n1\n7\n8\n1\n4\n9" "1\n1\n2\n3\n3\n4\n5\n6\n4\n6\n7\n8\n2\n5\n8\n9\n1\n10\n11\n1\n5\n12" "1\n2\n3\n4\n4\n1\n5\n6\n1\n6\n1\n7\n3\n5\n7\n8\n1\n9\n10\n1\n5\n11" "1\n2\n3\n4\n4\n5\n6\n7\n5\n7\n8\n9\n3\n6\n9\n10\n1\n11\n12\n1\n6\n13")
RID2=("11\n18\n23\n11\n23\n1" "15\n36\n37\n15\n37\n1" "13\n22\n30\n13\n30\n1" "16\n40\n41\n16\n41\n1")
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
