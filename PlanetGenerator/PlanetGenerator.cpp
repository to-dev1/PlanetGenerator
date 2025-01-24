#include <windows.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <chrono>

#include "Planet.h"

bool running = true;

//Windows
void* bufferMemory;
int bufferWidth;
int bufferHeight;
BITMAPINFO bitmapInfo;

//Cell system
std::vector<int> Cells;
std::vector<int> ChangedCells;
std::vector<int> DeactiveCells;

//Simulation
std::vector<int> Power;
std::vector<int> Light;
std::vector<int> Humidity;
std::vector<int> Height;
std::vector<int> ChangedHeight;

//Rendering
std::vector<int> Background;
std::vector<int> RenderArray;

int RenderMode = 0;
int SelectedRenderMode = 0;

//Misc
int Frames;
int repeatTotal = 0;

float DeltaTime = 0.0f;
float Update = 0.0f;

std::chrono::duration<float> TotalTime;

int seed = 1343141;

LRESULT CALLBACK windowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{
	case WM_CLOSE:
	{
		running = false;

		std::ostringstream s;
		s << Frames;

		OutputDebugString("Frames: ");
		OutputDebugString(s.str().c_str());
		OutputDebugString("|");

		std::ostringstream s2;
		s2 << repeatTotal;

		OutputDebugString(s2.str().c_str());
		OutputDebugString("|");

		break;
	}
	case WM_DESTROY:
	{
		running = false;

		std::ostringstream s;
		s << Frames;

		OutputDebugString("Frames: ");
		OutputDebugString(s.str().c_str());
		OutputDebugString("|");

		std::ostringstream s2;
		s2 << repeatTotal;

		OutputDebugString(s2.str().c_str());
		OutputDebugString("|");

		break;
	}

	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		bufferWidth = rect.right - rect.left;
		bufferHeight = rect.bottom - rect.top;

		int bufferSize = bufferWidth * bufferHeight * sizeof(unsigned int);

		if (bufferMemory)
		{
			VirtualFree(bufferMemory, 0, MEM_RELEASE);
		}

		bufferMemory = VirtualAlloc(0, bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
		bitmapInfo.bmiHeader.biWidth = bufferWidth;
		bitmapInfo.bmiHeader.biHeight = bufferHeight;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;

		break;
	}


	default:
	{
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return result;
}

inline int Random()
{
	seed = (214013 * seed + 2531011);
	return (seed >> 16) & 0x7FFF;
}

inline int RandomMK2(int InputSpeed)
{
	seed = (214013 * seed + InputSpeed);
	return (seed >> 16) & 0x7FFF;
}

inline void ChangeCell(int x, int y, int type, int type2)
{
	int pos = x + y * bufferWidth;

	if (Height[pos] != 0)
	{
		if (Cells[pos] != -100 && ChangedCells[pos] != -100 && Cells[pos] > 0)
		{
			ChangedCells[pos] = type;
			Power[pos] = type2;
		}
		if (type != -1)
		{
			ChangedCells[pos] = type;
		}
	}
}

inline void ChangeCellDirect(int x, int y, int type)
{
	int pos = x + y * bufferWidth;

	Cells[pos] = type;
}

inline void ChangeSurroundingCells(int x, int y, int type, int type2)
{
	if (x + 1 < bufferWidth)
	{
		ChangeCell(x + 1, y, type, type2);
	}
	if (x - 1 >= 0)
	{
		ChangeCell(x - 1, y, type, type2);
	}
	if (y + 1 < bufferHeight)
	{
		ChangeCell(x, y + 1, type, type2);
	}
	if (y - 1 >= 0)
	{
		ChangeCell(x, y - 1, type, type2);
	}
}

inline void ChangeHeight(int x, int y, int type)
{
	int pos = x + y * bufferWidth;

	if (Height[pos] == 1)
	{
		ChangedHeight[pos] = type;
	}
}

inline void ChangeSurroundingHeight(int x, int y, int type)
{
	if (x + 1 < bufferWidth)
	{
		ChangeHeight(x + 1, y, type);
	}
	if (x - 1 >= 0)
	{
		ChangeHeight(x - 1, y, type);
	}
	if (y + 1 < bufferHeight)
	{
		ChangeHeight(x, y + 1, type);
	}
	if (y - 1 >= 0)
	{
		ChangeHeight(x, y - 1, type);
	}
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Create
	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpszClassName = "GWclass";
	windowClass.lpfnWndProc = windowCallback;

	// Register
	RegisterClass(&windowClass);

	// Window
	//1280, 720
	//448, 448
	HWND window = CreateWindow(windowClass.lpszClassName, "Planet generator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, 0, 0, hInstance, 0);
	HDC hdc = GetDC(window);

	int test = 0;

	Cells.resize(bufferHeight * bufferWidth);
	Power.resize(bufferHeight * bufferWidth);
	Light.resize(bufferHeight * bufferWidth);
	Humidity.resize(bufferHeight * bufferWidth);
	Height.resize(bufferHeight * bufferWidth);
	ChangedHeight.resize(bufferHeight * bufferWidth);

	Background.resize(bufferHeight * bufferWidth);
	RenderArray.resize(bufferHeight * bufferWidth);

	DeactiveCells.resize(bufferHeight * bufferWidth);

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			Cells[x + y * bufferWidth] = 0;
			DeactiveCells[x + y * bufferWidth] = 0;
		}
	}

	int CenterX = bufferWidth / 2;
	int CenterY = bufferHeight / 2;

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			int X = CenterX - x;
			int Y = CenterY - y;

			//400
			if (std::sqrt(std::pow(X, 2) + std::pow(Y, 2)) < 350)
			{
				Cells[x + y * bufferWidth] = 1;
				Height[x + y * bufferWidth] = 1;
			}
		}
	}

	//Height simulation

	ChangedHeight = Height;

	int Layer = 0;

	//56754675, 1231283, 1390989, 9999999, 3879657, 38796571, 78767889, 457879, 7856423

	//89431, 12345678, 354367, 567225

	//Red: 846916791, 760, 888, 321731

	//Purple: 78767889

	//Habitable: 56754675, 74519891, 9833467968, 42

	//846916791

	int HeightSeed = 42;

	int AtmosphereSeed = RandomMK2(HeightSeed) % (10000000) + 5000000;

	float AtmosphereThickness = (float)(RandomMK2(AtmosphereSeed) % (20) / 10.0f);
	float WaterLevel = (float)(RandomMK2(HeightSeed) % (10)) / 30.0f + 0.7f;
	float HumidityMultiplier = (0.5f - (float)(RandomMK2(HeightSeed) % (10)) / 20.0f) + 0.5f;
	float LightMultiplier = (float)(RandomMK2(AtmosphereSeed) % (5) / 10.0f) + 0.5f;

	HumidityMultiplier *= AtmosphereThickness;

	if (HumidityMultiplier > 1.0f)
	{
		HumidityMultiplier = 1.0f;
	}

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			int CellType = Cells[x + y * bufferWidth];

			if (CellType != 0)
			{
				if (RandomMK2(HeightSeed) % (4000) == 1)
				{
					ChangeHeight(x, y, 2);
				}
			}
		}
	}

	Height = ChangedHeight;

	int LargestHeight = 0;

	for (int i = 0; i < 1000; i++)
	{
		for (int y = 0; y < bufferHeight; y++)
		{
			for (int x = 0; x < bufferWidth; x++)
			{
				int CellType = Height[x + y * bufferWidth];

				if (CellType > 1)
				{
					//Random() % (5 + CellType) == 1

					//int RandomLimit = (int)(RandomMK2(x) % (10)) + 1;

					if (RandomMK2(HeightSeed) % (15) == 1)
					{
						ChangeSurroundingHeight(x, y, CellType + 1);

						if (CellType + 1 > LargestHeight)
						{
							LargestHeight = CellType + 1;
						}
					}
				}
			}
		}

		Height = ChangedHeight;

		Layer += 1;
	}

	//y = sqrt(160000 - x^2)

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			if (Cells[x + y * bufferWidth] != 0)
			{
				//int Value = 400 - std::abs(CenterY - y);
				int Value = (int)(std::sqrt(160000 - std::pow(std::abs(CenterY - y), 2)));

				Light[x + y * bufferWidth] = Value * LightMultiplier;
			}
		}
	}

	//300, 350
	int LiquidForm = 350;

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			if (Cells[x + y * bufferWidth] != 0)
			{
				//400, 310, 360
				int Value = 360 - std::abs(LiquidForm - Light[x + y * bufferWidth]);
				int Value2 = (0.5f + ((float)Height[x + y * bufferWidth] / (float)LargestHeight) / 2.0f) * (float)Value * 2.0f;

				if (Value2 > 400)
				{
					Value2 = 400;
				}

				Humidity[x + y * bufferWidth] = Value;
			}
		}
	}

	bool Habitable = false;

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			if (Cells[x + y * bufferWidth] != 0)
			{
				float CurrentLight = (float)Light[x + y * bufferWidth] / 400.0f;
				float CurrentHumidity = ((float)(Humidity[x + y * bufferWidth])) / 400.0f * HumidityMultiplier;
				float CurrentHeight = (1.0f - (float)((float)Height[x + y * bufferWidth] / (float)LargestHeight)) * 0.3f + 0.7f;
				float CurrentHeightInverse = (float)((float)Height[x + y * bufferWidth] / (float)LargestHeight) * 0.3f + 0.7f;
				if (CurrentHeight > WaterLevel)
				{
					//Cells[x + y * bufferWidth] = RGB((int)(CurrentLight * 255.0f * 0.2f * CurrentHeight) + (int)(CurrentHumidity * 255.0f * 0.2f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.5f * CurrentHeight) + (int)(CurrentHumidity * 255.0f * 0.3f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.7f * CurrentHeight));

					if (CurrentHeight > 0.9f && CurrentHeight < 1.0f && CurrentHeight * CurrentHumidity * CurrentHumidity * CurrentLight > 0.6f)
					{
						if (RandomMK2(HeightSeed) % ((int)(CurrentHeight - 0.9f * 10.0f)) == 1)
						{
							Cells[x + y * bufferWidth] = RGB((int)((1.0f - CurrentLight) * 255.0f * 0.48f * CurrentHeight), (int)(CurrentHumidity * 255.0f * 0.48f * CurrentHeight) + (int)(CurrentLight * 255.0f * 0.1f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.45f * CurrentHeight));
						}
						else
						{
							Cells[x + y * bufferWidth] = RGB((int)((1.0f - CurrentLight) * 255.0f * 0.5f * CurrentHeight), (int)(CurrentHumidity * 255.0f * 0.5f * CurrentHeight) + (int)(CurrentLight * 255.0f * 0.1f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.5f * CurrentHeight));
						}

						Habitable = true;
					}
					else
					{
						Cells[x + y * bufferWidth] = RGB((int)(((1.0f - CurrentLight) * CurrentLight) * 255.0f * 0.5f * CurrentHeight), (int)(CurrentHumidity * 255.0f * 0.5f * CurrentHeight) + (int)(CurrentLight * 255.0f * 0.1f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.5f * CurrentHeight));
					}
				}
				else
				{
					//Cells[x + y * bufferWidth] = RGB((int)(CurrentLight * 255.0f * 0.5f * CurrentHeightInverse) + (int)(CurrentHumidity * 255.0f * 0.2f * CurrentHeightInverse), (int)(CurrentLight * 255.0f * 0.5f * CurrentHeight) + (int)(CurrentHumidity * 255.0f * 0.3f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.7f * CurrentHeight));

					Cells[x + y * bufferWidth] = RGB((int)(CurrentLight * 255.0f * 0.5f * CurrentHeight), (int)(CurrentHumidity * 255.0f * 0.5f * CurrentHeight) + (int)(CurrentLight * 255.0f * 0.1f * CurrentHeight), (int)(CurrentLight * 255.0f * 0.5f * CurrentHeight));
				}
			}
		}
	}

	//Background

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			if (RandomMK2(HeightSeed) % 3000 == 1)
			{
				Background[x + y * bufferWidth] = RGB(10, RandomMK2(HeightSeed) % 255, RandomMK2(HeightSeed) % 255);
			}
			else
			{
				//Background[x + y * bufferWidth] = RGB(RandomMK2(HeightSeed) % 5, RandomMK2(HeightSeed) % 5, RandomMK2(HeightSeed) % 5);
			}
		}
	}

	//Final render

	for (int y = 0; y < bufferHeight; y++)
	{
		for (int x = 0; x < bufferWidth; x++)
		{
			if (Cells[x + y * bufferWidth] > 0)
			{
				RenderArray[x + y * bufferWidth] = Cells[x + y * bufferWidth];
			}
			else
			{
				RenderArray[x + y * bufferWidth] = Background[x + y * bufferWidth];
			}
		}
	}

	std::ostringstream string1;

	string1 << "------------------------------------" << "\n";
	string1 << "Max height: " << LargestHeight << "\n";
	string1 << "Habitable: " << Habitable << "\n";
	string1 << "Water level: " << WaterLevel << "\n";
	string1 << "Humidity: " << HumidityMultiplier << "\n";
	string1 << "Atmosphere: " << AtmosphereThickness << "\n";
	string1 << "Light: " << LightMultiplier << "\n";
	string1 << "------------------------------------" << "\n";

	OutputDebugString(string1.str().c_str());
	OutputDebugString("\n");

	int StartPower = 10000;

	Planet testPlanet = Planet(200);
	Generator generator = Generator(1);
	generator.generate(testPlanet);

	//done

	while (running == true)
	{
		// Input
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		int repeat = 0;

		Frames += 1;

		TotalTime = TotalTime.zero();

		int typeFrame = 50000 - Frames;

		if (typeFrame < 2)
		{
			typeFrame = 1;
		}

		if (GetAsyncKeyState(0x31) < 0)
		{
			SelectedRenderMode = 1;
		}
		if (GetAsyncKeyState(0x32) < 0)
		{
			SelectedRenderMode = 2;
		}
		if (GetAsyncKeyState(0x33) < 0)
		{
			SelectedRenderMode = 3;
		}

		if (GetAsyncKeyState(VK_LBUTTON) < 0)
		{
			POINT p;

			GetCursorPos(&p);

			int x = p.x;
			int y = p.y;

			int pos = p.x - 8 + (bufferHeight - p.y + 30) * bufferWidth;

			if (Height[pos] > 0)
			{
				Cells[pos] = -1;
				Power[pos] = StartPower;
			}
		}

		if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			RenderMode = 1;
		}
		else
		{
			RenderMode = 0;
		}

		auto startTime = std::chrono::high_resolution_clock::now();

		if (Update < 0.0f)
		{
			ChangedCells = Cells;
			ChangedCells.resize(bufferHeight * bufferWidth);

			for (int y = 0; y < bufferHeight; y++)
			{
				for (int x = 0; x < bufferWidth; x++)
				{
					int CellType = Cells[x + y * bufferWidth];

					if (CellType < 0)
					{
						if (CellType > -20)
						{
							if (RandomMK2(HeightSeed) % 3 == 1)
							{
								if (CellType == -1 && Power[x + y * bufferWidth] > 0)
								{
									if (RandomMK2(AtmosphereSeed) % 3 == 1)
									{
										ChangeSurroundingCells(x, y, -1, Power[x + y * bufferWidth]);
									}
									else
									{
										ChangeSurroundingCells(x, y, -1, Power[x + y * bufferWidth] - 1);
									}
								}

								if (CellType == -1 && Power[x + y * bufferWidth] <= 0)
								{
									if (RandomMK2(AtmosphereSeed) % (Power[x + y * bufferWidth] * -1 + 3) == 1)
									{
										ChangeSurroundingCells(x, y, -1, Power[x + y * bufferWidth] - 1);
									}
								}

								ChangeCell(x, y, CellType - 1, 0);
							}
						}
						else
						{
							ChangeCell(x, y, -100, 0);
						}
					}
				}
			}

			Update = 1.0f;

			Cells = ChangedCells;

			for (int y = 0; y < bufferHeight; y++)
			{
				for (int x = 0; x < bufferWidth; x++)
				{
					if (Cells[x + y * bufferWidth] > 0)
					{
						RenderArray[x + y * bufferWidth] = Cells[x + y * bufferWidth];
					}
					else
					{
						if (Cells[x + y * bufferWidth] < 0)
						{
							float CurrentHeight = (1.0f - (float)((float)Height[x + y * bufferWidth] / (float)LargestHeight)) * 0.7f + 0.3f;

							if (Cells[x + y * bufferWidth] != -100)
							{
								RenderArray[x + y * bufferWidth] = RGB(10, 100 + 4 * Cells[x + y * bufferWidth], 255 + 10 * Cells[x + y * bufferWidth]);
							}
							else
							{
								RenderArray[x + y * bufferWidth] = RGB(50 * CurrentHeight, 50 * CurrentHeight, 50 * CurrentHeight);
							}
						}
						else
						{
							RenderArray[x + y * bufferWidth] = Background[x + y * bufferWidth];
						}
					}
				}
			}
		}

		// - Colour render -

		if (RenderMode == 0)
		{
			for (int y = 0; y < bufferHeight; y++)
			{
				unsigned int* pixel = (unsigned int*)bufferMemory;

				for (int x = 0; x < bufferWidth; x++)
				{
					//*(pixel + x + y * bufferWidth) = Cells[x + y * bufferWidth];
					*(pixel + x + y * bufferWidth) = RenderArray[x + y * bufferWidth];
				}
			}
		}
		else
		{
			if (SelectedRenderMode == 1)
			{
				// - Light render -

				for (int y = 0; y < bufferHeight; y++)
				{
					unsigned int* pixel = (unsigned int*)bufferMemory;

					for (int x = 0; x < bufferWidth; x++)
					{
						//*(pixel + x + y * bufferWidth) = Light[x + y * bufferWidth];
						float val = (float)Light[x + y * bufferWidth] / 400.0f;
						*(pixel + x + y * bufferWidth) = RGB((int)(val * 255.0f), (int)(val * 255.0f), 255);
					}
				}
			}

			if (SelectedRenderMode == 2)
			{
				// - Humidity render -

				for (int y = 0; y < bufferHeight; y++)
				{
					unsigned int* pixel = (unsigned int*)bufferMemory;

					for (int x = 0; x < bufferWidth; x++)
					{
						//*(pixel + x + y * bufferWidth) = Light[x + y * bufferWidth];
						float val = (float)Humidity[x + y * bufferWidth] / 400.0f;
						*(pixel + x + y * bufferWidth) = RGB((int)(val * 255.0f), 0, 0);
					}
				}
			}

			if (SelectedRenderMode == 3)
			{
				// - Height render -

				for (int y = 0; y < bufferHeight; y++)
				{
					unsigned int* pixel = (unsigned int*)bufferMemory;

					for (int x = 0; x < bufferWidth; x++)
					{
						//*(pixel + x + y * bufferWidth) = Light[x + y * bufferWidth];
						float val = (float)Height[x + y * bufferWidth] / 40.0f;
						*(pixel + x + y * bufferWidth) = Height[x + y * bufferWidth] * 1000;
					}
				}
			}
		}

		testPlanet.draw(100, 100, (unsigned int*)bufferMemory, bufferWidth, bufferHeight);

		//Render using Windows
		StretchDIBits(hdc, 0, 0, bufferWidth, bufferHeight, 0, 0, bufferWidth, bufferHeight, bufferMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

		if (test > bufferWidth)
		{
			test = 0;
		}

		test++;

		//Time system
		auto endTime = std::chrono::high_resolution_clock::now();
		TotalTime += endTime - startTime;

		std::chrono::microseconds TotalTime2 = std::chrono::duration_cast<std::chrono::microseconds>(TotalTime);

		DeltaTime = 1.0f / (TotalTime2.count() / 1000000.0f);

		std::ostringstream timeString;

		timeString << TotalTime2.count() << "| Current fps: " << 1000000.0f / TotalTime2.count();

		OutputDebugString("Total time: ");
		OutputDebugString(timeString.str().c_str());
		OutputDebugString("|");
		OutputDebugString("\n");

		Update -= 1.0f / (DeltaTime / 20.0f);
	}
}
