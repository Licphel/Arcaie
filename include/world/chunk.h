#pragma once
#include <core/def.h>
#include <cstring>

namespace arc::world
{

template <int Data> struct P_chunk_storage
{
    byte *bytes = new byte[16 * 16 * Data];

    byte *find(int x, int y)
    {
        return bytes + y * 16 + x;
    }

    template <int Data> P_chunk_storage<Data> make()
    {
        P_chunk_storage<Data> sto;
        memset(sto.bytes, 0, sizeof(sto.bytes));
        return sto;
    }
};

struct chunk
{
    P_chunk_storage<4> P_walls;
    P_chunk_storage<4 + 2> P_blocks;
};

} // namespace arc::world
