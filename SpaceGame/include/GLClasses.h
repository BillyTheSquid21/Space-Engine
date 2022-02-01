#pragma once
#ifndef GL_CLASSES_H
#define GL_CLASSES_H

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SGUtil.h"

class VertexBuffer
{
public:
	VertexBuffer() = default;
	~VertexBuffer();

	void create(unsigned int count);
	void bufferData(const void* data, unsigned int count);
	void bind() const;
	void unbind() const;

private:
	unsigned int m_ID = 0;
};

class IndexBuffer
{
public:
	IndexBuffer() = default;
	~IndexBuffer();

	void create(unsigned int count);
	void bufferData(const void* data, unsigned int count);
	void bind() const;
	void unbind() const;

	//return count
	inline unsigned int GetCount() const { return m_IndicesCount; }

private:
	unsigned int m_ID = 0;
	unsigned int m_IndicesCount = 0;
};

//Buffer layout abstraction layer
struct VertexBufferElement
{
	unsigned int count;
	unsigned int type;
	unsigned int normalized;

	static unsigned int getTypeSize(unsigned int type) {
		switch (type) {
		case GL_FLOAT: return 4;
		case GL_UNSIGNED_INT: return 4; //can extend for more data types
		case GL_UNSIGNED_BYTE: return 1;
		default: return 0;
		}
	}
};

class VertexBufferLayout
{
public:
	VertexBufferLayout() = default;
	~VertexBufferLayout() = default;

	template<typename T>
	void push(unsigned int count) { //default
		static_assert(false, "Unknown type pushed to VBL");
	}

	template<>
	void push<float>(unsigned int count) { //push float
		m_Elements.push_back({ count, GL_FLOAT, GL_FALSE });
		m_Stride += count * VertexBufferElement::getTypeSize(GL_FLOAT);
	}

	template<>
	void push<unsigned int>(unsigned int count) {  //push uint
		m_Elements.push_back({ count, GL_UNSIGNED_INT, GL_FALSE });
		m_Stride += count * VertexBufferElement::getTypeSize(GL_UNSIGNED_INT);
	}

	template<>
	void push<unsigned char>(unsigned int count) {  //push uint
		m_Elements.push_back({ count, GL_UNSIGNED_BYTE, GL_TRUE });
		m_Stride += count * VertexBufferElement::getTypeSize(GL_UNSIGNED_BYTE);
	}

	inline const std::vector<VertexBufferElement> getElements() const& {
		return m_Elements;
	}

	inline unsigned int getStride() const { return m_Stride; }

private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride = 0;
};

class VertexArray
{
public:
	VertexArray() = default;
	~VertexArray();

	void create();
	void bind() const;
	void unbind() const;

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);


private:
	unsigned int m_ID;
};

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
public:
	Shader() = default;
	~Shader();

	void create(const std::string& filepath);

	void bind() const;
	void unbind() const;

	//set uniforms - overload for each data type
	void setUniform(const std::string& name, float uniform);
	void setUniform(const std::string& name, const glm::vec3* uniform);
	void setUniform(const std::string& name, const glm::mat4* uniform);

private:
	unsigned int m_ID = 0;
	std::unordered_map<std::string, unsigned int> m_UniformLocationCache;
	unsigned int compileShader(const std::string& source, unsigned int type);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);
	ShaderProgramSource parseShader(const std::string& filePath);
	unsigned int getUniformLocation(const std::string& name);
};

#endif