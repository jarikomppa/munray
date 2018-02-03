#include "munray.h"


float mix(const float &a, const float &b, const float &mix)
{
	return b * mix + a * (1 - mix);
}

void findIntersection(Scene::Node *aNode, glm::vec3 &aRayOrigin, glm::vec3 &aRayDirection, glm::vec3 &aRayDirectionInv, float &aCollisionDistance, SceneObject **aColliderObject)
{
	if (aNode->mSceneObject)
	{
		SceneObject *walker = aNode->mSceneObject;
		while (walker)
		{
			float collision_distance;
			if (walker->intersect(aRayOrigin, aRayDirection, aRayDirectionInv, collision_distance))
			{
				if (collision_distance < aCollisionDistance)
				{
					aCollisionDistance = collision_distance;
					*aColliderObject = walker;
				}
			}
			walker = walker->mNext;
		}
	}

	if (aNode->mLeft && aNode->mLeft->intersect(aRayOrigin, aRayDirection))
		findIntersection(aNode->mLeft, aRayOrigin, aRayDirection, aRayDirectionInv, aCollisionDistance, aColliderObject);
	if (aNode->mRight && aNode->mRight->intersect(aRayOrigin, aRayDirection))
		findIntersection(aNode->mRight, aRayOrigin, aRayDirection, aRayDirectionInv, aCollisionDistance, aColliderObject);
	if (aNode->mNext)
		findIntersection(aNode->mNext, aRayOrigin, aRayDirection, aRayDirectionInv, aCollisionDistance, aColliderObject);
}

glm::vec3 trace(
	glm::vec3 &rayorig,
	glm::vec3 &raydir,
	Scene *scene,
	const int &depth)
{
	float collision_distance = (float)INFINITY;
	SceneObject *collider = NULL;

	glm::vec3 raydirinv = glm::vec3(1) / raydir;

	if (scene->mRoot)
		findIntersection(scene->mRoot, rayorig, raydir, raydirinv, collision_distance, &collider);

	if (!collider)
	{
		if (scene->mBackground)
			return scene->mBackground->getColor(raydir);// glm::vec3(0);
		return glm::vec3(0);
	}

	glm::vec3 surfaceColor(0);

	glm::vec3 hit_point = rayorig + raydir * collision_distance;
	glm::vec3 hit_normal;
	collider->getNormal(hit_point, hit_normal);
	hit_normal = glm::normalize(hit_normal); // normalize normal direction
	float bias = (float)1e-4;

	SceneObject *lightWalker = scene->mLight;
	for (; lightWalker; lightWalker = lightWalker->mNext)
	{
		glm::vec3 transmission(1);
		glm::vec3 lightDirection = lightWalker->mPosition - hit_point;
		glm::vec3 lightDirectionInv = glm::vec3(1) / lightDirection;
		float light_distance = glm::length(lightDirection);
		lightDirection = glm::normalize(lightDirection);


		float collision_distance = (float)INFINITY;
		SceneObject *shadowCollider = 0;
		findIntersection(scene->mRoot, hit_point + hit_normal * bias, lightDirection, lightDirectionInv, collision_distance, &shadowCollider);

		if (shadowCollider && collision_distance < light_distance)
			transmission = glm::vec3(0);

		surfaceColor += collider->mMaterial->getDiffuse(collider->getTextureCoord(hit_point)) * transmission *
			glm::max(float(0), glm::dot(hit_normal, lightDirection)) * lightWalker->mMaterial->mDiffuse;
	}


	if ((collider->mMaterial->mOpacity < 1 || collider->mMaterial->mReflection > 0) && depth < MAX_RAY_DEPTH)
	{
		//float facingratio = -glm::dot(raydir, hit_normal);
		//float fresnel_effect = mix(pow(1 - facingratio, 3), 1, 0.1f);
		glm::vec3 reflect_ray_direction = raydir - hit_normal * 2.0f * glm::dot(raydir, hit_normal);
		reflect_ray_direction = glm::normalize(reflect_ray_direction);
		glm::vec3 reflection = trace(hit_point + hit_normal * bias, reflect_ray_direction, scene, depth + 1);
		surfaceColor += (collider->mMaterial->mReflection /*+ fresnel_effect*/) * reflection * collider->mMaterial->getDiffuse(collider->getTextureCoord(hit_point));
		/*
		glm::vec3 refraction(0);
		if (collider->mOpacity < 1)
		{
		float ior = 1.1f, eta = (inside) ? ior : 1 / ior;
		float cosi = -glm::dot(hit_normal,raydir);
		float k = 1 - eta * eta * (1 - cosi * cosi);
		glm::vec3 refrdir = raydir * eta + hit_normal * (eta *  cosi - sqrt(k));
		refrdir = glm::normalize(refrdir);
		refraction = trace(hit_point - hit_normal * bias, refrdir, scene, depth + 1);
		}
		surfaceColor += (refraction * (1 - fresneleffect) * (1 - collider->mOpacity)) * collider->getColor(hit_point);
		*/
	}

	return surfaceColor;// +collider->mEmissionColor;
}

