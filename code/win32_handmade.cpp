#include <windows.h>
#include <cstdio>

#define internal static
#define local_persist static
#define global_variable static

//TODO: global for now
global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

internal void Win32ResizeDIBSection(int width, int height)
{
	if (bitmapHandle)
	{
		DeleteObject(bitmapHandle);
	}
	if(!bitmapDeviceContext)
	{
		// TODO: should we recreate these under certain special circumstances (unplug monitor, etc.)
		bitmapDeviceContext = CreateCompatibleDC(0);
	}

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;				// bits per pixel
	bitmapInfo.bmiHeader.biCompression = BI_RGB;		// uncompressed
	
	// TODO: allocate this ourselves?
	// create the new buffer
	bitmapHandle = CreateDIBSection(
		bitmapDeviceContext,
		&bitmapInfo,
		DIB_RGB_COLORS,
		&bitmapMemory,
		nullptr,
		0);


}

internal void Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
	StretchDIBits(deviceContext,
		x, y, width, height,
		x, y, width, height,
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
			Win32UpdateWindow(deviceContext, x, y, width, height);
			
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