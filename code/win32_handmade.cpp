#include <windows.h>
#include <cstdio>

//TODO: global for now
static bool running;

LRESULT CALLBACK MainWindowCallback(
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
			OutputDebugStringA("WM_SIZE\n");
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

			static DWORD operation = WHITENESS;
			PatBlt(deviceContext, x, y, width, height, operation);

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
	windowClass.lpfnWndProc = MainWindowCallback;
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