#include <core/io.h>
#include <net/packet.h>
#include <net/socket.h>
#include <core/buffer.h>

namespace arc::net
{

std::vector<byte> packet::pack(std::shared_ptr<packet> p)
{
    auto n = typeid(*p).hash_code();
    auto it = P_get_packet_map_h2i().find(n);

    if (it == P_get_packet_map_h2i().end())
        print_throw(ARC_FATAL, "unregistered packet.");

    int pid = it->second;
    byte_buf uncmped_buf;
    uncmped_buf.write<int>(pid);
    p->write(uncmped_buf);

    auto cmped_buf = io_compress(uncmped_buf.to_vector(), io_compression_level::OPTIMAL);
    uncmped_buf.set_write_pos(0);
    uncmped_buf.write<int>(cmped_buf.size());
    uncmped_buf.write_bytes(cmped_buf.data(), cmped_buf.size());
    int size = static_cast<int>(uncmped_buf.size());

    if (size > 32767)
        print_throw(ARC_FATAL, "too large packet with {} bytes!", size);

    return uncmped_buf.to_vector();
}

std::shared_ptr<packet> packet::unpack(byte_buf &buffer, int len)
{
    auto dcmped_buf = io_decompress(buffer.read_advance(len));
    byte_buf buf = byte_buf(std::move(dcmped_buf));

    buf.set_write_pos(dcmped_buf.size());
    int pid = buf.read<int>();

    auto it = P_get_packet_map_i2f().find(pid);

    if (it == P_get_packet_map_i2f().end())
        print_throw(ARC_FATAL, "unregistered packet.");

    std::shared_ptr<packet> p = it->second();
    p->read(buf);
    return p;
}

void packet::send_to_server()
{
    socket::remote().send_to_server(shared_from_this());
}

void packet::send_to_remote(const uuid &rid)
{
    socket::server().send_to_remote(rid, shared_from_this());
}

void packet::send_to_remotes(const std::vector<uuid> &rids)
{
    auto ptr = shared_from_this();
    auto &skt = socket::server();
    for (auto &r : rids)
        skt.send_to_remote(r, ptr);
}

void packet::send_to_remotes()
{
    socket::server().send_to_remotes(shared_from_this());
}

static int P_pid_counter_v;
static std::unordered_map<int, std::function<std::shared_ptr<packet>()>> P_pmap_v;
static std::unordered_map<size_t, int> P_pmap_rev_v;

int P_pid_counter()
{
    return P_pid_counter_v++;
}

std::unordered_map<int, std::function<std::shared_ptr<packet>()>> &P_get_packet_map_i2f()
{
    return P_pmap_v;
}

std::unordered_map<size_t, int> &P_get_packet_map_h2i()
{
    return P_pmap_rev_v;
}

} // namespace arc::net
