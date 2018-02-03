#pragma once
#ifndef PLANE_H
#define PLANE_H

class Plane : public SceneObject
{
public:
	glm::vec3 mNormal;
	Plane(
		char *aName,
		const glm::vec3 &aPoint,
		const glm::vec3 &aNormal,
		Material *aMaterial);

	virtual void getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const;
	virtual void getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const;
	virtual bool intersect(
		const glm::vec3 &aRayOrigin,
		const glm::vec3 &aRayDirection,
		const glm::vec3 &aRayDirectionInv,
		float &aCollisionDistance) const;

	virtual void getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal);
	virtual glm::vec3 getTextureCoord(glm::vec3 &aCollisioPoint);
	virtual Plane *getAsPlane() { return this; }
};

#endif