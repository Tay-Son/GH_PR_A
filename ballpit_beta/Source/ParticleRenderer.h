#pragma once
#define _CRT_SECURE_NO_WARNINGS 1

#include "auxi.h"
#include "interface.h"

#define GLFW_NO_GLU 1
#define GLFW_INCLUDE_GLCOREARB 1


namespace GS {
	class ParticleRenderer {
	public:
		BOOL wait_;
	private:
		//::
		options_vps opt_vps;
		controls_vps ctrl_vps;

		//::
		struct uniforms_block {
			vmath::mat4     mv_matrix;
			vmath::mat4     view_matrix;
			vmath::mat4     proj_matrix;
		};

		struct {
			UINT vertex;
			UINT uniform;
			UINT icos_indices;
			UINT icos_vertices;
		}buffers_;

		struct {
			char title[128];
			int windowWidth;
			int windowHeight;
			int majorVersion;
			int minorVersion;
			int samples;
			union {
				struct {
					unsigned int    fullscreen : 1;
					unsigned int    vsync : 1;
					unsigned int    cursor : 1;
					unsigned int    stereo : 1;
					unsigned int    debug : 1;
					unsigned int    robust : 1;
				};
				unsigned int        all;
			} flags;
		}app_info_;

		struct {
			UINT render_indvisual;
			UINT render_marching_cube;
			UINT ssao;
		}programs_;

		struct {
			BOOL render_indvisual;
			BOOL render_marching_cube;
		}modes_;

		struct {
			struct {
				GLint diffuse_albedo;
				GLint specular_albedo;
				GLint specular_power;
				GLint ambient;
			}dnm;
			struct {
				GLint icos_vertices;
				GLint icos_indices;
			}stt;
		}uniform_loc_;

		//::Objects
		VoxelParticleSystem o_vps_;
		Camera o_camera_;
		ICOSHEDRON o_icoshedron_;

		GLFWwindow* window_;
		UINT vao_;

		//::
	protected:
		static ParticleRenderer* pr;

	public:
		ParticleRenderer() {};
		~ParticleRenderer() {};

