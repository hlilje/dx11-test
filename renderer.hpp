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
	bool CreateContext(const Config& config);
	bool CreateResources();
	bool CreateShaders();

	void Update();
	void Render();
	void Present();

private:
	IDXGISwapChain* _swapChain = nullptr;
	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext* _deviceContext = nullptr;

	ID3D11Buffer* _vertexBuffer = nullptr;
	ID3D11Buffer* _indexBuffer = nullptr;
};
