#!/usr/bin/env ruby

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
