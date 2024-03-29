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
	enum class CamMotion
	{
		Move_Forward, Move_Sideways, Move_Up, Move_CurrentDir, Pan_Sideways, //TODO Fill out later
	};

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
		* Set the projection of the camera to any mat4
		*/
		void setProjection(glm::mat4 project) { m_Proj = project; }

		//move camera relative to orientation
		void moveInCurrentDirection(float speed);
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

		/**
		* Check if position and radius is in frustum
		*/
		bool inFrustum(glm::vec3& pos, float radius);

		/**
		* Passes the Uniform Buffer for the camera to write to
		*/
		void buffer(UniformBuffer& ub);

		/**
		* Get the world space position of a click on the screen
		*/
		glm::vec3 getWorldSpaceClick(float xPos, float yPos);

		/**
		* Signal camera movement to reset
		*/
		void resetMovementFlag() { m_HasMoved = false; };

		void setPos(float x, float y, float z);
		void setDir(glm::vec3& dir) { m_Direction = dir; }
		glm::vec3 getPos() const { return m_Position; }
		glm::vec3 getDir() const { return m_Direction; }
		glm::vec3 getRight() const { return m_Right; }
		void calcVP();
		const glm::mat4& getVP() const { return m_VP; }
		const glm::mat4& getProj() const { return m_Proj; }
		const glm::mat4& getView() const { return m_View; }
		glm::vec3 unprojectWindow(glm::vec3 pos);
		bool hasMoved() { return m_HasMoved; }

		//set camera
		void panYDegrees(float degrees);

		//getters and setters
		void setNearAndFarPlane(float near, float far) { m_NearPlane = near; m_FarPlane = far; m_LogFN = log(far/near); }
		void setFOV(float deg) { m_FOV = glm::radians(deg); }
		float speed() const { return m_Speed; }
		float width() const { return m_CameraWidth; }
		float height() const { return m_CameraHeight; }
		float FOV() const { return m_FOV; }
		float aspect() const { return m_CameraHeight / m_CameraWidth; }
		float nearPlane() const { return m_NearPlane; }
		float farPlane() const { return m_FarPlane; }

		/**
		* Get a reference to the camera frustum as it was when last updated
		*/
		Frustum& getFrustum() { return m_Frustum; }

	private:
		//buffered properties in order
		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Proj = glm::mat4(1.0f);
		glm::vec3 m_Position = {};
		float m_CameraWidth = 0.0f; 
		float m_CameraHeight = 0.0f;
		float m_NearPlane = 0.1f; 
		float m_FarPlane = 1000.0f;
		float m_LogFN = 1.0f; //Precalculate log

		//unbuffered properties
		glm::vec3 m_Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
		float m_Speed = 0.1f;
		float m_Sensitivity = 100.0f;
		float m_FOV = 0.0f;

		glm::mat4 m_VP = glm::mat4(1.0f);
		glm::mat4 m_InverseVP = glm::mat4(1.0f);

		Frustum m_Frustum;
		bool m_HasMoved = false;
	};
}

#endif