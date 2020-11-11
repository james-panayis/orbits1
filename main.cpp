#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>


#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include "font.hpp"
#include "shader_text.hpp"


#include "points.hpp"
#include "draw.hpp"

int ctx_{0};

int viewport_width_{0};
int viewport_height_{0};

int pixel_width_{0};
int pixel_height_{0};

double pixel_ratio_{1};

james::points p;
james::draw* d = nullptr;

int fcount = 0;

int xshift{0};
int yshift{0};

float iter1{1};
float iter2{1};
float iter3{1};
float iter4{1};
float iter5{0};

float s{0.0};
float r1{0};
float r2{0};
float i1{0};
float i2{0};

float mx{0};
float my{0};

double mouse_x{0};
double mouse_y{0};

float pattern{1};

int iteration{0};

unsigned int sphere_id_;

james::font font_;

//pass this to shaders
//scale * windowMatrix_ * viewMatrix_
//windowMatrix * reverse shift to origin * zoom * rotate * shift from to origin
float projectionMatrix_[16];

//deals with rotations of view
float viewMatrix_[16]   = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

james::quaternion rotation_;

//deals with window (size/view)
float windowMatrix_[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

//divide all distances by this, and shift bottom-left to centre based on this
float scale_{500000};

void set_windowMatrix(int width, int height)
{
  windowMatrix_[0] = 2.0f / width;
  windowMatrix_[1] = 0;
  windowMatrix_[2] = 0;
  windowMatrix_[3] = 0;

  windowMatrix_[4] = 0;
  windowMatrix_[5] = 2.0f / height;
  windowMatrix_[6] = 0;
  windowMatrix_[7] = 0;

  windowMatrix_[8] = 0;
  windowMatrix_[9] = 0;
  windowMatrix_[10] = -0.8/(float)std::max(width, height);;//-0.8/(float)std::max(width, height);
  windowMatrix_[11] = -1.0/(float)std::max(width, height);

  windowMatrix_[12] = -1;
  windowMatrix_[13] = -1;
  windowMatrix_[14] = 0.3;//0; //0.6;  // 0.5
  windowMatrix_[15] = 1;
}

void set_projection_matrix(const james::vec3& offset)
{
  //shift from centre of window to origin
  float t1[16];
  james::matrix44_shift(t1, -pixel_width_ / 2, -pixel_height_ / 2, 0);

  //scale
  float t2[16];
  james::matrix44_divide(t2, t1, scale_);

  //rotate
  float t3[16];
  matrix44_from_quaternion(t3, rotation_);
  james::matrix44_mult(t1, t3, t2);

  //shift from origin to centre of window
  james::matrix44_shift(t2, pixel_width_ / 2, pixel_height_ / 2, 0);
  james::matrix44_mult(t2, t1);

  //window
  james::matrix44_mult(projectionMatrix_, windowMatrix_, t2);
}


const char * emscripten_event_type_to_string(int event_type) {
  const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick",
                           "mousemove", "wheel", "resize", "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation",
                           "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange", "visibilitychange",
                           "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected",
                           "beforeunload", "batterychargingchange", "batterylevelchange", "webglcontextlost",
                           "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout", "(invalid)" };
  ++event_type;
  if (event_type < 0) event_type = 0;
  if (event_type >= sizeof(events)/sizeof(events[0])) event_type = sizeof(events)/sizeof(events[0])-1;


  return events[event_type];
};



EM_BOOL window_size_changed(int event_type, const EmscriptenUiEvent *uiEvent, void *user_data)
{

  if (event_type == EMSCRIPTEN_EVENT_RESIZE) {
    double w, h;
    EMSCRIPTEN_RESULT r = emscripten_get_element_css_size("canvas", &w, &h);
    if (r != EMSCRIPTEN_RESULT_SUCCESS) {
      fprintf(stderr, "Unable to call: emscripten_get_canvas_element_size\n");
      return false;
    }

    viewport_width_ = (int)w;
    viewport_height_ = (int)h;

    pixel_width_  = round(w * pixel_ratio_);
    pixel_height_ = round(h * pixel_ratio_);

    emscripten_set_canvas_element_size("canvas", pixel_width_, pixel_height_);

    glViewport(0, 0, pixel_width_, pixel_height_);
    set_windowMatrix(pixel_width_, pixel_height_);

    return EM_TRUE;
  }

  return EM_FALSE;
}


EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent *e, void *user_data)
{

  // for mousemove (could have multiple buttons pressed)

  if (e->buttons != 0)
  {
    double temp_y = 8 * (static_cast<double>(e->canvasY) / std::max(viewport_width_, viewport_height_) - mouse_y);
    double temp_x = 8 * (static_cast<double>(e->canvasX) / std::max(viewport_width_, viewport_height_) - mouse_x);

    rotation_ = james::quaternion(std::cos(temp_x / 2), 0, std::sin(temp_x / 2), 0) * rotation_;
    rotation_ = james::quaternion(std::cos(temp_y / 2), std::sin(temp_y / 2), 0, 0) * rotation_;

    //printf("r: %f\n", (float)rotation_.r*rotation_.r+rotation_.x*rotation_.x+rotation_.y*rotation_.y+rotation_.z*rotation_.z); //always 1

  }

  mouse_x = static_cast<double>(e->canvasX) / std::max(viewport_width_, viewport_height_);
  mouse_y = static_cast<double>(e->canvasY) / std::max(viewport_width_, viewport_height_);


  /*
  if (e->buttons == 0) {
    incoming(source::MOUSE, 0, state::EMPTY, ,  * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
  } else {
    if (e->buttons & 1) incoming(source::BUTTON, 0, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
    if (e->buttons & 2) incoming(source::BUTTON, 1, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
    if (e->buttons & 4) incoming(source::BUTTON, 2, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
  }

  // for others (individual button presses/releases fire off individual event callbacks)

  incoming(source::BUTTON, e->button, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, true);

*/

  return EM_TRUE;
}

EM_BOOL touch_callback(int event_type, const EmscriptenTouchEvent *e, void *user_data)
{
/*
  for (const auto& t : e->touches) {
    if (t.isChanged) {
      incoming(source::TOUCH, t.identifier, state::DOWN, t.canvasX * gl::pixel_ratio_, (gl::viewport_height_ - t.canvasY) * gl::pixel_ratio_, true);
    }
  }
*/
  printf("touch_cb\n");


  for (int i = 0; i < e->numTouches; i++)
  {
    const auto& t = e->touches[i];

    float x = t.clientX;// * pixel_ratio_;
    float y = t.clientY;// * pixel_ratio_);


    if (t.isChanged == EM_TRUE)
    {
      switch (event_type)
      {
        case EMSCRIPTEN_EVENT_TOUCHSTART:
        printf("down\n");
        mouse_x = static_cast<double>(x) / std::max(viewport_width_, viewport_height_);
        mouse_y = static_cast<double>(y) / std::max(viewport_width_, viewport_height_);
        break;
        case EMSCRIPTEN_EVENT_TOUCHEND:    printf("end\n"); break;
        case EMSCRIPTEN_EVENT_TOUCHMOVE:
        {
          double temp_y = 8 * (static_cast<double>(y) / std::max(viewport_width_, viewport_height_) - mouse_y);
          double temp_x = 8 * (static_cast<double>(x) / std::max(viewport_width_, viewport_height_) - mouse_x);

          rotation_ = james::quaternion(std::cos(temp_x / 2), 0, std::sin(temp_x / 2), 0) * rotation_;
          rotation_ = james::quaternion(std::cos(temp_y / 2), std::sin(temp_y / 2), 0, 0) * rotation_;

          mouse_x = static_cast<double>(x) / std::max(viewport_width_, viewport_height_);
          mouse_y = static_cast<double>(y) / std::max(viewport_width_, viewport_height_);

          break;
        }
        case EMSCRIPTEN_EVENT_TOUCHCANCEL: printf("cancel\n"); break;
      }
    }
  }

  return EM_TRUE;
}

EM_BOOL wheel_callback(int event_type, const EmscriptenWheelEvent *e, void *user_data)
{

  if (e->deltaY < 0)
    scale_ /= 1.04;
  else
    scale_ *= 1.04;

	return EM_FALSE;
}


EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
  printf("%s key: \"%s\", code: \"%s\", location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu, keyCode: %lu, which: %lu\n",
             emscripten_event_type_to_string(event_type), e->key, e->code, e->location,
             e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
             e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which);

  switch(event_type)
  {
    case EMSCRIPTEN_EVENT_KEYDOWN:
    if(*(e->key) == 'q') {
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
          p.add_point ( { {301000000.0 + 21000000.0 * i, -301000000.0 + 100000000.0 * j, 0}, {500.0 + 200 * j, 27.0, 100.0}, 40000000000.0 / iteration * 400.0 / 2.0, 800000 * 4.0} );
        }
      }
      p.set_zero_net_momentum();

    }
    break;
  }

  return EM_TRUE;
}


void generate_frame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //if (fcount++ >= 50) return;

  //p.add_point ( { {301000000.0 + 11000000.0, 101000000.0 + 11000000.0, 0}, {1500.0, 27.0, 100.0}, 40000000000.0, 800000 * 4.0} );
  
  //p.add_point ( { {901000000.0 + 11000000.0, 501000000.0 + 11000000.0, 0}, {0.0, 500.0, 0.0}, 4000000000.0, 800000 * 3.0} );
  //p.add_point ( { {701000000.0 + 11000000.0, 601000000.0 + 11000000.0, 0}, {0.0, -500.0, 0.0}, 4000000000.0, 800000 * 3.0} );
  //p.set_zero_net_momentum();

  p.iterations(3000.0, 1);
  p.print_net_momentum();
  p.print_net_energy();
  
  //p.print_net_energy2();
  auto out = p.get_points();

  /*for (auto& x : out) {
    std::cout << x.id << " " << x.position.x << " " << x.position.y << " " << x.position.z << "     " << x.velocity.x << " " << x.velocity.y << " " << x.velocity.z << "         ";
  }
  std::cout << std::endl;
*/
  std::vector<james::points::point> data;
  for (auto& x : out) data.push_back(x);
    d->set_points(data);

  james::vec3 offset = p.get_position_offset();
  set_projection_matrix(offset);
  d->display(scale_ * pixel_width_ / 2 - offset.x, scale_ * pixel_height_ / 2 - offset.y, -offset.z);

  if (iteration % 300 == 299 && iteration <= 300) {
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 5; j++) {
        p.add_point ( { {301000000.0 + 11000000.0 * i, 101000000.0 + 11000000.0 * j, 0}, {1500.0, 27.0, 100.0}, 40000000000.0 / iteration * 400.0 / 2.0, 800000 * 4.0} );
      }
    }
    p.set_zero_net_momentum();

  }

  if (p.size() < 20)
  {
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 5; j++) {
        p.add_point ( { {301000000.0 + 21000000.0 * i, -301000000.0 + 100000000.0 * j, 0}, {500.0 + 200 * j, 27.0, 100.0}, 40000000000.0 / iteration * 400.0 / 2.0, 800000 * 4.0} );
      }
    }
    p.set_zero_net_momentum();
  }

  iteration++;
}
 
