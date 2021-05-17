#pragma once

#include "renderer.hpp"

#include <windows.h>

#include <string>


class Window {
public:
	bool Create();
	void Run();

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	static const std::wstring _className;
	static HINSTANCE _instance;
	static int _mouseWheelDelta;

	const int _width = 1500;
	const int _height = 1000;
	HWND _window = nullptr;

	Renderer _renderer;
};
