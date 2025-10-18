#include <core/math.h>
#define ARC_CHUNK_SIZE_2POW 4

namespace arc 
{

struct pos2i;
struct pos2d;

// integer point 2d
struct pos2i
{
    int x = 0;
    int y = 0;

    pos2i();
    pos2i(int x, int y);
    pos2i(double x, double y);
    pos2i(const pos2d& v);

    pos2i operator+(const pos2i &v) const;
    pos2i operator-(const pos2i &v) const;
    pos2i operator*(const pos2i &v) const;
    pos2i operator/(const pos2i &v) const;

    pos2i findc() const;
    operator pos2d() const;
    
    static double dist_powered(const pos2i& v1, const pos2i& v2);
    static double dist(const pos2i& v1, const pos2i& v2);
};

// double point 2d
struct pos2d
{
    double x = 0;
    double y = 0;

    pos2d();
    pos2d(double x, double y);
    pos2d(const pos2i& v);

    pos2d operator+(const pos2d &v) const;
    pos2d operator-(const pos2d &v) const;
    pos2d operator*(const pos2d &v) const;
    pos2d operator/(const pos2d &v) const;

    pos2i findc() const;
    operator pos2i() const;
    
    static double dist_powered(const pos2d& v1, const pos2d& v2);
    static double dist(const pos2d& v1, const pos2d& v2);
};

int findc(double v);

}