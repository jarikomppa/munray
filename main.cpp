#include "munray.h"
#include "json.h"
#include <imgui.h>
#include "imgui_impl_sdl.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tinyfiledialogs.h"

int tex_frame = 0;

Scene *gScene;

class RayWorker : public Thread::PoolTask
{
public:
	int mScanline;
	int *mFramebuffer;
	virtual void work()
	{
		render(gScene, mFramebuffer, 640, 400, 640, 640.0/400.0, mScanline);
	}
};

Thread::Pool pool;
int gFramebuffer[1024 * 1024];

char *mystrdup(const char *src);
void reoptimize();

SceneObject **gObject = 0;
int *gObjectOrder = 0;
int gObjectCount = 0;
int gMaterialCount = 0;
char **gMaterialName = 0;
Material **gMaterial = 0;



int getObjectCount(Scene::Node *n)
{
	int c = 0;
	if (!n) return 0;
	if (n->mSceneObject)
	{
		SceneObject *walker = n->mSceneObject;
		while (walker)
		{
			c++;
			walker = walker->mNext;
		}
	}
	if (n->mLeft)
		c += getObjectCount(n->mLeft);
	if (n->mRight)
		c += getObjectCount(n->mRight);
	if (n->mNext)
		c += getObjectCount(n->mNext);
	return c;
}

void gatherObjects(Scene::Node *n, int &index)
{
	if (!n) return;
	if (n->mSceneObject)
	{
		SceneObject *walker = n->mSceneObject;
		while (walker)
		{
			gObject[index] = walker;
			index++;
			walker = walker->mNext;
		}
	}
	if (n->mLeft)
		gatherObjects(n->mLeft, index);
	if (n->mRight)
		gatherObjects(n->mRight, index);
	if (n->mNext)
		gatherObjects(n->mNext, index);
}

int sortobj(const void * a, const void *b)
{
	int ai = *(int*)a;
	int bi = *(int*)b;
	int av = (int)gObject[ai]->mSortkey;
	int bv = (int)gObject[bi]->mSortkey;
	return bv - av;
}

void updateSceneLists()
{
	gObjectCount = getObjectCount(gScene->mRoot);
	delete[] gObject;
	gObject = new SceneObject*[gObjectCount];
	delete[] gObjectOrder;
	gObjectOrder = new int[gObjectCount];
	
	int i;
	for (i = 0; i < gObjectCount; i++)
		gObjectOrder[i] = i;
	i = 0;

	gatherObjects(gScene->mRoot, i);

	reoptimize();

	qsort(gObjectOrder, gObjectCount, sizeof(int), sortobj);

	Material *walker = gScene->mMaterial;
	gMaterialCount = 0;
	while (walker)
	{
		gMaterialCount++;
		walker = walker->mNext;
	}
	delete[] gMaterial;
	delete[] gMaterialName;
	gMaterialName = new char*[gMaterialCount];
	gMaterial = new Material*[gMaterialCount];
	walker = gScene->mMaterial;
	gMaterialCount = 0;
	while (walker)
	{
		gMaterial[gMaterialCount] = walker;
		gMaterialName[gMaterialCount] = walker->mName;
		gMaterialCount++;
		walker = walker->mNext;
	}
}

int materialIndex(Material *m)
{
	for (int i = 0; i < gMaterialCount; i++)
		if (gMaterial[i] == m)
			return i;
	return 0;
}


void teardown(Scene::Node *node)
{
	if (node->mLeft)
		teardown(node->mLeft);
	if (node->mRight)
		teardown(node->mRight);
	if (node->mNext)
		teardown(node->mNext);
	delete node;
}

void deoptimize(SceneObject *aObjectToRemove = 0)
{
	int i;
	if (!gScene->mRoot)
		return;
	gScene->mRoot->mSceneObject = 0;
	teardown(gScene->mRoot);
	gScene->mRoot = 0;
	for (i = 0; i < gObjectCount; i++)
	{
		if (gObject[i] != aObjectToRemove)
			gScene->insert(gObject[i]);
	}
}

void reoptimize()
{
	deoptimize();
	gScene->optimize();
}

