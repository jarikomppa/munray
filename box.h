#pragma once
#ifndef BOX_H
#define BOX_H

class Box : public SceneObject
{
public:
	glm::vec3 mSize;
	glm::vec3 mMin, mMax;
	Box(
		char *aName,
		const glm::vec3 &aCenter,
		const glm::vec3 &aSize,
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
	virtual Box *getAsBox() { return this; }
	void calcSizes();
};

#endif