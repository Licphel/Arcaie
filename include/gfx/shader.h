#pragma once
#include <core/def.h>
#include <functional>
#include <gfx/color.h>
#include <core/math.h>
#include <core/io.h>

namespace arc::gfx
{

enum class shader_vertex_data_type
{
    BYTE,
    INT,
    HALF_FLOAT,
    FLOAT
};

struct shader_attrib
{
    unsigned int P_attrib_id = 0;

    shader_attrib(unsigned int id);

    void layout(shader_vertex_data_type size, int components, int stride, int offset, bool normalize = false);
};

struct shader_uniform
{
    unsigned int P_uniform_id = 0;

    shader_uniform(unsigned int id);

    // bind the uniform to a texture unit, and use #bind_texture to actually bind it.
    void set_texture_unit(int unit);
    void seti(int v);
    void set(double v);
    void set(const vec2 &v);
    void set(const vec3 &v);
    void set(const color &v);
    void set(const transform &v);
};

enum class builtin_program_type
{
    TEXTURED,
    COLORED
};

struct program
{
    /* unstable */ unsigned int P_program_id = 0;
    std::function<void(std::shared_ptr<program> program)> callback_setup;
    /* unstable */ bool P_has_setup = false;
    std::vector<shader_uniform> cached_uniforms;

    ~program();
    shader_attrib get_attrib(const std::string &name);
    shader_attrib get_attrib(int index);
    shader_uniform get_uniform(const std::string &name);
    shader_uniform cache_uniform(const std::string &name);

    static std::shared_ptr<program> make(const std::string &vert, const std::string &frag,
                                         std::function<void(std::shared_ptr<program> program)> callback_setup);
    static std::shared_ptr<program> make(builtin_program_type type);
};

} // namespace arc::gfx