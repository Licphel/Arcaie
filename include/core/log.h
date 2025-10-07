#pragma once
#include <iostream>
#include <fmt/core.h>
#include <string>
#include <stdexcept>
#include <core/def.h>
#include <vector>
#include <mutex>

namespace arcaie
{

enum log_type
{
    ARC_DEBUG = 0,
    ARC_INFO = 1,
    ARC_WARN = 2,
    ARC_FATAL = 3
};

inline static std::mutex P_log_mutex;

void log_redirect();
std::string P_get_header(log_type type);

template <typename... Args> void arclog(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(P_log_mutex);
    std::string formatted = P_get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
}

template <typename... Args> [[noreturn]] void arcthrow(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(P_log_mutex);
    std::string formatted = P_get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
    throw std::runtime_error(formatted);
}

} // namespace arcaie
