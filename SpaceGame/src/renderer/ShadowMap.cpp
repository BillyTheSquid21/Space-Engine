#include "renderer/ShadowMap.h"

SGRender::ShadowMap::ShadowMap(unsigned int width, unsigned int height) 
	: SHADOW_WIDTH(width), SHADOW_HEIGHT(height) 
{
	
}

void SGRender::ShadowMap::init()
{
	m_FBO.init(SHADOW_WIDTH, SHADOW_WIDTH);
}

void SGRender::ShadowMap::bindForWriting()
{
	m_FBO.bindForWriting();
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
}