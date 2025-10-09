#pragma once
#include <any>
#include <core/id.h>
#include <core/io.h>
#include <functional>
#include <map>
#include <stack>

namespace arcaie
{

std::unordered_map<unique_id, std::any> &P_get_resource_map();

// get a loaded resource by its id.
template <typename T> T make_res(const unique_id &id)
{
    const auto &m = P_get_resource_map();
    auto it = m.find(id);
    bool cf = it != m.end() && it->second.has_value();
    return cf ? std::any_cast<T>(it->second) : std::decay_t<T>{};
}

using proc_strategy = std::function<void(const path_handle &path, const unique_id &id)>;

struct asset_loader
{
    std::string scope;
    path_handle root;
    double progress;
    int P_done_tcount;
    int P_total_tcount;
    std::map<std::string, proc_strategy> process_strategy_map;
    std::stack<std::function<void()>> tasks;
    std::vector<shared<asset_loader>> subloaders;
    std::function<void()> event_on_start;
    std::function<void()> event_on_end;
    bool P_start_called;
    bool P_end_called;

    ~asset_loader();

    void scan(const path_handle &path_root);
    void add_sub(shared<asset_loader> subloader);
    // run a task in the queue.
    // you may need to check the #progress to see if all tasks are done.
    void next();
    void free_node(const unique_id &id);
    void free();
};

shared<asset_loader> make_loader(const std::string &scope, const path_handle &root);

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

// add a built-in loader behavior to the loader.
void make_loader_equipment(shared<asset_loader> loader, asset_loader_equip equipment);

} // namespace arcaie
