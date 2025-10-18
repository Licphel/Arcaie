#pragma once
#include <core/def.h>
#include <cstring>
#include <unordered_map>

namespace arc::world
{

template <int Data> struct P_chunk_storage
{
    uint8_t *bytes = new uint8_t[16 * 16 * Data];

    uint8_t *find(int x, int y)
    {
        return bytes + y * 16 + x;
    }

    template <int Data_> P_chunk_storage<Data> make()
    {
        P_chunk_storage<Data_> sto;
        memset(sto.bytes, 0, sizeof(sto.bytes));
        return sto;
    }
};

struct chunk
{
    P_chunk_storage<4> P_walls;
    P_chunk_storage<4 + 2> P_blocks;
    P_chunk_storage<4> P_biomes;
    std::unordered_map<class Key, class Tp>
};

} // namespace arc::world
