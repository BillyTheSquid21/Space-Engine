#include "renderer/Camera.h"
//Camera
Camera::Camera(float width, float height, glm::vec3 position)
{
	//Set width and height
	m_CameraWidth = width; m_CameraHeight = height;
	m_Position = position;

}

void Camera::sendCameraUniforms(Shader& shader) {

	view = glm::lookAt(m_Position, m_Position + m_Direction, m_Up); 

	shader.setUniform("u_View", &view);
	shader.setUniform("u_Proj", &proj);
}

void Camera::moveForwards(float speed) {
	m_Position += speed * m_Direction;
}

void Camera::moveUp(float speed) {
	m_Position += speed * m_Up;
}

void Camera::moveSideways(float speed) {
	m_Position += speed * -glm::normalize(glm::cross(m_Direction, m_Up));
}

void Camera::panSideways(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::panUp(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), -glm::normalize(glm::cross(m_Direction, m_Up)));
}

void Camera::panYDegrees(float angle) {
	m_Direction = glm::rotate(m_Direction, glm::radians(angle), -glm::normalize(glm::cross(m_Direction, m_Up)));
}

void Camera::moveX(float speed) {
	m_Position += speed * glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Camera::moveY(float speed) {
	m_Position += speed * glm::vec3(0.0f, -1.0f, 0.0f);
}

void Camera::moveZ(float speed) {
	m_Position += speed * glm::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::setPos(float x, float y, float z)
{
	m_Position = { x,y,z };
}