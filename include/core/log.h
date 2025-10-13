#pragma once
#include <core/def.h>
#include <fmt/core.h>
#include <iostream>

namespace arc
{

enum log_type
{
    ARC_DEBUG = 0,
    ARC_INFO = 1,
    ARC_WARN = 2,
    ARC_FATAL = 3
};

static std::mutex P_log_mutex;

void log_redirect(const std::string &logv);
std::string P_get_header(log_type type);

template <typename... Args> void print(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(P_log_mutex);
    std::string formatted = P_get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
    log_redirect(formatted);
}

template <typename... Args> [[noreturn]] void print_throw(log_type type, const std::string &fmt, Args &&...args)
{
    std::lock_guard<std::mutex> lock(P_log_mutex);
    std::string formatted = P_get_header(type) + fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
    std::cout << formatted << std::endl;
    std::cout << std::flush;
    log_redirect(formatted);
    throw std::runtime_error(formatted);
}

} // namespace arc