void saveScene(char *aFilename)
{
	FILE * f = fopen(aFilename, "wb");
	if (!f) return;
	bool first = true;
	fprintf(f, "{\n");
	{
		Material *walker = gScene->mMaterial;
		while (walker)
		{
			if (!first)
				fprintf(f, ",\n");
			fprintf(f, "\t\"material\":\n");
			fprintf(f, "\t{\n");
			fprintf(f, "\t\t\"name\":\"%s\",\n", walker->mName);
			fprintf(f, "\t\t\"opacity\":%3.3f,\n", walker->mOpacity);
			fprintf(f, "\t\t\"reflection\":%3.3f,\n", walker->mReflection);
			fprintf(f, "\t\t\"ambient\":[%3.3f,%3.3f,%3.3f],\n", walker->mAmbient.x, walker->mAmbient.y, walker->mAmbient.z);
			fprintf(f, "\t\t\"diffuse\":[%3.3f,%3.3f,%3.3f],\n", walker->mDiffuse.x, walker->mDiffuse.y, walker->mDiffuse.z);
			fprintf(f, "\t\t\"specular\":[%3.3f,%3.3f,%3.3f]\n", walker->mSpecular.x, walker->mSpecular.y, walker->mSpecular.z);
			fprintf(f, "\t}"); 
			first = false;
			walker = walker->mNext;
		}
	}
	{
		int i;
		for (i = 0; i < gObjectCount; i++)
		{
			fprintf(f, ",\n");

			SceneObject *ob = gObject[i];

			Sphere *sp = ob->getAsSphere();
			Plane *pl = ob->getAsPlane();
			Box *bx = ob->getAsBox();
			if (sp) fprintf(f, "\t\"sphere\":\n");
			if (pl) fprintf(f, "\t\"plane\":\n");
			if (bx) fprintf(f, "\t\"box\":\n");

			fprintf(f, "\t{\n");

			fprintf(f, "\t\t\"name\":\"%s\",\n", ob->mName);
			fprintf(f, "\t\t\"material\":\"%s\",\n", ob->mMaterial->mName);
			if (ob->mDynamic)
			{
				fprintf(f, "\t\t\"dynamic\":\"1\",\n");
			}

			fprintf(f, "\t\t\"position\":[%3.3f,%3.3f,%3.3f],\n", ob->mPosition.x, ob->mPosition.y, ob->mPosition.z);

			if (sp)
			{
				fprintf(f, "\t\t\"radius\":%3.3f\n", sp->mRadius);
			}

			if (pl)
			{
				fprintf(f, "\t\t\"normal\":[%3.3f,%3.3f,%3.3f]\n", pl->mNormal.x, pl->mNormal.y, pl->mNormal.z);
			}

			if (bx)
			{
				fprintf(f, "\t\t\"size\":[%3.3f,%3.3f,%3.3f]\n", bx->mSize.x, bx->mSize.y, bx->mSize.z);
			}

			fprintf(f, "\t}");
		}
	}

	{
		SceneObject *walker = gScene->mLight;
		while (walker)
		{
			fprintf(f, ",\n");

			SceneObject *ob = walker;

			fprintf(f, "\t\"light\":\n");
			fprintf(f, "\t{\n");

			fprintf(f, "\t\t\"name\":\"%s\",\n", ob->mName);
			fprintf(f, "\t\t\"material\":\"%s\",\n", ob->mMaterial->mName);
			fprintf(f, "\t\t\"position\":[%3.3f,%3.3f,%3.3f]\n", ob->mPosition.x, ob->mPosition.y, ob->mPosition.z);

			fprintf(f, "\t}");
			walker = walker->mNext;
		}
	}
	fprintf(f, "\n}\n");
	fclose(f);
}

float r1()
{
	return (rand() % 8192) / 8192.0f;
}

