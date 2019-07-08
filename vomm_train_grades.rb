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

require_relative 'grades'

# - Training data -
# Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
SymbolicData = "phoenix/app/strangebeta/symbolic.txt"
# Mapping of symbols (numbers) to semantics 
SymbolsFile = "phoenix/app/strangebeta/symbols.txt"

# - Params -
# [SymbolSet] [LangSize] [ModelDepth] [ExcludedRoutes] 
WhichSymbolSet = ARGV[0].nil? ? 1 : ARGV[0].to_i
LangSize = ARGV[1].to_i
ModelDepth = ARGV[2].to_i
ExcludedRoutes = ARGV[3].nil? ? [] : ARGV[3].split(",").map{ |r| r.to_i }

# - Functions -
# Returns a symbol-set-specific and grade-class-specific model file
def getModelFile(grade_class)
    return "vomm/data/grades/set_#{WhichSymbolSet}/vomm_#{grade_class}.ser"
end

# - Program -
grades = getGrades()
puts "Model depth = #{ModelDepth}"
puts "Skipping routes #{ExcludedRoutes}\n#{ExcludedRoutes.map {|r| getGradeClass(grades[r])}}"
# puts "grades = #{grades}"
# Building the model file
File.open(SymbolicData, "r") { |file|
    ph = nil
    on_rid = nil
    last_skip = nil
    # Removing header of file
    file.gets
    file.each_line{ |l|
        # uid rid mid hand crux k1 k2 k3 k4 hold.goodness cross big.move match
        # 1 1 1 right false 1 1 1 1 0 false false false
        uid, rid, mid, hand, crux, k1, k2, k3, k4, good, cross, big, match = l.chomp.split(/\s+/)
        rid = rid.to_i
        
        # Skip routes that are excluded
        if ExcludedRoutes.include? rid
            # puts "Skipping #{rid} [EXCLUDED]"
            next
        end
        
        symbol = [k1, k2, k3, k4][WhichSymbolSet-1]
        grade = grades[rid]
        
        # When entering a totally new route
        if rid != on_rid
            puts "Learning route #{rid} (#{grade})"
            ph.close unless ph.nil?
            grade_class = getGradeClass(grade)
            # puts "class #{grade_class}"
            model_file = getModelFile(grade_class)
            # Start the learning process for a route
            ph = File.popen("java -cp vomm/src Test learn #{model_file} #{LangSize} #{ModelDepth}", "w")
            on_rid = rid
        end
        
        # Add a symbol to the sequence to be learnt by the VOMM
        ph.puts "#{symbol}"
    }
    ph.close unless ph.nil?
}
puts "Done."