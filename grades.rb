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

# Mapping of grades to routes
GradesFile = "data/route_grade.csv"
MergeMode = true

# Determines the matching grade class for a grade
# Based on the merge argument, the boulder and climb grades are merged or saved separately
def getGradeClass(grade, merge=MergeMode)
    if merge
        return getGradeClassMerged(grade)
    else
        return getGradeClassSplit(grade)
    end
end

def getGradeClassSplit(grade)
    # Define exclusive grade classes
    bc0 = [(0..0),(1..1),(2..2),(3..3),(4..4),(5..5),(6..6),(7..7),(8..8),(9..9),(10..10)]
    bc1 = [(0..3),(4..4),(5..10)]
    cc0 = [(7..7),(8..8),(9..9),(10..10),(11..11),(12..12)]
    cc1 = [(7..10),(11..12)]
    bc2 = [(0..1),(2..3),(4..5),(6..7),(8..10)]
    bc3 = [(0..4),(5..10)]
    boulder_classes = bc1
    climb_classes = cc1
    # puts "input getgradeclass #{grade}"
    if isBoulderGrade?(grade)
        # Hueco scale. V[0-9]+
        g = grade[1..grade.length()-1].to_i
        boulder_classes.each { |boulder_class|
            if boulder_class.include?(g)
                return "V" + boulder_class.begin.to_s + "-V" + boulder_class.end.to_s
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

def getGradeClassMerged(grade)
    bc0 = {(0..0)=>"easy",(1..3)=>"medium",(4..6)=>"hard",(7..10)=>"expert"}
    bc1 = {(0..0)=>"easy",(1..3)=>"medium",(4..10)=>"hard++"}
    bc2 = {(0..3)=>"easymed",(4..10)=>"hard++"}
    bc3 = {(0..0)=>"easymed2",(1..10)=>"hard++2"}
    cc0 = {(7..10)=>"easy",(11..11)=>"medium",(12..12)=>"hard"}
    cc1 = {(7..10)=>"easy",(11..11)=>"medium",(12..12)=>"hard++"}
    cc2 = {(7..11)=>"easymed",(12..12)=>"hard++"}
    cc3 = {(7..10)=>"easymed2",(11..12)=>"hard++2"}
    boulder_conversion = bc2
    climb_conversion = cc2
    if isBoulderGrade?(grade)
        # Hueco scale. V[0-9]+
        g = grade[1..grade.length()-1].to_i
        boulder_conversion.keys.each { |boulder_class|
            # puts boulder_class
            if boulder_class.include?(g)
                return boulder_conversion[boulder_class]
            end
        }
    else
        # YSD scale. 5.[0-9]+
        g = grade[2..grade.length()-1].to_i
        climb_conversion.keys.each { |climb_class|
            # puts climb_class
            if climb_class.include?(g)
                return climb_conversion[climb_class]
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
        # Remove header of file
        file.gets
        file.each_line{ |l|
            # route_id,grade,type
            # 1,5.10,Sport 
            rid, grade, type = l.chomp.split(/,/)
            rid = rid.to_i
            # Clean up Hueco and YSD grade notation to integers only
            # f.e. v6/7 -> V6, 5.10D -> 5.10, etc
            grade = grade.upcase[/(V|5.)[0-9]+/]
            if not grade.nil?
                grades[rid] = grade
            end
        }
    }
    return grades
end

def getModelFile(grade_class, symbol_set, identifier=nil)
    if identifier.nil?
        return "vomm/data/grades/set_#{symbol_set}/vomm_#{grade_class}.ser"
    else
        return "vomm/data/grades/#{identifier}/set_#{symbol_set}/vomm_#{grade_class}.ser"
    end
end

# symbol_set {1..4}
def getModelFileFromGrade(grade, symbol_set, identifier=nil)
    return getModelFile(getGradeClass(grade), symbol_set, identifier)
end

# Splits a list of models into bouldering and climbing categories
# Returns [[bouldering_models] [climbing_models]]
def splitModelFiles(model_files)
    if MergeMode
        return [model_files,[]]
    else
        return model_files.partition { |m| m.split("/")[-1][5] == 'V' }
    end
end

def splitGradeClasses(grade_classes)
    if MergeMode
        return [grade_classes,[]]
    else
        return grade_classes.partition { |m| m[0] == 'V' }
    end
end

def isMergeMode?()
    return MergeMode
end

def fileToGradeClass(model_file)
    return model_file.split("/")[-1][5..-5]
end
