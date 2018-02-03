#pragma once
#ifndef GRADIENTBACKGROUND_H
#define GRADIENTBACKGROUND_H

class GradientBackground : public Background
{
public:
	glm::vec3 mColor1, mColor2;
	GradientBackground(glm::vec3 aColor1, glm::vec3 aColor2);
	virtual glm::vec3 getColor(glm::vec3 aRayDirection);
};

#endif