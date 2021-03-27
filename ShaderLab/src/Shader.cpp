#include "Shader.h"

#include <type_traits>
#include <iostream>
#include <fstream>
#include <sstream>

#pragma region Helper function declarations

std::string getShaderCode(const char* filePath);

GLuint compileShader(const char* shaderCode, GLenum type);

template<class... GLuints>
GLuint createShaderProgram(GLuints ...components);

#pragma endregion

Shader::Shader(const char* computePath)
{
	std::string computeCode = getShaderCode(computePath);
	GLuint compute = compileShader(computeCode.c_str(), GL_COMPUTE_SHADER);
	id = createShaderProgram(compute);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vShaderCode = getShaderCode(vertexPath);
	std::string fShaderCode = getShaderCode(fragmentPath);

	GLuint vertex = compileShader(vShaderCode.c_str(), GL_VERTEX_SHADER);
	GLuint fragment = compileShader(fShaderCode.c_str(), GL_FRAGMENT_SHADER);

	id = createShaderProgram(vertex, fragment);
}

void Shader::use() const
{
	glUseProgram(id);
}

#pragma region Helper function definitions

std::string getShaderCode(const char* filePath)
{
	std::ifstream shaderFile;
	std::string shaderCode;
	shaderFile.exceptions(std::ifstream::badbit);
	try
	{
		shaderFile.open(filePath);
		std::stringstream shaderStream;

		shaderStream << shaderFile.rdbuf();
		shaderFile.close();

		shaderCode = shaderStream.str();
	}
	catch (const std::ifstream::failure& e)
	{
		std::cout << "[ERROR] " << e.what() << "\n";
	}
	return shaderCode;
}

GLuint compileShader(const char* shaderCode, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderCode, 0);
	glCompileShader(shader);
	GLint success;
	GLchar log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(log), 0, log);
		std::cout << "[ERROR] Shader compilation failed\n" << log << "\n";
	}
	return shader;
}

template<class... GLuints>
GLuint createShaderProgram(GLuints ...shaderIDs)
{
	static_assert((std::is_same<decltype(shaderIDs), GLuint>::value && ...), "Invalid shader id");

	GLuint program = glCreateProgram();
	(glAttachShader(program, shaderIDs), ...);
	glLinkProgram(program);

	GLint success;
	GLchar log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(log), 0, log);
		std::cout << "[ERROR] Shader program linking failed\n" << log << "\n";
	}

	(glDeleteShader(shaderIDs), ...);

	return program;
}

#pragma endregion