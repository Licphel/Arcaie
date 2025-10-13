#pragma once
#include <core/def.h>
#include <core/ecs.h>
#include <core/math.h>
#include <functional>
#include <core/uuid.h>

namespace arc::world
{

struct P_ecs_motion
{
    quad bound;
    vec2 velocity;
};

struct level
{
    using P_sysfn = std::function<void(level &)>;

    std::unordered_map<std::string, std::shared_ptr<ecs_pool_terased>> P_ecs_pools;
    std::vector<P_sysfn> P_ecs_syses[ECS_PHASE_COUNT];

    entity_ref make_entity()
    {
        entity_ref ref = uuid::make();
        return ref;
    }

    void destroy_entity(const entity_ref &e)
    {
        for (auto &kv : P_ecs_pools)
            kv.second->remove(e);
    }

    template <typename T> ecs_pool<T> *get_pool(const std::string &k)
    {
        auto it = P_ecs_pools.find(k);
        if (it == P_ecs_pools.end())
        {
            P_ecs_pools[k] = std::make_shared<ecs_pool<T>>();
            it = P_ecs_pools.find(k);
        }
        return static_cast<ecs_pool<T> *>(it->second.get());
    }

    template <typename T> void add_component(const std::string &k, const entity_ref &e, const T &cmp)
    {
        get_pool<T>(k)->add_raw(e, &cmp);
    }

    // never keep a component object!
    template <typename T> T *get_component(const std::string &k, const entity_ref &e)
    {
        return get_pool<T>(k)->get(e);
    }

    template <typename T> void remove_component(const std::string &k, const entity_ref &e)
    {
        get_pool<T>(k)->remove(e);
    }

    template <typename F> void add_system(ecs_phase ph, F &&f)
    {
        P_ecs_syses[int(ph)].emplace_back(std::forward<F>(f));
    }

    void tick_phase(ecs_phase ph)
    {
        for (auto &sys : P_ecs_syses[int(ph)])
            sys(*this);
    }

    void tick_systems()
    {
        tick_phase(ecs_phase::PRE);
        tick_phase(ecs_phase::COMMON);
        tick_phase(ecs_phase::POST);
    }

    template <typename T>
    void each(const std::string &k, const std::function<void(level &lvl, const entity_ref &ref, T &cmp)> &f)
    {
        get_pool<T>(k)->each([this, f](const entity_ref &ref, T &cmp) { f(*this, ref, cmp); });
    }
};

} // namespace arc::world