#include <core/log.h>
#include <vector>
#include <fstream>
#include <core/io.h>

namespace arcaie
{

static std::ofstream P_logstream = std::ofstream(io_open_local("latest.log").P_npath);

void log_redirect()
{
    std::cout.rdbuf(P_logstream.rdbuf());
    std::cerr.rdbuf(P_logstream.rdbuf());
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

} // namespace arcaie