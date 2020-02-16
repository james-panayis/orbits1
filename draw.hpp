#pragma once

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>

#include <SDL/SDL_image.h>

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <map>

#include "vec3.hpp"

extern float viewMatrix_[16];
extern float projectionMatrix_[16];

namespace james {

	class draw {

	public:

		draw() {

			if (!shaderCreated_) {
				create_shader();
			}

		};

		~draw() {
			if (vertexId_) {
				glDeleteBuffers(1, &vertexId_);
				vertexId_ = 0;
			}
		};

		void set_points(std::vector<points::point> ps)
		{
			v.clear();
			v.reserve(36*ps.size());
			boxno_ = 0;
			for (auto& p : ps) {
				//addbox(p.position.x - 10 * p.radius / viewMatrix_[0], p.position.y - 10 * p.radius / viewMatrix_[0], 20 * p.radius / viewMatrix_[0], 20 * p.radius / viewMatrix_[0], 1.0/12.7, 1.0/13, 1.0/12.8, 1.0/13);
				addbox(p.position.x - p.radius, p.position.y - p.radius, 2 * p.radius, 2 * p.radius, 1.0/12.7, 1.0/13, 1.0/12.8, 1.0/13);
			}
			bind();
		}

		void bind() {
			if (vertexId_ == 0) {
				glGenBuffers(1, &vertexId_);
			}
			glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
			glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
		}

		void addbox(float x1, float y1, float x2, float y2, float X1, float Y1, float X2, float Y2) {

			boxno_ += 6;

			// setup the vertex data

			//note: flipped

			v.push_back(x1);
			v.push_back(y1);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1);
			v.push_back(Y1 + Y2);


			v.push_back(x1 + x2);
			v.push_back(y1);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1 + X2);
			v.push_back(Y1 + Y2);


