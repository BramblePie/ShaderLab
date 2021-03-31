#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Shader.h"

GLFWwindow* InitWindow(int width, int height);

int main()
{
	GLFWwindow* window = InitWindow(800, 600);

	double time = glfwGetTime();
	double prevTime = time;
	float delta = .0f;
	while (!glfwWindowShouldClose(window))
	{
		// Timing
		time = glfwGetTime();
		delta = static_cast<float>(time - prevTime);
		prevTime = time;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		glClearColor(.0f, 0.0f, .0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	system("pause");
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
							GLsizei length, const GLchar* message, const void* userParam);

GLFWwindow* InitWindow(int width, int height)
{
	if (!glfwInit())
		return 0;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_SAMPLES, 4);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Vector Fields", 0, 0);
	if (!window)
	{
		glfwTerminate();
		return 0;
	}

	//auto monitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	//glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	//glfwMaximizeWindow(window);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GLenum init = glewInit();
	if (GLEW_OK != init)
	{
		printf("Error: %s\n", glewGetErrorString(init));
	}

	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_MULTISAMPLE);

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("[INFO] Using OpenGL %i.%i\n", major, minor);

#ifdef _DEBUG

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		printf("[INFO] OpenGL debug context available\n");
	}
	else
		printf("[WARNING] No OpenGL debug context\n");

#endif // _DEBUG

	return window;
}

#pragma region Debugging callback

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	//if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "[GL_DEBUG] " << std::hex << "0x" << id << std::dec << " " << message << "\n\t";

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		std::cout << "Source: API, ";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		std::cout << "Source: Window System, ";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		std::cout << "Source: Shader Compiler, ";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		std::cout << "Source: Third Party, ";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		std::cout << "Source: Application, ";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		std::cout << "Source: Other, ";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "Type: Error, ";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "Type: Deprecated Behaviour, ";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "Type: Undefined Behaviour, ";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "Type: Portability, ";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "Type: Performance, ";
		break;
	case GL_DEBUG_TYPE_MARKER:
		std::cout << "Type: Marker, ";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		std::cout << "Type: Push Group, ";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		std::cout << "Type: Pop Group, ";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "Type: Other, ";
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "Severity: high\n";
		throw std::runtime_error(message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "Severity: medium\n";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "Severity: low\n";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "Severity: notification\n";
		break;
	}
}

#pragma endregion