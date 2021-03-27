#pragma once

#include <GL/glew.h>

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

	Shader(const char* computePath);
	Shader(const char* vertexPath, const char* fragmentPath);
	//Shader(const char* vertexPath, const char* geometreyPath, const char* fragmentPath);
	~Shader() { glDeleteProgram(id); }

	void use() const;

	template<class Type>
	void SetUniform(GLint location, Type value)
	{
		static_assert(true, "Uniform type not implemented");
	}

	template<>
	void SetUniform<GLint>(GLint location, GLint value)
	{
		glUniform1i(View, value);
	}

private:
};