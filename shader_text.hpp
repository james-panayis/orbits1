#pragma once

#include <iostream>
#include "gl.hpp"


extern float projectionMatrix_[16];

namespace james {

class shader_text {

public:

  static GLuint program_;

  static GLuint vertex_shader_;
  static GLuint fragment_shader_;

  static GLint  attrib_position_;
  static GLint  attrib_texcoords_;
  static GLint  attrib_sample_;
  static GLint  uniform_proj_;
  static GLint  uniform_offset_;
  static GLint  uniform_scale_;
  static GLint  uniform_rot_;
  static GLint  uniform_col_;

  static bool created_;


  static void init() {

    if (created_) return;
    created_ = true;

    std::string prog = "\
      attribute vec4 position; \
      attribute vec2 a_texCoord; \
      uniform mat4 proj; \
      uniform vec4 offset; \
      uniform vec2 scale; \
      uniform vec4 rot; \
      varying highp vec4 v_texCoord; \
      void main() \
      { \
        mat4 a_scale = mat4(scale.x, 0.0, 0.0, 0.0, 0.0, scale.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0); \
        mat4 rotation = mat4(cos(rot.z), sin(rot.z), 0, 0,  -sin(rot.z), cos(rot.z), 0, 0,  0, 0, 1, 0,  0, 0, 0, 1); \
        vec4 t = rotation * position; \
        gl_Position = proj * ((a_scale * t) + offset); \
        v_texCoord = vec4(a_texCoord.x, a_texCoord.y, 0.0, 1.0); \
      } \
      ";

    const char * c_str = prog.c_str();

    vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_, 1, &c_str, NULL);
    glCompileShader(vertex_shader_);
    gl::check_compiled(vertex_shader_);

    prog = "\
      varying highp vec4 v_texCoord; \
      uniform mediump vec3 col; \
      uniform sampler2D s_texture; \
      void main() \
      { \
        gl_FragColor = vec4(col, texture2DProj(s_texture, v_texCoord.xyw).a); \
      } \
      ";

    c_str = prog.c_str();

    fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_, 1, &c_str, NULL);
    glCompileShader(fragment_shader_);
    gl::check_compiled(fragment_shader_);

    program_ = glCreateProgram();
    glAttachShader(program_, vertex_shader_);
    glAttachShader(program_, fragment_shader_);
    glLinkProgram(program_);
    gl::check_linked(program_);

    attrib_position_  = glGetAttribLocation(program_,  "position");
    attrib_texcoords_ = glGetAttribLocation(program_,  "a_texCoord");

    attrib_sample_    = glGetUniformLocation(program_,  "s_texture");
    uniform_proj_     = glGetUniformLocation(program_, "proj");
    uniform_offset_   = glGetUniformLocation(program_, "offset");
    uniform_scale_    = glGetUniformLocation(program_, "scale");
    uniform_rot_      = glGetUniformLocation(program_, "rot");
    uniform_col_      = glGetUniformLocation(program_, "col");
  };


  static void draw(GLfloat x_offset, GLfloat y_offset, const gl::buffer& vertex_buffer, const gl::texture& t, const gl::colour_rgb& c, const gl::scale& s, const gl::rotation& r) {

    glUseProgram(program_);

    glUniformMatrix4fv(uniform_proj_, 1, GL_FALSE, projectionMatrix_);

    GLfloat x = gl::x_offset_ + x_offset;
    GLfloat y = gl::y_offset_ + y_offset;

    glUniform4f(uniform_offset_, x, y, 0.0, 0.0);
    glUniform2f(uniform_scale_, s.x, s.y);
    glUniform4f(uniform_rot_, r.x, r.y, r.z, 0.0);
    glUniform3f(uniform_col_, c.r, c.g, c.b);

    glUniform1i(attrib_sample_, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.id_);
    glEnableVertexAttribArray(attrib_position_);
    glEnableVertexAttribArray(attrib_texcoords_);

    glVertexAttribPointer(attrib_position_, 4, GL_FLOAT, GL_FALSE, vertex_buffer.stride_, (const GLvoid *)0);
    glVertexAttribPointer(attrib_texcoords_, 2, GL_FLOAT, GL_FALSE, vertex_buffer.stride_, (const GLvoid *)(0 + (4 * sizeof(GLfloat))));

    glBindTexture(GL_TEXTURE_2D, t.get_id());

    glDrawArrays(vertex_buffer.mode_, 0, vertex_buffer.count_);
  };
};


GLuint shader_text::program_{0};
GLuint shader_text::vertex_shader_{0};
GLuint shader_text::fragment_shader_{0};

GLint shader_text::attrib_position_{0};
GLint shader_text::attrib_texcoords_{0};
GLint shader_text::attrib_sample_{0};
GLint shader_text::uniform_proj_{0};
GLint shader_text::uniform_offset_{0};
GLint shader_text::uniform_scale_{0};
GLint shader_text::uniform_rot_{0};
GLint shader_text::uniform_col_{0};

bool shader_text::created_{false};


}
