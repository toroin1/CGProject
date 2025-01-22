#pragma once

#include "Node.h"
#include "TransformNode.h"
#include "Model.h"
#include "BoundingObjects.h"
#include <glm\gtc\matrix_transform.hpp>

class GeometryNode : public Node
{
	Model model;
	Shader* shader;
	Shader* ShadowShader;
	BoundingSphere* boundingSphere = NULL;
	BoundingBox* boundingBox = NULL;

public:
	GeometryNode() :  Node()
	{
		type = nt_GeometryNode;
	}

	GeometryNode(const std::string& name) : Node(name, nt_GeometryNode)
	{

	}

	GeometryNode(const std::string& name, const std::string& path) : Node(name, nt_GeometryNode)
	{
		LoadFromFile(path);
	}

	~GeometryNode()
	{
		if (boundingSphere != NULL)
		{
			delete boundingSphere;
		}
		if (boundingBox != NULL)
		{
			delete boundingBox;
		}
	}

	void LoadFromFile(const std::string& path)
	{
		model.LoadModel(path);
		boundingSphere = new BoundingSphere(this, model);
		boundingBox = new BoundingBox(this, model);
	}

	const Model& GetModel() const
	{
		return model;
	}

	void SetShader(Shader* s)
	{
		shader = s;
	}

	void SetShadowShader(Shader* sh)
	{
		ShadowShader = sh;
	}

	const BoundingSphere& GetBoundingSphere()
	{
		return *boundingSphere;
	}

	BoundingBox& GetBoundingBox()
	{
		return *boundingBox;
	}

	void Traverse()
	{
		glm::mat4 transform = TransformNode::GetTransformMatrix();
		shader->setMat4("model", transform);
		glm::mat3 normalMat = glm::transpose(glm::inverse(transform));
		shader->setMat3("normalMat", normalMat);
		boundingSphere->Transform(transform);
		boundingBox->Transform(transform);
		//printf("\n(%f, %f, %f)", boundingBox->getMin().x, boundingBox->getMin().y, boundingBox->getMin().z);
		model.Draw(*shader);
	}

	void TraverseShadows()
	{
		glm::mat4 transform = TransformNode::GetTransformMatrix();
		ShadowShader->setMat4("model", transform);
		model.Draw(*ShadowShader);
	}

	virtual void TraverseIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		vector<Intersection*>& hits, vector<Node*>& path)
	{
		glm::mat4 transform = TransformNode::GetTransformMatrix();
		boundingSphere->Transform(transform);
		Intersection* hit = new Intersection();
		if (boundingSphere->CollidesWithRay(rayOrigin, rayDirection, *hit))
		{
			hit->path = path;
			hits.push_back(hit);
		}
	}

	virtual void TraverseCollisions(BoundingBox& player, const glm::vec3& velocity, vector<collision*>& collisions)
	{
		glm::mat4 transform = TransformNode::GetTransformMatrix();
		boundingBox->Transform(transform);
		
		
		if(player.BroadCheck(*boundingBox,velocity))
		{
			collision* coll = new collision();
			coll->entrytime = player.PlayerCollidesWithAABBSwept(*boundingBox, coll->normal, velocity);
			collisions.push_back(coll);
			
		}

	}
};
