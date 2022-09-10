#include "renderer/Camera.h"
//Camera
SGRender::Camera::Camera(float width, float height, glm::vec3 position)
{
	//Set width and height
	m_CameraWidth = width; m_CameraHeight = height;
	m_Position = position;

}

void SGRender::Camera::sendCameraUniforms(Shader& shader) {

	m_View = glm::lookAt(m_Position, m_Position + m_Direction, m_Up); 

	shader.setUniform("u_View", &m_View);
	shader.setUniform("u_Proj", &m_Proj);
}

void SGRender::Camera::moveForwards(float speed) {
	m_Position += speed * m_Direction;
}

void SGRender::Camera::moveUp(float speed) {
	m_Position += speed * m_Up;
}

void SGRender::Camera::moveSideways(float speed) {
	m_Position += speed * -glm::normalize(glm::cross(m_Direction, m_Up));
}

void SGRender::Camera::panSideways(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void SGRender::Camera::panUp(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), -glm::normalize(glm::cross(m_Direction, m_Up)));
}

void SGRender::Camera::panYDegrees(float angle) {
	m_Direction = glm::rotate(m_Direction, glm::radians(angle), -glm::normalize(glm::cross(m_Direction, m_Up)));
}

void SGRender::Camera::moveX(float speed) {
	m_Position += speed * glm::vec3(-1.0f, 0.0f, 0.0f);
}

void SGRender::Camera::moveY(float speed) {
	m_Position += speed * glm::vec3(0.0f, -1.0f, 0.0f);
}

void SGRender::Camera::moveZ(float speed) {
	m_Position += speed * glm::vec3(0.0f, 0.0f, -1.0f);
}

void SGRender::Camera::setPos(float x, float y, float z)
{
	m_Position = { x,y,z };
}

void SGRender::Camera::updateFrustum()
{
	using namespace glm;

	//Calc the vertical fov using distance to known near plane
	float fovY = atan((m_CameraHeight / 2.0f) / m_NearPlane);

	//Calculate the dimensions of the far plane
	float halfVSide = m_FarPlane * tanf(fovY*0.25f);
	float halfHSide = halfVSide * (m_CameraWidth / m_CameraHeight);
	vec3 frontMultFar = m_FarPlane * m_Direction;
	vec3 right = normalize(cross(m_Direction, m_Up));

	//Calculate faces
	m_Frustum.nearFace = { m_Position + m_NearPlane * m_Direction, m_Direction };
	m_Frustum.farFace = { m_Position + frontMultFar, -m_Direction };
	m_Frustum.rightFace = { m_Position,
							cross(m_Up,frontMultFar + right * halfHSide) };
	m_Frustum.leftFace = { m_Position,
							cross(frontMultFar - right * halfHSide, m_Up) };
	m_Frustum.topFace = { m_Position,
							cross(right, frontMultFar - m_Up * halfVSide) };
	m_Frustum.bottomFace = { m_Position,
							cross(frontMultFar + m_Up * halfVSide, right) };
}