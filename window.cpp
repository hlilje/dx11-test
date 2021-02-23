#include "window.hpp"

#include <comdef.h>

#include <iostream>


const std::wstring Window::_className = L"Direct3DWindowClass";
HINSTANCE Window::_instance = nullptr;

namespace {
	bool CheckHadError() {
		const DWORD error = GetLastError();
		if (!error)
			return false;

		const _com_error comError(HRESULT_FROM_WIN32(error));
		std::wcout << "ERROR: " << comError.ErrorMessage() << std::endl;

		return true;
	}
}


LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE: {
			if (HMENU hmenu = GetMenu(hwnd))
				DestroyMenu(hmenu);
			DestroyWindow(hwnd);
			UnregisterClass(_className.c_str(), _instance);
			return 0;
		} case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Window::Create() {
	_instance = GetModuleHandle(nullptr);

	WNDCLASS wndClass;
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = Window::WindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = _instance;
	wndClass.hIcon = nullptr;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = _className.c_str();

	if (!RegisterClass(&wndClass))
	{
		if (CheckHadError())
			return false;
	}

	_window = CreateWindow(
		wndClass.lpszClassName,
		L"DX11 Test",
		WS_OVERLAPPEDWINDOW, 0, 0,
		_width, _height,
		nullptr,
		nullptr,
		_instance,
		nullptr);

	if (!_window) {
		if (CheckHadError())
			return false;
	}

	Renderer::Config renderConfig;
	renderConfig._width = _width;
	renderConfig._height = _height;
	renderConfig._window = _window;
	if (!_renderer.Create(renderConfig))
		return false;

	return true;
}

void Window::Run() {
	if (!IsWindowVisible(_window))
		ShowWindow(_window, SW_SHOW);

	MSG message;
	message.message = WM_NULL;

	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		} else {
			_renderer.Run();
		}
	}
}
