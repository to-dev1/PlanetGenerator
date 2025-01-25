#include "Physics.h"

void PhysicsObject::update(float deltaTime, std::vector<PhysicsObject*>& objects)
{
	//Velocity
	position += velocity * deltaTime;


}

World::World(const World& w)
{
	for (auto it : w.objects)
	{
		objects.push_back(it->copy());
	}
}

World& World::operator=(const World& w)
{
	if (this != &w)
	{
		for (auto it : objects)
		{
			delete it;
		}

		for (auto it : w.objects)
		{
			objects.push_back(it->copy());
		}
	}
}

World::~World()
{
	for (auto it : objects)
	{
		delete it;
	}
}

void World::update(float deltaTime)
{
	const int count = objects.size();

	for (int i = 0; i < count; i++)
	{
		PhysicsObject* obj = objects[i];
		obj->update(deltaTime, objects);
	}

	int index = 0;
	for (int i = 0; i < count; i++)
	{
		PhysicsObject* obj = objects[index];
		if (obj->destroyed)
		{
			delete obj;
			index--;
		}

		index++;
	}
}

void World::render(unsigned int* pixels, int width, int height) const
{
	const int count = objects.size();

	for (int i = 0; i < count; i++)
	{
		const PhysicsObject* obj = objects[i];
		obj->render(pixels, width, height);
	}
}