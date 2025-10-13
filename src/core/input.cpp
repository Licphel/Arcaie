#include <core/input.h>
#include <core/time.h>
#include <gfx/device.h>

namespace arc
{

bool key_held(int key, int mod)
{
    return gfx::P_tk_key_held(key, mod);
}

bool key_press(int key, int mod)
{
    return gfx::P_tk_key_press(key, mod);
}

bool key_repeat(int key, int mod)
{
    return gfx::P_tk_key_repeat(key, mod);
}

vec2 get_cursor()
{
    return gfx::P_tk_get_cursor();
}

double consume_scroll()
{
    return gfx::P_tk_consume_scroll();
}

int get_scroll_towards()
{
    return gfx::P_tk_get_scroll_towards();
}

std::string consume_chars()
{
    return gfx::P_tk_consume_chars();
}

std::string consume_clipboard_text()
{
    return gfx::P_tk_consume_clipboard_text();
}

void set_clipboard_text(const std::string &str)
{
    gfx::P_tk_set_clipboard_text(str);
}

} // namespace arc
