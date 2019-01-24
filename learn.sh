#!/bin/bash

# Params [sequence] [model] [language_size] [model_depth]
echo $1 | sed "s/,/\n/g" | java -cp vomm/src Test learn $2 $3 $4