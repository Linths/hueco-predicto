#!/usr/bin/env ruby

SymbolicData="phoenix/app/strangebeta/symbolic.txt"
SymbolsFile="phoenix/app/strangebeta/symbols.txt"
WhichSymbolSet=ARGV[0].nil? ? 1 : ARGV[0].to_i
WhichUserId=ARGV[1].nil? ? nil : ARGV[1].to_i
ModelFile="vomm/data/vomm_#{WhichSymbolSet}#{WhichUserId.nil? ? "" : "_"+WhichUserId.to_s}.ser"
LangSize=ARGV[2].to_i
ModelDepth=ARGV[3].to_i

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
