#include "renderer/Camera.h"
//Camera
SGRender::Camera::Camera(float width, float height, glm::vec3 position)
{
	//Set width and height
	m_CameraWidth = width; m_CameraHeight = height;
	m_Position = position;
	m_HasMoved = true;
}

void SGRender::Camera::buffer(UniformBuffer& ub)
{
	size_t size = (4 * sizeof(float)) + (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3));
	ub.bind();
	ub.bufferData(this, size);
	ub.unbind();
}

void SGRender::Camera::calcVP() {

	m_View = glm::lookAt(m_Position, m_Position + m_Direction, m_Up); 
	m_VP = m_Proj * m_View;				//Avoid GPU calculation
	m_InverseVP = glm::inverse(m_VP);	//Keep for unmapping purposes
}

void SGRender::Camera::moveInCurrentDirection(float speed) {
	m_Position += speed * m_Direction;
	m_HasMoved = true;
}

void SGRender::Camera::moveUp(float speed) {
	m_Position += speed * m_Up;
	m_HasMoved = true;
}

void SGRender::Camera::moveForwards(float speed) {
	m_Position += speed * glm::vec3(m_Direction.x, 0.0f, m_Direction.z);
	m_HasMoved = true;
}

void SGRender::Camera::moveSideways(float speed) {
	m_Position += speed * -glm::normalize(glm::cross(m_Direction, m_Up));
	m_HasMoved = true;
}

void SGRender::Camera::panSideways(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_HasMoved = true;
}

void SGRender::Camera::panUp(float speed) {
	m_Direction = glm::rotate(m_Direction, speed * glm::radians(100.0f), -glm::normalize(glm::cross(m_Direction, m_Up)));
	m_HasMoved = true;
}

void SGRender::Camera::panYDegrees(float angle) {
	m_Direction = glm::rotate(m_Direction, glm::radians(angle), -glm::normalize(glm::cross(m_Direction, m_Up)));
	m_HasMoved = true;
}

void SGRender::Camera::moveX(float speed) {
	m_Position += speed * glm::vec3(-1.0f, 0.0f, 0.0f);
	m_HasMoved = true;
}

void SGRender::Camera::moveY(float speed) {
	m_Position += speed * glm::vec3(0.0f, -1.0f, 0.0f);
	m_HasMoved = true;
}

void SGRender::Camera::moveZ(float speed) {
	m_Position += speed * glm::vec3(0.0f, 0.0f, -1.0f);
	m_HasMoved = true;
}

void SGRender::Camera::setPos(float x, float y, float z)
{
	m_Position = { x,y,z };
	m_HasMoved = true;
}

glm::vec3 SGRender::Camera::getWorldSpaceClick(float xPos, float yPos)
{
	return unprojectWindow(glm::vec3(xPos, m_CameraHeight - yPos, 0.0f));
}

glm::vec3 SGRender::Camera::unprojectWindow(glm::vec3 pos)
{
	using namespace glm;
	return unProject(pos, m_View, m_Proj, vec4(0.0f, 0.0f, m_CameraWidth, m_CameraHeight));
}

void SGRender::Camera::updateFrustum()
{
	using namespace glm;

	if (!m_HasMoved)
	{
		return;
	}

	//Calc the vertical fov using distance to known near plane
	float fovY = atan((m_CameraHeight / 2.0f) / m_NearPlane);

	//Calculate the dimensions of the far plane
	float halfVSide = m_FarPlane * tanf(fovY*0.25f);
	float halfHSide = halfVSide * (m_CameraWidth / m_CameraHeight);
	vec3 frontMultFar = m_FarPlane * m_Direction;
	vec3 right = normalize(cross(m_Direction, m_Up));
	m_Right = right;

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

bool SGRender::Camera::inFrustum(glm::vec3& pos, float radius)
{
	float threshold = -radius * 1.05f;
	return m_Frustum.bottomFace.getSignedDistanceToPlane(pos) > threshold
		&& m_Frustum.topFace.getSignedDistanceToPlane(pos) > threshold
		&& m_Frustum.nearFace.getSignedDistanceToPlane(pos) > threshold
		&& m_Frustum.farFace.getSignedDistanceToPlane(pos) > threshold
		&& m_Frustum.leftFace.getSignedDistanceToPlane(pos) > threshold
		&& m_Frustum.rightFace.getSignedDistanceToPlane(pos) > threshold;
}