#include <core/io.h>
#include <core/log.h>
#include <fstream>

namespace arc
{

static std::ofstream P_logstream = std::ofstream(io_open_local("latest.log").P_npath);

void log_redirect(const std::string &logv)
{
    P_logstream << logv << std::endl;
    P_logstream << std::flush;
}

std::string P_get_header(log_type type)
{
    std::string header;
    switch (type)
    {
    case ARC_DEBUG:
        header = "[DEBUG] ";
        break;
    case ARC_INFO:
        header = "[INFO] ";
        break;
    case ARC_WARN:
        header = "[WARN] ";
        break;
    case ARC_FATAL:
        header = "[FATAL] ";
        break;
    };
    return header;
}

} // namespace arc