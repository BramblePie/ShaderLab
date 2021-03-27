#pragma once

#include <array>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

struct Circle
{
	template<unsigned Count, class VecType>
	static inline std::array<VecType, Count> GetShape(const float radius)
	{
		std::array<VecType, Count> vertices;

		constexpr float theta = glm::two_pi<float>() / Count;
		for (size_t i = 0; i < Count; i++)
			vertices[i] = glm::vec4(radius * glm::sin(i * theta), radius * glm::cos(i * theta), .0f, .0f);

		return vertices;
	}

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

	static inline void DrawShape(const VertexArray& VAO)
	{
		glBindVertexArray(VAO.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, VAO.buffer_size);
	}
};