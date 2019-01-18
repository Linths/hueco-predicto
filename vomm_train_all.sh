#!/bin/bash
LANG_SIZE=256
MODEL_DEPTH=7

UIDS=$(ruby data/sb_ucd_anal.rb < data/strangebeta_user_climb_data_20100208.txt | cut -f 1 -d '|' | sort -u)
for k in $(seq 1 4);do
  echo "Key $k"
  if [ -f vomm/data/vomm_${k}.ser ];then
    rm vomm/data/vomm_${k}.ser
  fi
  ruby vomm_train.rb $k
  for uid in $UIDS;do
    echo " -> User $uid"
    if [ -f vomm/data/vomm_${k}_${uid}.ser ];then
      rm vomm/data/vomm_${k}_${uid}.ser
    fi
    ruby vomm_train.rb $k $uid $LANG_SIZE $MODEL_DEPTH
  done
done
