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
#include <stdint.h>
#include "utility/SGUtil.h"
#include "renderer/GLResource.hpp"

namespace SGRender
{
	enum class UniformType
	{
		INT, FLOAT, VEC2, VEC3, MAT4, TEXTURE
	};

	template<GLenum buffertype>
	class GLBuffer
	{
	public:
		GLBuffer() = default;

		void create(GLsizeiptr count)
		{
			GLuint id = 0;
			glGenBuffers(1, &id);
			glBindBuffer(buffertype, id);
			m_Resource = GLResource::GLResource(1, id);
		}

		void resize(GLsizeiptr size)
		{
			//If the buffer isn't used yet, just buffer data
			if (!m_Size)
			{
				glBufferData(buffertype, size, nullptr, GL_STATIC_DRAW);
				m_Size = size;
				return;
			}

			//If the buffer is already in use, generate a new buffer and copy
			GLResource prev = m_Resource; //Copy ptr to preserve
			unbind();

			//Create new pointer
			GLuint newID = 0;
			glGenBuffers(1, &newID);
			m_Resource = GLResource::GLResource(1, newID);

			bind();
			glBufferData(buffertype, size, nullptr, GL_STATIC_DRAW);

			//Copy then delete old buffer
			glCopyBufferSubData(prev.id(), m_Resource.id(), 0, 0, size);
			m_Size = size;
		}

		void bufferData(const void* data, GLsizeiptr size)
		{
			if (size == m_Size)
			{
				glBufferSubData(buffertype, 0, size, data);
				return;
			}
			glBufferData(buffertype, size, data, GL_STATIC_DRAW);
			m_Size = size; //Changes size if different
		}

		void bufferData(const void* data, GLsizeiptr offset, GLsizeiptr size)
		{
			if (offset + size <= m_Size)
			{
				glBufferSubData(buffertype, offset, size, data);
			}
			else
			{
				resize(offset + size);
				glBufferSubData(buffertype, offset, size, data);
				m_Size = offset + size;
			}
		}

		void bind() const
		{
			glBindBuffer(buffertype, m_Resource.id());
		}

		void unbind() const
		{
			glBindBuffer(buffertype, 0);
		}

	protected:
		GLsizeiptr m_Size = 0;
		GLResource m_Resource;
	};

	class VertexBuffer : public GLBuffer<GL_ARRAY_BUFFER> {};
	class IndexBuffer : public GLBuffer<GL_ELEMENT_ARRAY_BUFFER> {};

	class SSBO : public GLBuffer<GL_SHADER_STORAGE_BUFFER>
	{
	public:
		void create(GLsizeiptr count);
		GLuint bindingPoint() const { return m_BindingPoint; }

	private:
		GLuint m_BindingPoint = 0;
	};

	class UniformBuffer : public GLBuffer<GL_UNIFORM_BUFFER>
	{
	public:
		void create();
		GLuint bindingPoint() const { return m_BindingPoint; }

	private:
		GLuint m_BindingPoint = 0;
	};

	//Credit OGLDev for ShadowMapFBO layout
	class ShadowMapFBO
	{
	public:
		ShadowMapFBO();

		~ShadowMapFBO();

		bool init(int width, int height);

		void bindForWriting();
		void bindForReading(uint32_t slot);
		void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

	private:
		GLuint m_fbo;
		GLuint m_shadowMap;
	};

	//Buffer layout abstraction layer
	struct VertexBufferElement
	{
		uint32_t count;
		uint32_t type;
		uint32_t normalized;
		bool instanced;

		static size_t getTypeSize(GLuint type) {
			switch (type) {
			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4; //can extend for more data types
			case GL_UNSIGNED_BYTE: return 1;
			default: return 0;
			}
		}