int main(int argc, char *argv[])
{

  double w, h;
  EMSCRIPTEN_RESULT r = emscripten_get_element_css_size("canvas", &w, &h);

  viewport_width_ = (int)w;
  viewport_height_ = (int)h;

  pixel_ratio_ = emscripten_get_device_pixel_ratio();

  pixel_width_  = round(w * pixel_ratio_);
  pixel_height_ = round(h * pixel_ratio_);

  // window ui events
  auto res = emscripten_set_resize_callback(0, nullptr, 1, window_size_changed);

  // mouse events
  res = emscripten_set_click_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mousedown_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mouseup_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_dblclick_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mousemove_callback(0, nullptr, 1, mouse_callback);

  // touch events
  res = emscripten_set_touchstart_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchend_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchmove_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchcancel_callback(0, nullptr, 1, touch_callback);

  // key events
  res = emscripten_set_keypress_callback(0, nullptr, 1, key_callback);
  res = emscripten_set_keydown_callback(0, nullptr, 1, key_callback);
  res = emscripten_set_keyup_callback(0, nullptr, 1, key_callback);

  // wheel events
  res = emscripten_set_wheel_callback(0, nullptr, 1, wheel_callback);

  emscripten_set_canvas_element_size("canvas", pixel_width_, pixel_height_);

  EmscriptenWebGLContextAttributes ctxAttrs;
  emscripten_webgl_init_context_attributes(&ctxAttrs);
  ctxAttrs.antialias = true;
  ctx_ = emscripten_webgl_create_context(nullptr, &ctxAttrs);
  emscripten_webgl_make_context_current(ctx_);

  // init shaders

  set_windowMatrix(pixel_width_, pixel_height_);
  glViewport(0, 0, pixel_width_, pixel_height_);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(0, 0, 0, 1.0);
  glEnable (GL_DEPTH_TEST);
  // create texture space for 400x400 terrain with 4 bytes of depth for RGBA
  //std::uint8_t * data = (std::uint8_t *)malloc(pixel_width_ * pixel_height_ * 4);
  //memset(data, 100, pixel_width_ * pixel_height_ * 4);

  // create a terrain object (pushed data to the gpu)

  james::shader_text::init();
  font_.load_font("font-data-40.data", nullptr);

  r1 = -0;
  r2 = 1;
  i1 = -0;
  i2 = 1;

  iter1 = 1;
  iter2 = 1;
  iter3 = 0;
  iter4 = 0;
  iter5 = 0;

  double factor = 4;

  //p.add_point( { {800000000, 300000000, 0}, {0, 7.34767309 / 5.972 * 0.01 * 1000*-1.022/1.0, 0}, 5.972*std::pow(10.0, 24.0)*(6.67408*0.00000000001), 6371000 * factor} );

  p.add_point( { {400'000'000, 300000000, 0}, {0, 7.34767309 / 5.972 * 0.01 * 1000*-1.022/1.0, 0}, 5.972*std::pow(10.0, 24.0)*(6.67408*0.00000000001), 6371000 * factor} );
  p.add_point( { {784400.0*1000.0, 300000000, 0}, /*{0, 1000*1.022/1.0, 100}*/{0, 0, 1000*1.022/1.0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1737.1*1000 * factor} );
  //p.add_point( { {300000000 - 384400.0*1000.0, 250000000, 0}, {0, -1300*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );
  //p.add_point( { {684400.0*1000.0, 250000000, 0}, {0, 1000*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );
  //p.add_point( { {684400.0*1000.0, 250000000, 0}, {0, 1000*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );

  double inc = 0.05;
  
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 5; j++) {
      //p.add_point ( { {151000000.0 + 31000000.0 * i, 41000000.0 + 21000000.0 * j, 0}, {1500, 27, 100}, 1, 1200000 * factor} );
      p.add_point ( { {301000000.0 + 11000000.0 * i, 101000000.0 + 11000000.0 * j, 0}, {1500.0, 27.0, 100.0}, 4000000.0/*40000000000.0*/, 800000 * factor} );
    }
  }

  //p.add_point( { {3*400000000/2, 300000000, 0}, {0, 1000, 0}, 1, 1000000 * factor} );
  //p.add_point( { {400000000/2, 300000000, 0}, {0, -1000, 0}, 1, 1000000 * factor} );
  //p.add_point( { {3*400000000/2, 300000000, 0}, {0, -1000, 0}, 1, 1000000 * factor} );
  //p.add_point( { {400000000/2, 300000000, 0}, {0, 1000, 0}, 1, 1000000 * factor} );

  /*p.add_point( { {1000, 1000, 0}, {0, -0.25, 0}, 0.5, 1 } );
  p.add_point( { {1010, 1000, 0}, {0, 0.25, 0}, 0.5, 1 } );
  p.add_point( { {1080, 1000, 0}, {0, -0.2, 0}, 0.02, 1 } );
  p.add_point( { {1150, 1000, 0}, {0, -0.14, 0}, 0.01, 1 } );
  p.add_point( { {1175, 1000, 0}, {0, -0.2, 0}, 0.01, 1 } );
  p.add_point( { {1400, 1000, 0}, {0, 0.1, 0}, 0.1, 1 } );
  p.add_point( { {-500, -200, 0}, {0, 0, 0}, 0.2, 1 } );
  */
  //p.add_point( { {990, 1000, 0}, {-1, 0, 0}, 1, 1 } );


  p.set_zero_net_momentum();

  d = new james::draw();

  sphere_id_ = d->upload_sphere();

  emscripten_set_main_loop(generate_frame, 0, 0);
}

