#include <gfx/mesh.h>
#include <gfx/buffer.h>
#include <gfx/brush.h>

// clang-format off
#include <gl/glew.h>
#include <gl/gl.h>
// clang-format on

namespace arc::gfx
{

mesh::mesh()
{
    buffer = complex_buffer::make();
    P_brush = buffer->derive_brush();
    P_brush->P_mesh_root = this;
}

mesh::~mesh()
{
    glDeleteVertexArrays(1, &P_vao);
    glDeleteBuffers(1, &P_vbo);
    glDeleteBuffers(1, &P_ebo);
}

std::shared_ptr<brush> mesh::retry()
{
    buffer->clear();
    if (!P_is_direct)
        P_brush->P_is_in_mesh = true;
    return P_brush;
}

void mesh::record()
{
    state = P_brush->P_state;
    if (!P_is_direct)
        P_brush->P_is_in_mesh = false;
}

void mesh::draw(std::shared_ptr<brush> gbrush)
{
    auto old_state = gbrush->P_state;
    auto old_buf = gbrush->wbuf;
    auto old_msh = gbrush->P_mesh_root;

    gbrush->use_state(state);
    gbrush->wbuf = buffer.get();
    gbrush->P_mesh_root = this;
    gbrush->P_clear_when_flush = false;

    gbrush->flush();

    gbrush->P_clear_when_flush = true;
    gbrush->P_mesh_root = old_msh;
    gbrush->wbuf = old_buf;
    gbrush->use_state(old_state);
}

std::shared_ptr<mesh> mesh::make()
{
    std::shared_ptr<mesh> msh = std::make_shared<mesh>();
    std::shared_ptr<complex_buffer> buf = msh->buffer;

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    msh->P_vao = vao;
    msh->P_vbo = vbo;
    msh->P_ebo = ebo;

    return msh;
}

} // namespace arc::gfx
