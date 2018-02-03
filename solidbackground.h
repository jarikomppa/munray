#pragma once
#ifndef SOLIDBACKGROUND_H
#define SOLIDBACKGROUND_H

class SolidBackground : public Background
{
public:
	glm::vec3 mColor;
	SolidBackground(glm::vec3 aColor);
	virtual glm::vec3 getColor(glm::vec3 aRayDirection);
};

#endif