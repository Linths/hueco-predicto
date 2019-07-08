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

# GOAL
#   Performs k-fold cross-validation. Classifies climbing routes, in 6 variations.
#   Outputs classifications and confusion matrices for every variation.
# INPUT
#   Climbing routes in symbolic form, along with their difficulty grades.
# ARGUMENTS
#   - k
#   - model depth   Max length of subsequences considered for the probability calculation
#   - identifier    Determines save location for VOMMs (vomm/src/data/grades/<identifier>) 

require_relative 'grades'
require_relative 'matrix'
require_relative 'human'

# --- Preparation ---

StartTime = Time.new() 
puts(StartTime)
SymbolicData = "phoenix/app/strangebeta/symbolic.txt" # Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
K = ARGV[0].to_i
ModelDepth = ARGV[1].to_i
Identifier = ARGV[2]
DoRemove = false #ARGV[3] == "true"
ToBeRemoved = [283, 296, 298, 301, 325, 329, 332, 380, 384, 385] # 10 easy/med routes, to balance out easymed (43-10) | hard++ (33)
LangSize = 400
Grades = getGrades()
Human = getHuman()
Blacklist = [67, 160, 344, 312, 270] # Routes that taint the data set
DoReplace = false
SymbolReplaceSet1 = {13=>18,14=>1,16=>24,19=>49,20=>3,21=>26,25=>23,27=>10,28=>12,30=>12,31=>23,32=>12,33=>3,35=>4,36=>23,37=>17,39=>68,40=>3,41=>3,43=>17,44=>23,45=>24,46=>23,47=>55,48=>23,50=>1,52=>11,53=>23,54=>4,56=>11,57=>12,58=>12,59=>3,5=>11,60=>4,62=>4,64=>34,65=>11,66=>42,67=>18,7=>6,8=>3,9=>17}
SymbolReplaceSet3 = {101=>60,104=>1,106=>66,109=>22,10=>21,11=>19,15=>22,16=>7,18=>31,24=>25,26=>21,27=>34,32=>30,35=>82,37=>14,41=>14,42=>30,46=>14,48=>21,50=>5,51=>30,52=>19,57=>110,58=>21,59=>21,61=>19,62=>30,63=>31,64=>30,65=>83,68=>30,6=>13,71=>91,72=>1,79=>30,81=>5,84=>13,85=>14,86=>102,87=>14,88=>21,89=>5,93=>5,95=>49,97=>13,9=>8}
DoMatchReplace = false
MatchSymbols = [2, 2, 3, 3] # The symbol for 'match' for every set
puts "K = #{K}, depth = #{ModelDepth}"
puts "DoRemove = #{DoRemove}, DoReplace = #{DoReplace}, DoMatchReplace = #{DoMatchReplace}"
puts "blacklist = #{Blacklist} #{Blacklist.map {|r| getGradeClass(Grades[r])}}"
puts ""

# SymbolSequence contains per route and per symbolset a symbol sequence
# NB: SymbolsFile contains routes which are not necessary in the right order of moves, some even interrupting each other
sequences = Hash.new(Array.new)
File.open(SymbolicData, "r") { |file|
    on_rid = nil
    file.gets
    file.each_line{ |l|
        # uid rid mid hand crux k1 k2 k3 k4 hold.goodness cross big.move match
        # 1 1 1 right false 1 1 1 1 0 false false false
        uid, rid, mid, hand, crux, k1, k2, k3, k4, good, cross, big, match = l.chomp.split(/\s+/)
        rid = rid.to_i
        mid = mid.to_i

        # Working on a different route
        if on_rid != rid
            # Make an entry if it doesn't exist yet
            if !sequences.key?(rid)
                sequences[rid] = [[], [], [], []]
            end
            on_rid = rid
        end

        # Only store routes that have been graded
        next unless Grades.key?(rid)

        # For every move, add a symbol to the sequence -- done for every symbol set
        (1..4).each { |k|
            symbol = [k1, k2, k3, k4][k-1].to_i
            # Append symbol
            if DoReplace
                if k == 1 && SymbolReplaceSet1.key?(symbol)
                    symbol = SymbolReplaceSet1[symbol]
                elsif k == 3 && SymbolReplaceSet3.key?(symbol)
                    symbol = SymbolReplaceSet3[symbol]
                end
            end
            sequences[rid][k-1][mid-1] = symbol
        }
    }
}
# puts "Sequences #{sequences}"
# Replace all match moves by the move preceding it.
if DoMatchReplace
    sequences.each { |rid,seqs|
        (1..4).each { |k|
            seq = seqs[k-1]
            # puts "#{seq}"
            (1..(seq.length()-1)).each { |i|
                if seq[i] == MatchSymbols[k-1]
                    seq[i] = seq[i-1]
                end
            }
        }
    }
