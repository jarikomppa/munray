#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

class Texture
{
public:
	unsigned int *mImgData;
	int mWidth;
	int mHeight;
	Texture();
	~Texture();
	void load(char *aFilename);
	glm::vec3 samplePoint(glm::vec3 aUV);
	glm::vec3 sampleBilinear(glm::vec3 aUV);
};

#endif