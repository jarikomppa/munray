#include "munray.h"

GradientBackground::GradientBackground(glm::vec3 aColor1, glm::vec3 aColor2)
{
	mColor1 = aColor1;
	mColor2 = aColor2;
}

glm::vec3 GradientBackground::getColor(glm::vec3 aRayDirection)
{
	return mColor1 + (mColor2 - mColor1) * (aRayDirection.y + 1) * 0.5f;
}
