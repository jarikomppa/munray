#include "munray.h"

TextureCylinderBackground::TextureCylinderBackground(char * aFilename)
{
	mTexture.load(aFilename);
}

glm::vec3 TextureCylinderBackground::getColor(glm::vec3 aRayDirection)
{
	float fy = 1 - (((aRayDirection.y) + 1) / 2);
	float fx = (float)((atan2(aRayDirection.z, aRayDirection.x) / M_PI / 2) + 1);
	return mTexture.samplePoint(glm::vec3(fx, fy, 0));
}
