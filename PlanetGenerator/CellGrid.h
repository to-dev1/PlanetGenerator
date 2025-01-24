#pragma once

#include <vector>
#include <functional>

template<typename T>
class CellGrid
{
public:
	const int width;
	const int height;
	
	CellGrid(int h = 100, int w = 100) : width(w), height(h)
	{
		grid.resize(width * height);
	}

	size_t getIndex(int x, int y) const
	{
		return static_cast<size_t>(x + y * width);
	}

	void set(int x, int y, const T& val)
	{
		grid[getIndex(x, y)] = val;
	}

	//Check that a cell are active before changing it
	void set(int x, int y, const T& val, const CellGrid<int>& active)
	{
		if(!active.read(x, y)) grid[getIndex(x, y)] = val;
	}

	void setAround(int x, int y, const T& val)
	{
		if (x + 1 < width)
		{
			set(x + 1, y, val);
		}
		if (x - 1 >= 0)
		{
			set(x - 1, y, val);
		}
		if (y + 1 < height)
		{
			set(x, y + 1, val);
		}
		if (y - 1 >= 0)
		{
			set(x, y - 1, val);
		}
	}

	//Check that a cell are active before changing it
	void setAround(int x, int y, const T& val, const CellGrid<int>& active)
	{
		if (x + 1 < width)
		{
			set(x + 1, y, val, active);
		}
		if (x - 1 >= 0)
		{
			set(x - 1, y, val, active);
		}
		if (y + 1 < height)
		{
			set(x, y + 1, val, active);
		}
		if (y - 1 >= 0)
		{
			set(x, y - 1, val, active);
		}
	}

	T& at(int x, int y)
	{
		return grid[getIndex(x, y)];
	}

	const T& read(int x, int y) const
	{
		return grid[getIndex(x, y)];
	}

	void call(const std::function<T(int, int)>& func, const CellGrid<int>& active)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if(!active.read(x, y)) set(x, y, func(x, y));
			}
		}
	}

	template<typename Func>
	void run(Func func, const CellGrid<int>& active) //void run(const std::function<void(int, int, T&)>& func, const CellGrid<int>& active)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (!active.read(x, y)) func(x, y, at(x, y));
			}
		}
	}

	void copy(const CellGrid<T>& src)
	{
		grid = src.grid;
	}

private:
	std::vector<T> grid;
};