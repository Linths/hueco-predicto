#!/bin/bash
echo "Learning abracadabra"
echo -e "1\n2\n18\n1\n3\n1\n4\n1\n2\n18\n1" | java -cp . Test learn test.ser 26 5
echo "Predicting P(c|abra)"
echo -e "1\n2\n18\n1" | java -cp . Test predict test.ser 3
echo "LogEval(cadabra)"
echo -e "3\n1\n4\n1\n2\n18\n1" | java -cp . Test eval test.ser
echo "Smoothing a,b,r c,a,d"
java -cp . Test smooth test.ser 1,2,18 3,1,4 1
