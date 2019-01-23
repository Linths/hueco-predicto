#!/usr/bin/env ruby

# Mapping of grades to routes
GradesFile = "data/route_grade.csv"

# Determines the matching grade class for a grade 
def getGradeClass(grade)
    # puts "input getgradeclass #{grade}"
    if isBoulderGrade?(grade)
        # Hueco scale. V[0-9]+
        g = grade[1..grade.length()-1].to_i
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
        if g < 10
            return "5.8-5.10"
        else
            return "5.11-5.12"
        end
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