void generateScene()
{
#if 0
	int i,j,c;
	delete gScene;
	gScene = new Scene;
	setupScene(gScene);
	int count = 8 * 8;
	Material **m = new Material*[count];
	for (i = 0; i < count; i++)
	{
		char *t = new char[16];
		sprintf(t, "mat%03d", i);
		m[i] = new Material(t,
			glm::vec3(0),
			glm::vec3(r1()*0.5 + 0.75, r1()*0.5 + 0.75, r1()*0.5 + 0.75),
			glm::vec3(0),
			0.0f,
			1.0f);
		m[i]->mNext = gScene->mMaterial;
		gScene->mMaterial = m[i];
	}


	for (i = 0, c = 0; i < 8; i++)
		for (j = 0; j < 8; j++, c++)
	{
		float h = r1() * 4 + 1;
		glm::vec3 pos((i-3.5) * 4, h, (j-3.5) * 4);
		char *t = new char[16];
		sprintf(t, "box%03d", c);
		gScene->insert(new Box(t,
			pos,
			glm::vec3(1+r1(),h,1+r1()),
			m[c]));
	}
	gScene->insert(new Light("l1", glm::vec3(0, 64, 0), gScene->getMaterialByName("default")));
	gScene->insert(new Light("l1", glm::vec3(60, 0, 60), gScene->getMaterialByName("default")));
	gScene->insert(new Light("l1", glm::vec3(-60, 0, -60), gScene->getMaterialByName("default")));
#endif
#if 0
	int i;
	int count = 100;
	delete gScene;
	gScene = new Scene;
	setupScene(gScene);

	Material **m = new Material*[count];
	for (i = 0; i < count; i++)
	{
		char *t = new char[16];
		sprintf(t, "mat%03d", i);
		m[i] = new Material(t,
			glm::vec3(0),
			glm::vec3(r1()*0.5 + 0.75, r1()*0.5 + 0.75, r1()*0.5 + 0.75),
			glm::vec3(0),
			0.0f,
			1.0f);
		m[i]->mNext = gScene->mMaterial;
		gScene->mMaterial = m[i];
	}

	for (i = 0; i < count; i++)
	{
		glm::vec3 pos(sin(i*3.14 / (count * 0.5)), sin(i*3.14 / 20.0) * 0.5, cos(i*3.14 / (count * 0.5)));
		pos = glm::normalize(pos);
		pos *= (cos(i*3.14 / 50.0) + 1) * 10 + 15;
		char *t = new char[16];
		sprintf(t, "sphere%03d", i);
		gScene->insert(new Sphere(t,
			pos,
			r1() * 1 + 0.5f,m[i]));
	}
	gScene->insert(new Light("l1", glm::vec3(0, -64, 0), gScene->getMaterialByName("default")));
#endif
	int i;
	int count = 100;
	delete gScene;
	gScene = new Scene;
	setupScene(gScene);

	Material **m = new Material*[count];
	for (i = 0; i < count; i++)
	{
		char *t = new char[16];
		sprintf(t, "mat%03d", i);
		m[i] = new Material(t,
			glm::vec3(0),
			glm::vec3(r1()*0.5 + 0.75, r1()*0.5 + 0.75, r1()*0.5 + 0.75),
			glm::vec3(0),
			0.0f,
			1.0f);
		m[i]->mNext = gScene->mMaterial;
		gScene->mMaterial = m[i];
	}

	for (i = 0; i < count; i++)
	{
		glm::vec3 pos(r1()-0.5,r1()-0.5,r1()-0.5);
		pos = glm::normalize(pos);
		pos *= (r1()-0.5)*100;
		char *t = new char[16];
		sprintf(t, "sphere%03d", i);
		gScene->insert(new Sphere(t,
			pos,
			r1() * 1 + 0.5f, m[i]));
	}
	gScene->insert(new Light("l1", glm::vec3(0, -64, 0), gScene->getMaterialByName("default")));
}

