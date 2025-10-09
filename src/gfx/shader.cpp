#include <core/def.h>
#include <core/log.h>
#include <functional>
#include <gfx/shader.h>
#include <memory>

// clang-format off
#include <gl/glew.h>
#include <gl/gl.h>
// clang-format on

namespace arcaie::gfx
{

shader_attrib::shader_attrib(unsigned int id) : P_attrib_id(id)
{
}

void shader_attrib::layout(shader_vertex_data_type size, int components, int stride, int offset, bool normalize)
{
    glEnableVertexAttribArray(P_attrib_id);
    GLenum type;
    switch (size)
    {
    case shader_vertex_data_type::BYTE:
        type = GL_UNSIGNED_BYTE;
        break;
    case shader_vertex_data_type::INT:
        type = GL_UNSIGNED_SHORT;
        break;
    case shader_vertex_data_type::HALF_FLOAT:
        type = GL_HALF_FLOAT;
        break;
    case shader_vertex_data_type::FLOAT:
        type = GL_FLOAT;
        break;
    default:
        type = GL_UNSIGNED_BYTE;
        break;
    }
    glVertexAttribPointer(P_attrib_id, components, type, normalize, stride, reinterpret_cast<void *>(offset));
}

shader_uniform::shader_uniform(unsigned int id) : P_uniform_id(id)
{
}

void shader_uniform::set_texture_unit(int unit)
{
    glUniform1i(P_uniform_id, unit);
}

void shader_uniform::seti(int v)
{
    glUniform1i(P_uniform_id, v);
}

void shader_uniform::set(double v)
{
    glUniform1f(P_uniform_id, v);
}

void shader_uniform::set(const vec2 &v)
{
    glUniform2f(P_uniform_id, v.x, v.y);
}

void shader_uniform::set(const vec3 &v)
{
    glUniform3f(P_uniform_id, v.x, v.y, v.z);
}

void shader_uniform::set(const color &v)
{
    glUniform4f(P_uniform_id, v.r, v.g, v.b, v.a);
}

void shader_uniform::set(const transform &v)
{
    float m[16] = {v.m00, v.m10, 0.0f, 0.0f, v.m01, v.m11, 0.0f, 0.0f,
                   0.0f,  0.0f,  1.0f, 0.0f, v.m02, v.m12, 0.0f, 1.0f};
    glUniformMatrix4fv(P_uniform_id, 1, GL_FALSE, m);
}

program::~program()
{
    glDeleteProgram(P_program_id);
}

shader_attrib program::get_attrib(const std::string &name)
{
    return shader_attrib((unsigned int)glGetAttribLocation(P_program_id, name.c_str()));
}

shader_attrib program::get_attrib(int index)
{
    return shader_attrib((unsigned int)index);
}

shader_uniform program::get_uniform(const std::string &name)
{
    return shader_uniform((unsigned int)glGetUniformLocation(P_program_id, name.c_str()));
}

shader_uniform program::cache_uniform(const std::string &name)
{
    auto uni = get_uniform(name);
    cached_uniforms.push_back(uni);
    return uni;
}

static int P_build_shader_part(std::string source, GLenum type)
{
    int id = glCreateShader(type);

    if (id == 0)
        arcthrow(ARC_FATAL, "Fail to create shader.");

    const char *src = source.c_str();
    GLint len = static_cast<GLint>(source.size());

    glShaderSource(id, 1, &src, &len);
    glCompileShader(id);

    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(id, sizeof(log), nullptr, log);
        glDeleteShader(id);
        arcthrow(ARC_FATAL, "glsl compile error: {}", std::string(log));
    }

    return id;
}

