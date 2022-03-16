#pragma once

#include "ParticleRenderer.h"


namespace GS {
	//::
	void ParticleRenderer::_InitGL() {
		//CODE::
		if (!glfwInit())
		{
			fprintf(stderr, "Failed to initialize GLFW\n");
			return;
		}

		glfwSetErrorCallback(_onError);

		strcpy_s(app_info_.title, "Ballpit Beta");
		app_info_.windowWidth = 800;
		app_info_.windowHeight = 800;
		app_info_.majorVersion = 4;
		app_info_.minorVersion = 3;
		app_info_.samples = 0;
		app_info_.flags.all = 0;
		app_info_.flags.cursor = 1;
		app_info_.flags.vsync = 1;
#ifdef _DEBUG
		app_info_.flags.debug = 1;
#endif
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, app_info_.majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, app_info_.minorVersion);

#ifndef _DEBUG
		if (app_info_.flags.debug)
#endif /* _DEBUG */
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		}
		if (app_info_.flags.robust)
		{
			glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
		}
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, app_info_.samples);
		glfwWindowHint(GLFW_STEREO, app_info_.flags.stereo ? GL_TRUE : GL_FALSE);

		window_ = glfwCreateWindow(
			app_info_.windowWidth,
			app_info_.windowHeight,
			app_info_.title,
			app_info_.flags.fullscreen ? glfwGetPrimaryMonitor() : NULL,
			NULL);
		if (!window_) {
			fprintf(stderr, "Failed to open window\n");
			return;
		}
		glfwSwapInterval((int)app_info_.flags.vsync);

		glfwMakeContextCurrent(window_);

		//
		glfwSetWindowSizeCallback(window_, _onResize);
		glfwSetKeyCallback(window_, _onKey);
		glfwSetMouseButtonCallback(window_, _onMouseButton);
		glfwSetCursorPosCallback(window_, _onMouseMove);
		glfwSetScrollCallback(window_, _onMouseWheel);
		if (!app_info_.flags.cursor) {
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		glewInit();

		//

#ifdef _DEBUG
		fprintf_s(stderr, "VENDOR: %s\n", (char*)glGetString(GL_VENDOR));
		fprintf_s(stderr, "VERSION: %s\n", (char*)glGetString(GL_VERSION));
		fprintf_s(stderr, "RENDERER: %s\n", (char*)glGetString(GL_RENDERER));
#endif
	}
	//::
	void ParticleRenderer::_LoadProgram() {
		//::
		GLuint shaders[3];

		//::
		shaders[0] = shader_load("shaders/render_indivisual.vs.glsl", GL_VERTEX_SHADER);
		shaders[1] = shader_load("shaders/render_indivisual.gs.glsl", GL_GEOMETRY_SHADER);
		shaders[2] = shader_load("shaders/render_indivisual.fs.glsl", GL_FRAGMENT_SHADER);

		if (programs_.render_indvisual)
			glDeleteProgram(programs_.render_indvisual);

		programs_.render_indvisual = link_from_shaders(shaders, 3, true);

		uniform_loc_.dnm.diffuse_albedo = glGetUniformLocation(programs_.render_indvisual, "diffuse_albedo");
		uniform_loc_.dnm.specular_albedo = glGetUniformLocation(programs_.render_indvisual, "specular_albedo");
		uniform_loc_.dnm.specular_power = glGetUniformLocation(programs_.render_indvisual, "specular_power");
		uniform_loc_.dnm.ambient = glGetUniformLocation(programs_.render_indvisual, "ambient");

		uniform_loc_.stt.icos_vertices = glGetUniformLocation(programs_.render_indvisual, "icos_vertices");
		uniform_loc_.stt.icos_indices = glGetUniformLocation(programs_.render_indvisual, "icos_indices");
	}
	void ParticleRenderer::_StartupGL() {
		//:: 버텍스 버퍼
		glGenVertexArrays(1, &vao_);
		glBindVertexArray(vao_);

		glGenBuffers(1, &buffers_.vertex);
		glBindBuffer(GL_ARRAY_BUFFER, buffers_.vertex);
		glBufferData(GL_ARRAY_BUFFER, o_vps_.get_particles_pos_buffer_size(), NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		//:: 유니폼 버퍼
		glGenBuffers(1, &buffers_.uniform);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers_.uniform);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);
		//glBufferData(GL_UNIFORM_BUFFER, 1024, NULL, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &buffers_.icos_indices);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers_.icos_indices);
		glBufferData(GL_UNIFORM_BUFFER, 12 * 12, NULL, GL_STATIC_DRAW);
		glGenBuffers(1, &buffers_.icos_vertices);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers_.icos_vertices);
		glBufferData(GL_UNIFORM_BUFFER, 4 * 60, NULL, GL_STATIC_DRAW);

		//::
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glUseProgram(programs_.render_indvisual);
		glUniform3fv(uniform_loc_.stt.icos_vertices, 12, o_icoshedron_.vertices);
		glUniform1iv(uniform_loc_.stt.icos_indices, 60, o_icoshedron_.indices);

	}
	//::
	void ParticleRenderer::_ControlGL() {}
	//::
	void ParticleRenderer::_UpdateGL() {}
	//::
	void ParticleRenderer::_Render() {
		static const GLfloat ones[] = { 1.0f };

		glViewport(0, 0, app_info_.windowWidth, app_info_.windowHeight);
		glClearBufferfv(GL_COLOR, 0, colors_.black);
		glClearBufferfv(GL_DEPTH, 0, ones);



		glUseProgram(programs_.render_indvisual);

		glUniform1f(uniform_loc_.dnm.specular_power, powf(2.0f, 2.0f));
		glUniform3fv(uniform_loc_.dnm.ambient, 1, vmath::vec3(0.3f, 0.3f, 0.3f));
		glUniform3fv(uniform_loc_.dnm.specular_albedo, 1, vmath::vec3(1.0f / 9.0f + 1.0f / 9.0f));
		glUniform3fv(uniform_loc_.dnm.diffuse_albedo, 1, vmath::vec3(0.2f, 0.2f, 0.2f));

		//::
		vmath::vec3 view_position = vmath::vec3(300.f, 400.f, 0.f);
		vmath::mat4 view_matrix = vmath::lookat(view_position, vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 0.0f, 1.0f));
		vmath::vec3 light_position = vmath::vec3(200.0f, 200.0f, 200.0f);
		vmath::mat4 light_proj_matrix = vmath::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 200.0f);
		vmath::mat4 light_view_matrix = vmath::lookat(light_position,
			vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));


		//::

		//::








		glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffers_.uniform);
		uniforms_block* block =
			(uniforms_block*)glMapBufferRange(
				GL_UNIFORM_BUFFER,
				0,
				sizeof(uniforms_block),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		vmath::mat4 model_matrix = vmath::scale(1.0f);


		//block->mv_matrix = view_matrix * model_matrix;
		block->mv_matrix = vmath::translate(0.0f, 0.0f, -8.0f) *
			vmath::rotate((float)glfwGetTime() * 71.0f, 0.0f, 1.0f, 0.0f) *
			vmath::rotate((float)glfwGetTime() * 10.0f, 1.0f, 0.0f, 0.0f);

		block->mv_matrix = model_matrix * view_matrix;
		block->view_matrix = view_matrix;
		block->proj_matrix = vmath::perspective(
			50.0f,
			(float)app_info_.windowWidth / (float)app_info_.windowHeight,
			0.1f,
			1000.0f);

		glUnmapBuffer(GL_UNIFORM_BUFFER);



		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			o_vps_.get_particles_pos_buffer_size(),
			o_vps_.get_particles_pos());

		glDrawArrays(GL_POINTS, 0, o_vps_.get_particles_num());
	}
	//::
	void ParticleRenderer::_DeleteProgram() {
		glDeleteProgram(programs_.render_indvisual);
		programs_.render_indvisual = 0;
	}
	void ParticleRenderer::_TerminateGL() {
		//::
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;

		glDeleteBuffers(1, &buffers_.vertex);
		buffers_.vertex = 0;

		//::
		glDeleteBuffers(1, &buffers_.uniform);
		buffers_.uniform = 0;
	}

}