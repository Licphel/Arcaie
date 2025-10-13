#pragma once
#include <core/def.h>
#include <core/log.h>
#include <any>
#include <vector>
#include <unordered_map>
#include <core/io.h>
#include <core/buffer.h>

namespace arc
{

struct binary_map;
struct binary_array;

enum class P_bincvt : byte
{
    BYTE,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    STRING_C,
    BOOL,
    MAP,
    ARRAY,
    BUF,
    MAP_ENDV
};

struct binary_value
{
    P_bincvt type;
    std::any P_anyv;

    template <typename T> static binary_value make(const T &v)
    {
        using decay_t = std::decay_t<T>;

        if constexpr (std::is_same_v<byte, decay_t>)
            return {P_bincvt::BYTE, std::any(v)};
        else if constexpr (std::is_same_v<short, decay_t>)
            return {P_bincvt::SHORT, std::any(v)};
        else if constexpr (std::is_same_v<int, decay_t>)
            return {P_bincvt::INT, std::any(v)};
        else if constexpr (std::is_same_v<long, decay_t>)
            return {P_bincvt::LONG, std::any(v)};
        else if constexpr (std::is_same_v<float, decay_t>)
            return {P_bincvt::FLOAT, std::any(v)};
        else if constexpr (std::is_same_v<double, decay_t>)
            return {P_bincvt::DOUBLE, std::any(v)};
        // it's tricky to check string types, so we just check if it's constructible.
        // I've tried const char* & char[], but they don't cover all cases.
        else if constexpr (std::is_constructible_v<std::string, decay_t>)
            return {P_bincvt::STRING_C, std::any(std::string(v))};
        else if constexpr (std::is_same_v<bool, decay_t>)
            return {P_bincvt::BOOL, std::any(v)};
        else if constexpr (std::is_same_v<binary_map, decay_t>)
            return {P_bincvt::MAP, std::any(v)};
        else if constexpr (std::is_same_v<binary_array, decay_t>)
            return {P_bincvt::ARRAY, std::any(v)};
        else if constexpr (std::is_same_v<byte_buf, decay_t>)
            return {P_bincvt::BUF, std::any(v)};

        print_throw(ARC_FATAL, "unsupported type.");
    }

    template <typename T> T cast() const
    {
        switch (type)
        {
        case P_bincvt::BYTE:
            if constexpr (std::is_convertible_v<byte, T>)
                return static_cast<T>(std::any_cast<byte>(P_anyv));
            else
                break;
        case P_bincvt::SHORT:
            if constexpr (std::is_convertible_v<short, T>)
                return static_cast<T>(std::any_cast<short>(P_anyv));
            else
                break;
        case P_bincvt::INT:
            if constexpr (std::is_convertible_v<int, T>)
                return static_cast<T>(std::any_cast<int>(P_anyv));
            else
                break;
        case P_bincvt::LONG:
            if constexpr (std::is_convertible_v<long, T>)
                return static_cast<T>(std::any_cast<long>(P_anyv));
            else
                break;
        case P_bincvt::FLOAT:
            if constexpr (std::is_convertible_v<float, T>)
                return static_cast<T>(std::any_cast<float>(P_anyv));
            else
                break;
        case P_bincvt::DOUBLE:
            if constexpr (std::is_convertible_v<double, T>)
                return static_cast<T>(std::any_cast<double>(P_anyv));
            else
                break;
        case P_bincvt::STRING_C:
            if constexpr (std::is_convertible_v<std::string, T>)
                return std::any_cast<std::string>(P_anyv);
            else
                break;
        case P_bincvt::BOOL:
            if constexpr (std::is_convertible_v<bool, T>)
                return static_cast<T>(std::any_cast<bool>(P_anyv));
            else
                break;
        case P_bincvt::MAP:
            if constexpr (std::is_convertible_v<binary_map, T>)
                return static_cast<T>(std::any_cast<binary_map>(P_anyv));
            else
                break;
        case P_bincvt::ARRAY:
            if constexpr (std::is_convertible_v<binary_array, T>)
                return static_cast<T>(std::any_cast<binary_array>(P_anyv));
            else
                break;
        case P_bincvt::BUF:
            if constexpr (std::is_convertible_v<byte_buf, T>)
                return static_cast<T>(std::any_cast<byte_buf>(P_anyv));
            else
                break;
        default:
            print_throw(ARC_FATAL, "not convertible.");
        }
        return T{};
    }
};

struct binary_map
{
    struct P_proxy
    {
        binary_map &src;
        std::string key;

        template <typename T> P_proxy &operator=(T &&value)
        {
            src.set(key, std::forward<T>(value));
            return *this;
        }

        template <typename T> operator T() const
        {
            return src.get<T>(key);
        }
    };

    std::unordered_map<std::string, binary_value> data;

    size_t size() const
    {
        return data.size();
    }

    template <typename T> T get(const std::string &key, const T &def = T()) const
    {
        auto it = data.find(key);
        if (it == data.end())
            return def;
        return it->second.cast<T>();
    }

    bool has(const std::string &key)
    {
        return data.find(key) != data.end();
    }

    template <typename T> void set(const std::string &key, const T &val)
    {
        data[key] = binary_value::make(val);
    }

    P_proxy operator[](const std::string &key)
    {
        return {*this, key};
    }
};

struct binary_array
{
    struct P_proxy
    {
        binary_array &src;
        int i;

        template <typename T> P_proxy &operator=(T &&value)
        {
            src.set(i, std::forward<T>(value));
            return *this;
        }

        template <typename T> operator T() const
        {
            return src.get<T>(i);
        }
    };

    std::vector<binary_value> data;

    size_t size() const
    {
        return data.size();
    }

    template <typename T> T get(int i, const T &def = T()) const
    {
        if (i < 0 || i >= data.size())
            return def;
        return data[i].cast<T>();
    }

    template <typename T> void set(int i, const T &val)
    {
        data[i] = binary_value::make(val);
    }

    template <typename T> void push(const T &val)
    {
        data.push_back(binary_value::make(val));
    }

    P_proxy operator[](int i)
    {
        return {*this, i};
    }
};

} // namespace arc