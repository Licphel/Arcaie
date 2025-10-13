#include <lua/lua.h>
#include <net/packet.h>
#include <net/socket.h>
#include <core/buffer.h>

using namespace arc::net;

namespace arc::lua
{

static std::unordered_map<std::string, lua_function> P_lua_performers;

struct lua_packet : packet
{
    lua_table data;
    std::string type;

    lua_packet()
    {
        data = lua_make_table();
    }

    lua_packet(const std::string &type, const lua_table &tb) : type(type), data(tb)
    {
    }

    void read(byte_buf &buf) override
    {
        data["str"] = buf.read_string();
    }

    void write(byte_buf &buf) const override
    {
        buf.write_string(data["str"]);
    }

    void perform(packet_context *ctx) override
    {
        auto it = P_lua_performers.find(type);
        if (it == P_lua_performers.end())
            print_throw(ARC_FATAL, "no performer for {}", type);
        it->second(ctx, data);
    }
};

void lua_bind_net(lua_state &lua)
{
    packet::mark_id<lua_packet>();

    auto _n = lua_make_table();

    // component
    auto pkt_type = lua_new_usertype<lua_packet>(_n, "P_packet",
                                                 lua_constructors<lua_packet(), lua_packet(std::string, lua_table)>());
    pkt_type["data"] = &lua_packet::data;
    pkt_type["write"] = &lua_packet::write;
    pkt_type["read"] = &lua_packet::read;
    pkt_type["perform"] = &lua_packet::perform;

    // socket
    auto sock_type = lua_new_usertype<socket>(_n, "socket", lua_native);
    sock_type["send_to_server"] = [](socket &self, const std::string &type, const lua_table &tb) {
        self.send_to_server(packet::make<lua_packet>(type, tb));
    };
    sock_type["send_to_remote"] = [](socket &self, const uuid &uuid, const std::string &type, const lua_table &tb) {
        self.send_to_remote(uuid, packet::make<lua_packet>(type, tb));
    };
    sock_type["send_to_remotes"] = lua_combine(
        [](socket &self, const lua_table &uuids, const std::string &type, const lua_table &tb) {
            auto pkt = packet::make<lua_packet>(type, tb);
            for (auto &rid : uuids)
                self.send_to_remote(rid.second.as<uuid>(), pkt);
        },
        [](socket &self, const std::string &type, const lua_table &tb) {
            self.send_to_remotes(packet::make<lua_packet>(type, tb));
        });
    sock_type["server"] = []() -> socket * { return &socket::server(); };
    sock_type["remote"] = []() -> socket * { return &socket::remote(); };

    _n["add_packet_performer"] = [](const std::string &type, const lua_function &f) { P_lua_performers[type] = f; };

    // context
    auto ctx_type = lua_new_usertype<packet_context>(_n, "packet_context", lua_native);
    ctx_type["hold_alive"] = &packet_context::hold_alive;

    lua["arc"]["net"] = _n;
}

} // namespace arc::lua