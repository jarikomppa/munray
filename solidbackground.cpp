#include "munray.h"

SolidBackground::SolidBackground(glm::vec3 aColor)
{
	mColor = aColor;
}
glm::vec3 SolidBackground::getColor(glm::vec3 aRayDirection)
{
	return mColor;
}

