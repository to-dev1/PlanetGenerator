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

void Planet::draw(int x, int y, unsigned int* pixels, int width, int height) const
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

void Planet::render(unsigned int* pixels, int width, int height) const
{
	int x = static_cast<int>(position.x) - radius;
	int y = static_cast<int>(position.y) - radius;

	draw(x, y, pixels, width, height);
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
		Color flora = Color(50 + rn.next() % 100, 200 + rn.next() % 55, rn.next() % 255) * elevMulti;

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
			planet.flora.at(x, y) = planet.flora.at(x, y) + flora * elev;
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

	const Color starColor = Color(255, 220, 200);
	const float starMulti = 1.0f; //0.5f
	planet.lightMap.call([&](int x, int y) {
		int vy = y - planet.radius;
		return (std::sqrt(planet.radius * planet.radius - vy * vy) / static_cast<float>(planet.radius) * 0.9f + 0.1f) * starMulti;
	}, planet.planetCells);

	//Humidity

	const float atmosphere = 1.0f;
	const float humidityElevation = 0.95f; //0.85f
	planet.humidity.call([&](int x, int y) {
		float solar = planet.lightMap.read(x, y);
		float h = solar * (humidityElevation - planet.elevation.read(x, y) * humidityElevation + (1.0f - humidityElevation));
		if (solar > 1.0f)
		{
			h = h / (solar * solar);
		}
		return h * atmosphere;
	}, planet.planetCells);

	//Temperature

	const float freezingPoint = 1.0f;
	const float boilingPoint = 10.0f;
	float maxTemperature = 0.0f;
	planet.temperature.call([&](int x, int y) {
		float solar = planet.lightMap.read(x, y) * 2.0f;
		float temp = std::pow(solar, 1.0f - 0.4f * (atmosphere * planet.humidity.read(x, y))) * ((1.0f - planet.elevation.read(x, y) * 0.5f) + 0.5f);
		maxTemperature = std::max(temp, maxTemperature);
		return temp;
	}, planet.planetCells);

	//Water

	float waterElevation = 0.35f;
	planet.water.run([&](int x, int y, float& w) {
		float temp = planet.temperature.read(x, y);
		if (planet.elevation.read(x, y) < waterElevation)
		{
			w = waterElevation - planet.elevation.read(x, y);
		}
		else if (temp < 1.0f)
		{
			w = -std::min(planet.humidity.read(x, y) / (temp), 1.0f);
		}
		else
		{
			w = 0.0f;
		}
	}, planet.planetCells);

	//Habitability

	float minHabitability = 0.3f;
	planet.habitability.run([&](int x, int y, float& h) {
		float temp = planet.temperature.read(x, y);
		if (std::abs(planet.water.read(x, y)) < epsilon && maxTemperature < boilingPoint)
		{
			//h = temp * planet.humidity.read(x, y);
			h = planet.humidity.read(x, y);
		}
		else
		{
			h = 0.0f;
		}

		if (h < minHabitability) h = 0.0f;
		else h = std::sqrt(h);

	}, planet.planetCells);

	//Flora
	/*
	planet.flora.run([&](int x, int y, Color& f) {
		float habitability = planet.habitability.read(x, y);
		if (std::abs(planet.water.read(x, y)) < epsilon)
		{
			//w = waterElevation - planet.elevation.read(x, y);
		}
		else
		{
			f = Color();
		}
	}, planet.planetCells);
	*/

	//Final color
	
	Color waterColor = Color(50, 75, 100);
	Color deepWaterColor = Color(20, 30, 60); //Color(10, 20, 40);
	Color iceColor = Color(150, 220, 255);
	Color snowColor = Color(255, 255, 255);
	planet.colors.run([&](int x, int y, Color& col) {
		col = planet.mineral.read(x, y);
		float waterDepth = planet.water.read(x, y);
		if (waterDepth < epsilon)
		{
			//Snow
			float snowColMulti = -waterDepth;
			col = col * (1.0f - snowColMulti) + snowColor * snowColMulti;
		}
		if (waterDepth > epsilon && maxTemperature < boilingPoint)
		{
			//Water
			if (planet.temperature.read(x, y) > freezingPoint)
			{
				//Liquid
				float waterColMulti = std::min(0.3f + waterDepth * 2.0f, 1.0f);
				waterColMulti = std::cbrt(waterColMulti);
				Color depthColor = waterColor * (1.0f - waterColMulti) + deepWaterColor * waterColMulti;
				col = col * (1.0f - waterColMulti) + depthColor * waterColMulti;
			}
			else
			{
				//Ice
				float iceColMulti = 0.8f;
				col = iceColor * iceColMulti * (0.25f + planet.elevation.read(x, y) * 0.75f);
			}
		}
		else
		{
			//Terrain
			float habitability = std::min(planet.habitability.read(x, y) * 2.0f, 1.0f);
			col = col * (1.0f - habitability) + planet.flora.read(x, y) * habitability;
		}

		//Star color
		float solarMulti = 1.0f - 1.0f / std::max(starMulti, 1.0f);
		//col = col * (1.0f - solarMulti) + starColor * solarMulti;

	}, planet.planetCells);
	
	/*
	planet.colors.call([&](int x, int y){
		return Color(static_cast<int>(planet.habitability.read(x, y) * 255.0f), 0, 0);
	}, planet.planetCells);
	*/
	/*
	planet.colors.run([&](int x, int y, Color& col) {
		if (planet.humidity.read(x, y) > 0.0f) col = Color(255, 255, 255);
	}, planet.planetCells);
	*/
	/*
	planet.colors.call([&](int x, int y) {
		return planet.mineral.read(x, y);
	}, planet.planetCells);
	*/
}