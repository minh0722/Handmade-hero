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
struct win32_offscreen_buffer
{
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct win32_window_dimension
{
	int width;
	int height;
};

global_variable bool globalRunning;
global_variable win32_offscreen_buffer globalBackBuffer;

win32_window_dimension Win32GetWindowDimension(HWND window)
{
	win32_window_dimension result;

	RECT clientRect;
	GetClientRect(window, &clientRect);

	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

	return result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer buffer, int blueOffset, int greenOffset)
{
	// TODO: let's see what the optimizer does with pointer aliasing

	uint8* row = (uint8*)buffer.memory;

	for (int y = 0; y < buffer.height; ++y)
	{
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer.width; ++x)
		{
			uint8 blue = (x + blueOffset);
			uint8 green = (y + greenOffset);
			*pixel++ = (green << 8) + blue;
		}
		row += buffer.pitch;
	}
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer* buffer, int width, int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	int bytesPerPixel = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
	buffer->info.bmiHeader.biHeight = -height;		// use a top-down DIB
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;				// bits per pixel
	buffer->info.bmiHeader.biCompression = BI_RGB;		// uncompressed
	
	// NOTE: no more DC. We can allocate memory ourselves
	int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(nullptr, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * bytesPerPixel;

	// TODO: clear to black
}

internal void Win32DisplayBufferInWindow(HDC deviceContext, 
										 int windowWidth, int windowHeight,
										 win32_offscreen_buffer buffer)
{
	// TODO: aspect ratio correction
	StretchDIBits(deviceContext,
		/*x, y, width, height,
		x, y, width, height,*/
		0, 0, windowWidth, windowHeight,
		0, 0, buffer.width, buffer.height,
		buffer.memory,
		&buffer.info,
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
		} break;

		case WM_DESTROY:
		{
			// TODO: handle this as an error - recreate window?
			globalRunning = false;
		} break;

		case WM_CLOSE:
		{
			// TODO: handle this with a message to the user? because we may want to close just an internal window in the game
			globalRunning = false;
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
			
			win32_window_dimension dimension = Win32GetWindowDimension(window);			
			Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, 
										globalBackBuffer);
			EndPaint(window, &paint);
		} break;

		default:
		{
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	WNDCLASS windowClass = {};

	// resize here, not in the WM_RESIZE event
	Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);
	
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = Instance; // or GetModuleHandle(0);
	//windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";
	
	// register window class before creating the window
	if (RegisterClass(&windowClass))
	{
		HWND window = CreateWindowExA(
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

		if (window)
		{
			// NOTE: we used CS_OWNDC so we can get one device context and use it
			// forever because we are not sharing it.
			HDC deviceContext = GetDC(window);

			// start message loop
			globalRunning = true;

			int xOffset = 0;
			int yOffset = 0;

			while (globalRunning)
			{
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						globalRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				RenderWeirdGradient(globalBackBuffer, xOffset, yOffset);
								
				win32_window_dimension dimension = Win32GetWindowDimension(window);
				Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, globalBackBuffer);

				ReleaseDC(window, deviceContext);

				++xOffset;
				++yOffset;
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