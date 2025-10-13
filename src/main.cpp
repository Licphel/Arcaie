#include <audio/device.h>
#include <core/bin.h>
#include <core/bio.h>
#include <core/buffer.h>
#include <core/input.h>
#include <core/load.h>
#include <core/log.h>
#include <core/rand.h>
#include <core/uuid.h>
#include <gfx/atlas.h>
#include <gfx/brush.h>
#include <gfx/device.h>
#include <gfx/font.h>
#include <gfx/gui.h>
#include <gfx/image.h>
#include <gfx/mesh.h>
#include <lua/lua.h>
#include <net/packet.h>
#include <net/socket.h>
#include <world/level.h>
#include <core/io.h>
#include <core/buffer.h>
#include <core/id.h>

using namespace arc;
using namespace arc::gfx;
using namespace arc::net;
using namespace arc::audio;
using namespace arc::lua;
using namespace arc::world;

int i;
socket &sockc = socket::remote();
socket &socks = socket::server();
std::shared_ptr<gui> g;
std::shared_ptr<gui_button> b;
std::shared_ptr<gui_text_view> tv;
std::shared_ptr<font> fnt;
nine_patches pct;
level *elvl;

struct posic
{
    double x, y;

    void write(byte_buf &buf)
    {
        buf.write<double>(x);
        buf.write<double>(y);
    }

    void read(byte_buf &buf)
    {
        x = buf.read<double>();
        y = buf.read<double>();
    }
};

int main()
{
    // binary_map map_0 = bio_read_langd(io_open_local("dat/main.qk"));
    binary_map map;
    map["a"] = 1.02;
    map["c"] = binary_array();
    map["d"] = "Hello binary";

    bio_write(map, io_open_local("binary.bin"));
    map = bio_read(io_open_local("binary.bin"));

    tk_make_handle();
    tk_title("Arcaie v1.0.0");
    tk_size(vec2(800, 450));
    tk_end_make_handle();

    elvl = new level();

    fnt = font::load(io_open_local("gfx/font/main.ttf"), 32, 12);
    auto tex = texture::make(image::load(io_open_local("gfx/misc/test.png")));
    pct = nine_patches(tex);
    P_get_resource_map()["arcaie:gfx/misc/test.png"] = tex;

    lua_make_state();
    lua_bind_modules();
    lua_eval(io_read_str(io_open_local("main.lua")));
    packet::mark_id<packet_2s_heartbeat>();

    g = make_gui<gui>();

    b = make_gui_component<gui_button>();
    b->locator = [](quad &region, const quad &view) {
        region = quad::center(view.center_x(), view.center_y(), 200, 40);
    };
    b->on_render = [](std::shared_ptr<brush> brush, gui_button *cmp) {
        if (cmp->state == button_state::IDLE)
            brush->cl_set(color(1, 1, 1, 1));
        else if (cmp->state == button_state::HOVERING)
            brush->cl_set(color(0.8, 0.8, 1, 1));
        else if (cmp->state == button_state::PRESSED)
            brush->cl_set(color(0.6, 0.6, 1, 1));
        pct.make_vtx(brush, b->region);
        brush->cl_norm();
    };
    b->on_click = []() { print(ARC_INFO, "I'm clicked!"); };
    g->join(b);

    tv = make_gui_component<gui_text_view>();
    tv->font = fnt;
    tv->locator = [](quad &region, const quad &view) {
        region = quad::center(view.center_x() - 250, view.center_y(), 300, 120);
    };
    tv->on_render = [](std::shared_ptr<brush> brush, gui_text_view *cmp) { brush->draw_rect_outline(cmp->region); };
    g->join(tv);

    g->display();

    uint16_t port = P_gen_tcp_port();
    socks.start(port);
    sockc.connect(connection_type::lan_server);

    tk_hook_event_tick([]() {
        elvl->tick_systems();
        lua_protected_call(lua_get<lua_function>("tick"), elvl);
    });

    tk_hook_event_render([](std::shared_ptr<brush> brush) {
        brush->clear({0, 0, 0, 1});
        brush->use_camera(camera::normal());
        brush->use_blend(blend_mode::NORMAL);
        gui::tick_currents();
        gui::render_currents(brush);

        brush->use_camera(camera::gui());
        fnt->make_vtx(brush, "DEBUG FPS: " + std::to_string(tk_real_fps()), 15, 15);
        lua_protected_call(lua_get<lua_function>("draw"), brush);
    });

    tk_make_device();
    tk_set_device_option(device_option::ROLLOFF, 2.0);
    tk_set_device_option(device_option::REFERENCE_DIST, 8.0);
    tk_set_device_option(device_option::MAX_DIST, 42.0);
    tk_set_device_option(device_option::LISTENER, vec3(0, 0, 0));
    tk_end_make_device();

    tk_lifecycle(60, 20, false);

    delete elvl;
}
