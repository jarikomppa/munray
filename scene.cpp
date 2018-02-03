#include "munray.h"
#include <string.h>

Scene::Node::Node()
{
	mNext = 0;
	mLeft = 0;
	mRight = 0;
	mSceneObject = 0;
#ifdef BOUND_WITH_SPHERES
	mPosition = glm::vec3(0);
	mRadius = (float)INFINITY;
#else
	mMin = mMax = glm::vec3(INFINITY);
#endif
}

bool Scene::Node::intersect(
	const glm::vec3 &aRayOrigin,
	const glm::vec3 &aRayDirection) const
{
#ifdef BOUND_WITH_SPHERES
	glm::vec3 l = mPosition - aRayOrigin;
	float d = glm::length(l);
	if (d <= mRadius) return true;
	float tca = glm::dot(l, aRayDirection);
	if (tca < 0) return false;
	float d2 = glm::dot(l, l) - tca * tca;
	if (d2 > mRadius2) return false;
	return true;
#else
	float t1 = (mMin.x - aRayOrigin.x) / aRayDirection.x;
	float t2 = (mMax.x - aRayOrigin.x) / aRayDirection.x;

	float tmin = glm::min(t1, t2);
	float tmax = glm::max(t1, t2);

	t1 = (mMin.y - aRayOrigin.y) / aRayDirection.y;
	t2 = (mMax.y - aRayOrigin.y) / aRayDirection.y;

	tmin = glm::max(tmin, glm::min(t1, t2));
	tmax = glm::min(tmax, glm::max(t1, t2));

	t1 = (mMin.z - aRayOrigin.z) / aRayDirection.z;
	t2 = (mMax.z - aRayOrigin.z) / aRayDirection.z;

	tmin = glm::max(tmin, glm::min(t1, t2));
	tmax = glm::min(tmax, glm::max(t1, t2));

	return tmax > glm::max(tmin, 0.0f);

#endif
}


Scene::Scene()
{
	mRoot = 0;
	mLight = 0;
	mBackground = 0;
	mMaterial = 0;
}

// Insert objects at root
void Scene::insert(SceneObject *aObject)
{
	if (mRoot == 0)
		mRoot = new Node;
#ifdef BOUND_WITH_SPHERES
	glm::vec3 pos;
	float radius = (float)INFINITY;
	aObject->getBoundingSphere(pos, radius);
#else
	glm::vec3 lmin, lmax;
	aObject->getBoundingBox(lmin, lmax);
#endif
	if (aObject->mLight)
	{
		aObject->mNext = mLight;
		mLight = aObject;
	}
	else
	{
#ifdef BOUND_WITH_SPHERES
		if (radius == INFINITY)
#else
		if (lmin.x == INFINITY)
#endif
		{
			// infinite objects go to root
			aObject->mNext = mRoot->mSceneObject;
			mRoot->mSceneObject = aObject;
		}
		else
		{
			Node *n = new Node;
#ifdef BOUND_WITH_SPHERES
			n->mRadius = radius;
			n->mRadius2 = radius * radius;
			n->mPosition = pos;
#else
			n->mMin = lmin;
			n->mMax = lmax;
#endif
			n->mSceneObject = aObject;
			n->mNext = mRoot->mNext;
			mRoot->mNext = n;
		}
	}
}

#ifdef BOUND_WITH_SPHERES
void Scene::calcCommonBoundingSphere(Node *aNodeA, Node *aNodeB, glm::vec3 &aPosition, float &aRadius)
{
	glm::vec3 d = aNodeA->mPosition - aNodeB->mPosition;
	d = glm::normalize(d);
	glm::vec3 edge_a = aNodeA->mPosition + d * aNodeA->mRadius;
	glm::vec3 edge_b = aNodeB->mPosition - d * aNodeB->mRadius;
	aPosition = (edge_a + edge_b) * 0.5f;
	aRadius = glm::distance(edge_a, edge_b) * 0.5f;
}
#else
void BoundingTree::calcCommonBoundingBox(Node *aNodeA, Node *aNodeB, glm::vec3 &aMin, glm::vec3 &aMax)
{
	aMin.x = glm::min(aNodeA->mMin.x, aNodeB->mMin.x);
	aMax.x = glm::max(aNodeA->mMax.x, aNodeB->mMax.x);
	aMin.y = glm::min(aNodeA->mMin.y, aNodeB->mMin.y);
	aMax.y = glm::max(aNodeA->mMax.y, aNodeB->mMax.y);
	aMin.z = glm::min(aNodeA->mMin.z, aNodeB->mMin.z);
	aMax.z = glm::max(aNodeA->mMax.z, aNodeB->mMax.z);
}
#endif

