#pragma once

namespace arcaie
{

template <typename E> struct bitmask
{
    static_assert(std::is_enum_v<E>, "E must be enum or enum class");
    using underlying = std::underlying_type_t<E>;
    underlying P_bits = {0};

    constexpr bitmask() = default;
    constexpr bitmask(E e) : P_bits(static_cast<underlying>(e))
    {
    }
    constexpr bitmask(underlying raw) : P_bits(raw)
    {
    }

    constexpr bitmask operator|(bitmask rhs) const
    {
        return bitmask(P_bits | rhs.P_bits);
    }

    constexpr bitmask operator&(bitmask rhs) const
    {
        return bitmask(P_bits & rhs.P_bits);
    }

    constexpr bitmask operator^(bitmask rhs) const
    {
        return bitmask(P_bits ^ rhs.P_bits);
    }

    constexpr bitmask operator~() const
    {
        return bitmask(~P_bits);
    }

    constexpr bitmask &operator|=(bitmask rhs)
    {
        P_bits |= rhs.P_bits;
        return *this;
    }

    constexpr bitmask &operator&=(bitmask rhs)
    {
        P_bits &= rhs.P_bits;
        return *this;
    }

    constexpr bitmask &operator^=(bitmask rhs)
    {
        P_bits ^= rhs.P_bits;
        return *this;
    }

    constexpr bool test(E e) const
    {
        return P_bits & static_cast<underlying>(e);
    }

    constexpr void set(E e)
    {
        P_bits |= static_cast<underlying>(e);
    }

    constexpr void clear(E e)
    {
        P_bits &= ~static_cast<underlying>(e);
    }

    constexpr void toggle(E e)
    {
        P_bits ^= static_cast<underlying>(e);
    }

    constexpr underlying value() const
    {
        return P_bits;
    }

    constexpr explicit operator bool() const
    {
        return P_bits != 0;
    }
};

template <typename E, typename... Es> constexpr bitmask<E> mask(E e, Es... rest)
{
    return (bitmask<E>(e) | ... | bitmask<E>(rest));
}

} // namespace arcaie