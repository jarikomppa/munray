#include <string.h>
#include "munray.h"
#include "json.h"

glm::vec3 json_get_vec3(json_stream *json)
{
	glm::vec3 vec;
	json_type type = json_next(json);
	assert(type == JSON_NUMBER);
	vec.x = (float)json_get_number(json);
	type = json_next(json);
	assert(type == JSON_NUMBER);
	vec.y = (float)json_get_number(json);
	type = json_next(json);
	assert(type == JSON_NUMBER);
	vec.z = (float)json_get_number(json);
	type = json_next(json);
	assert(type == JSON_ARRAY_END);
	return vec;
}

char *mystrdup(const char *src)
{
	int l = strlen(src);
	int ml = l + 1;
	if (ml < 64) ml = 64;
	char *s = new char[ml];
	memcpy(s, src, l + 1);
	return s;
}

void setupScene(Scene *scene)
{
	if (scene->getMaterialByName("default") == 0)
	{
		Material *m = new Material();
		m->mName = mystrdup("default");
		m->mNext = scene->mMaterial;
		scene->mMaterial = m;
	}
}

int getJSONString(json_stream *json, char *aField, const char *aOType, char **var)
{
	if (strcmp(aOType, aField) == 0)
	{
		json_type type = json_next(json);
		assert(type == JSON_STRING);
		*var = mystrdup(json_get_string(json, 0));
		return 1;
	}
	return 0;
}

int getJSONVec3(json_stream *json, char *aField, const char *aOType, glm::vec3 &var)
{
	if (strcmp(aOType, aField) == 0)
	{
		json_type type = json_next(json);
		assert(type == JSON_ARRAY);
		var = json_get_vec3(json);
		return 1;
	}
	return 0;
}

int getJSONNumber(json_stream *json, char *aField, const char *aOType, float &var)
{
	if (strcmp(aOType, aField) == 0)
	{
		json_type type = json_next(json);
		assert(type == JSON_NUMBER);
		var = (float)json_get_number(json);
		return 1;
	}
	return 0;
}


