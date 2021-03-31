//#include "Shader.h"
//
//#include <type_traits>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//
//#pragma region Helper function declarations
//
//std::string getShaderCode(const char* filePath)
//{
//	std::ifstream shaderFile;
//	std::string shaderCode;
//	shaderFile.exceptions(std::ifstream::badbit);
//	try
//	{
//		shaderFile.open(filePath);
//		std::stringstream shaderStream;
//
//		shaderStream << shaderFile.rdbuf();
//		shaderFile.close();
//
//		shaderCode = shaderStream.str();
//	}
//	catch (const std::ifstream::failure& e)
//	{
//		std::cout << "[ERROR] " << e.what() << "\n";
//	}
//	return shaderCode;
//}
//
//GLuint compileShader(const char* shaderCode, GLenum type)
//{
//	GLuint shader = glCreateShader(type);
//	glShaderSource(shader, 1, &shaderCode, 0);
//	glCompileShader(shader);
//	GLint success;
//	GLchar log[512];
//	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(shader, sizeof(log), 0, log);
//		std::cout << "[ERROR] Shader compilation failed\n" << log << "\n";
//	}
//	return shader;
//}
//
//template<class... GLuints>
//GLuint createShaderProgram(GLuints ...shaderIDs)
//{
//	static_assert((std::is_same<decltype(shaderIDs), GLuint>::value && ...), "Invalid shader id");
//
//	GLuint program = glCreateProgram();
//	(glAttachShader(program, shaderIDs), ...);
//	glLinkProgram(program);
//
//	GLint success;
//	GLchar log[512];
//	glGetProgramiv(program, GL_LINK_STATUS, &success);
//	if (!success)
//	{
//		glGetProgramInfoLog(program, sizeof(log), 0, log);
//		std::cout << "[ERROR] Shader program linking failed\n" << log << "\n";
//	}
//
//	(glDeleteShader(shaderIDs), ...);
//
//	return program;
//}
//
//#pragma endregion
//
//#pragma region Helper function definitions
//
//#pragma endregion