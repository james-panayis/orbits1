#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


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

float pattern{1};

int iteration{0};

james::font font_;

float projectionMatrix_[16];
float viewMatrix_[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
float startMatrix_[16];

void set_projection_matrix(int width, int height)
{
  projectionMatrix_[0] = 2.0f / width;
  projectionMatrix_[1] = 0;
  projectionMatrix_[2] = 0;
  projectionMatrix_[3] = 0;

  projectionMatrix_[4] = 0;
  projectionMatrix_[5] = 2.0f / height;
  projectionMatrix_[6] = 0;
  projectionMatrix_[7] = 0;

  projectionMatrix_[8] = 0;
  projectionMatrix_[9] = 0;
  projectionMatrix_[10] = -0.8/(float)std::max(width, height);
  projectionMatrix_[11] = -1.0/(float)std::max(width, height);

  projectionMatrix_[12] = -1;
  projectionMatrix_[13] = -1;
  projectionMatrix_[14] = 0.3; //0.6;  // 0.5
  projectionMatrix_[15] = 1;

	for (int i = 0; i < 16; i++) startMatrix_[i] = projectionMatrix_[i];

	james::matrix44_mult(projectionMatrix_, startMatrix_, viewMatrix_);
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
    set_projection_matrix(pixel_width_, pixel_height_);

	 // james::matrix44_mult(projectionMatrix_, viewMatrix_, startMatrix_);

    return EM_TRUE;
  }

  return EM_FALSE;
}


EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent *e, void *user_data)
{
/*
  // for mousemove (could have multiple buttons pressed)

  if (e->buttons == 0) {
    incoming(source::MOUSE, 0, state::EMPTY, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
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
  switch (event_type) {
      case EMSCRIPTEN_EVENT_TOUCHSTART:  {
        //r1 *= 2;
        for (int i = 0; i < 9; i++) {
          for (int j = 0; j < 7; j++) {
            p.add_point ( { {301000000.0 + 11000000.0 * i, 101000000.0 + 11000000.0 * j, 0}, {1500.0, 27.0, 100.0}, 1.0, 800000 * 2.2} );
          }
        }
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHEND:    {
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHMOVE:   {
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHCANCEL: {
        break;
      }
      default: printf("Unknown event type: %d\n", event_type);
    }

  return EM_TRUE;
}

EM_BOOL wheel_callback(int event_type, const EmscriptenWheelEvent *e, void *user_data)
{
	if (e->deltaY < 0)
	{
		float mr[16];
		james::matrix44_scale(mr, 1.02);
		float mr2[16];
		james::matrix44_mult(mr2, viewMatrix_, mr);
		for (int i = 0; i < 16; i++) viewMatrix_[i] = mr2[i];
		james::matrix44_mult(projectionMatrix_, startMatrix_, viewMatrix_);
	} else
	{
		float mr[16];
		james::matrix44_scale(mr, 0.98);
		float mr2[16];
		james::matrix44_mult(mr2, viewMatrix_, mr);
		for (int i = 0; i < 16; i++) viewMatrix_[i] = mr2[i];
		james::matrix44_mult(projectionMatrix_, startMatrix_, viewMatrix_);
	}
	return EM_FALSE;
}


EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
//  printf("%s key: \"%s\", code: \"%s\", location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu, keyCode: %lu, which: %lu\n",
//             emscripten_event_type_to_string(event_type), e->key, e->code, e->location,
//             e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
//             e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which);

  return EM_TRUE;
}


void generate_frame()
{
	glClear(GL_COLOR_BUFFER_BIT);

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
  d->display(0, 0);

  if (iteration % 300 == 299 && iteration <= 1500) {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 7; j++) {
        p.add_point ( { {301000000.0 + 11000000.0 * i, 101000000.0 + 11000000.0 * j, 0}, {1500.0, 27.0, 100.0}, 40000000000.0 / iteration * 400.0 / 2.0, 800000 * 4.0} );
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

  // init james's shaders

  set_projection_matrix(pixel_width_, pixel_height_);
  glViewport(0, 0, pixel_width_, pixel_height_);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(1.0, 1.0, 1.0, 1.0);

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

  p.add_point( { {400000000, 300000000, 0}, {0, 7.34767309 / 5.972 * 0.01 * 1000*-1.022/1.0, 0}, 5.972*std::pow(10.0, 24.0)*(6.67408*0.00000000001), 6371000 * factor} );
  p.add_point( { {784400.0*1000.0, 300000000, 0}, {0, 1000*1.022/1.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1737.1*1000 * factor} );
  //p.add_point( { {300000000 - 384400.0*1000.0, 250000000, 0}, {0, -1300*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );
  //p.add_point( { {684400.0*1000.0, 250000000, 0}, {0, 1000*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );
  //p.add_point( { {684400.0*1000.0, 250000000, 0}, {0, 1000*1.022/2.0, 0}, 7.34767309*std::pow(10.0, 22.0)*(6.67408*0.00000000001), 1 } );

  double inc = 0.05;
  
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 7; j++) {
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
  d->load("terrain_atlas.png");

	float mr[16];
	james::matrix44_scale(mr, 1.0/800000.0);

	float mr2[16];

	james::matrix44_mult(mr2, viewMatrix_, mr);
	for (int i = 0; i < 16; i++) viewMatrix_[i] = mr2[i];

	james::matrix44_mult(projectionMatrix_, startMatrix_, viewMatrix_);

  emscripten_set_main_loop(generate_frame, 0, 0);
}

