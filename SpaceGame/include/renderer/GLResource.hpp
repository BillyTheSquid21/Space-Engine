#pragma once
#ifndef SG_GL_RESOURCE_HPP
#define SG_GL_RESOURCE_HPP

#include "GL/glew.h"
#include "stdint.h"

namespace SGRender
{
	/**
	* Class to wrap general opengl resource deallocation
	*/
	class GLResource
	{
	public:

		GLResource()
			: m_RefCount(nullptr), m_ID(0), m_Count(0) {}

		GLResource(GLuint count, GLuint id)
			: m_RefCount(nullptr), m_ID(id), m_Count(count)
		{
			if (id == 0)
			{
				return;
			}
			initRefCount();
		}

		~GLResource()
		{
			if (m_RefCount == nullptr)
			{
				return;
			}

			--(*m_RefCount);
			if (*m_RefCount == 0)
			{
				glDeleteBuffers(m_Count, &m_ID);
				m_ID = 0;
				m_Count = 0;
				delete m_RefCount;
				m_RefCount = nullptr;
			}
		}

		GLResource(const GLResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
		}

		GLResource& operator=(const GLResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
			return *this;
		}

		inline GLuint id() const { return m_ID; }

	private:

		void initRefCount()
		{
			if (m_RefCount != nullptr)
			{
				return;
			}

			m_RefCount = new int32_t;
			*m_RefCount = 1;
		}

	protected:
		GLuint m_ID = 0;
		GLuint m_Count = 0;
		int32_t* m_RefCount = nullptr;
	};

	/**
	* Class to wrap vertex array deallocation
	*/
	class VAResource
	{
	public:

		VAResource()
			: m_RefCount(nullptr), m_ID(0), m_Count(0) {}

		VAResource(GLuint count, GLuint id)
			: m_RefCount(nullptr), m_ID(id), m_Count(count)
		{
			if (id == 0)
			{
				return;
			}
			initRefCount();
		}

		~VAResource()
		{
			if (m_RefCount == nullptr)
			{
				return;
			}

			--(*m_RefCount);
			if (*m_RefCount == 0)
			{
				glDeleteVertexArrays(m_Count, &m_ID);
				m_ID = 0;
				m_Count = 0;
				delete m_RefCount;
				m_RefCount = nullptr;
			}
		}

		VAResource(const VAResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
		}

		VAResource& operator=(const VAResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
			return *this;
		}

		inline GLuint id() const { return m_ID; }

	private:

		void initRefCount()
		{
			if (m_RefCount != nullptr)
			{
				return;
			}

			m_RefCount = new int32_t;
			*m_RefCount = 1;
		}

	protected:
		GLuint m_ID = 0;
		GLuint m_Count = 0;
		int32_t* m_RefCount = nullptr;
	};

	/**
	* Class to wrap shader deallocation
	*/
	class SHResource
	{
	public:

		SHResource()
			: m_RefCount(nullptr), m_ID(0) {}

		SHResource(GLuint count, GLuint id)
			: m_RefCount(nullptr), m_ID(id)
		{
			if (id == 0)
			{
				return;
			}
			initRefCount();
		}

		~SHResource()
		{
			if (m_RefCount == nullptr)
			{
				return;
			}

			--(*m_RefCount);
			if (*m_RefCount == 0)
			{
				glDeleteProgram(m_ID);
				m_ID = 0;
				delete m_RefCount;
				m_RefCount = nullptr;
			}
		}

		SHResource(const SHResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
		}

		SHResource& operator=(const SHResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
			return *this;
		}

		inline GLuint id() const { return m_ID; }

	private:

		void initRefCount()
		{
			if (m_RefCount != nullptr)
			{
				return;
			}

			m_RefCount = new int32_t;
			*m_RefCount = 1;
		}

	protected:
		GLuint m_ID = 0;
		int32_t* m_RefCount = nullptr;
	};

	/**
	* Class to wrap texture deallocation
	*/
	class TXResource
	{
	public:

		TXResource()
			: m_RefCount(nullptr), m_ID(0), m_Count(0) {}

		TXResource(GLuint count, GLuint id)
			: m_RefCount(nullptr), m_ID(id), m_Count(count)
		{
			if (id == 0)
			{
				return;
			}
			initRefCount();
		}

		~TXResource()
		{
			if (m_RefCount == nullptr)
			{
				return;
			}

			--(*m_RefCount);
			if (*m_RefCount == 0)
			{
				glDeleteTextures(m_Count, &m_ID);
				m_ID = 0;
				m_Count = 0;
				delete m_RefCount;
				m_RefCount = nullptr;
			}
		}

		TXResource(const TXResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
		}

		TXResource& operator=(const TXResource& copy)
		{
			m_ID = copy.m_ID;
			m_RefCount = copy.m_RefCount;
			m_Count = copy.m_Count;
			if (m_RefCount != nullptr)
			{
				++(*m_RefCount);
			}
			return *this;
		}

		inline GLuint id() const { return m_ID; }

	private:

		void initRefCount()
		{
			if (m_RefCount != nullptr)
			{
				return;
			}

			m_RefCount = new int32_t;
			*m_RefCount = 1;
		}

	protected:
		GLuint m_ID = 0;
		GLuint m_Count = 0;
		int32_t* m_RefCount = nullptr;
	};
}

#endif