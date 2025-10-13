#pragma once
#include <core/def.h>
#include <string>
#include <any>
#include <unordered_map>
#include <stack>
#include <functional>
#include <core/io.h>
#include <core/uuid.h>
#include <core/id.h>

namespace arc
{

std::unordered_map<unique_id, std::any> &P_get_resource_map();

// get a loaded resource by its id.
template <typename T> T fetch(const unique_id &id)
{
    const auto &m = P_get_resource_map();
    auto it = m.find(id);
    bool cf = it != m.end() && it->second.has_value();
    return cf ? std::any_cast<T>(it->second) : std::decay_t<T>{};
}

enum class asset_loader_equip
{
    PNG_AS_TEXTURE,
    PNG_AS_IMAGE,
    TXT,
    FONT,
    SHADER,
    WAVE,
    SCRIPT
};

struct asset_loader
{
    using proc_strategy = std::function<void(const path_handle &path, const unique_id &id)>;

    std::string scope;
    path_handle root;
    double progress;
    int P_done_tcount;
    int P_total_tcount;
    std::unordered_map<std::string, proc_strategy> process_strategy_map;
    std::stack<std::function<void()>> tasks;
    std::vector<std::shared_ptr<asset_loader>> subloaders;
    std::function<void()> event_on_start;
    std::function<void()> event_on_end;
    bool P_start_called;
    bool P_end_called;

    ~asset_loader();

    void scan(const path_handle &path_root);
    void add_sub(std::shared_ptr<asset_loader> subloader);
    // run a task in the queue.
    // you may need to check the #progress to see if all tasks are done.
    void next();
    void free_node(const unique_id &id);
    void free();

    // add a built-in loader behavior to the loader.
    void add_equipment(asset_loader_equip equipment);

    static std::shared_ptr<asset_loader> make(const std::string &scope, const path_handle &root);
};

} // namespace arc
