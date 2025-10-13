local tex = arc.asset.texture("arcaie:gfx/misc/test.png")
local quad = arc.math.quad
local vec2 = arc.math.vec2
local nt = arc.net;

function draw(brush)
    brush:draw_rect(quad(20, 50, 100, 100))
    brush:draw_line(vec2(0, 0), vec2(200, 200))

    for i = 0, 10000 do brush:cl_norm() end
end

function tick(level)
    if (made_sys == nil or not made_sys) then
        level:add_system(arc.ecs.phase.COMMON, sys_proc_pos)
        made_sys = true
    end
    local ref = level:make_entity()
    level:add_component("pos", ref, { x = 1.2, y = 3.4 })
    local cmp = level:get_component("pos", ref)
    cmp.x = 3.5
    level:destroy_entity(ref)

    --nt.socket.remote():send_to_server("dummy", { str = "hello lua!" })
end

function sys_proc_pos(level)
    level:each_components("pos", function(level, ref, cmp)
        print("COMPONENT IN SYSTEM: ", ref, cmp.x, cmp.y)
    end)
end
