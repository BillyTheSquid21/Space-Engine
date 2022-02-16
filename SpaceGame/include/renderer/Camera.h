#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "GLClasses.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera
{
public:
	Camera() = default;
	Camera(float width, float height, glm::vec3 position);
	~Camera() = default;

	//get data from camera
	void sendCameraUniforms(Shader& shader);

	//move camera relative to orientation
	void moveForwards(float speed);
	void moveSideways(float speed);
	void moveUp(float speed);
	void panSideways(float speed);
	void panUp(float speed);

	//move camera in absolute terms
	void moveX(float speed);
	void moveY(float speed);
	void moveZ(float speed);

	//set camera
	void panYDegrees(float degrees);

	//getters and setters
	void setNearAndFarPlane(float near, float far) { nearPlane = near; farPlane = far; }
	float speed() const { return m_Speed; }

private:
	float m_CameraWidth; float m_CameraHeight;

	//properties
	glm::vec3 m_Position;
	glm::vec3 m_Direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_Speed = 0.1f;
	float sensitivity = 100.0f;
	float nearPlane = 0.1f; float farPlane = 1000.0f;

	//mvp
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);
};

#endif