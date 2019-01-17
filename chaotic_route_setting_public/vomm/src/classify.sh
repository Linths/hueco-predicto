#!/bin/bash

# Playground for eventually classifying routes wrt difficulty

# Predict the probabilty of symbol 4 being generated
# if the context are the symbols:
# 1
echo -e 1 | java -cp . Test predict ../data/vomm_1.ser 4
# 1,4
echo -e "1\n4" | java -cp . Test predict ../data/vomm_1.ser 4
# 4,1
echo -e "4\n1" | java -cp . Test predict ../data/vomm_1.ser 4
# 1,4,1
echo -e "1\n4\n1" | java -cp . Test predict ../data/vomm_1.ser 4