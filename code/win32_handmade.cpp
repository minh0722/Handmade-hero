#include <windows.h>
#include <cstdio>
#include <stdint.h>
#include <Xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

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

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return 0;
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return 0;
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void Win32LoadXInput()
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	}

}

internal win32_window_dimension Win32GetWindowDimension(HWND window)
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

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer* buffer,
										 HDC deviceContext,
										 int windowWidth, int windowHeight)
{
	// TODO: aspect ratio correction
	StretchDIBits(deviceContext,
		/*x, y, width, height,
		x, y, width, height,*/
		0, 0, windowWidth, windowHeight,
		0, 0, buffer->width, buffer->height,
		buffer->memory,
		&buffer->info,
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

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_KEYDOWN:
		{
			uint32 vkCode = wParam;
			bool wasDown = (lParam & (1 << 30)) != 0;		// 30-th bit tells if key was down
			bool isDown = (lParam & (1 << 31)) == 0;

			if (wasDown != isDown)
			{
				if (vkCode == 'W')
				{
				}
				else if (vkCode == 'A')
				{
				}
				else if (vkCode == 'S')
				{
				}
				else if (vkCode == 'D')
				{
				}
				else if (vkCode == 'Q')
				{
				}
				else if (vkCode == 'E')
				{
				}
				else if (vkCode == VK_UP)
				{
				}
				else if (vkCode == VK_DOWN)
				{
				}
				else if (vkCode == VK_LEFT)
				{
				}
				else if (vkCode == VK_RIGHT)
				{
				}
				else if (vkCode == VK_ESCAPE)
				{
					OutputDebugStringA("ESCAPE ");
					if (isDown)
						OutputDebugStringA("isDown");
					if (wasDown)
						OutputDebugStringA("wasDown");
					OutputDebugStringA("\n");
				}
				else if (vkCode == VK_SPACE)
				{
				}
				else
				{

				}
			}

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
			Win32DisplayBufferInWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);
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
	Win32LoadXInput();

	WNDCLASSA windowClass = {};

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

				// poll for xinput
				// TODO: should we poll this more frequently
				for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
				{
					XINPUT_STATE controllerState;

					if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
					{
						// controller is plugged in
						// TODO: see if controllerState.dwPacketNumber increments too rapidly

						XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

						bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down= (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButtong = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 stickX = pad->sThumbLX;
						int16 stickY = pad->sThumbLY;

						if (AButton)
						{
							++yOffset;
						}
					}
					else
					{
						// controller is not available
					}
				}

				XINPUT_VIBRATION vibration;
				vibration.wLeftMotorSpeed = 50000;
				vibration.wRightMotorSpeed = 50000;
				XInputSetState(0, &vibration);

				RenderWeirdGradient(globalBackBuffer, xOffset, yOffset);
								
				win32_window_dimension dimension = Win32GetWindowDimension(window);
				Win32DisplayBufferInWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);

				ReleaseDC(window, deviceContext);

				++xOffset;
				//++yOffset;
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