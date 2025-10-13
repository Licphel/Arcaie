#include <audio/device.h>
#include <core/id.h>
#include <core/load.h>
#include <gfx/font.h>
#include <gfx/image.h>
#include <lua/lua.h>
#include <gfx/shader.h>

using namespace arc::gfx;
using namespace arc::audio;

namespace arc::lua
{

void lua_bind_asset(lua_state &lua)
{
    auto _n = lua_make_table();

    // unique_id
    auto uid_type = lua_new_usertype<unique_id>(
        _n, "unique_id",
        lua_constructors<unique_id(const std::string &), unique_id(const std::string &, const std::string &)>());
    uid_type["concat"] = &unique_id::concat;
    uid_type["scope"] = &unique_id::scope;
    uid_type["key"] = &unique_id::key;
    uid_type["P_hash"] = &unique_id::P_hash;
    uid_type["find_path"] = &unique_id::find_path;
    uid_type["__eq"] = &unique_id::operator==;
    uid_type["__lt"] = &unique_id::operator<;

    // asset_loader
    auto loader_type = lua_new_usertype<asset_loader>(_n, "asset_loader", lua_native);
    loader_type["scan"] = &asset_loader::scan;
    loader_type["next"] = &asset_loader::next;
    loader_type["add_sub"] = &asset_loader::add_sub;
    loader_type["add_strategy"] = [](asset_loader &self, const std::string &fmt, const lua_function &f) {
        self.process_strategy_map[fmt] = [f](const path_handle &path, const unique_id &id) {
            P_get_resource_map()[id] = lua_protected_call(f, path);
        };
    };
    loader_type["make"] = &asset_loader::make;

    // asset_mapping
    _n["has"] =
        lua_combine([](const unique_id &id) { return P_get_resource_map().find(id) != P_get_resource_map().end(); },
                    [](const std::string &id) { return P_get_resource_map().find(id) != P_get_resource_map().end(); });
    _n["texture"] = lua_combine([](const unique_id &id) { return fetch<std::shared_ptr<texture>>(id); },
                                [](const std::string &id) { return fetch<std::shared_ptr<texture>>(id); });
    _n["track"] = lua_combine([](const unique_id &id) { return fetch<std::shared_ptr<track>>(id); },
                              [](const std::string &id) { return fetch<std::shared_ptr<track>>(id); });
    _n["font"] = lua_combine([](const unique_id &id) { return fetch<std::shared_ptr<font>>(id); },
                             [](const std::string &id) { return fetch<std::shared_ptr<font>>(id); });
    _n["image"] = lua_combine([](const unique_id &id) { return fetch<std::shared_ptr<image>>(id); },
                              [](const std::string &id) { return fetch<std::shared_ptr<image>>(id); });
    _n["program"] = lua_combine([](const unique_id &id) { return fetch<std::shared_ptr<program>>(id); },
                                [](const std::string &id) { return fetch<std::shared_ptr<program>>(id); });
    _n["text"] = lua_combine([](const unique_id &id) { return fetch<std::string>(id); },
                             [](const std::string &id) { return fetch<std::string>(id); });

    lua["arc"]["asset"] = _n;
}

} // namespace arc::lua