#include <core/id.h>

namespace arc
{

unique_id::unique_id() = default;

unique_id::unique_id(const std::string &cat)
{
    auto pos = cat.find_last_of(':');

    if (pos == std::string::npos)
    {
        key = cat;
        scope = LIB_NAME;
        concat = LIB_NAME + cat;
    }
    else
    {
        key = cat.substr(pos + 1);
        scope = cat.substr(0, pos);
        concat = cat;
    }

    P_hash = std::hash<std::string>{}(concat);
}

unique_id::unique_id(const std::string &sc, const std::string &k)
{
    scope = sc;
    key = k;
    concat = sc + ":" + key;
    P_hash = std::hash<std::string>{}(concat);
}

unique_id::unique_id(const char ch_arr[]) : unique_id(std::string(ch_arr))
{
}

path_handle unique_id::find_path()
{
    if (scope == LIB_NAME)
        return io_open_local("") / key;
    return {}; // todo: add mod system
}

unique_id::operator std::string() const
{
    return concat;
}

bool unique_id::operator==(const unique_id &other) const
{
    if (other.P_hash != P_hash)
        return false;
    return other.concat == concat;
}

bool unique_id::operator<(const unique_id &other) const
{
    return other.concat < concat;
}

} // namespace arc
