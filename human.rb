#!/usr/bin/env ruby

=begin
A Fair Grade: Assessing Difficulty of Climbing Routes through Machine Learning
Copyright (C) 2019  Lindsay Kempen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
=end

# Mapping of symbols to human-readable text
HumanFile = "phoenix/app/strangebeta/human.txt"

# Build mapping
def getHuman()
    human = [{},{},{},{}]
    File.open(HumanFile, "r") { |file|
        file.each_line{ |l|
            # 1 10 pocket
            # 1 11 pinch
            set, symbol, meaning = l.chomp.split(' ', 3)
            set = set.to_i
            symbol = symbol.to_i
            # puts "#{set}, #{symbol}, #{meaning}"
            human[set-1][symbol] = meaning
        }
    }
    return human
end