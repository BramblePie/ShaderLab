#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>

// Show delta time in window title
#ifdef _DEBUG
#define DELTA_TITLE
#endif // _DEBUG

#include "Shader.h"

#include "Shapes.h"

constexpr int Scale = 1;
constexpr int Width = 1920 / Scale;
constexpr int Height = 1080 / Scale;

struct Agent
{
	glm::vec2 position = glm::vec2(Width / 2.f, Height / 2.f);
	//glm::vec2 direction;
	float angle;
	int id = 0;
};

struct Settings
{
	float speed;
	float turnSpeed;
	float sensorAngle;
	float sensorDistance;
	int sensorSize;
};

GLFWwindow* InitWindow(int width, int height);

bool processInput(GLFWwindow* window);

int main()
{
	//const glm::ivec2 res = { Width, Height };
	GLFWwindow* window = InitWindow(Width * Scale, Height * Scale);

	glClearColor(.0f, 0.0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Shader shader{ R"(src\shaders\shader.vert)", R"(src\shaders\shader.frag)" };
	//const GLuint count = 6;
	//auto vertices = Circle::GetShape<count, glm::vec2>(.5f);
	//std::array<glm::vec3, count> colors =
	//{
	//	glm::vec3{ 1.0f, 0.0f, 0.0f },
	//	glm::vec3{ 0.0f, 1.0f, 0.0f },
	//	glm::vec3{ 0.0f, 0.0f, 1.0f },
	//	glm::vec3{ 1.0f, 0.0f, 0.0f },
	//	glm::vec3{ 0.0f, 1.0f, 0.0f },
	//	glm::vec3{ 0.0f, 0.0f, 1.0f }
	//};
	//auto circle = Circle::InitializeShape(vertices, 0);
	//circle.AppendBuffer(colors, 1);

	const std::array quad = {
		glm::vec2{ -1.0f, -1.0f },
		glm::vec2{ -1.0f, 1.0f },
		glm::vec2{ 1.0f, -1.0f },
		glm::vec2{ 1.0f, 1.0f }
	};

	// Simple quad drawing shader
	Shader quadShader{ R"(src\shaders\quad.vert)", R"(src\shaders\drawTex.frag)" };
	quadShader.use();
	Circle::InitializeShape(quad, 0);
	glUniform1i(0, 0);	// Texture unit 0
	glUniform1i(1, 1);	// Texture unit 1

	//Shader postShader{ R"(src\shaders\quad.vert)", R"(src\shaders\fade.frag)" };
	//postShader.use();
	//Circle::InitializeShape(quad, 0);
	//glUniform1i(0, 0);	// Texture unit 0
	//glUniform1i(1, 1);	// Texture unit 1

	Shader fadeShader{ R"(src\shaders\fade.comp)" };
	fadeShader.use();
	glUniform1i(0, 0);	// Texture unit 0
	glUniform1i(1, 1);	// Texture unit 1
	glUniform1f(3, 0.8f);		// Fade strength
	glUniform1f(4, 8.0f);		// Diffuse strength

	Shader compute{ R"(src\shaders\LagueSlime.comp)" };
	compute.use();
	glUniform1i(0, 0);	// Texture unit 0

	Settings settings
	{
		40.0f,
		glm::pi<float>() * 2.0f,	// Turn speed: lower for understeer, higher srinking cells
		glm::pi<float>() / 6.0f,	// Sensor angle: wider more stable, thin chaotic and grows
		32.0f,						// Sensor distance: ?Thickness of strands
		3
	};
	for (size_t i = 0; i < 4; i++)
		glUniform1fv(3 + i, 1, (GLfloat*)&settings + i);
	glUniform1i(7, settings.sensorSize);
	const auto t = (glm::pi<float>() / 16.0f);
	// Initialize agents
	const size_t num_groups = 310;
	const size_t num_agents = num_groups * num_groups * 4 * 4;
	Agent* agents = new Agent[num_agents]{};
	for (size_t i = 0; i < num_agents; i++)
	{
		const auto rng = glm::diskRand(Height / 2.0f);
		//const auto pos = glm::vec2(glm::linearRand(0.0f, Width + 8.0f), glm::linearRand(0.0f, Height + 4.0f));
		agents[i].position += rng;
		//agents[i].angle = glm::pi<float>() * 0.5f;
		const auto n = glm::normalize(rng);
		agents[i].angle = glm::orientedAngle(glm::vec2(1.0f, 0.0f), n);
	}

	// Send agents to gpu
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * num_agents, &agents[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	// Create texture for the agents to write to
	GLuint mapTex[2];
	glGenTextures(2, mapTex);
	for (size_t i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mapTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Width, Height, 0, GL_RGBA, GL_FLOAT, 0);
		glBindImageTexture(i, mapTex[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	}

	const glm::uvec2 fadeGroups = { glm::ceil(Width / 4.0f), glm::ceil(Height / 4.0f) };

	GLuint map = 0, post = 1;
	GLuint seed = 0;
	double time = glfwGetTime();
	double prevTime = time;
	float delta = .0f;
	glUniform1f(2, delta);
	bool go = false;
	while (!glfwWindowShouldClose(window))
	{
		// Timing
		time = glfwGetTime();
		delta = static_cast<float>(time - prevTime);
		prevTime = time;

#pragma region Inputs

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			go = true;
		if (!go)
		{
			glClearColor(.0f, 0.0f, .0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwSwapBuffers(window);
			glfwPollEvents();
			continue;
		}

		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			settings.speed = 50.0f;
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			settings.speed += 2.0f;
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			settings.speed -= 2.0f;

		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			settings.sensorDistance = 40.0f;
		else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			settings.sensorDistance += 2.0f;
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			settings.sensorDistance -= 2.0f;

		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
			settings.turnSpeed = glm::two_pi<float>();
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			settings.turnSpeed += glm::quarter_pi<float>();
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			settings.turnSpeed -= glm::quarter_pi<float>();

		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
			settings.sensorAngle = glm::pi<float>() / 4.0f;
		else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
			settings.sensorAngle += (glm::pi<float>() / 32.0f);
		else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			settings.sensorAngle -= (glm::pi<float>() / 32.0f);

#pragma endregion

#ifdef DELTA_TITLE
		char title[20];
		std::snprintf(title, sizeof(title), "%f", delta);
		glfwSetWindowTitle(window, title);
#endif
		const double stop = 2.0;
		fadeShader.use();
		glUniform1i(0, post);	// Set map unit
		glUniform1i(1, map);	// Set post unit
		glUniform1f(2, delta);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glDispatchCompute(fadeGroups.x, fadeGroups.y, 1);

		// Process agents
		compute.use();
		glUniform1i(0, map);	// Set map unit
		glUniform1ui(1, ++seed);	// Keep changing randoms
		glUniform1f(2, delta);
		for (size_t i = 0; i < 4; i++)		// Set settings
			glUniform1fv(3 + i, 1, (GLfloat*)&settings + i);
		//glUniform1i(7, settings.sensorSize);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glDispatchCompute(num_groups, num_groups, 1);

		glClearColor(.0f, 0.0f, .0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw quad
		quadShader.use();
		glUniform1i(1, post);	// Set post unit
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// Swap texture unit uniforms
		const GLuint tmp = map;
		map = post;
		post = tmp;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &ssbo);
	glDeleteTextures(2, mapTex);
	delete[] agents;

	glfwTerminate();
	//system("pause");
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

	glfwWindowHint(GLFW_SAMPLES, 4);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "ShaderLab", 0, 0);
	if (!window)
	{
		glfwTerminate();
		return 0;
	}

	auto monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	//glfwMaximizeWindow(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
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