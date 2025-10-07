#include <lua/lua.h>
#include <core/id.h>
#include <core/math.h>
#include <gfx/brush.h>

using namespace sol;
using namespace arcaie::gfx;

namespace arcaie::lua
{

static lua_state lua;

lua_state &lua_get_gstate()
{
    return lua;
}

int __c_lua_load(lua_State *L)
{
    res_id module_id = res_id(luaL_checkstring(L, 1));
    hio_path path = module_id.find_path();

    if (!hio_exists(path) || hio_judge(path) != hpath_type::FILE)
        return 0;

    std::string code = hio_read_str(path);

    if (luaL_loadbuffer(L, code.data(), code.size(), path.absolute.c_str()) != LUA_OK)
        return lua_error(L);
    return 1;
}

void lua_make_state()
{
    lua.open_libraries(lib::base, lib::bit32, lib::coroutine, lib::count, lib::debug, lib::ffi, lib::io, lib::jit,
                       lib::math, lib::os, lib::package, lib::string, lib::table, lib::utf8);
    // add package researcher
    lua_table searchers = lua["package"]["searchers"];
    // this is a lua-c-function (not packed by sol2)
    // stupid lua, I cannot insert my function to the beginning. why?
    searchers.add(__c_lua_load);
}

void lua_eval(const std::string &code)
{
    lua.script(code);
}

void lua_eval(lua_program &code)
{
    code();
}

lua_program lua_compile(const std::string &code)
{
    return lua.load(code);
}

void lua_bind_modules()
{
    auto vec2_type = lua_new_usertype<vec2>("vec2", lua_constructors<vec2(), vec2(double, double)>());
    vec2_type["x"] = &vec2::x;
    vec2_type["y"] = &vec2::y;
    vec2_type["length"] = &vec2::length;
    vec2_type["length_powered"] = &vec2::length_powered;
    vec2_type["normal"] = &vec2::normal;
    vec2_type["from"] = &vec2::from;
    vec2_type["dot"] = &vec2::dot;
    vec2_type["__add"] = lua_combine(static_cast<vec2 (vec2::*)(const vec2 &) const>(&vec2::operator+),
                                     static_cast<vec2 (vec2::*)(double) const>(&vec2::operator+));
    vec2_type["__sub"] = lua_combine(static_cast<vec2 (vec2::*)(const vec2 &) const>(&vec2::operator-),
                                     static_cast<vec2 (vec2::*)(double) const>(&vec2::operator-));
    vec2_type["__mul"] = lua_combine(static_cast<vec2 (vec2::*)(const vec2 &) const>(&vec2::operator*),
                                     static_cast<vec2 (vec2::*)(double) const>(&vec2::operator*));
    vec2_type["__div"] = lua_combine(static_cast<vec2 (vec2::*)(const vec2 &) const>(&vec2::operator/),
                                     static_cast<vec2 (vec2::*)(double) const>(&vec2::operator/));
    auto vec3_type = lua_new_usertype<vec3>("vec3", lua_constructors<vec3(), vec3(double, double, double)>());
    vec3_type["x"] = &vec3::x;
    vec3_type["y"] = &vec3::y;
    vec3_type["z"] = &vec3::z;
    vec3_type["length"] = &vec3::length;
    vec3_type["length_powered"] = &vec3::length_powered;
    vec3_type["normal"] = &vec3::normal;
    vec3_type["dot"] = &vec3::dot;
    vec3_type["__add"] = lua_combine(static_cast<vec3 (vec3::*)(const vec3 &) const>(&vec3::operator+),
                                     static_cast<vec3 (vec3::*)(double) const>(&vec3::operator+));
    vec3_type["__sub"] = lua_combine(static_cast<vec3 (vec3::*)(const vec3 &) const>(&vec3::operator-),
                                     static_cast<vec3 (vec3::*)(double) const>(&vec3::operator-));
    vec3_type["__mul"] = lua_combine(static_cast<vec3 (vec3::*)(const vec3 &) const>(&vec3::operator*),
                                     static_cast<vec3 (vec3::*)(double) const>(&vec3::operator*));
    vec3_type["__div"] = lua_combine(static_cast<vec3 (vec3::*)(const vec3 &) const>(&vec3::operator/),
                                     static_cast<vec3 (vec3::*)(double) const>(&vec3::operator/));
    auto quad_type = lua_new_usertype<quad>("quad", lua_constructors<vec3(), vec3(double, double, double, double)>());
    quad_type["x"] = &quad::x;
    quad_type["y"] = &quad::y;
    quad_type["width"] = &quad::width;
    quad_type["height"] = &quad::height;
    quad_type["corner"] = &quad::corner;
    quad_type["center"] = &quad::center;
    quad_type["intersection_of"] = &quad::intersection_of;
    quad_type["intersect"] = &quad::intersect;
    quad_type["contain"] = &quad::contain;
    quad_type["corner_x"] = &quad::corner_x;
    quad_type["corner_y"] = &quad::corner_y;
    quad_type["center_x"] = &quad::center_x;
    quad_type["center_y"] = &quad::center_y;
    quad_type["prom_x"] = &quad::prom_x;
    quad_type["prom_y"] = &quad::prom_y;
    quad_type["translate"] = &quad::translate;
    quad_type["locate_center"] = &quad::locate_center;
    quad_type["locate_corner"] = &quad::locate_corner;
    quad_type["resize"] = &quad::resize;
    quad_type["scale"] = &quad::scale;
    quad_type["inflate"] = &quad::inflate;
    quad_type["area"] = &quad::area;
    auto brush_type = lua_new_usertype<brush>("brush", lua_native);
    brush_type["draw_rect"] = &brush::draw_rect;
    brush_type["draw_line"] = &brush::draw_line;
    brush_type["draw_point"] = &brush::draw_point;
}

} // namespace arcaie::lua
