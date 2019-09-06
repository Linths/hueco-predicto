#!/usr/bin/env ruby

=begin
Strange Beta: An Assistance System for Indoor Rock Climbing Route Setting
Copyright (C) 2011  Caleb Philips, Lee Becker, Elizabeth Bradley
Website: http://strangebeta.com
Article: https://aip.scitation.org/doi/10.1063/1.3693047

Modifications are made by Lindsay Kempen (2019)
2019 January & February  Modified and documented existing code
2019 January & February  Used parts of (altered) code in other files

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

require 'move'

AmbiguousParseChoice = 0 # always pick the zero^th parse
SymbolOutFile = ARGV[0]

File.open(SymbolOutFile,"r"){ |fh|
  fh.each_line{ |l|
    k,i,s = l.chomp.split(/\s+/)
    i = i.to_i
    k = k.to_i
    $symbols[k][s] = i
  }
}

parsing = nil
last_parsing = nil
last_move = nil
move = nil
parse_num = 0

$stdin.each_line{ |l|
   if l =~ /^\;{1,3} (.*)$/
     parsing = $1
     next
   end
   if l =~ /^No parse$/ and !parsing.nil?
     puts "0 0 0 0 0 false false false"
     parsing = nil
     next
   end
   if l =~ /^PARSE_(\d+)/ or l =~ /^\s*$/
     parse_num = $1.to_i
     next
   end
   if l =~ /^IsMove:(.*)$/ and parse_num == AmbiguousParseChoice
     move = Move.new if move.nil?
     parse = $1
     if parse =~ /\[Match\]/
       move.match = true
     else
       if parse =~ /(\[HoldType\][^\s]+)/
         move.type.push $1
       end
       if parse =~ /(\[HoldSize\][^\s]+)/
         move.size = $1
       end
       if parse =~ /(\[HoldShape\][^\s]+)/
         move.shape = $1
       end
       if parse =~ /\[Cross\]/
         move.cross = true
       end
       if parse =~ /\[ActionVerbBig\]/
         move.big = true
       end
     end
   end
   if l =~ /^END_PARSE/
     next if parse_num != AmbiguousParseChoice
     puts move.to_s
     last_move = move
     last_parsing = parsing
     parsing = nil
     parse_num = 0
     move = nil
   end
}
