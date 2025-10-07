#pragma once
#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/font.h>
#include <audio/device.h>
#include <algorithm>
#include <core/time.h>

namespace arcaie::gfx
{

struct gui;

struct gui_component
{
    quad region;
    std::function<void(quad &region, const quad &view)> locator;
    vec2 cursor_pos;
    int P_index_in_gui;
    gui *parent;

    virtual ~gui_component() = default;
    virtual void render(brush *brush) = 0;
    virtual void tick() = 0;
};

struct gui : std::enable_shared_from_this<gui>
{
    static std::vector<shared<gui>> active_guis;
    static void render_currents(brush *brush);
    static void tick_currents();

    std::vector<shared<gui_component>> P_components;
    std::function<void(gui &g)> on_closed;
    std::function<void(gui &g)> on_displayed;
    shared<gui_component> focus;

    void join(shared<gui_component> comp);
    void remove(shared<gui_component> comp);
    void clear();
    void display();
    void close();
    bool is_top();

    virtual void render(brush *brush);
    virtual void tick();
};

template <typename T, class... Args> shared<T> make_gui_component(Args &&...args)
{
    static_assert(std::is_base_of<gui_component, T>::value, "T must be a subclass of gui_component.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, class... Args> shared<T> make_gui(Args &&...args)
{
    static_assert(std::is_base_of<gui, T>::value, "T must be a subclass of gui.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#define P_SIDESCROLLER_SPEED 250
#define P_SIDESCROLLER_INF 2
#define P_CURSOR_SHINE_INTERVAL 0.6

struct P_xside_scroller
{
    gui_component *c;
    double P_acc;
    double bx, by, bw, bh;
    bool is_dragging;
    double P_lcy, P_sp0;
    double P_inflation = P_SIDESCROLLER_INF;
    double P_prevp;
    double P_speed = P_SIDESCROLLER_SPEED;
    double pos = -P_SIDESCROLLER_INF;
    double hsize;
    std::function<void(brush *brush, P_xside_scroller *cmp)> on_render;

    void set_speed(double spd);
    void set_inflation(double otl);
    void to_top();
    void to_ground();
    void P_clamp();
    void tick();
    void render(brush *brush);
};

enum class button_state
{
    IDLE,
    PRESSED,
    HOVERING
};

struct gui_button : gui_component
{
    std::function<void(brush *brush, gui_button *cmp)> on_render;
    std::function<void()> on_click;
    std::function<void()> on_right_click;
    bool enable_switching = false;
    button_state state = button_state::IDLE;

    void render(brush *brush) override;
    void tick() override;
};

struct gui_text_view : gui_component
{
    bool enable_input = true;
    std::u32string content;
    P_xside_scroller P_scroller;
    watch cursor_shiner = watch(P_CURSOR_SHINE_INTERVAL, 0.5);
    // the background renderer.
    // the text renderer cannot be customized.
    std::function<void(brush *brush, gui_text_view *cmp)> on_render;
    bool all_selecting;
    int pointer;
    shared<font> font;

    void render(brush *brush) override;
    void tick() override;
};

} // namespace arcaie::gfx