#include <core/ecs.h>
#include <lua/lua.h>
#include <world/level.h>
#include <core/uuid.h>

using namespace arc::world;

namespace arc::lua
{

struct lua_ecs_component
{
    lua_table data;

    lua_ecs_component()
    {
        data = lua_get_gstate().create_table();
    }

    lua_ecs_component(const lua_table& tb) : data(tb)
    {
    }

    void write(byte_buf &buf)
    {
    }

    void read(byte_buf &buf)
    {
    }
};

void lua_bind_ecs(lua_state &lua)
{
    auto _n = lua_make_table();

    // component
    auto cmp_type = lua_new_usertype<lua_ecs_component>(
        _n, "P_component", lua_constructors<lua_ecs_component(), lua_ecs_component(lua_table)>());
    cmp_type["data"] = &lua_ecs_component::data;
    cmp_type["write"] = &lua_ecs_component::write;
    cmp_type["read"] = &lua_ecs_component::read;

    // level
    auto level_type = lua_new_usertype<level>(_n, "level", lua_native);
    level_type["add_component"] = [](level &self, const std::string &k, const entity_ref &e, const lua_table &cmp) {
        self.add_component<lua_ecs_component>(k, e, lua_ecs_component(cmp));
    };
    level_type["get_component"] = [](level &self, const std::string &k, const entity_ref &e) {
        auto *ptr = self.get_component<lua_ecs_component>(k, e);
        return ptr ? ptr->data : lua_make_table();
    };
    level_type["has_component"] = [](level &self, const std::string &k, const entity_ref &e) {
        return self.get_component<lua_ecs_component>(k, e) != nullptr;
    };
    level_type["remove_component"] = [](level &self, const std::string &k, const entity_ref &e) {
        self.remove_component<lua_ecs_component>(k, e);
    };
    level_type["make_entity"] = [](level &self) { return self.make_entity(); };
    level_type["destroy_entity"] = [](level &self, const entity_ref &e) { self.destroy_entity(e); };
    level_type["add_system"] = [](level &self, ecs_phase ph, const lua_function &f) {
        self.add_system(ph, [f](level &lvl) { lua_protected_call(f, lvl); });
    };
    level_type["each_components"] = [](level &self, const std::string &k, const lua_function &f) {
        self.each<lua_ecs_component>(k, [f](level &lvl, const entity_ref &ref, lua_ecs_component &cmp) {
            lua_protected_call(f, lvl, ref, cmp.data);
        });
    };

    // ecs_phase
    auto table_sys_ph = lua_make_table();
    table_sys_ph["PRE"] = ecs_phase::PRE;
    table_sys_ph["COMMON"] = ecs_phase::COMMON;
    table_sys_ph["POST"] = ecs_phase::POST;
    _n["phase"] = table_sys_ph;

    lua["arc"]["ecs"] = _n;
}

} // namespace arc::lua