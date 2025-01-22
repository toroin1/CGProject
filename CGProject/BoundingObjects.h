#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>


class GeometryNode;
class Node;

struct Intersection
{
	//the intersection point in world space
	glm::vec3 point;
	//the distance between the the intersection ray origin and the intersection point
	float distance;
	GeometryNode* intersectedNode;
	vector<Node*> path;
};

struct collision 
{
	//the normal to calc the responce velocity vector
	glm::vec3 normal;
	//the time in the frame when collision happens, value [0,1]
	float entrytime;
};

class IBoundingVolume
{
public:
	virtual bool CollidesWithRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, Intersection& intersection) = 0;
};

class BoundingSphere : public IBoundingVolume
{
	//local center
	glm::vec3 center;
	float radius;
	//center in world coordinates after model transformation
	glm::vec3 worldCenter;
	//the radius after applied model scale transformations
	float worldRadius;

protected:
	//the node the bounding volume belongs to
	GeometryNode* node;

public:
	BoundingSphere(GeometryNode* gn, const Model& model)
	{
		node = gn;
		float xmin, ymin, zmin, xmax, ymax, zmax;
		xmin = xmax = model.meshes[0].vertices[0].Position.x;
		ymin = ymax = model.meshes[0].vertices[0].Position.y;
		zmin = zmax = model.meshes[0].vertices[0].Position.z;
		for (unsigned int m = 0; m < model.meshes.size(); m++)
		{
			const Mesh& mesh = model.meshes[m];
			for (unsigned int i = 1; i < mesh.vertices.size(); i++) //popalva masiva s vertexite
			{
				if (mesh.vertices[i].Position.x < xmin)
					xmin = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.x > xmax)
					xmax = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.y < ymin)
					ymin = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.y > ymax)
					ymax = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.z < zmin)
					zmin = mesh.vertices[i].Position.z;
				if (mesh.vertices[i].Position.z > zmax)
					zmax = mesh.vertices[i].Position.z;
			}
		}
		center.x = (xmin + xmax) / 2;
		center.y = (ymin + ymax) / 2;
		center.z = (zmin + zmax) / 2;
		radius = 0;
		for (unsigned int m = 0; m < model.meshes.size(); m++)
		{
			const Mesh& mesh = model.meshes[m];
			for (unsigned int i = 0; i < mesh.vertices.size(); i++)
			{
				float r = glm::length(mesh.vertices[i].Position - center);
				if (r > radius)
					radius = r;
			}
		}
	}

	const glm::vec3& GetCenter() const
	{
		return center;
	}

	const glm::vec3 GetWorldCenter() const
	{
		return worldCenter;
	}

	const float GetRadius() const
	{
		return radius;
	}

	void Transform(const glm::mat4& model)
	{
		worldCenter = model * glm::vec4(center, 1);

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(model, scale, rotation, translation, skew, perspective);
		//multiply the radius with the largest scale (in case of non-uniform scale)
		if (scale.x > scale.y && scale.x > scale.z)
		{
			worldRadius = radius * scale.x;
		}
		else if (scale.y > scale.x && scale.y > scale.z)
		{
			worldRadius = radius * scale.y;
		}
		else
		{
			worldRadius = radius * scale.z;
		}
	}

	bool CollidesWithRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		Intersection& hit)
	{
		glm::vec3 originToCenter = worldCenter - rayOrigin;
		float radiusSqr = worldRadius * worldRadius;
		float lengthOCSqr = pow(glm::length(originToCenter), 2);
		bool  startsOutside = lengthOCSqr > radiusSqr;
		if (!startsOutside)
		{
			return false; //don't handle intersections from inside the bounding sphere
		}

		// 'distance' from ray origin to the perpendicular from the sphere's center to the ray
		float dLen = glm::length(rayDirection);
		float distOF = glm::dot(originToCenter, rayDirection) / dLen;

		// ray starts outside the sphere
		if (startsOutside && distOF < 0)
			return false;

		// 'distance' between the perpendicular and the intersection
		float distFS = radiusSqr - (lengthOCSqr - distOF*distOF);

		// the ray doesn't intersect the sphere
		if (distFS < 0)
			return false;

		// calculate the parameter for the line equation
		float t;
		t = (distOF - (float)sqrt(distFS)) / dLen; //this is the nearer intersection from the two solutions

		hit.point = rayOrigin + rayDirection * t;
		hit.intersectedNode = node;
		hit.distance = t;
	}
};

