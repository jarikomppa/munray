#include "munray.h"

Box::Box(
	char *aName,
	const glm::vec3 &aCenter,
	const glm::vec3 &aSize,
	Material *aMaterial)
{
	mName = aName;
	mPosition = aCenter;
	mSize = aSize;
	mMaterial = aMaterial;
	calcSizes();
}

void Box::calcSizes()
{
	mMin = mPosition - mSize;
	mMax = mPosition + mSize;
}

void Box::getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const
{
	aCenter = mPosition;
	aRadius = glm::length(mSize);
}

void Box::getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const
{
	aMin = mMin;
	aMax = mMax;
}

bool Box::intersect(
	const glm::vec3 &aRayOrigin,
	const glm::vec3 &aRayDirection,
	const glm::vec3 &aRayDirectionInv,
	float &aCollisionDistance) const
{
	float t1 = (mMin.x - aRayOrigin.x) * aRayDirectionInv.x;
	float t2 = (mMax.x - aRayOrigin.x) * aRayDirectionInv.x;

	float tmin = glm::min(t1, t2);
	float tmax = glm::max(t1, t2);

	t1 = (mMin.y - aRayOrigin.y) * aRayDirectionInv.y;
	t2 = (mMax.y - aRayOrigin.y) * aRayDirectionInv.y;

	tmin = glm::max(tmin, glm::min(t1, t2));
	tmax = glm::min(tmax, glm::max(t1, t2));

	t1 = (mMin.z - aRayOrigin.z) * aRayDirectionInv.z;
	t2 = (mMax.z - aRayOrigin.z) * aRayDirectionInv.z;

	tmin = glm::max(tmin, glm::min(t1, t2));
	tmax = glm::min(tmax, glm::max(t1, t2));

	aCollisionDistance = tmin;

	return tmax > glm::max(tmin, 0.0f);
}

void Box::getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal)
{
	glm::vec3 p = glm::normalize(aCollisionPoint - mPosition);
	glm::vec3 ap = glm::abs(p / mSize);
	glm::vec3 as = glm::sign(p);
	aNormal = glm::vec3(0, 0, as.z);
	if (ap.x > ap.z)
	{
		aNormal = glm::vec3(as.x, 0, 0);
		if (ap.y > ap.x)
		{
			aNormal = glm::vec3(0, ap.y, 0);
		}
	}
	else
	{
		if (ap.y > ap.z)
		{
			aNormal = glm::vec3(0, ap.y, 0);
		}
	}
}

glm::vec3 Box::getTextureCoord(glm::vec3 &aCollisionPoint)
{
	// this is all wrong..
	return glm::normalize(aCollisionPoint);
}

