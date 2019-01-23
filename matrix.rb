#!/usr/bin/env ruby

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