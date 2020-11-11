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

extern float projectionMatrix_[16];

extern unsigned int sphere_id_;

extern int iteration;


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


		/*void set_points(std::vector<points::point> ps)
		{
			v.clear();
			v.reserve(36*ps.size());
			boxno_ = 0;
			for (auto& p : ps) {
				//addbox(p.position.x - 10 * p.radius / viewMatrix_[0], p.position.y - 10 * p.radius / viewMatrix_[0], 20 * p.radius / viewMatrix_[0], 20 * p.radius / viewMatrix_[0], 1.0/12.7, 1.0/13, 1.0/12.8, 1.0/13);
				addbox(p.position.x - p.radius, p.position.y - p.radius, p.position.z, 2 * p.radius, 2 * p.radius, 1.0/12.7, 1.0/13, 1.0/12.8, 1.0/13);
			}
			bind();
		}*/


		void set_points(std::vector<points::point> ps)
		{
			v.clear();
			v.reserve(24 * ps.size());
			for (auto & p : ps) {
				addsphere(p.position.x, p.position.y, p.position.z, ps[0].position.x, ps[0].position.y, ps[0].position.z, p.radius, p.radius / 16000000.0, p.radius / 600000000.0, 1600000.0 / p.radius, 1);
			}
		}


		void addsphere(float x, float y, float z, float lx, float ly, float lz, float r, float red, float green, float blue, float alpha)
		{
			v.push_back(x);
			v.push_back(y);
			v.push_back(z);
			v.push_back(1);

			v.push_back(r*1.2); //radius
			v.push_back((lx - x) / 100000000); //light source coords
			v.push_back((ly - y) / 100000000);
			v.push_back((lz - z) / 100000000);

			v.push_back(red);
			v.push_back(green);
			v.push_back(blue);
			v.push_back(alpha);
		}


		unsigned int upload_sphere()
		{
			// build sphere vertex/normals
			float rrt3 = 0.5773502692;

			std::vector<float> s {

				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     0,     -1,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     0,     -1,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     0,     -1,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     0,     -1,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,

				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     0,     +1,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     0,     +1,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     0,     +1,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     0,     +1,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,

				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-1, 0,     0,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-1, 0,     0,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-1, 0,     0,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-1, 0,     0,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,

				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				+1, 0,     0,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+1, 0,     0,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+1, 0,     0,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+1, 0,     0,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,

				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     -1, 0,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     -1, 0,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     -1, 0,    0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     -1, 0,    0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,

				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     +1, 0,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				0,     +1, 0,    0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     +1, 0,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				0,     +1, 0,    0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,






				/*+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,

				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,

				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,

				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,

				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				+rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				+rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,

				-rrt3, +rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, -rrt3, 0, 0, 0, 0, 0,
				-rrt3, -rrt3, +rrt3, 0, 0, 0, 0, 0,
				-rrt3, +rrt3, +rrt3, 0, 0, 0, 0, 0,*/
			};

			for (std::uint32_t i = 0; i < s.size(); i += 8)
			{
				s[i + 4] = s[i] / 1.7320508;
				s[i + 5] = s[i + 1] / 1.7320508;
				s[i + 6] = s[i + 2] / 1.7320508;
			}

			// create opengl buffer_id
			unsigned int buffer_id;
			glGenBuffers(1, &buffer_id);

			// upload sphere data to opengl
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
			glBufferData(GL_ARRAY_BUFFER, s.size() * sizeof(float), s.data(), GL_STATIC_DRAW);

			return buffer_id;
		}




		void display(float xPos, float yPos, float zPos) {
			
			glUseProgram(program_);

			glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, projectionMatrix_);

			glUniform4f(uniformOffset_, xPos, yPos, zPos, 0.0);


			glBindBuffer(GL_ARRAY_BUFFER, sphere_id_);
			glEnableVertexAttribArray(attribM_position_);
			glEnableVertexAttribArray(attribM_normal_);

			glVertexAttribPointer(attribM_position_, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const GLvoid *)0);
			glVertexAttribPointer(attribM_normal_,   4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const GLvoid *)(4 * sizeof(GLfloat)));

			for (std::uint32_t i = 0; i < v.size(); i += 12)
			{
				//glUniform4f(uniformRot_,      iteration/100.0,0,0,0);//v[i]/8000000.0, v[i+1]/8000000.0, v[i+2]/8000000.0, 0);
				glUniform4f(uniformRot_,      0,      0,      0,      0      );
				glUniform4f(uniformPosition_, v[i+0], v[i+1], v[i+2], v[i+3] );
				glUniform4f(uniformRadius_,   v[i+4], v[i+5], v[i+6], v[i+7] );
				glUniform4f(uniformColor_,    v[i+8], v[i+9], v[i+10], v[i+11] );
				//glUniform4f(uniformColor_,    ((float)((i+2)%3))/2, ((float)(i%10))/9, ((float)(i%7))/6, v[i+11] );

				glDrawArrays(GL_TRIANGLES, 0, 12*6);
			}

		};

		

	private:


		bool create_shader()
		{
			GLint status;

			// build the vertex shader

			std::string prog = "\
				attribute vec4 m_position; \
				attribute vec4 m_normal; \
				uniform mat4 proj; \
				uniform vec4 offset; \
				uniform vec4 rot; \
				uniform vec4 radius; \
				uniform vec4 position; \
				varying vec4 v_normal; \
				void main() \
				{ \
					float sin_x = sin(rot.x); \
					float cos_x = cos(rot.x); \
					float sin_y = sin(rot.y); \
					float cos_y = cos(rot.y); \
					float sin_z = sin(rot.z); \
					float cos_z = cos(rot.z); \
					mat4 rotation = mat4( cos_y * cos_z, cos_y * sin_z, -sin_y, 0, cos_z * sin_x * sin_y - cos_x * sin_z, cos_x * cos_z + sin_x * sin_y * sin_z, cos_y * sin_x, 0, cos_x * cos_z * sin_y + sin_x * sin_z, cos_x * sin_y * sin_z - cos_z * sin_x, cos_x * cos_y, 0, 0, 0, 0, 1 ); \
					v_normal = rotation * m_normal; \
					gl_Position = proj * ((radius.x * rotation * m_position) + position + offset); \
				} \
				";


			//gl_VertexID
			const char * c_str = prog.c_str();

			vertexShader_ = glCreateShader(GL_VERTEX_SHADER);

			glShaderSource(vertexShader_, 1, &c_str, NULL);
			glCompileShader(vertexShader_);

			glGetShaderiv(vertexShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not compile vertex\n"; check_compiled(vertexShader_); return false; }

			// build the fragment shader

			prog = "\
				uniform highp vec4 color; \
				uniform highp vec4 radius; \
				varying highp vec4 v_normal; \
				highp vec4 lightVector = vec4(radius.y, radius.z, radius[3], 0.0); \
				void main() \
				{ \
					highp float brightness = dot(v_normal, lightVector) + 0.00001 / radius.y + 0.00001 / radius.z + 0.00001 / radius[3]; \
				  gl_FragColor = vec4(color.xyz * (0.5 + brightness * 1.0), color.a); \
				} \
				";

			c_str = prog.c_str();

			fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(fragmentShader_, 1, &c_str, NULL);
			glCompileShader(fragmentShader_);

			glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not compile fragment\n"; check_compiled(fragmentShader_); return false; }

			// link the program and store the entry points

			program_ = glCreateProgram();

			glAttachShader(program_, vertexShader_);
			glAttachShader(program_, fragmentShader_);

			glLinkProgram(program_);

			glGetProgramiv(program_, GL_LINK_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not link\n"; return false; }

			attribM_position_  = glGetAttribLocation(program_,  "m_position");
			attribM_normal_    = glGetAttribLocation(program_,  "m_normal");
			uniformProj_       = glGetUniformLocation(program_, "proj");
			uniformOffset_     = glGetUniformLocation(program_, "offset");
			uniformRot_        = glGetUniformLocation(program_, "rot");
			uniformRadius_     = glGetUniformLocation(program_, "radius");
			uniformPosition_   = glGetUniformLocation(program_, "position");
			uniformColor_      = glGetUniformLocation(program_, "color");

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
		double rot = 0;

		// Shader setings

		inline static bool shaderCreated_ = false;

		inline static GLuint program_ = 0;

		inline static GLuint vertexShader_ = 0;
		inline static GLuint fragmentShader_ = 0;

		inline static GLint  attribM_position_ = 0;
		inline static GLint  attribM_normal_ = 0;

		inline static GLint  uniformProj_ = 0;
		inline static GLint  uniformOffset_ = 0;
		inline static GLint  uniformRot_ = 0;
		inline static GLint  uniformRadius_ = 0;
		inline static GLint  uniformPosition_ = 0;
		inline static GLint  uniformColor_ = 0;

	}; // class coins

} // namespace
