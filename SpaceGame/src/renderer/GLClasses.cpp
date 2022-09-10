#include "renderer/GLClasses.h"
#include "renderer/Vertex.hpp"

//count means number of, size is in bytes

void SGRender::VertexBuffer::create(size_t dataSize) {
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	//tells how much data to set aside - size in bytes
	glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
}

SGRender::VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &m_ID);
}

void SGRender::VertexBuffer::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void SGRender::VertexBuffer::unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SGRender::VertexBuffer::bufferData(const void* data, unsigned int count) {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), data);
}

void SGRender::IndexBuffer::create(unsigned int count)
{
	m_IndicesCount = count;
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
}

void SGRender::IndexBuffer::bufferData(const void* data, unsigned int count) {
	m_IndicesCount = count;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(unsigned int), data);
}

SGRender::IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &m_ID);
}

void SGRender::IndexBuffer::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void SGRender::IndexBuffer::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

bool SGRender::ShadowMapFBO::init(unsigned int width, unsigned int height)
{
	// Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


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


void SGRender::ShadowMapFBO::bindForReading(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}

//Shader
std::string SGRender::Shader::parseShader(const std::string& filePath) {
	std::ifstream stream(filePath);
	std::stringstream buff;
	buff << stream.rdbuf();
	return buff.str();
}

unsigned int SGRender::Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader) {
	//returns id of shader program
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(vertexShader, GL_VERTEX_SHADER);
	unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

unsigned int SGRender::Shader::createGeoShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
	//returns id of shader program
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(vertexShader, GL_VERTEX_SHADER);
	unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
	unsigned int gs = compileShader(geometryShader, GL_GEOMETRY_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glAttachShader(program, gs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteShader(gs);

	return program;
}

unsigned int SGRender::Shader::compileShader(const std::string& source, unsigned int type) {
	unsigned int id = glCreateShader(type);
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
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		EngineLog(message);

		//Handle
		glDeleteShader(id);
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
}

void SGRender::Shader::create(const std::string& vert, const std::string& geo, const std::string& frag) {
	GeoShaderProgramSource source =
	{
		parseShader(vert),
		parseShader(geo),
		parseShader(frag)
	};
	m_ID = createGeoShader(source.VertexSource, source.FragmentSource, source.GeometrySource);
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
	glUniform1i(location, uniform);
}

void SGRender::Shader::setUniform(const std::string& name, float uniform) {
	int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	glUniform1f(location, uniform);
}

void SGRender::Shader::setUniform(const std::string& name, const glm::mat4* uniform) {
	unsigned int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	const float* pointer = (float*)(void*)uniform;
	glUniformMatrix4fv(location, 1, GL_FALSE, pointer);
}

void SGRender::Shader::setUniform(const std::string& name, const glm::vec3* uniform) {
	unsigned int location = getUniformLocation(name);
	if (location == -1) {
		return;
	}
	const float* pointer = (float*)(void*)uniform;
	glUniform3fv(location, 1, pointer);
}

int SGRender::Shader::getUniformLocation(const std::string& name) {

	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
		return m_UniformLocationCache[name];
	}

	int location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		EngineLog("Uniform doesn't exist: ", name);
	}
	m_UniformLocationCache[name] = location;

	return location;
}