int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

	gScene = loadScene("data/test.json");

	updateSceneLists();

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window *window = SDL_CreateWindow("MunRay v0.0001", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    // Setup ImGui binding
    ImGui_ImplSdlGL2_Init(window);

	glGenTextures(1, (GLuint*)&tex_frame);
	glBindTexture(GL_TEXTURE_2D, tex_frame);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    bool show_test_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	gCameraEye = glm::vec3(0, 10, 50);


    // Main loop
    bool done = false;
	bool animate = false;
	float animateradius = 50;
    while (!done)
    {
		bool dirty = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdlGL2_NewFrame(window);

        {
            ImGui::Begin("Render",0,ImGuiWindowFlags_NoResize);
			ImGui::Image((ImTextureID)tex_frame, ImVec2(640, 400), ImVec2(0,0), ImVec2(640/1024.0,400/1024.0));
			if (ImGui::Button("Reset"))
			{
				delete gScene;
				gScene = new Scene;
				dirty = true;
				setupScene(gScene);
			}
			ImGui::SameLine();
			if (ImGui::Button("Generate"))
			{
				dirty = true;
				generateScene();
			}
			ImGui::SameLine();
			if (ImGui::Button("Load"))
			{
				char const * fn = tinyfd_openFileDialog(
					NULL, /* NULL or "" */
					NULL, /* NULL or "" */
					0, /* 0 */
					NULL, /* NULL {"*.jpg","*.png"} */
					NULL, /* NULL | "image files" */
					0); /* 0 or 1 */
				if (fn)
				{
					Scene *newscene = loadScene((char*)fn);
					if (newscene)
					{
						delete gScene;
						gScene = newscene;
						dirty = true;
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				char const * fn = tinyfd_saveFileDialog(
					NULL, /* NULL or "" */
					NULL, /* NULL or "" */
					0, /* 0 */
					NULL, /* NULL | {"*.jpg","*.png"} */
					NULL); /* NULL | "text files" */
				
				if (fn)
				{
					saveScene((char*)fn);
				}
			}
			ImGui::Text("%d objects", gObjectCount);
			ImGui::Separator();
			ImGui::Checkbox("Animate", &animate);
			ImGui::DragFloat("Animate radius", &animateradius);
			ImGui::DragFloat3("Camera eye", (float*)&gCameraEye);
			ImGui::DragFloat3("Camera at", (float*)&gCameraAt);
			ImGui::DragFloat3("Camera up", (float*)&gCameraUp);
			ImGui::End();
        }

        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

		{
			ImGui::Begin("Objects");
			if (ImGui::Button("Spere"))
			{
				Sphere *s = new Sphere(mystrdup("New sphere"), glm::vec3(0), 5, gScene->getMaterialByName("default"));
				gScene->insert(s);
				dirty = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Box"))
			{
				Box *b = new Box(mystrdup("New box"), glm::vec3(0), glm::vec3(5), gScene->getMaterialByName("default"));
				gScene->insert(b);
				dirty = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Plane"))
			{
				Plane *p = new Plane(mystrdup("New plane"), glm::vec3(0), glm::vec3(0,1,0), gScene->getMaterialByName("default"));
				gScene->insert(p);
				dirty = true;
			}

			SceneObject *doomed = 0, *walker;
			int i;
			for (i = 0; i < gObjectCount; i++)
			{
				walker = gObject[gObjectOrder[i]];
				ImGui::PushID(i);
				ImGui::Separator();
				ImGui::InputText("Name", walker->mName, 63);
				ImGui::Checkbox("Dynamic", &walker->mDynamic);
				if (ImGui::DragFloat3("Position", (float*)&walker->mPosition))
				{
					Box *bx = walker->getAsBox();
					if (bx)
					{
						bx->mMin = bx->mPosition - bx->mSize;
						bx->mMax = bx->mPosition + bx->mSize;
					}
					dirty = true;
				}
				int m = materialIndex(walker->mMaterial);
				if (ImGui::Combo("Material", &m, gMaterialName, gMaterialCount))
					walker->mMaterial = gMaterial[m];

				Sphere *sp = walker->getAsSphere();
				if (sp)
				{
					if (ImGui::DragFloat("Radius", &sp->mRadius))
					{
						dirty = true;
						sp->mRadius2 = sp->mRadius * sp->mRadius;
					}
				}

				Plane *pl = walker->getAsPlane();
				if (pl)
				{
					if (ImGui::DragFloat3("Normal", (float*)&pl->mNormal))
					{
						dirty = true;
					}
				}

				Box *bx = walker->getAsBox();
				if (bx)
				{
					if (ImGui::DragFloat3("Size", (float*)&bx->mSize))
					{
						bx->mMin = bx->mPosition - bx->mSize;
						bx->mMax = bx->mPosition + bx->mSize;
						dirty = true;
					}
				}

				if (ImGui::Button("Delete object"))
				{
					dirty = true;
					doomed = walker;
				}
				ImGui::PopID();
			}
			if (doomed)
			{
				deoptimize(doomed);
				delete doomed;
			}
				
			ImGui::End();
		}

		{
			ImGui::Begin("Lights");
			if (ImGui::Button("New light"))
			{
				Light *l = new Light(mystrdup("New light"), glm::vec3(0), gScene->getMaterialByName("default"));
				l->mNext = gScene->mLight;
				gScene->mLight = l;
				dirty = true;
			}
			SceneObject *walker = gScene->mLight;
			SceneObject *doomed = 0, *last = walker;
			int i = 0;
			while (walker)
			{
				ImGui::PushID(i);
				ImGui::Separator();
				ImGui::InputText("Name", walker->mName, 63);
				ImGui::DragFloat3("Position", (float*)&walker->mPosition);
				int m = materialIndex(walker->mMaterial);
				
				ImGui::Combo("Material", &m, gMaterialName, gMaterialCount);
				walker->mMaterial = gMaterial[m];
				if (ImGui::Button("Delete light"))
				{
					dirty = true;
					doomed = walker;
					if (walker == gScene->mLight)
					{
						gScene->mLight = walker->mNext;
					}
					else
					{
						last->mNext = walker->mNext;
					}
				}
				ImGui::PopID();
				last = walker;
				walker = walker->mNext;
				i++;
			}
			if (doomed)
				delete doomed;
			ImGui::End();
		}

		{
			ImGui::Begin("Materials", 0);
			Material *walker = gScene->mMaterial;
			int i = 0;
			if (ImGui::Button("Add new material"))
			{
				Material *m = new Material();
				m->mName = mystrdup("New material");
				m->mNext = gScene->mMaterial;
				gScene->mMaterial = m;
				dirty = true;
			}
			Material *last = walker;
			Material *doomed = 0;
			while (walker)
			{
				i++;
				ImGui::Separator();
				ImGui::PushID(i);
				if (ImGui::InputText("Name", walker->mName, 63, (strcmp(walker->mName, "default") == 0 ? ImGuiInputTextFlags_ReadOnly:0))) dirty = true;				
				ImGui::DragFloat("Opacity", &walker->mOpacity,0.01,0,1);
				ImGui::DragFloat("Reflection", &walker->mReflection, 0.01, 0, 1);
				ImGui::ColorEdit3("Ambient", (float*)&walker->mAmbient);
				ImGui::ColorEdit3("Diffuse", (float*)&walker->mDiffuse);
				ImGui::ColorEdit3("Specular", (float*)&walker->mSpecular);				
				
				if (strcmp(walker->mName, "default") != 0 && ImGui::Button("Delete material"))
				{
					dirty = true;
					if (walker == gScene->mMaterial)
					{
						// This is teh firsth
						gScene->mMaterial = gScene->mMaterial->mNext;
						doomed = walker;
					}
					else
					{
						last->mNext = walker->mNext;
						doomed = walker;
					}
				}
				last = walker;
				ImGui::PopID();
				
				walker = walker->mNext;
			}


			if (doomed)
			{
				// Scan through everything and move the material to default
				int i;
				for (i = 0; i < gObjectCount; i++)
				{
					if (gObject[i]->mMaterial == doomed)
					{
						gObject[i]->mMaterial = gScene->getMaterialByName("default");
					}
				}
				SceneObject *walker = gScene->mLight;
				while (walker)
				{
					if (walker->mMaterial == doomed)
					{
						walker->mMaterial = gScene->getMaterialByName("default");
					}
					walker = walker->mNext;
				}
				delete doomed;
			}
			ImGui::End();
		}

		int tick = SDL_GetTicks();
		if (animate)
		{
			gCameraEye = glm::vec3(sin(tick * 0.00153) * animateradius, sin(tick * 0.001) * 10, cos(tick * 0.00153) * animateradius);
		}
		glm::vec3 camfwd = gCameraAt - gCameraEye;
		camfwd = glm::normalize(camfwd);
		glm::vec3 camside = glm::cross(gCameraUp, camfwd);
		glm::vec3 camup = glm::cross(camside, camfwd);
		camside = glm::normalize(camside);
		camup = glm::normalize(camup);
		gCameraMatrix = glm::mat3(camside, camup, camfwd);
		if (pool.mRunning == 0)
		{
			pool.init(8);
		}
		RayWorker work[400];
		for (int i = 0; i < 400; i++)
		{
			//render(gScene, (int*)gScreen->pixels, i);
			work[i].mScanline = i;
			work[i].mFramebuffer = gFramebuffer;
			pool.addWork(&work[i]);
		}
		pool.waitUntilDone();

		glBindTexture(GL_TEXTURE_2D, tex_frame);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 400, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)gFramebuffer);


        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui::Render();
        SDL_GL_SwapWindow(window);

		if (dirty)
			updateSceneLists();
    }

    // Cleanup
    ImGui_ImplSdlGL2_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
