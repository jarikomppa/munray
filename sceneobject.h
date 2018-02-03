#pragma once
#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

class Box;
class Sphere;
class Plane;
class Light;

class SceneObject
{
public:
	int mSortkey;
	SceneObject *mNext;
	bool mLight;
	bool mDynamic;
	glm::vec3 mPosition;
	Material *mMaterial;
	char *mName;
	virtual bool intersect(const glm::vec3 &aRayOrigin, const glm::vec3 &aRayDirection, const glm::vec3 &aRayDirectionInv, float &aCollisionDistance) const = 0;
	virtual void getNormal(glm::vec3 &aCollisionPoint, glm::vec3 &aNormal) = 0;
	virtual void getBoundingSphere(glm::vec3 &aCenter, float &aRadius) const = 0;
	virtual void getBoundingBox(glm::vec3 &aMin, glm::vec3 &aMax) const = 0;
	virtual glm::vec3 getTextureCoord(glm::vec3 &aCollisioPoint) = 0;
	SceneObject() : mNext(0), mMaterial(0), mName(0), mLight(false), mDynamic(false) { static int sk = 0; sk++; mSortkey = sk; };
	virtual Box *getAsBox() { return 0; }
	virtual Sphere *getAsSphere() { return 0; }
	virtual Plane *getAsPlane() { return 0; }
	virtual Light *getAsLight() { return 0; }
};

#endif