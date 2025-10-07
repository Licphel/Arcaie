#include <core/buffer.h>

namespace arcaie
{

bool P_check_is_system_little_endian()
{
    constexpr uint32_t test_value = 0x01020304;
    return reinterpret_cast<const byte *>(&test_value)[0] == 0x04;
}

byte_buf::byte_buf() = default;

byte_buf::byte_buf(size_t initial_size) : P_data(initial_size)
{
}

byte_buf::byte_buf(const std::vector<byte> &vec)
{
    P_data = vec;
    P_wpos = vec.size();
}

size_t byte_buf::size() const
{
    return P_wpos;
}

size_t byte_buf::capacity() const
{
    return P_data.capacity();
}

size_t byte_buf::free_bytes() const
{
    return P_data.capacity() - P_wpos;
}

size_t byte_buf::remaining() const
{
    return P_data.size() - P_wpos;
}

size_t byte_buf::readable_bytes() const
{
    return P_wpos - P_rpos;
}

bool byte_buf::is_empty() const
{
    return P_wpos == 0;
}

void byte_buf::clear()
{
    P_rpos = 0;
    P_wpos = 0;
}

void byte_buf::reserve(size_t size)
{
    P_data.reserve(size);
}

void byte_buf::resize(size_t size)
{
    P_data.resize(size);
    if (P_wpos > size)
        P_wpos = size;
    if (P_rpos > size)
        P_rpos = size;
}

void byte_buf::ensure_capacity(size_t needed)
{
    if (remaining() < needed)
        P_data.resize(P_data.size() + std::max(needed, P_data.size() * 2));
}

void byte_buf::ensure_readable(size_t needed) const
{
    if (readable_bytes() < needed)
        arcthrow(ARC_FATAL, "byte buffer read out of range!");
}

void byte_buf::write_bytes(const void *src, size_t len)
{
    ensure_capacity(len);
    std::memcpy(P_data.data() + P_wpos, src, len);
    P_wpos += len;
}

void byte_buf::write_byte_buf(const byte_buf &buf)
{
    write<unsigned int>((unsigned int)buf.size());
    write_bytes(buf.P_data.data(), buf.size());
}

void byte_buf::write_string(const std::string &str)
{
    write<unsigned int>(str.size());
    write_bytes(str.data(), str.size());
}

void byte_buf::write_uuid(const uuid &id)
{
    write_bytes(id.bytes, 16);
}

void byte_buf::read_bytes(void *dst, size_t len)
{
    ensure_readable(len);
    std::memcpy(dst, P_data.data() + P_rpos, len);
    P_rpos += len;
}

byte_buf byte_buf::read_byte_buf()
{
    ensure_readable(sizeof(unsigned int));
    size_t size = read<unsigned int>();
    byte_buf buf = byte_buf(size);
    read_bytes(buf.P_data.data(), size);
    return buf;
}

std::string byte_buf::read_string()
{
    ensure_readable(sizeof(unsigned int));
    size_t len = read<unsigned int>();
    ensure_readable(len);
    std::string str(reinterpret_cast<const char *>(P_data.data() + P_rpos), len);
    P_rpos += len;
    return str;
}

uuid byte_buf::read_uuid()
{
    ensure_readable(16);
    uuid u = uuid_null();
    read_bytes(u.bytes, 16);
    return u;
}

void byte_buf::skip(size_t len)
{
    ensure_readable(len);
    P_rpos += len;
}

void byte_buf::set_read_pos(size_t pos)
{
    if (pos > P_wpos)
        arcthrow(ARC_FATAL, "byte buf read pos out of range!");
    P_rpos = pos;
}

void byte_buf::set_write_pos(size_t pos)
{
    if (pos > P_data.size())
        P_data.resize(pos);
    P_wpos = pos;
    if (P_rpos > P_wpos)
        P_rpos = P_wpos;
}

size_t byte_buf::read_pos() const
{
    return P_rpos;
}

size_t byte_buf::write_pos() const
{
    return P_wpos;
}

std::vector<byte> byte_buf::to_vector() const
{
    if (P_wpos > P_data.size())
        arcthrow(ARC_FATAL, "byte buf write pos out of range!");
    return std::vector<byte>(P_data.begin(), P_data.begin() + P_wpos);
}

std::vector<byte> byte_buf::read_advance(int len)
{
    ensure_readable(len);
    auto vec = std::vector<byte>(P_data.begin() + P_rpos, P_data.begin() + P_rpos + len);
    P_rpos += len;
    return vec;
}

void byte_buf::rewind()
{
    P_rpos = 0;
}

void byte_buf::compact()
{
    if (P_rpos > 0)
    {
        size_t remaining = P_wpos - P_rpos;
        if (remaining > 0)
            std::memmove(P_data.data(), P_data.data() + P_rpos, remaining);
        P_wpos = remaining;
        P_rpos = 0;
    }
}

} // namespace arcaie
