#pragma once

namespace arcaie
{

template <typename E> struct bitmask
{
    static_assert(std::is_enum_v<E>, "E must be enum or enum class");
    using underlying = std::underlying_type_t<E>;
    underlying __bits = {0};

    constexpr bitmask() = default;
    constexpr bitmask(E e) : __bits(static_cast<underlying>(e))
    {
    }

    constexpr bitmask operator|(bitmask rhs) const
    {
        return bitmask(__bits | rhs.__bits);
    }

    constexpr bitmask operator&(bitmask rhs) const
    {
        return bitmask(__bits & rhs.__bits);
    }

    constexpr bitmask operator^(bitmask rhs) const
    {
        return bitmask(__bits ^ rhs.__bits);
    }

    constexpr bitmask operator~() const
    {
        return bitmask(~__bits);
    }

    constexpr bitmask &operator|=(bitmask rhs)
    {
        __bits |= rhs.__bits;
        return *this;
    }

    constexpr bitmask &operator&=(bitmask rhs)
    {
        __bits &= rhs.__bits;
        return *this;
    }

    constexpr bitmask &operator^=(bitmask rhs)
    {
        __bits ^= rhs.__bits;
        return *this;
    }

    constexpr bool test(E e) const
    {
        return __bits & static_cast<underlying>(e);
    }

    constexpr void set(E e)
    {
        __bits |= static_cast<underlying>(e);
    }

    constexpr void clear(E e)
    {
        __bits &= ~static_cast<underlying>(e);
    }

    constexpr void toggle(E e)
    {
        __bits ^= static_cast<underlying>(e);
    }

    constexpr underlying value() const
    {
        return __bits;
    }

    constexpr explicit operator bool() const
    {
        return __bits != 0;
    }
};

template <typename E, typename... Es> constexpr bitmask<E> mask(E e, Es... rest)
{
    return (mask<E>(e) | ... | mask<E>(rest));
}

} // namespace arcaie