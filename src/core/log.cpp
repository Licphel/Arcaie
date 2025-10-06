#include <core/log.h>
#include <vector>
#include <fstream>
#include <core/hio.h>

namespace arcaie
{

static std::ofstream __logstream = std::ofstream(hio_open_local("latest.log").__npath);

void __log_redirect()
{
    std::cout.rdbuf(__logstream.rdbuf());
    std::cerr.rdbuf(__logstream.rdbuf());
}

std::string __get_header(log_type type)
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