		void Run(ParticleRenderer* the_pr) {
			pr = the_pr;
			BOOL is_running = TRUE;
			_LoadOption();
			_InitComponents();
			_InitGL();

			_StartupComponents();
			_LoadProgram();
			_StartupGL();
			do {
				glfwPollEvents();

				_LoadControl();
				_ControlComponents();
				_ControlGL();

				_UpdateComponents();
				_UpdateGL();

				_Render();
				glfwSwapBuffers(window_);

				is_running &= (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
				is_running &= (glfwWindowShouldClose(window_) != GL_TRUE);
			} while (is_running);

			_TerminateComponents();
			_DeleteProgram();
			_TerminateGL();

			glfwDestroyWindow(window_);
			glfwTerminate();
		}
		void RunBenchmark(ParticleRenderer* the_pr) {
			pr = the_pr;
			BOOL is_running = TRUE;
			DOUBLE time;
			_LoadOption();
			_InitComponents();
			_InitGL();

			_StartupComponents();
			_LoadProgram();
			_StartupGL();

			time = glfwGetTime();
			for (UINT i = 0; i < 200 && is_running; i++)
			{
				glfwPollEvents();

				_LoadControl();
				_ControlComponents();
				_ControlGL();

				_UpdateComponents();
				_UpdateGL();

				_Render();
				glfwSwapBuffers(window_);

				is_running &= (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
				is_running &= (glfwWindowShouldClose(window_) != GL_TRUE);
			}
			time = glfwGetTime() - time;
			printf_s("ET = %f\n", time);

			_TerminateComponents();
			_DeleteProgram();
			_TerminateGL();

			glfwDestroyWindow(window_);
			glfwTerminate();
		}
		void RunDebug(ParticleRenderer* the_pr) {
			pr = the_pr;
			BOOL is_running = TRUE;
			wait_ = TRUE;
			_LoadOption();
			_InitComponents();
			_InitGL();

			_StartupComponents();
			_LoadProgram();
			_StartupGL();
			do {
				glfwPollEvents();

				_LoadControl();
				_ControlComponents();
				_ControlGL();
				if (wait_ == FALSE) {

					_UpdateComponents();
					_UpdateGL();
				}

				_Render();
				glfwSwapBuffers(window_);

				is_running &= (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
				is_running &= (glfwWindowShouldClose(window_) != GL_TRUE);


			} while (is_running);

			_TerminateComponents();
			_DeleteProgram();
			_TerminateGL();

			glfwDestroyWindow(window_);
			glfwTerminate();
		}

	private:
		void _InitComponents() {
			o_vps_.Init(&opt_vps);
			o_camera_.Init();
			o_icoshedron_.Init(opt_vps.particles.diameter);
		}
		void _InitGL();

		void _StartupComponents() {
			o_vps_.Startup();
			o_camera_.Startup();
		}
		void _LoadProgram();
		void _StartupGL();

		void _ControlComponents() {
			o_vps_.Control(&ctrl_vps);
			o_camera_.Control();
		}
		void _ControlGL();

		void _UpdateComponents() {
			o_vps_.Update();
			o_camera_.Update();
		}
		void _UpdateGL();

		void _Render();

		void _TerminateComponents() {
			o_vps_.Terminate();
			o_camera_.Terminate();
		}
		void _DeleteProgram();
		void _TerminateGL();

		//Callback::
		static void _onError(int errorCode, const char* errorDescription) {
			//printf_s("ERROR : ");
			//printf_s(errorDescription);
			//printf_s("/n");
			fprintf_s(stderr, "ERROR : ");
			fprintf_s(stderr, errorDescription);
			fprintf_s(stderr, "\n");
		}
		static void _onResize(GLFWwindow* window, int w, int h) {}
		static void _onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {

			if (action == GLFW_PRESS)
			{
				switch (key)
				{
				case GLFW_KEY_SPACE:
					pr->wait_ = !pr->wait_;
					break;
				}
			}


		}
		static void _onMouseButton(GLFWwindow* window, int button, int action, int mods) {}
		static void _onMouseMove(GLFWwindow* window, double x, double y) {}
		static void _onMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {}

		//
		void _LoadOption() {
			//::CM,G
#ifdef _DEBUG
			opt_vps.num_threads = 12;
			opt_vps.num_works = 12;
#else
			opt_vps.num_threads = 12;
			opt_vps.num_works = 12;
#endif
			opt_vps.iteration = 1;
			opt_vps.tick = 1.0f / 120.0f;
			opt_vps.limit = 210.0f;

			opt_vps.voxel.unit = 5.7735027f;
			opt_vps.voxel.scale_x = 150;
			opt_vps.voxel.scale_y = 150;
			opt_vps.voxel.scale_z = 150;
			opt_vps.voxel.pos_center = { 0.0f,0.0f,0.0f };
			opt_vps.voxel.num_elements = 1;

			opt_vps.voxel.density.intr_distance = 5.0f;
			opt_vps.voxel.density.intr_offset = opt_vps.voxel.unit * 1.49f - opt_vps.voxel.density.intr_distance;

			opt_vps.particles.interval = .6f;
			opt_vps.particles.diameter = 10.f;
			opt_vps.particles.max_intr_distance = 2.0f * opt_vps.voxel.unit;
			//opt_vps.particles.max_intr_distance = opt_vps.particles.diameter;

			opt_vps.particles.mass = 1.0;
#ifdef _DEBUG
			opt_vps.particles.num = 20 * 20 * 20;
#else
			opt_vps.particles.num = 64 * 64 * 64;
#endif
			opt_vps.particles.num_neighbors = 0;
		}

		void _LoadControl() {
			//ctrl_vps.g_gravity_ = { 0.0f,0.0f,-9.8f };
			ctrl_vps.g_gravity = { 0.0f,0.0f,-.0f };
			ctrl_vps.g_reduction = 0.90f;
			ctrl_vps.particle_coef.spring = 16.0f;
			ctrl_vps.particle_coef.damping = .02f;
			ctrl_vps.particle_coef.shear = 0.1f;
			ctrl_vps.particle_coef.drag = 0.0005f;
			//ctrl_vps.particle_coef.attraction = 0.01f;
			ctrl_vps.particle_coef.attraction = 2.f;
		}

		void _SetVsync(BOOL enable) {
			app_info_.flags.vsync = enable ? 1 : 0;
			glfwSwapInterval((int)app_info_.flags.vsync);
		}

		void getMousePosition(int& x, int& y) {
			double dx, dy;
			glfwGetCursorPos(window_, &dx, &dy);

			x = static_cast<int>(floor(dx));
			y = static_cast<int>(floor(dy));
		}

		GLuint shader_load(
			const char* filename,
			GLenum shader_type = GL_FRAGMENT_SHADER,
#ifdef _DEBUG
			bool check_errors = true)
#else
			bool check_errors = false)
#endif
		{
			GLuint result = 0;
			FILE* fp;
			size_t filesize;
			char* data;

			fp = fopen(filename, "rb");

			if (!fp)
				return 0;

			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			data = new char[filesize + 1];

			if (!data)
				goto fail_data_alloc;

			fread(data, 1, filesize, fp);
			data[filesize] = 0;
			fclose(fp);

			result = glCreateShader(shader_type);

			if (!result)
				goto fail_shader_alloc;

			glShaderSource(result, 1, &data, NULL);

			delete[] data;

			glCompileShader(result);

			if (check_errors) {
				GLint status = 0;
				glGetShaderiv(result, GL_COMPILE_STATUS, &status);

				if (!status) {
					char buffer[4096];
					glGetShaderInfoLog(result, 4096, NULL, buffer);
					printf_s(filename);
					printf_s(":");
					printf_s(buffer);
					printf_s("\n");
					goto fail_compile_shader;
				}
			}
			return result;

		fail_compile_shader:
			glDeleteShader(result);

		fail_shader_alloc:;
		fail_data_alloc:
			return result;
		}

		GLuint from_string(
			const char* source,
			GLenum shader_type,
#ifdef _DEBUG
			bool check_errors = true)
#else
			bool check_errors = false)
#endif
		{
			GLuint sh;

			sh = glCreateShader(shader_type);

			const char* strings[] = { source };
			glShaderSource(sh, 1, strings, nullptr);

			glCompileShader(sh);

			if (check_errors) {
				GLint status = 0;
				glGetShaderiv(sh, GL_COMPILE_STATUS, &status);

				if (!status) {
					char buffer[4096];
					glGetShaderInfoLog(sh, 4096, NULL, buffer);
					printf_s(buffer);
					printf_s("\n");
					goto fail_compile_shader;
				}
			}
			return sh;

		fail_compile_shader:
			glDeleteShader(sh);

			return 0;
		}

		GLuint link_from_shaders(
			const GLuint* shaders,
			int shader_count,
			bool delete_shaders,
#ifdef _DEBUG
			bool check_errors = true)
#else
			bool check_errors = false)
#endif
		{
			int i;
			GLuint program;

			program = glCreateProgram();

			for (i = 0; i < shader_count; i++) {
				glAttachShader(program, shaders[i]);
			}

			glLinkProgram(program);

			if (check_errors) {
				GLint status;
				glGetProgramiv(program, GL_LINK_STATUS, &status);

				if (!status) {
					char buffer[4096];
					glGetProgramInfoLog(program, 4096, NULL, buffer);
					printf_s(buffer);
					printf_s("\n");
					glDeleteProgram(program);
					return 0;
				}
			}

			if (delete_shaders) {
				for (i = 0; i < shader_count; i++) {
					glDeleteShader(shaders[i]);
				}
			}

			return program;
		}

	};
}


