#pragma once
#ifndef TEXTURECYLINDERBACKGROUND_H
#define TEXTURECYLINDERBACKGROUND_H

class TextureCylinderBackground : public Background
{
public:
	Texture mTexture;
	TextureCylinderBackground(char * aFilename);
	virtual glm::vec3 getColor(glm::vec3 aRayDirection);
};

#endif