#!/bin/bash

echo $1 | sed "s/,/\n/g" | java -cp vomm/src Test eval $2