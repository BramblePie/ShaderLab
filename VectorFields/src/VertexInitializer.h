#pragma once

#include <array>
#include <vector>

#include <GL/glew.h>

struct VertexArray
{
	GLuint vao;
	std::vector<GLuint> vbos;
	// Number of vertices in a vertex buffer
	GLsizei buffer_size;

	template<class Type, unsigned Count>
	void AppendBuffer(const std::array<Type, Count>& vertices, const GLuint& location)
	{
		glBindVertexArray(vao);
		GLuint& vbo = vbos.emplace_back();
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(location, Type::length(), GL_FLOAT, GL_FALSE, sizeof(Type), 0);
		glEnableVertexAttribArray(location);
	}
};

template<unsigned Count, class VecType>
static inline VertexArray InitializeShape(const std::array<VecType, Count>& vertices, const GLuint& location)
{
	VertexArray VA{};
	VA.buffer_size = Count;

	glGenVertexArrays(1, &VA.vao);
	glBindVertexArray(VA.vao);

	GLuint& vbo = VA.vbos.emplace_back();
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(location, VecType::length(), GL_FLOAT, GL_FALSE, sizeof(VecType), 0);
	glEnableVertexAttribArray(location);

	return VA;
}