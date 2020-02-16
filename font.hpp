#pragma once

#include "emscripten.h"
#include <map>

#include "gl.hpp"

namespace james {

class font {

public:

  ~font() {
    delete raw_data_;
  }

  struct kerning_t {
    std::uint32_t codepoint;
    float         kerning;
  };

  struct texture_glyph {
    std::uint32_t codepoint;

    int           width;
    int           height;

    int           offset_x;
    int           offset_y;

    float         advance_x;
    float         advance_y;

    float         s0;
    float         t0;
    float         s1;
    float         t1;

    std::size_t   kerning_count;
    kerning_t     kerning[1];
  };

  std::size_t       tex_width_{0};
  std::size_t       tex_height_{0};
  std::size_t       tex_depth_{0};

  std::byte *       tex_data_{nullptr};

  float             size_{0};
  float             height_{0};
  float             linegap_{0};
  float             ascender_{0};
  float             descender_{0};

  std::size_t       glyphs_count_{0};

  std::map<std::uint32_t, texture_glyph> glyphs_;

  void load_font(std::string font_file_name, std::function< void (bool)> cb) {

    on_load_callback_ = cb;
    font_file_name_ = font_file_name;

    // pull font files accoring to gl::pixel_ratio

    request_handle_ = emscripten_async_wget2_data(std::string("fonts/" + font_file_name).c_str(), "GET", "", this, EM_FALSE,

      [] (unsigned int handle, void * user_data, void * data, unsigned int data_size) {
        auto handler = reinterpret_cast<font *>(user_data);
        handler->onload(handle, data, data_size);
        handler->request_handle_ = 0;
      },

      [] (unsigned int handle, void * user_data, int error_code, const char * error_msg) {
        auto handler = reinterpret_cast<font *>(user_data);
        handler->onerror(handle, error_code, error_msg);
        handler->request_handle_ = 0;
      },

      [] (unsigned int handle, void * user_data, int number_of_bytes, int total_bytes) {
        auto handler = reinterpret_cast<font *>(user_data);
        handler->onprogress(handle, number_of_bytes, total_bytes);
      }
    );
  };


  gl::texture texture_;


private:


  void onload(unsigned int handle, void * data, unsigned int data_size) {

    printf("Loaded font data size: %u\n", data_size);

    raw_data_ = (std::byte *)data;
    raw_data_size_ = data_size;

    // check the data is ok and then build fonts values

    std::byte * p = raw_data_;

    tex_width_  = *(std::size_t *)p;                 p += 8;
    tex_height_ = *(std::size_t *)p;                 p += 8;
    tex_depth_  = *(std::size_t *)p;                 p += 8;

    tex_data_   = p;                                 p += (tex_width_ * tex_height_ * tex_depth_);

    size_       = *(float *)p;                       p += 4;
    height_     = *(float *)p;                       p += 4;
    linegap_    = *(float *)p;                       p += 4;
    ascender_   = *(float *)p;                       p += 4;
    descender_  = *(float *)p;                       p += 4;

    glyphs_count_ = *(std::size_t *)p;               p += 8;

    //printf("width: %d height: %d depth: %d size: %f glyphs: %d\n", (int)tex_width_, (int)tex_height_, (int) tex_depth_, size_, (int)glyphs_count_);

    for (int i = 0; i < glyphs_count_; i++) {

      //printf("glyph: %d is: %u\n", i, *(std::uint32_t *)p);

      glyphs_[*(std::uint32_t *)p] = *(texture_glyph *)p;   p += 4 + 8 + 8 + 8 + 16 + 8 + 8;
    }

    texture_.upload(tex_data_, tex_width_ * tex_height_ * tex_depth_, tex_width_, tex_height_, tex_depth_);

    delete raw_data_;
    raw_data_ = nullptr;
    raw_data_size_ = 0;

    if (on_load_callback_) on_load_callback_(true);
  };


  void onerror(unsigned int handle, int error_code, const char * error_msg) {
    printf("failed to get fonts with error code: %d msg: %s\n", error_code, error_msg);

    // retry after x seconds...

    if (on_load_callback_) on_load_callback_(false);
  };


  void onprogress(unsigned int handle, int number_of_bytes, int total_bytes) {
    printf("received font data: %d total is: %d\n", number_of_bytes, total_bytes);
  };


  std::function< void (bool)> on_load_callback_;

  std::string font_file_name_;

  std::byte * raw_data_{nullptr};
  std::uint32_t raw_data_size_{0};

  int request_handle_{0};

};


}
