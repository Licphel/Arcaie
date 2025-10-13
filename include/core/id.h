#pragma once
#include <core/def.h>
#include <core/io.h>
#include <string>
#include <core/io.h>

namespace arc
{

struct unique_id
{
    std::string scope;
    std::string key;
    std::string concat;
    size_t P_hash;

    unique_id();
    // supports: I. scope:key, II. :key -> def_scope:key, III. key -> def_scope:key
    unique_id(const std::string &cat);
    unique_id(const std::string &sc, const std::string &k);
    unique_id(const char ch_arr[]);

    path_handle find_path();
    operator std::string() const;
    bool operator==(const unique_id &other) const;
    bool operator<(const unique_id &other) const;
};

} // namespace arc

namespace std
{

template <> struct hash<arc::unique_id>
{
    std::size_t operator()(const arc::unique_id &id) const noexcept
    {
        return id.P_hash;
    }
};

} // namespace std
