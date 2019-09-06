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
  "diagnol" => "diagonal",
  "ledgematch" => "ledge match",
  "crimppinch" => "crimp pinch",
  "flakecling" => "flake cling",
  "crimpslopper" => "crimp sloper",
  "gastone" => "gaston",
  "sidepul" => "sidepull",
  "rignt" => "right",
  "slop" => "sloper",
  "crinp" => "crimp",
  "crimpsidepull" => "crimp sidepull",
  "slipper" => "sloper",
  "knee drops" => "dropknee",
  "thumbcatch" => "thumb catch",
  "drop knee" => "dropknee"
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
