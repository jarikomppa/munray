#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

class Material
{
public:
	char *mName;
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;
	glm::vec3 mTextureScale;
	glm::vec3 mTextureTranslate;
	Texture mDiffuseTexture;
	float mOpacity;
	float mReflection;
	glm::vec3 getDiffuse(glm::vec3 aTexcoord);
	Material();
	Material(char *aName, glm::vec3 aAmbient, glm::vec3 aDiffuse, glm::vec3 mSpecular, float mReflection = 0, float mOpacity = 1);
	Material *mNext;
};

#endif