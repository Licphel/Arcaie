#pragma once
#include <core/def.h>

namespace arc
{

struct uuid
{
    byte bytes[16] = {0};
    size_t P_hash;

    uuid() = default;

    bool operator==(const uuid &other) const;
    bool operator<(const uuid &other) const;
    operator std::string() const;

    // a null uuid, all bytes are 0.
    static uuid empty();
    // generate a random uuid with current time and a random number from #get_grand.
    // hopefully they won't collide.
    static uuid make();
};

} // namespace arc

namespace std
{

template <> struct hash<arc::uuid>
{
    std::size_t operator()(const arc::uuid &id) const noexcept
    {
        return id.P_hash;
    }
};

} // namespace std