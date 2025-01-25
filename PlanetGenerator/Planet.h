#pragma once

#include <cmath>

#include "Utility.h"
#include "CellGrid.h"
#include "Physics.h"

class Planet : public PhysicsObject
{
public:
	int radius; //Planet radius in pixels
	int diameter;
	CellGrid<int> planetCells;
	CellGrid<int> heightMap;
	CellGrid<float> elevation;
	CellGrid<Color> mineral;
	CellGrid<float> lightMap;
	CellGrid<float> humidity;
	CellGrid<float> temperature;
	CellGrid<float> water;
	CellGrid<float> habitability;
	CellGrid<Color> flora;
	CellGrid<Color> colors;

	Planet(int r = 100, const Vector2& pos = Vector2(), const Vector2& vel = Vector2()) : PhysicsObject(pos, vel, r * r), radius(r), diameter(r * 2), planetCells(diameter, diameter), heightMap(diameter, diameter), elevation(diameter, diameter), mineral(diameter, diameter), lightMap(diameter, diameter), humidity(diameter, diameter), temperature(diameter, diameter), water(diameter, diameter), habitability(diameter, diameter), flora(diameter, diameter), colors(diameter, diameter)
	{
		//Initialize planet cells to create circle shape
		for (int y = 0; y < planetCells.height; y++)
		{
			for (int x = 0; x < planetCells.width; x++)
			{
				int rx = x - r;
				int ry = y - r;

				if (std::sqrt(rx * rx + ry * ry) >= r)
				{
					planetCells.set(x, y, 1);
				}
			}
		}
	}

	void draw(int x, int y, CellGrid<int>& pixels);
	void draw(int x, int y, unsigned int* pixels, int width, int height) const;

	virtual void render(unsigned int* pixels, int width, int height) const override;
};

class Generator
{
public:
	float atmosphereThickness;
	float waterLevel;
	float humidityMultiplier;
	float lightMultiplier;

	Generator(float a = 1.0f, float w = 0.0f, float h = 0.0f, float l = 1.0f) : atmosphereThickness(a), waterLevel(l), humidityMultiplier(h), lightMultiplier(l), seed(1) {};
	Generator(int s);

	void generate(Planet& planet);

private:
	int seed;
};