#!/usr/bin/env ruby

# Mapping of symbols to human-readable text
HumanFile = "phoenix/app/strangebeta/human.txt"

# Build mapping
def getHuman()
    human = [{},{},{},{}]
    File.open(HumanFile, "r") { |file|
        file.each_line{ |l|
            # 1 10 pocket
            # 1 11 pinch
            set, symbol, meaning = l.chomp.split(' ', 3)
            set = set.to_i
            symbol = symbol.to_i
            # puts "#{set}, #{symbol}, #{meaning}"
            human[set-1][symbol] = meaning
        }
    }
    return human
end