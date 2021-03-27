#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

// Show delta time in window title
#define DELTA_TITLE

#include "Shader.h"

#include "Shapes.h"

/*
	uint hash(uint state)
	{
		state ^= 2747636419u;
		state *= 2654435769u;
		state ^= state >> 16;
		state *= 2654435769u;
		state ^= state >> 16;
		state *= 2654435769u;
		return state;
	}
*/

GLFWwindow* InitWindow();

int main()
{
	GLFWwindow* window = InitWindow();

	Shader shader{ R"(src\shaders\shader.vert)", R"(src\shaders\shader.frag)" };
	const GLuint count = 6;
	auto vertices = Circle::GetShape<count, glm::vec2>(.5f);

	std::array<glm::vec3, count> colors =
	{
		glm::vec3{ 1.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f },
		glm::vec3{ 1.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f }
	};

	const std::array quad = {
		glm::vec2{ -1.0f, -1.0f },
		glm::vec2{ -1.0f, 1.0f },
		glm::vec2{ 1.0f, -1.0f },
		glm::vec2{ 1.0f, 1.0f }
	};

	auto circle = Circle::InitializeShape(vertices, 0);
	circle.AppendBuffer(colors, 1);

	Shader example{ R"(src\shaders\example.vert)", R"(src\shaders\example.frag)" };
	example.use();
	Circle::InitializeShape(quad, 0);
	glUniform1i(0, 0);

	Shader compute{ R"(src\shaders\shader.comp)" };
	compute.use();
	GLuint texHandle, texUnit = 0;
	glGenTextures(1, &texHandle);

	glBindTexture(GL_TEXTURE_2D, texHandle);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	const GLsizei res = 512;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, res, res, 0, GL_RGBA, GL_FLOAT, NULL);

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	glBindImageTexture(texUnit, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Set uniform destTex
	glUniform1i(0, texUnit);

	shader.use();

	double time = glfwGetTime();
	double prevTime = time;
	float delta = .0f;
	while (!glfwWindowShouldClose(window))
	{
		// Timing
		time = glfwGetTime();
		delta = static_cast<float>(time - prevTime);
		prevTime = time;
#ifdef DELTA_TITLE
		char title[20];
		std::snprintf(title, sizeof(title), "%f", delta);
		glfwSetWindowTitle(window, title);
#endif

		compute.use();
		// Set uniform roll
		glUniform1f(1, .01f);
		glDispatchCompute(res / 32, res / 32, 1);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		example.use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//shader.use();
		//Circle::DrawShape(circle);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	//system("pause");
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
							GLsizei length, const GLchar* message, const void* userParam);

GLFWwindow* InitWindow()
{
	if (!glfwInit())
		return 0;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(640, 480, "ShaderLab", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GLenum init = glewInit();
	if (GLEW_OK != init)
	{
		printf("Error: %s\n", glewGetErrorString(init));
	}

	glViewport(0, 0, 640, 480);

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