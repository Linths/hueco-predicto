#!/usr/bin/env ruby

require_relative 'grades'
require_relative 'matrix'

N = ARGV[0].to_i

# Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
SymbolicData = "phoenix/app/strangebeta/symbolic.txt"

# Make a random selection of test | train
AllRids = `ruby data/sb_ucd_anal.rb < data/strangebeta_user_climb_data_20100208.txt | cut -f 2 -d '|' | sort -u`.split("\n").map{|rid| rid.to_i}
TestRids = AllRids.sample(N)
puts "Random test set = #{TestRids}"

# Train models
puts "\n\n--- Training ---"
system("./vomm_train_grades.sh #{TestRids.join(',')}")

# SymbolSequence contains per route and per symbolset a symbol sequence
sequences = Hash.new(Array.new)
routeSequences = ["", "", "", ""] 
File.open(SymbolicData, "r") { |file|
    on_rid = nil
    file.gets
    file.each_line{ |l|
        # uid rid mid hand crux k1 k2 k3 k4 hold.goodness cross big.move match
        # 1 1 1 right false 1 1 1 1 0 false false false
        uid, rid, mid, hand, crux, k1, k2, k3, k4, good, cross, big, match = l.chomp.split(/\s+/)
        rid = rid.to_i

        # Properly ending a route of which all moves have been saved
        if on_rid != rid && routeSequences != ["", "", "", ""]
            # Remove the last unnecessary separator of every symbol sequence
            sequences[on_rid] = routeSequences.map{|s| s.chop}
            routeSequences = ["", "", "", ""]
        end

        on_rid = rid

        # Only save moves for routes in the test set
        next unless TestRids.include? rid
        
        # For every move, add a symbol to the sequence -- done for every symbol set
        (1..4).each { |k|
            symbol = [k1, k2, k3, k4][k-1]
            # Append symbol
            routeSequences[k-1] += symbol + ","
        }
    }
}
# puts "Sequences #{sequences}"

puts "\n\n--- Testing ---"

# Logeval per test route for every model: every symbol set * every grade class
# Make two confusion matrices per symbol set: one for bouldering, one for climbing
grades = getGrades()
puts grades
allModels = []
(1..4).each { |k|
    puts "\n- Set #{k} -"
    allModels = `find vomm/data/grades/set_#{k} -name '*ser'`.split("\n")
    # Split models in type of routes: 0. bouldering, 1. climbing
    models = splitModelFiles(allModels)
    puts "\n#{models}"
    # Creates empty confusion matrices: 0. bouldering, 1. climbing
    confusions = models.map {|m| getEmptyMatrix(m.length())}
    # Calculate log losses for every route
    TestRids.each { |rid|
        grade = grades[rid]
        loglosses = {}
        seq = sequences[rid][k-1]
        route_type = isBoulderGrade?(grade) ? 0 : 1
        models[route_type].each { |m|
            # chop: Remove unnecessary newline at end
            loglosses[m] = `./logeval.sh #{seq} #{m}`.chop.to_f
        }
        # Take the model that yields the smallest logloss
        predicted = loglosses.min_by{|k,v| v}[0]
        pred_index = models[route_type].index(predicted)
        # puts "predicted #{predicted}, #{pred_index}"
        actual = getModelFileFromGrade(grade, k)
        act_index = models[route_type].index(actual)
        # puts "\n#{rid} #{actual}: #{predicted}\nloglosses #{loglosses}"
        # puts "actual #{actual}, #{act_index}"
        confusions[route_type][act_index][pred_index] += 1
    }
    # Print results
    confusions.each {|m| 
        printMatrix(m)
        accuracy = totalCorrectlyClassified(m) / TestRids.length().to_f
        puts "accuracy = #{accuracy}"
    }
}