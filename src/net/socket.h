#pragma once
#include <net/packet.h>
#include <string>
#include <core/uuid.h>

namespace arc::net
{

enum class connection_type
{
    // the server is integrated, in the same process with the remote.
    integrated_server,
    // the server is connected to the remote via lan.
    lan_server,
    // the server is connected to the remote via ip:port.
    address_server
};

// a packet-socket.
// a socket can be either a server or a remote socket.
struct socket : packet_context
{
    struct P_impl;
    std::unique_ptr<P_impl> P_pimpl;

    socket();
    ~socket();

    // remote starts
    void connect(connection_type type, const std::string &host = "", uint16_t port = 0);
    void send_to_server(std::shared_ptr<packet> pkt);
    void disconnect();
    // discover lan server, and connect to it.
    void discover();

    // server starts
    void start(uint16_t port = 0);
    void stop();
    void send_to_remote(const uuid &remote_id, std::shared_ptr<packet> pkt);
    void send_to_remotes(std::shared_ptr<packet> pkt);

    // process packets. this should be called in the main thread.
    void tick();
    void hold_alive(const uuid &id);

    static socket &server();
    static socket &remote();
};

// get an assigned tcp port
uint16_t P_gen_tcp_port();
// get an assigned tcp port
uint16_t P_gen_udp_port();

} // namespace arc::net
