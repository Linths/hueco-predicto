#!/bin/bash
grep "^$1 " symbols.txt | sort -n | tr '[]:.+' ' ' \
   | sed 's/BIG/\(big move\)/' \
   | sed 's/GOOD/\(good hold\)/' \
   | sed 's/CROSS/\(cross\)/' \
   | awk 'gsub(/(HoldTypeT*|HoldSize(Big|Small)T*|HoldShape(Good|Bad)T*)|HoldShapeT*|HoldSizeT*/,"")' \
   | awk 'gsub(/ +/," ")' | tr '[A-Z]' '[a-z]'
