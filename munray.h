#pragma once
#ifndef MUNRAY_H
#define MUNRAY_H

#include "stfu.h"
#include <math.h>
#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "threadpool.h"
#include "scene.h"
#include "texture.h"
#include "material.h"
#include "sceneobject.h"
#include "light.h"
#include "sphere.h"
#include "plane.h"
#include "box.h"
#include "background.h"
#include "solidbackground.h"
#include "gradientbackground.h"
#include "texturecylinderbackground.h"

extern glm::vec3 gCameraEye;
extern glm::vec3 gCameraAt;
extern glm::vec3 gCameraUp;
extern glm::mat3 gCameraMatrix;

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif


#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#define M_PI 3.141592653589793
//#define INFINITY 1e8
#define MAX_RAY_DEPTH 5
// Trace one scanline.
void render(Scene *aScene, int *aPixels, int aWidth, int aHeight, int aPitch, float aAspect, int aScanline);

// Load scene from json file
Scene *loadScene(char *aFilename);
void setupScene(Scene *scene);


//#define SCREENIE

#ifdef SCREENIE
#define WIDTH 192*8
#define HEIGHT 108*8
#else
// Screen width
#define WIDTH 160//192*2//160
// Screen height
#define HEIGHT 100//108*2//100
//#define PIXEL_DOUBLING
#define PIXEL_QUADING
#endif



#endif