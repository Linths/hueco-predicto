#!/usr/bin/env ruby

require_relative 'grades'

SymbolicData = "phoenix/app/strangebeta/symbolic.txt" # Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
Grades = getGrades()

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

        # Working on a different route
        if on_rid != rid
            # Make an entry if it doesn't exist yet
            if !sequences.key?(rid)
                sequences[rid] = [[], [], [], []]
            end
            on_rid = rid
        end

        # For every move, add a symbol to the sequence -- done for every symbol set
        (1..4).each { |k|
            symbol = [k1, k2, k3, k4][k-1].to_i
            # Append symbol
            # if k == 1 && SymbolReplaceSet1.key?(symbol)
            #     symbol = SymbolReplaceSet1[symbol]
            # end
            sequences[rid][k-1][mid-1] = symbol
        }
    }
}

# puts sequences

# Count the amount of data points a grade has
count_data = {"easy"=>0, "medium"=>0, "hard++"=>0}
sequences.each do |rid,seqs|
    seq = seqs[0]
    count_data[getGradeClass(Grades[rid])] += seq.length() - 2
    # puts(rid)
    # puts("#{seqs}")
    # puts("#{seq}")
    # puts(seq)
end

puts count_data