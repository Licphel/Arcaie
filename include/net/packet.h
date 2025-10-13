#pragma once
#include <core/def.h>
#include <core/uuid.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <core/buffer.h>

#define ARC_USE_BUILTIN_PACKETS

namespace arc::net
{

struct packet_context
{
    // signals the remote is still alive.
    // needed to be done every #NET_TIMEOUT seconds.
    virtual void hold_alive(const uuid &id) = 0;
};

struct packet;

int P_pid_counter();
std::unordered_map<int, std::function<std::shared_ptr<packet>()>> &P_get_packet_map_i2f();
std::unordered_map<size_t, int> &P_get_packet_map_h2i();

struct packet : std::enable_shared_from_this<packet>
{
    uuid sender;

    virtual ~packet() = default;
    virtual void read(byte_buf &buf) = 0;
    virtual void write(byte_buf &buf) const = 0;
    virtual void perform(packet_context *ctx) = 0;

    // packet procotol:
    // unzipped int: LENGTH
    // zipped int: PID
    // zipped int: DATA

    static std::vector<byte> pack(std::shared_ptr<packet> p);
    static std::shared_ptr<packet> unpack(byte_buf &buf, int len);

    void send_to_server();
    void send_to_remote(const uuid &rid);
    void send_to_remotes(const std::vector<uuid> &rids);
    void send_to_remotes();

    template <typename T> static void mark_id()
    {
        const auto &tid = typeid(T);
        int pid = P_pid_counter();
        P_get_packet_map_i2f()[pid] = []() { return packet::make<T>(); };
        P_get_packet_map_h2i()[tid.hash_code()] = pid;
    }

    template <typename T, typename... Args> static std::shared_ptr<T> make(Args &&...args)
    {
        static_assert(std::is_base_of_v<packet, T>, "make a non-packet object.");
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
};

#ifdef ARC_USE_BUILTIN_PACKETS
struct packet_2s_heartbeat : packet
{
    packet_2s_heartbeat() = default;

    void read(byte_buf &) override
    {
    }

    void write(byte_buf &) const override
    {
    }

    void perform(packet_context *ctx) override
    {
        ctx->hold_alive(sender);
    }
};
#endif

} // namespace arc::net
