#pragma once

#include "shader_text.hpp"
#include "font.hpp"

namespace james {

class widget_text
{

public:

  enum class align { EMPTY, LEFT, RIGHT, CENTER, TOP, BOTTOM };

  widget_text(font* f, const std::string& text, const gl::int_box& coords, align al, gl::colour_rgb c, const gl::rotation& r) :
    font_(f), text_(text), align_(al), colour_(c), rotation_(r), coords_(coords)
  {
    upload_vertex();
  };


  void set_text(const std::string& text) {
    text_ = text;

    upload_vertex();
  };

  
  void display() {
    shader_text::draw(coords_.p1.x + x_align_offset_, coords_.p1.y + y_align_offset_, vertex_buffer_, font_->texture_, colour_, { 1.0, 1.0, 1.0 }, rotation_);
  };



private:

    
  void upload_vertex() {

    std::vector<GLfloat> vertex;

    float pen_x = 0;
    float pen_y = 0;

    y_low_  =  999;
    y_high_ = -999;

    for (int i = 0; i < text_.size();) {

      auto it = font_->glyphs_.find(utf8_to_utf32(text_.c_str() + i));

      i += utf8_surrogate_len(text_.c_str() + i);

      if (it == font_->glyphs_.end()) continue; // skip unknown character

      auto& glyph = it->second;

      float x = pen_x + glyph.offset_x;
      float y = pen_y + glyph.offset_y - glyph.height;
      float w  = glyph.width;
      float h  = glyph.height;

      GLfloat glyph_vertex[36] = { x,     y,     0, 1,    glyph.s0, glyph.t1,
                                   x + w, y,     0, 1,    glyph.s1, glyph.t1,
                                   x,     y + h, 0, 1,    glyph.s0, glyph.t0,
                                   x + w, y,     0, 1,    glyph.s1, glyph.t1,
                                   x,     y + h, 0, 1,    glyph.s0, glyph.t0,
                                   x + w, y + h, 0, 1,    glyph.s1, glyph.t0 };

      vertex.insert(vertex.end(), glyph_vertex, glyph_vertex + 36);

      pen_x += ceil(glyph.advance_x);
      //pen_x += glyph.advance_x;
      pen_y += ceil(glyph.advance_y);
      //pen_y += glyph.advance_y;

      if (y < y_low_) y_low_ = y;
      if (y + h > y_high_) y_high_ = y + h;

      //printf("char: %c offset: (%d, %d) size: (%f, %f) advance: (%f, %f) font: (%f, %f, %f)\n", text_[i-1], glyph.offset_x, glyph.offset_y, w, h, glyph.advance_x, glyph.advance_y, font_->ascender_, font_->descender_, font_->height_);
    }

    max_width_ = ceil(pen_x);

    // align the coords so the center of the text is at (0,0) - allows for rotation

    int x_shift = max_width_/2;
    int y_shift = (y_high_ + y_low_)/2; // NB: y_low is usually negative

    for (int i = 0; i < vertex.size(); i += 6) {
      vertex[i]   -= x_shift;
      vertex[i+1] -= y_shift;
    }

    // align it to the box

    auto w = coords_.p2.x - coords_.p1.x;
    auto h = coords_.p2.y - coords_.p1.y;

    if (align_ == align::LEFT) {
      x_align_offset_ = x_shift;
      y_align_offset_ = h / 2.0;
    }
    if (align_ == align::RIGHT) {
      x_align_offset_ = w - x_shift;
      y_align_offset_ = h / 2.0;
    }
    if (align_ == align::CENTER) {
      x_align_offset_ = w / 2.0;
      y_align_offset_ = h / 2.0;
    }

    vertex_buffer_.upload(vertex, 6, GL_TRIANGLES);
  };


  int utf8_surrogate_len(const char* c) {

    if (!c) return 0;

    if ((c[0] & 0x80) == 0) return 1;

    auto test_char = c[0];

    int result = 0;

    while (test_char & 0x80) {
      test_char <<= 1;
      result++;
    }

    return result;
  };


  int utf8_strlen(const char* c) {

    const char* ptr = c;
    int result = 0;

    while (*ptr) {
      ptr += utf8_surrogate_len(ptr);
      result++;
    }

    return result;
  };


  int utf8_to_utf32(const char * c) {
    uint32_t result = -1;

    if (!c) return result;

    if ((c[0] & 0x80) == 0) result = c[0];

    if ((c[0] & 0xC0) == 0xC0 && c[1]) result = ((c[0] & 0x3F) << 6) | (c[1] & 0x3F);

    if ((c[0] & 0xE0) == 0xE0 && c[1] && c[2]) result = ((c[0] & 0x1F) << (6 + 6)) | ((c[1] & 0x3F) << 6) | (c[2] & 0x3F);

    if ((c[0] & 0xF0) == 0xF0 && c[1] && c[2] && c[3]) result = ((c[0] & 0x0F) << (6 + 6 + 6)) | ((c[1] & 0x3F) << (6 + 6)) | ((c[2] & 0x3F) << 6) | (c[3] & 0x3F);

    if ((c[0] & 0xF8) == 0xF8 && c[1] && c[2] && c[3] && c[4]) result = ((c[0] & 0x07) << (6 + 6 + 6 + 6)) | ((c[1] & 0x3F) << (6 + 6 + 6)) | ((c[2] & 0x3F) << (6 + 6)) | ((c[3] & 0x3F) << 6) | (c[4] & 0x3F);

    return result;
  };


  gl::buffer vertex_buffer_;

  align align_;

  std::string text_;

  font * font_{nullptr};

  gl::colour_rgb colour_;
  gl::rotation rotation_;

  int x_align_offset_{0};
  int y_align_offset_{0};

  int y_low_, y_high_, max_width_; // y is relative to font baseline

  gl::int_box coords_;

}; // widget_text


}
