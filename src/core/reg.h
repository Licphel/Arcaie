#pragma once
#include <core/def.h>
#include <stack>
#include <map>
#include <functional>
#include <core/id.h>

namespace arc
{

template <typename T> struct registry;

template <typename T> struct ref
{
    registry<T> &reg;
    int idx;

    operator T()
    {
        return reg[idx];
    }
};

template <typename T> struct registry
{
    std::map<unique_id, T> map;
    std::stack<std::function<void()>> P_delayed;
    int P_idx_next = 0;

    ref<T> make(const unique_id &id, const T &obj)
    {
        map[id] = obj;
        int idx = P_idx_next++;
        // here, the template value should have members reg_index & reg_id.
        P_delayed.push([this, idx, id]() {
            map[id].reg_index = idx;
            map[id].reg_id = id;
        });
        return {*this, idx};
    }

    void work()
    {
        while (!P_delayed.empty())
        {
            P_delayed.top()();
            P_delayed.pop();
        }
    }

    T operator[](int idx)
    {
        return map[idx];
    }

    T operator[](const unique_id &id)
    {
        return map[id];
    }
};

} // namespace arc
