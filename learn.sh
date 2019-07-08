#!/bin/bash
# ========== HEADER =======================================================
# A Fair Grade: Assessing Difficulty of Climbing Routes through Machine Learning
# Copyright (C) 2019  Lindsay Kempen
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

# Initiates VOMM learning. Used by the performance_X.rb files.
# Params [sequence] [model] [language_size] [model_depth]
echo $1 | sed "s/,/\n/g" | java -cp vomm/src Test learn $2 $3 $4