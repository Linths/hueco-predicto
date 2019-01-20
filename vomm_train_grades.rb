#!/usr/bin/env ruby

# - Training data -
# Symbols (numbers) are bound to every move, for every symbol set (k1..k4).
SymbolicData = "phoenix/app/strangebeta/symbolic.txt"
# Mapping of symbols (numbers) to semantics 
SymbolsFile = "phoenix/app/strangebeta/symbols.txt"
# Mapping of grades to routes
GradesFile = "data/route_grade.csv"

# - Params -
WhichSymbolSet = ARGV[0].nil? ? 1 : ARGV[0].to_i
LangSize = ARGV[1].to_i
ModelDepth = ARGV[2].to_i

# Returns a symbol-set-specific and grade-specific model file
def get_model_file(grade)
    return "vomm/data/grades/set_#{WhichSymbolSet}/vomm_#{grade}.ser"
end

# Build route_id-grade pairs
grades = {}
File.open(GradesFile, "r") { |file|
    # puts "#{file}"
    # Remove header of file
    file.gets
    file.each_line{ |l|
        # puts "This is a line: #{l}"
        # route_id,grade,type
        # 1,5.10,Sport 
        rid, grade, type = l.chomp.split(/,/)
        # puts "#{rid}, #{grade}, #{type}"
        rid = rid.to_i
        # Clean up Hueco and YSD grade notation to integers only
        # f.e. v6/7 -> V6, 5.10D -> 5.10, etc
        grade = grade.upcase[/(V|5.)[0-9]+/]
        grades[rid] = grade
    }
}
puts "grades = #{grades}"

# Determines the matching grade class for a grade 
def get_grade_class(grade)
    # puts "input getgradeclass #{grade}"
    if grade[0] == 'V'
        # Hueco scale. V[0-9]+
        g = grade[1..grade.length()-1].to_i
        # puts "g = #{g}"
        if g < 4
            return "V0-V3"
        elsif g == 4
            return "V4"
        else
            return "V5-V10"
        end
    else 
        # YSD scale. 5.[0-9]+
        g = grade[2..grade.length()-1].to_i
        # puts "g = #{g}"
        if g < 10
            return "5.8-5.10"
        else
            return "5.11-5.12"
        end
    end
    return 
end

# # Throw away old model
# if File.exists? ModelFile
#     $stderr.puts "#{ModelFile} already exists. I'm going to make you delete it manually..."
#   exit
# end

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
        symbol = [k1, k2, k3, k4][WhichSymbolSet-1]
        grade = grades[rid]
        # puts "grade = #{grade}"

        # # If the route has no grade, skip
        # if grade == nil
        #     # Show message for skipped route just once
        #     if rid != last_skip
        #         puts "Skip #{rid}, no grade"
        #         last_skip = rid
        #     end
        #     next
        # end

        # When entering a totally new route
        if rid != on_rid
            puts "Learning route #{rid} (#{grade})"
            ph.close unless ph.nil?
            grade_class = get_grade_class(grade)
            # puts "class #{grade_class}"
            model_file = get_model_file(grade_class)
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