		static size_t getTypeSize(GLuint type, size_t count) {
			return getTypeSize(type) * count;
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;
		~VertexBufferLayout() = default;

		template<typename T>
		void push(int count, bool instance) { //default
			static_assert(false, "Unknown type pushed to VBL");
		}

		template<>
		void push<float>(int count, bool instance) { //push float
			m_Elements.emplace_back(count, GL_FLOAT, GL_FALSE, instance );
			m_Stride += count * VertexBufferElement::getTypeSize(GL_FLOAT);
		}

		template<>
		void push<unsigned int>(int count, bool instance) {  //push uint
			m_Elements.emplace_back(count, GL_UNSIGNED_INT, GL_FALSE, instance );
			m_Stride += count * VertexBufferElement::getTypeSize(GL_UNSIGNED_INT);
		}

		template<>
		void push<unsigned char>(int count, bool instance) {  //push char
			m_Elements.emplace_back(count, GL_UNSIGNED_BYTE, GL_TRUE, instance);
			m_Stride += count * VertexBufferElement::getTypeSize(GL_UNSIGNED_BYTE);
		}

		inline const std::vector<VertexBufferElement> getElements() const& {
			return m_Elements;
		}

		inline unsigned int getStride() const { return m_Stride; }

	private:
		std::vector<VertexBufferElement> m_Elements;
		int32_t m_Stride = 0;
	};

	class VertexArray
	{
	public:
		VertexArray() = default;

		void create();
		void bind() const;
		void unbind() const;

		void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
		void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout, int start);

	private:
		VAResource m_Resource;
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

	/**
	* Classes that are sampled as texture 2D textures can derive this
	*/
	class TextureBase
	{
	public:

		/**
		* Generate texture in a given slot from an external buffer that is wrapped instead of clamped to edge
		*/
		void bind() const
		{
			glActiveTexture(GL_TEXTURE0 + m_Slot);

			if (!m_Resource.id())
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				return;
			}
			glBindTexture(GL_TEXTURE_2D, m_Resource.id());
		};

		void unbind() const
		{
			glActiveTexture(GL_TEXTURE0 + m_Slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		};

		int32_t& getSlot() { return m_Slot; };

	protected:
		int32_t m_Slot = 0;
		TXResource m_Resource;
	};

	class Shader
	{
	public:
		Shader() = default;

		void create(const std::string& vert, const std::string& frag);
		void create(const std::string& vert, const std::string& geo, const std::string& frag);

		void bind();
		void unbind();

		//set uniforms - overload for each data type
		void setUniform(const std::string& name, void* uniform, UniformType type);
		void setUniform(const std::string& name, int uniform);
		void setUniform(const std::string& name, float uniform);
		void setUniform(const std::string& name, const glm::vec2* uniform);
		void setUniform(const std::string& name, const glm::vec3* uniform);
		void setUniform(const std::string& name, const glm::mat4* uniform);

		//Set uniforms from direct location
		void setUniform(const GLint location, int uniform);
		void setUniform(const GLint location, float uniform);
		void setUniform(const GLint location, const glm::vec2* uniform);
		void setUniform(const GLint location, const glm::vec3* uniform);
		void setUniform(const GLint location, const glm::mat4* uniform);

		template<typename T>
		void setUniform(const T id, void* uniform, UniformType type)
		{
			static_assert(std::is_same<T, GLint>::value || std::is_same<T, std::string>::value, "Incorrect uniform type passed!");

			switch (type)
			{
			case UniformType::INT:
				setUniform(id, *(int*)uniform);
				break;
			case UniformType::FLOAT:
				setUniform(id, *(float*)uniform);
				break;
			case UniformType::MAT4:
				setUniform(id, (glm::mat4*)uniform);
				break;
			case UniformType::VEC2:
				setUniform(id, (glm::vec2*)uniform);
				break;
			case UniformType::VEC3:
				setUniform(id, (glm::vec3*)uniform);
				break;
			case UniformType::TEXTURE:
				((TextureBase*)uniform)->bind();
				setUniform(id, ((TextureBase*)uniform)->getSlot());
				break;
			default:
				break;
			}
		}

		void bindToUniformBlock(const std::string& name, GLuint binding);

		GLint getUniformLocation(const std::string& name);

	private:
		SHResource m_Resource;
		bool m_Bound = false;
		std::unordered_map<std::string, int> m_UniformLocationCache;
		GLuint compileShader(const std::string& source, unsigned int type);
		GLuint createShader(const std::string& vertexShader, const std::string& fragmentShader);
		GLuint createGeoShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);
		std::string parseShader(const std::string& filePath);
	};
}

#endif