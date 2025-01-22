#pragma once

#include <string>
#include <glm/glm.hpp>
#include "BoundingObjects.h"

enum NodeType
{
	nt_Node = 0,
	nt_GroupNode,
	nt_TransformNode,
	nt_GeometryNode
};

class Node
{
	static unsigned int genID;

protected:
	std::string name;
	unsigned int ID;
	NodeType type;

public:
	Node()
	{
		name = "";
		ID = ++genID;
		type = nt_Node;
	}

	const std::string& GetName()
	{
		return name;
	}

	Node(const std::string& name, NodeType t = nt_Node)
	{
		this->name = name;
		ID = ++genID;
	}

	virtual void Traverse() = 0;
	virtual void TraverseShadows() = 0;
	virtual void TraverseIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, vector<Intersection*>& hits, vector<Node*>& path) = 0;
	virtual void TraverseCollisions(BoundingBox& player, const glm::vec3& velocity, vector<collision*>& collisions) = 0;
};