shared<program> make_program(const std::string &vert, const std::string &frag,
                             std::function<void(shared<program> program)> callback_setup)
{
    shared<program> prog = std::make_shared<program>();
    int id = prog->P_program_id = glCreateProgram();
    int vert_id = P_build_shader_part(vert, GL_VERTEX_SHADER);
    int frag_id = P_build_shader_part(frag, GL_FRAGMENT_SHADER);

    glAttachShader(id, vert_id);
    glAttachShader(id, frag_id);
    glBindFragDataLocation(id, 0, "fragColor");
    glLinkProgram(id);
    glDetachShader(id, vert_id);
    glDetachShader(id, frag_id);
    glValidateProgram(id);
    glDeleteShader(vert_id);
    glDeleteShader(frag_id);

    prog->callback_setup = callback_setup;

    return prog;
}

static const std::string P_dvert_textured = "#version 330 core\n"
                                            "layout(location = 0) in vec2 i_position;\n"
                                            "layout(location = 1) in vec4 i_color;\n"
                                            "layout(location = 2) in vec2 i_texCoord;\n"
                                            "out vec4 o_color;\n"
                                            "out vec2 o_texCoord;\n"
                                            "uniform mat4 u_proj;\n"
                                            "void main() {\n"
                                            "    o_color = i_color;\n"
                                            "    o_texCoord = i_texCoord;\n"
                                            "    gl_Position = u_proj * vec4(i_position.x, i_position.y, 0.0, 1.0);\n"
                                            "}";

static const std::string P_dfrag_textured = "#version 330 core\n"
                                            "in vec4 o_color;\n"
                                            "in vec2 o_texCoord;\n"
                                            "out vec4 fragColor;\n"
                                            "uniform sampler2D u_tex;\n"
                                            "void main() {\n"
                                            "    fragColor = o_color * texture(u_tex, o_texCoord);\n"
                                            "}";

static const std::string P_dvert_colored = "#version 330 core\n"
                                           "layout(location = 0) in vec2 i_position;\n"
                                           "layout(location = 1) in vec4 i_color;\n"
                                           "out vec4 o_color;\n"
                                           "uniform mat4 u_proj;\n"
                                           "void main() {\n"
                                           "    o_color = i_color;\n"
                                           "    gl_Position = u_proj * vec4(i_position.x, i_position.y, 0.0, 1.0);\n"
                                           "}";

static const std::string P_dfrag_colored = "#version 330 core\n"
                                           "in vec4 o_color;\n"
                                           "out vec4 fragColor;\n"
                                           "void main() {\n"
                                           "    fragColor = o_color;\n"
                                           "}";

static shared<program> P_builtin_colored = nullptr, P_builtin_textured = nullptr;

shared<program> make_program(builtin_shader_type type)
{
    if (P_builtin_colored == nullptr || P_builtin_textured == nullptr)
    {
        P_builtin_colored = make_program(P_dvert_colored, P_dfrag_colored, [](shared<program> program) {
            program->get_attrib(0).layout(shader_vertex_data_type::FLOAT, 2, 16, 0);
            program->get_attrib(1).layout(shader_vertex_data_type::HALF_FLOAT, 4, 16, 8);

            if (program->cached_uniforms.size() > 0)
                return;
            program->cache_uniform("u_proj"); // 0
        });
        P_builtin_textured = make_program(P_dvert_textured, P_dfrag_textured, [](shared<program> program) {
            program->get_attrib(0).layout(shader_vertex_data_type::FLOAT, 2, 24, 0);
            program->get_attrib(1).layout(shader_vertex_data_type::HALF_FLOAT, 4, 24, 8);
            program->get_attrib(2).layout(shader_vertex_data_type::FLOAT, 2, 24, 16);

            if (program->cached_uniforms.size() > 0)
                return;
            program->cache_uniform("u_proj");                    // 0
            program->cache_uniform("u_tex").set_texture_unit(1); // 1
        });
    }
    switch (type)
    {
    case builtin_shader_type::COLORED:
        return P_builtin_colored;
    case builtin_shader_type::TEXTURED:
        return P_builtin_textured;
    }
    return nullptr;
}

} // namespace arcaie::gfx
