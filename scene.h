#pragma once
#ifndef SCENE_H
#define SCENE_H

#define BOUND_WITH_SPHERES

class SceneObject;
class Background;
class Material;

class Scene
{
public:
	class Node
	{
	public:
#ifdef BOUND_WITH_SPHERES
		glm::vec3 mPosition;
		float mRadius;
		float mRadius2;
#else
		glm::vec3 mMin;
		glm::vec3 mMax;
#endif
		Node *mNext;
		Node *mLeft;
		Node *mRight;
		SceneObject *mSceneObject;
		Node();
		bool intersect(
			const glm::vec3 &aRayOrigin,
			const glm::vec3 &aRayDirection) const;
	};

	Node *mRoot;
	SceneObject *mLight;
	Background *mBackground;
	Material *mMaterial;

	Scene();
	void insert(SceneObject *aObject);

#ifdef BOUND_WITH_SPHERES
	void calcCommonBoundingSphere(Node *aNodeA, Node *aNodeB, glm::vec3 &aPosition, float &aRadius);
#else
	void calcCommonBoundingBox(Node *aNodeA, Node *aNodeB, glm::vec3 &aMin, glm::vec3 &aMax);
#endif
	void flatten(Node *aNode);
	void optimize();

	SceneObject *getObjectByName(char *aName, Node *aNode);
	SceneObject *getObjectByName(char *aName);
	SceneObject *getLightByName(char *aName);
	Material *getMaterialByName(char *aName);

};

#endif