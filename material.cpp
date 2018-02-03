#include "munray.h"

glm::vec3 Material::getDiffuse(glm::vec3 aTexcoord)
{
	if (mDiffuseTexture.mImgData)
	{
		return mDiffuseTexture.sampleBilinear(aTexcoord * mTextureScale + mTextureTranslate) * mDiffuse;
	}
	return mDiffuse;
}

Material::Material()
{
	mAmbient = glm::vec3(0);
	mDiffuse = glm::vec3(1);
	mSpecular = glm::vec3(0);
	mOpacity = 1;
	mReflection = 0;
	mTextureScale = glm::vec3(1);
	mTextureTranslate = glm::vec3(0);
	mNext = 0;
	mName = "";
}

Material::Material(char *aName, glm::vec3 aAmbient, glm::vec3 aDiffuse, glm::vec3 aSpecular, float aReflection, float aOpacity)
{
	mAmbient = aAmbient;
	mDiffuse = aDiffuse;
	mSpecular = aSpecular;
	mOpacity = aOpacity;
	mReflection = aReflection;
	mTextureScale = glm::vec3(1);
	mTextureTranslate = glm::vec3(0);
	mNext = 0;
	mName = aName;
}
