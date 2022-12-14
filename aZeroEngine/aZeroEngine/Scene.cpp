#include "Scene.h"

Scene::~Scene()
{
	for (auto& [key, value] : entities)
	{
		ecs->ObliterateEntity(value);
	}
}
