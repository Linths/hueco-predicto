#!/usr/bin/env ruby

#user_id | route_id | seq_no | hand |                                                           move                                                            | crux 
#--------+----------+--------+------+---------------------------------------------------------------------------------------------------------------------------+------
#      1 |        1 |      1 | R    | Matched on jug                                                                                                            | f


spellcheck = {
  "crip" => "crimp",
  "bumb" => "bump",
  "sixepull" => "sidepull",
  "ladge" => "ledge",
  "slopper" => "sloper",
  "incutpinch" => "incut pinch",
  "sidepullundercling" => "sidepull undercling",
  "pinchslopper" => "pinch sloper",
  "diagnol" => "diagonal"
}

n = 0
$stdin.each_line{ |l|
  n += 1
  next if n <= 2
  break if l =~ /^\(\d+ rows\)/
  uid,rid,mid,hand,move,crux = l.split(/\|/).collect{ |e| e.strip }
  uid,rid,mid = [uid,rid,mid].collect{ |e| e.to_i }
  hand = (hand == "R") ? :right : :left
  crux = (crux == "f") ? false : true

  move = move.downcase.tr('-',' ').tr('^a-z0-9 ','').split(/\s+/).collect{ |e| spellcheck[e].nil? ? e : spellcheck[e] }.join(" ")
  puts [uid,rid,mid,hand,crux,move].join("|")
}
