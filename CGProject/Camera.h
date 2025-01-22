#pragma once
//camera class from learnopengl.com

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include "Model.h"
#include "BoundingObjects.h"
#include "GroupNode.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.5f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	BoundingBox* playerbox = NULL;
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 VelVec;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{//create default for playerbox
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		setupPlayerbox();
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime,GroupNode*& gRoot)
	{
		float velocity = MovementSpeed * deltaTime;
		
		//make the resolution
		vector<collision*> coll;
		glm::vec3 normaly = glm::vec3(0, 0, 0);

		if (direction == FORWARD)
			VelVec = Front * velocity;
		if (direction == BACKWARD)
			VelVec= -Front * velocity;
		if (direction == LEFT)
			VelVec = -Right * velocity;
		if (direction == RIGHT)
			VelVec = Right * velocity;

		/*for limiting the movement of the camera to x and z directions
		if(VelVec.y != 0)
		{
			normaly = VelVec.y > 0 ? glm::vec3(0, 1, 0) : glm::vec3(0, 1, 0);
		}
		float length = glm::length(VelVec);
		VelVec = (VelVec - normaly * glm::dot(VelVec, normaly));
		VelVec *= length / glm::length(VelVec);*/

		//find all collisions and calc the resulting velocity vector
		gRoot->TraverseCollisions(*playerbox, VelVec, coll);
		if(coll.size()>0)
		{
			
			for (unsigned int i = 0;i < coll.size();i++) 
			{
		
				float remainingtime = 1.0 - coll[i]->entrytime;
				float dotpr = glm::dot(VelVec, coll[i]->normal)*remainingtime;
				VelVec = VelVec - coll[i]->normal * dotpr;
				delete coll[i];/*delete the collision's pointers made in treversecollisions*/
				
			}
		}
		
		
		VelVec.y = 0;
		Position += VelVec;
		playerbox->TransformWithVelocity(VelVec);
	

	}



	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void setupPlayerbox() 
	{
		playerbox = new BoundingBox(Position, 1.0f, 0.5f, 1.0f);
	}
};

