#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <GLES3/gl3.h>


namespace james {

class gl {

public:

  class buffer {

    friend gl;

    public:

    ~buffer() {
      if (id_) glDeleteBuffers(1, &id_);
    };


    bool upload(const std::vector<GLfloat>& d, GLsizei stride, GLenum mode) {
      return upload((std::byte *)d.data(), d.size() * sizeof(GLfloat), stride * sizeof(GLfloat), mode);
    };


    bool upload(std::byte * data, int total_bytes, GLsizei stride, GLenum mode) {

      if (!id_) glGenBuffers(1, &id_);

      glBindBuffer(GL_ARRAY_BUFFER, id_);
      glBufferData(GL_ARRAY_BUFFER, total_bytes, data, GL_STATIC_DRAW);

      stride_ = stride;
      mode_ = mode;
      count_ = total_bytes / stride_;

      return true;
    };
    

    GLuint  id_{0};
    GLenum  mode_{0};
    GLsizei stride_{0};
    GLsizei count_{0};
  };


  class texture {

    friend gl;

    public:

    texture(texture&& other) {
      id_ = other.id_;
      data_ = other.data_;
      total_bytes_ = other.total_bytes_;
      width_ = other.width_;
      height_ = other.height_;
      depth_ = other.depth_;

      other.id_ = 0;
    };

    texture() { };

    texture(std::byte * data, int total_bytes, int width, int height, int depth) :
      data_(data), total_bytes_(total_bytes), width_(width), height_(height), depth_(depth)
    {
    };


    ~texture() {
      if (id_) glDeleteTextures(1, &id_);
    };


    texture& operator=(texture&& other) {
      id_ = other.id_;
      data_ = other.data_;
      total_bytes_ = other.total_bytes_;
      width_ = other.width_;
      height_ = other.height_;
      depth_ = other.depth_;

      other.id_ = 0;

      return *this;
    };


    bool upload(std::byte * data, int total_bytes, int width, int height, int depth) {
      data_ = data;
      total_bytes_ = total_bytes;
      width_ = width;
      height_ = height;
      depth_ = depth;

      return upload();
    };


    bool upload() {

      if (data_ == nullptr || width_ == 0 || height_ == 0 || depth_ == 0 || width_ * height_ * depth_ != total_bytes_) {
        printf("Bad data\n");
        return false;
      }

      if (!id_) glGenTextures(1, &id_);

      if (!id_) {
        printf("Unable to generate texture\n");
        return false;
      }

      glBindTexture(GL_TEXTURE_2D, id_);

      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST_MIPMAP_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

      if (depth_ == 1) glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width_, height_, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data_);
      if (depth_ == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data_);
      if (depth_ == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_);

      if (auto err = glGetError(); err != GL_NO_ERROR) {
        printf("got gl error: %u\n", err);
        return false;
      }

      glBindTexture(GL_TEXTURE_2D, 0);

      return true;
    };


    GLuint get_id() const { return id_; };


    GLuint  id_{0};

    std::byte* data_{nullptr};
    int total_bytes_{0};

    int width_{0};
    int height_{0};
    int depth_{0};
  };


  class scissor {

  public:

    scissor(int x, int y, int w, int h) : x_(x), y_(y), w_(w), h_(h) {

      had_scissor_ = glIsEnabled(GL_SCISSOR_TEST);

      if (had_scissor_) {
        glGetIntegerv(GL_SCISSOR_BOX, had_);

        int new_x = std::max(x_, had_[0]);
        int new_y = std::max(y_, had_[1]);

        int new_w = 0;
        int new_x2 = std::min(had_[0] + had_[2], x_ + w_);
        if (new_x2 > new_x) new_w = new_x2 - new_x;

        int new_h = 0;
        int new_y2 = std::min(had_[1] + had_[3], y_ + h_);
        if (new_y2 > new_y) new_h = new_y2 - new_y;

        x_ = new_x;
        y_ = new_y;
        w_ = new_w;
        h_ = new_h;

      } else {
        glEnable(GL_SCISSOR_TEST);
      }

      glScissor(x_, y_, w_, h_);
    };


    ~scissor() {
      if (had_scissor_) {
        glScissor(had_[0], had_[1], had_[2], had_[3]);
      } else {
        glDisable(GL_SCISSOR_TEST);
      }
    }

  private:

    int x_, y_, w_, h_;

    bool had_scissor_;
    int had_[4];
  };

