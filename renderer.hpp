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
	void Run(long mousePosX, long mousePosY);

private:
	using Vector = DirectX::XMVECTOR;

	bool CreateContext(const Config& config);
	bool CreateRenderTarget();
	bool CreateDepthStencil();
	bool CreateResources();
	bool CreateShaders();
	void CreateMatrices();

	void CreateViewMatrix(const Vector& eye, const Vector& at, const Vector& up);
	bool CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob);

	void Update(long mousePosX, long mousePosY);
	void Render();
	void Present();

	void UpdateArcballCamera(long mousePosX, long mousePosY);

private:
	using Matrix = DirectX::XMMATRIX;

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;
	ComPtr<IDXGISwapChain> _swapChain;

	ComPtr<ID3D11Texture2D> _backBuffer;
	ComPtr<ID3D11RenderTargetView> _renderTarget;
	UINT _backBufferWidth = 0;
	UINT _backBufferHeight = 0;

	ComPtr<ID3D11Texture2D> _depthStencil;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;

	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;
	ComPtr<ID3D11Buffer> _constantBuffer;

	ComPtr<ID3DBlob> _vertexShaderBlob;
	ComPtr<ID3DBlob> _pixelShaderBlob;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11InputLayout> _inputLayout;

	D3D11_VIEWPORT _viewport;
	UINT _indexCount = 0;

	long _lastMousePosX = 0;
	long _lastMousePosY = 0;

	struct Camera {
		Vector _eye;
		Vector _at;
		Vector _up;
	} _camera;

	struct Projection {
		Matrix _model;
		Matrix _view;
		Matrix _projection;
	} _projection;

	struct ConstantBufferData {
		DirectX::XMFLOAT4X4 _mvp;
	} _constantBufferData;
	static_assert((sizeof(ConstantBufferData) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
};