end
# puts "Sequences #{sequences}"

# --- Data selection ---

# Make a random selection of test | train
GradedSelection = sequences.keys().find_all { |r| Grades.key?(r) }
AllRids = DoRemove ? GradedSelection - Blacklist - ToBeRemoved : GradedSelection - Blacklist
GroupSize = (AllRids.length() / K.to_f).ceil()
TestGroups = AllRids.each_slice(GroupSize).to_a
puts "whole set = #{AllRids} #{AllRids.map {|r| getGradeClass(Grades[r])}} #{AllRids.map {|r| Grades[r]}}"
puts "test groups = #{TestGroups} #{TestGroups.map{|group| group.map{|r| getGradeClass(Grades[r])}}}"

# --- Analyse route by route ---
gradeClassesK = []
confusionsK = []

trap "SIGINT" do
    puts "Interrupted"
    if !confusionsK.nil?
        (1..4).each { |k|
            confusionsK[k-1].each {|m| 
                printMatrix(m)
                printAccuracy(m)
            }
        }
    end
    exit 130
end

TestGroups.each{ |test_group|

    # --- Training ---
    # Train for each model with all routes except the test group
    puts "Training #{(AllRids - test_group)}"
    (1..4).each { |k|
        `rm -rf vomm/data/grades/#{Identifier}/set_#{k}/*`
        `mkdir -p vomm/data/grades/#{Identifier}/set_#{k}`
        (AllRids - test_group).each { |train_rid|
            model_file = getModelFileFromGrade(Grades[train_rid], k, Identifier)
            # It's a bit hacky to not directly call the Java VOMM learn method.
            # I don't remember the reason anymore, but it was most probably due
            # to how the sequence strings are formatted.
            `./learn.sh #{sequences[train_rid][k-1].join(',')} #{model_file} #{LangSize} #{ModelDepth} #{test_group}`
            # `./vomm_train_grades.sh #{ModelDepth} #{TestGroups.join(',')}`
        }
    }

    # --- Testing ---
    # Test every route in the test group
    test_group.each{ |test_rid|
        grade = Grades[test_rid]
        actual = getGradeClass(grade)
        puts "#{test_rid} #{actual} (#{grade})"
        
        # Create empty confusion matrices
        if gradeClassesK == [] || confusionsK == []
            (1..4).each { |k|
                allGradeClasses = `find vomm/data/grades/#{Identifier}/set_#{k} -name '*ser'`.split("\n").map{ |f| fileToGradeClass(f)}
                # Split models in type of routes: 0. bouldering, 1. climbing
                gradeClassesK[k-1] = splitGradeClasses(allGradeClasses)
                # Creates empty confusion matrices: 0. bouldering, 1. climbing
                confusionsK[k-1] = gradeClassesK[k-1].map {|m| getEmptyMatrix(m.length())}
            }
        end

        # Log loss for every model: every symbol set * every grade class
        (1..4).each { |k|  
            # Calculate log losses for every route
            seq = sequences[test_rid][k-1]
            puts "#{seq.join(',')}\t#{seq.map{|symb| Human[k-1][symb.to_i]}.join(',')}"
            gradeClasses = gradeClassesK[k-1]
            confusions = confusionsK[k-1]
            loglosses = {}
            route_type = isMergeMode?() ? 0 : isBoulderGrade?(grade) ? 0 : 1
            # puts "models #{models}"
            # puts "route_type #{route_type}"
            gradeClasses[route_type].each { |gc|
                # 1) It's a bit hacky to not directly call the Java VOMM logeval method.
                #    I don't remember the reason anymore, but it was most probably due
                #    to how the sequence strings are formatted.
                # 2) chop: Remove unnecessary newline at end
                loglosses[gc] = `./logeval.sh #{seq.join(',')} #{getModelFile(gc, k, Identifier)}`.chop.to_f
            }
            # Take the model that yields the smallest logloss
            predicted = loglosses.min_by{|k,v| v}[0]
            pred_index = gradeClasses[route_type].index(predicted)
            # puts "predicted #{predicted}, pred_index #{pred_index}"
            act_index = gradeClasses[route_type].index(actual)
            # puts "actual #{actual}, act_index #{act_index}"
            puts "#{loglosses}\n -> #{predicted}"
            # prettyPrintLoglosses()
            confusions[route_type][act_index][pred_index] += 1
        }
    }
}

# Print results
(1..4).each { |k|
    confusionsK[k-1].each {|m| 
        printMatrix(m)
        printAccuracy(m)
    }
}

puts("\nDone")
EndTime = Time.new()
puts("#{EndTime}")
puts ("Took #{((EndTime-StartTime)/60).to_i} mins")