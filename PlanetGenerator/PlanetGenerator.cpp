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

int Frames;

LRESULT CALLBACK windowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{
	case WM_CLOSE:
	{
		running = false;
		break;
	}
	case WM_DESTROY:
	{
		running = false;
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

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//Create
	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpszClassName = "GWclass";
	windowClass.lpfnWndProc = windowCallback;

	//Register
	RegisterClass(&windowClass);

	//Window
	HWND window = CreateWindow(windowClass.lpszClassName, "Planet generator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, 0, 0, hInstance, 0);
	HDC hdc = GetDC(window);

	Planet* testPlanet = new Planet(200, Vector2(800, 500));
	Generator generator = Generator(1);
	generator.generate(*testPlanet);

	Planet* planet2 = new Planet(30, Vector2(1200, 500), Vector2(0, 150));
	generator.generate(*planet2);

	World world = World();
	world.add(testPlanet);
	world.add(planet2);

	//done
	
	float deltaTime = 0.0f;

	while (running == true)
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		//Input
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		Frames += 1;

		//Clear buffer
		memset(bufferMemory, 0, bufferWidth * bufferHeight * sizeof(unsigned int));

		world.update(deltaTime);
		world.render((unsigned int*)bufferMemory, bufferWidth, bufferHeight);

		//Render using Windows
		StretchDIBits(hdc, 0, 0, bufferWidth, bufferHeight, 0, 0, bufferWidth, bufferHeight, bufferMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

		//Time system
		auto endTime = std::chrono::high_resolution_clock::now();
		auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		deltaTime = static_cast<float>(totalTime.count()) * 0.000001f;

		std::ostringstream timeString;

		timeString << "| Current fps: " << 1000000.0f / totalTime.count() << ", delta time: " << deltaTime;

		OutputDebugString(timeString.str().c_str());
		OutputDebugString("|");
		OutputDebugString("\n");
	}
}
