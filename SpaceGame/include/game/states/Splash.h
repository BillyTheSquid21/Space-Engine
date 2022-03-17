#pragma once
#ifndef SPLASH_H
#define SPLASH_H

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "core/State.hpp"

class Splash : public State
{
public:
	void init(int width, int height);
	void update(double deltaTime, double time);
	void render();
private:
	Render::Renderer<ColorTextureVertex> m_Renderer;
	Texture m_SplashTexture;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;

	Quad m_Splash;
	Quad m_Splash2;
	float angle = 0.0f;
	double m_FadeIn = 3.0;
	double m_CurrentTime = 0.0;
};

#endif