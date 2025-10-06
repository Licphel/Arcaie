#pragma once
#include <gfx/image.h>
#include <iostream>
#include <core/hio.h>
#include <core/math.h>
#include <functional>
#include <core/key.h>

namespace arcaie::gfx
{

struct brush;

int tk_real_fps();
int tk_real_tps();

void tk_make_handle();
void tk_title(const std::string &title);
void tk_size(vec2 size);
void tk_pos(vec2 pos);
void tk_visible(bool visible);
void tk_maximize();
void tk_icon(shared<image> img);
void tk_cursor(shared<image> img, vec2 hotspot);
void tk_end_make_handle();
std::string tk_get_title();
vec2 tk_get_size();
vec2 tk_get_pos();
vec2 tk_get_device_size();

// begins the main loop (blocks the current thread).
// fps, if negative, means uncapped.
void tk_lifecycle(int fps, int tps, bool vsync);

void tk_swap_buffers();
bool tk_poll_events();

void tk_hook_event_tick(std::function<void()> callback);
void tk_hook_event_render(std::function<void(brush *brush)> callback);
void tk_hook_event_dispose(std::function<void()> callback);
void tk_hook_event_resize(std::function<void(int w, int h)> callback);
void tk_hook_mouse_state(std::function<void(int button, int action, int mods)> callback);
void tk_hook_cursor_pos(std::function<void(double x, double y)> callback);
void tk_hook_key_state(std::function<void(int button, int scancode, int action, int mods)> callback);

bool tk_key_held(int key, int mod = ARC_MOD_ANY);
bool tk_key_press(int key, int mod = ARC_MOD_ANY);
vec2 tk_get_cursor();
// consumes an abs value of scroll.
double tk_consume_scroll();
// returns ARC_SCROLL_UP, ARC_SCROLL_DOWN or ARC_SCROLL_NO
int tk_get_scroll_towards();
// consume the typed characters since last call.
std::string tk_consume_chars();
std::string tk_consume_clipboard_text();
void tk_set_clipboard_text(const std::string &str);

} // namespace arcaie::gfx
