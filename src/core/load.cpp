#include <audio/device.h>
#include <core/load.h>
#include <gfx/device.h>
#include <gfx/font.h>
#include <gfx/image.h>
#include <core/io.h>
#include <core/id.h>

using namespace arc::gfx;
using namespace arc::audio;

namespace arc
{

std::unordered_map<unique_id, std::any> P_resource_map;

std::unordered_map<unique_id, std::any> &P_get_resource_map()
{
    return P_resource_map;
}

void asset_loader::scan(const path_handle &path_root)
{
    for (const path_handle &path : io_recurse_files(path_root))
    {
        if (io_judge(path) == path_type::FILE)
        {
            unique_id id = unique_id(scope, path - root);
            std::string fmt = path.file_format();

            if (process_strategy_map.find(fmt) != process_strategy_map.end())
            {
                proc_strategy sttg = process_strategy_map[fmt];
                tasks.push([sttg, path, id]() { sttg(path, id); });
                P_total_tcount++;
            }
        }
    }
}

void asset_loader::add_sub(std::shared_ptr<asset_loader> subloader)
{
    subloaders.push_back(subloader);
    P_total_tcount += subloader->P_total_tcount;
}

void asset_loader::next()
{
    if (P_end_called)
        return;

    if (!P_start_called && event_on_start)
    {
        event_on_start();
        P_start_called = true;
    }

    if (!tasks.empty())
    {
        auto fn = tasks.top();
        tasks.pop();
        fn();
        P_done_tcount++;
        progress = static_cast<double>(P_done_tcount) / static_cast<double>(P_total_tcount);
    }
    else
    {
        progress = 1;

        bool can_find = false;
        for (auto &sub : subloaders)
        {
            if (sub->progress < 1)
            {
                sub->next();
                can_find = true;
                break;
            }
        }
        if (!can_find)
        {
            if (!P_end_called && event_on_end)
            {
                event_on_end();
                free();
                P_end_called = true;
            }
        }
    }
}

void asset_loader::free_node(const unique_id &id)
{
    P_resource_map.erase(id);
}

void asset_loader::free()
{
    P_resource_map.clear();
}

asset_loader::~asset_loader()
{
    free();
}

std::shared_ptr<asset_loader> asset_loader::make(const std::string &scope, const path_handle &root)
{
    std::shared_ptr<asset_loader> lptr = std::make_shared<asset_loader>();
    lptr->scope = scope;
    lptr->root = root;
    return lptr;
}

void asset_loader::add_equipment(asset_loader_equip equipment)
{
    switch (equipment)
    {
    case asset_loader_equip::PNG_AS_TEXTURE:
        process_strategy_map[".png"] = [](const path_handle &path, const unique_id &id) {
            std::shared_ptr<image> img = image::load(path);
            std::shared_ptr<texture> tex = texture::make(img);
            P_resource_map[id] = std::any(tex);
        };
        break;
    case asset_loader_equip::PNG_AS_IMAGE:
        process_strategy_map[".png"] = [](const path_handle &path, const unique_id &id) {
            std::shared_ptr<image> img = image::load(path);
            P_resource_map[id] = std::any(img);
        };
        break;
    case asset_loader_equip::TXT:
        process_strategy_map[".txt"] = [](const path_handle &path, const unique_id &id) {
            P_resource_map[id] = std::any(io_read_str(path));
        };
        break;
    case asset_loader_equip::WAVE:
        process_strategy_map[".wav"] = [](const path_handle &path, const unique_id &id) {
            std::shared_ptr<track> track = track::load(path);
            P_resource_map[id] = std::any(track);
        };
        break;
    case asset_loader_equip::FONT:
        break;
    case asset_loader_equip::SCRIPT:
    case asset_loader_equip::SHADER:
        break;
    }
}

} // namespace arc
