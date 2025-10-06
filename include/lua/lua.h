#pragma once
#include <sol/sol.hpp>
#include <core/def.h>

namespace arcaie::lua
{

using lua_table = sol::table;
using lua_state = sol::state;
using lua_vargs = sol::variadic_args;
using lua_program = sol::load_result;
using lua_function = sol::function;
template <typename T> using lua_type = sol::usertype<T>;
template <typename... Args> using lua_constructors = sol::constructors<Args...>;
inline static auto lua_native = sol::no_constructor;

lua_state &lua_get_gstate();
void lua_make_state();
void lua_bind_modules();
void lua_eval(const std::string &code);
void lua_eval(lua_program &code);
lua_program lua_compile(const std::string &code);

template <typename T> void lua_push(const std::string &key, const T &v)
{
    lua_get_gstate()[key] = v;
}

template <typename T> T lua_get(const std::string &key)
{
    return lua_get_gstate()[key];
}

template <typename... Args> decltype(auto) lua_combine(Args &&...args)
{
    return sol::overload(std::forward<Args>(args)...);
}

template <typename Class, typename... Args>
lua_type<Class> lua_new_usertype(lua_table &t, const std::string &name, Args &&...args)
{
    return t.new_usertype<Class>(name, sol::call_constructor, std::forward<Args>(args)...);
}

template <typename Class, typename... Args> lua_type<Class> lua_new_usertype(const std::string &name, Args &&...args)
{
    return lua_get_gstate().new_usertype<Class>(name, sol::call_constructor, std::forward<Args>(args)...);
}

} // namespace arcaie::lua