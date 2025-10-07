#include <core/io.h>
#include <core/log.h>
#include <fstream>
#include <iostream>

#define BROTLI_IMPLEMENTATION
#include <brotli/encode.h>
#include <brotli/decode.h>

namespace arcaie
{

path_handle::path_handle() = default;

path_handle::path_handle(const std::string &name) : absolute(name), P_npath(fs::path(name))
{
    P_check();
}

void path_handle::P_check()
{
    for (size_t pos = 0; (pos = absolute.find('\\', pos)) != std::string::npos;)
        absolute.replace(pos, 1, "/");
}

std::string path_handle::file_name() const
{
    return P_npath.filename().string();
}

std::string path_handle::file_format() const
{
    return P_npath.extension().string();
}

path_handle path_handle::operator/(const std::string &name) const
{
    bool append = absolute[absolute.length() - 1] != '/';
    return path_handle(append ? absolute + '/' + name : absolute + name);
}

std::string path_handle::operator-(const path_handle &path) const
{
    return fs::relative(P_npath, path.P_npath).generic_string();
}

path_handle io_open(const std::string &name)
{
    return path_handle(name);
}

path_handle io_open_local(const std::string &name)
{
    return io_execution_path() / name;
}

path_handle io_parent(const path_handle &path)
{
    return {path.P_npath.parent_path().string()};
}

void io_del(const path_handle &path)
{
    fs::remove(path.P_npath);
}

void io_rename(const path_handle &path, const std::string &name)
{
    fs::rename(path.P_npath, name);
}

bool io_exists(const path_handle &path)
{
    return fs::exists(path.P_npath);
}

void io_mkdirs(const path_handle &path)
{
    fs::create_directories(io_judge(path) == path_type::DIR ? path.P_npath : io_parent(path).P_npath);
    if (io_judge(path) == path_type::FILE)
    {
        std::ofstream stream(path.P_npath);
        stream.close();
    }
}

path_type io_judge(const path_handle &path)
{
    if (fs::is_directory(path.P_npath))
        return path_type::DIR;
    if (fs::is_regular_file(path.P_npath))
        return path_type::FILE;
    return path_type::UNKNOWN;
}

std::vector<path_handle> io_sub_dirs(const path_handle &path)
{
    std::vector<path_handle> paths;
    for (auto &k : fs::directory_iterator(path.P_npath))
    {
        if (k.is_directory())
            paths.push_back(path_handle(k.path().string()));
    }
    return paths;
}

std::vector<path_handle> io_sub_files(const path_handle &path)
{
    std::vector<path_handle> paths;
    for (auto &k : fs::directory_iterator(path.P_npath))
    {
        if (k.is_regular_file())
            paths.push_back(path_handle(k.path().string()));
    }
    return paths;
}

std::vector<path_handle> io_recurse_files(const path_handle &path)
{
    std::vector<path_handle> paths;
    for (auto &k : fs::recursive_directory_iterator(path.P_npath))
    {
        if (k.is_regular_file())
            paths.push_back(path_handle(k.path().string()));
    }
    return paths;
}

path_handle io_execution_path()
{
    return path_handle(fs::current_path().string()) / LIB_NAME;
}

static std::vector<byte> brotli_compress(const std::vector<byte> &src, int quality)
{
    if (src.empty())
        return {};
    size_t max_sz = BrotliEncoderMaxCompressedSize(src.size());
    std::vector<byte> out(max_sz);
    size_t encoded_sz = max_sz;
    if (BROTLI_TRUE != BrotliEncoderCompress(quality, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, src.size(),
                                             src.data(), &encoded_sz, out.data()))
        arcthrow(ARC_FATAL, "Brotli encoder failed");
    out.resize(encoded_sz);
    return out;
}

static std::vector<byte> brotli_decompress(const std::vector<byte> &src)
{
    if (src.empty())
        return {};
    std::vector<byte> dst;
    size_t avail_in = src.size();
    const byte *nxt_in = src.data();
    BrotliDecoderState *st = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!st)
        arcthrow(ARC_FATAL, "brotli decoder create failed");

    for (;;)
    {
        byte buf[64 * 1024];
        size_t avail_out = sizeof(buf);
        byte *nxt_out = buf;
        auto rc = BrotliDecoderDecompressStream(st, &avail_in, &nxt_in, &avail_out, &nxt_out, nullptr);
        size_t produced = sizeof(buf) - avail_out;
        if (produced)
            dst.insert(dst.end(), buf, buf + produced);
        if (rc == BROTLI_DECODER_RESULT_SUCCESS)
            break;
        if (rc == BROTLI_DECODER_RESULT_ERROR)
            arcthrow(ARC_FATAL, "brotli decoder error");
    }
    BrotliDecoderDestroyInstance(st);
    return dst;
}

std::vector<byte> io_read_bytes(const path_handle &path, io_compression_level clvl)
{
    std::ifstream file(path.P_npath, std::ios::binary | std::ios::ate);
    if (!file)
        arcthrow(ARC_FATAL, "cannot find {}", path.absolute);
    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<byte> raw(len);
    file.read(reinterpret_cast<char *>(raw.data()), len);
    if (!file)
        arcthrow(ARC_FATAL, "short read in {}", path.absolute);

    if (clvl == io_compression_level::RAW_READ)
        return raw;

    return io_decompress(raw);
}

void io_write_bytes(const path_handle &path, const std::vector<byte> &data, io_compression_level clvl)
{
    if (!io_exists(path))
        io_mkdirs(path);
    auto out = io_compress(data, clvl);
    std::ofstream file(path.P_npath, std::ios::binary);
    if (!file)
        arcthrow(ARC_FATAL, "cannot open {} for write", path.absolute);
    file.write(reinterpret_cast<const char *>(out.data()), out.size());
}

std::string io_read_str(const path_handle &path)
{
    auto raw = io_read_bytes(path);
    return std::string(reinterpret_cast<const char *>(raw.data()), raw.size());
}

void io_write_str(const path_handle &path, const std::string &text)
{
    io_write_bytes(path, std::vector<byte>(reinterpret_cast<const byte *>(text.data()),
                                           reinterpret_cast<const byte *>(text.data() + text.size())));
}

std::vector<byte> io_compress(std::vector<byte> buf, io_compression_level clvl)
{
    std::vector<byte> out;
    switch (clvl)
    {
    case io_compression_level::NO:
        out = buf;
        break;
    case io_compression_level::FASTEST:
        out = brotli_compress(buf, 1);
        break;
    case io_compression_level::OPTIMAL:
        out = brotli_compress(buf, 6);
        break;
    case io_compression_level::SMALLEST:
        out = brotli_compress(buf, 11);
        break;
    default:
        arcthrow(ARC_FATAL, "unsupported compression level {}", (int)clvl);
    }
    return out;
}

std::vector<byte> io_decompress(std::vector<byte> buf)
{
    return brotli_decompress(buf);
}

} // namespace arcaie