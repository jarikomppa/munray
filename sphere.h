#pragma once
#ifndef SPHERE_H
#define SPHERE_H

class Sphere : public SceneObject
{
public:
	float mRadius, mRadius2;
	Sphere(
		char *aName,
		const glm::vec3 &aCenter,
		const float &aRadius,
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
	virtual Sphere *getAsSphere() { return this; }
};

#endif