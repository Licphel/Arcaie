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

inline static std::mutex __log_mutex;

void __log_redirect();
std::string __get_header(log_type type);

template <typename... Args> void prtlog(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(__log_mutex);
    std::string formatted = __get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
}

template <typename... Args> [[noreturn]] void prtlog_throw(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(__log_mutex);
    std::string formatted = __get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
    throw std::runtime_error(formatted);
}

} // namespace arcaie
