#pragma once
#include <string>
#include <functional>
#include <core/def.h>
#include <core/hio.h>

namespace arcaie
{

struct res_id
{
    std::string scope;
    std::string key;
    std::string concat;
    size_t __hash;

    res_id();
    // supports: I. scope:key, II. :key -> def_scope:key, III. key -> def_scope:key
    res_id(const std::string &cat);
    res_id(const std::string &sc, const std::string &k);
    res_id(const char ch_arr[]);

    hio_path find_path();
    operator std::string() const;
    bool operator==(const res_id &other) const;
    bool operator<(const res_id &other) const;
};

} // namespace arcaie

namespace std
{

template <> struct hash<arcaie::res_id>
{
    std::size_t operator()(const arcaie::res_id &id) const noexcept
    {
        return id.__hash;
    }
};

} // namespace std
