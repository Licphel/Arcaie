#pragma once
#include <bitset>
#include <core/buffer.h>
#include <core/log.h>
#include <core/uuid.h>
#include <functional>
#include <lua/lua.h>
#include <memory>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace arcaie
{

using entity_ref = arcaie::uuid;

enum class ecs_component_sync_mode : byte
{
    NONE,
    PREDICT,
    AUTHORITY
};

struct ecs_pool_terased
{
    int index;

    virtual ~ecs_pool_terased() = default;

    virtual void add_raw(const entity_ref &e, const void *src) = 0;
    virtual void *get_raw(const entity_ref &e) = 0;
    virtual void remove(const entity_ref &e) = 0;
    virtual void write(const entity_ref &e, arcaie::byte_buf &) = 0;
    virtual void read(const entity_ref &e, arcaie::byte_buf &) = 0;
};

template <typename T> struct ecs_pool : ecs_pool_terased
{
    struct slot
    {
        T data{};
        bool alive = false;
        size_t idx = 0;
    };

    std::unordered_map<entity_ref, slot> sparse;
    std::vector<entity_ref> dense;
    std::vector<T> data;

    T *add(const entity_ref &e, const T &v)
    {
        auto &s = sparse[e];
        if (s.alive)
            return nullptr;
        s.alive = true;
        s.data = v;
        s.idx = dense.size();
        dense.push_back(e);
        data.push_back(v);
        return &data.back();
    }

    T *get(const entity_ref &e)
    {
        auto it = sparse.find(e);
        if (it == sparse.end())
            return nullptr;
        auto &s = it->second;
        return (s.alive) ? &data[it->second.idx] : nullptr;
    }

    void remove(const entity_ref &e) override
    {
        auto it = sparse.find(e);
        if (it == sparse.end() || !it->second.alive)
            return;
        // move the last slot to the removed one, ensuring no empty slots.
        size_t idx = it->second.idx;
        entity_ref tail = dense.back();
        dense[idx] = tail;
        data[idx] = data.back();
        sparse[tail].idx = idx;
        dense.pop_back();
        data.pop_back();
        sparse.erase(it);
    }

    void clear()
    {
        sparse.clear();
        dense.clear();
        data.clear();
    }

    void each(const std::function<void(const entity_ref &ref, T &cmp)> &f)
    {
        for (size_t i = 0; i < dense.size(); ++i)
            f(dense[i], data[i]);
    }

    void write(const entity_ref &e, arcaie::byte_buf &buf) override
    {
        if (T *p = get(e))
            p->write(buf);
    }

    void read(const entity_ref &e, arcaie::byte_buf &buf) override
    {
        if (T *p = get(e))
            p->read(buf);
    }

    void add_raw(const entity_ref &e, const void *src) override
    {
        // recover T
        add(e, *static_cast<const T *>(src));
    }

    void *get_raw(const entity_ref &e) override
    {
        return get(e);
    }
};

#define ECS_PHASE_COUNT 3
enum class ecs_phase : byte
{
    PRE,
    COMMON,
    POST
};

/*
an accepted ecs component is like:

struct position
{
    double x;
    double y;

    void write/read(const byte_buf& buf)
    {
        ...
    }
};
*/

} // namespace arcaie