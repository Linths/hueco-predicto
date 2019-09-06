#!/usr/bin/env ruby

=begin
Strange Beta: An Assistance System for Indoor Rock Climbing Route Setting
Copyright (C) 2011  Caleb Philips, Lee Becker, Elizabeth Bradley
Website: http://strangebeta.com
Article: https://aip.scitation.org/doi/10.1063/1.3693047

Modifications are made by Lindsay Kempen (2019)
2019 January & February  Modified and documented this file
2019 January & February  Used (altered) parts of this file in other files

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

SymbolicData="phoenix/app/strangebeta/symbolic.txt"
SymbolsFile="phoenix/app/strangebeta/symbols.txt"
WhichSymbolSet=ARGV[0].nil? ? 1 : ARGV[0].to_i
LangSize=ARGV[1].to_i
ModelDepth=ARGV[2].to_i
WhichUserId=ARGV[3].nil? ? nil : ARGV[3].to_i
ModelFile="vomm/data/vomm_#{WhichSymbolSet}#{WhichUserId.nil? ? "" : "_"+WhichUserId.to_s}.ser"

if File.exists? ModelFile
  $stderr.puts "#{ModelFile} already exists. I'm going to make you delete it manually..."
  exit
end

class String
  def to_b
    self == "true" ? true : false
  end
end

# uses least significant N bits as a bitstring for boolean numbers
def full_key(hk,bools)
  (hk << bools.length) + bools.collect{ |torf| torf ? "1" : "0" }.join("").to_i(2)
end

File.open(SymbolicData,"r"){ |fh|
  ph = nil
  on_rid = nil
  fh.gets # throw away header
  fh.each_line{ |l|
    # uid rid mid hand crux k1 k2 k3 k4 hold.goodness cross big.move match
    # 1 1 1 right false 1 1 1 1 0 false false false
    uid,rid,mid,hand,crux,hk1,hk2,hk3,hk4,good,cross,big,match = l.chomp.split(/\s+/)
    rid = rid.to_i
    uid = uid.to_i
    next unless WhichUserId.nil? or (uid == WhichUserId)
    sym = [hk1,hk2,hk3,hk4][WhichSymbolSet-1]
    if rid != on_rid
      puts "Learning Route #{rid}"
      ph.close unless ph.nil?
      ph = File.popen("java -cp vomm/src Test learn #{ModelFile} #{LangSize} #{ModelDepth}","w")
      on_rid = rid
    end
    ph.puts "#{sym}"
  }
}
puts "Done."
