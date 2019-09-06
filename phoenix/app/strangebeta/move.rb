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

$symbols = {1 => {}, 2 => {}, 3 => {}, 4 => {}}

class Move
  def initialize
    @type = []
    @shape = nil
    @size = nil
    @cross = false
    @big = false
    @match = false
  end

  def remove_terminal(t)
    if t =~ /^(.*\]).([a-z]+)$/
      return $1
    else
      $stderr.puts "Can't remove terminal from #{t}!?"
      exit
    end
  end

  def k1
    # hold key is sorted list of hold types (i.e. "[...].sloper [...].crimp" becomes "[...].crimp.[...].sloper"
    # we must remove the terminal from those holds with these non-terminals to avoid having "lay back" and "layback"
    # be treated as two different holds (for instance)
    #([UnderCling])
    #([SidePull])
    #([FootHook])
    #([GenericHold])
    #([Layback])
    #([Mantle])
    #([Jib])
    return "[HoldType].[Match]" if @type.length == 0 and @match
    ret = @type.uniq.collect{ |e|
       if e =~ /(UnderCling|SidePull|DownPull|FootHook|GenericHold|Layback|Mantle|Jib)/
         if e =~ /GenericHold/ and @type.length > 1
           nil
         else
           remove_terminal(e)
         end
       else
         e
       end
    }.compact.sort.join(".")
    return "[HoldType].[GenericHold]" if ret == ""
    return ret
  end

  def k2
    # Remove the terminals from descriptions
    # --- Uncomment below to adjust symbol set 2 and 4 to 2* and 4* --- 
    # if @size =~ /HoldSizeBig/
    #   @size = "[HoldSize].[HoldSizeBig].big"
    # elsif @size =~ /HoldSizeSmall/
    #   @size = "[HoldSize].[HoldSizeSmall].small"
    # end
    # if @shape =~ /HoldShapeGood/
    #   @shape = "[HoldShape].[HoldShapeGood].good"
    # elsif @shape =~ /HoldShapeBad/
    #   @shape = "[HoldShape].[HoldShapeBad].bad"
    # end
    # --- End of uncomment ---
    [k1,@size,@shape].join(":")
  end

  def bools
    r = [@cross ? "CROSS" : nil,@big ? "BIG" : nil,(goodness > 0) ? "GOOD" : nil].compact.join("+")
    r == "" ? nil : r
  end

  def k3
    [k1,bools].compact.join(":")
  end

  def k4
    [k2,bools].compact.join(":")
  end

  # uses least significant N bits as a bitstring for boolean numbers
  # def full_key(hk,bools)
  #   (hk << bools.length) + bools.collect{ |torf| torf ? "1" : "0" }.join("").to_i(2)
  #   end
  #

  # FIXME: this is very arbitrary
  def goodness
    g = 0
    g += 1 if @size =~ /HoldSizeBig/
    g -= 1 if @shape =~ /HoldShapeBad/
    g += 1 if @shape =~ /HoldShapeGood/
    g -= 1 if @size =~ /HoldSizeSmall/
    # consider @cross or @big?
    return g
  end

  def update_symbols
    $symbols[1][k1] = (($symbols[1].length == 0) ? 1 : $symbols[1].values.max + 1) if $symbols[1][k1].nil?
    $symbols[2][k2] = (($symbols[2].length == 0) ? 1 : $symbols[2].values.max + 1) if $symbols[2][k2].nil?
    $symbols[3][k3] = (($symbols[3].length == 0) ? 1 : $symbols[3].values.max + 1) if $symbols[3][k3].nil?
    $symbols[4][k4] = (($symbols[4].length == 0) ? 1 : $symbols[4].values.max + 1) if $symbols[4][k4].nil?
    @s1 = $symbols[1][k1]
    @s2 = $symbols[2][k2]
    @s3 = $symbols[3][k3]
    @s4 = $symbols[4][k4]
  end

  def to_s
    update_symbols
    return [@s1,@s2,@s3,@s4,goodness,@cross,@big,@match].join(" ")
  end

  attr_writer :type,:size,:shape,:cross,:big,:match
  attr_reader :type
end
