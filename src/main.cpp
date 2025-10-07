#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/device.h>
#include <core/log.h>
#include <audi/device.h>
#include <gfx/atlas.h>
#include <gfx/mesh.h>
#include <gfx/font.h>
#include <core/bin.h>
#include <core/buffer.h>
#include <core/bio.h>
#include <core/load.h>
#include <core/rand.h>
#include <core/uuid.h>
#include <net/packet.h>
#include <net/socket.h>
#include <gfx/gui.h>
#include <lua/lua.h>
#include <core/input.h>
#include <world/level.h>

using namespace arcaie;
using namespace arcaie::gfx;
using namespace arcaie::net;
using namespace arcaie::audi;
using namespace arcaie::lua;
using namespace arcaie::world;
using namespace arcaie::ecs;

int i;
socket &sockc = get_gsocket_remote();
socket &socks = get_gsocket_server();
shared<gui> g;
shared<gui_button> b;
shared<gui_text_view> tv;
shared<font> fnt;
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
    tk_make_handle();
    tk_title("Arcaie");
    tk_size(vec2(800, 450));
    tk_end_make_handle();

    elvl = new level();
    entity_ref eref1 = elvl->make_entity();
    entity_ref eref2 = elvl->make_entity();
    entity_ref eref3 = elvl->make_entity();
    elvl->add_system(ecs_phase::COMMON, [](level &lvl) {
        elvl->each<posic>("position",
                          [lvl](const entity_ref &ref, posic &cmp) { prtlog(ARC_INFO, std::to_string(cmp.x)); });
    });
    elvl->add_component("position", eref1, posic{1.5, 3.5});
    elvl->add_component("position", eref2, posic{2.5, 3.5});
    elvl->add_component("position", eref3, posic{3.5, 3.5});

    auto *pool = elvl->get_pool<posic>("position");
    auto *cmp = pool->get(eref2);

    __log_redirect();

    lua_make_state();
    lua_bind_modules();
    lua_eval(hio_read_str(hio_open_local("main.lua")));
    register_packet<packet_2s_heartbeat>();
    register_packet<packet_dummy>();

    fnt = load_font(hio_open_local("gfx/font/main.ttf"), 32, 12);
    auto tex = make_texture(load_image(hio_open_local("gfx/misc/test.png")));
    pct = nine_patches(tex);

    g = make_gui<gui>();

    b = make_gui_component<gui_button>();
    b->locator = [](quad &region, const quad &view) {
        region = quad::center(view.center_x(), view.center_y(), 200, 40);
    };
    b->on_render = [](brush *brush, gui_button *cmp) {
        if (cmp->curstate == button_state::IDLE)
            brush->cl_set(color(1, 1, 1, 1));
        else if (cmp->curstate == button_state::HOVERING)
            brush->cl_set(color(0.8, 0.8, 1, 1));
        else if (cmp->curstate == button_state::PRESSED)
            brush->cl_set(color(0.6, 0.6, 1, 1));
        pct.make_vtx(brush, b->region);
        brush->cl_norm();
    };
    b->on_click = []() { prtlog(ARC_INFO, "clicked!"); };
    g->join(b);

    tv = make_gui_component<gui_text_view>();
    tv->font = fnt;
    tv->locator = [](quad &region, const quad &view) {
        region = quad::center(view.center_x() - 250, view.center_y(), 300, 120);
    };
    tv->on_render = [](brush *brush, gui_text_view *cmp) { brush->draw_rect_outline(cmp->region); };
    g->join(tv);

    g->display();

    socks.start(8080);
    sockc.connect(connection_type::lan_server, "127.0.0.1", 8080);

    tk_hook_event_tick([]() { elvl->tick_systems(); });

    tk_hook_event_render([](brush *brush) {
        brush->clear({0, 0, 0, 1});
        brush->use(get_absolute_camera());
        brush->use(blend_mode::NORMAL);
        gui::tick_currents();
        gui::render_currents(brush);

        brush->use(get_gui_camera());
        fnt->make_vtx(brush, "DEBUG FPS: " + std::to_string(tk_real_fps()), 15, 15);
        lua_get<lua_function>("draw")(brush);
    });

    tk_make_device();
    tk_set_device_option(device_option::ROLLOFF, 2.0);
    tk_set_device_option(device_option::REFERENCE_DIST, 8.0);
    tk_set_device_option(device_option::MAX_DIST, 42.0);
    tk_set_device_option(device_option::LISTENER, vec3(0, 0, 0));
    tk_end_make_device();

    tk_lifecycle(0, 20, false);

    delete elvl;
}
