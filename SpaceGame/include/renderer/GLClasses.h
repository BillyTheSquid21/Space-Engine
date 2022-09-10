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

#include "utility/SGUtil.h"

namespace SGRender
{
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		~VertexBuffer();

		void create(size_t dataSize);
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

	//Credit OGLDev for ShadowMapFBO layout
	class ShadowMapFBO
	{
	public:
		ShadowMapFBO();

		~ShadowMapFBO();

		bool init(unsigned int width, unsigned int height);

		void bindForWriting();
		void bindForReading(unsigned int slot);
		void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

	private:
		unsigned int m_fbo;
		unsigned int m_shadowMap;
	};

	//Buffer layout abstraction layer
	struct VertexBufferElement
	{
		unsigned int count;
		unsigned int type;
		unsigned int normalized;
		bool instanced;

		static unsigned int getTypeSize(unsigned int type) {
			switch (type) {
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4; //can extend for more data types
			case GL_UNSIGNED_BYTE: return 1;
			default: return 0;
			}
		}

		static unsigned int getTypeSize(unsigned int type, size_t count) {
			return getTypeSize(type) * count;
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;
		~VertexBufferLayout() = default;

		template<typename T>
		void push(unsigned int count, bool instance) { //default
			static_assert(false, "Unknown type pushed to VBL");
		}

		template<>
		void push<float>(unsigned int count, bool instance) { //push float
			m_Elements.push_back({ count, GL_FLOAT, GL_FALSE, instance });
			m_Stride += count * VertexBufferElement::getTypeSize(GL_FLOAT);
		}

		template<>
		void push<unsigned int>(unsigned int count, bool instance) {  //push uint
			m_Elements.push_back({ count, GL_UNSIGNED_INT, GL_FALSE, instance });
			m_Stride += count * VertexBufferElement::getTypeSize(GL_UNSIGNED_INT);
		}

		template<>
		void push<unsigned char>(unsigned int count, bool instance) {  //push char
			m_Elements.push_back({ count, GL_UNSIGNED_BYTE, GL_TRUE, instance });
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
		void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout, int start);

	private:
		unsigned int m_ID;
	};

	struct ShaderProgramSource {
		std::string VertexSource;
		std::string FragmentSource;
	};

	struct GeoShaderProgramSource {
		std::string VertexSource;
		std::string GeometrySource;
		std::string FragmentSource;
	};

	class Shader
	{
	public:
		Shader() = default;
		~Shader();

		void create(const std::string& vert, const std::string& frag);
		void create(const std::string& vert, const std::string& geo, const std::string& frag);

		void bind() const;
		void unbind() const;
		void deleteShader() { glDeleteProgram(m_ID); }

		//set uniforms - overload for each data type
		void setUniform(const std::string& name, int uniform);
		void setUniform(const std::string& name, float uniform);
		void setUniform(const std::string& name, const glm::vec3* uniform);
		void setUniform(const std::string& name, const glm::mat4* uniform);

	private:
		unsigned int m_ID = 0;
		std::unordered_map<std::string, int> m_UniformLocationCache;
		unsigned int compileShader(const std::string& source, unsigned int type);
		unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);
		unsigned int createGeoShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);
		std::string parseShader(const std::string& filePath);
		int getUniformLocation(const std::string& name);
	};
}

#endif