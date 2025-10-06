#include <gfx/gui.h>
#include <core/log.h>
#include <core/time.h>
#include <core/input.h>

namespace arcaie::gfx
{

std::vector<shared<gui>> gui::CURRENTS;

void gui::render_currents(brush *brush)
{
    for (auto &g : CURRENTS)
        g->render(brush);
}

void gui::tick_currents()
{
    for (auto &g : CURRENTS)
        g->tick();
}

void gui::join(shared<gui_component> comp)
{
    comp->__index_in_gui = (int)__components.size();
    __components.push_back(comp);
    comp->parent = this;
}

void gui::remove(shared<gui_component> comp)
{
    int idx = comp->__index_in_gui;
    __components.erase(__components.begin() + idx);
    for (int i = idx; i < (int)__components.size(); i++)
        __components[i]->__index_in_gui = i;
    comp->__index_in_gui = -1;
    comp->parent = nullptr;
}

void gui::clear()
{
    __components.clear();
}

void gui::display()
{
    CURRENTS.push_back(shared_from_this());
    if (on_displayed)
        on_displayed(this);
}

void gui::close()
{
    CURRENTS.erase(
        std::remove_if(CURRENTS.begin(), CURRENTS.end(), [this](const shared<gui> &g) { return g.get() == this; }),
        CURRENTS.end());
    if (on_closed)
        on_closed(this);
}

bool gui::is_top()
{
    auto &v = gui::CURRENTS;
    return v.size() == 0 ? false : v[v.size() - 1].get() == this;
}

void gui::render(brush *brush)
{
    auto cam = get_gui_camera();
    // do not refer to the old camera - it will change soon.
    auto old_cam = brush->camera_binded;
    brush->use(cam);
    vec2 cursor_projected = get_cursor();
    // transform cursor to world coordinates
    cam.unproject(cursor_projected);

    for (auto &c : __components)
    {
        c->cursor_pos = cursor_projected;
        c->render(brush);
    }

    brush->use(old_cam);
}

void gui::tick()
{
    auto cam = get_gui_camera();

    bool top = is_top();
    if (!top)
        focus = nullptr;

    for (auto &c : __components)
    {
        if (c->locator)
            c->locator(c->region, cam.view);
        c->tick();

        if (top && quad::contain(c->region, c->cursor_pos) && key_press(ARC_MOUSE_BUTTON_LEFT))
            focus = c;
    }
}

// ---------------- xside_scroller ------------

void __xside_scroller::set_speed(double spd)
{
    __speed = spd;
}

void __xside_scroller::set_inflation(double otl)
{
    __inflation = otl;
    pos = -__inflation;
}

void __xside_scroller::to_top()
{
    pos = -__inflation;
    __clamp();
    __prevp = pos;
}

void __xside_scroller::to_ground()
{
    pos = hsize - c->region.height + __inflation * 2;
    __clamp();
    __prevp = pos;
}

void __xside_scroller::__clamp()
{
    // A minimum offset.
    if (pos <= -__inflation)
    {
        pos = -__inflation;
        __acc = 0;
    }

    if (pos - hsize - __inflation >= -c->region.height)
    {
        pos = hsize - c->region.height + __inflation;
        __acc = 0;
    }

    if (hsize + __inflation * 2 < c->region.height)
    {
        pos = -__inflation;
        __acc = 0;
    }
}

void __xside_scroller::tick()
{
    int scrd = get_scroll_towards();
    double scr = consume_scroll();

    __prevp = pos;

    if (quad::contain(c->region, c->cursor_pos) && scr != 0)
    {
        switch (scrd)
        {
        case ARC_SCROLL_UP:
            __acc -= __speed;
            break;
        case ARC_SCROLL_DOWN:
            __acc += __speed;
            break;
        }
    }

    pos += __acc * clock::now().delta;
    if (__acc > 0)
        __acc = std::clamp(__acc - clock::now().delta * __speed * 6, 0.0, double(INT_MAX));
    else if (__acc < 0)
        __acc = std::clamp(__acc + clock::now().delta * __speed * 6, double(INT_MIN), 0.0);
    __clamp();

    if (key_held(ARC_MOUSE_BUTTON_LEFT))
    {
        if (!is_dragging)
        {
            double mx = c->cursor_pos.x;
            double my = c->cursor_pos.y;

            if (mx >= bx - 1 && mx <= bx + bw + 1 && my >= by - 1 && my <= by + bh + 1)
            {
                is_dragging = true;
                __lcy = c->cursor_pos.y;
                __sp0 = pos;
            }
        }
    }
    else
    {
        is_dragging = false;
    }

    if (is_dragging)
    {
        pos = __sp0 - (__lcy - c->cursor_pos.y) / c->region.height * hsize;
        __clamp();
    }
}

void __xside_scroller::render(brush *brush)
{
    double per = (c->region.height - __inflation * 2) / hsize;
    if (per > 1)
        per = 1;

    double sper = abs(lerp(__prevp, pos)) / hsize;
    double h = (c->region.height - __inflation * 2) * per;
    double oh = sper * c->region.height;

    bh = h;
    bx = c->region.prom_x() - 4 - __inflation;
    by = c->region.y + oh + __inflation;
    bw = 4;

    if (per < 1)
    {
        if (on_render)
            on_render(brush, this);
        else
            brush->draw_rect(quad(bx, by, bw, bh));
    }
}

// ---------------- gui_button ----------------

void gui_button::render(brush *brush)
{
    on_render(brush, this);
}

void gui_button::tick()
{
    bool hovering = quad::contain(region, cursor_pos);
    bool ld = key_press(ARC_MOUSE_BUTTON_LEFT);
    bool rd = key_press(ARC_MOUSE_BUTTON_RIGHT);

    if (hovering)
    {
        if (ld && on_click)
            on_click();
        if (rd && on_right_click)
            on_right_click();
    }

    if (enable_switching)
    {
        if (hovering && (ld || rd))
            curstate = (curstate == PRESSED) ? (hovering ? HOVERING : IDLE) : PRESSED;
    }
    else
    {
        curstate = IDLE;
        if (hovering)
        {
            curstate = HOVERING;
            if (ld)
                curstate = PRESSED;
            if (rd)
                curstate = PRESSED;
        }
    }
}

// ---------------- gui_text_view -------------

void gui_text_view::render(brush *brush)
{
    __scroller.c = this;
    on_render(brush, this);

    double o = __scroller.__inflation;
    double x = region.x + o;
    double y = region.y + o;

    double pos = lerp(__scroller.__prevp, __scroller.pos);

    brush->scissor(region);
    font_render_bound bd =
        font->make_vtx(brush, content, region.x + o, region.y - pos, 1.0, region.width - o * 3 - __scroller.bw);
    brush->scissor_end();
    __scroller.hsize = bd.region.height;
    __scroller.render(brush);

    if (parent->focus.get() == this && !cursor_shiner)
    {
        bd = font->make_vtx(nullptr, content.substr(0, pointer), region.x + o, region.y - pos, 1.0,
                            region.width - o * 3 - __scroller.bw);
        x += bd.last_width;
        y += pos + (bd.lines == 0 ? 0 : (bd.lines - 1) * font->f_lspc);
        brush->scissor(region);
        brush->cl_set(color(1, 1, 1, 0.6));
        brush->draw_rect(quad(x + 2, y - 0.5, 1, font->f_lspc));
        brush->cl_norm();
        brush->scissor_end();
    }
}

static void __insert(std::string &str, int &pointer, const std::string ins)
{
    if (pointer == (int)str.size())
        str += ins;
    else
        str.insert(pointer, ins);
    pointer += ins.length();
}

static void __on_edit(gui_text_view *c)
{
    if (c->all_selecting)
        c->content.clear();
    c->all_selecting = false;
}

void gui_text_view::tick()
{
    __scroller.c = this;
    __scroller.tick();

    if (parent->focus.get() != this)
        return;

    std::string txt = consume_chars();
    if (txt.length() > 0)
        __insert(content, pointer, txt);

    if (key_press(ARC_KEY_V, ARC_MOD_CONTROL))
    {
        __on_edit(this);
        __insert(content, pointer, consume_clipboard_text());
    }

    if (key_press(ARC_KEY_A, ARC_MOD_CONTROL))
        all_selecting = !all_selecting;

    if (key_press(ARC_KEY_C, ARC_MOD_CONTROL) && all_selecting)
    {
        set_clipboard_text(content);
        all_selecting = false;
    }

    if (key_press(ARC_KEY_X, ARC_MOD_CONTROL) && all_selecting)
    {
        set_clipboard_text(content);
        pointer = 0;
        content.clear();
        all_selecting = false;
    }

    if (key_press(ARC_KEY_ENTER))
    {
        __on_edit(this);
        __insert(content, pointer, "\n");
    }

    if (key_press(ARC_KEY_LEFT))
        pointer = std::max(0, pointer - 1);
    if (key_press(ARC_KEY_RIGHT))
        pointer = std::min((int)content.length(), pointer + 1);

    if (key_press(ARC_KEY_BACKSPACE))
    {
        if (content.length() > 0 && pointer != 0)
        {
            content.erase(pointer - 1, 1);
            pointer = std::max(0, pointer - 1);
        }

        if (all_selecting)
        {
            pointer = 0;
            content.clear();
            all_selecting = false;
        }
    }
}

} // namespace arcaie::gfx