#include "munray.h"
#include "stb_image.h"

Texture::Texture()
{
	mImgData = 0;
}

Texture::~Texture()
{
	if (mImgData)
		stbi_image_free(mImgData);
	mImgData = 0;
}

void Texture::load(char *aFilename)
{
	int comp;
	mImgData = (unsigned int*)stbi_load(aFilename, &mWidth, &mHeight, &comp, 4);
}

glm::vec3 Texture::samplePoint(glm::vec3 aUV)
{
	float fx = aUV.x * mWidth;
	float fy = aUV.y * mHeight;
	int x = (int)floor(fx);
	int y = (int)floor(fy);

	unsigned int c1 = mImgData[((y) % mHeight) * mWidth + ((x) % mWidth)];

	glm::vec3 cv1 = glm::vec3(((c1 >> 0) & 0xff) / 256.0f,
		((c1 >> 8) & 0xff) / 256.0f,
		((c1 >> 16) & 0xff) / 256.0f);
	return cv1;
}

glm::vec3 Texture::sampleBilinear(glm::vec3 aUV)
{
	float fx = aUV.x * mWidth;
	float fy = aUV.y * mHeight;
	int x = (int)floor(fx);
	int y = (int)floor(fy);

	fx = fx - x;
	fy = fy - y;

	float f1 = (1 - fx)*(1 - fy);
	float f2 = fx * (1 - fy);
	float f3 = fx * fy;
	float f4 = (1 - fx) * fy;

	unsigned int c1 = mImgData[((y) % mHeight) * mWidth + ((x) % mWidth)];
	unsigned int c2 = mImgData[((y) % mHeight) * mWidth + ((x + 1) % mWidth)];
	unsigned int c3 = mImgData[((y + 1) % mHeight) * mWidth + ((x + 1) % mWidth)];
	unsigned int c4 = mImgData[((y + 1) % mHeight) * mWidth + ((x) % mWidth)];

	glm::vec3 cv1 = glm::vec3(((c1 >> 0) & 0xff) / 256.0f,
		((c1 >> 8) & 0xff) / 256.0f,
		((c1 >> 16) & 0xff) / 256.0f);
	glm::vec3 cv2 = glm::vec3(((c2 >> 0) & 0xff) / 256.0f,
		((c2 >> 8) & 0xff) / 256.0f,
		((c2 >> 16) & 0xff) / 256.0f);
	glm::vec3 cv3 = glm::vec3(((c3 >> 0) & 0xff) / 256.0f,
		((c3 >> 8) & 0xff) / 256.0f,
		((c3 >> 16) & 0xff) / 256.0f);
	glm::vec3 cv4 = glm::vec3(((c4 >> 0) & 0xff) / 256.0f,
		((c4 >> 8) & 0xff) / 256.0f,
		((c4 >> 16) & 0xff) / 256.0f);
	return cv1*f1 + cv2*f2 + cv3*f3 + cv4*f4;
}
