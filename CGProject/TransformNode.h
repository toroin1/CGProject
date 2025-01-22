#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "GroupNode.h"

class TransformNode : public GroupNode
{
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 scale;

	static glm::mat4 transformMatrix;

public:
	TransformNode(const std::string& name) : GroupNode(name)
	{
		type = nt_TransformNode;
		translation = glm::vec3(0.0f);
		rotation = glm::vec3(0.0f);
		scale = glm::vec3(1.0f);
	}

	void SetTranslation(const glm::vec3& tr)
	{
		translation = tr;
	}

	void SetScale(const glm::vec3& sc)
	{
		scale = sc;
	}

	void SetRotation(const glm::vec3& rt)
	{
		rotation = rt;
	}

	void MoveX(float delta)
	{
		translation.x += delta;
	}

	void TraverseShadows()
	{
		//push
		glm::mat4 matCopy = transformMatrix;

		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transformMatrix = glm::scale(transformMatrix, scale);
		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->TraverseShadows();
		}

		//pop
		transformMatrix = matCopy;
	}

	void Traverse()
	{
		//push
		glm::mat4 matCopy = transformMatrix;

		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transformMatrix = glm::scale(transformMatrix, scale);
		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->Traverse();
		}

		//pop
		transformMatrix = matCopy;
	}

	virtual void TraverseIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		vector<Intersection*>& hits, vector<Node*>& path)
	{
		//push
		glm::mat4 matCopy = transformMatrix;
		path.push_back(this);

		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transformMatrix = glm::scale(transformMatrix, scale);
		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->TraverseIntersection(rayOrigin, rayDirection, hits, path);
		}
		//pop
		transformMatrix = matCopy;
		path.pop_back();
	}

	virtual void TraverseCollisions(BoundingBox& player, const glm::vec3& velocity, vector<collision*>& collisions)
	{
		glm::mat4 matCopy = transformMatrix;
		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		transformMatrix = glm::scale(transformMatrix, scale);

		for (unsigned int i = 0; i < children.size(); i++) {
			children[i]->TraverseCollisions(player, velocity, collisions);
		}
		transformMatrix = matCopy;
	}

	static const glm::mat4 GetTransformMatrix()
	{
		return transformMatrix;
	}
};