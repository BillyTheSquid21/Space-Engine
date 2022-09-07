#pragma once
#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "renderer/GLClasses.h"

class ShadowMap
{
public:
	ShadowMap() = default;
	ShadowMap(unsigned int width, unsigned int height);
	
	void init();
	void setProjection(glm::mat4 proj) { m_Proj = proj; }
	void bindForWriting();
	void bindForReading(unsigned int slot) { m_FBO.bindForReading(slot); }
	
	glm::mat4* calcMVP(glm::mat4 model, glm::mat4 lightView) { m_MVP = m_Proj * lightView * model; return &m_MVP; }
	glm::mat4* getMVP() { return &m_MVP; }

	void startCapture() const { glCullFace(GL_FRONT); };
	void endCapture(unsigned int width, unsigned int height) const { glCullFace(GL_BACK); m_FBO.unbind(); glViewport(0, 0, width, height); };

private:
	ShadowMapFBO m_FBO;
	glm::mat4 m_Proj;
	glm::mat4 m_MVP; 
	unsigned int SHADOW_WIDTH; unsigned int SHADOW_HEIGHT;
};

#endif