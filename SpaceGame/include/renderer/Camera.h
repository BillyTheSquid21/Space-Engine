#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "renderer/GLClasses.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "cmath"

namespace SGRender
{
	/**
	* Mathmatical definition of a plane (Normal and distance)
	* Not to be confused with the geometric plane
	*/
	struct Plane
	{
		Plane() = default;

		Plane(const glm::vec3& p1, const glm::vec3& norm)
			: normal(glm::normalize(norm)),
			distance(glm::dot(normal, p1))
		{}

		inline float getSignedDistanceToPlane(const glm::vec3& point) const
		{
			return glm::dot(normal, point) - distance;
		}

		glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
		float distance = 0.0f;
	};

	/**
	* Contains the plane required to define a frustum
	*/
	struct Frustum
	{
		Plane topFace;
		Plane bottomFace;

		Plane nearFace;
		Plane farFace;

		Plane leftFace;
		Plane rightFace;
	};

	/**
	* Contains the projection, position, and other data required for a camera
	* Supports simple (non quaternion) motion currently
	*/
	class Camera
	{
	public:
		Camera() = default;
		Camera(float width, float height, glm::vec3 position);
		~Camera() = default;

		/**
		* Send the uniforms for the camera to the given shader (ensure shader is bound first)
		*/
		void sendCameraUniforms(Shader& shader);

		/**
		* Set the projection of the camera to any mat4
		*/
		void setProjection(glm::mat4 project) { m_Proj = project; }

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

		/**
		* Update the frustum for the camera in its current position/rotation
		*/
		void updateFrustum();

		//set pos
		void setPos(float x, float y, float z);
		glm::vec3 getPos() const { return m_Position; }
		glm::mat4 getVP() { return m_Proj * m_View; }

		//set camera
		void panYDegrees(float degrees);

		//getters and setters
		void setNearAndFarPlane(float near, float far);
		void setFOV(float deg) { m_FOV = glm::radians(deg); }
		float speed() const { return m_Speed; }
		float width() const { return m_CameraWidth; }
		float height() const { return m_CameraHeight; }

		/**
		* Get a reference to the camera frustum as it was when last updated
		*/
		Frustum& getFrustum() { return m_Frustum; }

	private:
		float m_CameraWidth = 0.0f; float m_CameraHeight = 0.0f;

		//properties
		glm::vec3 m_Position = {};
		glm::vec3 m_Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		float m_Speed = 0.1f;
		float m_Sensitivity = 100.0f;
		float m_NearPlane = 0.1f; float m_FarPlane = 1000.0f;
		float m_FOV = 0.0f;

		//mvp
		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Proj = glm::mat4(1.0f);

		//frustum
		Frustum m_Frustum;
	};
}

#endif