  // colours

  struct colour_rgb { GLfloat r, g, b; };
  struct colour { GLfloat r, g, b, a; };


  // 2 somethings

  template<class T>
  struct any_vec2 {
    T x, y;
    bool operator==(const any_vec2<T>& other) const { return x == other.x && y == other.y; };
    T& operator[](std::size_t i) { return *(T *)((std::byte *)this + (i * sizeof(T))); };
  };

  typedef any_vec2<int>     int_vec2;
  typedef any_vec2<int>     int_point;
  typedef any_vec2<GLfloat> float_point;

  typedef any_vec2<GLfloat> float_vec2;


  // 3 somethings

  template<class T>
  struct any_vec3 {
    T x, y, z;
    bool operator==(const any_vec3<T>& other) const { return x == other.x && y == other.y && z == other.z; };
    //T& operator[](std::size_t i) { return (i == 0) ? x : ((i == 1) ? y : z); };
    T& operator[](std::size_t i) { return *(T *)((std::byte *)this + (i * sizeof(T))); };
  };

  typedef any_vec3<int>     int_vec3;

  typedef any_vec3<GLfloat> float_vec3;
  typedef any_vec3<GLfloat> scale;
  typedef any_vec3<GLfloat> rotation;

  // 4 somethings

  template<class T>
  struct any_vec4 {
    T x, y, z, w;
    bool operator==(const any_vec4<T>& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; };
    //T& operator[](std::size_t i) { return (i == 0) ? x : ((i == 1) ? y : ((i == 2) ? z : w)); };
    T& operator[](std::size_t i) { return *(T *)((std::byte *)this + (i * sizeof(T))); };
  };

  typedef any_vec4<int>     int_vec4;

  typedef any_vec4<GLfloat> float_vec4;
  typedef any_vec4<GLfloat> coord;

  // compound structs

  struct int_box   { int_point   p1, p2; bool operator==(const int_box&   other) const { return p1 == other.p1 && p2 == other.p2; } };
  struct float_box { float_point p1, p2; bool operator==(const float_box& other) const { return p1 == other.p1 && p2 == other.p2; } };



  static void init() {
  };


  static void set_projection(float *p) {
    memcpy(projection_matrix_.data(), p, 16 * sizeof(float));
  };


  static void set_projection(int width, int height) {

    float w = width;
    float h = height;
    float m = std::max(w, h);

    projection_matrix_ = { 2 / w, 0,     0,         0,
                           0,     2 / h, 0,         0,
                           0,     0,     -0.8f / m, -1.0f / m,
                           -1,    -1,    0.3,       1 };
  };


  static void set_x_offset(GLfloat x_offset) { x_offset_ = x_offset; };
  static void set_y_offset(GLfloat y_offset) { y_offset_ = y_offset; };

  static GLfloat get_x_offset() { return x_offset_; };
  static GLfloat get_y_offset() { return y_offset_; };


  static int check_compiled(GLuint shader) {

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
      GLint max_len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

      GLchar err_log[max_len];
      glGetShaderInfoLog(shader, max_len, &max_len, &err_log[0]);
      glDeleteShader(shader);

      fprintf(stderr, "Shader compilation failed: %s\n", err_log);
    }

    return success;
  };


  static int check_linked(GLuint program) {

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
      GLint max_len = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len);

      GLchar err_log[max_len];
      glGetProgramInfoLog(program, max_len, &max_len, &err_log[0]);

      fprintf(stderr, "Program linking failed: %s\n", err_log);
    }

    return success;
  };


  static void do_draw() {
    if (!to_draw_) {
      to_draw_ = true;
      if (!in_draw_) emscripten_resume_main_loop();
    }
  };


  static int viewport_width_, viewport_height_;
  static int pixel_width_, pixel_height_;

  static double pixel_ratio_;

  static std::array<GLfloat, 16> projection_matrix_;

  static GLfloat x_offset_;
  static GLfloat y_offset_;

  static bool to_draw_;
  static bool in_draw_;
};


int gl::viewport_width_ = 0;
int gl::viewport_height_ = 0;

int gl::pixel_width_ = 0;
int gl::pixel_height_ = 0;

double gl::pixel_ratio_ = 1;

std::array<GLfloat, 16> gl::projection_matrix_;

GLfloat gl::x_offset_{0};
GLfloat gl::y_offset_{0};

bool gl::to_draw_{true};
bool gl::in_draw_{false};

}
