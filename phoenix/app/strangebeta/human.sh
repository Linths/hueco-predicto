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

grep "^$1 " symbols.txt | sort -n | tr '[]:.+' ' ' \
   | sed 's/BIG/\(big move\)/' \
   | sed 's/GOOD/\(good hold\)/' \
   | sed 's/CROSS/\(cross\)/' \
   | awk 'gsub(/(HoldTypeT*|HoldSize(Big|Small)T*|HoldShape(Good|Bad)T*)|HoldShapeT*|HoldSizeT*/,"")' \
   | awk 'gsub(/ +/," ")' | tr '[A-Z]' '[a-z]'
