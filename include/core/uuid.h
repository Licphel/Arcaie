#pragma once
#include <vector>
#include <core/def.h>

namespace arcaie
{

struct uuid
{
    byte bytes[16] = {0};
    size_t P_hash;

    uuid() = default;

    bool operator==(const uuid &other) const;
    bool operator<(const uuid &other) const;
    operator std::string() const;
};

// returns a null uuid, all bytes are 0.
uuid uuid_null();
// generate a random uuid with current time and a random number from #get_grand.
// hopefully they won't collide.
uuid uuid_generate();

} // namespace arcaie

namespace std
{

template <> struct hash<arcaie::uuid>
{
    std::size_t operator()(const arcaie::uuid &id) const noexcept
    {
        return id.P_hash;
    }
};

} // namespace std