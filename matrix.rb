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

def printMatrix(matrix)
    matrix.each {|row| puts row.inspect}
end

def totalCorrectlyClassified(matrix)
    # Matrix is a 'square'
    len = matrix.length()
    sum = 0
    (0..len-1).each { |i|
        sum += matrix[i][i]
    }
    # puts "sum #{sum}"
    return sum
end

def totalSum(matrix)
    sum = 0
    matrix.each { |row|
        row.each { |elem| 
            sum += elem
        }
    }
    return sum
end

def getEmptyMatrix(n)
    Array.new(n){ Array.new(n) {0}}
end

def getAccuracy(matrix)
    return totalCorrectlyClassified(matrix) / totalSum(matrix).to_f
end

def printAccuracy(matrix)
    if totalSum(matrix) != 0.0
        puts "accuracy = #{getAccuracy(matrix)}"
    end
end