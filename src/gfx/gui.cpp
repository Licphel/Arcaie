#include <core/chcvt.h>
#include <core/embit.h>
#include <core/input.h>
#include <core/log.h>
#include <core/time.h>
#include <gfx/gui.h>

namespace arcaie::gfx
{

std::vector<shared<gui>> gui::active_guis;

void gui::render_currents(brush *brush)
{
    for (auto &g : active_guis)
        g->render(brush);
}

void gui::tick_currents()
{
    for (auto &g : active_guis)
        g->tick();
}

void gui::join(shared<gui_component> comp)
{
    comp->P_index_in_gui = (int)P_components.size();
    P_components.push_back(comp);
    comp->parent = this;
}

void gui::remove(shared<gui_component> comp)
{
    int idx = comp->P_index_in_gui;
    P_components.erase(P_components.begin() + idx);
    for (int i = idx; i < (int)P_components.size(); i++)
        P_components[i]->P_index_in_gui = i;
    comp->P_index_in_gui = -1;
    comp->parent = nullptr;
}

void gui::clear()
{
    P_components.clear();
}

void gui::display()
{
    active_guis.push_back(shared_from_this());
    if (on_displayed)
        on_displayed(*this);
}

void gui::close()
{
    active_guis.erase(std::remove_if(active_guis.begin(), active_guis.end(),
                                     [this](const shared<gui> &g) { return g.get() == this; }),
                      active_guis.end());
    if (on_closed)
        on_closed(*this);
}

bool gui::is_top()
{
    auto &v = gui::active_guis;
    return v.size() == 0 ? false : v[v.size() - 1].get() == this;
}

void gui::render(brush *brush)
{
    auto &cam = get_gui_camera();
    // do not refer to the old camera - it will change soon.
    auto old_cam = brush->P_camera;
    brush->use(cam);
    vec2 cursor_projected = get_cursor();
    // transform cursor to world coordinates
    cam.unproject(cursor_projected);

    for (auto &c : P_components)
    {
        c->cursor_pos = cursor_projected;
        c->render(brush);
    }

    brush->use(old_cam);
}

void gui::tick()
{
    auto &cam = get_gui_camera();

    bool top = is_top();
    if (!top)
        focus = nullptr;

    for (auto &c : P_components)
    {
        if (c->locator)
            c->locator(c->region, cam.view);
        c->tick();

        if (top && quad::contain(c->region, c->cursor_pos) && key_press(ARC_MOUSE_BUTTON_LEFT))
            focus = c;
    }
}

// ---------------- xside_scroller ------------

void P_xside_scroller::set_speed(double spd)
{
    P_speed = spd;
}

void P_xside_scroller::set_inflation(double otl)
{
    P_inflation = otl;
    pos = -P_inflation;
}

void P_xside_scroller::to_top()
{
    pos = -P_inflation;
    P_clamp();
    P_prevp = pos;
}

void P_xside_scroller::to_ground()
{
    pos = hsize - c->region.height + P_inflation * 2;
    P_clamp();
    P_prevp = pos;
}

void P_xside_scroller::P_clamp()
{
    // A minimum offset.
    if (pos <= -P_inflation)
    {
        pos = -P_inflation;
        P_acc = 0;
    }

    if (pos - hsize - P_inflation >= -c->region.height)
    {
        pos = hsize - c->region.height + P_inflation;
        P_acc = 0;
    }

    if (hsize + P_inflation * 2 < c->region.height)
    {
        pos = -P_inflation;
        P_acc = 0;
    }
}

void P_xside_scroller::tick()
{
    int scrd = get_scroll_towards();
    double scr = consume_scroll();

    P_prevp = pos;

    if (quad::contain(c->region, c->cursor_pos) && scr != 0)
    {
        switch (scrd)
        {
        case ARC_SCROLL_UP:
            P_acc -= P_speed;
            break;
        case ARC_SCROLL_DOWN:
            P_acc += P_speed;
            break;
        }
    }

    pos += P_acc * clock::now().delta;
    if (P_acc > 0)
        P_acc = std::clamp(P_acc - clock::now().delta * P_speed * 6, 0.0, double(INT_MAX));
    else if (P_acc < 0)
        P_acc = std::clamp(P_acc + clock::now().delta * P_speed * 6, double(INT_MIN), 0.0);
    P_clamp();

    if (key_held(ARC_MOUSE_BUTTON_LEFT))
    {
        if (!is_dragging)
        {
            double mx = c->cursor_pos.x;
            double my = c->cursor_pos.y;

            if (mx >= bx - 1 && mx <= bx + bw + 1 && my >= by - 1 && my <= by + bh + 1)
            {
                is_dragging = true;
                P_lcy = c->cursor_pos.y;
                P_sp0 = pos;
            }
        }
    }
    else
    {
        is_dragging = false;
    }

    if (is_dragging)
    {
        pos = P_sp0 - (P_lcy - c->cursor_pos.y) / c->region.height * hsize;
        P_clamp();
    }
}

void P_xside_scroller::render(brush *brush)
{
    double per = (c->region.height - P_inflation * 2) / hsize;
    if (per > 1)
        per = 1;

    double sper = abs(lerp(P_prevp, pos)) / hsize;
    double h = (c->region.height - P_inflation * 2) * per;
    double oh = sper * c->region.height;

    bh = h;
    bx = c->region.prom_x() - 4 - P_inflation;
    by = c->region.y + oh + P_inflation;
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
            state = (state == button_state::PRESSED) ? (hovering ? button_state::HOVERING : button_state::IDLE)
                                                     : button_state::PRESSED;
    }
    else
    {
        state = button_state::IDLE;
        if (hovering)
        {
            state = button_state::HOVERING;
            if (ld)
                state = button_state::PRESSED;
            if (rd)
                state = button_state::PRESSED;
        }
    }
}

