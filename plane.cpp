#include "munray.h"

Plane::Plane(
	char *aName,
	const glm::vec3 &aPoint,
	const glm::vec3 &aNormal,
	Material *aMaterial)
{
	mName = aName;
	mPosition = aPoint;
	mNormal = glm::normalize(aNormal);
	mMaterial = aMaterial;
}

void Plane::getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const
{
	aCenter = mPosition;
	aRadius = (float)INFINITY;
}

void Plane::getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const
{
	// this could actually be set to something sensible..
	aMax = aMin = glm::vec3((float)INFINITY);
}

bool Plane::intersect(
	const glm::vec3 &aRayOrigin,
	const glm::vec3 &aRayDirection,
	const glm::vec3 &aRayDirectionInv,
	float &aCollisionDistance) const
{
	float distance = (glm::dot(mNormal, mPosition - aRayOrigin)) / glm::dot(mNormal, aRayDirection);
	aCollisionDistance = distance;
	if (distance >= 0)
		return true;
	return false;
}

void Plane::getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal)
{
	aNormal = mNormal;
}

glm::vec3 Plane::getTextureCoord(glm::vec3 &aCollisionPoint)
{
	unsigned int x = (int)(fmod(1e6f + aCollisionPoint.x * 0.2f, 16.0f));
	unsigned int y = (int)(fmod(1e6f + aCollisionPoint.z * 0.2f, 16.0f));
	return glm::vec3(x, y, 0);
}

/*
glm::vec3 Plane::getColor(glm::vec3 &aCollisionPoint) const
{
	unsigned int x = (int)(fmod(1e6 + aCollisionPoint.x * 0.2f, 16.0f));
	unsigned int y = (int)(fmod(1e6 + aCollisionPoint.z * 0.2f, 16.0f));
	if ((x & 1) ^ (y & 1))
		return mColor;

	return glm::vec3(0.5f);
}
*/