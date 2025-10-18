#include <audio/device.h>
#include <core/bin.h>
#include <core/bio.h>
#include <core/buffer.h>
#include <core/io.h>
#include <core/uuid.h>
#include <lua/lua.h>


namespace arc::lua
{

void lua_bind_base(lua_state &lua)
{
    auto _n = lua_make_table();

    // path_handle
    auto pth_type = lua_new_usertype<path_handle>(_n, "path_handle", lua_native);
    pth_type["abs_path"] = &path_handle::abs_path;
    pth_type["file_name"] = &path_handle::file_name;
    pth_type["file_format"] = &path_handle::file_format;

    // io_op
    _n["open"] = &io_open;
    _n["open_local"] = &io_open_local;
    _n["exists"] = &io_exists;
    _n["del"] = &io_del;
    _n["parent"] = &io_parent;
    _n["exists"] = &io_exists;
    _n["rename"] = &io_rename;
    _n["mkdirs"] = &io_mkdirs;
    _n["judge"] = &io_judge;
    _n["sub_dirs"] = &io_sub_dirs;
    _n["sub_files"] = &io_sub_files;
    _n["recurse_files"] = &io_recurse_files;
    _n["execution_path"] = &io_execution_path;
    _n["read_str"] = &io_read_str;
    _n["write_str"] = &io_write_str;
    _n["read_bytes"] = lua_combine(
        [](const path_handle &path) { return byte_buf(io_read_bytes(path)); },
        [](const path_handle &path, io_compression_level lvl) { return byte_buf(io_read_bytes(path, lvl)); });
    _n["write_bytes"] = [](const path_handle &path, const byte_buf &buf) {
        return io_write_bytes(path, buf.to_vector());
    };

    // compression level
    auto cmplvl_type = lua_make_table();
    cmplvl_type["NO"] = io_compression_level::NO;
    cmplvl_type["SMALLEST"] = io_compression_level::SMALLEST;
    cmplvl_type["OPTIMAL"] = io_compression_level::OPTIMAL;
    cmplvl_type["FASTEST"] = io_compression_level::FASTEST;
    cmplvl_type["RAW_READ"] = io_compression_level::RAW_READ;
    cmplvl_type["DCMP_READ"] = io_compression_level::DCMP_READ;
    _n["compression_level"] = cmplvl_type;

    // byte_buf
    auto buf_type = lua_new_usertype<byte_buf>(
        _n, "byte_buf", lua_constructors<byte_buf(), byte_buf(size_t), byte_buf(const byte_buf &)>());
    buf_type["size"] = &byte_buf::size;
    buf_type["capacity"] = &byte_buf::capacity;
    buf_type["free_bytes"] = &byte_buf::free_bytes;
    buf_type["readable_bytes"] = &byte_buf::readable_bytes;
    buf_type["is_empty"] = &byte_buf::is_empty;
    buf_type["read_pos"] = &byte_buf::read_pos;
    buf_type["write_pos"] = &byte_buf::write_pos;
    buf_type["clear"] = &byte_buf::clear;
    buf_type["reserve"] = &byte_buf::reserve;
    buf_type["resize"] = &byte_buf::resize;
    buf_type["rewind"] = &byte_buf::rewind;
    buf_type["compact"] = &byte_buf::compact;
    buf_type["skip"] = &byte_buf::skip;
    buf_type["set_read_pos"] = &byte_buf::set_read_pos;
    buf_type["set_write_pos"] = &byte_buf::set_write_pos;
    buf_type["write_bytes"] = &byte_buf::write_bytes;
    buf_type["read_bytes"] = &byte_buf::read_bytes;
    buf_type["read_advance"] = &byte_buf::read_advance;
    buf_type["to_vector"] = &byte_buf::to_vector;
    buf_type["write_byte_buf"] = &byte_buf::write_byte_buf;
    buf_type["write_string"] = &byte_buf::write_string;
    buf_type["write_uuid"] = &byte_buf::write_uuid;
    buf_type["read_byte_buf"] = &byte_buf::read_byte_buf;
    buf_type["read_string"] = &byte_buf::read_string;
    buf_type["read_uuid"] = &byte_buf::read_uuid;
    buf_type["write_byte"] = [](byte_buf &self, uint8_t value) { self.write<uint8_t>(value); };
    buf_type["write_int"] = [](byte_buf &self, int value) { self.write<int>(value); };
    buf_type["write_uint"] = [](byte_buf &self, unsigned int value) { self.write<unsigned int>(value); };
    buf_type["write_long"] = [](byte_buf &self, long value) { self.write<long>(value); };
    buf_type["write_ulong"] = [](byte_buf &self, unsigned long value) { self.write<unsigned long>(value); };
    buf_type["write_float"] = [](byte_buf &self, float value) { self.write<float>(value); };
    buf_type["write_double"] = [](byte_buf &self, double value) { self.write<double>(value); };
    buf_type["write_bool"] = [](byte_buf &self, bool value) { self.write<bool>(value); };
    buf_type["write_short"] = [](byte_buf &self, short value) { self.write<short>(value); };
    buf_type["write_ushort"] = [](byte_buf &self, unsigned short value) { self.write<unsigned short>(value); };
    buf_type["read_byte"] = [](byte_buf &self) -> uint8_t { return self.read<uint8_t>(); };
    buf_type["read_int"] = [](byte_buf &self) -> int { return self.read<int>(); };
    buf_type["read_uint"] = [](byte_buf &self) -> unsigned int { return self.read<unsigned int>(); };
    buf_type["read_long"] = [](byte_buf &self) -> long { return self.read<long>(); };
    buf_type["read_ulong"] = [](byte_buf &self) -> unsigned long { return self.read<unsigned long>(); };
    buf_type["read_float"] = [](byte_buf &self) -> float { return self.read<float>(); };
    buf_type["read_double"] = [](byte_buf &self) -> double { return self.read<double>(); };
    buf_type["read_bool"] = [](byte_buf &self) -> bool { return self.read<bool>(); };
    buf_type["read_short"] = [](byte_buf &self) -> short { return self.read<short>(); };
    buf_type["read_ushort"] = [](byte_buf &self) -> unsigned short { return self.read<unsigned short>(); };
    buf_type["peek_byte"] = [](byte_buf &self) -> uint8_t { return self.peek<uint8_t>(); };
    buf_type["peek_int"] = [](byte_buf &self) -> int { return self.peek<int>(); };
    buf_type["peek_uint"] = [](byte_buf &self) -> unsigned int { return self.peek<unsigned int>(); };
    buf_type["peek_long"] = [](byte_buf &self) -> long { return self.peek<long>(); };
    buf_type["peek_ulong"] = [](byte_buf &self) -> unsigned long { return self.peek<unsigned long>(); };
    buf_type["peek_float"] = [](byte_buf &self) -> float { return self.peek<float>(); };
    buf_type["peek_double"] = [](byte_buf &self) -> double { return self.peek<double>(); };
    buf_type["peek_bool"] = [](byte_buf &self) -> bool { return self.peek<bool>(); };
    buf_type["peek_short"] = [](byte_buf &self) -> short { return self.peek<short>(); };
    buf_type["peek_ushort"] = [](byte_buf &self) -> unsigned short { return self.peek<unsigned short>(); };

    auto uuid_type = lua_new_usertype<uuid>(_n, "uuid", lua_native);
    uuid_type["make"] = &uuid::make;
    uuid_type["empty"] = &uuid::empty;
    uuid_type["P_hash"] = &uuid::P_hash;
    uuid_type["__eq"] = &uuid::operator==;
    uuid_type["__lt"] = &uuid::operator<;

    lua["arc"]["io"] = _n;
}

} // namespace arc::lua