#include <core/id.h>

namespace arcaie
{

res_id::res_id() = default;

res_id::res_id(const std::string &cat)
{
    auto pos = cat.find_last_of(':');

    if (pos == std::string::npos)
    {
        key = pos == 0 ? cat.substr(1) : cat;
        scope = LIB_NAME;
        concat = LIB_NAME + cat;
    }
    else
    {
        key = cat.substr(pos + 1);
        scope = cat.substr(0, pos);
        concat = cat;
    }

    __hash = std::hash<std::string>{}(concat);
}

res_id::res_id(const std::string &sc, const std::string &k)
{
    scope = sc;
    key = k;
    concat = sc + ":" + key;
    __hash = std::hash<std::string>{}(concat);
}

res_id::res_id(const char ch_arr[]) : res_id(std::string(ch_arr))
{
}

hio_path res_id::find_path()
{
    if (scope == LIB_NAME)
        return hio_open_local("") / key;
    return {}; // todo: add mod system
}

res_id::operator std::string() const
{
    return concat;
}

bool res_id::operator==(const res_id &other) const
{
    if (other.__hash != __hash)
        return false;
    return other.concat == concat;
}

bool res_id::operator<(const res_id &other) const
{
    return other.concat < concat;
}

} // namespace arcaie
