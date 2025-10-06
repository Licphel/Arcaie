#pragma once
#include <gfx/brush.h>
#include <gfx/image.h>
#include <gfx/font.h>
#include <audi/device.h>
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
    int __index_in_gui;
    gui *parent;

    virtual ~gui_component() = default;
    virtual void render(brush *brush) = 0;
    virtual void tick() = 0;
};

struct gui : std::enable_shared_from_this<gui>
{
    static std::vector<shared<gui>> CURRENTS;
    static void render_currents(brush *brush);
    static void tick_currents();

    std::vector<shared<gui_component>> __components;
    std::function<void(gui *g)> on_closed;
    std::function<void(gui *g)> on_displayed;
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

template <class T, class... Args> shared<T> make_gui_component(Args &&...args)
{
    static_assert(std::is_base_of<gui_component, T>::value, "T must be a subclass of gui_component.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T, class... Args> shared<T> make_gui(Args &&...args)
{
    static_assert(std::is_base_of<gui, T>::value, "T must be a subclass of gui.");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#define __SIDESCROLLER_SPEED 250
#define __SIDESCROLLER_INF 2
#define __CURSOR_SHINE_INTERVAL 0.6

struct __xside_scroller
{
    gui_component *c;
    double __acc;
    double bx, by, bw, bh;
    bool is_dragging;
    double __lcy, __sp0;
    double __inflation = __SIDESCROLLER_INF;
    double __prevp;
    double __speed = __SIDESCROLLER_SPEED;
    double pos = -__SIDESCROLLER_INF;
    double hsize;
    std::function<void(brush *brush, __xside_scroller *cmp)> on_render;

    void set_speed(double spd);
    void set_inflation(double otl);
    void to_top();
    void to_ground();
    void __clamp();
    void tick();
    void render(brush *brush);
};

struct gui_button : gui_component
{
    enum button_state
    {
        IDLE,
        PRESSED,
        HOVERING
    };

    std::function<void(brush *brush, gui_button *cmp)> on_render;
    std::function<void()> on_click;
    std::function<void()> on_right_click;
    bool enable_switching = false;
    button_state curstate = IDLE;

    void render(brush *brush) override;
    void tick() override;
};

struct gui_text_view : gui_component
{
    bool enable_input = true;
    std::string content;
    __xside_scroller __scroller;
    watch cursor_shiner = watch(__CURSOR_SHINE_INTERVAL, 0.5);
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