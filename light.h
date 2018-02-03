#pragma once
#ifndef LIGHT_H
#define LIGHT_H

class Light : public SceneObject
{
public:
	Light(
		char *aName,
		const glm::vec3 aPosition,
		Material *aMaterial);
	virtual bool intersect(const glm::vec3 &aRayOrigin, const glm::vec3 &aRayDirection, const glm::vec3 &aRayDirectionInv, float &aCollisionDistance) const;
	virtual void getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal);
	virtual void getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const;
	virtual void getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const;
	virtual glm::vec3 getTextureCoord(glm::vec3 &aCollisioPoint);
	virtual Light *getAsLight() { return this; }
};

#endif