#!/usr/bin/env ruby

n = ARGV[0].to_i

# Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
SymbolicData = "phoenix/app/strangebeta/symbolic.txt"

# Make a random selection of test | train
AllRids = `ruby data/sb_ucd_anal.rb < data/strangebeta_user_climb_data_20100208.txt | cut -f 2 -d '|' | sort -u`.split("\n").map{|rid| rid.to_i}
puts "#{AllRids}"
TestRids = AllRids.sample(n)
puts "Test set: #{TestRids}"

# Train models
# system("./vomm_train_grades.sh #{TestRids.join(',')}")

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
puts "Sequences #{sequences}"

# Logeval per test route for every model: every symbol set * every grade class
models = []
TestRids.each { |r|
    puts "route #{r}"
    (1..4).each { |k|
        puts "set #{k}"
        # Make a confusion matrix per symbol set
        models = `find vomm/data/grades/set_#{k} -name '*ser'`.split("\n")
        # puts "#{models}"
        logloss = {}
        models.each { |m|
            seq = sequences[r][k-1]
            # chop: Remove unnecessary newline at end
            logloss[m] = `./logeval.sh #{seq} #{m}`.chop
        }
        puts "logloss #{logloss}"
    }
}

# choose minimum one as winner
# print confusion matrices
