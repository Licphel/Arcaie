#include <audio/device.h>
#include <core/bin.h>
#include <core/bio.h>
#include <core/io.h>
#include <lua/lua.h>

namespace arcaie::lua
{

void lua_bind_tcodec(lua_state &lua)
{
    auto _n = lua_make_table();

    lua["arc"]["codec"] = _n;
}

} // namespace arcaie::lua