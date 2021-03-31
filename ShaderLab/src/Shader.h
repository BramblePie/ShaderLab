#pragma once

#include <fstream>
#include <sstream>

#include <GL/glew.h>

namespace details
{
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
}

enum Glob
{
	one = 1,
	two,
	three
};

class Shader
{
public:
	enum UniformLocation : GLint
	{
		View = 0
	};

	GLuint id;

	Shader(const char* computePath)
	{
		std::string computeCode = details::getShaderCode(computePath);
		GLuint compute = details::compileShader(computeCode.c_str(), GL_COMPUTE_SHADER);
		id = details::createShaderProgram(compute);
	}
	Shader(const char* vertexPath, const char* fragmentPath)
	{
		std::string vShaderCode = details::getShaderCode(vertexPath);
		std::string fShaderCode = details::getShaderCode(fragmentPath);

		GLuint vertex = details::compileShader(vShaderCode.c_str(), GL_VERTEX_SHADER);
		GLuint fragment = details::compileShader(fShaderCode.c_str(), GL_FRAGMENT_SHADER);

		id = details::createShaderProgram(vertex, fragment);
	}
	//Shader(const char* vertexPath, const char* geometreyPath, const char* fragmentPath);
	~Shader() { glDeleteProgram(id); }

	void use() const
	{
		glUseProgram(id);
	}

	template<class Type>
	void SetUniform(GLint location, Type value)
	{
		static_assert(true, "Uniform type not implemented");
	}

	template<>
	void SetUniform<GLint>(GLint location, GLint value)
	{
		glUniform1i(location, value);
	}

private:
};