#include "munray.h"

Sphere::Sphere(
	char *aName,
	const glm::vec3 &aCenter,
	const float &aRadius,
	Material *aMaterial)
{
	mName = aName;
	mPosition = aCenter;
	mRadius = aRadius;
	mRadius2 = aRadius * aRadius;
	mMaterial = aMaterial;
}

void Sphere::getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const
{
	aCenter = mPosition;
	aRadius = mRadius;
}

void Sphere::getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const
{
	aMin = mPosition - mRadius;
	aMax = mPosition + mRadius;
}


bool Sphere::intersect(
	const glm::vec3 &aRayOrigin,
	const glm::vec3 &aRayDirection,
	const glm::vec3 &aRayDirectionInv,
	float &aCollisionDistance) const
{
	glm::vec3 l = mPosition - aRayOrigin;
	float tca = glm::dot(l, aRayDirection);
	if (tca < 0) return false;
	float d2 = glm::dot(l, l) - tca * tca;
	if (d2 > mRadius2) return false;
	float thc = sqrt(mRadius2 - d2);
	float point1 = tca - thc;
	float point2 = tca + thc;
	if (point1 < 0 || ((point2 >= 0) && (point2 < point1))) aCollisionDistance = point2;
	aCollisionDistance = point1;

	return true;
}

void Sphere::getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal)
{
	aNormal = aCollisionPoint - mPosition;
}

glm::vec3 Sphere::getTextureCoord(glm::vec3 &aCollisionPoint)
{
	return glm::normalize(aCollisionPoint);
}
