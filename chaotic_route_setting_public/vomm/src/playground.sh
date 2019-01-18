#!/bin/bash

# Playground for eventually classifying routes wrt difficulty

echo "Probability playground"

# --- Symbol set 1 ---
# 1 jug
# 2 match
# 3 side-pull
# 4 crimp
# 18 edge

# Predict the probabilty of symbol x being generated
# with symbol set 1 with the given context

echo -e "\n--- Using symbol set 1 ---"
printf "P( crimp | jug )                = "
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 4
printf "P( crimp | jug, crimp )         = "
echo -e "1\n4" | java -cp . Test predict ../data/vomm_1.ser 4
printf "P( crimp | crimp , jug )        = "
echo -e "4\n1" | java -cp . Test predict ../data/vomm_1.ser 4
printf "P( crimp | jug, crimp, jug )    = "
echo -e "1\n4\n1" | java -cp . Test predict ../data/vomm_1.ser 4
printf "P( edge | crimp, jug )          = "
echo -e "4\n1" | java -cp . Test predict ../data/vomm_1.ser 18
printf "P( edge | jug, crimp )          = "
echo -e "1\n4" | java -cp . Test predict ../data/vomm_1.ser 18

# What would happen after a jug?
printf "P( jug | jug )                  = " # = 0.023242027815896966"
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 1
printf "P( match | jug )                = " # = 0.28352181459744497"
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 2
printf "P( side-pull | jug )            = " # = 0.002187668322511149"
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 3
printf "P( crimp | jug )                = " # = 0.34670949530089196"
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 4
printf "P( edge | jug )                 = " # = 5.340987115505735E-7"
echo -e "1" | java -cp . Test predict ../data/vomm_1.ser 18

# --- Symbol set 4 ---
# 1 jug good=0
# 2 jug good=0 big.move
# 3 match good=0 match
# 4 side-pull good=1
# 7 crimp good=0
# 30 edge good=0 "unwind to good left edge"

echo -e "\n--- Using symbol set 1 ---"

printf "P( edge | crimp, jug )          = "
echo -e "7\n1" | java -cp . Test predict ../data/vomm_4.ser 30
printf "P( edge | jug, crimp )          = "
echo -e "1\n7" | java -cp . Test predict ../data/vomm_4.ser 30

# What would happen after a jug?
printf "P( jug | jug )                  = " # = 0.023242027815896966"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 1
printf "P( big-move-jug | jug )         = " # = 0.0010898947825235617"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 2
printf "P( match | jug )                = " # = 0.29932975357326624"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 3
printf "P( good-side-pull | jug )       = " # = 5.449473912617808E-4"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 4
printf "P( crimp | jug )                = " # = 1.362368478154452E-4"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 7
printf "P( edge | jug )                 = " # = 3.2481395677434256E-11"
echo -e "1" | java -cp . Test predict ../data/vomm_4.ser 30