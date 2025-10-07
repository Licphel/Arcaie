#pragma once
#include <ecs/sys.h>

using namespace arcaie::ecs;

namespace arcaie::world
{

struct level;

struct level_ecs_manager
{
    using __sysfn = std::function<void(level *)>;

    std::unordered_map<std::string, shared<ecs_pool_terased>> __ecs_pools;
    std::vector<__sysfn> __ecs_syses[ECS_PHASE_COUNT];
    level *level;

    entity_ref make_entity()
    {
        entity_ref ref = uuid_generate();
        return ref;
    }

    void destroy_entity(const entity_ref &e)
    {
        for (auto kv : __ecs_pools)
            kv.second->remove(e);
    }

    template <typename T> ecs_pool<T> *get_pool(const std::string &k)
    {
        auto it = __ecs_pools.find(k);
        if (it == __ecs_pools.end())
        {
            __ecs_pools[k] = std::make_shared<ecs_pool<T>>();
            it = __ecs_pools.find(k);
        }
        return static_cast<ecs_pool<T> *>(it->second.get());
    }

    template <typename T> void add_component(const std::string &k, const entity_ref &e, const T &cmp)
    {
        get_pool<T>(k)->add_raw(e, &cmp);
    }

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
        __ecs_syses[int(ph)].emplace_back(std::forward<F>(f));
    }

    void tick_phase(ecs_phase ph)
    {
        for (auto &sys : __ecs_syses[int(ph)])
            sys(level);
    }

    void tick_systems()
    {
        tick_phase(ecs_phase::PRE);
        tick_phase(ecs_phase::COMMON);
        tick_phase(ecs_phase::POST);
    }

    template <typename T> void each(const std::string &k, const std::function<void(const entity_ref &ref, T &cmp)> &f)
    {
        get_pool<T>(k)->each(f);
    }
};

struct level
{
    level_ecs_manager *ecs_manager;
};

} // namespace arcaie::world