#include <windows.h>
#include <windowsx.h>

/*
	prototype for WndProc
*/
LRESULT CALLBACK WindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

/*
	Entry point for windows program
*/
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	
	HWND hWnd;
	WNDCLASSEX wc;

	//clear out the window class for use (initializes a block of memory of size WNDCLASSEX to NULL)
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = LPCSTR("WindowClass1");

	RegisterClassEx(&wc);

	RECT wr = { 0,0,500,500 }; //sets the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(
		NULL,
		LPCSTR("WindowClass1"),
		LPCSTR("DX11 Window!"),
		WS_OVERLAPPEDWINDOW,
		300,//x pos
		300,//y pos
		wr.right - wr.left,//width
		wr.bottom - wr.top,//height
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nShowCmd);

	//MSG is a struct that holds Windows events messages
	MSG msg;

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		else
		{
			//run game code...
		}
	}

	return msg.wParam;
}

/*
	main message handler for the program
*/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}