#include "renderer/GLClasses.h"

//Safe versions that checks address is set
//Lets destructors call without worrying was generated
void glDeleteBuffers_s(GLsizei n, const GLuint* buffers)
{
	if (!buffers)
	{
		return;
	}
	else if (!(*buffers) || !n)
	{
		return;
	}
	glDeleteBuffers(1, buffers);
}

void glDeleteShader_s(GLuint shader)
{
	if (!shader)
	{
		return;
	}
	glDeleteShader(shader);
}

void glDeleteProgram_s(GLuint program)
{
	if (!program)
	{
		return;
	}
	glDeleteProgram(program);
}

void SGRender::VertexBuffer::create(size_t dataSize) {
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	//tells how much data to set aside - size in bytes
	glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
}

SGRender::VertexBuffer::~VertexBuffer() {
	glDeleteBuffers_s(1, &m_ID);
}

void SGRender::VertexBuffer::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void SGRender::VertexBuffer::unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SGRender::VertexBuffer::bufferData(const void* data, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), data);
}

void SGRender::IndexBuffer::create(int count)
{
	m_IndicesCount = count;
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);
}

void SGRender::IndexBuffer::bufferData(const void* data, int count) {
	m_IndicesCount = count;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(unsigned int), data);
}

SGRender::IndexBuffer::~IndexBuffer() {
	glDeleteBuffers_s(1, &m_ID);
}

void SGRender::IndexBuffer::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void SGRender::IndexBuffer::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Uniform Buffer
void SGRender::UniformBuffer::create()
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ID);

	//Set binding point
	glBindBufferBase(GL_UNIFORM_BUFFER, GL_Binding_Point, m_ID);
	m_BindingPoint = GL_Binding_Point;
	GL_Binding_Point++;

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SGRender::UniformBuffer::bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
}

void SGRender::UniformBuffer::unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SGRender::UniformBuffer::reserveData(GLsizeiptr size)
{
	reserveData(size, GL_STATIC_DRAW);
}

void SGRender::UniformBuffer::reserveData(GLsizeiptr size, GLenum drawtype)
{
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, drawtype);
	m_ReservedSize = size;
}

void SGRender::UniformBuffer::bufferData(void* data, GLsizeiptr size)
{
	bufferData(data, 0, size);
}

void SGRender::UniformBuffer::bufferData(void* data, int offset, GLsizeiptr size)
{
	//If overruns, send warning
	if (offset + size > m_ReservedSize)
	{
		EngineLogFail("Buffering data beyond UBO end!");
		return;
	}

	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

SGRender::UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers_s(1, &m_ID);
}

//SSBO
void SGRender::SSBO::create()
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);

	//Set binding point
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GL_Binding_Point, m_ID);
	m_BindingPoint = GL_Binding_Point;
	GL_Binding_Point++;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SGRender::SSBO::reserveData(GLsizeiptr size)
{
	reserveData(size, GL_STATIC_DRAW);
}

void SGRender::SSBO::reserveData(GLsizeiptr size, GLenum drawtype)
{
	//If the buffer isn't used yet, just buffer data
	if (!m_Size)
	{
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, drawtype);
		m_Size = size;
		return;
	}

	//If the buffer is already in use, generate a new buffer and copy
	GLuint prev = m_ID;
	
	unbind();
	glGenBuffers(1, &m_ID);
	bind();
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, drawtype);

	//Copy then delete old buffer
	glCopyBufferSubData(prev, m_ID, 0, 0, m_Size);
	glDeleteBuffers(1, &prev);
	m_Size = size;
}

void SGRender::SSBO::bufferFullData(void* data, GLsizeiptr size)
{
	bufferFullData(data, size, GL_STATIC_DRAW);
}

void SGRender::SSBO::bufferFullData(void* data, GLsizeiptr size, GLenum drawtype)
{
	if (size == m_Size)
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
		return;
	}
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, drawtype);
	m_Size = size; //Changes size if different
}

void SGRender::SSBO::bufferData(void* data, int offset, GLsizeiptr size)
{
	bufferData(data, offset, size, GL_STATIC_DRAW);
}

void SGRender::SSBO::bufferData(void* data, int offset, GLsizeiptr size, GLenum drawtype)
{
	if (size + offset <= m_Size)
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}
	else //Expands if overrun
	{
		reserveData(size + offset);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}
}

void SGRender::SSBO::bind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
}

void SGRender::SSBO::unbind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

SGRender::SSBO::~SSBO()
{
	glDeleteBuffers_s(1, &m_ID);
}

//Vertex Array
SGRender::VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_ID);
}

void SGRender::VertexArray::create() {
	glGenVertexArrays(1, &m_ID);
	glBindVertexArray(m_ID);
}

void SGRender::VertexArray::bind() const {
	glBindVertexArray(m_ID);
}

void SGRender::VertexArray::unbind() const {
	glBindVertexArray(0);
}

void SGRender::VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	bind();
	vb.bind();
	const std::vector<VertexBufferElement> elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const VertexBufferElement element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized,
			layout.getStride(), (const void*)offset);
		offset += element.count * VertexBufferElement::getTypeSize(element.type);
		glVertexAttribDivisor(i, (int)element.instanced);
	}
}

void SGRender::VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout, int start)
{
	bind();
	vb.bind();
	const std::vector<VertexBufferElement> elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const VertexBufferElement element = elements[i];
		glEnableVertexAttribArray(i + start);
		glVertexAttribPointer(i + start, element.count, element.type, element.normalized,
			layout.getStride(), (const void*)offset);
		offset += element.count * VertexBufferElement::getTypeSize(element.type);
		glVertexAttribDivisor(i + start, (int)element.instanced);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

SGRender::ShadowMapFBO::ShadowMapFBO()
{
	m_fbo = 0;
	m_shadowMap = 0;
}

SGRender::ShadowMapFBO::~ShadowMapFBO()
{
	if (m_fbo != 0) {
		glDeleteFramebuffers(1, &m_fbo);
	}

	if (m_shadowMap != 0) {
		glDeleteTextures(1, &m_shadowMap);
	}
}

bool SGRender::ShadowMapFBO::init(int width, int height)
{
	// Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);


    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);

    // Disable writes to the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}


void SGRender::ShadowMapFBO::bindForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}


