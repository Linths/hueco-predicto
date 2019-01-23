#!/usr/bin/env ruby

def print_matrix(matrix)
    matrix.each {|row| puts row.inspect}
end

def total_correctly_classified(matrix)
    # Matrix is a 'square'
    len = matrix.length()
    sum = 0
    (0..len-1).each { |i|
        sum += matrix[i][i]
    }
    # puts "sum #{sum}"
    return sum
end

def get_empty_matrix(n)
    Array.new(n){ Array.new(n) {0}}
end