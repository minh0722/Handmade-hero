#include <windows.h>
#include <cstdio>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

//TODO: global for now
global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;

internal void Win32ResizeDIBSection(int width, int height)
{
	if (bitmapMemory)
	{
		VirtualFree(bitmapMemory, 0, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;		// use a top-down DIB
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;				// bits per pixel
	bitmapInfo.bmiHeader.biCompression = BI_RGB;		// uncompressed
	
	// NOTE: no more DC. We can allocate memory ourselves
	int bytesPerPixel = 4;
	int bitmapMemorySize = (width * height) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(nullptr, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	int pitch = width * bytesPerPixel;
	uint8* row = (uint8*)bitmapMemory;
	for (int y = 0; y < bitmapHeight; ++y)
	{
		uint8* pixel = (uint8*)row;
		for (int x = 0; x < bitmapWidth; ++x)
		{
			/*
			pixel in memory: rr gg bb xx
			*/
			*pixel = 255;
			++pixel;

			*pixel = 0;
			++pixel;

			*pixel = 0;
			++pixel;

			*pixel = 0;
			++pixel;
		}
		row += pitch;
	}
}

internal void Win32UpdateWindow(HDC deviceContext, RECT* windowRect, int x, int y, int width, int height)
{
	int windowWidth = windowRect->right - windowRect->left;
	int windowHeight = windowRect->bottom - windowRect->top;

	int cnt = StretchDIBits(deviceContext,
		/*x, y, width, height,
		x, y, width, height,*/
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight,
		&bitmapMemory,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		// when window is resized
		case WM_SIZE:
		{
			// get rect of the window excluding the borders
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			char buf[256];
			sprintf(buf, "x: %d\n", clientRect.left);
			OutputDebugStringA(buf);

			sprintf(buf, "y: %d\n", clientRect.top);
			OutputDebugStringA(buf);

			sprintf(buf, "width: %d\n", width);
			OutputDebugStringA(buf);

			sprintf(buf, "height: %d\n", height);
			OutputDebugStringA(buf);


			Win32ResizeDIBSection(width, height);
		} break;

		case WM_DESTROY:
		{
			// TODO: handle this as an error - recreate window?
			running = false;
		} break;

		case WM_CLOSE:
		{
			// TODO: handle this with a message to the user? because we may want to close just an internal window in the game
			running = false;
		} break;

		// when window becomes the active window
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;

			RECT clientRect;
			GetClientRect(window, &clientRect);
			
			Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
			
			EndPaint(window, &paint);
		} break;

		default:
		{
			//OutputDebugStringA("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	WNDCLASS windowClass = {};
	
	// TODO: Check if these flags still matter
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = Instance; // or GetModuleHandle(0);
	//windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";
	
	// register window class before creating the window
	if (RegisterClass(&windowClass))
	{
		HWND windowHandle = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);

		if (windowHandle)
		{
			// start message loop
			MSG message;
			running = true;
			while (running)
			{
				BOOL messageResult = GetMessageA(&message, 0, 0, 0);
				if (messageResult > 0)
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// TODO: logging
		}
	}
	else
	{
		//TODO: logging
	}

	return 0;
}