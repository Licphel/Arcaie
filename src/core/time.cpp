#include <core/time.h>
#include <gfx/device.h>

namespace arcaie
{

watch::watch(double interval) : interval(interval), accp(0.0)
{
}

watch::watch(double interval, double perc) : interval(interval), accp(perc)
{
}

watch::operator bool()
{
    double cs = clock::now().seconds;
    if (cs >= __timestamp + interval)
    {
        __timestamp = cs;
        return true;
    }
    return cs - __timestamp >= (1.0 - accp) * interval;
}

static clock __clock;

clock &clock::now()
{
    return __clock;
}

double lerp(double old, double now)
{
    return old + __clock.partial * (now - old);
}

} // namespace arcaie
