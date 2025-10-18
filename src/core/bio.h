#pragma once
#include <core/io.h>
#include <core/bin.h>
#include <core/buffer.h>

namespace arc
{

binary_map bio_read_buf(byte_buf &v);
byte_buf bio_write_buf(const binary_map &map);
binary_map bio_read(const path_handle &path);
void bio_write(const binary_map &map, const path_handle &path);
// read a script-form binary map (like json, but not the same).
binary_map bio_read_langd(const path_handle &path);

} // namespace arc
