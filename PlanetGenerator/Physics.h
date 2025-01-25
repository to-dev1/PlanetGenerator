#pragma once

#include <vector>

#include "Utility.h"

class PhysicsObject
{
public:
	bool destroyed;
	Vector2 position;
	Vector2 velocity;
	float mass;

	PhysicsObject(const Vector2& pos = Vector2(), const Vector2& vel = Vector2(), float m = 1.0f) : destroyed(false), position(pos), velocity(vel), mass(m) {};

	void update(float deltaTime, std::vector<PhysicsObject*>& objects);

	virtual PhysicsObject* copy()
	{
		return new PhysicsObject(*this);
	}

	virtual void render(unsigned int* pixels, int width, int height) const {};
};

class World
{
public:
	std::vector<PhysicsObject*> objects;

	World() {};
	World(const World& w);
	World& operator=(const World& w);
	~World();

	void update(float deltaTime);

	void render(unsigned int* pixels, int width, int height) const;
};