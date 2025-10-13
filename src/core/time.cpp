#include <core/time.h>
#include <gfx/device.h>

namespace arc
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
    if (cs >= P_timestamp + interval)
    {
        P_timestamp = cs;
        return true;
    }
    return cs - P_timestamp >= (1.0 - accp) * interval;
}

static clock P_clock;

clock &clock::now()
{
    return P_clock;
}

double lerp(double old, double now)
{
    return old + P_clock.partial * (now - old);
}

} // namespace arc
