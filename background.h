#pragma once
#ifndef BACKGROUND_H
#define BACKGROUND_H

class Background
{
public:
	virtual glm::vec3 getColor(glm::vec3 aRayDirection) = 0;
};

#endif