// ---------------- gui_text_view -------------

void gui_text_view::render(brush *brush)
{
    P_scroller.c = this;
    on_render(brush, this);

    if (content.length() >= 32767)
        return;

    double o = P_scroller.P_inflation;
    double x = region.x + o;
    double y = region.y + o;

    double pos = lerp(P_scroller.P_prevp, P_scroller.pos);

    const bitmask<font_align> align = mask(font_align::UP, font_align::LEFT);

    brush->scissor(region);
    font_render_bound bd =
        font->make_vtx(brush, content, region.x + o, region.y - pos, align, region.width - o * 3 - P_scroller.bw);
    brush->scissor_end();
    P_scroller.hsize = bd.region.height;
    P_scroller.render(brush);

    if (parent->focus.get() == this && !cursor_shiner)
    {
        bd = font->make_vtx(nullptr, content.substr(0, pointer), region.x + o, region.y - pos, align,
                            region.width - o * 3 - P_scroller.bw);
        x += bd.last_width;

        if (content.length() > 0)
        {
            auto lglyph = font->get_glyph(content[content.length() - 1]);
            x += lglyph.advance - lglyph.size.x;
        }

        y += pos + (bd.lines == 0 ? 0 : (bd.lines - 1) * font->lspc);
        brush->scissor(region);
        brush->cl_set(color(1, 1, 1, 0.6));
        brush->draw_rect(quad(x, y + 1, 1, font->lspc));
        brush->cl_norm();
        brush->scissor_end();
    }
}

static void P_insert(std::u32string &str, int &pointer, const std::string ins)
{
    static std::u32string P_cvtbuf;
    P_cvt_u32(ins, &P_cvtbuf);

    if (str.length() >= 32767)
        return;
    if (pointer == (int)str.size())
        str += P_cvtbuf;
    else
        str.insert(pointer, P_cvtbuf);
    pointer += P_cvtbuf.length();
}

static void P_on_edit(gui_text_view *c)
{
    if (c->all_selecting)
    {
        c->content.clear();
        c->pointer = 0;
    }
    c->all_selecting = false;
}

void gui_text_view::tick()
{
    static std::string P_cvtbuf;

    P_scroller.c = this;
    P_scroller.tick();

    if (parent->focus.get() != this)
        return;

    std::string txt = consume_chars();
    if (txt.length() > 0)
        P_insert(content, pointer, txt);

    if (key_press(ARC_KEY_V, ARC_MOD_CONTROL))
    {
        P_on_edit(this);
        P_insert(content, pointer, consume_clipboard_text());
    }

    if (key_press(ARC_KEY_A, ARC_MOD_CONTROL))
        all_selecting = !all_selecting;

    if (key_press(ARC_KEY_C, ARC_MOD_CONTROL) && all_selecting)
    {
        P_cvt_u8(content, &P_cvtbuf);
        set_clipboard_text(P_cvtbuf);
        all_selecting = false;
    }

    if (key_press(ARC_KEY_X, ARC_MOD_CONTROL) && all_selecting)
    {
        P_cvt_u8(content, &P_cvtbuf);
        set_clipboard_text(P_cvtbuf);
        pointer = 0;
        content.clear();
        all_selecting = false;
    }

    if (key_press(ARC_KEY_ENTER) || key_repeat(ARC_KEY_ENTER))
    {
        P_on_edit(this);
        P_insert(content, pointer, "\n");
    }

    if (key_press(ARC_KEY_LEFT) || key_repeat(ARC_KEY_LEFT))
        pointer = std::max(0, pointer - 1);
    if (key_repeat(ARC_KEY_RIGHT) || key_press(ARC_KEY_RIGHT))
        pointer = std::min((int)content.length(), pointer + 1);

    if (key_repeat(ARC_KEY_BACKSPACE) || key_press(ARC_KEY_BACKSPACE))
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