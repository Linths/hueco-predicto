#!/usr/bin/env ruby

require_relative 'grades'
require_relative 'matrix'
require_relative 'human'

# --- Preparation ---

N = ARGV[0].to_i
ModelDepth = ARGV[1].to_i
LangSize = 256
puts "\n\n---\ndepth = #{ModelDepth}"
# Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
SymbolicData = "phoenix/app/strangebeta/symbolic.txt"
Grades = getGrades()
Human = getHuman()
# puts Human
# puts Grades

# SymbolSequence contains per route and per symbolset a symbol sequence
# NB: SymbolsFile contains routes which are not necessary in the right order of moves, some even interrupting each other
sequences = Hash.new(Array.new)
# routeSequences = nil
File.open(SymbolicData, "r") { |file|
    on_rid = nil
    file.gets
    file.each_line{ |l|
        # uid rid mid hand crux k1 k2 k3 k4 hold.goodness cross big.move match
        # 1 1 1 right false 1 1 1 1 0 false false false
        uid, rid, mid, hand, crux, k1, k2, k3, k4, good, cross, big, match = l.chomp.split(/\s+/)
        rid = rid.to_i
        mid = mid.to_i

        # Properly ending a route of which all moves have been saved
        if on_rid != rid
            # Working on a different route, make an entry if it doesn't exist yet
            if !sequences.key?(rid)
                sequences[rid] = [[], [], [], []]
            end
            on_rid = rid
        end

        # For every move, add a symbol to the sequence -- done for every symbol set
        (1..4).each { |k|
            symbol = [k1, k2, k3, k4][k-1].to_i
            # Append symbol
            sequences[rid][k-1][mid-1] = symbol
        }
    }
}
# puts "Sequences #{sequences}"

# Create empty confusion matrices
gradeClassesK = []
confusionsK = []
(1..4).each { |k|
    allGradeClasses = `find vomm/data/grades/set_#{k} -name '*ser'`.split("\n").map{ |f| fileToGradeClass(f)}
    # Split models in type of routes: 0. bouldering, 1. climbing
    gradeClassesK[k-1] = splitGradeClasses(allGradeClasses)
    # puts "\n#{models}"
    # Creates empty confusion matrices: 0. bouldering, 1. climbing
    confusionsK[k-1] = gradeClassesK[k-1].map {|m| getEmptyMatrix(m.length())}
}

# --- Data selection ---

# Make a random selection of test | train
AllRids = sequences.keys()
TestRids = AllRids.sample(N)
puts "test set = #{TestRids} #{TestRids.map {|r| getGradeClass(Grades[r])}}"

# --- Analyse route by route ---

TestRids.each{ |test_rid|
    grade = Grades[test_rid]
    actual = getGradeClass(grade)
    puts "#{test_rid} #{actual}"

    # --- Training ---
    # Train for each model with all routes except one test route
    (1..4).each { |k|
        `rm -rf vomm/data/grades/set_#{k}/*`
        (AllRids - [test_rid]).each { |train_rid|
            model_file = getModelFileFromGrade(Grades[train_rid], k)
            `./learn.sh #{sequences[train_rid][k-1].join(',')} #{model_file} #{LangSize} #{ModelDepth} #{test_rid}`
            # `./vomm_train_grades.sh #{ModelDepth} #{TestRids.join(',')}`
        }
    }

    # --- Testing ---
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
            # chop: Remove unnecessary newline at end
            loglosses[gc] = `./logeval.sh #{seq.join(',')} #{getModelFile(gc, k)}`.chop.to_f
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

# Print results
(1..4).each { |k|
    confusionsK[k-1].each {|m| 
        printMatrix(m)
        printAccuracy(m)
    }
}

# def prettyPrintLoglosses(loglosses)
#     str = ""
#     loglosses.each { |model_file, value|
#         str += makeReadable(model_file) + " = " + v
#     }
#     puts str
# end