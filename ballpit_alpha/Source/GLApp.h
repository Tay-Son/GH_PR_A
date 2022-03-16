#pragma once

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "vmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>
#include <thread>

#include <Windows.h>

//OpenGL App Framework 클래스
//모든 앱의 필수기능들이 정의되어 있음
class GLApp
{
private:
public:
	GLApp() {}
	virtual ~GLApp() {}
	virtual void run(GLApp* the_app)
	{
		bool running = true;
		app = the_app;

		Init();
		InitWIndow();
		Startup();
		do
		{
			glfwPollEvents();
			Update();
			glfwSwapBuffers(window);
			Render();

			running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
			running &= (glfwWindowShouldClose(window) != GL_TRUE);

			SetWindowTitle(info.title);
		} while (running);
		Shutdown();

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	virtual void Init()
	{
		strcpy_s(info.title, "GLApp");
		info.windowWidth = 1024;
		info.windowHeight = 768;

		info.majorVersion = 4;
		info.minorVersion = 3;

		info.samples = 0;
		info.flags.all = 0;
		info.flags.cursor = 1;
	}
	void InitWIndow() {
		if (!glfwInit())
		{
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.minorVersion);
		if (info.flags.robust)
		{
			glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
		}
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, info.samples);
		glfwWindowHint(GLFW_STEREO, info.flags.stereo ? GL_TRUE : GL_FALSE);

		window = glfwCreateWindow(
			info.windowWidth,
			info.windowHeight,
			info.title,
			info.flags.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL
		);
		if (!window)
		{
			return;
		}

		glfwMakeContextCurrent(window);

		glfwSetWindowSizeCallback(window, glfw_onResize);
		glfwSetKeyCallback(window, glfw_onKey);
		glfwSetMouseButtonCallback(window, glfw_onMouseButton);
		glfwSetCursorPosCallback(window, glfw_onMouseMove);
		glfwSetScrollCallback(window, glfw_onMouseWheel);
		if (!info.flags.cursor)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		gl3wInit();

	}

	virtual void Startup(){}
	virtual void Render(){}
	virtual void Shutdown(){}
	virtual void Update(){}

	GLuint LoadShader(const GLchar* path, const GLenum type)
	{
		GLuint out = 0;
		std::ifstream ifstr(path);
		std::string text;
		const GLchar* srcptr;

		if (ifstr.is_open()) {
			GLint log_length;

			std::vector<GLchar> log;
			std::stringstream sstr;

			sstr << ifstr.rdbuf();
			text = sstr.str();
			ifstr.close();

			srcptr = text.c_str();

			out = glCreateShader(type);
			glShaderSource(out, 1, &srcptr, NULL);
			glCompileShader(out);

			glGetShaderiv(out, GL_INFO_LOG_LENGTH, &log_length);
			log.reserve(log_length);
			log.clear();
			glGetShaderInfoLog(out, log_length, NULL, (GLchar*)&log);
			std::cout << "DEBUG::compile shader : " <<path << std::endl;
			if (log_length > 0) {
				std::cout << (GLchar*)&log << std::endl;
			}
		}
		std::cout << std::endl;
		return out;
	}

	void SetWindowTitle(const char* title)
	{
		glfwSetWindowTitle(window, title);
	}
	virtual void onResize(int w, int h)
	{
		info.windowWidth = w;
		info.windowHeight = h;
	}
	virtual void onKey(int key, int action)
	{

	}
	virtual void onMouseButton(int button, int action)
	{

	}
	virtual void onMouseMove(int x, int y)
	{

	}
	virtual void onMouseWheel(int pos)
	{

	}
	void getMousePosition(int& x, int& y)
	{
		double dx, dy;
		glfwGetCursorPos(window, &dx, &dy);

		x = static_cast<int>(floor(dx));
		y = static_cast<int>(floor(dy));
	}

public:
	struct APPINFO
	{
		char title[128];
		int windowWidth;
		int windowHeight;
		int majorVersion;
		int minorVersion;
		int samples;
		union
		{
			struct
			{
				unsigned int    fullscreen : 1;
				unsigned int    vsync : 1;
				unsigned int    cursor : 1;
				unsigned int    stereo : 1;
				unsigned int    debug : 1;
				unsigned int    robust : 1;
			};
			unsigned int        all;
		} flags;
	};

protected:
	APPINFO     info;
	static      GLApp* app;
	GLFWwindow* window;

	static void glfw_onResize(GLFWwindow* window, int w, int h)
	{
		app->onResize(w, h);
	}
	static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		app->onKey(key, action);
	}
	static void glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods)
	{
		app->onMouseButton(button, action);
	}
	static void glfw_onMouseMove(GLFWwindow* window, double x, double y)
	{
		app->onMouseMove(static_cast<int>(x), static_cast<int>(y));
	}
	static void glfw_onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
	{
		app->onMouseWheel(static_cast<int>(yoffset));
	}
	void setVsync(bool enable)
	{
		info.flags.vsync = enable ? 1 : 0;
		glfwSwapInterval((int)info.flags.vsync);
	}
};

#define DECLARE_MAIN(a)                             \
sb7::application *app = 0;                          \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    a *app = new a;                                 \
    app->run(app);                                  \
    delete app;                                     \
    return 0;                                       \
}
