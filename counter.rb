#!/usr/bin/env ruby

a = [[1,1,2,3,3,1,4,5,1,5,1,1,2,4,1,6,1,7,8,1,4,9],[1,1,2,3,3,4,5,6,4,6,7,8,2,5,8,9,1,10,11,1,5,12],[1,2,3,4,4,1,5,6,1,6,1,7,3,5,7,8,1,9,10,1,5,11],[1,2,3,4,4,5,6,7,5,7,8,9,3,6,9,10,1,11,12,1,6,13],[1,10,1,4,1,11,1,1,12,13,1,8,1,2,14,10,2,1,15,15,15,16,17],[1,13,1,14,4,15,16,17,18,19,1,11,16,2,20,13,2,1,21,21,21,22,23],[1,12,1,5,1,13,7,7,14,15,1,10,7,3,16,12,3,1,17,17,17,18,19],[1,14,1,15,5,16,17,18,19,20,1,12,17,3,21,14,3,1,22,22,22,23,24],[1,5,8,15,12,1,2,8,3,11,18,1,1,2,11,1,2,11,12,18,18,11,17],[1,6,11,21,18,1,2,11,24,15,25,1,26,2,15,26,2,15,18,25,27,28,23],[1,6,10,17,14,20,3,10,21,13,22,1,7,3,13,7,3,13,14,23,22,13,19],[1,7,12,22,19,25,3,12,26,16,27,1,28,3,16,28,3,16,19,29,30,31,24],[19,2,19,4,4,3,3,11,20,20,21,12,4,2,20,4,22,2,4,3,11,4,1],[29,2,29,30,5,31,24,15,32,32,33,18,5,2,32,34,35,2,5,24,15,5,1],[24,3,24,25,5,4,21,13,26,26,27,14,28,3,26,5,29,3,5,21,13,5,1],[32,3,32,33,6,34,26,16,35,35,36,19,37,3,35,38,39,3,6,26,16,6,1],[11,18,23,11,23,1],[15,36,37,15,37,1],[13,22,30,13,30,1],[16,40,41,16,41,1],[0,17,4,4,23,1,1,24,1,0,0],[0,23,5,5,37,1,1,38,1,0,0],[0,19,5,5,30,1,1,31,1,0,0],[0,24,6,6,41,1,1,42,1,0,0],[4,3,23,4,25,1],[39,24,37,39,40,1],[5,21,30,5,32,2],[43,26,41,43,44,2],[3,4,4,23,24],[24,5,5,37,38],[21,5,5,30,31],[26,6,6,41,42],[1,4,11,20,26,27,2,1,1,1,17],[1,41,42,32,43,44,2,45,1,1,23],[1,25,33,26,34,35,3,1,1,1,19],[1,45,46,35,47,48,3,49,1,1,24],[4,22,28,4,4,4,4,4,22,2,1],[46,47,48,39,49,50,50,41,51,2,1],[5,36,37,5,5,5,28,25,36,3,1],[50,51,52,43,53,54,55,45,56,3,1],[1,1,4,29,11,2,1,11,29,18,23,2,29,22,2,4,4,2,1],[1,1,52,53,15,2,26,54,53,25,55,2,53,56,2,41,41,2,1],[1,1,25,38,13,3,39,33,38,22,40,3,38,29,3,25,25,3,1],[1,1,57,58,16,3,59,60,58,27,61,3,58,62,3,45,45,3,1],[11,2,23,30,23,11,31,1],[57,2,37,58,59,54,60,1],[13,3,30,41,30,33,42,2],[63,3,41,64,65,60,66,2],[24,24,2,4,29,4,1],[61,61,2,62,53,5,1],[43,43,3,5,38,5,1],[67,67,3,68,58,6,1],[22,22,29,4,2,23],[63,63,53,41,2,64],[29,29,38,25,3,40],[69,69,58,45,3,70],[3,3,11,20,1,5,15,12,1,4,20],[31,24,15,32,1,6,21,18,1,5,32],[4,21,13,26,1,6,17,14,20,28,26],[34,26,16,35,1,7,22,19,25,37,35],[4,3,1,3,10,10],[5,24,1,24,65,66],[5,44,1,21,12,12],[6,71,1,26,72,73],[1,1,1,15,1,1],[1,1,1,67,1,1],[2,1,2,45,2,2],[2,1,2,74,2,2],[11,32,8,1,1,4,8,1,24,8,8,20,4,8,23,4,8,1,8,1,1,8,11,33,8,4,20],[68,69,11,70,71,72,11,45,38,73,11,32,72,73,74,75,11,76,11,71,71,73,15,77,11,78,32],[33,46,10,1,1,5,10,20,31,10,47,26,5,47,30,5,10,1,10,1,1,47,13,48,10,5,26],[75,76,12,77,78,79,12,80,42,81,82,35,79,83,84,85,12,86,12,78,78,83,16,87,12,88,35],[18,2,34,20,11,23,4],[27,2,79,32,15,80,5],[22,3,49,26,13,30,5],[30,3,89,35,16,90,6],[38,23,1,1,24,4,23,0,29,38,23,4,1,42,22,29,0,23,17],[86,37,1,1,38,5,37,0,53,86,37,5,1,90,47,53,0,37,23],[53,30,1,20,31,5,54,0,38,55,56,5,2,60,36,38,0,30,19],[96,41,1,25,42,6,97,0,58,98,99,6,2,104,51,58,0,41,24],[12,1,39],[18,1,87],[14,1,57],[19,1,100],[40,41,23,24],[88,89,37,38],[58,59,56,31],[101,102,99,42],[23,18,11,3],[37,27,15,24],[30,23,13,21],[41,103,16,26],[22,22,3,0,1,2,1,1,43],[47,47,24,0,1,2,91,91,92],[36,36,21,0,1,3,1,20,61],[51,51,26,0,1,3,105,106,107],[22,22,1,1,44,45,46,46,46,46,47,17],[47,47,1,1,93,94,95,95,95,95,96,23],[36,36,1,1,62,63,64,64,64,64,65,66],[51,51,1,1,108,109,110,110,110,110,111,112],[4,12,11,23],[39,97,98,99],[5,67,13,40],[43,113,114,115],[3,4,12,23,23,24,4,1],[24,39,18,37,37,38,39,1],[21,5,14,56,30,31,5,1],[26,43,19,99,41,42,43,1],[48,11,4,1],[100,15,5,1],[68,13,5,1],[116,16,6,1],[10,1,4,11,23,1],[13,4,5,15,37,1],[12,1,5,13,54,1],[14,5,6,16,97,1],[11,4,12,34,34,1],[101,5,126,127,127,1],[13,5,14,80,49,2],[117,6,149,150,151,2],[4,4,1,11,4,23,1],[5,5,1,15,5,37,1],[5,5,1,69,28,54,1],[6,6,1,118,37,97,1],[4,10,10,11,4,4,23,3,1,24,12,11,2,1,3,3,49,2,1],[81,13,13,57,81,102,37,103,1,38,18,98,2,1,24,24,104,2,1],[5,12,12,69,5,5,30,21,1,31,14,13,3,2,21,21,70,3,1],[91,14,14,119,91,120,41,121,1,42,19,114,3,2,26,26,122,3,1],[12,8,22,12,22,1],[105,11,47,18,47,1],[14,10,36,14,36,20],[123,12,51,19,51,25],[11,11,4,24,24,4],[15,15,50,38,38,41],[13,13,5,31,31,25],[16,16,54,42,42,45],[23,23,1,0,12,2,17,11,11,24,4,4,1,12,23,0,1,12,11,11,11,11,1,1,49,1,1,17,24],[37,37,1,0,18,2,23,15,15,38,5,5,1,18,37,0,1,18,15,15,15,15,1,1,104,1,1,23,38],[30,30,1,0,14,3,19,13,13,31,5,5,1,14,30,0,1,14,13,13,13,13,1,1,70,1,1,19,31],[41,41,1,0,19,3,24,16,16,42,6,6,1,19,41,0,1,19,16,16,16,16,1,1,122,1,1,24,42],[23,3,10,23,23,9],[106,107,108,37,37,12],[40,4,12,30,30,11],[124,125,126,41,41,13],[22,22,11,36,23,9],[47,47,15,83,37,12],[36,36,69,51,30,71],[51,51,118,93,41,127],[2,2,50,50,11,11,1],[2,2,109,110,101,15,1],[3,3,72,72,69,13,2],[3,3,128,129,130,16,2],[4,4,23,11,4,1,2,3,4,12,23,23,4],[5,5,37,15,5,1,2,24,5,18,37,37,5],[5,5,30,13,28,1,3,21,28,14,30,30,5],[6,6,41,16,37,1,3,26,37,19,41,41,6],[0,24,3,24,2,3,0],[0,38,24,38,2,24,0],[0,31,21,31,3,21,0],[0,42,26,42,3,26,0],[4,3,10,34,3,12,2,4,10,1,2,23],[5,24,65,111,24,18,2,5,65,1,2,37],[5,21,12,49,21,14,3,5,12,1,3,30],[6,26,72,131,26,19,3,6,72,1,3,41],[24,4,1,23,3,24,4,23,1,1,4,24],[38,5,1,37,24,38,5,37,1,1,5,38],[31,5,1,30,21,31,5,30,1,1,5,31],[42,6,1,41,26,42,6,41,1,1,6,42],[12,4,10,51,34,4,4,1],[18,39,108,112,113,114,115,1],[14,5,12,73,49,5,5,2],[19,43,126,132,133,134,135,2],[23,23,18,25,11,11,52,24,11,17],[37,37,27,40,15,15,116,38,15,23],[30,30,22,32,74,13,75,31,13,19],[41,41,30,44,136,16,137,42,16,24],[11,34,11,34,4,0,11,12,1,2,4,4],[15,111,117,118,39,0,119,18,120,2,39,39],[13,49,13,49,5,0,13,14,1,3,5,5],[16,131,138,139,43,0,140,19,141,3,43,43],[4,24,0,40,28,4,18,40,18,4,4,18,23,18,23,23,22,53,17],[5,38,0,88,48,5,121,88,122,5,5,121,55,123,55,55,124,125,23],[5,31,0,58,37,5,76,58,77,5,5,22,40,22,40,78,36,79,19],[6,42,0,101,52,6,142,101,143,6,6,144,61,145,61,146,147,148,24],[3,3],[24,24],[21,21],[26,26],[23,23,23,23,11,2],[37,37,37,37,15,2],[30,30,30,30,13,3],[41,41,41,41,16,3],[1,1,22,23,23,54,54,1,24,4,1,4,4,4,1,1,4,1,1,12,12,1,1,4,1,1],[128,1,124,37,37,129,129,1,38,130,7,5,5,5,120,1,5,120,128,18,18,1,1,75,1,1],[1,1,36,30,30,81,81,1,31,5,1,5,5,5,1,1,5,1,1,14,14,1,1,5,1,1],[152,1,147,41,41,153,153,1,42,154,8,6,6,6,141,1,6,141,152,19,19,1,1,85,1,1],[1,11,12,1,1,12,3,20,23,23],[1,131,18,1,1,18,24,132,37,37],[1,33,14,1,1,14,21,26,30,30],[1,155,19,1,1,19,26,156,41,41],[10,18,2,1],[13,27,2,26],[82,22,3,39],[157,30,3,59],[55,3,23,23,11,56,2,3,57,57,18,1,1,11,4,41,8,55,1,55,11,55,11,17],[133,24,37,37,15,134,2,24,135,135,27,1,1,15,5,89,11,133,1,133,15,133,15,23],[83,21,30,30,13,84,3,21,85,86,22,1,1,13,5,59,47,83,1,83,13,83,13,19],[158,26,41,41,16,159,3,26,160,161,30,1,1,16,6,102,82,158,1,158,16,158,16,24],[1,1,58,11,59,21,23,23,34,4,60,60,1,4,11,4,61,61,4,4,23,3,1,1,4,4,55,23,55,33,17],[1,1,136,137,138,139,37,37,111,140,141,141,120,5,15,5,142,142,143,143,74,24,1,1,5,5,133,37,133,77,23],[1,1,87,13,88,27,30,30,49,5,89,89,1,5,13,5,90,90,5,5,30,21,1,1,5,5,83,30,83,48,91],[1,1,162,163,164,165,41,41,131,166,167,167,141,6,16,6,168,168,169,169,84,26,1,1,6,6,158,41,158,87,170],[23,23,23,4,4,9],[37,37,37,5,5,12],[30,30,30,5,5,11],[41,41,41,6,6,13],[4,4,1,1,4,17],[5,5,1,1,5,23],[5,5,1,1,5,19],[6,6,1,1,6,24],[11,4,24,2,3,3,3,34,34,3,2,4,23],[15,39,144,2,24,145,24,118,147,103,2,39,37],[74,5,92,3,21,21,21,49,80,21,3,5,30],[136,43,171,3,26,172,26,139,174,121,3,43,41],[1,55,12,23,12,62,23,62,23,23,18],[1,133,18,37,18,146,37,146,37,37,27],[1,83,14,30,14,93,30,93,56,30,22],[1,158,19,41,19,173,41,173,99,41,30],[1,1,24,3,1,1,1,42,1,1,23,23,23,57,1,1,4,4,1,1],[45,45,38,24,1,1,148,149,45,45,150,37,37,135,1,1,5,5,1,1],[1,20,31,21,1,1,1,60,1,20,40,30,30,85,20,1,5,5,1,20],[49,80,42,26,1,1,175,176,49,80,177,41,41,160,25,1,6,6,1,25],[23,1,4,29,3,29,4,4,23,63,4,23],[37,1,5,53,24,53,5,5,37,151,5,37],[54,1,28,38,21,38,5,5,54,94,5,30],[97,1,37,58,26,58,6,6,97,178,6,41],[64,2,11,4,11,4,23],[153,2,15,5,15,5,37],[95,3,74,5,13,5,56],[180,3,136,6,16,6,99],[4,29,4,1,4,0,1,4,11,29,24,29,24,4,4],[5,53,5,1,5,0,1,5,15,53,38,53,38,5,39],[5,38,5,1,5,0,1,28,13,38,31,38,31,96,5],[6,58,6,1,6,0,1,37,16,58,42,58,42,181,43],[62,62,23,11,1,17],[154,154,150,15,1,23],[93,93,40,13,1,19],[182,182,177,16,1,24],[65,11,11,22,2,11,11,15,4,53,2,3,24,2,10,10,0,4,23,2,9],[155,54,54,63,2,156,157,21,41,158,2,159,160,2,161,161,0,41,64,2,12],[97,98,33,29,3,33,33,17,25,99,3,21,31,3,12,100,0,25,78,3,11],[183,184,60,69,3,185,186,22,45,187,3,188,189,3,190,191,0,45,192,3,13],[23,2,53,53,3],[55,2,162,162,163],[40,3,79,79,4],[61,3,193,193,194],[10,4,23,11,10,4,12,12,23,10,4,1,17],[164,5,37,165,13,166,18,18,37,65,5,120,23],[12,5,30,13,12,5,14,14,30,12,5,1,19],[195,6,41,196,14,197,19,19,41,72,6,141,24],[4,24,3,66,0],[5,38,24,167,0],[5,31,21,101,0],[6,42,26,198,0],[10,10,4,1,17],[65,65,5,1,23],[12,100,5,1,91],[72,199,6,1,170],[12,12,11,11,1,41,11,1,1],[18,18,168,168,169,170,168,1,1],[102,14,33,33,1,59,103,1,1],[200,19,201,201,202,203,204,1,1],[8,1,3,4,2,22,22,2,24,11,17],[11,148,24,171,2,172,172,2,38,156,23],[10,1,21,25,3,36,36,3,31,98,19],[12,175,26,205,3,206,206,3,42,207,24],[14,2,20,4,53,2,0,1,1,8,9],[173,2,32,5,158,2,0,174,175,73,12],[104,3,26,5,105,3,0,20,7,10,106],[208,3,35,6,209,3,0,210,211,81,212],[32,1,2,1,1,2,1,9],[84,1,2,1,1,2,1,12],[46,20,3,1,1,3,1,11],[94,25,3,1,1,3,1,13],[1,12,8,1,11,11,10,14,9],[148,18,11,71,54,176,177,173,12],[1,107,10,1,33,13,108,104,11],[175,213,12,78,60,214,215,208,13],[1,18,18,0,22,1,23,23,23,0,18,22,1,67,68],[1,27,27,0,47,178,37,37,37,0,27,179,120,180,181],[1,22,22,0,36,7,30,30,30,0,23,29,1,109,110],[1,30,30,0,51,216,41,41,41,0,103,217,141,218,219],[32,36,22,33,42,11,17],[84,83,47,77,90,15,182],[46,51,36,48,60,74,19],[94,93,51,87,104,136,220],[23,36,1,22,11,2,17],[37,83,4,47,15,2,23],[30,51,1,36,13,3,19],[41,93,5,51,16,3,24],[4,4,1,2,29,11,4,1,4,1,17],[5,5,1,2,53,57,5,1,5,1,23],[5,5,1,3,38,13,5,1,5,1,19],[6,6,1,3,58,63,6,1,6,1,24]]

def average(arr)
    return arr.reduce(:+) / arr.size.to_f
end

puts "#{a}"
b = a.map{ |route|
    average(route)
}
puts puts "#{b}"