#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>


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
	bool CreateRenderTarget();
	bool CreateResources();
	bool CreateShaders();
	void CreateMatrices();

	bool CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);

	void Update();
	void Render();
	void Present();

private:
	template<typename T>
	using ComPtrT = Microsoft::WRL::ComPtr<T>;

	ComPtrT<ID3D11Device> _device;
	ComPtrT<ID3D11DeviceContext> _deviceContext;
	ComPtrT<IDXGISwapChain> _swapChain;

	ComPtrT<ID3D11Texture2D> _backBuffer;
	ComPtrT<ID3D11RenderTargetView> _renderTarget;
	UINT _backBufferWidth = 0;
	UINT _backBufferHeight = 0;

	ComPtrT<ID3D11Buffer> _vertexBuffer;
	ComPtrT<ID3D11Buffer> _indexBuffer;

	ComPtrT<ID3DBlob> _vertexShaderBlob;
	ComPtrT<ID3DBlob> _pixelShaderBlob;
	ComPtrT<ID3D11VertexShader> _vertexShader;
	ComPtrT<ID3D11PixelShader> _pixelShader;
	ComPtrT<ID3D11InputLayout> _inputLayout;

	D3D11_VIEWPORT _viewport;
	UINT _indexCount = 0;

	DirectX::XMFLOAT4X4 _world;
	DirectX::XMFLOAT4X4 _view;
	DirectX::XMFLOAT4X4 _projection;

	unsigned int _frame = 0;
};
