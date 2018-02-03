#include "munray.h"

Light::Light(
	char *aName,
	const glm::vec3 aPosition,
	Material *aMaterial)
{
	mName = aName;
	mPosition = aPosition;
	mMaterial = aMaterial;
	mLight = true;
}

bool Light::intersect(const glm::vec3 &aRayOrigin, const glm::vec3 &aRayDirection, const glm::vec3 &aRayDirectionInv, float &aCollisionDistance) const {
	return false;
};

void Light::getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal)  {};
void Light::getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const { aRadius = (float)INFINITY; aCenter = mPosition; };
void Light::getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const { aMin = aMax = glm::vec3((float)INFINITY); };
glm::vec3 Light::getTextureCoord(glm::vec3 &aCollisionPoint) { return glm::vec3(0); }
