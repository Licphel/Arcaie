local printer = require("arcaie:printer.lua")

function draw(brush)
    brush:draw_rect(quad(20, 50, 100, 100))
    brush:draw_line(vec2(0, 0), vec2(200, 200))
end