void Scene::flatten(Node *aNode)
{
	if (aNode->mSceneObject == 0)
	{
		if (aNode->mNext != 0 &&
			aNode->mNext->mSceneObject != 0 && 
			aNode->mNext->mSceneObject->mNext == 0 &&
			aNode->mNext->mNext == 0 &&
			aNode->mNext->mLeft == 0 &&
			aNode->mNext->mRight == 0)
		{
			aNode->mNext->mSceneObject->mNext = aNode->mSceneObject;
			aNode->mSceneObject = aNode->mNext->mSceneObject;
			delete aNode->mNext;
			aNode->mNext = 0;
		}
		if (aNode->mLeft != 0 &&
			aNode->mLeft->mSceneObject != 0 &&
			aNode->mLeft->mSceneObject->mNext == 0 &&
			aNode->mLeft->mNext == 0 &&
			aNode->mLeft->mLeft == 0 &&
			aNode->mLeft->mRight == 0)
		{
			aNode->mLeft->mSceneObject->mNext = aNode->mSceneObject;
			aNode->mSceneObject = aNode->mLeft->mSceneObject;
			delete aNode->mLeft;
			aNode->mLeft = 0;
		}
		if (aNode->mRight != 0 &&
			aNode->mRight->mSceneObject != 0 &&
			aNode->mRight->mSceneObject->mNext == 0 &&
			aNode->mRight->mNext == 0 &&
			aNode->mRight->mLeft == 0 &&
			aNode->mRight->mRight == 0)
		{
			aNode->mRight->mSceneObject->mNext = aNode->mSceneObject;
			aNode->mSceneObject = aNode->mRight->mSceneObject;
			delete aNode->mRight;
			aNode->mRight = 0;
		}
	}

	if (aNode->mNext)
		flatten(aNode->mNext);
	if (aNode->mLeft)
		flatten(aNode->mLeft);
	if (aNode->mRight)
		flatten(aNode->mRight);
}


void Scene::optimize()
{
	if (mRoot == 0)
		return;
	int changed = 1;
	while (changed)
	{
		changed = 0;
		// find two objects that would make the smallest bounding sphere
		float minsize = (float)INFINITY;
#ifdef BOUND_WITH_SPHERES
		glm::vec3 minpos;
#else
		glm::vec3 lmin, lmax;
#endif

		Node *first = 0, *second = 0;
		Node *walker;
		Node *rewalker;

		walker = mRoot;
		while (walker)
		{
			rewalker = walker->mNext;
			while (rewalker)
			{
				if (walker != rewalker)
				{
#ifdef BOUND_WITH_SPHERES
					glm::vec3 c;
					float psize;
					calcCommonBoundingSphere(walker, rewalker, c, psize);
#else
					glm::vec3 pmin, pmax;
					calcCommonBoundingBox(walker, rewalker, pmin, pmax);
					float psize = glm::length(pmax - pmin);
#endif
					if (psize < minsize)
					{
						first = walker;
						second = rewalker;
						minsize = psize;
#ifdef BOUND_WITH_SPHERES
						minpos = c;
#else
						lmin = pmin;
						lmax = pmax;
#endif
					}
				}
				rewalker = rewalker->mNext;
			}
			walker = walker->mNext;
		}

		if (minsize != INFINITY && first->mSceneObject && !first->mSceneObject->mDynamic && second->mSceneObject && !second->mSceneObject->mDynamic)
		{
			// Remove the pair from the list
			walker = mRoot;
			while (walker)
			{
				if (walker->mNext == first)
				{
					walker->mNext = walker->mNext->mNext;
					first->mNext = 0;
				}
				if (walker->mNext == second)
				{
					walker->mNext = walker->mNext->mNext;
					second->mNext = 0;
				}
				walker = walker->mNext;
			}
			Node *n = new Node;
			n->mLeft = first;
			n->mRight = second;
#ifdef BOUND_WITH_SPHERES
			n->mRadius = minsize;
			n->mRadius2 = minsize * minsize;
			n->mPosition = minpos;
#else
			n->mMin = lmin;
			n->mMax = lmax;
#endif
			n->mNext = mRoot->mNext;
			mRoot->mNext = n;
			changed = 1;
		}
	}
//	flatten(mRoot);

}

SceneObject *Scene::getLightByName(char *aName)
{
	SceneObject *walker = mLight;
	while (walker)
	{
		if (stricmp(walker->mName, aName) == 0)
			return walker;
		walker = walker->mNext;
	}
	return 0;
}

SceneObject *Scene::getObjectByName(char *aName)
{
	return getObjectByName(aName, mRoot);
}

SceneObject *Scene::getObjectByName(char *aName, Node *aNode)
{
	if (aNode->mSceneObject)
	{
		SceneObject *walker = aNode->mSceneObject;
		while (walker)
		{
			if (stricmp(walker->mName, aName) == 0)
				return walker;
			walker = walker->mNext;
		}
	}
	SceneObject *n = 0;
	if (aNode->mNext)
		n = getObjectByName(aName, aNode->mNext);
	if (!n && aNode->mLeft)
		n = getObjectByName(aName, aNode->mLeft);
	if (!n && aNode->mRight)
		n = getObjectByName(aName, aNode->mRight);
	return n;
}


Material *Scene::getMaterialByName(char *aName)
{
	Material *walker = mMaterial;
	while (walker)
	{
		if (strcmp(walker->mName, aName) == 0)
			return walker;
		walker = walker->mNext;
	}
	return 0;
}