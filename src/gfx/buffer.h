#pragma once
#include <core/def.h>
#include <cstring>
#include <vector>

namespace arc::gfx
{

struct brush;

// currently it only supports quad-drawing indexing.
// maybe in the future I'll extend it.
struct complex_buffer
{
    std::vector<uint8_t> vertex_buf;
    std::vector<unsigned int> index_buf;
    int vertex_count = 0;
    int index_count = 0;
    bool dirty;
    bool P_icap_changed;
    bool P_vcap_changed;

    // write a vertex, generally, T is float.
    template <typename T> complex_buffer &vtx(T t)
    {
        size_t s = sizeof(t);
        size_t old = vertex_buf.size();
        if (old + s > vertex_buf.capacity())
        {
            vertex_buf.reserve(vertex_buf.capacity() * 2);
            P_vcap_changed = true;
        }

        vertex_buf.resize(old + s);
        std::memcpy(vertex_buf.data() + old, &t, s);
        dirty = true;

        return *this;
    }

    // write an index.
    inline complex_buffer &idx(unsigned int t)
    {
        size_t s = sizeof(t);
        size_t old = index_buf.size();
        if (old + s > index_buf.capacity())
        {
            index_buf.reserve(index_buf.capacity() * 2);
            P_icap_changed = true;
        }

        index_buf.push_back(t);
        dirty = true;

        return *this;
    }

    void new_vertex(int count);
    void new_index(int count);
    void end_quad();
    void clear();
    std::shared_ptr<brush> derive_brush();

    static std::shared_ptr<complex_buffer> make();
};

} // namespace arc::gfx