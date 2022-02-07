#include "Renderer.h"

//RENDERER

//Camera
Camera::Camera(float width, float height)
	:m_CameraWidth{ width }, m_CameraHeight{ height },
	m_Projection{ glm::ortho(-width / 2, width / 2, -height / 2, height / 2, -1.0f, 2.0f) } //depth buffer layer - above +1 is GUI
{
	m_CameraBounds = { -m_CameraWidth / 2,m_CameraWidth / 2,-m_CameraHeight / 2,m_CameraHeight / 2 };
}

void Camera::positionCamera(float x, float y) {
	m_Translation[3][0] = x;
	m_Translation[3][1] = y;
	m_Translation[3][2] = 1.0f;

	//position bounds
	m_CameraBounds = { x - (m_CameraWidth / 2), x + (m_CameraWidth / 2), y - (m_CameraHeight / 2), y + (m_CameraHeight / 2) };
}

void Camera::translateCamera(float deltaX, float deltaY) {
	m_Translation[3][0] += deltaX;
	m_Translation[3][1] += deltaY;
	m_Translation[3][2] = 1.0f;

	//translate bounds
	m_CameraBounds.a += deltaX;
	m_CameraBounds.b += deltaX;
	m_CameraBounds.c += deltaY;
	m_CameraBounds.d += deltaY;
}

void Camera::zoomCamera(float deltaZoom) {
	setZoomCamera(m_Scaling.x + deltaZoom);
}

void Camera::setZoomCamera(float zoom) {
	//correct back to original scale
	m_CameraBounds.a *= m_Scaling.x; m_CameraBounds.b *= m_Scaling.x;
	m_CameraBounds.c *= m_Scaling.y; m_CameraBounds.d *= m_Scaling.y;

	//scale
	m_Scaling.x = zoom;
	m_Scaling.y = zoom;

	//scale accordingly
	m_CameraBounds.a /= m_Scaling.x; m_CameraBounds.b /= m_Scaling.x;
	m_CameraBounds.c /= m_Scaling.y; m_CameraBounds.d /= m_Scaling.y;
}

void Camera::sendCameraUniforms(Shader& shader) {
	//multiply
	m_ResultantMatrix = glm::scale(m_Projection * m_Translation, m_Scaling);
	shader.setUniform("u_Projection", &m_ResultantMatrix);
}

//GUI
GUI::GUI(float width, float height)
	:m_GUIWidth{ width }, m_GUIHeight{ height },
	m_Projection{ glm::ortho(-width / 2, width / 2, -height, 0.0f, -1.0f, 2.0f) }
{}

void GUI::setScale(float scale) {
	//scale
	m_Scaling.x = scale;
	m_Scaling.y = scale;
}

void GUI::sendGUIUniforms(Shader& shader) {
	//multiply
	m_ResultantMatrix = glm::scale(m_Projection, m_Scaling);
	shader.setUniform("u_Projection", &m_ResultantMatrix);
}