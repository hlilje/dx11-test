#pragma once

#include <d3d11.h>


class Renderer {
public:
	struct Config {
		int _width = 0;
		int _height = 0;
		HWND _window = nullptr;
	};
	bool Create(const Config& config);
	void Run();

private:
	void Update();
	void Render();
	void Present();

private:
};
