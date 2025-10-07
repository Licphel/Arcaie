#pragma once
#include <filesystem>
#include <vector>
#include <core/def.h>

namespace fs = std::filesystem;

namespace arcaie
{

struct path_handle
{
    std::string absolute;
    /* unstable */ fs::path P_npath;

    path_handle();
    path_handle(const std::string &name);

    // regulate the path form.
    void P_check();
    // returns the file name with extension.
    std::string file_name() const;
    // returns the file extension with the dot, like ".txt".
    std::string file_format() const;
    // get the subdir
    path_handle operator/(const std::string &name) const;
    // get the relative path, say 'minus' the #path from #this path.
    // for example, "C:/dir/file.txt" - "C:/dir" = "file.txt".
    std::string operator-(const path_handle &path) const;
};

enum class path_type
{
    DIR,
    FILE,
    UNKNOWN
};

// open a absolute path.
path_handle io_open(const std::string &name);
// open a path relative to the execution path of the executable file.
path_handle io_open_local(const std::string &name);
path_handle io_parent(const path_handle &path);
void io_del(const path_handle &path);
void io_rename(const path_handle &path, const std::string &name);
bool io_exists(const path_handle &path);
// if the path is a file, create its parent directories and an empty file.
// if the path is a directory, create it and its parent directories.
void io_mkdirs(const path_handle &path);
// get the type of the path.
path_type io_judge(const path_handle &path);
std::vector<path_handle> io_sub_dirs(const path_handle &path);
// get all files in the directory, but not in its sub-directories.
std::vector<path_handle> io_sub_files(const path_handle &path);
// get all files in the directory and its sub-directories, and, so on, recursively.
std::vector<path_handle> io_recurse_files(const path_handle &path);
path_handle io_execution_path();

enum class io_compression_level
{
    NO = 0,
    FASTEST = 1,
    OPTIMAL = 2,
    SMALLEST = 3,

    RAW_READ = 8,
    DCMP_READ = 9
};

// note: if you want to read a compressed file, use compression_level::DCMP_READ instead of compression_level::RAW_READ.
std::vector<byte> io_read_bytes(const path_handle &path, io_compression_level clvl = io_compression_level::RAW_READ);
void io_write_bytes(const path_handle &path, const std::vector<byte> &data,
                    io_compression_level clvl = io_compression_level::NO);
std::string io_read_str(const path_handle &path);
void io_write_str(const path_handle &path, const std::string &text);
std::vector<byte> io_compress(std::vector<byte> buf, io_compression_level clvl = io_compression_level::OPTIMAL);
std::vector<byte> io_decompress(std::vector<byte> buf);

} // namespace arcaie