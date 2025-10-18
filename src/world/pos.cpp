#include <world/pos.h>
#include <cmath>

namespace arc
{

pos2i::pos2i(int x, int y) : x(x), y(y)
{
}

pos2i::pos2i(double x, double y) : x(static_cast<int>(x)), y(static_cast<int>(y))
{
}

pos2i::pos2i(const pos2d &v)
{
    x = static_cast<int>(std::floor(v.x));
    y = static_cast<int>(std::floor(v.y));
}

pos2i pos2i::operator+(const pos2i &v) const
{
    return {x + v.x, y + v.y};
}
pos2i pos2i::operator-(const pos2i &v) const
{
    return {x - v.x, y - v.y};
}
pos2i pos2i::operator*(const pos2i &v) const
{
    return {x * v.x, y * v.y};
}
pos2i pos2i::operator/(const pos2i &v) const
{
    return {x / v.x, y / v.y};
}

pos2i pos2i::findc() const
{
    return pos2i(arc::findc(static_cast<double>(x)), arc::findc(static_cast<double>(y)));
}

pos2i::operator pos2d() const
{
    return pos2d(static_cast<double>(x), static_cast<double>(y));
}

double pos2i::dist_powered(const pos2i &v1, const pos2i &v2)
{
    double dx = static_cast<double>(v1.x - v2.x);
    double dy = static_cast<double>(v1.y - v2.y);
    return dx * dx + dy * dy;
}

double pos2i::dist(const pos2i &v1, const pos2i &v2)
{
    return std::sqrt(dist_powered(v1, v2));
}

int findc(double v)
{
    return static_cast<int>(std::floor(v / 16.0));
}

} // namespace arc