glm::vec3 gCameraEye(0);
glm::vec3 gCameraAt(0);
glm::vec3 gCameraUp(0.0f, -1.0f, 0.0f);
glm::mat3 gCameraMatrix(0);

// Trace one scanline.
void render(Scene *aScene, int *aPixels, int aWidth, int aHeight, int aPitch, float aAspect, int aScanline)
{
	unsigned width = aWidth, height = aHeight;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 45.0f;
	float aspect = aAspect;//width / float(height);
	float angle = (float)tan(M_PI * 0.5 * fov / 180.);
	unsigned y = aScanline;
	{
		float yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
		for (unsigned x = 0; x < width; ++x)
		{
			float xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
			glm::vec3 raydir(xx, yy, 1);
			raydir = gCameraMatrix * raydir;
			raydir = glm::normalize(raydir);
			glm::vec3 res = trace(gCameraEye, raydir, aScene, 0);
			if (res == glm::vec3(2))
				res = glm::vec3(0);
			unsigned int pix =
				0xff000000 |
				((unsigned int)floor(MIN(1.0f, res.x) * 255) << 16) |
				((unsigned int)floor(MIN(1.0f, res.y) * 255) << 8) |
				((unsigned int)floor(MIN(1.0f, res.z) * 255));
			/*
#ifdef PIXEL_DOUBLING
			*((int*)aPixels + (y * 2 + 0) * aPitch + x * 2 + 0) = pix;
			*((int*)aPixels + (y * 2 + 0) * aPitch + x * 2 + 1) = pix;
			*((int*)aPixels + (y * 2 + 1) * aPitch + x * 2 + 0) = pix;
			*((int*)aPixels + (y * 2 + 1) * aPitch + x * 2 + 1) = pix;
#elif defined(PIXEL_QUADING)
			*((int*)aPixels + (y * 4 + 0) * aPitch + x * 4 + 0) = pix;
			*((int*)aPixels + (y * 4 + 0) * aPitch + x * 4 + 1) = pix;
			*((int*)aPixels + (y * 4 + 0) * aPitch + x * 4 + 2) = pix;
			*((int*)aPixels + (y * 4 + 0) * aPitch + x * 4 + 3) = pix;
			*((int*)aPixels + (y * 4 + 1) * aPitch + x * 4 + 0) = pix;
			*((int*)aPixels + (y * 4 + 1) * aPitch + x * 4 + 1) = pix;
			*((int*)aPixels + (y * 4 + 1) * aPitch + x * 4 + 2) = pix;
			*((int*)aPixels + (y * 4 + 1) * aPitch + x * 4 + 3) = pix;
			*((int*)aPixels + (y * 4 + 2) * aPitch + x * 4 + 0) = pix;
			*((int*)aPixels + (y * 4 + 2) * aPitch + x * 4 + 1) = pix;
			*((int*)aPixels + (y * 4 + 2) * aPitch + x * 4 + 2) = pix;
			*((int*)aPixels + (y * 4 + 2) * aPitch + x * 4 + 3) = pix;
			*((int*)aPixels + (y * 4 + 3) * aPitch + x * 4 + 0) = pix;
			*((int*)aPixels + (y * 4 + 3) * aPitch + x * 4 + 1) = pix;
			*((int*)aPixels + (y * 4 + 3) * aPitch + x * 4 + 2) = pix;
			*((int*)aPixels + (y * 4 + 3) * aPitch + x * 4 + 3) = pix;
#else
			*((int*)aPixels + y * aPitch + x) = pix;
#endif*/
			*((int*)aPixels + y * aPitch + x) = pix;
		}
	}
}