Scene *loadScene(char *aFilename)
{
	Scene *t = new Scene;
	json_stream json;
	FILE * f = fopen(aFilename, "rb");

	json_open_stream(&json, f);
	json_type type = json_next(&json);
	assert(type == JSON_OBJECT);
	while (json_peek(&json) != JSON_OBJECT_END && json_peek(&json) != JSON_ERROR)
	{
		type = json_next(&json);
		assert(type == JSON_STRING);
		const char *otype = json_get_string(&json, 0);
		if (strcmp(otype, "material") == 0)
		{
			const char *name = "[untitled]";
			glm::vec3 diffuse(1);
			glm::vec3 specular(0);
			glm::vec3 ambient(0);
			float opacity = 1;
			float reflection = 0;


			type = json_next(&json);
			assert(type == JSON_OBJECT);
			while (json_peek(&json) != JSON_OBJECT_END)
			{
				type = json_next(&json);
				assert(type == JSON_STRING);
				otype = json_get_string(&json, 0);

				if (!getJSONString(&json, "name", otype, (char**)&name))
					if (!getJSONVec3(&json, "diffuse", otype, diffuse))
						if (!getJSONVec3(&json, "ambient", otype, ambient))
							if (!getJSONVec3(&json, "specular", otype, specular))
								if (!getJSONNumber(&json, "opacity", otype, opacity))
									if (!getJSONNumber(&json, "reflection", otype, reflection))
										assert(0 && "error parsing material");
			}
			type = json_next(&json);
			assert(type == JSON_OBJECT_END);
			Material *m = new Material();
			m->mName = (char*)name;
			m->mDiffuse = diffuse;
			m->mAmbient = ambient;
			m->mSpecular = specular;
			m->mOpacity = opacity;
			m->mReflection = reflection;
			m->mNext = t->mMaterial;
			t->mMaterial = m;
		}
		else
			if (strcmp(otype, "box") == 0)
			{
				const char *name = "[untitled]";
				const char *material = "default";
				glm::vec3 center;
				glm::vec3 size;
				float dynamic = 0;

				type = json_next(&json);
				assert(type == JSON_OBJECT);
				while (json_peek(&json) != JSON_OBJECT_END)
				{
					type = json_next(&json);
					assert(type == JSON_STRING);
					otype = json_get_string(&json, 0);
					if (!getJSONString(&json, "name", otype, (char**)&name))
						if (!getJSONString(&json, "material", otype, (char**)&material))
							if (!getJSONNumber(&json, "dynamic", otype, dynamic))
								if (!getJSONVec3(&json, "position", otype, center))
									if (!getJSONVec3(&json, "center", otype, center))
										if (!getJSONVec3(&json, "size", otype, size))
											assert(0 && "error parsing box");
				}
				type = json_next(&json);
				assert(type == JSON_OBJECT_END);
				SceneObject *so;
				t->insert(so = new Box((char*)name, center, size, t->getMaterialByName((char*)material)));
				so->mDynamic = dynamic != 0;
			}
			else
				if (strcmp(otype, "plane") == 0)
				{
					const char *name = "[untitled]";
					const char *material = "default";
					glm::vec3 point;
					glm::vec3 normal;
					float dynamic = 0;

					type = json_next(&json);
					assert(type == JSON_OBJECT);
					while (json_peek(&json) != JSON_OBJECT_END)
					{
						type = json_next(&json);
						assert(type == JSON_STRING);
						otype = json_get_string(&json, 0);
						if (!getJSONString(&json, "name", otype, (char**)&name))
							if (!getJSONString(&json, "material", otype, (char**)&material))
								if (!getJSONNumber(&json, "dynamic", otype, dynamic))
									if (!getJSONVec3(&json, "point", otype, point))
										if (!getJSONVec3(&json, "normal", otype, normal))
											assert(0 && "error parsing box");
					}
					type = json_next(&json);
					assert(type == JSON_OBJECT_END);
					SceneObject *so;
					t->insert(so = new Plane((char*)name, point, normal, t->getMaterialByName((char*)material)));
					so->mDynamic = dynamic != 0;
				}
				else
					if (strcmp(otype, "sphere") == 0)
					{
						const char *name = "[untitled]";
						const char *material = "default";
						glm::vec3 center;
						float radius = 5;
						float dynamic = 0;

						type = json_next(&json);
						assert(type == JSON_OBJECT);
						while (json_peek(&json) != JSON_OBJECT_END)
						{
							type = json_next(&json);
							assert(type == JSON_STRING);
							otype = json_get_string(&json, 0);
							if (!getJSONString(&json, "name", otype, (char**)&name))
								if (!getJSONString(&json, "material", otype, (char**)&material))
									if (!getJSONNumber(&json, "dynamic", otype, dynamic))
										if (!getJSONVec3(&json, "center", otype, center))
											if (!getJSONVec3(&json, "position", otype, center))
												if (!getJSONNumber(&json, "radius", otype, radius))
												assert(0 && "error parsing sphere");
						}
						type = json_next(&json);
						assert(type == JSON_OBJECT_END);
						SceneObject *so;
						t->insert(so = new Sphere((char*)name, center, radius, t->getMaterialByName((char*)material)));
						so->mDynamic = dynamic != 0;
					}
					else
						if (strcmp(otype, "light") == 0)
						{
							const char *name = "[untitled]";
							const char *material = "default";
							glm::vec3 position;

							type = json_next(&json);
							assert(type == JSON_OBJECT);
							while (json_peek(&json) != JSON_OBJECT_END)
							{
								type = json_next(&json);
								assert(type == JSON_STRING);
								otype = json_get_string(&json, 0);
								if (!getJSONString(&json, "name", otype, (char**)&name))
									if (!getJSONString(&json, "material", otype, (char**)&material))
										if (!getJSONVec3(&json, "position", otype, position))
											assert(0 && "error parsing light");
							}
							type = json_next(&json);
							assert(type == JSON_OBJECT_END);
							t->insert(new Light((char*)name, position, t->getMaterialByName((char*)material)));
						}
						else
						{
							assert(0);
						}
	}
	type = json_next(&json);
	if (type == JSON_ERROR)
	{
		const char * err = json_get_error(&json);
		err = err;
	}
	assert(type == JSON_OBJECT_END);
	type = json_next(&json);
	assert(type == JSON_DONE);
	json_close(&json);

	setupScene(t);

	t->optimize();
	return t;
};
