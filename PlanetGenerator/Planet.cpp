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
		for (int px = x; px < lx; px++)
		{
			int pos = px + py * width;
			unsigned int col = colors.read(gx, gy).toInt();
			if (!planetCells.read(gx, gy))
			{
				pixels[pos] = col;
			}
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

	float epsilon = 0.0001f;

	//Height map

	int heightCount = 3;
	float elevMulti = 1.0f / static_cast<float>(heightCount);
	for (int j = 0; j < heightCount; j++)
	{
		CellGrid<int> height = CellGrid<int>(planet.diameter, planet.diameter);
		Color mineral = Color(100 + rn.next() % 100, 100 + rn.next() % 100, 100 + rn.next() % 100) * elevMulti;

		height.call([&](int x, int y) {
			return rn.next() % 4000 == 1;
		}, planet.planetCells);

		int greatestHeight = 0;

		for (int i = 0; i < 1000; i++)
		{
			CellGrid<int> tmp = height;

			height.run([&](int x, int y, int& h) {
				if (h != 0 && rn.next() % 15 == 1) tmp.setAround(x, y, h + 1, tmp);
				if (h >= greatestHeight) greatestHeight = h + 1;
			}, planet.planetCells);

			height.copy(tmp);
		}

		planet.elevation.run([&](int x, int y, float& h) {
			float elev = static_cast<float>(height.read(x, y)) / static_cast<float>(greatestHeight);
			h += elevMulti * elev;
			planet.mineral.at(x, y) = planet.mineral.at(x, y) + mineral * elev;
		}, planet.planetCells);
	}

	/*
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
	
	planet.elevation.call([&](int x, int y) {
		return static_cast<float>(planet.heightMap.read(x, y)) / static_cast<float>(greatestHeight);
	}, planet.planetCells);
	*/

	//Light map

	planet.lightMap.call([&](int x, int y) {
		int vy = y - planet.radius;
		return std::sqrt(planet.radius * planet.radius - vy * vy) / static_cast<float>(planet.radius);
	}, planet.planetCells);

	//Humidity

	const float humidityElevation = 0.85f;
	planet.humidity.call([&](int x, int y) {
		float h = planet.lightMap.read(x, y) * (humidityElevation - planet.elevation.read(x, y) * humidityElevation + (1.0f - humidityElevation));
		return h;
	}, planet.planetCells);

	//Water

	float waterElevation = 0.35f;
	planet.water.run([&](int x, int y, float& w) {
		if (planet.elevation.read(x, y) < waterElevation) w = waterElevation - planet.elevation.read(x, y);
		else w = 0.0f;
	}, planet.planetCells);



	//Final color

	Color waterColor = Color(50, 75, 100);
	planet.colors.run([&](int x, int y, Color& col) {
		col = planet.mineral.read(x, y);
		float waterDepth = planet.water.read(x, y);
		if (waterDepth > epsilon)
		{
			//Water
			float waterColMulti = std::min(0.3f + waterDepth * 2.0f, 1.0f);
			col = col * (1.0f - waterColMulti) + waterColor * waterColMulti;
		}
		else
		{

		}
	}, planet.planetCells);

	/*
	planet.colors.call([&](int x, int y){
		return Color(static_cast<int>(planet.water.read(x, y) * 255.0f), 0, 0);
	}, planet.planetCells);
	*/
	/*
	planet.colors.call([&](int x, int y) {
		return planet.mineral.read(x, y);
	}, planet.planetCells);
	*/
}