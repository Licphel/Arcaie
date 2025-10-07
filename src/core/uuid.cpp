#include <chrono>
#include <core/def.h>
#include <core/uuid.h>
#include <cstring>
#include <random>

namespace arcaie
{

bool uuid::operator==(const uuid &other) const
{
    return std::memcmp(bytes, other.bytes, 16) == 0;
}

bool uuid::operator<(const uuid &other) const
{
    return std::memcmp(bytes, other.bytes, 16) < 0;
}

uuid::operator std::string() const
{
    std::string result;
    result.reserve(32);

    const char hex_chars[] = "0123456789abcdef";

    for (byte byte : bytes)
    {
        result += hex_chars[(byte >> 4) & 0x0F];
        result += hex_chars[byte & 0x0F];
    }

    return result;
}

uuid uuid_null()
{
    return uuid();
}

uuid uuid_generate()
{
    uuid u;

    // generate a timestamp to higher 8 bytes
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t ns = static_cast<uint64_t>(now.time_since_epoch().count()) & 0x0FFFFFFFFFFFFFFF;

    // generate a random number to lower 8 bytes
    thread_local static std::random_device rd;
    thread_local static std::mt19937_64 rng(rd());
    std::uniform_int_distribution<uint64_t> dist;

    uint64_t rand_lo = dist(rng);
    uint64_t rand_hi = dist(rng);

    struct alignas(16) uuid_raw
    {
        uint64_t hi;
        uint64_t lo;
    } raw = {ns, rand_lo ^ rand_hi};

    std::memcpy(&u, &raw, 16);
    u.P_hash = std::hash<uint64_t>{}(raw.hi) ^ (std::hash<uint64_t>{}(raw.lo) << 1);

    return u;
}

} // namespace arcaie
