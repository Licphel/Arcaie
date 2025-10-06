#include <core/input.h>
#include <core/time.h>
#include <gfx/device.h>

namespace arcaie
{

bool key_held(int key, int mod)
{
    return gfx::tk_key_held(key, mod);
}

bool key_press(int key, int mod)
{
    return gfx::tk_key_press(key, mod);
}

vec2 get_cursor()
{
    return gfx::tk_get_cursor();
}

double consume_scroll()
{
    return gfx::tk_consume_scroll();
}

int get_scroll_towards()
{
    return gfx::tk_get_scroll_towards();
}

std::string consume_chars()
{
    return gfx::tk_consume_chars();
}

std::string consume_clipboard_text()
{
    return gfx::tk_consume_clipboard_text();
}

void set_clipboard_text(const std::string &str)
{
    gfx::tk_set_clipboard_text(str);
}

} // namespace arcaie