			v.push_back(x1);
			v.push_back(y1 + y2);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1);
			v.push_back(Y1);


			v.push_back(x1 + x2);
			v.push_back(y1);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1 + X2);
			v.push_back(Y1 + Y2);


			v.push_back(x1);
			v.push_back(y1 + y2);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1);
			v.push_back(Y1);


			v.push_back(x1 + x2);
			v.push_back(y1 + y2);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(X1 + X2);
			v.push_back(Y1);
		}


		void display(float xPos, float yPos) {

                        if (request_handle_ != 0 || textureId_ == 0) return;
			
			glUseProgram(program_);

			glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, projectionMatrix_);

			glUniform4f(uniformOffset_, xPos, yPos, 0.0, 0.0);
			glUniform2f(uniformScale_, 1.0, 1.0);
			glUniform4f(uniformRot_, 0.0, rot, 0.0, 0.0);

			glUniform1i(attribSample_, 0);

			glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
			glEnableVertexAttribArray(attribPosition_);
			glEnableVertexAttribArray(attribTexcoords_);

			glVertexAttribPointer(attribPosition_, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const GLvoid *)0);
			glVertexAttribPointer(attribTexcoords_, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const GLvoid *)(4 * sizeof(GLfloat)));

			glBindTexture(GL_TEXTURE_2D, textureId_);
			glDrawArrays(GL_TRIANGLES, 0, boxno_);
		};

		void load(const std::string url) {
			url_ = url;

			request_handle_ = emscripten_async_wget2_data(url_.c_str(), "GET", "", this, EM_FALSE,

				[](unsigned int handle, void * user_data, void * data, unsigned int data_size) {
				auto handler = reinterpret_cast<draw *>(user_data);
				handler->onload(handle, data, data_size);
			},

				[](unsigned int handle, void * user_data, int error_code, const char * error_msg) {
				auto handler = reinterpret_cast<draw *>(user_data);
				handler->onerror(handle, error_code, error_msg);
			},

				[](unsigned int handle, void * user_data, int number_of_bytes, int total_bytes) {
				auto handler = reinterpret_cast<draw *>(user_data);
				handler->onprogress(handle, number_of_bytes, total_bytes);
			}
			);

		};

		void load(const std::uint8_t * data, int width, int height) {

			if (!textureId_) {

				// setup the texture

				glGenTextures(1, &textureId_);

				glBindTexture(GL_TEXTURE_2D, textureId_);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


			} else {
				glBindTexture(GL_TEXTURE_2D, textureId_);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			glBindTexture(GL_TEXTURE_2D, 0);
		};

	private:

void onload(unsigned int handle, void * data, unsigned int data_size) {

			printf("got data size: %u\n", data_size);

			emscripten_run_preload_plugins_data((char *)data, data_size, "png", this,

				[](void * user_data, const char * file) {
				auto handler = reinterpret_cast<draw *>(user_data);
				handler->ondecoded(file);
			},

				[](void * user_data) {
				auto handler = reinterpret_cast<draw *>(user_data);
				handler->ondecodederror();
			}
			);
		};

		void ondecoded(const char * file) {
			printf("ondecoded: %s\n", file);

			SDL_Surface * image = IMG_Load(file);

			if (!image) {
				printf("IMG_Load of: %s failed with: %s\n", file, IMG_GetError());
				return;
			}

			load((std::uint8_t *)image->pixels, image->w, image->h);

			SDL_FreeSurface(image);

			bind();

			request_handle_ = 0;
		};


		void ondecodederror() {
			printf("ondecodederror\n");
			request_handle_ = 0;
		};

		void onerror(unsigned int handle, int error_code, const char * error_msg) {
			printf("failed to get fonts with error code: %d msg: %s\n", error_code, error_msg);

			request_handle_ = 0;
		};

		void onprogress(unsigned int handle, int number_of_bytes, int total_bytes) {
			printf("received: %d total is: %d\n", number_of_bytes, total_bytes);
		}

		bool create_shader()
		{
			GLint status;

			// build the vertex shader

			std::string prog = "\
				attribute vec4 position; \
				attribute vec2 a_texCoord; \
				uniform mat4 proj; \
				uniform vec4 offset; \
				uniform vec2 scale; \
				uniform vec4 rot; \
				varying vec4 v_texCoord; \
				void main() \
				{ \
					float sin_x = sin(rot.x); \
					float cos_x = cos(rot.x); \
					float sin_y = sin(rot.y); \
					float cos_y = cos(rot.y); \
					float sin_z = sin(rot.z); \
					float cos_z = cos(rot.z); \
					mat4 a_scale = mat4(scale.x, 0.0, 0.0, 0.0, 0.0, scale.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0); \
					mat4 rotation = mat4( cos_y * cos_z, cos_y * sin_z, -sin_y, 0, cos_z * sin_x * sin_y - cos_x * sin_z, cos_x * cos_z + sin_x * sin_y * sin_z, cos_y * sin_x, 0, cos_x * cos_z * sin_y + sin_x * sin_z, cos_x * sin_y * sin_z - cos_z * sin_x, cos_x * cos_y, 0, 0, 0, 0, 1 ); \
					vec4 tpos = vec4(0.0, 0.0, 0.0, 0.0); \
					int txi = 0; \
					if (a_texCoord.x < 0.1) { tpos.x = -30.0; txi = 30; } \
					else { tpos.x = +30.0; txi = -30; } \
					vec4 t = (rotation * tpos) + position + vec4(txi, 0.0, 0.0, 0.0); \
					gl_Position = proj * ((a_scale * t) + offset); \
					v_texCoord = vec4(a_texCoord.x, a_texCoord.y, 0.0, 1.0); \
				} \
				";
			//					mat4 rotation = mat4(cos(rot.z), sin(rot.z), 0, 0,  -sin(rot.z), cos(rot.z), 0, 0,  0, 0, 1, 0,  0, 0, 0, 1); \
			//					vec4 t = rotation * position; \

			//gl_VertexID
			const char * c_str = prog.c_str();

			vertexShader_ = glCreateShader(GL_VERTEX_SHADER);

			glShaderSource(vertexShader_, 1, &c_str, NULL);
			glCompileShader(vertexShader_);

			glGetShaderiv(vertexShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not compile\n"; check_compiled(vertexShader_); return false; }

			// build the fragment shader

			prog = "\
				varying highp vec4 v_texCoord; \
				uniform sampler2D s_texture; \
				void main() \
				{ \
				  gl_FragColor = texture2DProj(s_texture, v_texCoord.xyw); \
				} \
				";

			c_str = prog.c_str();

			fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(fragmentShader_, 1, &c_str, NULL);
			glCompileShader(fragmentShader_);

			glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not compile\n"; check_compiled(fragmentShader_); return false; }

			// link the program and store the entry points

			program_ = glCreateProgram();

			glAttachShader(program_, vertexShader_);
			glAttachShader(program_, fragmentShader_);

			glLinkProgram(program_);

			glGetProgramiv(program_, GL_LINK_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not link\n"; return false; }

			attribPosition_ = glGetAttribLocation(program_, "position");
			attribTexcoords_ = glGetAttribLocation(program_, "a_texCoord");
			attribSample_ = glGetUniformLocation(program_, "s_texture");
			uniformProj_ = glGetUniformLocation(program_, "proj");
			uniformOffset_ = glGetUniformLocation(program_, "offset");
			uniformScale_ = glGetUniformLocation(program_, "scale");
			uniformRot_ = glGetUniformLocation(program_, "rot");

//			glUniform1i(attribSample_, 0); // try having this here rather than for each texture

			shaderCreated_ = true;

			std::cout << "Shaders created\n";
			return true;
		};


		static int check_compiled(GLuint shader) {

			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (success == GL_FALSE) {
				GLint max_len = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

				GLchar err_log[max_len];
				glGetShaderInfoLog(shader, max_len, &max_len, &err_log[0]);
				glDeleteShader(shader);

				std::cout << "Shader compilation failed: " << err_log << std::endl;
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

				std::cout << "Program linking failed: " << err_log << std::endl;
			}

			return success;
		}


		GLuint textureId_ = 0; // Store the opengl texture id
		GLuint vertexId_ = 0;  // Store the opengl vertex id

		std::string url_;

		int request_handle_{ 0 };
		std::vector<float> v;
		int boxno_ = 0;
		double rot = 0;

		// Shader setings

		inline static bool shaderCreated_ = false;

		inline static GLuint program_ = 0;

		inline static GLuint vertexShader_ = 0;
		inline static GLuint fragmentShader_ = 0;

		inline static GLint  attribPosition_ = 0;
		inline static GLint  attribTexcoords_ = 0;
		inline static GLint  attribSample_ = 0;

		inline static GLint  uniformProj_ = 0;
		inline static GLint  uniformOffset_ = 0;
		inline static GLint  uniformScale_ = 0;
		inline static GLint  uniformRot_ = 0;

	}; // class coins

} // namespace
