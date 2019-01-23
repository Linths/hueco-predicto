#!/usr/bin/env ruby

# Mapping of grades to routes
GradesFile = "data/route_grade.csv"

# Determines the matching grade class for a grade 
def getGradeClass(grade)
    # Define exclusive grade classes
    boulder_classes = [(0..3),(4..4),(5..10)]
    climb_classes = [(8..10),(11..12)]
    # puts "input getgradeclass #{grade}"
    if isBoulderGrade?(grade)
        # Hueco scale. V[0-9]+
        g = grade[1..grade.length()-1].to_i
        boulder_classes.each { |bc|
            if bc.include?(g)
                return "V" + bc.begin.to_s + "-V" + bc.end.to_s
            end
        }
    else 
        # YSD scale. 5.[0-9]+
        g = grade[2..grade.length()-1].to_i
        climb_classes.each { |cc|
            if cc.include?(g)
                return "5." + cc.begin.to_s + "-5." + cc.end.to_s
            end
        }
    end
end

# Determines if the grade is for a boulder route or a climbing route
# Assuming two systems are used: Hueco (bouldering) and YDS (climbing)
def isBoulderGrade?(grade)
    return grade[0] == 'V'
end

# Build route_id-grade pairs
def getGrades()
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
    return grades
end

def getModelFile(grade_class, symbol_set)
    return "vomm/data/grades/set_#{symbol_set}/vomm_#{grade_class}.ser"
end

def getModelFileFromRoute(rid, symbol_set)
    return getModelFile(getGradeClass(getGrades()[rid]), symbol_set)
end

def getModelFileFromGrade(grade, symbol_set)
    return getModelFile(getGradeClass(grade), symbol_set)
end

# Splits a list of models into bouldering and climbing categories
# Returns [[bouldering_models] [climbing_models]]
def splitModelFiles(model_files)
    return model_files.partition { |m| m.split("/")[-1][5] == 'V' }
end