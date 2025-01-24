#pragma once

#include <cmath>

#include "Utility.h"
#include "CellGrid.h"

class Planet
{
public:
	int radius; //Planet radius in pixels
	int diameter;
	CellGrid<int> planetCells;
	CellGrid<int> heightMap;
	CellGrid<int> lightMap;
	CellGrid<int> humidity;
	CellGrid<Color> colors;

	Planet(int r = 100) : radius(r), diameter(r * 2), planetCells(diameter, diameter), heightMap(diameter, diameter), lightMap(diameter, diameter), humidity(diameter, diameter), colors(diameter, diameter)
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
	void draw(int x, int y, unsigned int* pixels, int width, int height);
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