class BoundingBox :public IBoundingVolume 
{
	glm::vec3 center;
	glm::vec3 lowest;
	glm::vec3 highest;
	glm::vec3 worldcenter;
	glm::vec3 worldlowest;
	glm::vec3 worldhighest;


protected:
	GeometryNode* node;

public:
	BoundingBox(GeometryNode* gn, const Model& model) 
	{
		node = gn;
		float xmin, ymin, zmin, xmax, ymax, zmax;
		xmin = xmax = model.meshes[0].vertices[0].Position.x;
		ymin = ymax = model.meshes[0].vertices[0].Position.y;
		zmin = zmax = model.meshes[0].vertices[0].Position.z;
		for (unsigned int m = 0; m < model.meshes.size(); m++)
		{
			const Mesh& mesh = model.meshes[m];
			for (unsigned int i = 1; i < mesh.vertices.size(); i++) //popalva masiva s vertexite
			{
				if (mesh.vertices[i].Position.x < xmin)
					xmin = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.x > xmax)
					xmax = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.y < ymin)
					ymin = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.y > ymax)
					ymax = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.z < zmin)
					zmin = mesh.vertices[i].Position.z;
				if (mesh.vertices[i].Position.z > zmax)
					zmax = mesh.vertices[i].Position.z;
			}
		}
		lowest.x = xmin;
		lowest.y = ymin;
		lowest.z = zmin;
		highest.x = xmax;
		highest.y = ymax;
		highest.z = zmax;
		center.x = (xmin + xmax) / 2;
		center.y = (ymin + ymax) / 2;
		center.z = (zmin + zmax) / 2;
	}

	BoundingBox(const glm::vec3& Playercenter, float hight, float width, float length) {
		node = NULL;
		center = Playercenter;
		lowest.x = center.x - width / 2;
		lowest.y = center.y - hight / 2;
		lowest.z = center.z - length / 2;
		highest.x = center.x + width / 2;
		highest.y = center.y + hight / 2;
		highest.z = center.z + length / 2;
		worldcenter = center;
		worldhighest = highest;
		worldlowest = lowest;
	}

	void Transform(const glm::mat4& model) 
	{
		worldcenter = model * glm::vec4(center, 1);

		//the alghoritm is from: https://gist.github.com/cmf028/81e8d3907035640ee0e3fdd69ada543f
		glm::vec3 corners[8];
		corners[0] = lowest;
		corners[1] = glm::vec3(lowest.x, highest.y,lowest.z);
		corners[2] = glm::vec3(lowest.x, highest.y, highest.z);
		corners[3] = glm::vec3(lowest.x, lowest.y, highest.z);
		corners[4] = glm::vec3(highest.x, lowest.y, lowest.z);
		corners[5] = glm::vec3(highest.x, highest.y, lowest.z);
		corners[6] = highest;
		corners[7] = glm::vec3(highest.x, lowest.y, highest.z);
		

		glm::vec3 tmin = model * glm::vec4(corners[0], 1);
		glm::vec3 tmax = tmin;

		for (int i = 1;i < 8;i++) {
			glm::vec3 point = model * glm::vec4(corners[i], 1);

			tmin = min(tmin, point);
			tmax = max(tmax, point);
		}

		worldhighest = tmax;
		worldlowest = tmin;
		
	}

	const glm::vec3& getMin() const
	{
		return worldlowest;
	}
	
	const glm::vec3& getMax() const
	{
		return worldhighest;
	}

	const glm::vec3& getFirstMin() const
	{
		return lowest;
	}

	float PlayerCollidesWithAABBSwept(const BoundingBox& box, glm::vec3& normal,const glm::vec3& velocity)
	{
		//the algorithm for the swept aabb collision that returns the time of entry in a frame and
		//finds the normals of the collided surface
		//source: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/

		glm::vec3 InvEntry, Entry;
		glm::vec3 InvExit, Exit;

		if (velocity.x > 0.0f) {
			InvEntry.x = box.getMin().x - worldhighest.x;
			InvExit.x = box.getMax().x - worldlowest.x;
		}
		else 
		{
			InvEntry.x = box.getMax().x - worldlowest.x;
			InvExit.x = box.getMin().x - worldhighest.x;
		}

		if (velocity.y > 0.0f) {
			InvEntry.y = box.getMin().y - worldhighest.y;
			InvExit.y = box.getMax().y - worldlowest.y;
		}
		else
		{
			InvEntry.y = box.getMax().y - worldlowest.y;
			InvExit.y = box.getMin().y - worldhighest.y;
		}

		if (velocity.z > 0.0f) {
			InvEntry.z = box.getMin().z - worldhighest.z;
			InvExit.z = box.getMax().z - worldlowest.z;
		}
		else
		{
			InvEntry.z = box.getMax().z - worldlowest.z;
			InvExit.z = box.getMin().z - worldhighest.z;
		}

		if (velocity.x == 0) 
		{
			Entry.x = -std::numeric_limits<float>::infinity();
			Exit.x = std::numeric_limits<float>::infinity();
		}
		else 
		{
			Entry.x = InvEntry.x / velocity.x;
			Exit.x = InvExit.x / velocity.x;
		}

		if (velocity.y == 0)
		{
			Entry.y = -std::numeric_limits<float>::infinity();
			Exit.y = std::numeric_limits<float>::infinity();
		}
		else
		{
			Entry.y = InvEntry.y / velocity.y;
			Exit.y = InvExit.y / velocity.y;
		}

		if (velocity.z == 0)
		{
			Entry.z = -std::numeric_limits<float>::infinity();
			Exit.z = std::numeric_limits<float>::infinity();
		}
		else
		{
			Entry.z = InvEntry.z / velocity.z;
			Exit.z = InvExit.z / velocity.z;
		}

		float entrytime = max(max(Entry.x, Entry.y), Entry.z);
		float exittime = min(min(Exit.x, Exit.y), Exit.z);

		if (entrytime > exittime || Entry.x < 0.0f && Entry.y < 0.0f && Entry.z < 0.0f || Entry.x>1.0f || Entry.y>1.0f || Entry.z>1.0f) 
		{
			//no collision case returns 1.0f, which means that no collisions are happening until the end of the frame
			normal.x = 0.0f;
			normal.y = 0.0f;
			normal.z = 0.0f;
			return 1.0f;
		}
		else
		{
			if (Entry.x > Entry.y && Entry.x > Entry.z) {
				if (InvEntry.x < 0.0f) {
					normal.x = 1.0f;
					normal.y = 0.0f;
					normal.z = 0.0f;
				}
				else {
					normal.x = -1.0f;
					normal.y = 0.0f;
					normal.z = 0.0f;
				}
			}
			else if (Entry.y > Entry.x && Entry.y > Entry.z) {
				if (InvEntry.y < 0.0f) {
					normal.x = 0.0f;
					normal.y = -1.0f;
					normal.z = 0.0f;
				}
				else {
					normal.x = 0.0f;
					normal.y = 1.0f;
					normal.z = 0.0f;
				}
			}
			else if (Entry.z > Entry.x && Entry.z > Entry.y) {
				if (InvEntry.z < 0.0f) {
					normal.x = 0.0f;
					normal.y = 0.0f;
					normal.z = 1.0f;
				}
				else {
					normal.x = 0.0f;
					normal.y = 0.0f;
					normal.z = -1.0f;
				}
			}
		}
		return entrytime;
	}
	
	bool BroadCheck(const BoundingBox& box,const glm::vec3& velocity) 
	{
		//a broadphase check that adds the velocity to the dimentions of the aabb 
		//and returns if there might be collision in the immediate area of the box 
		float minx = velocity.x > 0 ? worldlowest.x : worldlowest.x + velocity.x;
		float maxx = velocity.x > 0 ? worldhighest.x + velocity.x : worldhighest.x;
		float miny = velocity.y > 0 ? worldlowest.y : worldlowest.y + velocity.y;
		float maxy = velocity.y > 0 ? worldhighest.y + velocity.y : worldhighest.y;
		float minz = velocity.z > 0 ? worldlowest.z : worldlowest.z + velocity.z;
		float maxz = velocity.z > 0 ? worldhighest.z + velocity.z : worldhighest.z;

		return !(maxx<box.getMin().x || minx > box.getMax().x ||
			maxy<box.getMin().y || miny > box.getMax().y ||
			maxz<box.getMin().z || minz > box.getMax().z);
	}

	bool CollidesWithRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		Intersection& hit) 
	{
		return false;
	}

	void TransformWithVelocity(const glm::vec3& velocity) {
		worldcenter += velocity;
		worldhighest += velocity;
		worldlowest += velocity;
	}
};
