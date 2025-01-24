#include "Planet.h"

void Planet::draw(int x, int y, CellGrid<int>& pixels)
{
	int lx = std::min(x + diameter, pixels.width);
	int ly = std::min(y + diameter, pixels.height);

	int gy = 0;
	for (int py = y; py < ly; py++)
	{
		int gx = 0;
		for (int px = y; px < lx; px++)
		{
			pixels.set(px, py, heightMap.read(gx, gy));
			gx++;
		}
		gy++;
	}
}

void Planet::draw(int x, int y, unsigned int* pixels, int width, int height)
{
	int lx = std::min(x + diameter, width);
	int ly = std::min(y + diameter, height);

	int gy = 0;
	for (int py = y; py < ly; py++)
	{
		int gx = 0;
		for (int px = y; px < lx; px++)
		{
			int pos = px + py * width;
			if (!planetCells.read(gx, py)) pixels[pos] = colors.read(gx, gy).toInt();
			gx++;
		}
		gy++;
	}
}

Generator::Generator(int s)
{
	//Initialize random generator
	seed = s;
	Random rn = Random(seed);

	atmosphereThickness = (float)(rn.next() % (20) / 10.0f);
	waterLevel = (float)(rn.next() % (10)) / 30.0f + 0.7f;
	humidityMultiplier = (0.5f - (float)(rn.next() % (10)) / 20.0f) + 0.5f;
	lightMultiplier = (float)(rn.next() % (5) / 10.0f) + 0.5f;

	humidityMultiplier *= atmosphereThickness;

	if (humidityMultiplier > 1.0f)
	{
		humidityMultiplier = 1.0f;
	}

}

void Generator::generate(Planet& planet)
{
	Random rn = Random(seed);

	//Height map

	planet.heightMap.call([&](int x, int y) {
		return rn.next() % 4000 == 1;
	}, planet.planetCells);

	int greatestHeight = 0;

	for (int i = 0; i < 1000; i++)
	{
		CellGrid<int> tmp = planet.heightMap;

		planet.heightMap.run([&](int x, int y, int& h) {
			if (h != 0 && rn.next() % 15 == 1) tmp.setAround(x, y, h + 1, tmp);
			if (h >= greatestHeight) greatestHeight = h + 1;
		}, planet.planetCells);

		planet.heightMap.copy(tmp);
	}

	//Light map

	planet.lightMap.call([&](int x, int y) {
		return static_cast<int>(std::sqrt(160000 - std::pow(std::abs(y - planet.radius), 2)));
	}, planet.planetCells);

	//Humidity

	planet.lightMap.call([&](int x, int y) {
		return 360 - std::abs(350 - planet.lightMap.read(x, y));
	}, planet.planetCells);

	//Final color

	planet.colors.call([&](int x, int y){
		return Color(planet.heightMap.read(x, y), 0, 0);
	}, planet.planetCells);
}