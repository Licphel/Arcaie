#include <gfx/brush.h>
#include <gfx/buffer.h>

namespace arc::gfx
{

void complex_buffer::end_quad()
{
    new_index(6);
    new_vertex(4);

    std::size_t s = sizeof(unsigned int) * 6;
    std::size_t old = index_buf.size();
    if (old + s > index_buf.capacity())
    {
        index_buf.reserve(index_buf.capacity() * 2);
        P_icap_changed = true;
    }

    unsigned int k = vertex_count - 4;
    // we don't use #idx to cut down 5 another cap checks.
    index_buf.push_back(0 + k);
    index_buf.push_back(1 + k);
    index_buf.push_back(3 + k);
    index_buf.push_back(1 + k);
    index_buf.push_back(2 + k);
    index_buf.push_back(3 + k);
}

void complex_buffer::new_vertex(int count)
{
    vertex_count += count;
}

void complex_buffer::new_index(int count)
{
    index_count += count;
}

void complex_buffer::clear()
{
    vertex_buf.clear();
    index_buf.clear();
    vertex_count = 0;
    index_count = 0;
    dirty = true;
}

std::shared_ptr<brush> complex_buffer::derive_brush()
{
    std::shared_ptr<brush> brush_ptr = std::make_shared<brush>();
    brush_ptr->wbuf = this;
    return brush_ptr;
}

std::shared_ptr<complex_buffer> complex_buffer::make()
{
    return std::make_shared<complex_buffer>();
}

} // namespace arc::gfx