#include <gfx/color.h>

namespace arc::gfx
{

color color::from_bytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return color(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

void color::get_bytes(uint8_t *br, uint8_t *bg, uint8_t *bb, uint8_t *ba)
{
    *br = (uint8_t)(255 * r);
    *bg = (uint8_t)(255 * g);
    *bb = (uint8_t)(255 * b);
    *ba = (uint8_t)(255 * a);
}

color color::operator+(double s) const
{
    return color(r + s, g + s, b + s, a);
}

color color::operator-(double s) const
{
    return color(r - s, g - s, b - s, a);
}

color color::operator/(double s) const
{
    return color(r / s, g / s, b / s, a);
}

} // namespace arc::gfx