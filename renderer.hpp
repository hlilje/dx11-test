#pragma once

#include <wrl.h>
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

	bool CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);

	void Update();
	void Render();
	void Present();

private:
	template<typename T>
	using ComPtrT = Microsoft::WRL::ComPtr<T>;

	ComPtrT<ID3D11Device> _device = nullptr;
	ComPtrT<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtrT<IDXGISwapChain> _swapChain = nullptr;

	ComPtrT<ID3D11Buffer> _vertexBuffer = nullptr;
	ComPtrT<ID3D11Buffer> _indexBuffer = nullptr;

	ComPtrT<ID3DBlob> _vertexShader = nullptr;
	ComPtrT<ID3DBlob> _pixelShader = nullptr;
};