void SGRender::ShadowMapFBO::bindForReading(uint32_t slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}

//Shader
void SGRender::Shader::deleteShader()
{
	glDeleteProgram_s(m_ID);
}

std::string SGRender::Shader::parseShader(const std::string& filePath) {
	
	const int INCLUDE_SIZE = 11; //Includes white space
	const char* INCLUDE_TOKEN = "#SGInclude";

	std::ifstream stream(filePath);
	std::string shader;
	if (stream.is_open()) {
		std::string line;
		while (std::getline(stream, line)) {
			//Check for SGInclude
			if (line.length() >= INCLUDE_SIZE && line.substr(0, INCLUDE_SIZE -1) == INCLUDE_TOKEN)
			{
				//Paste in file past macro
				std::string includepath = line.substr(INCLUDE_SIZE, line.length() - INCLUDE_SIZE);
				shader += parseShader(includepath);
			}
			else if (line != INCLUDE_TOKEN) //Continue if the line is not just the include text
			{
				shader += line + "\n";
			}
		}
		stream.close();
	}
	return shader;
}

GLuint SGRender::Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader) {
	//returns id of shader program
	GLuint program = glCreateProgram();
	GLuint vs = compileShader(vertexShader, GL_VERTEX_SHADER);
	GLuint fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader_s(vs);
	glDeleteShader_s(fs);

	return program;
}

GLuint SGRender::Shader::createGeoShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
	//returns id of shader program
	GLuint program = glCreateProgram();
	GLuint vs = compileShader(vertexShader, GL_VERTEX_SHADER);
	GLuint fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
	GLuint gs = compileShader(geometryShader, GL_GEOMETRY_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glAttachShader(program, gs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader_s(vs);
	glDeleteShader_s(fs);
	glDeleteShader_s(gs);

	return program;
}

GLuint SGRender::Shader::compileShader(const std::string& source, unsigned int type) {
	GLuint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr); //end specifies length of array which the string is
	glCompileShader(id);                  //(cont) c_str() will alway be null terminating

	//Error handling
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		EngineLog("Shader did not compile correctly");
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)malloc(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		EngineLog(message);
		free(message);

		//Handle
		glDeleteShader_s(id);
		return 0;
	}

	return id;
}


SGRender::Shader::~Shader() {
	deleteShader();
}

void SGRender::Shader::create(const std::string& vert, const std::string& frag) {
	ShaderProgramSource source =
	{
		parseShader(vert), 
		parseShader(frag)
	};
	m_ID = createShader(source.VertexSource, source.FragmentSource);
	EngineLog("Created shader: ", vert, " ", frag);
}

void SGRender::Shader::create(const std::string& vert, const std::string& geo, const std::string& frag) {
	GeoShaderProgramSource source =
	{
		parseShader(vert),
		parseShader(geo),
		parseShader(frag)
	};
	m_ID = createGeoShader(source.VertexSource, source.FragmentSource, source.GeometrySource);
	EngineLog("Created shader: ", vert, " ", geo, " ", frag);
}

void SGRender::Shader::bind() const {
	glUseProgram(m_ID);
}

void SGRender::Shader::unbind() const {
	glUseProgram(0);
}

//uniforms
void SGRender::Shader::setUniform(const std::string& name, int uniform) {
	int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	setUniform(location, uniform);
}

void SGRender::Shader::setUniform(const GLint id, int uniform) {
	glUniform1i(id, uniform);
}

void SGRender::Shader::setUniform(const std::string& name, float uniform) {
	int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	setUniform(location, uniform);
}

void SGRender::Shader::setUniform(const GLint location, float uniform) {
	glUniform1f(location, uniform);
}

void SGRender::Shader::setUniform(const std::string& name, const glm::mat4* uniform) {
	unsigned int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	setUniform(location, uniform);
}

void SGRender::Shader::setUniform(const GLint location, const glm::mat4* uniform) {
	const float* pointer = (float*)(void*)uniform;
	glUniformMatrix4fv(location, 1, GL_FALSE, pointer);
}

void SGRender::Shader::setUniform(const std::string& name, const glm::vec2* uniform) {
	unsigned int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	setUniform(location, uniform);
}

void SGRender::Shader::setUniform(const GLint location, const glm::vec2* uniform) {
	const float* pointer = (float*)(void*)uniform;
	glUniform2fv(location, 1, pointer);
}

void SGRender::Shader::setUniform(const std::string& name, const glm::vec3* uniform) {
	unsigned int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	setUniform(location, uniform);
}

void SGRender::Shader::setUniform(const GLint location, const glm::vec3* uniform) {
	const float* pointer = (float*)(void*)uniform;
	glUniform3fv(location, 1, pointer);
}

void SGRender::Shader::bindToUniformBlock(const std::string& name, GLuint binding)
{
	GLuint uniform_index = glGetUniformBlockIndex(m_ID, name.c_str());
	glUniformBlockBinding(m_ID, uniform_index, binding);
}

GLint SGRender::Shader::getUniformLocation(const std::string& name) {

	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
		return m_UniformLocationCache[name];
	}

	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		EngineLog("Uniform doesn't exist: ", name);
		return -1;
	}
	m_UniformLocationCache[name] = location;

	return location;
}