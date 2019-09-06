#!/bin/bash

# ========== HEADER =======================================================
# Strange Beta: An Assistance System for Indoor Rock Climbing Route Setting
# Copyright (C) 2011  Caleb Philips, Lee Becker, Elizabeth Bradley
# Website: http://strangebeta.com
# Article: https://aip.scitation.org/doi/10.1063/1.3693047
# 
# Modifications are made by Lindsay Kempen (2019)
# 2019 January & February  Modified and documented existing code
# 2019 January & February  Used parts of (altered) code in other files
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

LANG_SIZE=256
MODEL_DEPTH=7

UIDS=$(ruby data/sb_ucd_anal.rb < data/strangebeta_user_climb_data_20180128.txt | cut -f 1 -d '|' | sort -u)
for k in $(seq 1 4);do
  echo "Key $k"
  if [ -f vomm/data/vomm_${k}.ser ];then
    rm vomm/data/vomm_${k}.ser
  fi
  ruby vomm_train.rb $k $LANG_SIZE $MODEL_DEPTH
  for uid in $UIDS;do
    echo " -> User $uid"
    if [ -f vomm/data/vomm_${k}_${uid}.ser ];then
      rm vomm/data/vomm_${k}_${uid}.ser
    fi
    ruby vomm_train.rb $k $LANG_SIZE $MODEL_DEPTH $uid
  done
done
